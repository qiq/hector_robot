#!/bin/bash

. test_common.sh

id=test12
pkill dns_server.pl; pkill http_server.pl; ( dns_server.pl 5354 & http_server.pl 8012 & )
test_server_batch $id "$test_base/test/$id.filter"
pkill dns_server.pl; pkill http_server.pl

grep "M_dump5" $id.log|sed -e 's|.*: \[WR[^\]*\] ||'|sed -e 's|, scheduled: [0-9]*||g'|sort >$id.log.result
test_compare_result $id
exit $?
