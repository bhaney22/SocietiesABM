#!/bin/bash
#      Calls the q_nocollapse.sh.sh program that executes the qsub commands. 
#      example: "./q_nocollapse.sh NoDev_4_4 30" [will use the Configs/NoDev_4_4.conf file and submit 20 qsubs]
#
# the .pbs scripts are set up to receive the name of the config file as an argument
# to pass an argument when using qsub, you must use the -F command and put
# the argument(s) in double quotation marks.

./q_nocollapse.sh IOtest/NoDev_1_tool 1
./q_nocollapse.sh IOtest/NoDev_1_machine 1
./q_nocollapse.sh IOtest/NoDev_1_factory 1
./q_nocollapse.sh IOtest/NoDev_1_industry 1

./q_nocollapse.sh IOtest/NoDev_3_tool 1
./q_nocollapse.sh IOtest/NoDev_3_machine 1
./q_nocollapse.sh IOtest/NoDev_3_factory 1
./q_nocollapse.sh IOtest/NoDev_3_industry 1

./q_nocollapse.sh IOtest/NoDev_5_tool 1
./q_nocollapse.sh IOtest/NoDev_5_machine 1
./q_nocollapse.sh IOtest/NoDev_5_factory 1
./q_nocollapse.sh IOtest/NoDev_5_industry 1

./q_nocollapse.sh IOtest/NoDev_7_tool 1
./q_nocollapse.sh IOtest/NoDev_7_machine 1
./q_nocollapse.sh IOtest/NoDev_7_factory 1
./q_nocollapse.sh IOtest/NoDev_7_industry 1
