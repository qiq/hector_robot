#!/bin/bash

. test_common.sh

id=test8
rm -f test8.data.out
test_server_start $id
hector_client_set M_wsm[0].load test8.data.in
hector_client_set M_wsm[0].save test8.data.out
hector_client_set PE_dump.run 1
hector_client_set M_dump_load[0].filename test8.data.in
hector_client_wait M_dump_output[0].items 2
hector_client_set M_dump_load[0].filename test8.data.out
hector_client_wait M_dump_output[0].items 4
hector_client_set PE_dump.run 0
hector_server_shutdown

grep "\(M_dump_dump\|Disallow\)" $id.log|sed -e 's|.*: \[WSR[^\]*\] ||'|sed -e 's|, [a-zA-Z]* expire: [0-9]*||g' >$id.log.result
test_compare_result $id
exit $?
