#!/usr/bin/env perl
use strict;
use warnings;
use POSIX;

if (@ARGV == 0) {
	my $time = floor(time() / 1000);
	print $time."\n";
} else {
	while (@ARGV > 0) {
		my $time = localtime(shift(@ARGV)*1000);
		print $time."\n";
	}
}

exit(0);
