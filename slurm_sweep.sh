#!/bin/bash
#      slurm_societies.sh requires 2 command line arguments: 
$1) config_name 
$2) how many runs per batch

# for Res in 3 5 
for Res in 5
do
#	for Tier in tool machine factory industry
	for Tier in industry
    ## TODO - create sweep by appending parameter values to config_name 
    ## sweep_config = $1_$Res_$Tier
		do
		    sbatch slurm_societies.sh  sweep_config $2
	    done
	
done