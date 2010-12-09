# items: number of resources created
# redirect resources are marked with status == 1

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

sub getType {
	my ($self) = @_;
	return $Hector::Module::SIMPLE;
}

sub getValueSync {
	my ($self, $name) = @_;
	if (exists $self->{$name}) {
		return $self->{$name};
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return undef;
	}
}

sub setValueSync {
	my ($self, $name, $value) = @_;
	if (exists $self->{$name}) {
		$self->{$name} = $value;
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return 0;
	}
	return 1;
}

sub listNamesSync {
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

	if ($resource->getTypeStr() ne 'WebResource') {
		$self->{'_object'}->log_error($resource->toStringShort()." Invalid type: ".$resource->getTypeStr());
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $wsr = HectorRobot::ResourceToWebSiteResource($resource->getAttachedResource());
	if ($wsr->getTypeStr() ne 'WebSiteResource') {
		$self->{'_object'}->log_error($wsr->toStringShort()." Invalid type: ".$wsr->getTypeStr());
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
	
	$self->{'items'}++;

	my $currentTime = time();
	# check result of the Fetcher module
	if ($resource->getStatus() == 0) {
		# get status code
		my $status = $resource->getHeaderValue("X-Status");
		if (not defined $status or not $status =~ s/^HTTP([^ ]*) ([0-9]+).*/$2/) {
			$self->{'_object'}->log_error($resource->toStringShort()." Invalid status: ".$resource->getHeaderValue("X-Status"));
			my $disabled = $wsr->PathUpdateError($resource->getUrlPath(), $currentTime, $self->{'maxErrors'});
			$resource->setFlag($Hector::Resource::DELETED) if ($disabled);
		} else {
			$self->{'_object'}->log_debug($resource->toStringShort().' Status: '.$status.' '.$resource->getUrl());
			if ($status >= 100 and $status < 300) {
				# 1xx, 2xx: OK
				my $content = $resource->getContent();
				my $size = length($content);
				my $cksum = 0;
				$cksum = CountCksum($content, $size) if ($size == $wsp->getSize());
				$wsr->PathUpdateOK($resource->getUrlPath(), $currentTime, $size, $cksum);
				$resource->setStatus(0);
			} elsif ($status >= 300 and $status < 400) {
				# 3xx: redirect
				my $location = $resource->getHeaderValue("Location");
				if (not defined $location) {
					$self->{'_object'}->log_error($resource->toStringShort()." Redirect with no location: ".$resource->getUrl());
					my $disabled = $wsr->PathUpdateError($resource->getUrlPath(), $currentTime, $self->{'maxErrors'});
					$resource->setFlag($Hector::Resource::DELETED) if ($disabled);
				} else {
					$resource->setUrl($location);
					my $redirects = $resource->getRedirectCount();
					if ($redirects > $self->{'maxRedirects'}) {
						$self->{'_object'}->log_error($resource->toStringShort()." Too many redirects: ".$resource->getUrl());
						my $disabled = $wsr->PathUpdateError($resource->getUrlPath(), $currentTime, $self->{'maxErrors'});
						$resource->setFlag($Hector::Resource::DELETED) if ($disabled);
					} else {
						# correct redirects
						$resource->setRedirectCount($redirects+1);
						$wsr->PathUpdateOK($resource->getUrlPath(), $currentTime, $status == 301);
						$resource->setStatus(1);	# mark resource, so that we can filter redirection later
					}
				}
			} else {
				# 4xx, 5xx: client or server error
				my $disabled = $wsr->PathUpdateError($resource->getUrlPath(), $currentTime, $self->{'maxErrors'});
				$resource->setFlag($Hector::Resource::DELETED) if ($disabled);
			}
		}
	} else {
		# error fetching object
		$self->{'_object'}->log_error($resource->toStringShort()." Cannot fetch object: ".$resource->getUrl());
		my $disabled = $wsr->PathUpdateError($resource->getUrlPath(), $currentTime, $self->{'maxErrors'});
		$resource->setFlag($Hector::Resource::DELETED) if ($disabled);
	}
	return $resource;
}

1;
