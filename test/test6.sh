#!/bin/bash

. test_common.sh

test_init
test_server_start
( ./dns_server.pl 5354 &  ./http_server.pl 8012 & )
hector_client_set PE_test.run 1
hector_client_set PE_dns.run 1
hector_client_set PE_robots.run 1
hector_client_wait M_output[0].items 4
hector_client_set PE_test.run 0
hector_client_set PE_dns.run 0
hector_client_set PE_robots.run 0
hector_server_shutdown
pkill dns_server.pl
pkill http_server.pl

grep "M_dump" test.log|sed -e 's|M_dump[^\[]*\[[0-9]\+\]: \[WS\?R[^\]*\] ||'|sed -e 's|, [a-zA-Z]* expire: [0-9]*||g'|sort >$id.log.test
test_finish
test_compare_result
exit $?
