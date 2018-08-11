#!/bin/bash
# # # # # # # # # # # # # # # #

# Set number of of nodes - max is 20!
#SBATCH -N 1
# Set number of cpus-per-task - max is 16!
#SBATCH -c 1
# Set max execution time, ex:  minutes  minutes:seconds  hours:minutes:seconds 
# If one run of societies takes longer than say, 1 hour (1:00:00), something may be up, so cancel it.
# This can be adjusted if needed. -- BRH 08.09.2018
#SBATCH --time 8:00:00
#Set working directory
#SBATCH -D /home/brh22/SocietiesABM/_Slurm.out
# Pad the job names with leading zeroes to make them sort well. 
# And, separate the error files from the out files.
#SBATCH --output=slurm-J%7j.out
#SBATCH  --error=slurm-J%7j.err

# This will allow multiple runs of Societies to run at the same time.
# on the supercomputer cluster.
#
# Last revised: BRH 07.27.2018  
# (note: check out this link for picky rules about math and variables in bash shell scripts:
# http://faculty.salina.k-state.edu/tim/unix_sg/bash/math.html)

cd ~/SocietiesABM/

# NOTE: the number of digits in the following statement must match the number in the above SBATCH commands for naming .out and .err files.
jid=`printf "%07d" $SLURM_JOB_ID`

echo "J"$jid"" >> _Results/"$5".jobs

# Start the clock for this run of societies.
SECONDS=0 
StartDay=$(date +%D)
StartTime=$(date +%T)
#################################################################
#Run Societies
#Pass the entire .societies command in quotes as one parameter
if [ "$6" != "" ]; then   #set optional seed for static run
	srun $1 -S $6
else
	srun $1
fi

# Stop the clock after societies finishes
EndTime=$(date +%T)
((duration=$SECONDS))
((duration_min=$duration/60))

echo "J"$jid","$2","$3","$4",$StartDay,$StartTime,$EndTime,$duration,$duration_min" >> _Results/"$5"_runtime.csv
