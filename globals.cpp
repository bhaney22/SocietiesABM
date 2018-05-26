#include <cmath>
#include <ctime> 
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>	// for converting int to string
#include "boost/date_time/gregorian/gregorian.hpp"		// for date
#include "globals.h"
#include "agent.h"
#include "resource.h"
#include "marketplace.h"
#include "ran.h"
#include "statstracker.h"
#include "devmarketplace.h"
#include "utils.h"
#include "logging.h"


/**
 * Random number generator. Generates random numbers between 0 and 1.
 */
Ran01<double>	randGen;
/**
 * Random number generator. Generates random numbers using binomial distribution.
 */
RanBinomial<double>	randBinomial;

/**
 * Define an array of device types.
 */
string device_names[] = { "Tool", "Machine", "Factory", "Industry", "DevMachine", "DevFactory", "NoDevice!!" };

/**
 * Constructor.
 */
Globals::Globals()
{
    REMOVE_RES = false;
    REMOVE_AGENT = false;
    END_SAVE = false;
    SAVE_DAY_STATUS = false;
    SIM_NAME = "default";
    PARALLEL_TRADES = false;
}

/**
 * initialize random seed, EXPERIENCE_FOR_MAKING, RES_DECAY_SLOWNESS, RES_VALUES.
 */
void Globals::initGlobalStructures()
{
    if (randomSeedSet) {
        randGen = Ran01<double>(randomSeed);
    } else {
        randGen = Ran01<double>();
    }
    if (randomSeedSet) {
        randBinomial = RanBinomial<double>(randomSeed);
    } else {
        randBinomial = RanBinomial<double>();
    }
    
    discoveredDevices.resize(NUM_DEVICE_TYPES);	// there are 6 empty vectors in this vector.
    EXPERIENCE_FOR_MAKING.resize(NUM_DEVICE_TYPES);	// there are 6 empty vectors in this vector
    RES_DECAY_SLOWNESS.resize(NUM_RESOURCES);
    RES_VALUES.resize(NUM_RESOURCES);

    if (START_DAY == 0) {		// if not loading a previous run
        EXPERIENCE_FOR_MAKING.resize(NUM_DEVICE_TYPES);
        EXPERIENCE_FOR_MAKING[TOOL] = 1.0;
        EXPERIENCE_FOR_MAKING[MACHINE] = 2.0;
        EXPERIENCE_FOR_MAKING[FACTORY] = 4.0;
        EXPERIENCE_FOR_MAKING[INDUSTRY] = 8.0;
        EXPERIENCE_FOR_MAKING[DEVMACHINE] = 4.0;
        EXPERIENCE_FOR_MAKING[DEVFACTORY] = 8.0;
    } else {		// not START_DAY == 0
    	DAY_STATUS_LOAD_FOLDER = "";
        int slashIdx = configFilename.find_last_of('/');
        for (unsigned i = 0; i < (unsigned) slashIdx; i++) {
        	DAY_STATUS_LOAD_FOLDER += configFilename[i];
        }
//        loadDayStatus();	// from utils.cpp
    }

    // CODE HERE IN config.py to do GRAPH_X_VALUES, QUArtILE_COLORS.  Don't need to do this
    // for the cpp code.  leave in python.

    RES_IN_DEV.resize(NUM_DEVICE_TYPES);   // make room for a value for each device name.
    RES_IN_DEV[TOOL] = RESOURCES_IN_TOOL;
    RES_IN_DEV[MACHINE] = RESOURCES_IN_TOOL * NUM_DEVICE_COMPONENTS;
    RES_IN_DEV[FACTORY] = RESOURCES_IN_TOOL * NUM_DEVICE_COMPONENTS * NUM_DEVICE_COMPONENTS;
    RES_IN_DEV[INDUSTRY] = RESOURCES_IN_TOOL * NUM_DEVICE_COMPONENTS *
        NUM_DEVICE_COMPONENTS * NUM_DEVICE_COMPONENTS;
    RES_IN_DEV[DEVMACHINE] = RESOURCES_IN_TOOL * NUM_DEVICE_COMPONENTS * NUM_DEVICE_COMPONENTS;
    RES_IN_DEV[DEVFACTORY] = RESOURCES_IN_TOOL * NUM_DEVICE_COMPONENTS *
        NUM_DEVICE_COMPONENTS * NUM_DEVICE_COMPONENTS;   

    // inizialize memoMUs
    for (int i = 0; i < NUM_AGENT_GROUPS; i++) {
        memoMUs.push_back(map<pair<int,int>, double>());
    }
}
/**
 * Read agents values in from configAgentFilename file (the one the user set using -p or the default one),
 * and initialize each agent with its specific values.
 */
