#!/bin/bash

if [ -z "$1" ]; then
	echo "usage: dump.sh file"
	exit 1;
fi

. hector_common.sh

hector_server -c dump.xml -f dump $1 &
hector_client_set PE_dump.run 1
prev=`hector_client_get M_output[0].items`
while true; do
	perl -e 'select(undef, undef, undef, 0.1)'
	i=`hector_client_get M_output[0].items`
	if [ $i -eq $prev ]; then
		break;
	fi
	prev=$i
done
hector_client_set PE_dump.run 0
hector_server_shutdown

exit $?
