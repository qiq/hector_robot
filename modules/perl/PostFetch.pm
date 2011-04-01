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

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'items' => 0,
		'maxErrors' => 5,	# number of consecutive errors to cause resource to be disabled
		'maxRedirects' => 5,	# number of consecutive redirects to accept
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

sub GetValue {
	my ($self, $name) = @_;
	if (exists $self->{$name}) {
		return $self->{$name};
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return undef;
	}
}

sub SetValue {
	my ($self, $name, $value) = @_;
	if (exists $self->{$name}) {
		$self->{$name} = $value;
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return 0;
	}
	return 1;
}

sub ListNames {
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

sub ProcessSimple() {
	my ($self, $resource) = @_;

	if ($resource->GetTypeString() ne 'WebResource') {
		$self->{'_object'}->log_error($resource->ToStringShort()." Invalid type: ".$resource->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $wsr = HectorRobot::ResourceToWebSiteResource($resource->GetAttachedResource());
	if ($wsr->GetTypeString() ne 'WebSiteResource') {
		$self->{'_object'}->log_error($wsr->ToStringShort()." Invalid type: ".$wsr->GetTypeString());
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
			$self->{'_object'}->log_error($resource->ToStringShort()." Invalid status: ".$resource->GetHeaderValue("X-Status"));
			$error = 1;
		} else {
			$self->{'_object'}->log_debug($resource->ToStringShort().' Status: '.$status.' '.$resource->GetUrl());
			if ($status >= 100 and $status < 300) {
				# 1xx, 2xx: OK
				my $content = $resource->GetContent();
				my $size = length($content);
				my $cksum = 0;
				$cksum = CountCksum($content, $size) if ($size == $wsr->GetSize());
				$wsr->PathUpdateOK($resource->GetUrlPath(), $currentTime, $size, $cksum);
				$resource->SetStatus(0);
			} elsif ($status >= 300 and $status < 400) {
				# 3xx: redirect
				if (not defined $resource->GetHeaderValue("Location")) {
					$self->{'_object'}->log_error($resource->ToStringShort()." Redirect with no location: ".$resource->GetUrl());
					$error = 1;
				} else {
					my $redirects = $resource->GetRedirectCount();
					if ($redirects > $self->{'maxRedirects'}) {
						$self->{'_object'}->log_error($resource->ToStringShort()." Too many redirects: ".$resource->GetUrl());
						$error = 1;
					} else {
						# correct redirects
						$resource->SetRedirectCount($redirects+1);
						$wsr->PathUpdateRedirect($resource->GetUrlPath(), $currentTime, $status == 301);
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
		$self->{'_object'}->log_error($resource->ToStringShort()." Cannot fetch object: ".$resource->GetUrl());
		$resource->SetStatus(0);
		$error = 1;
	} elsif ($rs == 2) {
		# error fetching object (permanent error)
		$self->{'_object'}->log_error($resource->ToStringShort()." Invalid object: ".$resource->GetUrl());
		my $ok = $wsr->PathUpdateError($resource->GetUrlPath(), $currentTime, 1);
		$resource->SetFlag($Hector::Resource::DELETED) if (not $ok);
		$resource->SetStatus(0);
	}

	if ($error == 1) {
		my $ok = $wsr->PathUpdateError($resource->GetUrlPath(), $currentTime, $self->{'maxErrors'});
		$resource->SetFlag($Hector::Resource::DELETED) if (not $ok);
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
