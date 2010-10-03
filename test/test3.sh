#!/bin/bash

. test_common.sh

test_init
rm -f test3.data.out
test_server_start
hector_client_set test_processing_engine.run 1
hector_client_wait M_save[0].items 1
hector_client_set test_processing_engine.run 0
hector_server_shutdown

md5sum <test3.data.out >$id.log.test
test_finish
test_compare_result
exit $?
