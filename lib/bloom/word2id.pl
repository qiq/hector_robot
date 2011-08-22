#!/usr/bin/env perl
use strict;
use warnings;

my %dict = ();
my $id = 1;
while (<STDIN>) {
	chomp;
	if ($_ eq "") {
		print pack("V", 0);
	} elsif (exists $dict{$_}) {
		print pack("V", $dict{$_});
	} else {
		$dict{$_} = $id++;
		print pack("V", $dict{$_});
	}
}

my $fd;
open($fd, ">dict") or die;
foreach my $k (keys %dict) {
	print $fd "$k\t".$dict{$k}."\n";
}
close($fd);
