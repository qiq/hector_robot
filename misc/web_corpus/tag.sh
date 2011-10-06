#!/bin/bash
#$ -S /bin/bash
#$ -cwd
#$ -N tag_featurama
#$ -o log/$JOB_NAME.$JOB_ID.o
#$ -e log/$JOB_NAME.$JOB_ID.e
#$ -hard -l mem_free=2g -l act_mem_free=2g -l mnth_free=20g
#$ -p -500
####$ -pe make 4

echo $HOSTNAME: $JOB_NAME $@ >/dev/stderr
renice 10 $$
#ulimit -c unlimited

TMP=/mnt/h/tmp

. ~/hector_robot/setenv.sh
. ~/hector_robot/misc/setenv.sh

cp $1 $TMP/$$.tmp
hector_server -c WebCorpus.xml -fb tag $TMP/$$.tmp $TMP/$$.tmp.out $2.log
mv $TMP/$$.tmp.out $2
rm -f $TMP/$$.tmp

#qsub -q "$MACHINE" -hard -l mem_free=12g -l act_mem_free=12g -cwd -S /bin/bash -N "$JOBNAME" bin/experiment.sh "$@"
#qsub -hard -l mem_free=12g -l act_mem_free=12g -cwd -S /bin/bash -N "$JOBNAME" bin/experiment.sh "$@"
