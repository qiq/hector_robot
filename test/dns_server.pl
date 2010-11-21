#!/usr/bin/perl
use strict;
use warnings;

use Net::DNS::Nameserver;

# based on the Perl example

my $port = 5353;
$port = shift(@ARGV) if (@ARGV > 0);

sub reply_handler {
	my ($qname, $qclass, $qtype, $peerhost,$query,$conn) = @_;
	my ($rcode, @ans, @auth, @add);

#	print "Received query from $peerhost to ". $conn->{"sockhost"}. "\n";
#	$query->print;

	if ($qtype eq "A" && $qname eq "www.test.com") {
		my ($ttl, $rdata) = (3600, "127.0.0.1");
		push @ans, Net::DNS::RR->new("$qname $ttl $qclass $qtype $rdata");
		$rcode = "NOERROR";
	} elsif ($qtype eq "A" && $qname eq "www.test.org") {
		my ($ttl, $rdata) = (3600, "10.10.10.10");
		push @ans, Net::DNS::RR->new("$qname $ttl $qclass $qtype $rdata");
		$rcode = "NOERROR";
	} elsif ($qname eq "www.test.com" or $qname eq "www.test.org") {
		$rcode = "NOERROR";
	} elsif ($qtype eq "A" && $qname eq "shutdown") {
		exit;
	} else {
		$rcode = "NXDOMAIN";
	}

	# mark the answer as authoritive (by setting the 'aa' flag
	return ($rcode, \@ans, \@auth, \@add, { aa => 1 });
}
 
my $ns = Net::DNS::Nameserver->new(
	LocalAddr => "127.0.0.1",
	LocalPort => $port,
	ReplyHandler => \&reply_handler,
	Verbose => 0
) or die($!);

$ns->main_loop;
