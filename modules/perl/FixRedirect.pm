# FixRedirect.pm, simple, perl
# Copy Location header field of the redirect into WebResource's url.
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

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'items' => 0,
		'redirectStatus' => 1,
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
	
	if ($resource->getStatus() == $self->{'redirectStatus'}) {
		$resource->setUrl(HectorRobot::AbsolutizeUrl($resource->getUrl(), $resource->getHeaderValue("Location")));
		$self->{'items'}++;
	}
	return $resource;
}

1;
