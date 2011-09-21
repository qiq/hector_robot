# FixRedirect.pm, simple, perl
# Copy Location header field of the redirect into PageResource's url.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# 
# Status:

package FixRedirect;

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
	$self->{'redirectStatus'} = 1;

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
	
	if ($resource->GetStatus() == $self->{'redirectStatus'}) {
		my $location = HectorRobot::AbsolutizeUrl($resource->GetUrl(), $resource->GetHeaderValue("Location"));
		$resource->SetUrl($location);
		$self->LOG_DEBUG($resource, "New location: ".$location);
		$self->{'items'}++;
	}
	return $resource;
}

1;
