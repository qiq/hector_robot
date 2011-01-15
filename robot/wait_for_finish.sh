#!/usr/bin/env bash
#
# - wait for the state when no resources are being processed
# - save checkpoint data
# - stop processing engines and shutdown the server

. robot_common.sh

if [ -z "$1" ]; then
	echo "usage: wait_for_finish.sh checkpoint"
	exit 1;
fi

# wait for queues to become empty
hector_client_wait_all P_main.queue_size.0 0 P_main.queue_size.100 0 PE_robot.resourceCount 0 PE_dns.resourceCount 0 PE_robots.resourceCount 0

# save checkpoint data
hector_client_save_checkpoint "$1"

# stop PEs
hector_client_set PE_robot.run 0
hector_client_set PE_dns.run 0
hector_client_set PE_robots.run 0

# shutdown
hector_server_shutdown

exit 0;
