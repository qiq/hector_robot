#!/usr/bin/env bash

. test_common.sh

id=detect_language
test_server_batch $id test $id.in $id.out
grep "<doc" <$id.out >$id.log.result
test_compare_result $id
exit $?
