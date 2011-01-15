#!/usr/bin/env bash
#
# - pause input
# - save checkpoint data
# - unpause input

. robot_common.sh

if [ -z "$1" ]; then
	echo "usage: save_checkpoint.sh checkpoint"
	exit 1;
fi

# pause input
hector_client_set P_load.pauseInput 1

# wait for queues to become empty
hector_client_wait_all P_main.queue_size.0 0 P_main.queue_size.100 0 PE_robot.resourceCount 0 PE_dns.resourceCount 0 PE_robots.resourceCount 0

# load checkpoint data
hector_client_save_checkpoint "$1"

# unpause input
hector_client_set P_load.pauseInput 0

exit 0;
