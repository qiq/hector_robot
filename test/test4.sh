#!/bin/bash

. test_common.sh

id=test4
pkill http_server.pl; ( http_server.pl 8012 & )
test_server_batch $id
pkill http_server.pl

grep "M_dump[0-9]\[[0-9]\+\]: " $id.log|sed -e 's|M_dump[0-9]\[[0-9]\+\]: \(\[WR .*\)|\1|'|sed -e 's|, ip expire:.*||'|sort >$id.log.result
test_compare_result $id
exit $?
