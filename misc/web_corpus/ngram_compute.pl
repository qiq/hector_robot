#!/usr/bin/env perl
use strict;
use warnings;
use Data::Dumper;

@ARGV > 0 or die("usage: ./ngram_compute.pl file_1gram file_2gram ...");

my $n = @ARGV;

my @ngrams;

my @s;
my $lines = 0;
while (<STDIN>) {
	if ($lines++ % 1000000 == 0) {
		print STDERR "Lines read: ".($lines-1)."\n";
	}
	chomp;
	if ($_ =~ /^<[spd]/) {
		if (@s > 0) {
			unshift(@s, "<S>");
			push(@s, "</S>");
			for (my $i = 0; $i < @s; $i++) {
				my $key = '';
				for (my $j = 0; $j < $n and $i+$j < @s; $j++) {
					$key .= "\t" if ($j > 0);
					$key .= $s[$i+$j];
					$ngrams[$j]->{$key}++;
				}
			}
			@s = ();
		}
	} elsif ($_ =~ /^<g/) {
		# ignore
	} else {
		my @a = split(/\t/, $_);
		push(@s, $a[1]);
	}
}

if (@s > 0) {
	unshift(@s, "<S>");
	push(@s, "</S>");
	for (my $i = 0; $i < @s; $i++) {
		my $key = '';
		for (my $j = 0; $j < $n and $i+$j < @s; $j++) {
			$key .= "\t" if ($j > 0);
			$key .= $s[$i+$j];
			$ngrams[$j]->{$key}++;
		}
	}
	@s = ();
}

for (my $i = 0; $i < $n; $i++) {
	my $f;
	open($f, '>'.$ARGV[$i]) or die("Cannot open: ".$ARGV[$i]);
	foreach my $k (sort { $a cmp $b } (keys %{$ngrams[$i]})) {
		print $f "$k\t".$ngrams[$i]->{$k}."\n";
	}
	close($f);
}
