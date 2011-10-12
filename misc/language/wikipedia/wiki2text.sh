#!/usr/bin/env bash
#$ -S /bin/bash
#$ -cwd
#$ -N wiki2text
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=2g -l act_mem_free=2g -l mnth_free=20g
#$ -p -500

echo $HOSTNAME: $JOB_NAME $@ >/dev/stderr
renice 10 $$ >/dev/null

TMP=/mnt/h/tmp

. ~/hector_robot/setenv.sh
. ~/hector_robot/misc/setenv.sh

if [ $# -ne 2 ]; then
	echo "usage: wiki2text.sh in out"
	exit
fi
in=$1
out=$2

# clean up wiki page using quick 'n' dirty regexps

bzip2 -dc <$in |sed 's/>/>\n/g' | sed 's/</\n</g' | ./wiki2text.pl | bzip2 -c >$out
