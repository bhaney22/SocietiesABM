#!/bin/bash

############################################################################
# This script takes two commandline arguments:
# 1. Number of days (default = 100)
# 2. Number of random runs (default = 5) 
#

# This will single-thread multiple runs of one configuration of Societies
# on the supercomputer cluster.
#
# Last revised: BRH 07.27.2018  
# (note: check out this link for picky rules about math and variables in bash shell scripts:
# http://faculty.salina.k-state.edu/tim/unix_sg/bash/math.html)

cd ~/SocietiesABM/

#############################################################################################################
# Control amount of random variation by uncommenting the appropriate option:
# Option 1: The same configuration will have random results. 
# Option 2: No variation - used for testing. (comment out Option 1 and uncomment out Option 2)
#############################################################################################################

#############################################################################################################
# Option 1
##############################################################################################################
# The number of random runs can be passed as the first arg of the command line, otherwise only 2 are run.
num_random_runs=${2:-005}   
seed=" "

#############################################################################################################
# Option 2
#############################################################################################################
#num_random_runs=001
#seed="-S 50"	#Set the random seed so that societies' results will be the same if all parameters are the same

#############################################################################
# Adjust the number of resources used in a tool, if needed. 
# If RESOURCES_IN_TOOL is one of the sweep variables, use runSweepResTool.sh.
#############################################################################
let resources_in_tool=3                          # default value = 5 (or 1 less than NUM_RESOURCES if NUM_RESOURCES < 6 
let num_device_components=$resources_in_tool+1   # The default is for device components to be 1 more than tool components

#################################################################################################################
# BEGIN SPECIFIC CODE FOR INTEGRATION TEST.
# ITEST_devices:
# 	This Integration test randomizes the integer parameter values related to devices. The following comments
#	show the original (default) values and the range of values from which a random number is chosen in each run.
# 	The actual changes are in the config file below.
#################################################################################################################
sim_name=ITEST_devices

#TOOL_FACTOR = 3.0   "$(((RANDOM%5+1)))'.'$(((RANDOM%99+1)))"
#MACHINE_FACTOR = 9.0   "$(((RANDOM%18+10)))'.'$(((RANDOM%99+1)))"
#FACTORY_FACTOR = 27.0   "$(((RANDOM%55+30)))'.'$(((RANDOM%99+1)))"
#INDUSTRY_FACTOR = 81.0   "$(((RANDOM%200+70)))'.'$(((RANDOM%99+1)))"
#TOOL_LIFETIME = 150.0   "$(((RANDOM%180+120)))'.'$(((RANDOM%99+1)))"
#MACHINE_LIFETIME = 300.0   "$(((RANDOM%400+250)))'.'$(((RANDOM%99+1)))"
#FACTORY_LIFETIME = 600.0   "$(((RANDOM%800+550)))'.'$(((RANDOM%99+1)))"
#INDUSTRY_LIFETIME = "1200.0   $(((RANDOM%1500+1100)))'.'$(((RANDOM%99+1)))"


#Begin run log
StartDay=$(date +%D)
StartTime=$(date +%T)
((totaltime=0))
echo "
*****************************************************************************
* "$sim_name" 
*        For #Random Runs = "$num_random_runs"
*
* BEGAN at "$StartDay" "$StartTime".
*****************************************************************************
"| tee _Results/"$sim_name".log

# Begin random runs using the same configuration file.
# The formatting of the iterator (run) as %03g gives it leading zeros: 001,002, etc.
run_num=$num_random_runs   
for run in $(seq -f "%03g" 1 $run_num)
do
((configtime=0))
#################################################################################
# Create config file with input parameters. 
# Default values are found in ./Configs/default.conf
# Name the config file based on run_number
#################################################################################
config="$sim_name"_"$run"

#################################################################################
# Create a UniqueKey to save the parameters for each unique config of parameters
#################################################################################
UniqueKey=$(date +%d%m%Y%H%M%S%N)

echo "
*****************************************************************************
"CONFIG = "$config"  Unique Key = "$UniqueKey""
*****************************************************************************
" | tee -a _Results/"$sim_name".log


echo "
START_DAY = 0
DAY_LENGTH = 600

TRADE_EXISTS = True
DEVICES_EXIST = True
TOOLS_ONLY = False

NUM_DAYS = "${1:-100}  "
NUM_AGENTS = 8
NUM_RESOURCES = 4
RESOURCES_IN_TOOL = "$resources_in_tool"
NUM_DEVICE_COMPONENTS = "$num_device_components"

