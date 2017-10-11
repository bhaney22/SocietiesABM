# This programs appends all of the long_output.csv files from all results subfolders into one
# file of Societies runs across all configurations that were run for processing by
# Stata or Tableau.
#
# FYI: the head command saves the header row from the first individual long_output file.
# FYI: the grep command strips out all of the header rows (starts with UniueKey)
#      from the individual long_output files.
# BRH 3.20.2017

cat _Results/*/long_output.csv | head -n 1 > temp1
cat _Results/*/long_output.csv | grep -v UniqueKey > temp2
cat temp1  temp2 > _Results/long_output_all.csv
rm temp1 temp2

