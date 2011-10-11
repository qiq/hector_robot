# DetectLanguageTableTrain.pm, simple, perl
# Extract top N words that do not occur in other languages. We consider lower-cased words.
# 
# Dependencies:
# 
# Parameters:
# items		r/o	n/a	Total items processed
# maxWords	r/o	n/a	Max number of words to save in a file.
# filenamePrefix	r/o	n/a	Files containing unaccented words, one word per line.
# 
# Status:
# not changed

package DetectLanguageTableTrain;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);
use HectorRobotCommon;
use utf8;

binmode STDOUT, ":utf8";

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'maxWords'} = 100000;
	$self->{'filenamePrefix'} = "";

	$self->{'_freq'} = {};

	bless($self, $class);
	return $self;
}

sub Init {
	my ($self, $params) = @_;

	# second stage?
	return 1 if (not defined $params);

	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}

	if ($self->{'filenamePrefix'} eq "") {
		$self->LOG_ERROR("filenamePrefix not defined");
		return 0;
	}
	return 1;
}

sub Finish {
	my ($self) = @_;

	# count weights
	my @langs = keys %{$self->{'_freq'}};
	foreach my $lang (@langs) {
		my $total = $self->{'_freq'}->{$lang}->{' total'};
		delete $self->{'_freq'}->{$lang}->{' total'};
		foreach my $word (keys %{$self->{'_freq'}->{$lang}}) {
			$self->{'_freq'}->{$lang}->{$word} /= $total;
		}
	}

	foreach my $lang (@langs) {
		my $lf = $self->{'_freq'}->{$lang};
		my @words = sort { $lf->{$b} <=> $lf->{$a} } (keys %{$lf});
		my @result;
		for (my $i = 0; $i < @words and @result < $self->{'maxWords'}; $i++) {
			my $word = $words[$i];
#			my $n = 0;
#			foreach my $l (@langs) {
#				next if ($l eq $lang);
#				if (not exists $self->{'_freq'}->{$l}->{$word} or ($lf->{$word}*0.05 > ($self->{'_freq'}->{$l}->{$word}))) {
#					$n++;
#				} else {
#					print "out: $word (".$lf->{$word}."*0.05 <= ".$self->{'_freq'}->{$l}->{$word}.")\n";
#				}
#			}
#			push(@result, $word) if ($n == @langs-1);
push(@result, $word."\t".$lf->{$word});
		}
		my $fd;
		if (not open($fd, ">:encoding(UTF-8)", $self->{'filenamePrefix'}.'.'.$lang)) {
			$self->LOG_ERROR("Cannot open file: ".$self->{'filename'}.'.'.$lang.": $!");
			return;
		}
		foreach my $r (@result) {
			print $fd "$r\n";
		}
		close($fd);
	}
}

sub DESTROY {
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetFormCount();
	my $langs = $tr->GetLanguage();
	my @langs = split(/\s+/, $langs);
	my $lf;
	for (my $i = 0; $i < $nWords; $i++) {
		my $flags = $tr->GetFlags($i);
		if ($flags & $HectorRobot::TextResource::TOKEN_PARAGRAPH_START) {
			my $l = shift(@langs);
			$self->{'_freq'}->{$l} = {} if (not exists $self->{'_freq'}->{$l});
			$lf = $self->{'_freq'}->{$l};
		}
		my $word = $tr->GetForm($i);
		next if (($flags & $HectorRobot::TextResource::TOKEN_PUNCT) or ($flags & $HectorRobot::TextResource::TOKEN_NUMERIC and $word =~ /^[0-9]+$/));
		utf8::decode($word);
		$lf->{lc($word)}++;
		$lf->{' total'}++;
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
