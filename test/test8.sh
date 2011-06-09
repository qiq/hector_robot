#!/bin/bash

. test_common.sh

id=test8
test_server_batch $id test $id.data.in $id.data.out

grep "\(M_dump_dump\|Disallow\)" $id.log|sed -e 's|.*: \[SR[^\]*\] ||'|sed -e 's|, [a-zA-Z]* expire: [0-9]*||g' >$id.log.result
test_compare_result $id
exit $?
