# PreFetch.pm, simple, perl
# Pre-process PageResource before passing to the Fetch module, test that
# WebSitePath is not locked, lock it and propagate IP address from WSR to WR.
# If WR does not pass the test, it is discarded.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# 
# Status:
# not changed (on error, PageResources are discarded)

package PreFetch;

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

	return $resource if ($resource->GetTypeString() ne 'PageResource');
	my $sr = HectorRobot::ResourceToSiteResource($resource->GetAttachedResource());
	if ($sr->GetTypeString() ne 'SiteResource') {
		$self->LOG_ERROR($sr, "Invalid attached type: ".$sr->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}

	my $ip = $sr->GetIpAddr();
	$resource->SetIpAddr($ip);
	$self->{'items'}++;
	return $resource;
}

1;
