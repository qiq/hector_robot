#!/bin/bash

. test_common.sh

id=test7
pkill dns_server.pl; pkill http_server.pl; ( dns_server.pl 5354 & http_server.pl 8012 & )
test_server_batch $id test /dev/null $id.data.out
pkill dns_server.pl; pkill http_server.pl
test_server_batch $id dump $id.data.out

grep "M_dump_dump" $id.log|sed -e 's|.*: \[SR[^\]*\] ||'|sed -e 's|, [a-zA-Z]* expire: [0-9]*||g'|sort >$id.log.result
test_compare_result $id
exit $?
