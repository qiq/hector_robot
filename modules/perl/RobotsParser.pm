# items: number of resources created

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

	if ($resource->getTypeStr() ne 'WebResource') {
		$self->{'_object'}->log_error($resource->toStringShort()." Invalid type: ".$resource->getTypeStr());
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $wsr = HectorRobot::ResourceToWebSiteResource($resource->getAttachedResource());
	if ($wsr->getTypeStr() ne 'WebSiteResource') {
		$self->{'_object'}->log_error($resource->toStringShort()." No attaxhed WSR: ".$wsr->getTypeStr());
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
		
	my $mime = $resource->getHeaderValue("Content-Type");
	if ($mime eq '') {
		$self->{'_object'}->log_debug($resource->toStringShort()." Missing robots.txt mime type (".$resource->getUrlHost().")");
		$wsr->setRobots([], [], time()+$self->{'negativeTTL'});
		return $resource;
	}
	if ($mime !~ /^text\/plain/) {
		$self->{'_object'}->log_debug($resource->toStringShort()." Invalid robots.txt mime type: ".$mime." (".$resource->getUrlHost().")");
		$wsr->setRobots([], [], time()+$self->{'negativeTTL'});
		return $resource;
	}
	my $content = $resource->getContent();
	if (length($content) > 10000) {
		$self->{'_object'}->log_debug("Robots.txt too long: ".length($content)." (".$resource->getUrlHost().")");
		$wsr->setRobots([], [], time()+$self->{'negativeTTL'});
		return $resource;
	}
	my ($allow, $disallow) = $self->ParseRobots($content);
	$wsr->setRobots($allow, $disallow, time()+$self->{'TTL'});
	$self->{'items'}++;
	return $resource;
}

1;