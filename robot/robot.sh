#!/bin/bash

base=..
export LD_LIBRARY_PATH=$base/resources:$base/modules:$base/servers:$base/perl/.libs:$LD_LIBRARY_PATH
export PERL5LIB=$base/perl:$base/modules/perl:$PERL5LIB

. hector_common.sh
#set -x

rm -rf robot.log #refresh
#mkdir refresh
hector_server_start robot.xml robot
#valgrind hector_server -c robot.xml -fa robot &
#sleep 15
hector_client_set PE_robot.run 1
hector_client_set PE_dns.run 1
hector_client_set PE_robots.run 1
hector_client_wait_lower M_postprocess_save[0].items 2000
hector_client_set PE_robot.run 0
hector_client_set PE_dns.run 0
hector_client_set PE_robots.run 0
hector_server_shutdown

exit $?
