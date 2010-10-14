# items: number of resources created

package ResolveDns;

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
	};
	bless($self, $class);
	return $self;
}

sub DESTROY {
}

sub createUrlList {
	my ($self, $s) = @_;
	return [] if (not defined $s);
	my @a;
	foreach my $url (split(/[\n\r]+/, $s)) {
		$url =~ s/^\s+//;
		$url =~ s/\s+$//;
		next if ($url eq '');
		push(@a, $url);
	}
	return \@a;
}

sub Init {
	my ($self, $params) = @_;
	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}
	$self->{'_resolver'} = Net::DNS::Resolver->new(search => ''),

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
		my $request = $self->{'_resolver'}->search($host, 'A');
		if ($request) {
			foreach my $rr ($request->answer) {
				next unless $rr->type eq "A";
				$self->{'_object'}->log_error("$host: ".$rr->address.' ('.$rr->ttl.')');
				my $addr = Hector::str2Ip4Addr($rr->address);
				$resource->setIp4Addr($addr);
				Hector::ip4AddrDelete_w($addr);
				$resource->setIpAddrExpire(time() + $rr->ttl);
			}
		} else {
			$self->{'_object'}->log_debug("Query failed ($host): ".$self->{'_resolver'}->errorstring);
		}
	}

	$self->{'items'}++;
	return $resource;
}

1;
