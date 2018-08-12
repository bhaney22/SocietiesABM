#!/bin/bash
# This will MULTI-thread multiple runs of randomized configuration files of Societies
# on the supercomputer cluster.
#
# Last revised: BRH 08.08.2018  
# (note: check out this link for picky rules about math and variables in bash shell scripts:
# http://faculty.salina.k-state.edu/tim/unix_sg/bash/math.html)
#
# TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# 1. make sure that the sim_name is correct below! 
# 2. make sure that the created name of the configfile contains all of the variables being swept so that it
#    does not get overwritten
# 3. if any parameters are randomized rather than discretely stepped over, change that when building the config
#    and add it to the comment that is echoed to the log after the config is built.
# COLLNOCOLL runs exact same config with and without collapse
sim_name=COLLNOCOLL

cd ~/SocietiesABM/
############################################################################
# OPTIONAL commandline arguments.
# 
# The number before the colon is the order of the passed parameter on the command line.
# The number after the colon and the "-" is the default if no parameters are passed.
#
#########################################################################################################
# Run this script with "help" as the first parameter to get list of parameters and default values.
#
if [ "$1" == "help" ]; then
echo "
Order of Optional Passed Parameters and default value:
1. num_random_conf_files=001
2. num_random_runs=${2:-002} 
3. num_days=${3:-100}

4. trade=${4:-True}
5. devices=${5:-True}
6. tools_only=${6:-False}

7. num_agents=${7:-8} 
8. num_resources=${8:-4} 
9. resources_in_tool=${9:-5}  or 1 less than #RES if #RES<6 
"
exit
fi
#########################################################################################################

num_random_conf_files=${1:-001} 
num_random_runs=${2:-002} 
num_days=${3:-100}

trade=${4:-True}
devices=${5:-True}
tools_only=${6:-False}

num_agents=${7:-8} 
let num_resources=${8:-4}    #"let" is used so that math can be performed with the variable later

# if NUM_RESOURCES < 6 then resources_in_tool is 1 less than NUM_RESOURCES
# otherwise, it can be passed as a parameter with the default = 5 
if [ $num_resources -lt 6 ]; then
		let resources_in_tool=$num_resources-1
else 	let resources_in_tool=${9:-5}
fi

let num_device_components=$resources_in_tool+1   # The default is for device components to be 1 more than tool components
#
############################################################################

#############################################################################################################
# Control amount of random variation within one config file by uncommenting the appropriate option:
# Option 1: The same configuration will have random results. 
# Option 2: No variation - used for testing. (comment out Option 1 and uncomment out Option 2, if needed)
#############################################################################################################

#############################################################################################################
# 		Option 1
############################################################################################################## 
seed=""

#############################################################################################################
# 		Option 2
#############################################################################################################
#seed=50	#Set the random seed so that societies' results will be the same if all parameters are the same

#################################################################################################################
# BEGIN SPECIFIC CODE FOR INTEGRATION TEST.
# ITEST_trade:
# 	This Integration test randomizes the integer parameter values related to trade. The following comments
#	show the original (default) values and the range of values from which a random number is chosen in each run.
# 	The actual changes are in the config file below.
#################################################################################################################
#Begin run log
StartDay=$(date +%D)
StartTime=$(date +%T)
echo "
*****************************************************************************
* "$sim_name" STARTED: "$StartDay" "$StartTime".
*
*        #Randomized Config Files = "$num_random_conf_files"
*        #Random Runs = "$num_random_runs"
*
*        NUM_DAYS = "$num_days"
*        NUM_AGENTS = "$num_agents"
*        NUM_RESOURCES = "$num_resources"
*        RESOURCES_IN_TOOL = "$resources_in_tool"
*        TOOLS_ONLY = "$tools_only"
*        TRADE = "$trade"
*        DEVICES = "$devices"
*
*
*****************************************************************************
"| tee _Results/"$sim_name".log

echo "jobID,UniqueKey,Config,Run,StartDay,StartTime,EndTime,RunTimeInSeconds,RunTimeInMinutes" > _Results/"$sim_name"_runtime.csv

