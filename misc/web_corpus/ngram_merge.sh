#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N ngram_merge
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=20g -l act_mem_free=20g -l h_vmem=20g -l mnth_free=20g
#$ -p -500

echo $HOSTNAME: $JOB_NAME $@ >>/dev/stderr
export LC_ALL=cs_CZ.utf8
renice 10 $$ >/dev/null

TMP=/mnt/h/tmp

. ../setenv.sh

ls ngram/${2}gms/finished_${1}.hr.gz.*|./ngram_merge.pl|gzip -c >${TMP}/$$.tmp;

mv ${TMP}/$$.tmp ngram/${2}gms/finished_${1}.gz
