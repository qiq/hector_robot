#!/bin/bash

if [ -z "$1" ]; then
	echo "usage: dump_url.sh file"
	exit 1;
fi

./dump.sh $1 | grep M_dump | sed 's/.*\] url: \([^ ]\+\) (.*/\1/'
