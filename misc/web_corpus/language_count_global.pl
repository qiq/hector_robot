#!/usr/bin/env perl

use strict;
use warnings;

# usage: cat *.log|./languagecount_global.pl cs sk

#M_lc[0]: cs:77479(99.92) en:13(0.02) sk:12(0.02) az:10(0.01) de:4(0.01) zh:3(0.00) ca:3(0.00) cy:2(0.00) sl:2(0.00) af:2(0.00) fr:2(0.00) et:1(0.00) sv:1(0.00) hr:1(0.00) nl:1(0.00) lv:1(0.00) pl:1(0.00) id:1(0.00) be:1(0.00)

my %interesting;
my $other = 0;
foreach my $i (@ARGV) {
	$interesting{$i} = 1;
	$other = 1;
}

my %count;
while (<STDIN>) {
	chomp;
	if (s/.*M_lc\[0\]: //) {
		foreach my $a (split(/ /)) {
			$a =~ /(.*):([0-9]+).*/;
			if (not $other or exists $interesting{$1}) {
				$count{$1} += $2;
			} else {
				$count{'other'} += $2;
			}
		}
	}
}

foreach my $k (sort { $count{$b} <=> $count{$a} } (keys %count)) {
	print "$k\t".$count{$k}."\n";
}
