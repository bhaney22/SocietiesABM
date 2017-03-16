#ifndef _SOC_AGENT_H_
#define _SOC_AGENT_H_

#include <vector>
#include "properties.h"
#include "globals.h"
using namespace std;

/**
 * \struct MaxInfo
 */
struct MaxInfo {
	double maxGain;         //!< the most utility gain the devices of a type can get
	double maxIndex;        //!< the index of the device
	double costOfMax;       //!< the cost of the device
	double benefitOfMax;    //!< the benefit of the device
};

class Device;

class Agent
{
private:
    void initializeAll(int number, vector< vector<double> > agentValues);
    void defineProperties();

public:
    /**
     * A vector of vectors of 9 values.
     * The outer vector is one per resource.
     * The inner vector's values are:
     *    steepness, scaling, minResEff, maxResEff, maxResExp, minDevEff, maxDevEff, maxDevExp, lifetime
     */
    vector< vector<double> > personalValues;
    vector<ResProperties> resProp;              //!< will be size glob.NUM_RESOURCES
    vector< vector<DevProperties> > devProp;    //!< indexed by device type, then resId
    bool inSimulation;              //!< a boolean to indicate whether the agent is in simulation
    int name;                       //!< the name / number of agent

    /**
     * The trade power for resources for this agent.
     * default is 1; set in the csv file used for heterogeneous agents.
     */
    double resTradePower;
    /**
     * The trade power for devices for this agent.
     * Set in the csv file used for heterogeneous agents.
     */
    double devTradePower;
    /**
     * The power for the agent to invent a device.
     * Set in the csv file used for heterogeneous agents.
     */
    double inventSpeed;

//    double patent;        //!< not used yet; for monopoly.
    int group;              //!< the number of group the agent is in; set in the csv file used for heterogeneous agents.

    /**
     * penalty is the amount of experience lost from work experience on
     * a resource every day that the agent does not work on that
     * resource.
     */
    double penalty;
    /**
     * When valuing tools for trade, agents use the gain per minute (in
     * terms of utility) that they were getting at the end of the day to
     * calculate opportunity costs.
     */
    double endDayGPM;
    /**
     * When deciding how many tools an agent can make, agents keep track
     * of how much time it must set aside to build those tools.
     */
    double setAsideTime;
    /**
     * When an agent sells a tool that it doesn't own, it works
     * "overtime" and that time is subtracted from the next work day.
     */
    double overtime;
    /**
     * Each element in these lists is a list of the five most
     * recent device trade surpluses with each other agent.
     * indexed by [glob.NUM_DEVICE_TYPES][glob.NUM_AGENTS][glob.DEVICE_TRADE_MEMORY_LENGTH]
     */   
    vector< vector< vector<double> > > agentDeviceTradeMemory;
    /**
     * Occasionally, agents will continually trade a device back and
     * forth for the same price in an infinite loop, this is a list of
     *  the devices bought in a given round that the agent checks to
     *  prevent constant trade back and forth.
     */
    vector<int> deviceBoughtThisRound;

    /**
     * Tracks which agents this agent has traded devices with in the
     * given round of device trading.
     */
    vector<Agent *> tradedDeviceWithThisRound;
    /**
     * Keeps track of the number of units of all resources this agent
     * has sold.
     */
    int unitsSoldToday;
    int unitsSoldForDevicesToday; //!< the number of units of all resources sold for devices today.
    /**
     * Keeps track of the number of units of all resources this agent has sold
     * if its partner is from another group.
     * unitsSoldWinthinGroupToday = unitsSoldToday - unitsSoldCrossGroupToday;
     * Don't need another variable for that.
     */
    int unitsSoldCrossGroupToday;
    /**
     * Keeps track of the number of units of all resources this agent has sold for devices
     * if its partner is from another group.
     */
    int unitsSoldCrossGroupForDevicesToday;

