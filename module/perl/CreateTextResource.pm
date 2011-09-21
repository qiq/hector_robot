# CreateTextResource.pm, input, perl
# Create TextResources
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# maxItems		r/o	Max resources to create
# 
# Status:
# 0 (default of TR)

package CreateTextResource;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('INPUT', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'maxItems'} = 100;
	$self->{'mark'} = 0;

	$self->{'_finished'} = 0;

	bless($self, $class);
	return $self;
}

sub ProcessInput() {
	my ($self, $resource) = @_;

	if (defined $resource) {
		$self->LOG_ERROR($resource, "Resource is already defined.");
		return undef;
	}
	if ($self->{'items'} >= $self->{'maxItems'}) {
		if (not $self->{'_finished'}) {
			$self->LOG_INFO("Finished, total TextResources created: ".$self->{'items'});
			$self->{'_finished'} = 1;
			if ($self->{'mark'}) {
				return &Hector::Resource::GetRegistry()->AcquireResource("MarkerResource");
			}
		}
		return undef;
	}
	$resource = &Hector::Resource::GetRegistry()->AcquireResource("TextResource");
	if (not defined $resource) {
		$self->LOG_ERROR("Cannot create resource type: TextResource");
		return undef;
	}
	$resource = HectorRobot::ResourceToTextResource($resource);
	$resource->SetId($self->{'_threadIndex'}*10000+$self->{'items'});
	$resource->SetText("Working example příliš");
	$resource->SetFlags(0, $HectorRobot::TOKEN_SENTENCE_START);
	$resource->SetForm(0, "Working");
	$resource->SetLemma(0, "work");
	$resource->SetPosTag(0, "V");
	$resource->SetHead(0, "1");
	$resource->SetDepRel(0, "test");
	for (my $i = 1; $i < 2; $i++) {
		$resource->SetFlags($i, $HectorRobot::TOKEN_NONE);
		$resource->SetForm($i, "example");
		$resource->SetLemma($i, "example");
		$resource->SetPosTag($i, "N");
		$resource->SetHead($i, "1");
		$resource->SetDepRel($i, "nounization");
	}
	$self->{'items'}++;
	return $resource;
}

1;
