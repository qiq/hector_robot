#!/usr/bin/env bash

. test_common.sh

id=deduplicate
test_server_batch $id test $id.hr $id.hr.out
md5sum <$id.hr.out >$id.log.result
test_compare_result $id
exit $?
