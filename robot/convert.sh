#!/usr/bin/env bash
#
# - start server
# - load checkpoint data
# - start processing engines

HECTOR_HOST=localhost:1102
. robot_common.sh

if [ -z "$1" -o -z "$2" ]; then
	echo "usage: convert.sh url_list_file resource_file"
	exit 1;
fi

# start server
hector_server_start convert.xml convert $1 $2

hector_client_set PE_convert.run 1
hector_client_wait PE_convert.resourceCount 0

# stop PE and shutdown
hector_client_set PE_convert.run 0
hector_server_shutdown

exit 0;
