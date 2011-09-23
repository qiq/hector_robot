#!/usr/bin/env bash

. test_common.sh

id=deduplicate_para
test_server_batch $id test $id.in $id.out
md5sum <$id.out >$id.log.result
test_compare_result $id
exit $?
