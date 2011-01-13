#!/usr/bin/env bash
#
# - start server
# - load checkpoint data
# - start processing engines

. robot_common.sh

if [ -z "$1" ]; then
	echo "usage: start.sh file [checkpoint]"
	exit 1;
fi

#set -x
# prepare environment
rm -f robot.log
mkdir refresh 2>/dev/null

# start server
hector_server_start robot.xml robot $1

# load checkpoint data
if [ -n "$2" ]; then
	hector_client_restore_checkpoint "$2"
fi

# start PEs
hector_client_set PE_robot.run 1
hector_client_set PE_dns.run 1
hector_client_set PE_robots.run 1

exit 0;
