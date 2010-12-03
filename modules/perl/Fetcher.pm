# items: number of resources created

package Fetcher;

use warnings;
use strict;
use HectorRobot;
use LWP::UserAgent;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'_ua' => undef,
		'userAgent' => 'Mozilla/5.0 (compatible; hector_robot/Fetch.pm 1.0; +http://host/)',
		'from' => undef,
		'timeout' => 10,
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

	# second stage?
	return 1 if (not defined $params);

	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}
	my %options = ();
	$options{'agent'} = $self->{'userAgent'} if (defined $self->{'userAgent'});
	$options{'from'} = $self->{'from'} if (defined $self->{'from'});
	$options{'timeout'} = $self->{'timeout'} if (defined $self->{'timeout'});
	$self->{'_ua'} = LWP::UserAgent->new(%options);

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

	my $url = $resource->getUrl();
	if (not defined $url) {
		$self->{'_object'}->log_error($resource->toStringShort()." Resource does not contain URL");
		$resource->setFlag($Hector::Resource::DELETED);
		return $resource;
	}
	my $response = $self->{'_ua'}->get($url);
	my @names = $response->header_field_names();
	my @values;
	foreach my $name (@names) {
		push(@values, "".$response->header($name));	# N.B.: force conversion to string
	}
	$resource->setHeaderFields(\@names, \@values);
	if ($response->is_success) {
		$resource->setContent($response->decoded_content);
		$resource->setStatus(0);
	} else {
		$resource->setContent('');
		$resource->setStatus(1);
	}
	$self->{'items'}++;
	return $resource;
}

1;
