#!/bin/bash
#      Calls the q_collapse.sh .sh program that executes the qsub commands. 
#      example: "./q_collapse.sh  NoDev_4_4 30" [will use the Configs/NoDev_4_4.conf file and submit 20 qsubs]
#
# the .pbs scripts are set up to receive the name of the config file as an argument
# to pass an argument when using qsub, you must use the -F command and put
# the argument(s) in double quotation marks.

./q_collapse.sh  IOtest30/collapse/NoDev_1_tool 30
./q_collapse.sh  IOtest30/collapse/NoDev_1_machine 30
./q_collapse.sh  IOtest30/collapse/NoDev_1_factory 30
./q_collapse.sh  IOtest30/collapse/NoDev_1_industry 30

./q_collapse.sh  IOtest30/collapse/NoDev_3_tool 30
./q_collapse.sh  IOtest30/collapse/NoDev_3_machine 30
./q_collapse.sh  IOtest30/collapse/NoDev_3_factory 30
./q_collapse.sh  IOtest30/collapse/NoDev_3_industry 30

./q_collapse.sh  IOtest30/collapse/NoDev_5_tool 30
./q_collapse.sh  IOtest30/collapse/NoDev_5_machine 30
./q_collapse.sh  IOtest30/collapse/NoDev_5_factory 30
./q_collapse.sh  IOtest30/collapse/NoDev_5_industry 30

./q_collapse.sh  IOtest30/collapse/NoDev_7_tool 30
./q_collapse.sh  IOtest30/collapse/NoDev_7_machine 30
./q_collapse.sh  IOtest30/collapse/NoDev_7_factory 30
./q_collapse.sh  IOtest30/collapse/NoDev_7_industry 30
