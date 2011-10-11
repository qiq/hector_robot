#!/bin/sh
# clean up wiki page using quick 'n' dirty regexps

sed 's/>/>\n/g' | sed 's/</\n</g' | ./wiki2text.pl
