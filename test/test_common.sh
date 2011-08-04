# Include script for test*.sh
# Current directory is writable, directory with this script may be not (VPATH
# builds -- make distcheck).

base=$BASH_SOURCE
base=`dirname "$base"`
base=`readlink -f "$base"`'/..'
test_base=`readlink -f "$base"`

export "PATH=$test_base/test:$PATH"
export "LD_LIBRARY_PATH=$test_base/module/.libs:$test_base/resource/.libs:$test_base/server/.libs:$test_base/perl/.libs:$test_base/python/.libs:$LD_LIBRARY_PATH"
export "PERL5LIB=$test_base/perl:$test_base/module/perl:$test_base/resource/perl:$PERL5LIB"
export "PYTHONPATH=$test_base/python:$test_base/module/python:$test_base/resource/python:$PYTHONPATH"
HECTOR_HOST=localhost:1101

# hector helper functions
. hector_common.sh

function test_server_start {
	id=$1
	server=$2
	if [ -z "$id" -o -z "$server" ]; then
		echo "usage: test_server_start id serverId [args]"
		return
	fi
	shift
	hector_server_shutdown 2>/dev/null
	hector_server_start "$test_base/test/${id}_config.xml" $server $@
	hector_client_wait_dontfail PE_test.run 0
}

function test_server_shutdown {
	hector_server_shutdown
}

function test_server_batch {
	id=$1
	server=$2
	if [ -z "$id" -o -z "$server" ]; then
		echo "usage: test_server_batch id serverId"
		return
	fi
	shift; shift
	hector_server_start "$test_base/test/${id}_config.xml" -f -b $server $@
}

function test_compare_result {
	id=$1
	if [ -z "$id" ]; then
		echo "usage: test_compare_result id"
		return
	fi
	diff -u "$test_base/test/${id}.log.correct" "${id}.log.result"
}
