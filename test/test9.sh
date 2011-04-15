#!/bin/bash

. test_common.sh

id=test9
pkill dns_server.pl; pkill http_server.pl; ( dns_server.pl 5354 & http_server.pl 8012 & )
test_server_batch $id
pkill dns_server.pl; pkill http_server.pl

grep "M_dump\(4\|5\)" $id.log|sed -e 's|.*: \[WR[^\]*\] ||'|sort >$id.log.result
test_compare_result $id
exit $?
