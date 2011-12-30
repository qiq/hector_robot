#!/usr/bin/env bash

if [ $# -ne "1" ]; then
	echo "usage: gzip2bzip2.sh file.gz"
	exit 1;
fi

src=$1
dst=`echo $src|sed -e 's/\.gz$/.bz2'`
gzip -dc $i | bzip2 -c >$dst
