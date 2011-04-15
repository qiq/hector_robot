#!/bin/bash

. test_common.sh

id=test2
pkill dns_server.pl; ( dns_server.pl 5354 & )
test_server_batch $id
pkill dns_server.pl

grep "M_dump[0-9]\[[0-9]\+\]: " $id.log|sed -e 's|M_dump[0-9]\[[0-9]\+\]: \(\[WR .*\)|\1|'|sed -e 's|, ip expire:.*||'|sort >$id.log.result
test_compare_result $id
exit $?
