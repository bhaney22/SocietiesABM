#!/bin/bash
#      Requires 2 command line arguments: $1) how many runs per batch $2) no for nocollapse or blank for collapse
#	   Then calls the q_collapse.sh .sh program that executes the qsub commands. 
#      example: "./q_collapse.sh  NoDev_4_4 30" [will use the Configs/NoDev_4_4.conf file and submit 20 qsubs]
#
# the .pbs scripts are set up to receive the name of the config file as an argument
# to pass an argument when using qsub, you must use the -F command and put
# the argument(s) in double quotation marks.

for Res in 3 5 
do
	for Tier in tool machine factory industry
	do
		./q_"$2"collapse.sh IOtest/"$2"collapse/NoDev_"$Res"_"$Tier" $1
	done
done


