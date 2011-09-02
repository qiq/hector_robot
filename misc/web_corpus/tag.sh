#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N tag_featurama
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=2g -l act_mem_free=2g -l mnth_free=20g
#$ -pe make 4

echo $HOSTNAME: $JOB_NAME $@
#ulimit -c unlimited

TMP=/mnt/h/tmp

. ~/hector_robot/setenv.sh
. ~/hector_robot/misc/setenv.sh

cp deduplicated/$1.hr $TMP
hector_server -c WebCorpus.xml -fb tag4 $1 $TMP/$1.hr $TMP/$1.out
mv $TMP/$1.out tagged/$1.hr
rm -f $TMP/$1.hr

#qsub -q "$MACHINE" -hard -l mem_free=12g -l act_mem_free=12g -cwd -S /bin/bash -N "$JOBNAME" bin/experiment.sh "$@"
#qsub -hard -l mem_free=12g -l act_mem_free=12g -cwd -S /bin/bash -N "$JOBNAME" bin/experiment.sh "$@"
