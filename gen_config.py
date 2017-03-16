import random, sys

agent_values = []

modes = ('random', 'homogeneous', 'two')

# Soon, all of these will command line options
# make this an option
file = open(str(sys.argv[1]), 'r')
for line in file:
        if "NUM_AGENTS" in line:
                NUM_AGENTS = int(''.join(x for x in line if x.isdigit()))
        if "NUM_RESOURCES" in line:
                NUM_RESOURCES = int(''.join(x for x in line if x.isdigit()))

# make this an option
res_mode = modes[1]
dev_mode = modes[1]
util_mode = modes[1]
lifetime_mode = modes[1]

# make this an option
lifetime_rand_range = 3.0

# make this an option
lifetime_default = 5.0

# make this an option
lifetime_second_diff = 2.0

# make this an option
res_rand_min_effort_range = 1.0
res_rand_max_effort_range = 2.0
res_rand_experience_range = 50.0

# make this an option
dev_rand_min_effort_range = 2.0
dev_rand_max_effort_range = 3.0
dev_rand_experience_range = 6.0

# make this an option
res_default_min_effort = 3.0
res_default_max_effort = 9.0
res_default_max_experience = 600.0

# make this an option
dev_default_min_effort = 9.0
dev_default_max_effort = 27.0
dev_default_max_experience = 40.0

# make this an option
res_second_min_effort_diff = 2.0
res_second_max_effort_diff = 2.0
res_second_max_experience_diff = 25.0

# make this an option
dev_second_min_effort_diff = 2.0
dev_second_max_effort_diff = 2.0
dev_second_max_experience_diff = 5.0

# make this an option
util_default_steepness = 2.5
util_default_scaling = 600.0

# make this an option
util_second_steepness_diff = .5
util_second_scaling_diff = 100

# make this an option
util_rand_steepness_range = .4
util_rand_scaling_range = 50


for ag in range(NUM_AGENTS):
    agent_values.append({'res':[], 'dev':[], 'util':[], 'lifetime':[]})
    for res in range(NUM_RESOURCES):

	# generate resource extraction variable values
	if res_mode == 'random':
	    minResEffort = res_default_min_effort + random.uniform(-res_rand_min_effort_range, res_rand_min_effort_range)
	    maxResEffort = res_default_max_effort + random.uniform(-res_rand_max_effort_range, res_rand_max_effort_range)
	    maxResExperience = res_default_max_experience + random.uniform(-res_rand_experience_range, res_rand_experience_range)

	elif res_mode == 'two':
	    if ag % 2 == 0:
	        minResEffort = res_default_min_effort + res_second_min_effort_diff
		maxResEffort = res_default_max_effort + res_second_max_effort_diff
		maxResExperience = res_default_max_experience + res_second_max_experience_diff
	    else:
		minResEffort = res_default_min_effort
		maxResEffort = res_default_max_effort
		maxResExperience = res_default_max_experience

	else:
	    minResEffort = res_default_min_effort
	    maxResEffort = res_default_max_effort
	    maxResExperience = res_default_max_experience


	# generate device building variable values
	if dev_mode == 'random':
	    minDevEffort = dev_default_min_effort + random.uniform(-dev_rand_min_effort_range, dev_rand_min_effort_range)
	    maxDevEffort = dev_default_max_effort + random.uniform(-dev_rand_max_effort_range, dev_rand_max_effort_range)
	    maxDevExperience = dev_default_max_experience + random.uniform(-dev_rand_experience_range, dev_rand_experience_range)

	elif dev_mode == 'two':
	    if ag % 2 == 0:
	        minDevEffort = dev_default_min_effort + dev_second_min_effort_diff
		maxDevEffort = dev_default_max_effort + dev_second_max_effort_diff
		maxDevExperience = dev_default_max_experience + dev_second_max_experience_diff
	    else:
	        minDevEffort = dev_default_min_effort
		maxDevEffort = dev_default_max_effort
		maxDevExperience = dev_default_max_experience

	else:
	    minDevEffort = dev_default_min_effort
	    maxDevEffort = dev_default_max_effort
	    maxDevExperience = dev_default_max_experience

	# generate utility variable values
	if util_mode == 'random':
	    steepness = util_default_steepness + random.uniform(-util_rand_steepness_range, util_rand_steepness_range)
	    scaling = util_default_scaling + random.uniform(-util_rand_steepness_range, util_rand_steepness_range)

	elif util_mode == 'two':
	    if ag % 2 == 0:
	        steepness = util_default_steepness + util_second_steepness_diff
		scaling = util_default_scaling + util_second_scaling_diff
	    else:
	        steepness = util_default_steepness
		scaling = util_default_scaling

	else:
	    steepness = util_default_steepness
	    scaling = util_default_scaling
			
	# generate lifetime values
	if lifetime_mode == 'random':
	    lifetime = lifetime_default + random.randint(-lifetime_rand_range, lifetime_rand_range)
        elif lifetime_mode == 'two':
	    if ag % 2 == 0:
	        lifetime = lifetime_default + lifetime_second_diff
	    else:
	        lifetime = lifetime_default
	else:
	    lifetime = lifetime_default

        agent_values[ag]['res'].append([minResEffort, maxResEffort, maxResExperience])
	agent_values[ag]['dev'].append([minDevEffort, maxDevEffort, maxDevExperience])
	agent_values[ag]['util'].append([steepness, scaling])
	agent_values[ag]['lifetime'].append(lifetime)

# mode_dict = {'res_mode':res_mode, 'dev_mode':dev_mode, 'util_mode':util_mode, 'lifetime_mode':lifetime_mode}
# agent_values.append(mode_dict)

# make this an option
filename = str(sys.argv[2])

f = open(filename, 'w')

f.write("# For each agent and resource: steepness, scaling, minResEff, maxResEff, maxResExp, minDevEff, maxDevEff, maxDevExp, lifetime, resTradePower, devTradePower, inventSpeed, patent, group\n")
for agId in range(len(agent_values)):
    agvals = agent_values[agId]
    f.write("# Agent " + str(agId) + " values\n")
    for resId in range(NUM_RESOURCES):
	outstr = ", ".join(map(str, [agvals['util'][resId][0], agvals['util'][resId][1],
				     agvals['res'][resId][0], agvals['res'][resId][1], agvals['res'][resId][2],
				     agvals['dev'][resId][0], agvals['dev'][resId][1], agvals['dev'][resId][2],
				     agvals['lifetime'][resId]]))
	f.write(outstr + ", 1, 1, 1, 0, 0\n")

# Now write out a final line: the modes
f.write("# MODES: lifetime_mode, dev_mode, util_mode, res_mode\n")
f.write(lifetime_mode + ", " + dev_mode + ", " + util_mode + ", " + res_mode + "\n")
f.close()
