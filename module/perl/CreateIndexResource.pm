# CreateIndexResource.pm, input, perl
# Create IndexResources
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# maxItems		r/w	Max resources to create
# mark			r/w	Produce mark resource at the end.
# 
# Status:
# 0 (default of TR)

package CreateIndexResource;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new($object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'maxItems'} = 100;
	$self->{'mark'} = 0;

	$self->{'_finished'} = 0;

	bless($self, $class);
	return $self;
}

sub DESTROY {
}


sub GetType {
	my ($self) = @_;
	return $Hector::Module::INPUT;
}

sub ProcessInput() {
	my ($self, $resource) = @_;

	if (defined $resource) {
		$self->LOG_ERROR($resource, "Resource is already defined.");
		return undef;
	}
	if ($self->{'items'} >= $self->{'maxItems'}) {
		if (not $self->{'_finished'}) {
			$self->LOG_INFO("Finished, total IndexResources created: ".$self->{'items'});
			$self->{'_finished'} = 1;
			if ($self->{'mark'}) {
				return &Hector::Resource::GetRegistry()->AcquireResource("MarkerResource");
			}
		}
		return undef;
	}
	$resource = &Hector::Resource::GetRegistry()->AcquireResource("IndexResource");
	if (not defined $resource) {
		$self->LOG_ERROR("Cannot create resource type: IndexResource");
		return undef;
	}
	$resource = HectorRobot::ResourceToIndexResource($resource);
	$resource->SetId($self->{'_threadIndex'}*10000+$self->{'items'});
	$resource->SetSiteMD5(1);
	$resource->SetPathMD5(2);
	$resource->SetLastModified(3);
	$resource->SetModificationHistory(4);
	$resource->SetIndexStatus(5);
	$self->{'items'}++;
	return $resource;
}

1;
