#!/bin/bash

############################################################################
# This script takes two commandline arguments:
# 1. Number of days (default = 100)
# 1. Number of random runs (default = 5) 
#

# This will single-thread multiple runs of one configuration of Societies
# on the supercomputer cluster.
#
# Last revised: BRH 07.27.2018  
# (note: check out this link for picky rules about math and variables in bash shell scripts:
# http://faculty.salina.k-state.edu/tim/unix_sg/bash/math.html)

cd ~/SocietiesABM/

############################################################################
# Set up the parameter sweeps. 
# 1. Choose two parameters to sweep over.
# 2. Choose the list of values for each of them.
#
# 3. Scroll down to the config file below
#    a. replace the first parameter default value with "$i"
#    b. replace the second parameter default value with "$j"
#############################################################################
parm_name1=TRADE_ROUNDS
list_of_values1="05"

parm_name2=TRADE_ATTEMPTS
list_of_values2="5"

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

sim_name=Sweep_"$parm_name1"_"$parm_name2"

#Begin run log
StartDay=$(date +%D)
StartTime=$(date +%T)
((totaltime=0))
echo "
*****************************************************************************
* "$sim_name" 
*        For "$parm_name1" = ["$list_of_values1"]
*        For "$parm_name2" = ["$list_of_values2"]
*        For #Random Runs = "$num_random_runs"
*
* BEGAN at "$StartDay" "$StartTime".
*****************************************************************************
"| tee _Results/"$sim_name".log

#Use this type of loop to step through various configuration parameters discretely.
for i in $list_of_values1
do
for j in $list_of_values2
do
((configtime=0))
#################################################################################
# Create config file with input parameters. 
# Default values are found in ./Configs/default.conf
# Name the config file based on parameter that is changing and the value of the parameter
#################################################################################
config="$parm_name1"_"$i"_"$parm_name2"_"$j"

#################################################################################
# Create a UniqueKey to save the parameters for each unique config of parameters
#################################################################################
UniqueKey=$(date +%d%m%Y%H%M%S%N)

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
RES_TRADE_ROUNDS = "$i"
RES_TRADE_ATTEMPTS = "$j"
DEVICE_TRADE_ROUNDS = "$i" 
DEVICE_TRADE_ATTEMPTS = "$j"
DEVICE_TRADE_MEMORY_LENGTH = 5
DEVICE_PRODUCTION_MEMORY_LENGTH = 5

NUM_AGENT_GROUPS = 1
MIN_RES_UTIL = 1.0
MAX_RES_EFFORT = 9.0
MIN_RES_EFFORT = 3.0
MAX_DEVICE_EFFORT = 27.0
MIN_DEVICE_EFFORT = 9.0

TOOL_FACTOR = 3.0
MACHINE_FACTOR = 9.0
FACTORY_FACTOR = 27.0
INDUSTRY_FACTOR = 81.0

TOOL_LIFETIME = 150.0
MACHINE_LIFETIME = 300.0
FACTORY_LIFETIME = 600.0
INDUSTRY_LIFETIME = 1200.0

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

DEV_MACHINE_FACTOR = 0
DEV_MACHINE_LIFETIME = 0
DEV_FACTORY_FACTOR = 0
DEV_FACTORY_LIFETIME = 0
MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION = 5

OTHER_MARKETS = False 
" > Configs/"$config".conf

grep NUM Configs/"$config".conf | tee -a _Results/"$sim_name".log

# Begin random runs using the same configuration file.
# The formatting of the iterator (run) as %03g gives it leading zeros: 001,002, etc.
run_num=$num_random_runs   
for run in $(seq -f "%03g" 1 $run_num)
do

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
./societies -v 1 -p "$config" -s _Results/"$sim_name"/"$config" -d B"$UniqueKey" -t "$run" "$seed" 1>> _Results/"$sim_name"_run.log 

# Stop the clock after societies finishes
((duration=$SECONDS))
((duration_min=$duration/60))

# Add this run time to the total time of the simulation.
((totaltime=$totaltime+$duration))
    	
echo "$config RUN $run Duration: $duration seconds ($duration_min minutes)" | tee -a _Results/"$sim_name"_runtime
   
done    #Done with one individual run within same config

rm ./Configs/"$config".conf 
rm ./Configs/"$config"_AgentValues.aconf 

done    #Done with loop using second parameter values within first parameter value
done    #Done with loop using first parameter value

EndDay=$(date +%D)
EndTime=$(date +%T)
((totalminutes=$totaltime/60))

echo "

*****************************************************************************
* "$sim_name":
*        For "$parm_name1" = ["$list_of_values1"]
*        For "$parm_name2" = ["$list_of_values2"]
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
head UniqueKeyFile.csv -n 1 > temp1
grep $sim_name\/ UniqueKeyFile.csv > temp2
cat temp1 temp2 > ./$sim_name/UniqueKeyFile.csv
rm  temp1 temp2

mv $sim_name.log ./$sim_name/$sim_name.log
mv "$sim_name"_run.log ./$sim_name/run.log

savedate=$(date +%d%m%Y%H%M%S)
mv $sim_name $sim_name.$savedate



