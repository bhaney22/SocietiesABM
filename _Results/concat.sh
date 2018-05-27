# This programs appends all of the *.csv files from all results subfolders into one
# file of Societies runs across all configurations that were run for processing by
# Stata or Tableau.
#
# Run this file after all runs from the configuration have completed
# REQUIRES 1 command line parameter: the name of the config (eg. T4_24_24)
#
# FYI: the head command saves the header row from the first individual long_output file.
# FYI: the grep -v command strips out all of the header rows (starts with UniqueKey)
#      from the individual long_output files.
# BRH 3.20.2017
# BRH 05.26.2018 - change this to concat all files from each run into one for the config overall
# 

cat ./$1/001/IOMatrix.csv | head -n 1 > temp1
cat ./$1/*/IOMatrix.csv  | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$1/IOMatrix.csv
rm temp1 temp2

cat ./$1/001/DeviceRecipes.csv | head -n 1 > temp1
cat ./$1/*/DeviceRecipes.csv  | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$1/DeviceRecipes.csv
rm temp1 temp2

cat ./$1/001/long_output.csv | head -n 1 > temp1
cat ./$1/*/long_output.csv | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$1/long_output.csv
rm temp1 temp2

cat ./$1/001/unitsGathered.csv | head -n 1 > temp1
cat ./$1/*/unitsGathered.csv | grep -v UniqueKey >> temp2
cat temp1  temp2 > ./$1/unitsGathered.csv
rm temp1 temp2

savedate=$(date +%d%m%Y%H%M)

mv $1 $1.$savedate
ls -l