void Globals::initializeAgents()
{
    ifstream agentValsFile(configAgentFilename.c_str());
    /*
     * the agent values file is organized this way:
     * # comment line -- skip this.
     * Each non comment line has 12 values in it:
     *  steepness, scaling, minResEff, maxResEff, maxResExp, minDevEff, maxDevEff, maxDevExp, lifetime, resTradePower, devTradePower, patent, group
     * First there is a line of this data for each of the resources for agent 0.  (typically 24 lines)
     * Then there is a line of this data for each of the resources for agent 1.   (typically 24 lines)
     * Repeat for each agent.
     * At the very end are two lines like this:
     * # MODES: lifetime_mode, dev_mode, util_mode, res_mode
     * homogeneous, homogeneous, homogeneous, homogeneous
     *
     * resTradePower: default is 1, can be anything except for 0.
     */
    NUM_AGENTS_IN_GROUP = vector<int>(NUM_AGENT_GROUPS, 0);
    NUM_ACTIVE_AGENTS_IN_GROUP = vector<int>(NUM_AGENT_GROUPS, 0);
    agent.clear();
    for (int aId = 0; aId < NUM_AGENTS; aId++) {
        vector< vector<double> > agValues;
        for (int resId = 0; resId < NUM_RESOURCES; ) {
            string line;
            getline(agentValsFile, line);	// read line from file into string
            stringstream sline(line);		// make a stringstream of the line
            if (line[0] == '#') {		// if first char is #, skip line
                continue;
            }
            /* read in a line of 13 values for each resource, comma-separated. */
            vector<double> t(14, 0.0);
            for (int i = 0; i < 14; i++) {
                sline >> t[i];
                if (sline.peek() == ',') {
                    char comma;
                    sline >> comma;
                }
                /*
                 * Compute the number of agents in each group.
                 * For now, the resources of one agent are the same.
                 * So the type for all resources in one agent is the same.
                 * Only need to check once.
                 */
                if ((resId == 0) && (i == 13)) {
                    NUM_AGENTS_IN_GROUP[t[i]]++;
                }
            }
            agValues.push_back(t);  // put the temporary vector of 9 values on the end.
            // We read in values for the resource, so now we get ready for
            // the next resource.
            resId++;
        }
        agent.push_back(new Agent(aId, agValues));
    }

    activeAgents = 0;
    for (int aId = 0; aId < NUM_AGENTS; aId++) {
        if (agent[aId]->inSimulation) {
            activeAgents++;
            NUM_ACTIVE_AGENTS_IN_GROUP[agent[aId]->group]++;
        }
    }
}

/**
 * Create a directory and its "parent / grandparent" directories specified in the path if they don't exist.
 */
void Globals::createDirectory(string path)
{
    const char* pathChar = path.c_str();
    string subPath = "";
    for (unsigned int i = 1; i < strlen(pathChar); i++) {  // don't need to check the first char
        /*
         * Check for each parent directory and see if it exists.
         */
        if (pathChar[i] == '/') {
            subPath = path.substr(0, i);
            if (! boost::filesystem::exists(subPath)) {
                boost::filesystem::create_directory(subPath);
            }
        }
        if (i == strlen(pathChar)-1) {
            if (! boost::filesystem::exists(path)) {
                boost::filesystem::create_directory(path);
            }
        }
    }
}


/**
 * Set some global variables according to the command line the user enters.
 */
