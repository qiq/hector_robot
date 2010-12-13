# DnsResolver.pm, simple, perl
# Translate DNS name to IP address, supports WebResource and WebSiteResource.
# 
# Dependencies: Net::DNS
#
# Parameters:
# items		r/o	Total items processed
# forwardServer	init	DNS server to use
# forwardPort	init	Port number of the DNS server
# negativeTTL	r/w	Number of seconds to keep info about DNS failure/NXdomain

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
		'negativeTTL' => 86400,
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
	my %args = (
		'search' => '',
	);
	$args{'nameservers'} = [ $self->{'forwardServer'} ] if (defined $self->{'forwardServer'});
	$args{'port'} = $self->{'forwardPort'} if (defined $self->{'forwardPort'});
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

	if ($resource->getTypeStr() ne 'WebSiteResource' and $resource->getTypeStr() ne 'WebResource') {
		$self->{'_object'}->log_error($resource->toStringShort()." Invalid type: ".$resource->getTypeStr());
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $host = $resource->getUrlHost();
	if (not defined $host) {
		$self->{'_object'}->log_error($resource->toStringShort()." Resource does not contain URL host");
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
	if ($host =~ /^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$/ and $1 < 256 and $2 < 256 and $3 < 256 and $4 < 256) {
		my $ip = Hector::IpAddr->new();
		$ip->ParseIp4Addr($host);
		$resource->setIpAddr($ip);
	} elsif ($host =~ /^\[[0-9A-Fa-f:]+\]$/) {
		my $ip = Hector::IpAddr->new();
		$ip->ParseIp6Addr($host);
		$resource->setIpAddr($ip);
	} else {
		my $answer = $self->{'_resolver'}->search($host, 'A');
		if (defined $answer) {
			foreach my $rr ($answer->answer) {
				next unless $rr->type eq "A";
				$self->{'_object'}->log_debug($resource->toStringShort()." $host: ".$rr->address.' ('.$rr->ttl.')');
				my $ip = Hector::IpAddr->new();
				$ip->ParseIp4Addr($rr->address);
				$resource->setIpAddr($ip);
				$resource->setIpAddrExpire(time() + $rr->ttl) if ($resource->getTypeStr() eq 'WebSiteResource');
				$resource->setStatus(0);
				last;
			}
		} else {
			$self->{'_object'}->log_debug($resource->toStringShort()." Query failed ($host): ".$self->{'_resolver'}->errorstring);
			my $ip = Hector::IpAddr->new();
			$resource->setIpAddr($ip);
			$resource->setIpAddrExpire(time() + $self->{'negativeTTL'}) if ($resource->getTypeStr() eq 'WebSiteResource');
			$resource->setStatus(1);
		}
	}

	$self->{'items'}++;
	return $resource;
}

1;
