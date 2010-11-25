#!/bin/bash

. test_common.sh

test_init
test_server_start
hector_client_set PE_test.run 1
hector_client_wait M_output[0].items 15
hector_client_set PE_test.run 0
hector_server_shutdown

grep "M_dump\[[0-9]\+\]: " test.log|sed -e 's|M_dump\[[0-9]\+\]: \(\[WR.*\)|\1|' >$id.log.test
test_finish
test_compare_result
exit $?
