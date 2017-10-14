#!/bin/bash
# when running this script:
# pass (1) the name of the config file (without the .conf suffix) and 
#      (2) # runs
#
#      example: "./q NoDev_4_4 30" [will use the Configs/NoDev_4_4.conf file and submit 30 qsubs]
#
# the .pbs scripts are set up to receive the name of the config file as an argument
# to pass an argument when using qsub, you must use the -F command and put
# the argument(s) in double quotation marks.

echo "***"
echo "*** Config: $1"
echo "*** Number of Runs: $2"
echo "***"

# Remove the old results for this config file.
rm -r /home/brh22/societies2017/_Results/$1

qsub -F "$1 $2" run_collapse.pbs
