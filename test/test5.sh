#!/bin/bash

. test_common.sh

id=test5
pkill dns_server.pl; pkill http_server.pl; ( dns_server.pl 5354 & http_server.pl 8012 & )
test_server_batch $id test
pkill dns_server.pl; pkill http_server.pl

grep "M_dump[0-9]\[[0-9]\+\]: " $id.log|sed -e 's|M_dump[0-9]\[[0-9]\+\]: \(\[PR .*\)|\1|'|sed -e 's|, ip expire:.*||'|sort >$id.log.result
test_compare_result $id
exit $?
