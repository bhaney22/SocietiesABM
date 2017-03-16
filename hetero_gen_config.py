"""
Used to generate an .aconf file from a csv file.
The csv file needs to be in the form of

,property1,property2,property3, ... , num_agents
"type 0",n1,n2,n3, ... , n
"type 1",m1,m2,m3, ... , m
...

The last column/element needs to be the number of agents in this group.
The generated .aconf file will be saved in the same name as the csv file (without the csv extension).
When a new property wants to be added, basic things need to be changed in the program includes:
	a variable to store the value of the property in agent.h/.cpp
	globals:initializeAgents() (three places)
	documentation
	where it is used
"""
import shlex, sys

modes = ('random', 'homogeneous', 'two')
# make this an option
res_mode = modes[2]
dev_mode = modes[1]
util_mode = modes[1]
lifetime_mode = modes[1]

# read NUM_AGENTS and NUM_RESOURCES from conf file
fconfig = open(str(sys.argv[2]),'r')
for line in fconfig:
        if "NUM_AGENTS" in line:
                NUM_AGENTS = int(''.join(x for x in line if x.isdigit()))
        if "NUM_RESOURCES" in line:
                NUM_RESOURCES = int(''.join(x for x in line if x.isdigit()))

fcsvPath = str(sys.argv[1])
fcsv = open(fcsvPath, 'r')	# the csv file to read types from
faconfPath = fcsvPath[:-4] + '.aconf'
faconf = open(faconfPath, 'w')

lineList = fcsv.readlines()
attributes = [x for x in lineList[0].split(',')]
faconf.write("# For each agent and resource: ")
for i in range(1,len(attributes)-1):
	faconf.write(attributes[i] + ', ')
faconf.write("group\n")

countAgent = 0
for i in range(1,len(lineList)):
	lineSplit = [x for x in lineList[i].split(',')]
	agentNum = lineSplit[len(lineSplit)-1]
	for j in range(int(agentNum)):
		faconf.write("# Agent " + str(countAgent) + ' values\n')
		for k in range(NUM_RESOURCES):
			for m in range(1,len(lineSplit)-1):
				faconf.write(lineSplit[m] + ',')
			faconf.write(str(i-1) + '\n')
		countAgent += 1
if countAgent != NUM_AGENTS:
	print "Wrong number of agents!!! NUM_AGENTS must equal sum in agents.csv file!!"

# Now write out a final line: the modes
faconf.write("# MODES: lifetime_mode, dev_mode, util_mode, res_mode\n")
faconf.write(lifetime_mode + ", " + dev_mode + ", " + util_mode + ", " + res_mode + "\n")
faconf.close()
