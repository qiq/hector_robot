#!/usr/bin/perl
use strict;
use warnings;

use HTTP::Daemon;
use HTTP::Status;

# based on the Perl example

my $port = 1235;
$port = shift(@ARGV) if (@ARGV > 0);

my $d = HTTP::Daemon->new(
	LocalAddr => 'localhost',
	LocalPort => $port,
) || die($!);
#print "Please contact me at: <URL:", $d->url, ">\n";
while (my $c = $d->accept) {
	while (my $r = $c->get_request) {
		if ($r->method eq 'GET' and $r->url->path eq "/xyzzy") {
			# remember, this is *not* recommened practice :-)
			$c->send_file_response("/etc/passwd");
		} else {
			$c->send_error(RC_FORBIDDEN)
		}
	}
	$c->close;
	undef($c);
}
