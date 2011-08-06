# RobotsParser.pm, simple, perl
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

package RobotsParser;

use warnings;
use strict;
use HectorRobot;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'items' => 0,
		'robotName' => 'hector',
		'wildcards' => 1,
		'TTL' => 86400,			# default: one day
		'negativeTTL' => 86400,		# default: one day
	};
	bless($self, $class);
	return $self;
}

sub DESTROY {
}

sub Init {
	my ($self, $params) = @_;

	# second stage?
	return 1 if (not defined $params);

	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}
	return 1;
}

sub GetType {
	my ($self) = @_;
	return $Hector::Module::SIMPLE;
}

sub GetProperty {
	my ($self, $name) = @_;
	if (exists $self->{$name}) {
		return $self->{$name};
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return undef;
	}
}

sub SetProperty {
	my ($self, $name, $value) = @_;
	if (exists $self->{$name}) {
		$self->{$name} = $value;
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return 0;
	}
	return 1;
}

sub ListProperties {
	my ($self) = @_;
	return [ grep { $_ !~ /^_/ } keys %{$self} ];
}

sub SaveCheckpoint {
	my ($self, $path, $id) = @_;
	$self->{'_object'}->log_info("SaveCheckpoint($path, $id)");
}

sub RestoreCheckpoint {
	my ($self, $path, $id) = @_;
	$self->{'_object'}->log_info("RestoreCheckpoint($path, $id)");
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
		$self->{'_object'}->log_error($resource->ToStringShort()." Invalid type: ".$resource->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $sr = &HectorRobot::ResourceToSiteResource($resource->GetAttachedResource());
	if ($sr->GetTypeString() ne 'SiteResource') {
		$self->{'_object'}->log_error($resource->ToStringShort()." No attaxhed WSR: ".$sr->GetTypeString());
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
		$self->{'_object'}->log_error($resource->ToStringShort()." Invalid status: ".$resource->GetHeaderValue("X-Status"));
		$resource->SetStatus(1);
		return $resource;
	}
	if ($status >= 100 and $status < 300) {
		# 1xx, 2xx: OK
		my $mime = $resource->GetHeaderValue("Content-Type");
		if ($mime eq '') {
			$self->{'_object'}->log_debug($resource->ToStringShort()." Missing robots.txt mime type (".$resource->GetUrlHost().")");
			$sr->SetRobots([], [], time()+$self->{'negativeTTL'});
			# status is 0
			return $resource;
		}
		if ($mime !~ /^text\/plain/) {
			$self->{'_object'}->log_debug($resource->ToStringShort()." Invalid robots.txt mime type: ".$mime." (".$resource->GetUrlHost().")");
			$sr->SetRobots([], [], time()+$self->{'negativeTTL'});
			# status is 0
			return $resource;
		}
		my $content = $resource->GetContent();
		if (length($content) > 100000) {
			$self->{'_object'}->log_debug("Robots.txt too long: ".length($content)." (".$resource->GetUrlHost().")");
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
			$self->{'_object'}->log_error($resource->ToStringShort()." Redirect with no location: ".$resource->GetUrl());
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

sub Start() {
	my ($self) = @_;
}

sub Stop() {
	my ($self) = @_;
}

sub Pause() {
	my ($self) = @_;
}

sub Resume() {
	my ($self) = @_;
}

1;