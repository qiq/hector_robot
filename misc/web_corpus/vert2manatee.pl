#!/usr/bin/env perl
#
# run with LC_ALL=cs_CZ.utf8!
#
# - lemma is simplified
# - </s> is added
# - output format:
#   form lemma pos parent parent_lemma parent_pos deprel children(M) children_pos(M) children_lemma(M)

use strict;
use warnings;

sub ProcessSentence($$) {
	my ($s, $g) = @_;

	# prepare data structure
	my @s2;
	for (my $i = 0; $i < @{$s}; $i++) {
		push(@s2, [ '', '', '', '', '', '', '', '', {}, {}, {} ]);
	}

	# fill it
	for (my $i = 0; $i < @{$s}; $i++) {
		my ($x, $form, $lemma, $pos, $parent_index, $deprel) = @{$s->[$i]};
		$lemma =~ s/[-_`].*// if ($lemma !~ /^[-_`]/);
		my ($lcform, $parent, $parent_lemma, $parent_pos);
		$lcform = lc($form);
		if ($parent_index != 0) {
			$parent = $s->[$parent_index-1]->[1];
			$parent_lemma = $s->[$parent_index-1]->[2];
			$parent_lemma =~ s/[-_`].*// if ($parent_lemma !~ /^[-_`]/);
			$parent_pos = $s->[$parent_index-1]->[3];
		} else {
			$parent = '#root';
			$parent_lemma = '#root';
			$parent_pos = '#root';
		}
		splice(@{$s2[$i]}, 0, 8, $form, $lcform, $lemma, $pos, $parent, $parent_lemma, $parent_pos, $deprel);
		if ($parent_index != 0) {
			$s2[$parent_index-1]->[8]->{$form} = 1;
			$s2[$parent_index-1]->[9]->{$lemma} = 1;
			$s2[$parent_index-1]->[10]->{$pos} = 1;
		}
	}

	# print it
	for (my $i = 0; $i < @s2; $i++) {
		print "<g/>\n" if ($g->[$i]);
		print join("\t", @{$s2[$i]}[0..7])."\t".join(" ", keys %{$s2[$i]->[8]})."\t".join(" ", keys %{$s2[$i]->[9]})."\t".join(" ", keys %{$s2[$i]->[10]})."\n";
	}
}

my @s;
my @g;
my $g = 0;
while (<STDIN>) {
	if (substr($_, 0, 1) eq '<') {
		if (/^<g\/>/) {
			push(@g, 1);
			$g = 1;
		} elsif (/^<doc/ or /^<s>/ or /^<p>/) {
			if (@s > 0) {
				ProcessSentence(\@s, \@g);
				@s = ();
				@g = ();
			}
			print $_;
		} else {
			die($_);
		}
	} else {
		chomp;
		push(@s, [ split(/\t/) ]);
		push(@g, 0) if (!$g);
		$g = 0;
	}
}
if (@s > 0) {
	ProcessSentence(\@s, \@g);
}
