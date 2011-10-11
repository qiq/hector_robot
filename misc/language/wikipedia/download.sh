#!/usr/bin/env bash

CODES=codes.txt

cat $CODES|while read c; do
	wget http://dumps.wikimedia.org/${c}wiki/latest/${c}wiki-latest-pages-articles.xml.bz2
	sleep 1;
done
