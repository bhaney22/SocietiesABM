import os, datetime

#parameters = [0,5,10]

cores = 3

for round in range(cores):
	os.system("python runCollapseSub.py &")