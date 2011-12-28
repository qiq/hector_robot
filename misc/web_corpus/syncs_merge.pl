#!/usr/bin/env perl

use strict;
use warnings;
use locale;
use Data::Dumper;

# do a merge sort (files must be sorted)
# usage: ls file*.sync|./sync_merge.pl >data.syncs

my @heap;
my @files;

sub next() {
	my ($fh) = @_;
	my $l = <$fh>;
	return undef if (not defined $l);
	chomp($l);
	my $i = index($l, "\t");
	return [ substr($l, 0, $i), substr($l, $i+1) ];
}

sub parse() {
	my ($l) = @_;

	#align-D Adv     3       div     1       0
	my %data;
	while (@{$l} > 0) {
		my $deprel = shift(@{$l});
		my $n = shift(@{$l});
		my $i = 0;
		while ($i < $n) {
			my $w = shift(@{$l});
			my $m = shift(@{$l});
			$i += 2;
			for (my $j = 0; $j < $m; $j++) {
				push(@{$data{$deprel}->{$w}}, shift(@{$l}));
				$i++;
			}
		}
	}
	return \%data;
}

sub merge_parsed() {
	my ($a, $b) = @_;

	foreach my $d (keys %{$b}) {
		if (exists $a->{$d}) {
			# present deprel: join words
			foreach my $w (keys %{$b->{$d}}) {
				if (exists $a->{$d}->{$w}) {
					# present word: join
					push(@{$a->{$d}->{$w}}, @{$b->{$d}->{$w}});
				} else {
					# new word
					$a->{$d}->{$w} = $b->{$d}->{$w};
				}
			}
		} else {
			# new deprel
			$a->{$d} = $b->{$d};
		}
	}
}

sub print_parsed() {
	my ($key, $data) = @_;

	my @l;
	my $c1 = 0;
	foreach my $d (keys %{$data}) {
		my @l2;
		foreach my $l (keys %{$data->{$d}}) {
			my $a = $data->{$d}->{$l};
			push(@l2, $l, scalar(@{$a}), @{$a});
		}
		push(@l, $d, scalar(@l2), @l2);
	}
	print $key."\t".join("\t", @l)."\n";
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
print STDERR Dumper(\@files);
print STDERR Dumper(\@heap);

# cycle through heap and merge
while (@heap > 0) {
	my $min = &heap_extract(\@heap);
	my $min_parsed = undef;
	while (@heap > 0 and $min->[0] eq $heap[0]->[0]) {
		$min_parsed = &parse([split(/\t/, $min->[1])]) if (not defined $min_parsed);
		my $tmp = &heap_extract(\@heap);
		my $tmp_parsed = &parse([split(/\t/, $tmp->[1])]);
print STDERR "merge: ".Dumper($min_parsed)." ".Dumper($tmp_parsed)."\n";
		&merge_parsed($min_parsed, $tmp_parsed);
print STDERR "merge result: ".Dumper($min_parsed)."\n";
		my $l = &next($tmp->[2]);
		&heap_insert(\@heap, [$l->[0], $l->[1], $tmp->[2]]) if (defined $l);
	}
	# print result of (possible) merge
	if (defined $min_parsed) {
		&print_parsed($min->[0], $min_parsed);
	} else {
		print $min->[0]."\t".$min->[1]."\n";
	}
	my $l = &next($min->[2]);
	&heap_insert(\@heap, [$l->[0], $l->[1], $min->[2]]) if (defined $l);
}

foreach my $fh (@files) {
	close($fh);
}
