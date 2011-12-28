#!/usr/bin/env perl
#
# Remove leading numeric tags from token lines

use strict;
use warnings;

while (<STDIN>) {
	if (substr($_, 0, 1) eq '<') {
		if (/^<g\/>/ or /^<doc/ or /^<s>/ or /^<p>/) {
			if (/^<doc.*lang="([^"]*)"/) {
				my %a;
				foreach my $a (split(/ /, $1)) {
					$a{$a} = 1;
				}
				my $lang = join(" ", keys %a);
				$_ =~ s/lang="[^"]*"/lang="$lang"/;
			}
			print $_;
		} else {
			die($_);
		}
	} else {
		$_ =~ s/^[0-9]*\t//;
		print $_;
	}
}
