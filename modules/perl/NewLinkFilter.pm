# SaveNewUrl.pm, simple, perl
# Check that we are first to save a WebResource, set status to 0, otherwise
# delete it (set DELETED flag).
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# 
# Status:
# set status to 0

package NewLinkFilter;

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
	my $ok = $wsr->PathNewLinkReady($resource->GetUrlPath(), $currentTime);
	if ($ok) {
		$resource->SetStatus(0);
	} else {
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
