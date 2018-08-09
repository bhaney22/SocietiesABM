#!/bin/bash
# # # # # # # # # # # # # # # #

# Set number of of nodes - max is 20!
#SBATCH -N 1
# Set number of cpus-per-task - max is 16!
#SBATCH -c 1
#Set working directory
#SBATCH -D /home/brh22/SocietiesABM/Slurm.out
# Set job name
#SBATCH -J ABMSweep
# Set max execution time, ex:  minutes  minutes:seconds  hours:minutes:seconds 
#SBATCH -t 10:00:00

# This will multi-thread multiple runs of one configuration of Societies
# on the supercomputer cluster.
#
# Last revised: BRH 07.27.2018  
# (note: check out this link for picky rules about math and variables in bash shell scripts:
# http://faculty.salina.k-state.edu/tim/unix_sg/bash/math.html)

cd ~/SocietiesABM/
echo "Passed arguments:"
echo "1. "$1""
echo "2. UKEY "$2""
echo "3. CONF "$3""
echo "4. RUN  "$4""
echo "5. SIMNAME "$5""
echo "6. Optional Seed value: "$6""

# Start the clock for this run of societies.
SECONDS=0 

#################################################################
#Run Societies
#Pass the entire .societies command in quotes as one parameter
if [ "$6" != "" ]; then   #set optional seed for static run
	srun $1 -S $6
else
	srun $1
fi

# Stop the clock after societies finishes
((duration=$SECONDS))
((duration_min=$duration/60))
echo ""$2","$3","$4",$duration,$duration_min" >> _Results/"$5"_runtime.csv