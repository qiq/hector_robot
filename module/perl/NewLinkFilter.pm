# SaveNewUrl.pm, simple, perl
# Check that we are first to save a PageResource, set status to 0, otherwise
# delete it (set DELETED flag).
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# 
# Status:
# set status to 0

package NewLinkFilter;

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

	bless($self, $class);
	return $self;
}

sub DESTROY {
}

sub GetType {
	my ($self) = @_;
	return $Hector::Module::SIMPLE;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	if ($resource->GetTypeString() ne 'PageResource') {
		$self->LOG_ERROR($resource, "Invalid type: ".$resource->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $sr = HectorRobot::ResourceToSiteResource($resource->GetAttachedResource());
	if ($sr->GetTypeString() ne 'SiteResource') {
		$self->LOG_ERROR($sr, "Invalid type: ".$sr->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	
	$self->{'items'}++;

	my $currentTime = time();
	my $ok = $sr->PathNewLinkReady($resource->GetUrlPath(), $currentTime);
	if ($ok) {
		$resource->SetStatus(0);
	} else {
		$resource->SetFlag($Hector::Resource::DELETED);
	}
	return $resource;
}

1;
