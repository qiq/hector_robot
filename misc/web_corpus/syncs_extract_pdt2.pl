#!/usr/bin/env perl

use strict;
use warnings;
use locale;
use Data::Dumper;

# usage: ./extract_syncs_pdt2.pl <data.vert >data.syncs

# input format:
# word		lc		lemma		pos		parent	deprel
# WESTERBURG	westerburg	Westerburg	NNIS1-----A----	2	ExD_Co	–	-	Z:-------------			

# constants
my $word = 0;
my $lemma = 1;
my $pos = 2;
my $parent = 3;
my $deprel = 4;
my $children = 5;

# data
my %data = ();
my $position = 0;

my %interesting = (
	'N' => 1,
	'A' => 1,
	'V' => 1,
	'D' => 1,
);

sub extract_relations() {
	my ($s) = @_;
	my $len = @{$s};
	for (my $i = 0; $i < $len; $i++) {
		my $w = $s->[$i];
		my $p = $s->[$w->[$parent]];
		my $w_pos = substr($w->[$pos], 0, 1);
		next if (not exists $interesting{$w_pos});
		my $p_pos = substr($p->[$pos], 0, 1);
		next if (not exists $interesting{$p_pos});
		if ($w->[$deprel] eq 'Obj') {
			push(@{$data{$w->[$lemma].'-'.$w_pos}->{'Obj_of'}->{$p->[$lemma]}}, $position);
			push(@{$data{$p->[$lemma].'-'.$p_pos}->{'Obj'}->{$w->[$lemma]}}, $position);
		} elsif ($w->[$deprel] eq 'Sb') {
			push(@{$data{$w->[$lemma].'-'.$w_pos}->{'Sb_of'}->{$p->[$lemma]}}, $position);
			push(@{$data{$p->[$lemma].'-'.$p_pos}->{'Sb'}->{$w->[$lemma]}}, $position);
		} elsif ($w->[$deprel] eq 'Adv') {
			push(@{$data{$w->[$lemma].'-'.$w_pos}->{'Adv_of'}->{$p->[$lemma]}}, $position);
			push(@{$data{$p->[$lemma].'-'.$p_pos}->{'Adv'}->{$w->[$lemma]}}, $position);
		} elsif ($w->[$deprel] eq 'Atr') {
			push(@{$data{$w->[$lemma].'-'.$w_pos}->{'Atr_of'}->{$p->[$lemma]}}, $position);
			push(@{$data{$p->[$lemma].'-'.$p_pos}->{'Atr'}->{$w->[$lemma]}}, $position);
		}
		$position++;
	}
}

sub create_children() {
	my ($s) = @_;
	my @children;
	my $len = @{$s};
	for (my $i = 0; $i <= $len; $i++) {
		push(@children, []);
	}
	for (my $i = 0; $i < $len; $i++) {
		my $idx = $s->[$i]->[3];
		push(@{$children[$idx]}, $i) if ($idx >= 0);
	}
	for (my $i = 0; $i < $len; $i++) {
		push(@{$s->[$i]}, $children[$i]);
	}
}

sub print_data() {
	my ($data) = @_;

	foreach my $k (sort { $a cmp $b } keys %data) {
		my @l;
		my $c1 = 0;
		foreach my $d (keys %{$data{$k}}) {
			my @l2;
			foreach my $l (keys %{$data{$k}->{$d}}) {
				my $a = $data{$k}->{$d}->{$l};
				push(@l2, $l, scalar(@{$a}), @{$a});
			}
			push(@l, $d, scalar(@l2), @l2);
		}
		print $k."\t".join("\t", @l)."\n";
	}
}

my @s;
while (<STDIN>) {
	chomp;
	if (substr($_, 0, 1) eq '<') {
		my $second = substr($_, 1, 1);
		if ($second eq 's') {
			if (@s > 0) {
				&create_children(\@s);
				&extract_relations(\@s);
				@s = ();
			}
#		} elsif ($second ne 'd' and $second ne 'p' and $second ne 'g') {
#			my ($word, $lc, $lemma, $pos, $parent, $deprel, $other) = split(/\t/);
#			push(@s, [ $word, $lemma, $pos, $parent-1, $deprel ]);
		}
	} else {
#		my ($word, $lc, $lemma, $pos, $parent, $deprel, $other) = split(/\t/);
#		push(@s, [ $word, $lemma, $pos, $parent-1, $deprel ]);
		my ($flags, $word, $lemma, $pos, $parent, $deprel, $other) = split(/\t/);
		$lemma =~ s/[-_`].*// if ($lemma !~ /^[-_`]/);
		push(@s, [ $word, $lemma, $pos, $parent-1, $deprel ]);
	}
}
if (@s > 0) {
	&create_children(\@s);
	&extract_relations(\@s);
}
print $position."\n";
&print_data(\%data);
