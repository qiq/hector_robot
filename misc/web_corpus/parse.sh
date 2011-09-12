#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N parse_mst
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=8g -l act_mem_free=8g -l mnth_free=20g
#$ -p -500
##$ -pe make 4

echo $HOSTNAME: $JOB_NAME $@ >/dev/stderr
#ulimit -c unlimited

TMP=/mnt/h/tmp

. ~/hector_robot/setenv.sh
. ~/hector_robot/misc/setenv.sh

cp tagged/$1 $TMP
hector_server -c WebCorpus.xml -fb parse $1 $TMP/$1 $TMP/$1.out
mv $TMP/$1.out parsed/$1
rm -f $TMP/$1

#qsub -q "$MACHINE" -hard -l mem_free=12g -l act_mem_free=12g -cwd -S /bin/bash -N "$JOBNAME" bin/experiment.sh "$@"
#qsub -hard -l mem_free=12g -l act_mem_free=12g -cwd -S /bin/bash -N "$JOBNAME" bin/experiment.sh "$@"