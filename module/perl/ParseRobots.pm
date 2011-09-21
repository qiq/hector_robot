# ParseRobots.pm, simple, perl
# Parses WR content as robots.txt file and fills robots info in attached WSR.
# In case of errors in parsing, WSR Disallow/Allow lists are cleared and
# refresh time is set to negativeTTL. Otherwise lists are filled and refresh
# time is set to TTL.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# robotName		r/w	Robot name to match in robots.txt file (default is hector)
# wildcards		r/w	Whether to consider robots rules as wildcards
# TTL			r/w	Time to keep result after successful processing (default is one day)
# negativeTTL		r/w	Time to keep an info about error in processing
#				(default is one day)
# 
# Status:
# - input:
# 0: OK
# 1: error (by Fetcher)
# - output:
# 0: OK, robots fields filled or cleared (e.g. in case of bad mime type).
# 1: client/server error, e.g. 404 not found
# 2: redirect (temporary or permanent)

package ParseRobots;

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
	$self->{'robotName'} = 'hector';
	$self->{'wildcards'} = 1;
	$self->{'TTL'} = 86400;			# default: one day
	$self->{'negativeTTL'} = 86400;		# default: one day

	bless($self, $class);
	return $self;
}

sub ConvertPath() {
	my ($self, $path) = @_;
	chomp($path);
	return undef if ($path eq '');
	if ($self->{'wildcards'}) {
		$path =~ s/([^[:alnum:]%\*\/\$])/\\$1/g;
		$path =~ s/\*/.*/g;
		$path = '^'.$path;
	}
	return $path;
}

sub ParseRobots() {
	my ($self, $robots) = @_;
	my (@allow, @disallow);
	my $myRobot = 0;
	my $ignore = 1;

	foreach my $l (split(/[\n\r]+/, $robots)) {
		next if ($l =~ /^\s*#/);
		$l =~ s/\s*#.*//;
		$l =~ s/\s+$//;
		if ($l eq '') {
			$ignore = 1;
		} elsif ($l =~ /^\s*User-Agent\s*:\s*(.*)/i) {
			if (index($1, $self->{'robotName'}) >= 0) {
				$ignore = 0;
				$myRobot = 1;
				@allow = ();
				@disallow = ();
			} elsif ($1 eq "*" and not $myRobot) {
				$ignore = 0;
			}
		} elsif (not $ignore) {
			if ($l =~ /^\s*Disallow\s*:\s*(.*)/i) {
				my $path = $self->ConvertPath($1);
				push(@disallow, $path) if (defined $path);
			} elsif ($l =~ /^\s*Allow\s*:(.*)/i) {
				my $path = $self->ConvertPath($1);
				push(@allow, $path) if (defined $path);
			}
		}

	}
	return (\@allow, \@disallow);
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	if ($resource->GetTypeString() ne 'PageResource') {
		$self->LOG_ERROR($resource, "Invalid type: ".$resource->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $sr = &HectorRobot::ResourceToSiteResource($resource->GetAttachedResource());
	if ($sr->GetTypeString() ne 'SiteResource') {
		$self->LOG_ERROR($resource, "No attaxhed WSR: ".$sr->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}

	# status == 1 or 2: error
	my $s = $resource->GetStatus();
	if ($s != 0) {
		$resource->SetStatus(1) if ($s != 1);
		return $resource;
	}

	my $status = $resource->GetHeaderValue("X-Status");
	if (not defined $status or not $status =~ s/^HTTP([^ ]*) ([0-9]+).*/$2/) {
		$self->LOG_ERROR($resource, "Invalid status: ".$resource->GetHeaderValue("X-Status"));
		$resource->SetStatus(1);
		return $resource;
	}
	if ($status >= 100 and $status < 300) {
		# 1xx, 2xx: OK
		my $mime = $resource->GetHeaderValue("Content-Type");
		if ($mime eq '') {
			$self->LOG_DEBUG($resource, "Missing robots.txt mime type (".$resource->GetUrlHost().")");
			$sr->SetRobots([], [], time()+$self->{'negativeTTL'});
			# status is 0
			return $resource;
		}
		if ($mime !~ /^text\/plain/) {
			$self->LOG_DEBUG($resource, "Invalid robots.txt mime type: ".$mime." (".$resource->GetUrlHost().")");
			$sr->SetRobots([], [], time()+$self->{'negativeTTL'});
			# status is 0
			return $resource;
		}
		my $content = $resource->GetContent();
		if (length($content) > 100000) {
			$self->LOG_DEBUG("Robots.txt too long: ".length($content)." (".$resource->GetUrlHost().")");
			$sr->SetRobots([], [], time()+$self->{'negativeTTL'});
			# status is 0
			return $resource;
		}
		my ($allow, $disallow) = $self->ParseRobots($content);
		$sr->SetRobots($allow, $disallow, time()+$self->{'TTL'});
		# status is 0
		$self->{'items'}++;
	} elsif ($status >= 300 and $status < 400) {
		# 3xx: redirect
		my $location = $resource->GetHeaderValue("Location");
		if (not defined $location) {
			$self->LOG_ERROR($resource, "Redirect with no location: ".$resource->GetUrl());
			$sr->SetRobots([], [], time()+$self->{'negativeTTL'});
			$resource->SetStatus(1);
			return $resource;
		} else {
			my $pr = &HectorRobot::ResourceToPageResource($resource);
			$pr->SetUrl(&HectorRobot::AbsolutizeUrl($resource->GetUrl(), $location));
			$resource->SetStatus(2);
			$self->{'items'}++;
		}
	} else {
		# 4xx, 5xx: client or server error
		$sr->SetRobots([], [], time()+$self->{'negativeTTL'});
		$resource->SetStatus(1);
	}
	return $resource;
}

1;
