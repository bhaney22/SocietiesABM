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
# and removes duplicates. The file is later copied to the
# sim_name save folder to ensure all the UniqueKey data is
# saved along with the runs 
##################################################################
./runAddHdrToUkeyFile.sh


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
# Save the UniqueKey file data with the output.
#
################################################################################################
cp ~/SocietiesABM/_Results/UniqueKeyFileAndHeader.csv> ./$sim_name/UniqueKeyFile.csv

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

cd ~/SocietiesABM/_Results/$sim_name.$savedate
mkdir _Outfiles
mkdir _Configfiles
mkdir _Errfiles
mkdir _Runs
mv *.out ./_Outfiles
mv *.err ./_Errfiles
mv ../../Configs/"$sim_name"_* ./_Configfiles
mv "$sim_name"_* ./_Runs
exit