#!/bin/bash

. test_common.sh

test_init
test_server_start
rm -rf test10.tmp; mkdir test10.tmp
( ./dns_server.pl 5354 &  ./http_server.pl 8012 & )
hector_client_set PE_test.run 1
hector_client_set PE_dns.run 1
hector_client_set PE_robots.run 1
hector_client_wait M_output[0].items 3
hector_client_set PE_test.run 0
hector_client_set PE_dns.run 0
hector_client_set PE_robots.run 0
# close (flush) all files
hector_client_set M_scheduler[0].outputDir test10.tmp
hector_client_set PE_dump.run 1
file=`ls test10.tmp/*|head -n1`
hector_client_set M_dump_load[0].filename $file
hector_client_wait M_dump_output[0].items 3
hector_server_shutdown
pkill dns_server.pl
pkill http_server.pl
rm -rf test10.tmp

grep "M_dump_dump" test.log|sed -e 's|.*: \[WR[^\]*\] ||'|sed -e 's|, last scheduled: [0-9]*||g'|sort >$id.log.test
test_finish
test_compare_result
exit $?