void Globals::setAdvancedOptions()
{
    REMOVE_RES = false;
    if (removeResMidRun) {
        REMOVE_RES = true;
        RES_TO_REMOVE = removeResId;
        REMOVE_RES_DAY = removeResDay;
        ELIMINATE_RESERVES = removeResHoldings;
    } else {
		REMOVE_RES_DAY=-99;
		ELIMINATE_RESERVES=false;
		RES_TO_REMOVE=-99;
	}	

    REMOVE_AGENT = false;
    if (removeAgentMidRun) {
        REMOVE_AGENT = true;
        AGENT_TO_REMOVE = removeAgentId;
        REMOVE_AGENT_DAY = removeAgentDay;
    } else {
		AGENT_TO_REMOVE=-99;
		REMOVE_AGENT_DAY=-99;
	}

    SAVE_DAY_STATUS = false;
    if (saveInMiddle) {
        DAY_STATUS_SAVE_FOLDER = saveInMiddleFoldername;
        DAY_FOR_SAVE = saveInMiddleDay;
        createDirectory(DAY_STATUS_SAVE_FOLDER);
        SAVE_DAY_STATUS = true;
    } else {
		DAY_FOR_SAVE=-99;
	}

    END_SAVE = false;
    if (saveFileFolderSet) {
        END_SAVE = true;
        SAVE_FOLDER = saveFileFolder;
        createDirectory(SAVE_FOLDER);
        createDirectory(SAVE_FOLDER + "/configFiles");
    }

    if (! END_SAVE) {
        // simTitle has default value of "000" if not set explicitly.
        SIM_NAME = simTitle;
    } else {
        if (simTitle == "000") {	// sim title (run number) wasn't explicity tracked with -t.
        	/*
        	 * TODO: not sure about the name. python mkdtemp generates a random name with prefix sim_.
        	 * For path spliting, any easier way?
        	 */
        	SIM_SAVE_FOLDER = SAVE_FOLDER + "/sim_default";
        	createDirectory(SIM_SAVE_FOLDER);

            int slashIdx = SIM_SAVE_FOLDER.find_last_of('/');
            SIM_NAME = "";
            for (unsigned i = slashIdx + 1; i < SIM_SAVE_FOLDER.length(); i++) {
            	SIM_NAME += SIM_SAVE_FOLDER[i];
            }
        } else {
            SIM_NAME = simTitle;
            createDirectory(SAVE_FOLDER);
            createDirectory(SAVE_FOLDER + "/configFiles");

            SIM_SAVE_FOLDER = SAVE_FOLDER + '/' + SIM_NAME;
            createDirectory(SIM_SAVE_FOLDER);
        }
    }

    SAVE_TRADES = glob.saveExchangeRateData;
}

/**
 * Reinitialize the variables, saving paths that need to be reinitialized for each new run.
 */
void Globals::perRunInitialization()
{
    res.clear();
    discoveredDevices.clear();
    discoveredDevices.resize(NUM_DEVICE_TYPES);
    if (START_DAY == 0) {
        for (int resId = 0; resId < NUM_RESOURCES; resId++) {
            res.push_back(Resource(resId));
            for (int devId = TOOL; devId < NUM_DEVICE_TYPES; devId++) {
                discoveredDevices[devId].push_back(NULL);
            }
        }
    } else {
        /*
         * TODO: saveDayStatus and loadDayStatus in utils.cpp
         */
        DAY_STATUS_LOAD_FOLDER = "";
        int slashIdx = configFilename.find_last_of('/');
        for (unsigned i = 0; i < (unsigned) slashIdx; i++) {
            DAY_STATUS_LOAD_FOLDER += configFilename[i];
        }
    }
    setGlobalMarketPlaces();
    setGlobalStats();
    initializeAgents();

}

/**
 * Create new instances of ResourceMarketplace and DeviceMarketplace.
 */
void Globals::setGlobalMarketPlaces()
{
    resourceMarket = new ResourceMarketplace();
    deviceMarket = new DeviceMarketplace();
}

/**
 * Create new instances of TradeStats, ProductionStats, OtherStats.
 */
void Globals::setGlobalStats()
{
    tradeStats = new TradeStats();
    productionStats = new ProductionStats();
    otherStats = new OtherStats();
}

/**
 * Set the start time of the run.
 */
void Globals::startTimer()
{
    startTime = time(0);
}

/**
 * Set the end time of the run.
 */
void Globals::endTimer()
{
    endTime = time(0);
}

/**
 * Remove a certain agent on a specific day during the run.
 * \param agentNum the index of the agent to be removed
 * \param day the day that the agent needs to be removed
 */
void Globals::removeAgent(int agentNumber, int day)
{
    agent[agentNumber]->remove();
    activeAgents--;
    NUM_ACTIVE_AGENTS_IN_GROUP[agent[agentNumber]->group]--;
    LOG(3) << "Agent " << agentNumber << " has been removed on day " << day;
}

