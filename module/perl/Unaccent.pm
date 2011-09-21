# Unaccents.pm, simple, perl
# Remove all accents from words.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# 
# Status:
# not changed

package DetectNoAccents;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;

	bless($self, $class);
	return $self;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetWordCount();
	if ($nWords > 0) {
		for (my $i = 0; $i < $nWords; $i++) {
			$tr->SetForm($i, &unaccent($tr->GetForm($i)));
		}
	} else {
		$tr->SetText(&unaccent($tr->GetText()));
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
