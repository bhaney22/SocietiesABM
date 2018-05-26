#!/bin/bash
#
# Requires 2 command line arguments: 1. Config name  2. Number of runs
# This will single-thread multiple runs of one configuration of Societies 
# on the supercomputer cluster.
# Created by: BRH 03.17.2017 
# Last revised: BRH 05.26.2018
# to run this script, use the command line: 
# 			sbatch run_societies.slurm config numruns

#

# Societies command line arguments Naming conventions:
# 1. -p Name of the configuration (without Configs/ path and without .conf extention) ex. T4_100_100
#
# 2. -s Eventually, no need for arguments. Just simply switches on the saving function. 
#       The folder name can be automatically created as "_Results/configuration name/UniqueID"
#
# 3. -t tracks the run number
# 4. -d provides a unique batch ID that will be used to join the output measures
#       with the config file dimensions (*New* as of v1.5 of SocietiesABM)
# BRH 2017.10.02 changed the verbose level to 3 for testing purposes.

UniqueKey=$(date +%d%m%Y%H%M%S%N)
StartDay=$(date +%D)
StartTime=$(date +%T)
SECONDS=0

echo "Unique Key: $UniqueKey"
echo "Start Date: $StartDay $StartTime"

cd ~/SocietiesABM/

for run_num in $(seq -f "%03g" 1 $2)
do
	echo "     UniqueKey_run_num: $UniqueKey"_"$run_num"
	rm ~/SocietiesABM/_Results/$1 -r
	srun ./societies -v 1 -p $1 -s _Results/$1  -d B$UniqueKey -t $run_num
done

duration=$SECONDS
EndDay=$(date +%D)
EndTime=$(date +%T)

# Uncomment the following line if format of runlog file changes.
# echo "StartDate,StartTime,EndDate,EndTime, ElapsedSeconds,Config,Num_Runs,UniqueKey" > /home/brh22/societies2017/_Results/runlog.csv
echo "$StartDay,$StartTime,$EndDay,$EndTime,$duration,$1,$2,B$UniqueKey" >> ~/SocietiesABM/_Results/runlog.csv
