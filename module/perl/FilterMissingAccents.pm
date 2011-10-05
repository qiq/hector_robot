# FilterMissingAccents.pm, simple, perl
# Delete docs/paragraphs that does not contain (too many) accented words and
# contain (at least some) invalid unaccented words.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# filename		r/o	n/a	File containing unaccented words, one
#					word per line.
# paragraphLevel	r/w	1	Work on paragraph level (otherwise whole document)
# minAccentedRatio	r/w	0.1	Only judge paragraphs (docs) containing more
#					than value accented words.
# maxInvalidRatio	r/w	0.1	Keep paragraphs (docs) containing less
#					than value of invalid (unaccented)
#					words.
# 
# Status:
# not changed

package FilterMissingAccents;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);
use HectorRobotCommon;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'filename'} = "";
	$self->{'paragraphLevel'} = 1;
	$self->{'minAccentedRatio'} = 0.1;
	$self->{'maxInvalidRatio'} = 0.1;
	$self->{'reversed'} = 0;

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

	if ($self->{'filename'} eq "") {
		$self->LOG_ERROR("filename not defined");
		return 0;
	}
	my $fd;
	if (!open($fd, "<".$self->{'filename'})) {
		$self->LOG_ERROR("Cannot open file: ".$self->{'filename'}.": $!");
		return 0;
	}
	while (<$fd>) {
		chomp;
		$self->{'_words'}->{$_} = 1;
	}
	close($fd);

	return 1;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetFormCount();
	my $totalDeleted = 0;
	my $accented = 0;
	my $invalid = 0;
	my $words = 0;
	my @deleted;
	for (my $i = 0; $i < $nWords; $i++) {
		my $flags = $tr->GetFlags($i);
		my $word = $tr->GetForm($i);
		utf8::decode($word);
		if ($flags & $HectorRobot::TextResource::TOKEN_PARAGRAPH_START and $i > 0 and $self->{'paragraphLevel'}) {
#			my $to_delete = ($accented/$words > $self->{'minAccentedRatio'} and $invalid/$words > $self->{'maxInvalidRatio'});
			my $to_delete = ($invalid >= 2 and $accented/$words < $invalid/$words);
			$to_delete ^= 1 if ($self->{'reversed'});
			if ($to_delete) {
				for (my $j = 0; $j < $words; $j++) {
					push(@deleted, 1);
				}
				$totalDeleted += $words;
			} else {
				for (my $j = 0; $j < $words; $j++) {
					push(@deleted, 0);
				}
			}
			$words = 0;
			$invalid = 0;
			$accented = 0;
		}
		$accented++ if (&HectorRobotCommon::is_accented($word));
		$invalid++ if (exists $self->{'_words'}->{$word});
		$words++;
	}
	if ($words > 0) {
#		my $to_delete = ($accented/$words > $self->{'minAccentedRatio'} and $invalid/$words > $self->{'maxInvalidRatio'});
		my $to_delete = ($invalid >= 2 and $accented/$words < $invalid/$words);
		$to_delete ^= 1 if ($self->{'reversed'});
		if ($to_delete) {
			for (my $j = 0; $j < $words; $j++) {
				push(@deleted, 1);
			}
			$totalDeleted += $words;
		} else {
			for (my $j = 0; $j < $words; $j++) {
				push(@deleted, 0);
			}
		}
	}

	if ($totalDeleted < $nWords) {
		$tr->DeleteWords(\@deleted);
	} else {
		$self->LOG_ERROR($resource, "Document deleted: ".$resource->GetTextId());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
