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
	$self->{'filename'} = "";

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

	if ($self->{'filename'} eq "") {
		$self->LOG_ERROR("filename not defined");
		return 0;
	}
	return 1;
}

sub Finish {
	my ($self) = @_;

	my $fd;
	if (not open($fd, ">:encoding(UTF-8)", $self->{'filename'})) {
		$self->LOG_ERROR("Cannot open file: ".$self->{'filename'}.": $!");
		return;
	}
	my @words = sort { $self->{'_freq'}->{$b} <=> $self->{'_freq'}->{$a} } (keys %{$self->{'_freq'}});
	for (my $i = 0; $i < @words and $i < $self->{'maxWords'}; $i++) {
		my $word = $words[$i];
		print $fd $word."\t".($self->{'_freq'}->{$word}/$self->{'_total'})."\n";
	}
	close($fd);
}

sub DESTROY {
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetFormCount();
	for (my $i = 0; $i < $nWords; $i++) {
		my $flags = $tr->GetFlags($i);
		my $word = $tr->GetForm($i);
		next if (($flags & $HectorRobot::TextResource::TOKEN_PUNCT) or ($flags & $HectorRobot::TextResource::TOKEN_NUMERIC and $word =~ /^[0-9]+$/));
		utf8::decode($word);
		$self->{'_freq'}->{lc($word)}++;
		$self->{'_total'}++;
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
