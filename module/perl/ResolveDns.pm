# ResolveDns.pm, simple, perl
# Translate DNS name to IP address, supports PageResource and SiteResource.
# 
# Dependencies: Net::DNS
#
# Parameters:
# items		r/o	Total items processed
# forwardServer	init	DNS server to use
# forwardPort	init	Port number of the DNS server
# negativeTTL	r/w	Number of seconds to keep info about DNS failure/NXdomain

package ResolveDns;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);
use Net::DNS;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new($object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'forwardServer'} = undef;
	$self->{'forwardPort'} = undef;
	$self->{'negativeTTL'} = 86400;

	$self->{'_resolver'} = undef;

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

sub GetType {
	my ($self) = @_;
	return $Hector::Module::SIMPLE;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	if ($resource->GetTypeString() ne 'SiteResource' and $resource->GetTypeString() ne 'PageResource') {
		$self->LOG_ERROR($resource, "Invalid type: ".$resource->GetTypeString());
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $host = $resource->GetUrlHost();
	if (not defined $host) {
		$self->LOG_ERROR($resource, "Resource does not contain URL host");
		$resource->SetFlag($Hector::Resource::DELETED);
		return $resource;
	}
	if ($host =~ /^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$/ and $1 < 256 and $2 < 256 and $3 < 256 and $4 < 256) {
		my $ip = Hector::IpAddr->new();
		$ip->ParseIp4Addr($host);
		$resource->SetIpAddr($ip);
	} elsif ($host =~ /^\[[0-9A-Fa-f:]+\]$/) {
		my $ip = Hector::IpAddr->new();
		$ip->ParseIp6Addr($host);
		$resource->SetIpAddr($ip);
	} else {
		my $answer = $self->{'_resolver'}->search($host, 'A');
		if (defined $answer) {
			foreach my $rr ($answer->answer) {
				next unless $rr->type eq "A";
				$self->LOG_DEBUG($resource, "$host: ".$rr->address.' ('.$rr->ttl.')');
				my $ip = Hector::IpAddr->new();
				$ip->ParseIp4Addr($rr->address);
				$resource->SetIpAddr($ip);
				$resource->SetIpAddrExpire(time() + $rr->ttl) if ($resource->GetTypeString() eq 'SiteResource');
				$resource->SetStatus(0);
				last;
			}
		} else {
			$self->LOG_DEBUG($resource, "Query failed ($host): ".$self->{'_resolver'}->errorstring);
			my $ip = Hector::IpAddr->new();
			$resource->SetIpAddr($ip);
			$resource->SetIpAddrExpire(time() + $self->{'negativeTTL'}) if ($resource->GetTypeString() eq 'SiteResource');
			$resource->SetStatus(1);
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
