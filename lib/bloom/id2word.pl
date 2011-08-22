#!/usr/bin/env perl
use strict;
use warnings;

my %dict = ();

my $fd;
open($fd, "<dict") or die;
while (<$fd>) {
	my @a = split(/\s+/, $_);
	$dict{$a[1]} = $a[0];
}
close($fd);

my $data;
while (read(STDIN, $data, 4) == 4) {
	my $id = unpack('V', $data);
	if ($id == 0) {
		print "\n";
	} else {
		print $dict{$id}."\n";
	}
}
