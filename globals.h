#ifndef _SOC_GLOBALS_
#define _SOC_GLOBALS_

#include <iostream>
#include <vector>
#include <map>
#include "resource.h"

/**
 * \def DONT_RANDOMIZE
 * When not commented (turned on), the will choose first agent at index 0 when generating a pair of agents,
 * and choose resource[0].
 * Complete nonrandom when there are only two agents.
 */
//#define DONT_RANDOMIZE 1

using namespace std;

/**
 * \enum device_name_t
 * Define a enum of device types.
 */
enum device_name_t {
    TOOL = 0,
    MACHINE,	/*!< 1 */
    FACTORY,	/*!< 2 */
    INDUSTRY,	/*!< 3 */
    DEVMACHINE,	/*!< 4 */
    DEVFACTORY, /*!< 5 */
    NO_DEVICE	/*!< 6 */
};

/*!< names corresponding to the enum above. */
extern string device_names[];

#define NUM_DEVICE_TYPES	6               /*!< \def NUM_DEVICE_TYPES */
#define NUM_RESOURCE_GATHERING_DEVICES	4   /*!< \def NUM_RESOURCE_GATHERING_DEVICES */

/**
 * \struct ResValues
 * Contains steepnessFactor and scalingFactor of resources.
 */
struct ResValues {
    double steepnessFactor;
    double scalingFactor;
};

class Agent;
class Device;
class ResourceMarketplace;
//class Statstracker;
class ProductionStats;
class TradeStats;
class OtherStats;
class DeviceMarketplace;



class Globals
{
public:
    /**
     * TO memorize the used sub-set values of marginal utilities.
     * Used in agent::barterUtility.
     * Map a pair of <int, int> to a double.
     * Indexed by NUM_AGENT_GROUPS.
     * Each group of agents has its own memoMU.
     */
    vector< map<pair<int,int>,double> > memoMUs;
    /**
     * Values for command-line options.
     */

	string configName;          	// *** initialized in parse_args() This just has the name of the config (ex. test1)
	string configFilename;          // *** initialized in parse_args() This has the path and extenstion of the config, ex. Configs/test1.conf
    string configAgentFilename;     // *** initialized in parse_args()
    string configAgentCSV;          // *** initialized in parse_args()
    bool   saveFileFolderSet;       // *** initialized in parse_args()
    string saveFileFolder;          // *** initialized in parse_args()
	bool   saveInDatabase; 	        // *** initialized in parse_args()
	string UniqueKey;         	    // *** initialized in parse_args()
    bool   simTitleSet;             // *** initialized in parse_args()
    string simTitle;                // *** initialized in parse_args()
    bool   randomSeedSet;           // *** initialized in parse_args()
    int    randomSeed;              // *** initialized in parse_args()
    bool   removeAgentMidRun;       // *** initialized in parse_args()
    int    removeAgentId;           // *** initialized in parse_args()
    int    removeAgentDay;          // *** initialized in parse_args()
    bool   removeResMidRun;         // *** initialized in parse_args()
    int    removeResId;             // *** initialized in parse_args()
    int    removeResDay;            // *** initialized in parse_args()
    bool   removeResHoldings;       // *** initialized in parse_args()
    bool   saveInMiddle;            // *** initialized in parse_args()
    string saveInMiddleFoldername;  // *** initialized in parse_args()
    int    saveInMiddleDay;         // *** initialized in parse_args()
    int    verboseLevel;            // *** initialized in parse_args()
    bool   saveExchangeRateData;    // *** initialized in parse_args()
    bool   graphSet;                // *** initialized in parse_args()
    bool   noRunSet;                // *** initialized in parse_args()

    
    
    /**
     * These values are read from the config file.
     */
    int START_DAY;                      // *** initialized in parse_args()
    int DAY_LENGTH;                     // *** initialized in parse_args()
    int NUM_DAYS;                       // *** initialized in parse_args()
    int NUM_AGENTS;                     // *** initialized in parse_args()
    int NUM_RESOURCES;                  // *** initialized in parse_args()

    /**
     * The number of groups/types of agents.
     * For heterogeneous, it needs to be consistent with the csv file used to generate aconf ile.
     * For homogeneous, it should be set to 1.
     * Initialized in parse_args().
     */
    int NUM_AGENT_GROUPS;