MENU_SIZE = 4
RES_TRADE_ROUNDS = 12
RES_TRADE_ATTEMPTS = 8
DEVICE_TRADE_ROUNDS = 12 
DEVICE_TRADE_ATTEMPTS = 4
DEVICE_TRADE_MEMORY_LENGTH = 5
DEVICE_PRODUCTION_MEMORY_LENGTH = 5

NUM_AGENT_GROUPS = 1
MIN_RES_UTIL = 1.0
MAX_RES_EFFORT = 9.0
MIN_RES_EFFORT = 3.0
MAX_DEVICE_EFFORT = 27.0
MIN_DEVICE_EFFORT = 9.0

TOOL_FACTOR = "$(((RANDOM%5+1)))'.'$(((RANDOM%99+1)))"
MACHINE_FACTOR = "$(((RANDOM%18+10)))'.'$(((RANDOM%99+1)))"
FACTORY_FACTOR = "$(((RANDOM%55+30)))'.'$(((RANDOM%99+1)))"
INDUSTRY_FACTOR = "$(((RANDOM%200+70)))'.'$(((RANDOM%99+1)))"

TOOL_LIFETIME = "$(((RANDOM%180+120)))'.'$(((RANDOM%99+1)))"
MACHINE_LIFETIME = "$(((RANDOM%400+250)))'.'$(((RANDOM%99+1)))"
FACTORY_LIFETIME = "$(((RANDOM%800+550)))'.'$(((RANDOM%99+1)))"
INDUSTRY_LIFETIME = "$(((RANDOM%1500+1100)))'.'$(((RANDOM%99+1)))"

MAX_RES_EXPERIENCE = 600.0
MAX_DEVICE_EXPERIENCE = 40.0
INVENTOR_DEVICE_EXPERIENCE = 6

MIN_HELD_DEVICE_EXPERIENCE = 2
DAYS_OF_DEVICE_TO_HOLD = 2

DAILY_EXP_PENALTY = 3.0
DAILY_DEVICE_DECAY = 3
MIN_RES_HELD_FOR_DEVICE_CONSIDERATION = 3

TRADE_EPSILON = .1
TOOL_PROBABILITY_FACTOR = .000075
DEVICE_PROBABILITY_FACTOR = .00125
PRODUCTION_EPSILON = 0

DEV_MACHINE_FACTOR = .1
DEV_MACHINE_LIFETIME = 1
DEV_FACTORY_FACTOR = .1
DEV_FACTORY_LIFETIME = 1
MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION = 10000

OTHER_MARKETS = False 
" > Configs/"$config".conf

######################################################################################
# Print the random parameter values first, then
# Save the entire configuration to the log file.
######################################################################################
grep FACTOR Configs/"$config".conf |tee -a _Results/"$sim_name".log
grep LIFETIME Configs/"$config".conf |tee -a _Results/"$sim_name".log
cat Configs/"$config".conf >> _Results/"$sim_name".log

### SOCIETIES COMMAND-LINE ARGUMENTS:
# 1. -p Name of the configuration (without Configs/ path and without .conf extention)
#
# 2. -s Name of the save folder. 
#
# 3. -t tracks sequential run numbers in the results directories
#
# 4. -d provides a unique batch ID that will be used to join the output measures
#       with the config file dimensions (societies v1.5 and higher)
# 5. -v is the level of log messages to be printed to the log file. 0 is none.

# Start the clock for this run of societies.
SECONDS=0 
 
# The & before the re-direction (">>") will send error messages to the stdout file as well.
./societies -v 1 -p "$config" -s _Results/"$sim_name"/"$config" -d B"$UniqueKey" -t 001 "$seed" 1>> _Results/"$sim_name"_run.log 

# Stop the clock after societies finishes
((duration=$SECONDS))
((duration_min=$duration/60))

# Add this run time to the total time of the simulation.
((totaltime=$totaltime+$duration))
    	
echo "$config Duration: $duration seconds ($duration_min minutes)" | tee -a _Results/"$sim_name"_runtime

rm ./Configs/"$config".conf 
rm ./Configs/"$config"_AgentValues.aconf 

done    #Done with loop over one run of a randomly generated configuration

EndDay=$(date +%D)
EndTime=$(date +%T)
((totalminutes=$totaltime/60))

echo "

*****************************************************************************
* "$sim_name":
*        For #Random Runs = "$num_random_runs"
*
* BEGAN at "$StartDay" "$StartTime"
* ENDED at "$EndDay" "$EndTime" 
* Total Duration: $totaltime seconds ($totalminutes minutes)
*
*****************************************************************************
Runtime statistics:
" | tee -a _Results/"$sim_name".log

