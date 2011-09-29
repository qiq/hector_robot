# FilterUnaccented.pm, simple, perl
# Only keep paragraphs that contain at least some percent of accented words.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# minAccentedRatio	r/o	n/a	Minumal ratio of accented words to keep the paragraph
# 
# Status:
# not changed

package FilterUnaccented;

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
	$self->{'minAccentedRatio'} = 0.1;

	bless($self, $class);
	return $self;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetFormCount();
	my $accented = 0;
	my $words = 0;
	my @deleted;
	for (my $i = 0; $i < $nWords; $i++) {
		my $flags = $tr->GetFlags($i);
		my $word = $tr->GetForm($i);
		utf8::decode($word);
		if ($flags & $HectorRobot::TextResource::TOKEN_PARAGRAPH_START and $i > 0) {
			if ($accented/$words <= $self->{'minAccentedRatio'}) {
				for (my $j = 0; $j < $words; $j++) {
					push(@deleted, 1);
				}
			} else {
				for (my $j = 0; $j < $words; $j++) {
					push(@deleted, 0);
				}
			}
			$words = 0;
		}
		$accented++ if (&HectorRobotCommon::is_accented($word));
		$words++;
	}

	$tr->DeleteWords(\@deleted);
	
	$self->{'items'}++;
	return $resource;
}

1;
