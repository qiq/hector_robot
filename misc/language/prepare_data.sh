#!/usr/bin/env bash

#for l in af ar az be bg bn ca cs cy da de el en es et eu fa fi fr gl he hi hr hu id is it ka lt lv mk ml ms nl pl pt ro ru sk sl sq sr sv ta te th tr uk zh; do
for l in cs de en sk; do
	mkdir split 2>/dev/null
	bzip2 -dc <text/$l.bz2|split -d -l 10000 -a 5 - split/$l.
	find split -type f >data/$l.list
	n=`wc -l data/$l.list|cut -f1 -d' '`
	if [ $n -gt 100 ]; then
		te=100
	else
		te=`echo "$n/5"|bc`
	fi
	tr=`echo "$n-$te"|bc`
	head -n $tr data/$l.list >data/$l.train.list
	tail -n $te data/$l.list >data/$l.test.list

	hector_server -c Language.xml -fb prepare_train_data data/$l.train.list data/${l}_train.hr.gz "$l ?" data/top_words.$l 100000
	hector_server -c Language.xml -fb prepare_test_data data/$l.test.list data/${l}_test.hr.gz

	rm -rf split
done
