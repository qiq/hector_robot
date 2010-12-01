# items: number of resources created

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
		$resource->setFlag($Resource::DELETED);
		return $resource;
	}
	my $wsr = HectorRobot::ResourceToWebSiteResource($resource->getAttachedResource());
	if ($wsr->getTypeStr() ne 'WebSiteResource') {
		$self->{'_object'}->log_error($wsr->toStringShort()." Invalid type: ".$wsr->getTypeStr());
		$resource->setFlag($Resource::DELETED);
		return $resource;
	}

	my $path = $resource->getUrlPath();
	my $query = $resource->getUrlQuery();
	$path .= '?'.$query if ($query ne '');
	my $allowed = 0;
	my $au = $wsr->getAllowUrls();
	for (my $i = 0; $i < $au->size(); $i++) {
		my $prefix= $au->get($i);
		if ($self->{'wildcards'}) {
			if ($path =~ /$prefix/) {
				$allowed = 1;
				last;
			}
		} else {
			if (substr($path, length($prefix)) == $prefix) {
				$allowed = 1;
				last;
			}
		}
	}
	if (not $allowed) {
		my $disallowed = 0;
		my $du = $wsr->getDisallowUrls();
		for (my $i = 0; $i < $du->size(); $i++) {
			my $prefix= $du->get($i);
			if ($self->{'wildcards'}) {
				if ($path =~ /$prefix/) {
					$disallowed = 1;
					last;
				}
			} else {
				if (substr($path, length($prefix)) == $prefix) {
					$disallowed = 1;
					last;
				}
			}
		}
		if ($disallowed) {
			$self->{'_object'}->log_debug($resource->toStringShort()." Disallowed by robots.txt policy: ".$resource->getUrl());
			$resource->setFlag($Resource::DELETED);
		}
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
