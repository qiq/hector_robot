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

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'items' => 0,
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

sub ProcessSimple() {
	my ($self, $resource) = @_;

	if ($resource->GetTypeString() ne 'PageResource') {
		$self->{'_object'}->log_error($resource->ToStringShort()." Invalid type: ".$resource->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $sr = HectorRobot::ResourceToSiteResource($resource->GetAttachedResource());
	if ($sr->GetTypeString() ne 'SiteResource') {
		$self->{'_object'}->log_error($sr->ToStringShort()." Invalid type: ".$sr->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}

	my $currentTime = time();
	# check that path is ready to be fetched (not disabled, etc) and lock it
	my $err = $sr->PathReadyToFetch($resource->GetUrlPath(), $currentTime, $resource->GetScheduled());
	if ($err == 0) {
		my $ip = $sr->GetIpAddr();
		$resource->SetIpAddr($ip);
		$self->{'items'}++;
	} else {
		if ($err == 1) {
			$self->{'_object'}->log_debug($resource->ToStringShort()." Disabled (status): ".$resource->GetUrl());
		} elsif ($err == 2) {
			$self->{'_object'}->log_debug($resource->ToStringShort()." Disabled (updated recently): ".$resource->GetUrl());
		} elsif ($err == 3) {
			$self->{'_object'}->log_debug($resource->ToStringShort()." Disabled (currently updating): ".$resource->GetUrl());
		} else {
			$self->{'_object'}->log_debug($resource->ToStringShort()." Disabled: ".$resource->GetUrl());
		}
		$resource->SetFlag($Hector::Resource::DELETED);
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
