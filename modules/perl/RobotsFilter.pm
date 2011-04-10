# RobotsFilter.pm, simple, perl
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
# not changed (on error, WebResources are discarded)

package RobotsFilter;

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
		'wildcards' => 1,
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

	my $path = $resource->GetUrlPath();
	my $allowed = 0;
	my $au = $wsr->GetAllowUrls();
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
	HectorRobot::DeleteVectorOfString($au);
	if (not $allowed) {
		my $disallowed = 0;
		my $du = $wsr->GetDisallowUrls();
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
		HectorRobot::DeleteVectorOfString($du);
		if ($disallowed) {
			$self->{'_object'}->log_debug($resource->ToStringShort()." Disallowed by robots.txt policy ($prefix): ".$resource->GetUrl());
			$resource->SetFlag($Hector::Resource::DELETED);
		}
	}
	
	$self->{'items'}++;
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