    /**
     * Keeps track of the number of units of all resources this agent
     * has produced.
     * unitsGatheredWithDeviceToday is indexed by TOOL through INDUSTRY.
     * We'll still make 6 vectors, but will only fill in the 2 vectors inside of
     * those for the first 4.
     */
    vector<vector<int> > unitsGatheredWithDeviceToday;
    /**
     * unitsExtractedWithDeviceToday is indexed by TOOL through INDUSTRY.
     * We'll still make 6 vectors, but will only fill in the 2 vectors inside of
     * those for the first 4.
     * sizes of the vectors are: [glob.NUM_DEVICE_TYPES][glob.NUM_RESOURCES]
     */
    vector< vector<int> > unitsExtractedWithDeviceToday;
    /**
     * Each element in these lists is the number of devices made of the
     * corresponding resource.
     * devicesMadeWithDevDevicesToday is indexed by DEVMACHINE and
     * DEVFACTORY only.  We'll still make 6 vectors, but will only fill in
     * the 2 vectors inside of those for DEVMACHINE and DEVFACTORY.
     * sizes are: [glob.NUM_DEVICE_TYPES][glob.NUM_RESOURCES]
     */
    vector< vector<int> > devicesMadeWithDevDevicesToday;
    /**
     * Each element in this list is the number of minutes used of the
     * device of the corresponding resource. 
     * size is [glob.NUM_DEVICE_TYPES]
     */
    vector<double> timeSpentMakingDevicesToday;
    /**
     * Each element in this list is the number of minutes used of the
     * device of the corresponding resource. 
     * size is [glob.NUM_DEVICE_TYPES]
     * timeSpentGatheringWithDeviceToday is indexed by TOOL through INDUSTRY.
     * We'll still make 6 vectors, but will only fill in the 2 vectors inside of
     * those for the first 4.
     */
    vector<double> timeSpentGatheringWithDeviceToday; 
    /**
     * The number of minutes used to gather resources without using devices.
     */
    double timeSpentGatheringWithoutDeviceToday;
    /**
     * The sum of the utilities of all units of all resources held by
     * this agent in a given day.
     */
    double utilityToday;
    /**
     * The sum of the utilities of all resources gained by this agent
     * by selling devices.
     */
    double utilGainThroughDevSoldToday;

    Agent(int number, vector< vector<double> > agentValues);
    double utilCalc(int resIndex) const;
    double tempUtilCalc(int resIndex, int change) const;
    int resHeld(int resIndex) const; 
    double utilPerEffort(int resIndex) const;
    int getHeld (int resId) const;
    int getUnitsGatheredToday(int resId) const;
    int getUnitsGatheredWithDevice(int device, int resId) const;
    int getDevicesMadeToday(int deviceIndex, int deviceType) const;
    device_name_t bestDevice(int resIndex) const;
    device_name_t bestDevDevice(device_name_t device, int deviceIndex) const;
    void workDay();
    // vtn2: note: A better name would probably be extractWithDevice.
    void deviceUse(device_name_t device, int deviceIndex, double timeChange);
    bool resBundleHeldCheck(vector<int> bundle) const;
    void toolInvention();
    void deviceInvention(device_name_t device, device_name_t componentType);
    void makeDevice(int deviceIndex, device_name_t device);
    void work(int resIndex, device_name_t bestDevice);
    void workStatsUpdate(int resIndex, device_name_t bestDevice, double workTime);
    void deviceStatsUpdate(int deviceIndex, device_name_t device,
                           device_name_t bestDevDevice, double timeUse);
    void workDayEnd();
    void endDayChecks();
    void decay();
    void updateDeviceComponentExperience();
    double effortCalc(int resIndex) const;
    double tempEffortCalc(int resIndex, double change) const;
    double deviceEffortCalc(int deviceIndex, device_name_t device);
    double tempDeviceEffortCalc(int deviceIndex, device_name_t device,
                                       double change);
    double deviceHeldForRes(int resIndex) const;
    double devDeviceHeldForRes(device_name_t type, int resIndex) const;
    double sellerDeviceValue(int resIndex, device_name_t device);
    double buyerDeviceValue(int resIndex, device_name_t device);
    vector<int> preferredDeviceTraders(device_name_t device);
    void buys(int resIndex, int amount);
    void sells(int resIndex, int amount);
    void getBackRes(int toolIndex, device_name_t bestDevDevice);
    void getBackDeviceComponents(device_name_t device, int deviceIndex);
    void setAsideRes(int toolIndex, device_name_t bestDevDevice);
    void setAsideDeviceComponents(device_name_t device, int deviceIndex);
    pair<bool, double> canBuy(Device &device);	// seems like this is the only thing that takes a Device object as a parameter...  should we fix this?
    void buysDevice(int deviceIndex, device_name_t device);
    void sellsDevice(int deviceIndex, device_name_t device);
    double utilityHeld(int resIndex);
    double costOfResourceBundle(vector<int> &resourceBundle) const;
    double gainOfResourceBundle(vector<int> &resourceBundle) const;
    double myAccumulate(int resIndex, int change) const;
    double barterUtility(int resIndex, int change) const;
    double deviceCurrentlyHeldForResource(int resIndex, device_name_t device) const;
    void personalDevices(device_name_t device);
    void deviceProduction();
    void calcUtilityToday();
    void newDeviceTrade (device_name_t device);
    void resetDeviceGainAndCostMemory();
    void resetTodayStats();
    void remove();
    int calcMinHeld();
    int calcMaxHeld();
    MaxInfo calcMaxDeviceGainAndIndex(device_name_t deviceType);
    void logAgentData();
};


/**
 * \return the number of units an agent holds of a given resource.
 */
inline int Agent::resHeld(int resIndex) const
{
    return resProp[resIndex].getHeld();
}

/**
 * \return the utility per minute of producing a given resource.
 */
