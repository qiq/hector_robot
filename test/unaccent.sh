#!/usr/bin/env bash

. test_common.sh

id=unaccent
test_server_batch $id test $id.in $id.out
cp $id.out $id.log.result
test_compare_result $id
exit $?
