#!/bin/bash
# # # # # # # # # # # # # # # #

# Set number of of nodes - max is 20!
#SBATCH -N 1
# Set number of cpus-per-task - max is 16!
#SBATCH -c 1
# Set job name
#SBATCH -J concat
# Set max execution time, ex:  minutes  minutes:seconds  hours:minutes:seconds 
#SBATCH -t 1:00:00
#Set working directory
#SBATCH -D /home/brh22/SocietiesABM/_Slurm.out

##################################################################
# This will concatentate all of the output from one ITEST simulation
# RUN ONLY AFTER all jobs have completed.
# Last revised: BRH 08.08.2018  
##################################################################
# One required passed parameter:
# 1. sim_name (example, ITEST_trade)
sim_name=$1

howmany() { echo $#; } # quick little function to count jobs in the joblist
joblist=$(cat ~/SocietiesABM/_Results/"$sim_name".jobs)
jobruns=$(grep "J" ~/SocietiesABM/_Results/"$sim_name"_runtime.csv | wc -l)
numjobsstarted=$(howmany $joblist)
	
cd ~/SocietiesABM/

EndDay=$(date +%D)
EndTime=$(date +%T)
echo "***********************************************************************
* "$sim_name"  "$numjobsstarted" Jobs Started 
*              "$jobruns" Jobs Finished 
*****************************************************************************
" | tee -a _Results/"$sim_name".log

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

# Print out runtimes.
echo "
Runtime statistics:
" | tee -a $sim_name.log
column -s , -t < "$sim_name"_runtime.csv | tee -a $sim_name.log 

# Print out a summary of resources gathered.
echo "
Units Gathered in all runs:
" | tee -a $sim_name.log
column -s , -t < ./$sim_name/unitsGathered_all.csv | tee -a $sim_name.log 

################################################################################################
#
# Save all of the relevant UniqueKey entries with the output.
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


###############################################################################################
# Copy and rename the slurm out files
###############################################################################################

for j in $joblist
do
	# Only move out logs if the job finished and is in the runtime file.
	w=$(grep "$j" ~/SocietiesABM/_Results/"$sim_name"_runtime.csv | wc -l)
	if [[ $w > 0 ]]; then
		echo $(grep "$j" ~/SocietiesABM/_Results/"$sim_name"_runtime.csv) > tempgrep
		sed -i 's/,/ /g' tempgrep
		jobinfo=$(cat tempgrep)
		set -- $jobinfo
		mv ~/SocietiesABM/_Slurm.out/slurm-"$j".out ~/SocietiesABM/_Results/"$sim_name"/"$3"_run"$4"_"$j".out	
		# Only move error logs if they are present and not empty because Societies ended with an error; 
		# Otherwise delete the empty error log from the Slurm.out directory.
		if [[   -s ~/SocietiesABM/_Slurm.out/slurm-"$j".err  ]]; then
				mv ~/SocietiesABM/_Slurm.out/slurm-"$j".err ~/SocietiesABM/_Results/"$sim_name"/"$3"_run"$4"_"$j".err
		else  	rm ~/SocietiesABM/_Slurm.out/slurm-"$j".err
		fi
	# If a job was in the job list but not in the runtime.csv file, then it ended because it was cancelled because of over time limit (or something
	# other than a Societies error). Save the output and error logs to look at later.
	else 
			mv ~/SocietiesABM/_Slurm.out/slurm-"$j".out ~/SocietiesABM/_Results/"$sim_name"/NotFinished_"$j".out
			mv ~/SocietiesABM/_Slurm.out/slurm-"$j".err ~/SocietiesABM/_Results/"$sim_name"/NotFinished_"$j".err
	fi
done
rm tempgrep

echo "
Error Logs:" 			| tee -a $sim_name.log 
ls -la  ~/SocietiesABM/_Results/$sim_name/*.err  		| tee -a $sim_name.log
tail -n +1  ~/SocietiesABM/_Results/$sim_name/*.err  	| tee -a $sim_name.log 

mv $sim_name.log  ./$sim_name/$sim_name.log
mv $sim_name.jobs ./$sim_name/$sim_name.jobs
mv "$sim_name"_runtime.csv  ./$sim_name/"$sim_name"_runtime.csv

savedate=$(date +%d%m%Y%H%M%S)
mv $sim_name $sim_name.$savedate

exit