####################################################################################
# Begin LOOP 1: BUILD THE CONFIG FILE (optionally - loop over some randomized parameters within it)
#
# The formatting of the iterator as %03g gives it leading zeros: 001,002, etc.

for X in $(seq -f "%03g" 1 $num_random_conf_files)
do

#################################################################################
# BUILD config file with input parameters. 
# Default values are found in ./Configs/default.conf
# Include all variables being swept in the config!! Any and all random aspects of the config
# will be captured automatically in the "_X" suffix.
#################################################################################
config="$sim_name"_"$num_agents"_"$num_resources"_"$resources_in_tool"_"$X"

echo "
START_DAY = 0
DAY_LENGTH = 600

TRADE_EXISTS = "$trade"
DEVICES_EXIST = "$devices"
TOOLS_ONLY = "$tools_only"

NUM_DAYS = "$num_days"
NUM_AGENTS = "$num_agents"
NUM_RESOURCES = "$num_resources"
RESOURCES_IN_TOOL = "$resources_in_tool"
NUM_DEVICE_COMPONENTS = "$num_device_components"

MENU_SIZE =  4
RES_TRADE_ROUNDS = 4
RES_TRADE_ATTEMPTS = 2
DEVICE_TRADE_ROUNDS = 4
DEVICE_TRADE_ATTEMPTS = 2
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

DEV_MACHINE_FACTOR = .1
DEV_MACHINE_LIFETIME = 1
DEV_FACTORY_FACTOR = .1
DEV_FACTORY_LIFETIME = 1
MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION = 10000

OTHER_MARKETS = False 
" > Configs/"$config".conf

######################################################################################
# Print the randomized parameter values to the screen and the log first, then
# save the entire configuration to the log file.
######################################################################################
echo "
COLLNOCOLL: Runs the Collapse and NO Collapse Scenario for each config.
CONFIG = "$config"  " | tee -a _Results/"$sim_name".log
grep NUM_DAYS Configs/"$config".conf |tee -a _Results/"$sim_name".log
grep NUM_AGENTS Configs/"$config".conf |tee -a _Results/"$sim_name".log
grep NUM_RESOURCES Configs/"$config".conf |tee -a _Results/"$sim_name".log
grep RESOURCES_IN_TOOL Configs/"$config".conf |tee -a _Results/"$sim_name".log
#Add any randomized parameters here.


# Begin random runs using the same configuration file.
# The formatting of the iterator (run) as %03g gives it leading zeros: 001,002, etc.
#################################################################################
# Create a UniqueKey to save the parameters for each unique config of parameters
#################################################################################
UniqueKey=$(date +%d%m%Y%H%M%S%N)
for run in $(seq -f "%03g" 1 $num_random_runs)
#########################################################################
# Run the "No Collapse" Scenarios for this configuration. Generate a new UniqueKey.
##########################################################################
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

# Pass 6 values to runSocieties.sh. 2-5 are info used in the runtime.csv file.
# 1. the runSocieties command
# 2. UKEY 
# 3. CONF 
# 4. RUN 
# 5. SIMNAME 
# 6. SEED

	sleep 2   # Use "sleep" to create some time between each submission for smooth fileI-O
	sbatch --job-name=$sim_name runSocieties.sh "./societies -v 1 -p "$config" -s _Results/"$sim_name"/"$config" -d B"$UniqueKey" -t "$run"" "B"$UniqueKey"" ""$config"" ""$run"" ""$sim_name"" ""$seed""	
done    #Done with No Collapse scenario of one run of one randomly generated configuration


#########################################################################
# Now, run the Collapse Scenarios for this configuration. Generate a new UniqueKey.
##########################################################################
UniqueKey=$(date +%d%m%Y%H%M%S%N)
for run in $(seq -f "%03g" 1 $num_random_runs)
do
	sleep 2
	sbatch --job-name=$sim_name runSocieties.sh "./societies -v 1 -r 4 600 1 -p "$config" -s _Results/"$sim_name"/"$config" -d B"$UniqueKey" -t "$run"" "B"$UniqueKey"" ""$config"" ""$run"" ""$sim_name"" ""$seed""	
done    #Done with Collapse scenario of one run of one randomly generated configuration

done    #Done with all runs of one configuration

