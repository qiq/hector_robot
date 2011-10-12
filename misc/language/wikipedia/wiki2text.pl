#!/usr/bin/env perl

use strict;
use warnings;

my $DEBUG = 0;

my %tags = (
	'gallery' => 1,
	'timeline' => 1,
	'math' => 1,
	'source' => 1,
	'pre' => 1,
	'table' => 1,
	'imagemap' => 1,
	'nowiki' => 1,
	'includeonly' => 1,
	'onlyinclude' => 1,
	'noinclude' => 1,
	'references' => 1,
	'code' => 1,
	'ref' => 1,
);

sub process_text() {
	my ($text) = @_;

	# first, process line-dependent markup
	my @text2;
	foreach my $t (@{$text}) {
		print "X $t\n" if ($DEBUG);
		# start/end spaces
		$t =~ s/^\s+//;
		$t =~ s/\s+$//;
		next if ($t =~ s/^\s*$/__NLNL__/g);
		# horizontal rule
		$t =~ s/----+//g;
		# table rows	
		$t =~ s/^\|.*//;
		$t =~ s/^!.*//;
		# #REDIRECT
		next if ($t =~ /^#/);
		# definition
		next if ($t =~ /^\;/);
		# list
		$t =~ s/^[\*#:]+\s*(.*)/$1__NLNL__/;
		# tag-only
		next if ($t =~ /^\[\[[^[]+\]\][^[:alpha:]]*$/);
		# headers
		if ($t =~ /^={1,6}\s*([^=]+)\s*={1,6}$/) {
			# we do not want headers as they are usualy "Links, ..."
			next;
			#push(@text2, $1."__NLNL__");
		}

		push(@text2, $t);
	}

	my $t = join(" ", @text2);

	# bold, italics, ...
	$t =~ s/'''(.*?)'''/$1/g;
	$t =~ s/''(.*?)''/$1/g;
	$t =~ s/''+//g;

	# tags: gallery, timeline, ...
	my $in_tags = 0;
	my $t2 = "";
	while ($t =~ s/(.*?)(&lt;)(\/?)([[:alpha:]]+)(.*?&gt;)//) {
		if (exists $tags{lc($4)}) {
			if ($in_tags == 0) {
				$t2 .= $1;
			}
			if ($3 eq '') {
				# begin of a tag
				$in_tags++ if ($5 !~ /\s*\/\s*&gt;/);
			} else {
				# end of a tag
				$in_tags-- if ($in_tags > 0);
			}
		} else {
			# unknown tag
			if ($in_tags == 0) {
				$t2 .= $1.$2.$3.$4.$5;
			}
		}
	}
	$t2 .= $t if ($in_tags == 0);
	$t = $t2;

	# translate entities, ignore HTML
	$t =~ s/&lt;\s*br\s*\/?\s*&gt;/__NLNL__/g;
	$t =~ s/&lt;!--.*?--&gt;//g;
	$t =~ s/&lt;.*?&gt;//g;
	$t =~ s/&lt;/</g;
	$t =~ s/&gt;/>/g;
	$t =~ s/&quot;/"/g;
	$t =~ s/&amp;/&/g;
	$t =~ s/&nbsp;/ /g;

	# comment
	my $in_comment = 0;
	$t2 = "";
	while ($t =~ s/(.*?)(<!--|-->)//) {
		$t2 .= $1 if ($in_comment == 0);
		if ($2 eq '<!--') {
			$in_comment++;
		} else {
			$in_comment-- if ($in_comment > 0);
		}
	}
	$t2 .= $t if ($in_comment == 0);
	$t = $t2;

	# {{ }}
	my $in_cb = 0;
	$t2 = "";
	while ($t =~ s/(.*?)(\{\{|\}\})//) {
		$t2 .= $1 if ($in_cb == 0);
		if ($2 eq '{{') {
			$in_cb++;
		} else {
			$in_cb-- if ($in_cb > 0);
		}
	}
	$t2 .= $t if ($in_cb == 0);
	$t = $t2;

	# table
	my $in_table = 0;
	$t2 = "";
	while ($t =~ s/(.*?)(\{\||\|\})//) {
		$t2 .= $1 if ($in_table == 0);
		if ($2 eq '{|') {
			$in_table++;
		} else {
			$in_table-- if ($in_table > 0);
		}
	}
	$t2 .= $t if ($in_table == 0);
	$t = $t2;

	# delete all images (thumbnails)
	$t =~ s/\[\[[^]]*\|thumb\|[^]]*\]\]//g;
	# wiki markup, retain bar and fie from [[foo|bar]] [[fie]]
	$t =~ s/\[\[[^]]*\|//g;
	$t =~ s/\]\]//g;
	$t =~ s/\[\[//g;

	# URL
	$t =~ s/\[http:[^]\s]*\s+([^]]*)\]/$1/g;
	# ignore paragraphs starting with ',', '.' etc.
#	next if ($t =~ /^\s*[,.]/);
	# delete __XXX__
	$t =~ s/__NLNL__\s*/\n\n/g;
	$t =~ s/__[^_]+__//g;
	# delete URL
	$t =~ s/\[https?:\/\/[^]]*\]//g;
	# beginning is a space
	$t =~ s/^\s*//;
	# 'must' be text
	print "T: " if ($DEBUG);
	print "$t\n\n";
}

my $in_text = 0;
my @text;
while (<STDIN>) {
	next if (/<text[^\/>]*\/[^>]*>/);
	if (/<text[^>]*>/) {
		$in_text = 1;
		next;
	}
	next if (not $in_text);
	if (/<\/text>/) {
		$in_text = 0;
		&process_text(\@text);
		@text = ();
	} else {
		push(@text, $_);
	}
}

&process_text(\@text) if (@text > 0);
