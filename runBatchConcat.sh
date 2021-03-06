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
# This will concatentate all of the output from one batch run
# RUN ONLY AFTER all jobs have completed.
# Last revised: BRH 08.08.2018  
##################################################################
# One required passed parameter:
# 1. sim_name 
sim_name=$1

howmany() { echo $#; } # quick little function to count jobs in the joblist
joblist=$(cat ~/SocietiesABM/_Results/"$sim_name".jobs)
jobruns=$(grep "J" ~/SocietiesABM/_Results/"$sim_name"_runtime.csv | wc -l)
numjobsstarted=$(howmany $joblist)
	
cd ~/SocietiesABM
##################################################################
# The following script adds the Header to the UniqueKey File
# and removes duplicates. The file is then copied to the
# sim_name save folder to ensure all the UniqueKey data is
# saved along with the runs 
##################################################################
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
RUN_NUM,RUN_SAVE_FOLDER,SAVE_TRADES,PARALLEL_TRADES" > temp1

## the sort statement gets rid of any duplicates
cat ~/SocietiesABM/_Results/UniqueKeyFile.csv | sort -t, -u -k1,1 > temp2
cat temp1 temp2 > ~/SocietiesABM/_Results/$sim_name/UniqueKeyFile.csv
rm  temp1 temp2

##################################################################
# Add the Header to the runtime File and copy it to the sim_name folder
# and removes duplicates. The file is later copied to the
##################################################################
echo "jobID,UniqueKey,Config,Run,StartDay,StartTime,EndTime,RunTimeInSeconds,RunTimeInMinutes" > temp1
cat temp1 ~/SocietiesABM/_Results/"$sim_name"_runtime.csv > ~/SocietiesABM/_Results/"$sim_name"/"$sim_name".runtime.csv
rm temp1
rm ~/SocietiesABM/_Results/"$sim_name"_runtime.csv

echo "***********************************************************************
* Begin Concatenate All Results Process
* "$sim_name" 
*     Jobs Started:  "$numjobsstarted" 
*     Jobs Finished: "$jobruns" 
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

###############################################################################################
# Copy and rename the slurm out files
###############################################################################################

for j in $joblist
do
	# Only move out logs if the job finished and is in the runtime file.
	w=$(grep "$j" ~/SocietiesABM/_Results/"$sim_name"/"$sim_name".runtime.csv | wc -l)
	if [[ $w > 0 ]]; then
		echo $(grep "$j" ~/SocietiesABM/_Results/"$sim_name"/"$sim_name".runtime.csv ) > tempgrep
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

savedate=$(date +%d%m%Y%H%M%S)
mv $sim_name $sim_name.$savedate

#####################################################################################
# Now organize the timestamped results sub-folders.
#####################################################################################
cd ~/SocietiesABM/_Results/$sim_name.$savedate
mkdir _Outfiles
mkdir _Configfiles
mkdir _Errfiles
mkdir _Runs
mv *.out ./_Outfiles
mv *.err ./_Errfiles
mv ../../Configs/"$sim_name"_* ./_Configfiles
mv "$sim_name"_* ./_Runs

#####################################################################################
# Now concat all of the simulation runs that have been run and are timestamped 
# in the _Results directory.
#####################################################################################
cd ~/SocietiesABM/_Results
cat ./"$sim_name".*/long_output_all.csv | head -n 1 > temp1
cat ./"$sim_name".*/long_output_all.csv | grep -v UniqueKey >> temp2 #get everything BUT the first row
cat temp1  temp2 > ./"$sim_name"_long_output.csv
rm  temp1 temp2

cat ./"$sim_name".*/UniqueKeyFile.csv | head -n 1 > temp1
cat ./"$sim_name".*/UniqueKeyFile.csv | grep -v UniqueKey >> temp2  #get everything BUT the first row
cat temp1  temp2 > ./"$sim_name"_UniqueKeyFile.csv
rm  temp1 temp2

cat ./"$sim_name".*/*.runtime.csv | head -n 1 > temp1
cat ./"$sim_name".*/*.runtime.csv | grep J >> temp2                 #get everything BUT the first row (all jobs begin with "J")
cat temp1  temp2 > ./"$sim_name"_runtime.csv
rm  temp1 temp2


exit