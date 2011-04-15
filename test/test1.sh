#!/bin/bash

. test_common.sh

id=test1
test_server_batch $id "$test_base/test/$id.data.url"
grep "M_dump\[[0-9]\+\]: " $id.log|sed -e 's|M_dump\[[0-9]\+\]: \(\[WR.*\)|\1|' >$id.log.result
test_compare_result $id
exit $?
