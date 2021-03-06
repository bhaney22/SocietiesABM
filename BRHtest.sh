#!/bin/bash

# Requires 1 command line argument: 1. Number of runs
# This will single-thread multiple runs of one configuration of Societies
# on the supercomputer cluster.
#
# NOTE: Work with Chris W to figure out how multi-thread societies runs

# Run the following line if format of runlog file changes or to create it anew.
# echo "StartDate,StartTime,ElapsedMinutes,Config,UniqueKey" > /home/brh22/SocietiesABM/_Results/runlog.csv

# Created by: JYC 07.19.2018 
# Last revised: BRH 07.21.2018  
# (note: check out this link for picky rules about math and variables in bash shell scripts:
# http://faculty.salina.k-state.edu/tim/unix_sg/bash/math.html)

cd ~/SocietiesABM/

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

let run_num=$1	#Number of random runs is passed as a parameter when calling this shell script.
let seed=50		#Set the random seed for societies' results to always be the same if all parameters are the same

#Use this type of loop(s) outside the random loop(s) to step through various configuration parameters discretely.
roundsteps="1 2 5"

#Begin run log
	echo "
	************************************************
	Stepping through TRADE_ROUNDS ["$roundsteps"] 
	Sweeping the following parameters randomly randomly "$run_num" times. 
	   TRADE_ATTEMPTS								
	************************************************ 
	"| tee -a _Results/ITest.log

for rounds in $roundsteps 
do
	echo "TRADE ROUNDS = "$rounds""		| tee -a _Results/ITest.log

# Now run a sweep across the more technical and floating point parameters randomly
# The formatting of the iterator (run) as %03g gives it leading zeros: 001,002, etc.
# The leading zeroes helps to keep things in proper order.
for run in $(seq -f "%03g" 1 $1)
do
	echo "RUN "$run" OF "$run_num" " 	| tee -a _Results/ITest.log

# Create config file with input parameters. Default values are found in ./Configs/default.aconf
# To randomize an integer parameter, replace its default value with "$(((RANDOM%50+1)))"
# To randomize an floating point parameter, replace its default value with "$(((RANDOM%5+1)))'.'$(((RANDOM%99+1)))"
echo "
START_DAY = 0
DAY_LENGTH = 600

NUM_DAYS = 300 
NUM_AGENTS = 8 

TRADE_EXISTS = True
DEVICES_EXIST = True
TOOLS_ONLY = False

NUM_RESOURCES = 4 
RESOURCES_IN_TOOL = 3
NUM_DEVICE_COMPONENTS = 4

MENU_SIZE = 4
RES_TRADE_ROUNDS = "$rounds" 
RES_TRADE_ATTEMPTS = "$run"
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
" > Configs/ITest.conf

	
	echo "Selected Config Parameters:" 						| tee -a _Results/ITest.log
	grep NUM_DAYS  					./Configs/ITest.conf | tee -a _Results/ITest.log
	grep NUM_AGENTS 				./Configs/ITest.conf | tee -a _Results/ITest.log
	grep NUM_RESOURCES 				./Configs/ITest.conf | tee -a _Results/ITest.log
	grep RES_TRADE_ROUNDS 			./Configs/ITest.conf | tee -a _Results/ITest.log
	grep RES_TRADE_ATTEMPTS 		./Configs/ITest.conf | tee -a _Results/ITest.log
	
	SECONDS=0
		
### 
	./societies -S $seed -v 1 -p ITest -s _Results/ITest/Rounds_$rounds -d B$UniqueKey -t $run >> _Results/ITest.out
###				
	
	((duration=$SECONDS))
	((totaltime=$totaltime+$duration))
	
	echo "This run COMPLETED in "$duration" seconds" | tee -a _Results/ITest.log
	echo " " | tee -a _Results/ITest.log				

	rm ./Configs/ITest.conf

	
 	((run++)) 
done	#sweeping through random parameters
done  	#looping through stepwise parameters 

EndDay=$(date +%D)
EndTime=$(date +%T)
((totalminutes=$totaltime/60))

echo 'All TEST runs Completed in '$totaltime' seconds or '$totalminutes' minutes on '$EndDay' '$EndTime' ' | tee -a _Results/ITest.log


