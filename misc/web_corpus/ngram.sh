#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N ngram
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=5g -l act_mem_free=5g -l h_vmem=5g -l mnth_free=10g
#$ -p -500

echo $HOSTNAME: $JOB_NAME $@ >>/dev/stderr
renice 10 $$ >/dev/null

TMP=/mnt/h/tmp

. ../setenv.sh

cp $1 $TMP/$$.tmp
hector_server -c WebCorpus.xml -fb dump $TMP/$$.tmp | ./ngram_compute.pl $TMP/$$.1.out $TMP/$$.2.out $TMP/$$.3.out $TMP/$$.4.out $TMP/$$.5.out &
pid=$!
echo "hector_server: $pid" >>/dev/stderr
wait $pid
echo "result: $?" >>/dev/stderr
for i in 1 2 3 4 5; do
	mv $TMP/$$.${i}.out $1.${i}gr
done
rm -f $TMP/$$.tmp
