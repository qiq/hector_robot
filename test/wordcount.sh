#!/usr/bin/env bash

. test_common.sh

id=wordcount
test_server_batch $id test $id.in $id.out
grep M_wordcount $id.log >$id.log.result
test_compare_result $id
exit $?
