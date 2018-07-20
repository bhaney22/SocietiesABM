#!/bin/bash
# Shell script: run1config.pbs
# Requires 2 command line arguments: 1. Config name  2. Number of runs
# This will single-thread multiple runs of one configuration of Societies 
# on the supercomputer cluster.
# Created by: JYC 07.19.2018 
# Last revised: JYC 07.20.2018

cd /home/brh22/SocietiesABM/

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
run=0
run_num=2

echo "Unique Key: $UniqueKey"
echo "Start Date: $StartDay $StartTime"


until [ $run -eq $run_num ]
do
	echo "		UniqueKey_run_num: $UniqueKey"_"$run" 

	sed -e 's/NUM_AGENTS = 1/NUM_AGENTS = '$(((RANDOM%50+1)))'/g' \
	-e 's/NUM_RESOURCES = 1/NUM_RESOURCES = '$(((RANDOM%20+1)))'/g' \
	-e 's/NUM_DAYS = 100/NUM_DAYS = '$(((RANDOM%100+20)))'/g' \
	-e 's/TOOL_FACTOR = 3.0/TOOL_FACTOR = '$(((RANDOM%10+1)))'.'$(((RANDOM%100)))'/g' \
	-e 's/TOOL_LIFETIME = 150.0/TOOL_LIFETIME = '$(((RANDOM%500+6)))'.'$(((RANDOM%100+1)))'/g' \
	-e 's/DAILY_EXP_PENALTY = 3.0/DAILY_EXP_PENALTY = '$(((RANDOM%5+1)))'.'$(((RANDOM%99+1)))'/g' \
	./Configs/JYC_test.conf > ./Configs/JYCtemp.conf

	./societies -v 1 -p JYCtemp -s _Results/ITest$run > ITest$run.log
	
	rm ./Configs/JYCtemp.conf
 	echo 'run '$run 'done'
 	((run++))
done

duration=$SECONDS
EndDay=$(date +%D)
EndTime=$(date +%T)

echo 'Run Complete'

# Uncomment the following line if format of runlog file changes.
# echo "StartDate,StartTime,EndDate,EndTime, ElapsedSeconds,Config,Num_Runs,UniqueKey" > /home/brh22/societies2017/_Results/runlog.csv
echo "$StartDay,$StartTime,$EndDay,$EndTime,$duration,JYC_test,$run_num,B$UniqueKey" >> /home/brh22/SocietiesABM/_Results/runlog.csv