    int RES_TRADE_ROUNDS;               // *** initialized in parse_args()
    int RES_TRADE_ATTEMPTS;             // *** initialized in parse_args()
    int DEVICE_TRADE_ROUNDS;            // *** initialized in parse_args()
    int DEVICE_TRADE_ATTEMPTS;          // *** initialized in parse_args()
    int MENU_SIZE;                      // *** initialized in parse_args()
    int DEVICE_TRADE_MEMORY_LENGTH;     // *** initialized in parse_args()
    int DEVICE_PRODUCTION_MEMORY_LENGTH;      // *** initialized in parse_args()
    int MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION;    // *** initialized in parse_args()
    int MIN_RES_HELD_FOR_DEVICE_CONSIDERATION;      // *** initialized in parse_args()
    double DAILY_EXP_PENALTY;           // *** initialized in parse_args()
    double PRODUCTION_EPSILON;          // *** initialized in parse_args()
    int RESOURCES_IN_TOOL;              // *** initialized in parse_args()
    double MAX_RES_EXPERIENCE;          // *** initialized in parse_args()
    double INVENTOR_DEVICE_EXPERIENCE;  // *** initialized in parse_args()
    int NUM_DEVICE_COMPONENTS;          // *** initialized in parse_args()
    double MAX_DEVICE_EXPERIENCE;       // *** initialized in parse_args()
    double DAILY_DEVICE_DECAY;          // *** initialized in parse_args()

    double MIN_HELD_DEVICE_EXPERIENCE;  // *** initialized in parse_args()
    double MAX_RES_EFFORT;              // *** initialized in parse_args()
    double MIN_RES_EFFORT;              // *** initialized in parse_args()
    double MAX_DEVICE_EFFORT;           // *** initialized in parse_args()
    double MIN_DEVICE_EFFORT;           // *** initialized in parse_args()
    double MIN_RES_UTIL;                // *** initialized in parse_args()
    double TRADE_EPSILON;               // *** initialized in parse_args()
    double TOOL_PROBABILITY_FACTOR;     // *** initialized in parse_args()
    double DEVICE_PROBABILITY_FACTOR;   // *** initialized in parse_args()
    double TOOL_FACTOR;                 // *** initialized in parse_args()
    double TOOL_LIFETIME;               // *** initialized in parse_args()
    double MACHINE_FACTOR;              // *** initialized in parse_args()
    double MACHINE_LIFETIME;            // *** initialized in parse_args()
    double FACTORY_FACTOR;              // *** initialized in parse_args()
    double FACTORY_LIFETIME;            // *** initialized in parse_args()
    double INDUSTRY_FACTOR;             // *** initialized in parse_args()
    double INDUSTRY_LIFETIME;           // *** initialized in parse_args()
    double DEV_MACHINE_FACTOR;          // *** initialized in parse_args()
    double DEV_MACHINE_LIFETIME;        // *** initialized in parse_args()
    double DEV_FACTORY_FACTOR;          // *** initialized in parse_args()
    double DEV_FACTORY_LIFETIME;        // *** initialized in parse_args()
    double DAYS_OF_DEVICE_TO_HOLD;      // *** initialized in parse_args()
    bool TRADE_EXISTS;                  // *** initialized in parse_args()
    bool DEVICES_EXIST;                 // *** initialized in parse_args()
    bool TOOLS_ONLY;                    // *** initialized in parse_args()