inline double Agent::utilPerEffort(int resIndex) const
{
    return utilCalc(resIndex) / effortCalc(resIndex);
}

/**
 * \return the minutes held of the device-making device of the given type for the given resource.
 */
inline double Agent::devDeviceHeldForRes(device_name_t type, int resIndex) const
{
    return devProp[type][resIndex].deviceHeld;
}

/**
 * When an agent gets a resource from a trade it is considered
 * "bought", the unit is added to its "held".
 */
inline void Agent::buys(int resIndex, int amount)
{
    resProp[resIndex].setHeld(resProp[resIndex].getHeld() + amount);
}

/**
 * When an agent gives up a unit from trade, it is considered "sold".
 */
inline void Agent::sells(int resIndex, int amount)
{
    resProp[resIndex].setHeld(resProp[resIndex].getHeld() - amount);
}

/**
 * Calculates the sums of the marginal utilities of the units of a
 * given resource (essentially the integral of the MU function from 0
 * to the number of units held).
 */
inline double Agent::utilityHeld(int resIndex)
{
    if (resHeld(resIndex) == 0) {
        return 0.0;
    }
    return barterUtility(resIndex, - resHeld(resIndex));
}

/**
 * \return the marginal value of the next unit of a given resource based off of the previously calculated list of utilities.
 */
inline double Agent::utilCalc(int resIndex) const
{
    int i = resHeld(resIndex);
    if (i < (int) resProp[resIndex].marginalUtilities.size()) {
        return resProp[resIndex].marginalUtilities[i];
    }
    return glob.MIN_RES_UTIL;
}

/**
 * When considering buying or selling, each agent has to consider
 * what the marginal utility of a resource would be given that they
 * add or subtract a certain number of units of that resource.
 */
inline double Agent::tempUtilCalc(int resIndex, int change) const
{
    /*
     * Returns the marginal value of a resource given a specified change in
     * inventory without actually changing the inventory.
     */
    int i = resHeld(resIndex) + change;
    if (i < (int) resProp[resIndex].marginalUtilities.size()) {
        return resProp[resIndex].marginalUtilities[i];
    }
    return glob.MIN_RES_UTIL;
}


/**
 * Calculates required effort by consulting the universal efforts
 * list.
 */
inline double Agent::effortCalc(int resIndex) const
{
    int temp = (int) resProp[resIndex].experience;
    /*
     * PROBLEM: to be the same with python, should be:
     * Gives an error if effortCalc() uses glob and tempEffortCalc() doesn't use glob.
     * Doesn't give an error if they both use resProp instead of glob.
     */
     if (temp < (int) resProp[resIndex].resEfforts.size()) {
         return resProp[resIndex].resEfforts[temp];
     }
     return resProp[resIndex].minResEffort;
}


/**
 * When considering making, buying, or selling a device, each agent
 * has to consider what the effort of a resource would be given that
 * they add a certain amount of experience at that resource
 * Returns the effort of getting a resource given a specified
 * change in experience without actually changing the experience.
 */
inline double Agent::tempEffortCalc(int resIndex, double change) const
{
    int temp = (int) (resProp[resIndex].experience + change);
    if (temp < (int) resProp[resIndex].resEfforts.size()) {
    	return resProp[resIndex].resEfforts[temp];
    }
    return resProp[resIndex].minResEffort;
}


/**
 * Returns the number of minutes required to make a tool based off
 * of the agent's experience.
 */
inline double Agent::deviceEffortCalc(int deviceIndex, device_name_t device)
{
    int temp = (int) devProp[device][deviceIndex].getDeviceExperience();
    if (temp < (int) devProp[device][deviceIndex].deviceEfforts.size()) {
    	return devProp[device][deviceIndex].deviceEfforts[temp];
    }
    return devProp[device][deviceIndex].minDeviceEffort;
}


/**
 * Returns the effort of making a device given a specified change
 * in experience without actually changing the experience.
 */
inline double Agent::tempDeviceEffortCalc(int deviceIndex, device_name_t device,
                                          double change)
{
    int temp = (int) (devProp[device][deviceIndex].getDeviceExperience() + change);
    if (temp < (int) devProp[device][deviceIndex].deviceEfforts.size()){
    	return devProp[device][deviceIndex].deviceEfforts[temp];
    }
    return devProp[device][deviceIndex].minDeviceEffort;
}

/**
 * If the device is a device-making device, return the minutes held of
 * that type of device for the given resId; if the device is a resource-
 * extracting device, return the minutes held of all resource-extracting
 * devices for the given resId.
 */
inline double Agent::deviceCurrentlyHeldForResource(int resIndex, device_name_t device) const
{
    if (device == DEVMACHINE || device == DEVFACTORY) {
        return devDeviceHeldForRes(device, resIndex);
    }
    return deviceHeldForRes(resIndex);
}

#endif
