#!/bin/bash

# Requires 1 command line argument: 1. Number of runs
# This will single-thread multiple runs of one configuration of Societies
# on the supercomputer cluster.
#
# NOTE: To multi-thread, run societies in background (I think.) 

# Run the following line if format of runlog file changes or to create it anew.
# echo "StartDate,StartTime,ElapsedMinutes,Config,UniqueKey" > /home/brh22/SocietiesABM/_Results/runlog.csv

# Created by: JYC 07.19.2018 
# Last revised: BRH 07.21.2018  (note: check out this link for picky rules
# about math and variables in bash shell scripts:
# http://faculty.salina.k-state.edu/tim/unix_sg/bash/math.html)

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

rm -r _Results/ITest*

UniqueKey=$(date +%d%m%Y%H%M%S%N)
StartDay=$(date +%D)
StartTime=$(date +%T)
((totaltime=0))

let run_num=$1

echo ""$run_num" Test Runs Begin: "$StartDay" "$StartTime" " |tee _Results/ITest.log
echo "Unique Key: "$UniqueKey"  "| tee -a _Results/ITest.log

# The following formatting allows run to be 01,02,...10,11, etc. the leading zero helps to keep things in proper order.
for run in $(seq -f "%03g" 1 $run_num)
do
	echo "*****************" | tee -a _Results/ITest.log
	echo " RUN "$run" OF "$run_num" " | tee -a _Results/ITest.log
	echo "*****************" | tee -a _Results/ITest.log
	echo " " | tee -a _Results/ITest.log
	
	echo "Config Parameters:" | tee -a _Results/ITest.log
	
# Create config file with input parameters. Default values are found in ./Configs/default.aconf
# To randomize an integer parameter, replace its default value with "$(((RANDOM%50+1)))"
# To randomize an floating point parameter, replace its default value with "$(((RANDOM%5+1)))'.'$(((RANDOM%99+1)))"
echo "
START_DAY = 0
DAY_LENGTH = 600

NUM_DAYS = 100
NUM_AGENTS = 8

TRADE_EXISTS = True
DEVICES_EXIST = True
TOOLS_ONLY = False

NUM_RESOURCES = 4
RESOURCES_IN_TOOL = 3
NUM_DEVICE_COMPONENTS = 4

MENU_SIZE = 4
RES_TRADE_ROUNDS = 1
RES_TRADE_ATTEMPTS = 1
DEVICE_TRADE_ROUNDS = 1
DEVICE_TRADE_ATTEMPTS = 1
DEVICE_TRADE_MEMORY_LENGTH = 5
DEVICE_PRODUCTION_MEMORY_LENGTH = 5

NUM_AGENT_GROUPS = 1
MIN_RES_UTIL = 1.0
MAX_RES_EFFORT = 9.0
MIN_RES_EFFORT = 3.0
MAX_DEVICE_EFFORT = 27.0
MIN_DEVICE_EFFORT = 9.0

TOOL_FACTOR = 3.0
TOOL_LIFETIME = 150.0

MACHINE_FACTOR = 9.0
MACHINE_LIFETIME = 300.0

FACTORY_FACTOR = 27.0
FACTORY_LIFETIME = 600.0

INDUSTRY_FACTOR = 81.0
INDUSTRY_LIFETIME = 1200.0

MAX_RES_EXPERIENCE = 600.0
INVENTOR_DEVICE_EXPERIENCE = 6
MAX_DEVICE_EXPERIENCE = 40.0

MIN_HELD_DEVICE_EXPERIENCE = 2
DAYS_OF_DEVICE_TO_HOLD = 2

DAILY_EXP_PENALTY = 3.0
DAILY_DEVICE_DECAY = 3
MIN_RES_HELD_FOR_DEVICE_CONSIDERATION = 3

TRADE_EPSILON = .1
TOOL_PROBABILITY_FACTOR = .000075
DEVICE_PROBABILITY_FACTOR = .00125
PRODUCTION_EPSILON = 0

DEV_MACHINE_FACTOR = 0
DEV_MACHINE_LIFETIME = 0
DEV_FACTORY_FACTOR = 0
DEV_FACTORY_LIFETIME = 0
MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION = 5

OTHER_MARKETS = False 

" > ./Configs/ITest$run.conf
	
	echo " " | tee -a _Results/ITest.log
	echo "Input Parameters:" | tee -a _Results/ITest.log
	grep NUM_ ./Configs/ITest$run.conf | tee -a _Results/ITest.log
	grep FACTOR ./Configs/ITest$run.conf | tee -a _Results/ITest.log
	grep LIFETIME ./Configs/ITest$run.conf | tee -a _Results/ITest.log
	
	SECONDS=0
		
	echo "=== BEGIN SOCIETIES RUN "$run" SCREEN CAPTURE OUTPUT ===" | tee -a _Results/ITest.log
### 
				./societies -v 0 -p ITest$run -s _Results/ITest/Run$run -d B$UniqueKey -t $run | tee -a _Results/ITest.log
###				
	echo "=== END SOCIETIES "$run" SCREEN CAPTURE OUTPUT ===" | tee -a _Results/ITest.log
	echo " " | tee -a _Results/ITest.log
	
	((duration=$SECONDS))
	((totaltime=$totaltime+$duration))
	
	echo "Run "$run" of "$run_num" COMPLETED in "$duration" seconds" | tee -a _Results/ITest.log
	echo " " | tee -a _Results/ITest.log				

	rm ./Configs/ITest$run.conf
	
 	((run++))
done

EndDay=$(date +%D)
EndTime=$(date +%T)
((totalminutes=$totaltime/60))

echo ''$run_num' Test Runs Completed in '$totaltime' seconds or '$totalminutes' minutes on '$EndDay' '$EndTime' ' | tee -a _Results/ITest.log


