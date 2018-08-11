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

./runITESTCOLL_trade.sh 100 30 1500 True True False 24 8

#./runITESTCOLL_experience.sh 300 100
#./runITESTCOLL_devices.sh 300 100
#./runITESTCOLL_effort.sh 300 100
#./runITESTCOLL_epsilons.sh 300 100

#
# Use dependency so that the concat job won't run until all of the 
# jobs for that ITEST_xxxx have finished.
#
for itest in trade # experience devices effort epsilons
do
	sbatch  --dependency=singleton --job-name=ITESTCOLL_$itest ./runITEST_batch_concat.sh ITESTCOLL_$itest
done

exit
