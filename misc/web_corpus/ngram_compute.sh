#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N ngram_compute
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=20g -l act_mem_free=20g -l h_vmem=20g -l mnth_free=20g
#$ -p -500

echo $HOSTNAME: $JOB_NAME $@ >>/dev/stderr
export LC_ALL=cs_CZ.utf8
renice 10 $$ >/dev/null

TMP=/mnt/h/tmp

. ../setenv.sh

cp $1 $TMP/$$.tmp
n=`basename $1`
hector_server -c WebCorpus.xml -fb dump $TMP/$$.tmp | ./ngram_compute.pl $TMP/$$.1.out $TMP/$$.2.out $TMP/$$.3.out $TMP/$$.4.out $TMP/$$.5.out &
pid=$!
echo "hector_server: $pid" >>/dev/stderr
wait $pid
echo "result: $?" >>/dev/stderr
for i in 1 2 3 4 5; do
	gzip $TMP/$$.${i}.out
	mv $TMP/$$.${i}.out.gz $2/${i}gms/${n}.gz
done
rm -f $TMP/$$.tmp
