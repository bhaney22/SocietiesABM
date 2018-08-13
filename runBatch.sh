#!/bin/bash

#Order of Optional Passed Parameters and default value:
#1. num_random_conf_files=001
#2. num_random_runs=${2:-002} 
#3. num_days=${3:-100}
#
#4. trade=${4:-True}
#5. devices=${5:-True}
#6. tools_only=${6:-False}
#
#7. num_agents=${7:-8} 
#8. num_resources=${8:-4} 
#9. resources_in_tool=${9:-5}  or 1 less than #RES if #RES<6 

cd ~/SocietiesABM/

#
# Use dependency so that the concat job won't run until all of the 
# jobs for that ITEST_xxxx have finished.
#
for runtype in COLLNOCOLL # runs exact same config with and without collapse
do
	for numagents in 8 12 16 20 24 48
	do
		for numresources in 4       # SEE NOTE BELOW
		do
			for numresintool in 3  
				do
					./run$runtype.sh 001 30 1200 True True False $numagents $numresources $numresintool
				done
		done
	done
	sbatch  --dependency=singleton --job-name=$runtype ./runBatchConcat.sh $runtype
	
done

exit

# Note that if one (or more) of the number of resources is less than 6, then the number of resources in a tool is set to 3 no matter
# what parameter value for numresintool is passed.
# THUS
# There will be unexpected errors unless the numresintool only has one option and that option must be "3".