    /**
     * Advanced options, settable via command line or via GUI.
     */
    bool REMOVE_RES;            // *** initialized in constructor and setAdvancedOptions()
    int RES_TO_REMOVE;          // *** initialized in setAdvancedOptions()
    int REMOVE_RES_DAY;         // *** initialized in setAdvancedOptions()
    bool ELIMINATE_RESERVES;    // *** initialized in setAdvancedOptions()
    bool REMOVE_AGENT;          // *** initialized in constructor and setAdvancedOptions()
    int AGENT_TO_REMOVE;        // *** initialized in setAdvancedOptions()
    int REMOVE_AGENT_DAY;       // *** initialized in setAdvancedOptions()
    bool END_SAVE;              // *** initialized in constructor and setAdvancedOptions()
    string SAVE_FOLDER;         // *** initialized in setAdvancedOptions()
    bool SAVE_DAY_STATUS;           // *** initialized in constructor
    string DAY_STATUS_SAVE_FOLDER;  // *** initialized in setAdvancedOptions()
    int DAY_FOR_SAVE;               // *** initialized in setAdvancedOptions()
    string DAY_STATUS_LOAD_FOLDER;  // *** initialized in initGlobalStructures()
    string SIM_NAME;            // *** initialized in constructor and setAdvancedOptions() and reinitialize()
    string SIM_SAVE_FOLDER;     // *** initialized in setAdvancedOptions() and reinitialize()
    bool SAVE_TRADES;           // *** initialized in setAdvancedOptions()
    bool PARALLEL_TRADES;       // *** initialized in constructor

    /* "Global" constants that are data structures and computed
       at start-up time. */
    vector<ResValues> RES_VALUES;   // *** initialized in initGlobalStructures()
    vector<int> RES_DECAY_SLOWNESS; // *** initialized in initGlobalStructures()

    // indexed by device_name_t
    vector<double> EXPERIENCE_FOR_MAKING;   // *** initialized in initGlobalStructures()
    vector<double> RES_IN_DEV;		        // *** initialized in initGlobalStructures()

    /**
     * Number of agents in each agent group.
     * Indexed by agent type; initialized in initializeAgents()
     */
    vector<int> NUM_AGENTS_IN_GROUP;

    /**
     * Number of active agents in each agent group.
     * Indexed by agent group; initialized in initializeAgents()
     */
    vector<int> NUM_ACTIVE_AGENTS_IN_GROUP;

    /* "Global" variables -- not "constants" like all the above. */
    ResourceMarketplace *resourceMarket;    // *** initialized in setGlobalMarketPlaces()
    DeviceMarketplace *deviceMarket;        // *** initialized in setGlobalMarketPlaces()
    ProductionStats *productionStats;       // *** initialized in setGlobalStats()
    TradeStats *tradeStats;                 // *** initialized in setGlobalStats()
    OtherStats *otherStats;                 // *** initialized in setGlobalStats()

    int currentDay;     // *** the number of current day; initialized in main()

    int activeAgents;   // *** initialized in initializeAgents()
    vector<Agent *> agent;  // *** initialized in initializeAgents()

    // *** initialized in initGlobalStructures()
    vector< vector<Device *> > discoveredDevices;  // indexed by device_name_t and then deviceIdx.

    vector<Resource> res;       // *** initialized in initGlobalStructures()

    // Note: global updatedAgents in config.py not used anywhere...

    time_t	startTime, endTime; // *** initialized in startTimer() and endTimer()
//	this is used throughout the program, primarily in statstracker to serve as a place holder that will get filled eventually.
    vector<int> EMPTY_VECTOR_OF_INTS;
    vector<vector<int> > EMPTY_VECTOR_OF_VECTORS_OF_INTS;

    /*
     * To record day, month, year.
     */
    string day;     // *** initialized in setAdvancedOptions()
    string month;   // *** initialized in setAdvancedOptions()
    string year;    // *** initialized in setAdvancedOptions()

    Globals();	// constructor
    void initGlobalStructures();
    void initializeAgents();
    void setAdvancedOptions();
    void perRunInitialization();
    void setGlobalMarketPlaces();
    void setGlobalStats();
    void printConfig();
    void startTimer();
    void endTimer();
    void removeAgent(int agentNumber, int day);
    void removeRes(int resNumber, int day);
    void reinitialize();
    int calcScenarioNumber();
    string calcSimNumber(string simName);
    pair<string, string> calcSimName(string nameBegin);
    double random_range(double lower, double upper);
    double random_01();
    int random_int(int lower, int upper);
    int random_int_inclusive(int lower, int upper);
    int random_choice(vector<int> &vec);
    int random_binomial(int n, double p);
    int getNumDeviceTypes() {return NUM_DEVICE_TYPES;};
    int getNumResGatherDev() {return NUM_RESOURCE_GATHERING_DEVICES;};
    void createDirectory(string path);

};

extern Globals glob;


#endif
