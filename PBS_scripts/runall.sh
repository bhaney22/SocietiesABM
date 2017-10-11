#!/bin/bash
#      Calls the q.sh program that executes the qsub commands. 
#      example: "./q NoDev_4_4 30" [will use the Configs/NoDev_4_4.conf file and submit 30 qsubs]
#
# the .pbs scripts are set up to receive the name of the config file and # runs as arguments

./q T1_4_4 2

