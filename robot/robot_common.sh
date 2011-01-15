base=..
export LD_LIBRARY_PATH=$base/resources:$base/modules:$base/servers:$base/perl/.libs:$LD_LIBRARY_PATH
export PERL5LIB=$base/perl:$base/modules/perl:$PERL5LIB

. hector_common.sh

function hector_client_command {
	echo -ne "$@" | ( hector_client $HECTOR_HOST || exit )
}

function hector_client_wait_all {
	while true; do
		i=0
		q=""
		r=""
		for a in $@; do
			if [ `expr $i % 2` = 0 ]; then
				q="${q}get $a\n"
			else
				r="$r $a"
			fi
			i=`expr $i + 1`
		done
		r2=`hector_client_command $q | sed -e 's/.*= //'`
		if [ "`echo $r`" = "`echo $r2`" ]; then
			break;
		fi
		echo -ne "\033[100D\033[K";
		echo -n $r2;
		bash -c "$HECTOR_SLEEP_COMMAND"
	done
}
