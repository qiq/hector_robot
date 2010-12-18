# PreFetch.pm, simple, perl
# Pre-process WebResource before passing to the Fetch module, test that
# WebSitePath is not locked, lock it and propagate IP address from WSR to WR.
# If WR does not pass the test, it is discarded.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# 
# Status:
# not changed (on error, WebResources are discarded)

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

	# check that path is ready to be fetched (not disabled, etc) and lock it
	if (not $wsr->PathReadyToFetch($resource->getUrlPath(), $resource->getScheduled())) {
		$self->{'_object'}->log_debug($resource->toStringShort()." Disabled");
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $ip = $wsr->getIpAddr();
	$resource->setIpAddr($ip);
	
	$self->{'items'}++;
	return $resource;
}

1;
