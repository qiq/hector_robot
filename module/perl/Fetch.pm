# Fetch.pm, simple, perl
# Fetch an object (html page) using HTTP. Should not drop resources, because
# resource path may be locked. Sets X-Status value with HTTP Status-Line.
# 
# Dependencies: LWP::UserAgent
# 
# Parameters:
# items			r/o	Total items processed
# timeout			r/w	Download timeout
# from			init	From: header field
# userAgent		init	User-Agent: header field
# 
# Status:
# 0	OK
# 1	error

package Fetch;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);
use LWP::UserAgent;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self = {'userAgent'} = 'Mozilla/5.0 (compatible; hector_robot/Fetch.pm 1.0; +http://host/)';
	$self = {'from'} = undef;
	$self = {'timeout'} = 10;
	$self = {'items'} = 0;

	$self = {'_ua'} = undef;

	bless($self, $class);
	return $self;
}

sub DESTROY {
}

#sub CreateUrlList {
#	my ($self, $s) = @_;
#	return [] if (not defined $s);
#	my @a;
#	foreach my $url (split(/[\n\r]+/, $s)) {
#		$url =~ s/^\s+//;
#		$url =~ s/\s+$//;
#		next if ($url eq '');
#		push(@a, $url);
#	}
#	return \@a;
#}

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

sub ProcessSimple() {
	my ($self, $resource) = @_;

	my $url = $resource->GetUrl();
	if (not defined $url) {
		$self->LOG_ERROR($resource, "No URL found");
		return $resource;
	}
	my $response = $self->{'_ua'}->get($url);
	my @names = $response->header_field_names();
	my @values;
	foreach my $name (@names) {
		push(@values, "".$response->header($name));	# N.B.: force conversion to string
	}
	$resource->SetHeaderFields(\@names, \@values);
	$resource->SetHeaderValue("X-Status", $response->status_line);
	if ($response->is_success) {
		$resource->SetContent($response->decoded_content);
		$resource->SetStatus(0);
	} else {
		$resource->SetContent('');
		$resource->SetStatus(1);
	}
	$self->{'items'}++;
	return $resource;
}

1;
