#!/bin/bash

. test_common.sh

id=test6
pkill dns_server.pl; pkill http_server.pl; ( dns_server.pl 5354 & http_server.pl 8012 & )
test_server_batch $id test /dev/null $id.data.out
pkill dns_server.pl; pkill http_server.pl

grep "M_dump[12]\[0\]: \[[PS]R" $id.log|sed -e 's|M_dump[^\[]*\[[0-9]\+\]: \[[PS]R[^\]*\] ||'|sed -e 's|, [a-zA-Z]* expire: [0-9]*||g'|sort >$id.log.result
test_compare_result $id
exit $?
