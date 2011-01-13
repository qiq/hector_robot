#!/usr/bin/env bash
# recursively list hostnames in a domain using AXFR

if [ -z "$1" ]; then
	echo axfr.sh domain
	exit 1;
fi

TMP=/tmp

function list_domain {
	ns=`dig +short "$1" NS|head -n1`
	if [ -z "$ns" ]; then
		return;	# no NS
	fi
	dig "$1" @$ns axfr|grep -v "^$"|grep -v "^;"|while read a b c d e; do
		a=`echo $a|sed 's/\.$//'`
		case $d in
		A) echo $a ;;
		CNAME) echo $a ;;
		NS) if [ $a != $1 ]; then list_domain $a; fi ;;
		*) ;;
		esac
	done
}

list_domain $1
