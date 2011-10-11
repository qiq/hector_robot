#!/usr/bin/env bash

for f in text/*; do
#for f in text/ug.bz2; do
	mkdir split 2>/dev/null
	l=`echo $f|sed -e 's/text\/\(.*\)\.bz2/\1/'`
	bzip2 -dc <$f|split -d -l 10000 -a 5 - split/$l.
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

	hector_server -c Language.xml -fb prepare_train_data data/$l.train.list data/${l}_train.hr.gz "$l ?" data/top_words.$l
	hector_server -c Language.xml -fb prepare_test_data data/$l.test.list data/${l}_test.hr.gz

	rm -rf split
done
