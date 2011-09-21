# PostFetch.pm, simple, perl
# Post-process result of Fetch module, unlock WSR and update WebSitePath
# status. Redirect resources are marked with status == 1.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# maxErrors		r/w	number of consecutive errors to cause resource to be disabled (default is 5)
# maxRedirects		r/w	number of consecutive redirects to accept (default is 5)
# 
# Status:
# 0	OK
# 1	redirect

package PostFetch;

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
	$self->{'maxErrors'} = 5;	# number of consecutive errors to cause resource to be disabled
	$self->{'maxRedirects'} = 5;	# number of consecutive redirects to accept

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

	return $resource if ($resource->GetTypeString() ne 'PageResource');
	my $sr = HectorRobot::ResourceToSiteResource($resource->GetAttachedResource());
	if ($sr->GetTypeString() ne 'SiteResource') {
		$self->LOG_ERROR($sr, "Invalid attached type: ".$sr->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	
	$self->{'items'}++;

	my $currentTime = time();
	# check result of the Fetcher module
	my $error = 0;
	my $rs = $resource->GetStatus();
	if ($rs == 0) {
		# get "real" status code
		my $status = $resource->GetHeaderValue("X-Status");
		if (not defined $status or not $status =~ s/^HTTP([^ ]*) ([0-9]+).*/$2/) {
			$self->LOG_ERROR($resource, "Invalid status: ".$resource->GetHeaderValue("X-Status"));
			$error = 1;
		} else {
			$self->LOG_DEBUG($resource, 'Status: '.$status.' '.$resource->GetUrl());
			if ($status >= 100 and $status < 300) {
				# 1xx, 2xx: OK
				$resource->SetStatus(0);
			} elsif ($status >= 300 and $status < 400) {
				# 3xx: redirect
				if (not defined $resource->GetHeaderValue("Location")) {
					$self->LOG_ERROR($resource, "Redirect with no location: ".$resource->GetUrl());
					$error = 1;
				} else {
					my $redirects = $resource->GetRedirectCount();
					if ($redirects > $self->{'maxRedirects'}) {
						$self->LOG_ERROR($resource, "Too many redirects: ".$resource->GetUrl());
						$error = 1;
					} else {
						# correct redirects
						$resource->SetRedirectCount($redirects+1);
						$sr->PathUpdateRedirect($resource->GetUrlPath(), $currentTime, $status == 301);
						$resource->SetStatus(1);	# mark resource, so that we can filter redirection later
					}
				}
			} else {
				# 4xx, 5xx: client or server error
				$error = 1;
			}
		}
	} elsif ($rs == 1) {
		# error fetching object (temoporary error)
		$self->LOG_ERROR($resource, "Cannot fetch object: ".$resource->GetUrl());
		$resource->SetStatus(0);
		$error = 1;
	} elsif ($rs == 2) {
		# error fetching object (permanent error)
		$self->LOG_ERROR($resource, "Invalid object: ".$resource->GetUrl());
		my $ok = $sr->PathUpdateError($resource->GetUrlPath(), $currentTime, 1);
		$resource->SetFlag($Hector::Resource::DELETED) if (not $ok);
		$resource->SetStatus(0);
	}

	if ($error == 1) {
		my $ok = $sr->PathUpdateError($resource->GetUrlPath(), $currentTime, $self->{'maxErrors'});
		$resource->SetFlag($Hector::Resource::DELETED) if (not $ok);
	}

	return $resource;
}

1;
