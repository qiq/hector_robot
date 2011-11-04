#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N parse_mst
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=10g -l act_mem_free=10g -l h_vmem=10g -l mnth_free=20g
#$ -p -500

echo $HOSTNAME: $JOB_NAME $@ >>/dev/stderr
renice 10 $$ >/dev/null

TMP=/mnt/h/tmp

. ../setenv.sh

cp $1 $TMP/$$.tmp
hector_server -c WebCorpus.xml -fb parse $TMP/$$.tmp $TMP/$$.tmp.out $2.log &
pid=$!
echo "hector_server: $pid" >>/dev/stderr
wait $pid
echo "result: $?" >>/dev/stderr
mv $TMP/$$.tmp.out $2
rm -f $TMP/$$.tmp