cat _Results/"$sim_name"_runtime | tee -a _Results/"$sim_name".log
rm  _Results/"$sim_name"_runtime


#########################################################################################
# Concatenate and save all of the output files.
#
# FYI: the head command saves the header row from the first individual long_output file.
# FYI: the grep -v command strips out all of the header rows (starts with UniqueKey)
#      from the individual long_output files. BRH 3.20.2017
#########################################################################################
cd ~/SocietiesABM/_Results/
cat ./$sim_name/*/*/IOMatrix.csv | head -n 1 > temp1
cat ./$sim_name/*/*/IOMatrix.csv  | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$sim_name/IOMatrixAll.csv
rm  temp1 temp2

cat ./$sim_name/*/*/DeviceRecipes.csv | head -n 1 > temp1
cat ./$sim_name/*/*/DeviceRecipes.csv  | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$sim_name/DeviceRecipes.csv
rm  temp1 temp2

cat ./$sim_name/*/*/long_output.csv | head -n 1 > temp1
cat ./$sim_name/*/*/long_output.csv | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$sim_name/long_output_all.csv
rm  temp1 temp2

cat ./$sim_name/*/*/unitsGathered.csv | head -n 1 > temp1
cat ./$sim_name/*/*/unitsGathered.csv | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$sim_name/unitsGathered_all.csv
rm  temp1 temp2

# Print out a summary of resources gathered.
echo "
Units Gathered in all runs:
" | tee -a $sim_name.log
column -s , -t < ./$sim_name/unitsGathered_all.csv | tee -a $sim_name.log 

#
# Save all of the relevent UniqueKey entries with the output.
#
################################################################################################
# Create the file to record all of the configuration parameters in a database.
# This only needs to be run if this file has been deleted or to start a fresh file.
################################################################################################
echo "UniqueKey,ConfigName,Group_Num,NUM_AGENTS,NUM_RESOURCES,NUM_AGENT_GROUPS,TRADE_EXISTS,DEVICES_EXIST,TOOLS_ONLY, \
NUM_DAYS,START_DAY,DAY_LENGTH,RES_TRADE_ROUNDS,RES_TRADE_ATTEMPTS,DEVICE_TRADE_ROUNDS,DEVICE_TRADE_ATTEMPTS, \
MENU_SIZE,DEVICE_TRADE_MEMORY_LENGTH,DEVICE_PRODUCTION_MEMORY_LENGTH,MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION, \
MIN_RES_HELD_FOR_DEVICE_CONSIDERATION,DAILY_EXP_PENALTY,PRODUCTION_EPSILON,RESOURCES_IN_TOOL,MAX_RES_EXPERIENCE, \
INVENTOR_DEVICE_EXPERIENCE,NUM_DEVICE_COMPONENTS,MAX_DEVICE_EXPERIENCE,DAILY_DEVICE_DECAY,MIN_HELD_DEVICE_EXPERIENCE, \
MAX_RES_EFFORT,MIN_RES_EFFORT,MAX_DEVICE_EFFORT,MIN_DEVICE_EFFORT,MIN_RES_UTIL,TRADE_EPSILON,TOOL_PROBABILITY_FACTOR, \
DEVICE_PROBABILITY_FACTOR,TOOL_FACTOR,TOOL_LIFETIME,MACHINE_FACTOR,MACHINE_LIFETIME,FACTORY_FACTOR,FACTORY_LIFETIME, \
INDUSTRY_FACTOR,INDUSTRY_LIFETIME,DEV_MACHINE_FACTOR,DEV_MACHINE_LIFETIME,DEV_FACTORY_FACTOR,DEV_FACTORY_LIFETIME, \
DAYS_OF_DEVICE_TO_HOLD,REMOVE_RES,RES_TO_REMOVE,REMOVE_RES_DAY,ELIMINATE_RESERVES,REMOVE_AGENT,AGENT_TO_REMOVE, \
REMOVE_AGENT_DAY,END_SAVE,SAVE_FOLDER,SAVE_DAY_STATUS,DAY_STATUS_SAVE_FOLDER,DAY_FOR_SAVE,DAY_STATUS_LOAD_FOLDER, \
SIM_NAME,SIM_SAVE_FOLDER,SAVE_TRADES,PARALLEL_TRADES" > ~/SocietiesABM/_Results/ukey_header.csv

grep $sim_name\/ UniqueKeyFile.csv > temp2
cat ~/SocietiesABM/_Results/ukey_header.csv temp2 > ./$sim_name/UniqueKeyFile.csv
rm temp2