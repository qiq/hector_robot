# FilterRobots.pm, simple, perl
# Test whether current WR is not disabled by a rule in robots.txt file.
# Optionally supports wildcard matching rules.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# wildcards		r/w	Whether to consider robots rules as wildcards
# 
# Status:
# not changed (on error, PageResources are discarded)

package FilterRobots;

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
	$self->{'wildcards'} = 1;

	bless($self, $class);
	return $self;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'PageResource');
	my $sr = HectorRobot::ResourceToSiteResource($resource->GetAttachedResource());
	if ($sr->GetTypeString() ne 'SiteResource') {
		$self->LOG_ERROR($sr, "Invalid type: ".$sr->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}

	my $path = $resource->GetUrlPath();
	my $allowed = 0;
	my $au = $sr->GetAllowUrls();
	my $prefix;
	for (my $i = 0; $i < $au->size(); $i++) {
		$prefix = $au->get($i);
		if ($self->{'wildcards'}) {
			if ($path =~ /$prefix/) {
				$allowed = 1;
				last;
			}
		} else {
			my $l = length($prefix);
			if (length($path) >= $l and substr($path, 0, $l) eq $prefix) {
				$allowed = 1;
				last;
			}
		}
	}
$au = undef;
#	HectorRobot::DeleteVectorOfString($au);
	if (not $allowed) {
		my $disallowed = 0;
		my $du = $sr->GetDisallowUrls();
		for (my $i = 0; $i < $du->size(); $i++) {
			$prefix = $du->get($i);
			if ($self->{'wildcards'}) {
				if ($path =~ /$prefix/) {
					$disallowed = 1;
					last;
				}
			} else {
				my $l = length($prefix);
				if (length($path) >= $l and substr($path, 0, $l) eq $prefix) {
					$disallowed = 1;
					last;
				}
			}
		}
$du = undef;
#		HectorRobot::DeleteVectorOfString($du);
		if ($disallowed) {
			$self->LOG_DEBUG($resource, "Disallowed by robots.txt policy ($prefix): ".$resource->GetUrl());
			$resource->SetFlag($Hector::Resource::DELETED);
		}
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
