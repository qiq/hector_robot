# DetectLanguageTable.pm, simple, perl
# Detect language in doc/paragraphs using top most common words for every language.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# filenamePrefix	r/o	n/a	Files containing unaccented words, one
#					word per line.
# paragraphLevel	r/w	1	Work on paragraph level (otherwise whole document)
# 
# Status:
# not changed

package DetectLanguageTable;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);
use HectorRobotCommon;

binmode STDOUT, ":utf8";

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'filenamePrefix'} = "";
	$self->{'paragraphLevel'} = 1;

	$self->{'_words'} = {};

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
		$self->LOG_ERROR("filename not defined");
		return 0;
	}
	foreach my $f (glob($self->{'filenamePrefix'}.'.*')) {
		$f =~ /.*\.([^\.]*)$/;
		my $lang = $1;
		my $fd;
		if (not open($fd, "<", $f)) {
			$self->LOG_ERROR("Cannot open file: $f: $!");
			return 0;
		}
		while (<$fd>) {
			chomp;
			push(@{$self->{'_words'}->{$_}}, $lang);
		}
		close($fd);
	}

	return 1;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetFormCount();
	my @lang;
	my %score;
	my $words =  0;
	for (my $i = 0; $i < $nWords; $i++) {
		my $flags = $tr->GetFlags($i);
		my $word = $tr->GetForm($i);
		utf8::decode($word);
		if ($flags & $HectorRobot::TextResource::TOKEN_PARAGRAPH_START and $i > 0 and $self->{'paragraphLevel'}) {
#use Data::Dumper;
#print Dumper(\%score);
			my $max = -1;
			my $lang = '?';
			foreach my $k (keys %score) {
				if ($score{$k} > $max) {
					$max = $score{$k};
					$lang = $k;
				}
			}
			push(@lang, $lang);
		}
#print "$word\n";
		if (exists $self->{'_words'}->{lc($word)}) {
			foreach my $lang (@{$self->{'_words'}->{lc($word)}}) {
				$score{$lang}++;
			}
		}
		$words++;
	}
	if ($words > 0) {
		my $max = -1;
		my $lang = '?';
		foreach my $k (keys %score) {
			if ($score{$k} > $max) {
				$max = $score{$k};
				$lang = $k;
			}
		}
		push(@lang, $lang);
	}

	$tr->SetLanguage(join(" ", @lang));
	
	$self->{'items'}++;
	return $resource;
}

1;
