#!/bin/bash

. test_common.sh

test_init
test_server_start
hector_client_set test_processing_engine.run 1
hector_client_wait M_save[0].items 16
hector_client_set test_processing_engine.run 0
hector_server_shutdown

grep " WebResource \[" test.log|sed -e 's|M_dump\[[0-9]\+\]: WebResource \(.*\)|\1|' >$id.log.test
test_finish
test_compare_result
exit $?
