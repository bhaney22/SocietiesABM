#!/bin/bash
#      Requires 1 commandline paramater to indicate the number of runs per batch
#      Calls the q.sh program that executes the qsub commands. 
#      example: "./q NoDev_4_4 20" [will use the Configs/NoDev_4_4.conf file and submit 20 qsubs]
#
# the .pbs scripts are set up to receive the name of the config file as an argument
# to pass an argument when using qsub, you must use the -F command and put
# the argument(s) in double quotation marks.

./q_1.sh iotest1 $1

