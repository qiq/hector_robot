#!/usr/bin/env perl
use strict;
use warnings;
use Data::Dumper;

# do a merge sort (files must be sorted)
# usage: ls file*.ngram|./ngram_merge.pl >data.ngram

my @heap;
my @files;

sub next() {
	my ($fh) = @_;
	my $l = <$fh>;
	return undef if (not defined $l);
	chomp($l);
	my $i = rindex($l, "\t");
	return [ substr($l, 0, $i), int(substr($l, $i+1)) ];
}

# create heap
sub heap_insert() {
	my ($heap, $item) = @_;
	return if (not defined $item);
	push(@{$heap}, $item);
	&heap_down($heap);
}

sub heap_up() {
	my ($heap) = @_;
	my $last = @{$heap}-1;

	my $index = 0;
	return if ($last <= 0);

	my $iv = $heap->[$index];
	while ($index < $last) {
		my $child = 2*$index + 1;
		last if $child > $last;
		my $cv = $heap->[$child];
		if ($child < $last) {
			my $cv2 = $heap->[$child+1];
			if ($cv2->[0] lt $cv->[0]) {
				$cv = $cv2;
				++$child;
			}
		}
		last if $iv->[0] le $cv->[0];
		$heap->[$index] = $cv;
		$index = $child;
	}
	$heap->[$index] = $iv;
}

sub heap_down() {
	my ($heap) = @_;

	my $i = @{$heap}-1;
	my $value = $heap->[$i];
	while ($i > 0) {
		my $parent = int(($i-1)/2);
		my $pv = $heap->[$parent];
		last if ($pv->[0] lt $heap->[$i]->[0]);
		$heap->[$i] = $pv;
		$i = $parent;
	}
	$heap->[$i] = $value;
}

# heap_extract
sub heap_extract() {
	my ($heap) = @_;
	return [] if (@{$heap} == 0);
	my $min = $heap->[0];
	if (@{$heap} > 1) {
		$heap->[0] = pop(@{$heap});
		&heap_up($heap);
	} else {
		pop(@{$heap});
	}
	return $min;
}

while (<STDIN>) {
	chomp;
	my $fh;
	open($fh, '<'.$_) or die("Cannot open file: $_");
	my $l = &next($fh);
	push(@heap, [$l->[0], $l->[1], $fh]) if (defined $l);
	push(@files, $fh);
}

# cycle through heap and merge
while (@heap > 0) {
	my $min = &heap_extract(\@heap);
	while (@heap > 0 and $min->[0] eq $heap[0]->[0]) {
		my $tmp = &heap_extract(\@heap);
		$min->[1] += $tmp->[1];
		my $l = &next($tmp->[2]);
		&heap_insert(\@heap, [$l->[0], $l->[1], $tmp->[2]]) if (defined $l);
	}
	# print result of (possible) merge
	print $min->[0]."\t".$min->[1]."\n";
	my $l = &next($min->[2]);
	&heap_insert(\@heap, [$l->[0], $l->[1], $min->[2]]) if (defined $l);
}

foreach my $fh (@files) {
	close($fh);
}