/**
 * Remove a certain resource on a specific day during the run.
 * \param resNumber the index of the resource to be removed
 * \param day the day that the resource needs to be removed
 */
void Globals::removeRes(int resNumber, int day)
{
    res[resNumber].remove();
    BOOST_FOREACH(Agent *ag, agent) {
        for (int dev = TOOL; dev <= DEVFACTORY; dev++) {
            ag->devProp[dev][resNumber].deviceHeld = 0;
        }
    }
    if (ELIMINATE_RESERVES) {
        BOOST_FOREACH(Agent *ag, agent) {
            ag->resProp[resNumber].setHeld(0);
            ag->resProp[resNumber].experience = 0;
        }
    }
    LOG(4) << "Res " << resNumber << " has been removed on day " << day;
}

/**
 * \return a random double in range [lower, upper).
 */
double Globals::random_range(double lower, double upper) {
    return (randGen() * (upper - lower)) + lower;
}
/**
 * \return a random double in range [0.0, 1.0).
 */
double Globals::random_01() {
    return random_range(0.0, 1.0);
}
/**
 * \return an random integer value in range [lower, upper).
 */
int Globals::random_int(int lower, int upper) {
    return ((int) random_range(lower, upper));
}
/**
 * \returns an random integer value in range [lower, upper].
 */
int Globals::random_int_inclusive(int lower, int upper) {
    return random_int(lower, upper + 1);
}
/**
 * \param vec a vector of integers
 * \return an int randomly chosen from the vector.
 */
int Globals::random_choice(vector<int> &vec)
{
    return vec[random_int(0, vec.size())];
}
/**
 * \param n the number of independent experiements.
 * \param p the probability of success
 * \return an int randomly from a binomial distribution.
 */
int Globals::random_binomial(int n, double p)
{
    return randBinomial(n, p);
}

/**
 * Reinitialize SIM_NAME and SIM_SAVE_FOLDER.
 */
void Globals::reinitialize()
{
    pair<string, string> names = calcSimName(year + month + day);
    SIM_NAME = names.first;
    SIM_SAVE_FOLDER = SAVE_FOLDER + '/' + SIM_NAME;
    createDirectory(SIM_SAVE_FOLDER);
}


/**
 * \return the scenario number (1-4)
 */
int Globals::calcScenarioNumber()
{
    int scenarioNum = 1;
    if (TRADE_EXISTS) {
        scenarioNum = 2;
        if (DEVICES_EXIST) {
            scenarioNum = 4;
            if (TOOLS_ONLY) {
                scenarioNum = 3;
            }
        }
    }
    return scenarioNum;
}


/**
 * \return the simulation number (001 - 999)
 */
string Globals::calcSimNumber(string simName)
{
    char simNumChar[3];
    string simNumStr = "001";
    int simNumInt = 1;
    int temp;

    while ( boost::filesystem::exists(SAVE_FOLDER + '/' + simName + '_' + 'r' + simNumStr) ) {
        simNumInt++;
        if (((int) log10( (double) simNumInt ) + 1) == 1) {
             temp = sprintf(simNumChar, "00%d", simNumInt);
        } else if (((int) log10( (double) simNumInt ) + 1) == 2) {
            temp = sprintf(simNumChar, "0%d", simNumInt);
        } else if (((int) log10( (double) simNumInt ) + 1) == 3) {
            temp = sprintf(simNumChar, "%d", simNumInt);
        } else if (((int) log10( (double) simNumInt ) + 1) == 4) {
            cout << "Too much simulations!" << endl;
            return 0;
        }
        simNumStr = string(simNumChar);
    }
    return simNumStr;
}

/**
 * \return a pair of (simName_r_simNumber, simName)
 */
pair<string, string> Globals::calcSimName(string nameBegin)
{
    int scenarioNum = calcScenarioNumber();
    string simName = nameBegin + '_' + boost::lexical_cast<string>(NUM_RESOURCES)
            + '_' + boost::lexical_cast<string>(NUM_AGENTS)
            + '_' + boost::lexical_cast<string>(NUM_DAYS)
            + '_' + boost::lexical_cast<string>(scenarioNum);
    string simNumber = calcSimNumber(simName);
    pair<string, string> result = make_pair(simName + '_' + 'r' + simNumber, simName);
    return result;
}

