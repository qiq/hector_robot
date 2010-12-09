#!/usr/bin/perl
use strict;
use warnings;

use HTTP::Daemon;
use HTTP::Status;

# based on the Perl example

my $port = 8012;
$port = shift(@ARGV) if (@ARGV > 0);

my $d = HTTP::Daemon->new(
	LocalAddr => 'localhost',
	LocalPort => $port,
	ReuseAddr => 1, 
) || die($!);
#print "Please contact me at: <URL:", $d->url, ">\n";
while (my $c = $d->accept) {
	while (my $r = $c->get_request) {
		#print $r->url->path."\n";
		if ($r->method eq 'GET' and ($r->url->path eq "/foo.html" || $r->url->path eq "/boo.html")) {
			$c->send_basic_header(200);
			print $c "Content-Type: text/plain";
			$c->send_crlf;
			$c->send_crlf;
			$c->print("<html><head/><body><a href=\"bar\">bar bar BAR</a>!</body></html>\n");
			$c->force_last_request();
		} elsif ($r->method eq 'GET' and $r->url->path eq "/robots.txt") {
			$c->send_basic_header(200);
			print $c "Content-Type: text/plain";
			$c->send_crlf;
			$c->send_crlf;
			$c->print("User-agent: *\r\nDisallow: /*bar.html\$\n");
			$c->force_last_request();
		} elsif ($r->method eq 'GET' and $r->url->path eq "/redirect.html") {
			$c->send_basic_header(301);
			print $c "Content-Type: text/plain";
			$c->send_crlf;
			print $c "Location: ".$d->url."redirect2.html";
			$c->send_crlf;
			$c->send_crlf;
			$c->print("See other page\n");
			$c->force_last_request();
		} elsif ($r->method eq 'GET' and $r->url->path eq "/redirect2.html") {
			$c->send_basic_header(302);
			print $c "Content-Type: text/plain";
			$c->send_crlf;
			print $c "Location: ".$d->url."boo.html";
			$c->send_crlf;
			$c->send_crlf;
			$c->print("See other page\n");
			$c->force_last_request();
		} else {
			$c->send_error(RC_FORBIDDEN)
		}
	}
	$c->close;
	undef($c);
}
