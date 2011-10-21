#!/usr/bin/env bash
#$ -S /bin/bash
#$ -cwd
#$ -N prepare_data
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=2g -l act_mem_free=2g -l mnth_free=20g
#$ -p -500

echo $HOSTNAME: $JOB_NAME $@ >/dev/stderr
renice 10 $$ >/dev/null

TMP=/mnt/h/tmp

. ~/hector_robot/setenv.sh
. ~/hector_robot/misc/setenv.sh

function prepare {
	l=$1
	mkdir $TMP/split.$$ 2>/dev/null
	bzip2 -dc <wiki_text/$l.bz2|split -d -l 10000 -a 5 - $TMP/split.$$/$l.
	find $TMP/split.$$ -type f >language_data/$l.list
	n=`wc -l language_data/$l.list|cut -f1 -d' '`
	if [ $n -gt 100 ]; then
		te=50
	else
		te=`echo "$n/5"|bc`
	fi
	tr=`echo "$n-$te"|bc`
	head -n $tr language_data/$l.list >language_data/$l.train.list
	tail -n $te language_data/$l.list >language_data/$l.test.list

	hector_server -c Language.xml -fb prepare_train_data language_data/$l.train.list language_data/${l}_train.hr.gz "$l ?" language_data/top_words.$l 100000
	hector_server -c Language.xml -fb prepare_test_data language_data/$l.test.list language_data/${l}_test.hr.gz

	rm -rf $TMP/split.$$
}

if [ "$1" != "" ]; then
	prepare $1
else
	for l in af ar az be bg bn ca cs cy da de el en es et eu fa fi fr gl he hi hr hu id is it ka lt lv mk ml ms nl pl pt ro ru sk sl sq sr sv ta te th tr uk zh; do
		prepare $l
	done
fi
