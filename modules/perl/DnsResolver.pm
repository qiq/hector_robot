# items: number of resources created

package DnsResolver;

use warnings;
use strict;
use HectorRobot;
use Net::DNS;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'_resolver' => undef,
		'items' => 0,
		'forwardServer' => undef,
		'forwardPort' => undef,
	};
	bless($self, $class);
	return $self;
}

sub DESTROY {
}

sub Init {
	my ($self, $params) = @_;
	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}
	my %args = (
		'search' => '',
	);
	$args{'nameservers'} = [ $self->{'forwardServer'} ] if (defined $self->{'forwardServer'});
	$args{'forwardPort'} = $self->{'forwardPort'} if (defined $self->{'forwardPort'});
	$self->{'_resolver'} = Net::DNS::Resolver->new(%args),

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

	my $host = $resource->getUrlHost();
	if (not defined $host) {
		$self->{'_object'}->log_error("Resource does not contain URL host: ".$resource->getId());
		return $resource;
	}
	if ($host =~ /^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$/ and $1 < 256 and $2 < 256 and $3 < 256 and $4 < 256) {
		my $addr = Hector::str2Ip4Addr_w($host);
		$resource->setIp4Addr($addr);
		Hector::ip4AddrDelete_w($addr);
	} elsif ($host =~ /^\[[0-9A-Fa-f:]+\]$/) {
		my $addr = Hector::str2Ip6Addr($host);
		$resource->setIp6Addr($addr);
		Hector::ip6AddrDelete_w($addr);
	} else {
		my $answer = $self->{'_resolver'}->search($host, 'A');
		if (defined $answer) {
			foreach my $rr ($answer->answer) {
				next unless $rr->type eq "A";
				$self->{'_object'}->log_debug("$host: ".$rr->address.' ('.$rr->ttl.')');
				my $addr = Hector::str2Ip4Addr_w($rr->address);
				$resource->setIp4Addr($addr);
				Hector::ip4AddrDelete_w($addr);
				$resource->setIpAddrExpire(time() + $rr->ttl);
				$resource->setStatus(0);
				last;
			}
		} else {
			$self->{'_object'}->log_debug("Query failed ($host): ".$self->{'_resolver'}->errorstring);
			$resource->setStatus(1);
		}
	}

	$self->{'items'}++;
	return $resource;
}

1;
