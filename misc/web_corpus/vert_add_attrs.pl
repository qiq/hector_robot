#!/usr/bin/env perl
#
# - change docs, so that vertical file is suitable for manatee
# - extract subtype from id
# - map id to source URL
# - figure out timestamp to use
# - simplify lang attribute (only one value)

use strict;
use warnings;

@ARGV == 2 or die("usage: vert_add_attrs.pl time_map_file.txt url_map_file.txt");

my $time_map_file = shift(@ARGV);
my $url_map_file = shift(@ARGV);

my $fd;

my %time_map;
open($fd, "<", $time_map_file) or die;
while (<$fd>) {
	chomp;
	next if (/^\s*$/);
	my @a = split(/\t/);
	$time_map{$a[0]} = $a[1];
}
close($fd);

my %url_map;
open($fd, "<", $url_map_file) or die;
while (<$fd>) {
	chomp;
	my @a = split(/\s+/);
	$a[1] =~ s/\.html$/\.txt/;
	$url_map{$a[1]} = $a[0];
}
close($fd);

while (<STDIN>) {
	if (substr($_, 0, 4) eq '<doc') {
		chomp;
		my ($id, $lang, $src, $subtype, $timestamp);
		if (/id="([^"]*)"/) {
			$id = $1;
		} else {
			print STDERR "Missing id: $_\n";
		}
		if (/lang="([^"]*)"/) {
			my @l = split(/ /, $1);
			$lang = " lang=\"".$l[0]."\"";
		} else {
			$lang = "";
		}
		my $server;

		if (not $id =~ s|data/([^/]*)/([^/]*)/[^/]*\/([^/]*)$|$1/$2/$3|) {
			print STDERR "Invalid id: $id\n";
			$subtype = "UNKNOWN";
			$server = "UNKNOWN";
		} else {
			if ($1 eq 'clanky') {
				$subtype = 'article';
			} elsif ($1 eq 'tvorivost') {
				$subtype = 'blog';
			} elsif ($1 eq 'diskuse') {
				$subtype = 'discussion';
			} else {
				print STDERR "Invalid subtype: $1";
			}
			$server = $2;
		}
		$src = $id;

		if (exists $url_map{$id}) {
			$src = $url_map{$id};
		} else {
			print STDERR "Unknown id: $id\n";
			$src = $id;
		}
		if (exists $time_map{$server}) {
			$timestamp = $time_map{$server};
		} else {
			$timestamp = $time_map{'*'};
		}

		print "<doc src=\"$src\"${lang} subtype=\"$subtype\" timestamp=\"$timestamp\">\n";
	} else {
		print $_;
	}
}
