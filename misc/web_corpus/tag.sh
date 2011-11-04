#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N tag_featurama
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=2g -l act_mem_free=2g -l mnth_free=20g
#$ -p -500

echo $HOSTNAME: $JOB_NAME $@ >>/dev/stderr
renice 10 $$ >/dev/null

TMP=/mnt/h/tmp

. ../setenv.sh

cp $1 $TMP/$$.tmp
hector_server -c WebCorpus.xml -fb tag $TMP/$$.tmp $TMP/$$.tmp.out $2.log
mv $TMP/$$.tmp.out $2
rm -f $TMP/$$.tmp
