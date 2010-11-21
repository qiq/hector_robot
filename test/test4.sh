#!/bin/bash

. test_common.sh

test_init
test_server_start
( ./dns_server.pl 5354 & )
hector_client_set PE_test.run 1
hector_client_wait M_output[0].items 5
hector_client_set PE_test.run 0
hector_server_shutdown
pkill dns_server.pl

grep " WebResource \[" test.log|sed -e 's|M_dump[0-9]\[[0-9]\+\]: WebResource \(.*\)|\1|'|sed -e 's|ip expire:.*||'|sort >$id.log.test
test_finish
test_compare_result
exit $?
