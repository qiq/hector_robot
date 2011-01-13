#!/usr/bin/env perl
# Transform timestamps as seen in log files to current time. Timestamp is
# seconds since 01-01-1970. Specify no time to convert current time to the
# timestamp.
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
