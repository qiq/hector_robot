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
			my $data = "<html><head><meta http-equiv=\"refresh\" content=\"5;url=".$d->url."meta.html\"/></head>";
			$data .= <<END;
<body background="back.gif">
<a href="bar">bar bar BAR</a>!
<img alt="test" src="testa.html" width="1"/>bar bar BAR
<img alt="test" src="testb.pdf"/>bar bar BAR
<map>
<area shape="rect" coords="0,0,1,1" href="t.html" alt="t"/>
<area href="ta.html"/>
</map>
<frameset>
<frame src="frame.html"/>
</frameset>
<iframe src ="frame2.html" width="100%" height="300" />
</body></html>
END
			$c->print($data);
			$c->force_last_request();
		} elsif ($r->method eq 'GET' and $r->url->path eq "/robots.txt") {
			$c->send_basic_header(200);
			print $c "Content-Type: text/plain";
			$c->send_crlf;
			$c->send_crlf;
			$c->print("User-agent: *\r\nDisallow: /*bar.html\$\r\nDisallow: /disabled\r\n");
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
			print $c "Location: ".$d->url."redirect3.html";
			$c->send_crlf;
			$c->send_crlf;
			$c->print("See other page\n");
			$c->force_last_request();
		} elsif ($r->method eq 'GET' and $r->url->path eq "/redirect3.html") {
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