/**
 * Print the information in config file.
 */
void Globals::printConfig()
{
    cout << "Simulation variables:" << endl;
    cout << "Day length is " << DAY_LENGTH << endl;
    cout << "Number of days is " << NUM_DAYS << endl;
    cout << "Number of agents is " << NUM_AGENTS << endl;
    cout << "Number of resources is " << NUM_RESOURCES << endl << endl;
    cout << "Number of groups/types of agents is " << NUM_AGENT_GROUPS << endl;

    cout << "Trade Context variables: " << endl;
    cout << "Number of resource trade rounds is " << RES_TRADE_ROUNDS << endl;
    cout << "Epsilon of trade " << TRADE_EPSILON << endl;
    cout << "Number of resource trade attempts is " << RES_TRADE_ATTEMPTS << endl;
    cout << "Number of device trade rounds is " << DEVICE_TRADE_ROUNDS << endl;
    cout << "Number of device trade attempts is " << DEVICE_TRADE_ATTEMPTS << endl;
    cout << "Menu size is " << MENU_SIZE << endl << endl;

    cout << "Input booleans " << endl;
    cout << "A resource will be removed: " << REMOVE_RES << endl;
    cout << "Trade exists: " << TRADE_EXISTS << endl;
    cout << "Devices exist: " << DEVICES_EXIST << endl << endl;

    cout << "Agent Capacity variables: " << endl;
    cout << "Device trade memory length is " << DEVICE_TRADE_MEMORY_LENGTH << endl;
    cout << "Device production memory length is " <<
        DEVICE_PRODUCTION_MEMORY_LENGTH << endl;
    cout << "Daily experience loss for idle resources is " << DAILY_EXP_PENALTY << endl;
    cout << "Maximum resource experience is " << MAX_RES_EXPERIENCE << endl;
    cout << "Minimum effort that can go into a resource is " << MIN_RES_EFFORT << endl;
    cout << "Days of device to hold " << DAYS_OF_DEVICE_TO_HOLD << endl << endl;

    cout << "Extraction Efficiency variables: " << endl;
    cout << "Factor of a tool " << TOOL_FACTOR << endl;
    cout << "Lifetime of a tool " << TOOL_LIFETIME << endl;
    cout << "Factor of a machine " << MACHINE_FACTOR << endl;
    cout << "Lifetime of a machine " << MACHINE_LIFETIME << endl;
    cout << "Factor of a factory " << FACTORY_FACTOR << endl;
    cout << "Lifetime of a factory " << FACTORY_LIFETIME << endl;
    cout << "Factor of a industry " << INDUSTRY_FACTOR << endl;
    cout << "Lifetime of a industry " << INDUSTRY_LIFETIME << endl;
    cout << "Factor of a device machine " << DEV_MACHINE_FACTOR << endl;
    cout << "Lifetime of a device machine " << DEV_MACHINE_LIFETIME << endl;
    cout << "Factor of a device factory " << DEV_FACTORY_FACTOR << endl;
    cout << "Lifetime of a device factory " << DEV_FACTORY_LIFETIME << endl << endl;

    cout << "Production Efficiency variables: " << endl;
    cout << "Number of resources to make a tool is " << RESOURCES_IN_TOOL << endl;
    cout << "Experience the inventor gains for the invention is " << INVENTOR_DEVICE_EXPERIENCE << endl;
    cout << "Number of components that go into a device is " << NUM_DEVICE_COMPONENTS << endl;
    cout << "Maximum experience someone can have in a device is " << MAX_DEVICE_EXPERIENCE << endl;
    cout << "Amount of decay devices experience on a daily basis " << DAILY_DEVICE_DECAY << endl;
    cout << "Production Epsilon is " << PRODUCTION_EPSILON << endl << endl;

    cout << "Innovation Volume/Speed variables: " << endl;
    cout << "Minimum resources required for device construction is " << MIN_RES_HELD_FOR_DEVICE_CONSIDERATION << endl;
    cout << "Minimum device experience for an agent that holds the device is " << MIN_HELD_DEVICE_EXPERIENCE << endl;
    cout << "Minimum devices made recently to consider obtaining a device making device is " << MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION << endl << endl;

}

