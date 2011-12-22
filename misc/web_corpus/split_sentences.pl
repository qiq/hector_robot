#!/usr/bin/env perl

use strict;
use warnings;
use locale;
use Data::Dumper;

@ARGV == 2 or die("usage: split_sentences.pl max_sentences prefix <input.vert");

my $max_sentences =  $ARGV[0];
my $prefix = $ARGV[1];

my $s = 0;
my $fh;
my $id = 0;
open($fh, '>'.$prefix.'.'.$id) or die("Cannot open file: $prefix.$id");
while (<STDIN>) {
	if (/^<s/) {
		$s++;
		if ($s > $max_sentences) {
			close($fh);
			$id++;
			open($fh, '>'.$prefix.'.'.$id) or die("Cannot open file: $prefix.$id");
			$s = 1;
		}
	}
	print $fh $_;
}
close($fh);
