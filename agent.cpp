/*
 * agent.cpp
 * Contains the definition of the Agent class.
 * Each agent is an individual in the simulation and has some properties that can affect their behavior.
 * The agents can be either homogeneous or heterogeneous.
 * If they are homogeneous, the properties can be defined in .config files.
 * If they are heterogeneous, the properties can be defined in .csv files.
 */

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <numeric>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "globals.h"
#include "agent.h"
#include "device.h"
#include "logging.h"
using namespace std;

/**
 * Agent constructor.
 * The code simply calls initializeAll().
 * \param number unique identifier for this agent
 * \param agentValues a vector of vectors of 9 doubles. The outer vector is one per resource. The inner vector's values are: steepness, scaling, minResEff, maxResEff, maxResExp, minDevEff, maxDevEff, maxDevExp, lifetime
 */
Agent::Agent(int number, vector< vector<double> > agentValues)
{
    initializeAll(number, agentValues);
}


/**
 * Initialize all data for an agent instance.
 * \see Agent constructor for info about parameters.
 */
void Agent::initializeAll(int number, vector< vector<double> > agentValues)
{
    personalValues = agentValues;
    resProp.clear();
    devProp.clear();
    inSimulation = true;
    name = number;
    penalty = glob.DAILY_EXP_PENALTY;
    endDayGPM = 0.0;
    setAsideTime = 0.0;
    overtime = 0.0;
    agentDeviceTradeMemory =
         vector< vector< vector<double> > >
        (NUM_DEVICE_TYPES, vector< vector <double> >
         (glob.NUM_AGENTS, vector<double>(glob.DEVICE_TRADE_MEMORY_LENGTH, 0.0)));
    deviceBoughtThisRound.clear();
    tradedDeviceWithThisRound.clear();

    defineProperties();

    unitsSoldToday = 0;
    unitsSoldCrossGroupToday = 0;
    unitsSoldForDevicesToday = 0;
    unitsSoldCrossGroupForDevicesToday = 0;

    unitsGatheredWithDeviceToday = vector< vector<int> >(NUM_DEVICE_TYPES);

    devicesMadeWithDevDevicesToday = vector< vector<int> >(NUM_DEVICE_TYPES);

    timeSpentMakingDevicesToday = vector<double>(NUM_DEVICE_TYPES, 0.0);
    timeSpentGatheringWithDeviceToday = vector<double>(NUM_DEVICE_TYPES, 0.0);

    timeSpentGatheringWithoutDeviceToday = 0.0;

    utilityToday = 0.0;

    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        for (int type = DEVMACHINE; type <= DEVFACTORY; type++) {
            devicesMadeWithDevDevicesToday[type].push_back(0);
        }
        for (int type = TOOL; type <= INDUSTRY; type++) {
            unitsGatheredWithDeviceToday[type].push_back(0);
        }
    }
}


/**
 * Define the values for ResProperties and DevProperties.
 *    steepness, scaling, minResEff, maxResEff, maxResExp, minDevEff,
 *    maxDevEff, maxDevExp, lifetime, resTradePower, devTradePower, patent, groupNumber.
 */
void Agent::defineProperties()
{
    /*
     * the tradePower and group for one agent is the same for all resources.
     * So only use the first one.
     */
    resTradePower = personalValues[0][9];
    devTradePower = personalValues[0][10];
    inventSpeed = personalValues[0][11];
    group = personalValues[0][13];

    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        ResProperties newResProp = ResProperties();
        newResProp.steepness = personalValues[resId][0];
        newResProp.scaling = personalValues[resId][1];

        newResProp.calcMarginalUtilities();

        newResProp.minResEffort = personalValues[resId][2];

        newResProp.maxResEffort = personalValues[resId][3];
        newResProp.maxResExperience = personalValues[resId][4];
        newResProp.calcResEfforts();
        
        newResProp.averageLifetime = personalValues[resId][8];
            
        resProp.push_back(newResProp);
    }

    for (int devnum = 0; devnum < NUM_DEVICE_TYPES; devnum++) {
        devProp.push_back(vector<DevProperties>());
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            DevProperties newDevProp = DevProperties();
            
            newDevProp.minDeviceEffort = personalValues[resId][5];
            newDevProp.maxDeviceEffort = personalValues[resId][6];
            newDevProp.maxDeviceExperience = personalValues[resId][7];
            newDevProp.calcDeviceEfforts();
            devProp[devnum].push_back(newDevProp);
        }
    }    
}

/**
 * accessor method
 *  \return the number of units an agent holds of a given resource.
 */
int Agent::getHeld(int resId) const
{
    return resProp[resId].getHeld();
}

/**
 * \return the number of units gathered today for resource resId
 */
int Agent::getUnitsGatheredToday(int resId) const
{
    return resProp[resId].unitsGatheredToday;
}

/**
 * \return the number of units gahtered with device for resource resId
 */
int Agent::getUnitsGatheredWithDevice(int device, int resId) const
{
    return unitsGatheredWithDeviceToday[device][resId];
}

/**
 * \return the number of deviceIndex of deviceType made today.
 */
int Agent::getDevicesMadeToday(int deviceIndex, int deviceType) const
{
    return devProp[deviceType][deviceIndex].devicesMadeToday;
}

/**
 * \param resIndex resource index
 * \return the best device available (i.e., held) for the given resource. return None if no device held.
 */
device_name_t Agent::bestDevice(int resIndex) const
{
    if (devProp[INDUSTRY][resIndex].deviceHeld > 0) {
        return INDUSTRY;
    } else if (devProp[FACTORY][resIndex].deviceHeld > 0) {
        return FACTORY;
    } else if (devProp[MACHINE][resIndex].deviceHeld > 0) {
        return MACHINE;
    } else if (devProp[TOOL][resIndex].deviceHeld > 0) {
        return TOOL;
    }
    return NO_DEVICE;
}

/**
 * \param device device type
 * \param deviceIndex device index
 * \return the best device-making device available (i.e., held) for the given device. Return None if no device-making device held.
 */
device_name_t Agent::bestDevDevice(device_name_t device, int deviceIndex) const
{
    if (device == TOOL) {
        if (devProp[DEVMACHINE][deviceIndex].deviceHeld > 0) {
            return DEVMACHINE;
        }
    } else if (device == MACHINE) {
        if (devProp[DEVFACTORY][deviceIndex].deviceHeld > 0) {
            return DEVFACTORY;
        }
    }
    return NO_DEVICE;
}

/**
 * Agents extract resources with their given time.
 */
void Agent::workDay()
{
    // time is the number of minutes the agent has worked this day
    double time = 0.0;

    // workIndex is the resId of the resource that the agent is currently
    // extracting
    int workIndex = 0;

    /*
     * maxUtilIndexes is the list of resIds within PRODUCTION_EPSILON of the
     * resource with the maximum utility per minute; these are the resources
     * that the agent will consider extracting.
     */
    vector<int> maxUtilIndexes;

    //valuePerEfforts is a list of the utility per minute of each resource
    vector<double> valuePerEfforts;

    /*
     * epsilon is the amount of utility per minute below the maximum utility
     * per minute that the agent is willing to consider.
     */
    double epsilon = glob.PRODUCTION_EPSILON;

    // Creates a list of marginal utilities per effort of each resource
    /*
     * Calculate utility per minute based off of agent utility curves, the
     * experience in each resource, and what resource-extracting devices the
     * agent holds
     */
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (glob.res[resId].inSimulation) {
            device_name_t bestDev = bestDevice(resId);
            if (bestDev != NO_DEVICE) {
                valuePerEfforts.push_back(utilPerEffort(resId) *
                                          glob.discoveredDevices[bestDev][resId]->deviceFactor);
            } else {
                valuePerEfforts.push_back(utilPerEffort(resId));
            }
        } else {
            valuePerEfforts.push_back(0.0);
        }

        /*
         * Store the resHeld of this agent's each resource before they work.
         * For printing data.
         */
        resProp[resId].beforeWorkHeld = resProp[resId].getHeld();
        resProp[resId].beforeWorkMU = utilCalc(resId);   // For printing out data

    }
    double maxUtilPerEffort = *max_element(valuePerEfforts.begin(), valuePerEfforts.end());

    // If the agent worked overtime on tools, it can't use that time during this day
    /*
     * overtime is the amount of time that the agent spent making devices on
     * this day, so the agent is not allowed to use that time to extract.
     * resources.
     */
    time += overtime;

    /*
     * If the agent spent the entire day making devices, its gain per minute
     * for the day is the utility gain from selling all of those devices
     * over the time it spent to make those devices.
     */
    if (time >= glob.DAY_LENGTH) {
        endDayGPM = utilGainThroughDevSoldToday / time;
        overtime -= glob.DAY_LENGTH;
    } else {
        overtime = 0.0;
    }

    /*
     * While the agent has time left in the day, it continues to extract
     * resources (note that this may allow agents to work slightly more than
     * the allotted number of minutes, but that effect is basically
     * inconsequential -- particularly in longer runs).
     */
    while (time < glob.DAY_LENGTH) {
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            /*
             * All work that would yield utility per effort within epsilon of the
             * max utility is put into a list. A random resource is chosen from that
             * list to work on.
             */
            if (valuePerEfforts[resId] >= (maxUtilPerEffort - epsilon) && glob.res[resId].inSimulation) {
                maxUtilIndexes.push_back(resId);
            }
        }

        // A random resource is chosen from that list to work on.
#ifdef DONT_RANDOMIZE
        workIndex = 0;
#else
        workIndex = glob.random_choice(maxUtilIndexes);
#endif
        device_name_t bestDev = bestDevice(workIndex);

        /*
         * Once the resource is decided, the agent calculates the time to
         * extract the resource based off its experience and the resource-
         * extracting devices it has
         */
        double timeChange;
        if (bestDev != NO_DEVICE) {
            timeChange = effortCalc(workIndex) /
                glob.discoveredDevices[bestDev][workIndex]->deviceFactor;
            deviceUse(bestDev, workIndex, timeChange);
        } else {
            timeChange = effortCalc(workIndex);
        }

        /*
         * The time used is incremented by the amount of time the agent used
         * to extract this resource
         */
        time += timeChange;

        /* The agents on the resource (gains a unit and gains experience). */
        work(workIndex, bestDev);
        workStatsUpdate(workIndex, bestDev, timeChange);

        /*
         * The gain per minute that the agent was getting is stored for
         * reference in device production and trading (the endDayGPM is
         * treated as the opportunity cost of time).
         */
        endDayGPM = valuePerEfforts[workIndex];

        /*
         * The utility per minute of the extracted resource is recalculated,
         * and the process begins again.
         */
        device_name_t nextBestDevice = bestDevice(workIndex);
        if (nextBestDevice != NO_DEVICE) {
            valuePerEfforts[workIndex] = utilPerEffort(workIndex) *
                glob.discoveredDevices[nextBestDevice][workIndex]->deviceFactor;
        } else {
            valuePerEfforts[workIndex] = utilPerEffort(workIndex);
        }
        maxUtilPerEffort = *max_element(valuePerEfforts.begin(), valuePerEfforts.end());
        maxUtilIndexes.clear();		// empty the vector.
    }

    /*
     * The work day ends with the agent getting an experience penalty in the
     * resources that it did not extract on this day.
     */
    workDayEnd();
}

/**
 * Removes the amount of time of the device used from the remaining
 * lifetime of the device.
 * \param device device type
 * \param deviceIndex  device index
 * \param timeChange the amount of time of the device used from the remaining lifetime of the device.
 */
void Agent::deviceUse(device_name_t device, int deviceIndex, double timeChange)
{
    DevProperties &dev = devProp[device][deviceIndex];
    dev.deviceHeld -= timeChange;
    if (dev.deviceHeld < 0) {
        dev.deviceHeld = 0;
    }
}

/**
 * Checks to make sure the agent has all the units of resources
 * required by the given resource bundle
 * \param bundle a vector of resource ids
 */
bool Agent::resBundleHeldCheck(vector<int> bundle) const
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (bundle[resId] > resProp[resId].getHeld()) {
            return false;
        }
    }
    return true;
}

/**
 * Every day, after resource trading and before tool trading,
 * agents have a chance to invent a tool.
 */
void Agent::toolInvention()
{
    /*
     * When inventing a tool, the agent will consider RESOURCES_IN_TOOL+1
     * resources; two of theses resources are the two resources in which
     * this agent has the most experience, and the rest are randomly chosen
     * from all of the resources that the agent holds at least
     * MIN_RES_HELD_FOR_DEVICE_CONSIDERATION units; consideredResources is
     * the list of the resIds of the considered resources.
     */
    vector<int> consideredResources;
    /*
     * consideredResourceExp is the list of the amount of experience that
     * the agent has in the considered resources.
     */
    vector<double> consideredResourcesExp;
    /*
     * experienceCheck is a list of the amount of experience that the agent
     * has in all of the resources.
     */
    vector<double> experienceCheck;
    /*
     *  heldResources is the list of resIds for resources that the agent
     * holds at least MIN_RES_HELD_FOR_DEVICE_CONSIDERATION units.
     */
    vector<int> heldResources;

    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        experienceCheck.push_back(resProp[resId].experience);
        if ( (resHeld(resId) > glob.MIN_RES_HELD_FOR_DEVICE_CONSIDERATION) && glob.res[resId].inSimulation) {
            heldResources.push_back(resId);
        }
    }
    /*
     * The two resources in which the agent has the most experience are
     * identified and saved in consideredResources.
     */
    for (int i = 0; i < 2; i++) {
        vector<double>::const_iterator maxExperience =
            max_element(experienceCheck.begin(), experienceCheck.end());
        int maxExperienceIndex = maxExperience - experienceCheck.begin();
        consideredResources.push_back(maxExperienceIndex);
        /*
         * The resources with the most experience are removed from the
         * heldResources list so that they do not get put into the
         * consideredResources list twice.
         */
        vector<int>::iterator idx =
            find(heldResources.begin(), heldResources.end(), maxExperienceIndex);
        if (idx != heldResources.end()) {	// it is found.
            heldResources.erase(idx);
        }
        // add the max value in the vector to the other vector
        consideredResourcesExp.push_back(*maxExperience);
        /*
         * experienceCheck of the first resource must be set to 0 so that
         * the resource with the second-most experience can be found
         */
        experienceCheck[maxExperienceIndex] = 0.0;
    }

    /*
     * If the agent has enough resources in heldResources to get to
     * RESOURCES_IN_TOOL + 1 resources in consideredResources, the agent
     * proceeds to randomly pick more resources to consider.
     */
    if ((int) heldResources.size() >= glob.RESOURCES_IN_TOOL - 1) {
        for (int i = 0; i < glob.RESOURCES_IN_TOOL - 1; i++) {
#ifdef DONT_RANDOMIZE
            int considered = 0;
#else
            int considered = glob.random_choice(heldResources);
#endif
            consideredResources.push_back(considered);
            consideredResourcesExp.push_back(resProp[considered].experience);
            heldResources.erase( find(heldResources.begin(), heldResources.end(), considered) );
        }
        sort(consideredResources.begin(), consideredResources.end());
    }

    /*
     * The agent checks to see which of the considered resources don't have
     * tools.
     */
    vector<int> noTool;
    for (vector<int>::iterator resId = consideredResources.begin();
         resId < consideredResources.end(); resId++) {
        if (glob.discoveredDevices[TOOL][*resId] == NULL || glob.discoveredDevices[TOOL][*resId]->agentsKnown() == 0) {
            noTool.push_back(*resId);
        }
    }

    /* 
     * If at least one of the considered resources doesn't have a tool,
     * the agent has a chance of inventing a tool based off of the sum
     * of the experiences in the considered resources. If they all have tools invented,
     * the agent goes on to try to invent a resource-extracting machine or a
     * device-making machine.
     */
    if (! noTool.empty()) {
        double sumExp = accumulate(consideredResourcesExp.begin(),
                                   consideredResourcesExp.end(), 0.0);
        /*
         * The probability of inventing a tool is directly relate to the sum
         * of the experiences that the agent has in the considered resources
         * and the geometric mean of each consideredResources toolProbabilityFactor.
         */
#ifdef DONT_RANDOMIZE
        double probability = 1;
#else
        double probability = glob.TOOL_PROBABILITY_FACTOR * sumExp * inventSpeed;
#endif
        double ran = glob.random_01();
        if ( ran < probability ) {
            /* 
             * If the agent successfully invents a tool, one of the
             * considered resources without a tool is randomly selected
             * to get the tool, and the other five resources become the
             * components of the new tool.
             */
#ifdef DONT_RANDOMIZE
            int toolUse = 0;
#else
            int toolUse = glob.random_choice(noTool);
#endif
            consideredResources.erase( find(consideredResources.begin(), consideredResources.end(), toolUse) );

            /*
             * All agents find out about the existence of the new tool (this
             * prevents invention of multiple tools for a single resource),
             * but all agents except the inventor are given zero experience,
             * so they cannot build this tool
             */
            if (glob.discoveredDevices[TOOL][toolUse] != NULL) {
                LOG(4) << "Creating new " << device_names[TOOL]
                       << " to replace existing tool "
                       << toolUse << " for which agentsKnown is 0.\n";
                delete glob.discoveredDevices[TOOL][toolUse];
            }
            glob.discoveredDevices[TOOL][toolUse] = new Tool(consideredResources, toolUse);
            for (int agId = 0; agId < glob.NUM_AGENTS; agId++) {
                glob.agent[agId]->devProp[TOOL][toolUse].setDeviceExperience(0.0);
                glob.agent[agId]->devProp[TOOL][toolUse].idleDevice = true;
            }
            /*
             * The inventor is given INVENTOR_DEVICE_EXPERIENCE experience,
             * so it is the only agent able to make the tool.
             */
            devProp[TOOL][toolUse].setDeviceExperience(glob.INVENTOR_DEVICE_EXPERIENCE);
            devProp[TOOL][toolUse].idleDevice = false;
            LOG(3) << "Tool for " << toolUse << " has been invented";
        }
    }
    else if (! glob.TOOLS_ONLY) {
    	/*
    	 * If all of the considered resources have tools, the agent randomly
         * goes on to either try to invent a resource-extracting machine or
         * try to invent a device-making-machine.
         */
        LOG(4) << "No tools to consider so what about MACHINES or DEVMACHINES?";
        if (glob.random_int_inclusive(0, 1) == 0) {
            deviceInvention(MACHINE, TOOL);
        } else {
            deviceInvention(DEVMACHINE, TOOL);
        }
    }
}

/**
 * Every day, after resource trading and before tool trading,
 * agents have a chance to invent a device
 * \param device the type of the device being invented
 * \param componentType the type of the component device
 */
void Agent::deviceInvention(device_name_t device, device_name_t componentType)
{
    /*
     * When inventing a higher-order device, agents will consider
     * NUM_DEVICE_COMPONENTS devices of the component type; the first two
     * will be the components in which the agent has the most experience,
     * and the remaining will be components in which the agent has any
     * experience, and if the agent does not have experience in enough
     * components, it will consider components that exist, but those in
     * which it has no experience (the comments will sometimes indicate that
     * there are resources being considered; the considered resources are
     * the resources for which the considered devices are useful).
     */
    vector<int> consideredDevices;
    /*
     * consideredDeviceExp is the list of the amount of experience that the
     * agent has in the considered devices.
     */
    vector<double> consideredDevicesExp;
    /*
     * experienceCheck is a list of the amount of experience that the agent
     * has in all of the devices of the component type.
     */
    vector<double> experienceCheck;
    /*
     * myKnown devices is a list of all the devices of the component type in
     * which the agent has experience.
     */
    vector<int> myKnownDevices;
    /*
     * knownDevices is a list of all the devices of the component type that
     * are known by anyone
     */
    vector<int> knownDevices;

    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        experienceCheck.push_back(devProp[componentType][resId].getDeviceExperience());
        if (devProp[componentType][resId].getDeviceExperience() > 0 && glob.res[resId].inSimulation) {
            myKnownDevices.push_back(resId);
        }
        /*
         * A device is considered 'known' if it has been invented and if at
         * least one agent has experience with it.
         */
        else if (glob.res[resId].inSimulation){
            if ( (glob.discoveredDevices[componentType][resId] != NULL) &&
                 (glob.discoveredDevices[componentType][resId]->agentsKnown() > 0) ) {
                knownDevices.push_back(resId);
            }
        }
    }
    /*
     * To verify that the agent will be able to invent a device, the agent
     * checks to see if it has experience in any of the devices of the
     * component type; if the agent does have enough experience, it proceeds with
     * invention.
     */
    bool hasEnoughExp = false;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (experienceCheck[resId] > 0) {
            hasEnoughExp = true;
            break;
        }
    }
    /*
     * If the agent has experience in any component devices and enough
     * component devices are known to combine into a higher-order device,
     * the agent proceeds.
     */
    if (hasEnoughExp && ( (myKnownDevices.size() + knownDevices.size()) >=
                   (unsigned int) glob.NUM_DEVICE_COMPONENTS) ) {
    	/*
    	 * The two devices in which the agent has the most experience are
         * identified and saved in consideredDevices.
         */
        for (int i = 0; i < 2; i++) {
            vector<double>::const_iterator maxExp =
                max_element(experienceCheck.begin(), experienceCheck.end());
            // subtract beginning from maxExp to get offset into the "array".
            int maxExpOffset = maxExp - experienceCheck.begin();
            if (maxExpOffset > 0) {
                consideredDevices.push_back(maxExpOffset);
                vector<int>::iterator idx =
                    find(myKnownDevices.begin(), myKnownDevices.end(), maxExpOffset);
                /*
                 * The devices that are added to consideredDevices are
                 * removed from myKnownDevices and knownDevices so that they
                 * are not inserted into consideredDevices more than once.
                 */
                if (idx != myKnownDevices.end()) {  // if idx in myKnownDevices
                    myKnownDevices.erase(idx);
                }
                idx = find(knownDevices.begin(), knownDevices.end(), maxExpOffset);
                if (idx != knownDevices.end()) {    // if idx in knownDevices
                    knownDevices.erase(idx);
                }
                // add the max value in the vector to the other vector
                consideredDevicesExp.push_back(*maxExp);
                /*
                 * experienceCheck of the first device must be set to 0 so
                 * that the device with the second-most experience can be
                 * found.
                 */
                experienceCheck[maxExpOffset] = 0.0;
            }
        }
        /* Until the agent is considering the correct number of components,
         * it continues to add devices to the list.
         */
        while (consideredDevices.size() < (unsigned int) glob.NUM_DEVICE_COMPONENTS) {
            /*
             * If the agent has experience in any devices that are not yet
             * being considered, a device is randomly selected from the list
             * of known devices.
             */
            if (! myKnownDevices.empty()) {
                int considered = glob.random_choice(myKnownDevices);
                consideredDevices.push_back(considered);
                consideredDevicesExp.push_back(devProp[componentType][considered].getDeviceExperience());
                myKnownDevices.erase( find(myKnownDevices.begin(), myKnownDevices.end(), considered) );

                /*
                 * If the agent doesn't have any experience in any of the
                 * devices that have not been considered, a device is randomly
                 * selected from the list of devices that exist but are not
                 * known by the agent.
                 */
            } else {
                int considered = glob.random_choice(knownDevices);
                consideredDevices.push_back(considered);
                consideredDevicesExp.push_back(devProp[componentType][considered].getDeviceExperience());
                knownDevices.erase( find(knownDevices.begin(), knownDevices.end(), considered) );
            }
        }
        sort(consideredDevices.begin(), consideredDevices.end());
    }
    /*
     * The agent checks to see which of the considered resources don't have
     * a device.
     */
    vector<int> noDevice;
    for (vector<int>::iterator it = consideredDevices.begin();
         it != consideredDevices.end(); it++) {
        if (glob.discoveredDevices[device][*it] == NULL ||
            glob.discoveredDevices[device][*it]->agentsKnown() == 0) {
            noDevice.push_back(*it);
        }
    }

    /*
     * If at least one of the considered resources doesn't have a device,
     * the agent has a chance of inventing a device based off of the sum
     * of the experiences in the considered component devices.
     */
    int deviceUse;
    if (! noDevice.empty()) {
        double sumExp = accumulate(consideredDevicesExp.begin(),
                                   consideredDevicesExp.end(), 0.0);
        if (glob.random_01() < (glob.DEVICE_PROBABILITY_FACTOR * sumExp * inventSpeed)) {
            /*
             * If the agent successfully invents a device, one of the
             * considered resources without a device is randomly selected
             * to get the device, and the other five devices plus the
             * for the selected resource become the components of the new
             * device.
             */
            deviceUse = glob.random_choice(noDevice);
            /*
             * All agents find out about the existence of the new device,
             * but they are given zero experience, so they cannot build
             * it.
             */
            if (glob.discoveredDevices[device][deviceUse] != NULL) {
                LOG(4) << "Creating new " << device_names[device]
                       << " to replace existing device "
                       << deviceUse << " for which agentsKnown is 0.\n";
                delete glob.discoveredDevices[device][deviceUse];
            }
            switch (device) {
            case MACHINE:
                glob.discoveredDevices[device][deviceUse] =
                    new Machine(consideredDevices, deviceUse);
                break;
            case FACTORY:
                glob.discoveredDevices[device][deviceUse] =
                    new Factory(consideredDevices, deviceUse);
                break;
            case DEVMACHINE:
                glob.discoveredDevices[device][deviceUse] =
                    new DevMachine(consideredDevices, deviceUse);
                break;
            case INDUSTRY:
                glob.discoveredDevices[device][deviceUse] =
                    new Industry(consideredDevices, deviceUse);
                break;
            case DEVFACTORY:
                glob.discoveredDevices[device][deviceUse] =
                    new DevFactory(consideredDevices, deviceUse);
                break;
            default:
                cerr << "BAD device type in device variable at " << __FILE__ << ":" << __LINE__ << endl;
                break;
            }
            for (int agId = 0; agId < glob.NUM_AGENTS; agId++) {
                glob.agent[agId]->devProp[device][deviceUse].setDeviceExperience(0.0);
                glob.agent[agId]->devProp[device][deviceUse].idleDevice = true;
            }
            /*
             * The inventor is given six experience, so it is the only agent
             * able to make the device.
             */
            devProp[device][deviceUse].setDeviceExperience(glob.INVENTOR_DEVICE_EXPERIENCE);
            for (vector<int>::iterator comp = glob.discoveredDevices[device][deviceUse]->components.begin();
                 comp < glob.discoveredDevices[device][deviceUse]->components.end(); comp++) {
                if (devProp[componentType][*comp].getDeviceExperience() < 1) {
                    devProp[componentType][*comp].setDeviceExperience(1.0);
                }
            }
            devProp[device][deviceUse].idleDevice = false;
            LOG(4) << device_names[device] << " for " << deviceUse <<
                " has been invented. It is composed of " <<
                glob.discoveredDevices[device][deviceUse]->componentsAsString() << ".";
        }
    } else {
    	/*
    	 * If all of the considered resources have a device, the agent goes
         * on to attempt to invent the device of the next higher order.
    	 */
        if (device == MACHINE) {
            deviceInvention(FACTORY, MACHINE);
        } else if (device == FACTORY) {
            deviceInvention(INDUSTRY, FACTORY);
        } else if (device == DEVMACHINE) {
            deviceInvention(DEVFACTORY, DEVMACHINE);
        }
    }
}



/**
 * Makes the device the agent has agreed to make by removing the
 * appropriate devices or resource from the set aside pile and working
 * overtime minutes.
 * \param deviceIndex device index
 * \param device the type of the device to be made
 */
void Agent::makeDevice(int deviceIndex, device_name_t device)
{
    /*
     * The agent adds overtime and experience based on whether or not it
     * holds a device-making device for the device being made.
     */
    device_name_t bestDevDev = bestDevDevice(device, deviceIndex);
    double timeUse;
    if (bestDevDev != NO_DEVICE) {
        timeUse = deviceEffortCalc(deviceIndex, device) /
            glob.discoveredDevices[bestDevDev][deviceIndex]->deviceFactor;
        overtime += timeUse;
        deviceUse(bestDevDev, deviceIndex, timeUse);
        devProp[device][deviceIndex].setDeviceExperience(devProp[device][deviceIndex].getDeviceExperience() +
            glob.EXPERIENCE_FOR_MAKING[device] /
            glob.discoveredDevices[bestDevDev][deviceIndex]->deviceFactor);
    }
    else {
        timeUse = deviceEffortCalc(deviceIndex, device);
        overtime += timeUse;
        devProp[device][deviceIndex].setDeviceExperience(devProp[device][deviceIndex].getDeviceExperience() + glob.EXPERIENCE_FOR_MAKING[device]);
    }
    /*
     * Since this device was made this turn, it is not considered idle (i.e.
     * the agent will not lose experience in making this device).
     */
    devProp[device][deviceIndex].idleDevice = false;
    /* Everything that has been set aside is now officially removed. */
    for (vector<int>::iterator comp = glob.discoveredDevices[device][deviceIndex]->components.begin();
         comp < glob.discoveredDevices[device][deviceIndex]->components.end(); comp++) {
        if (device == TOOL) {
            resProp[*comp].resSetAside -= 1;
        } else if (devProp[glob.discoveredDevices[device][deviceIndex]->componentType][*comp].devicesSetAside > 0) {
            devProp[glob.discoveredDevices[device][deviceIndex]->componentType][*comp].devicesSetAside -= 1;
        }
    }
    /*
     * devicesRecentlyMade is tracked to help agents decide whether or not
     * they want to make device-making devices.
     */
    devProp[device][deviceIndex].devicesRecentlyMade[0]++;
    /* Relevant device statistics get updated. */
    deviceStatsUpdate(deviceIndex, device, bestDevDev, timeUse);
    LOG(4) << "I'm agent " << name << ", I made a " << device_names[device] <<
        " with " << glob.discoveredDevices[device][deviceIndex]->componentsAsString() <<
        " and it's good at making " << glob.discoveredDevices[device][deviceIndex]->use;
}



/**
 * Adds one unit of the resource to held and adds the
 * experience gained from making it.
 * \param resIndex resource index
 * \param bestDevice the type of the best device
 */
void Agent::work(int resIndex, device_name_t bestDevice)
{
    ResProperties &resPr = resProp[resIndex];
    if (bestDevice != NO_DEVICE) {
        resPr.experience += 1.0 / glob.discoveredDevices[bestDevice][resIndex]->deviceFactor;
    } else {
        resPr.experience++;
    }
    resPr.setHeld(resPr.getHeld() + 1);
    /*
     * Since this resource was extracted today, it is not considered idle
     * (i.e. the agent will not lose experience in this resource today).
     */
    resPr.idleResource = false;
}



/**
 * Agent statistics regarding resource extraction and device use are
 * updated.
 * \param resIndex resource index
 * \param bestDevice device type
 * \param workTime the amount of time the agent used to extract a resource
 */
void Agent::workStatsUpdate(int resIndex, device_name_t bestDevice, double workTime)
{
    resProp[resIndex].unitsGatheredToday++;
    if (bestDevice != NO_DEVICE) {
        unitsGatheredWithDeviceToday[bestDevice][resIndex]++;
        timeSpentGatheringWithDeviceToday[bestDevice] += workTime;
        devProp[bestDevice][resIndex].deviceMinutesUsedTotal += workTime;
    } else {
        timeSpentGatheringWithoutDeviceToday += workTime;
    }
}


/**
 * Agent statistics regarding device production and device-making
 * device use are updated.
 * \param deviceIndex device index
 * \param device type of component device
 * \param bestDevDevice type of the made device
 * \param timeUse time used to make bestDevDevice
 */
// TODO: comments correct?
void Agent::deviceStatsUpdate(int deviceIndex, device_name_t device,
                              device_name_t bestDevDevice, double timeUse)
{
    devProp[device][deviceIndex].devicesMadeTotal++;
    devProp[device][deviceIndex].devicesMadeToday++;
    timeSpentMakingDevicesToday[device] += timeUse;
    LOG(4) << "devicesMadeTotal for " << device_names[device] << " devIdx " << deviceIndex
           << " is now " << devProp[device][deviceIndex].devicesMadeTotal
           << ". MadeToday is " << devProp[device][deviceIndex].devicesMadeToday;

    if (bestDevDevice != NO_DEVICE) {
        devicesMadeWithDevDevicesToday[bestDevDevice][deviceIndex]++;
        devProp[bestDevDevice][deviceIndex].deviceMinutesUsedTotal += timeUse;
    }
}


/**
 * Adjust agent resource-extraction experience when it has completed
 * the resource-extraction phase.
 */
void Agent::workDayEnd()
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        ResProperties &resPr = resProp[resId];
        /*
         * If the agent did not extraction this resource today, it loses
         * penalty experience.
         */
        if (resPr.idleResource) {
            resPr.experience -= penalty;
            /* Agent resource experience cannot drop below 0. */
            if (resPr.experience < 0) {
                resPr.experience = 0.0;
            }
        }
        resPr.idleResource = true;
        /* Agent resource experience cannot exceed MAX_RES_EXPERIENCE. */
        if (resPr.experience > glob.MAX_RES_EXPERIENCE) {
            resPr.experience = glob.MAX_RES_EXPERIENCE;
        }
        /* Used for printing some data */
        resPr.unitsGatheredEndWork = resPr.unitsGatheredToday;
        resPr.boughtEndWork = resPr.boughtEndDay;
        resPr.soldEndWork = resPr.soldEndDay;
    }
}


/**
 * Updates device experience and memories.
 */
void Agent::endDayChecks()
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        for (int dev = TOOL; dev <= DEVFACTORY; dev++) {
            // Device experience can not raise above MAX_DEVICE_EXPERIENCE or drop below 0
            DevProperties &thisDevice = devProp[dev][resId];
            if (thisDevice.idleDevice) {
                thisDevice.setDeviceExperience(thisDevice.getDeviceExperience() - 1.0);
            }
            if (thisDevice.getDeviceExperience() <= 1) {
                thisDevice.setDeviceExperience(0.0);
            } else if (thisDevice.getDeviceExperience() > thisDevice.maxDeviceExperience) {
                thisDevice.setDeviceExperience(thisDevice.maxDeviceExperience);
            }
            /*
             * If the agent holds any minutes of the device, the agent's
             * experience in that device can not drop below MIN_HELD_DEVICE_EXPERIENCE.
             */
            if (thisDevice.getDeviceExperience() < glob.MIN_HELD_DEVICE_EXPERIENCE &&
                thisDevice.deviceHeld > 0) {
                thisDevice.setDeviceExperience(glob.MIN_HELD_DEVICE_EXPERIENCE);
            }
            assert( (thisDevice.getDeviceExperience() <= glob.MAX_DEVICE_EXPERIENCE) ||
                    (thisDevice.getDeviceExperience() >= 0) );
            thisDevice.idleDevice = true;
        }
    }
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        resProp[resId].endDayUtilities = utilCalc(resId);
    }
    /*
     * The agent begins the next day with overtime minutes set aside (note
     * that overtime is generally 0 minutes at this point, so agents usually
     * begin the day with 0 minutes set aside, but occasionally the agents
     * work more than their allotted time, so they have to set aside time
     * the next day).
     */
    setAsideTime = overtime;
    resetDeviceGainAndCostMemory();
    updateDeviceComponentExperience();
}


/**
 * Each unit of each resource has a chance of decaying based off of
 * its decay rate, and each device decays based off of the daily device
 * decay rate.
 */
void Agent::decay()
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
    	/*
    	 * Each unit of each resource has a 1.0/self.resProp[resId].averageLifetime
    	 * chance of decaying.
    	 */
        if (resProp[resId].getHeld() > 0) {
#ifdef DONT_RANDOMIZE
            // 0.25 is the most common and most approximate number used in the binomial.
            resProp[resId].setHeld(resProp[resId].getHeld() - resProp[resId].getHeld()*0.25);
#else
            resProp[resId].setHeld(resProp[resId].getHeld() -
                                    glob.random_binomial(resProp[resId].getHeld(),
                                            1.0 / resProp[resId].averageLifetime));
#endif
        }
        /*
         * Each device decays DAILY_DEVICE_DECAY minutes, and the number of
         * devices that an agent holds is the number of minutes the agent
         * holds divided by the lifetime of the device.
         */
        for (int dev = TOOL; dev <= DEVFACTORY; dev++) {
            DevProperties &thisDevice = devProp[dev][resId];
            double oldDeviceHeld = thisDevice.deviceHeld;
            if (thisDevice.deviceHeld > glob.DAILY_DEVICE_DECAY) {
                thisDevice.deviceHeld -= glob.DAILY_DEVICE_DECAY *
                    (thisDevice.deviceHeld / glob.discoveredDevices[dev][resId]->lifetime);
            } else if (thisDevice.deviceHeld <= glob.DAILY_DEVICE_DECAY) {
                thisDevice.deviceHeld = 0;
            }
            thisDevice.deviceMinutesDecayTotal += oldDeviceHeld - thisDevice.deviceHeld;
        }
    }
}


/**
 * Ensures that, if an agent has experience in a high order device, it
 * also has experience in the components of that device.

 */
void Agent::updateDeviceComponentExperience()
{
    device_name_t deviceTypes[] = {
        INDUSTRY, DEVFACTORY, FACTORY, DEVMACHINE, MACHINE
    };
    /*
     * If the agent has experience in a device, the agent's experience
     * in the device components can not drop below 1.
     */
    for (int idx = 0; idx < 5; idx++) {	// 5 items in deviceTypes[]
        device_name_t type = deviceTypes[idx];
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            assert(devProp[type][resId].getDeviceExperience() >= 0);
            if (devProp[type][resId].getDeviceExperience() > 0) {
                device_name_t compType = glob.discoveredDevices[type][resId]->componentType;
                for (vector<int>::iterator comp = glob.discoveredDevices[type][resId]->components.begin();
                    comp < glob.discoveredDevices[type][resId]->components.end(); comp++) {
                    if (devProp[compType][*comp].getDeviceExperience() < 1) {
                        devProp[compType][*comp].setDeviceExperience(1.0);
                    }
                }
            }
        }
    }
}



/**
 * \param resIndex resource id
 * \return the total number of minutes of resource-extracting devices for the given resource.
 */
double Agent::deviceHeldForRes(int resIndex) const
{
    device_name_t typeToCheck = TOOL;
    double minutesHeld = 0.0;
    while (glob.discoveredDevices[typeToCheck][resIndex] != NULL) {
        minutesHeld += devProp[typeToCheck][resIndex].deviceHeld;
        typeToCheck = glob.discoveredDevices[typeToCheck][resIndex]->componentOf;
        if (typeToCheck == NO_DEVICE) {
            break;
        }
    }
    return minutesHeld;
}


/**
 * The value of a device to an agent selling it is the cost to make it.
 * \param resIndex resource id
 * \param device device type
 */
double Agent::sellerDeviceValue(int resIndex, device_name_t device)
{
    return glob.discoveredDevices[device][resIndex]->costs(*this);
}


/**
 * Returns the value of a device to someone who is buying the device.
 * \param resIndex resource id
 * \param device device type
 */
double Agent::buyerDeviceValue(int resIndex, device_name_t device)
{
    double gainOverDeviceLife =
        glob.discoveredDevices[device][resIndex]->gainOverLifetime(*this);
    /*
     * If the agent doesn't know how to make the tool (i.e. has less than 1.0 experience),
     * it considers the
     * gain over the lifetime of the tool minus the gain that it could
     * get in the lifetime of the tool times the gain per minute the
     * agent was getting at the end of the last work day.
     * If the agent does know how to make the tool, it considers the
     * cost of making the tool on its own.
     */
    double value1 = gainOverDeviceLife -
        (endDayGPM * glob.discoveredDevices[device][resIndex]->lifetime);
    vector< double > consideredValues;
    if (value1 > 0) {
        consideredValues.push_back(value1);
        if (devProp[device][resIndex].getDeviceExperience() >= 1.0) {
            consideredValues.push_back(glob.discoveredDevices[device][resIndex]->costs(*this));
        }
        return *min_element(consideredValues.begin(), consideredValues.end());
    } else {
        return 0.0;
    }
}

/**
 * The agent looks at the agents that have traded with itself for
 * the most personal surplus, and prefers to trade with them.
 * \paramm device device type
 */
vector<int> Agent::preferredDeviceTraders(device_name_t device)
{
    vector<double> tradeMemoriesSort;
    for (int agId = 0; agId < glob.NUM_AGENTS; agId++) {
        double sum = accumulate(agentDeviceTradeMemory[device][agId].begin(),
                                agentDeviceTradeMemory[device][agId].end(), 0.0);
        tradeMemoriesSort.push_back(sum);
    }
    vector<int> preference;
    while ( *max_element(tradeMemoriesSort.begin(), tradeMemoriesSort.end()) > 0) {
        preference.push_back(max_element(tradeMemoriesSort.begin(), tradeMemoriesSort.end()) - tradeMemoriesSort.begin());
        tradeMemoriesSort[max_element(tradeMemoriesSort.begin(), tradeMemoriesSort.end()) - tradeMemoriesSort.begin()] = 0.0;
    }
    return preference;
}


/**
 * If an agent ends up buying a tool that it has already agreed to
 * sell, then it gets back the resources associated with that tool and
 * the amount of time that it had set aside to make the tool.
 * \param toolIndex tool id
 * \param device device type
 */
void Agent::getBackRes(int toolIndex, device_name_t bestDevDevice)
{
    if (bestDevDevice != NO_DEVICE) {
        setAsideTime -= deviceEffortCalc(toolIndex, TOOL) /
            glob.discoveredDevices[bestDevDevice][toolIndex]->deviceFactor;
    } else {
        setAsideTime -= deviceEffortCalc(toolIndex, TOOL);
    }
    devProp[TOOL][toolIndex].devicesToMake--;
    for (vector<int>::iterator comp = glob.discoveredDevices[TOOL][toolIndex]->components.begin();
         comp < glob.discoveredDevices[TOOL][toolIndex]->components.end(); comp++) {
        resProp[*comp].setHeld(resProp[*comp].getHeld() + 1);
        resProp[*comp].resSetAside--;
    }
}


/**
 * If an agent ends up buying a higher-order device that it has
 * already agreed to sell, then it gets back the components associated
 * with that device and the amount of time that it had set aside to
 * make the device.
 * \param device device type
 * \param deviceIndex device id
 */
void Agent::getBackDeviceComponents(device_name_t device, int deviceIndex)
{
    device_name_t bestDevDev = bestDevDevice(device, deviceIndex);
    if (device == TOOL) {
        getBackRes(deviceIndex, bestDevDev);
    } else {
        if (bestDevDev != NO_DEVICE) {
            setAsideTime -= deviceEffortCalc(deviceIndex, device) /
                glob.discoveredDevices[bestDevDev][deviceIndex]->deviceFactor;
        } else {
            setAsideTime -= deviceEffortCalc(deviceIndex, device);
        }
        devProp[device][deviceIndex].devicesToMake--;
        device_name_t compType = glob.discoveredDevices[device][deviceIndex]->componentType;
        for (vector<int>::iterator comp = glob.discoveredDevices[device][deviceIndex]->components.begin();
             comp < glob.discoveredDevices[device][deviceIndex]->components.end(); comp++) {
            DevProperties &thisComponent = devProp[compType][*comp];
            if (thisComponent.devicesSetAside > 0) {
                thisComponent.deviceHeld += glob.discoveredDevices[compType][*comp]->lifetime;
                thisComponent.devicesSetAside--;
            } else if (thisComponent.devicesToMake > 0) {
                getBackDeviceComponents(compType, *comp);
            }
        }
    }
}


/**
 * If an agent agrees to sell a tool that it does not already own,
 * then it must set aside the necessary resources to make that tool
 * and the amount of time it takes the agent to make the tool.
 * \param toolIndex tool id
 * \param bestDevDevice device type
 */
void Agent::setAsideRes(int toolIndex, device_name_t bestDevDevice)
{
    if (bestDevDevice != NO_DEVICE) {
        setAsideTime += deviceEffortCalc(toolIndex, TOOL) /
            glob.discoveredDevices[bestDevDevice][toolIndex]->deviceFactor;
    } else {
        setAsideTime += deviceEffortCalc(toolIndex, TOOL);
    }
    devProp[TOOL][toolIndex].devicesToMake++;
    for (vector<int>::iterator comp = glob.discoveredDevices[TOOL][toolIndex]->components.begin();
         comp < glob.discoveredDevices[TOOL][toolIndex]->components.end(); comp++) {
        resProp[*comp].setHeld(resProp[*comp].getHeld() - 1);
        resProp[*comp].resSetAside++;
    }
}


/**
 * If an agent agrees to sell a device that it does not already
 * own, then it must set aside the necessary components to make that
 * device and the amount of time it takes the agent to make the device.
 * \param device device type
 * \param deviceIndex device id
 */
void Agent::setAsideDeviceComponents(device_name_t device, int deviceIndex)
{
    device_name_t bestDevDev = bestDevDevice(device, deviceIndex);
    if (device == TOOL) {
        setAsideRes(deviceIndex, bestDevDev);
    } else {
        if (bestDevDev != NO_DEVICE) {
            setAsideTime += deviceEffortCalc(deviceIndex, device) /
                glob.discoveredDevices[bestDevDev][deviceIndex]->deviceFactor;
        } else {
            setAsideTime += deviceEffortCalc(deviceIndex, device);
        }
        devProp[device][deviceIndex].devicesToMake++;
        device_name_t compType = glob.discoveredDevices[device][deviceIndex]->componentType;
        for (vector<int>::iterator comp = glob.discoveredDevices[device][deviceIndex]->components.begin();
             comp < glob.discoveredDevices[device][deviceIndex]->components.end(); comp++) {
            if (devProp[compType][*comp].deviceHeld >= glob.discoveredDevices[compType][*comp]->lifetime) {
                devProp[compType][*comp].deviceHeld -= glob.discoveredDevices[compType][*comp]->lifetime;
                devProp[compType][*comp].devicesSetAside++;
            } else {
                setAsideDeviceComponents(compType, *comp);
            }
        }
    }
}



/**
 * If more than half of the most recent devices that the agent has
 * acquired were purchased from another agent -- rather than made for self
 * or sold -- this agent decides that it is able to purchase the device.
 * NOTE: we could change this to return just bool and use a reference param
 * to "return" the averagePrice.
 * \return a pair of (buy, averagePrice) where
 * buy is a boolean indicating whether or not the agent plans on buying this device,
 * and averagePrice is the price (in utils) it expects to pay
 */
pair<bool, double> Agent::canBuy(Device &device)
{
    /*
     * bought is the number of the given device in the last
     * TRADE_MEMORY_LENGTH to be obtained by this agent that were bought
     * from other agents (as opposed to being self-made).
     */
    // seems like this is the only thing that takes a Device object as
    // a parameter...  should we fix this?
    int bought = 0;
    /*
     * paid is the total amount (in utility) paid by the agent for the
     * purchased devices.
     */
    double paid = 0.0;
    for (int devTrade = 0; devTrade < glob.DEVICE_TRADE_MEMORY_LENGTH; devTrade++) {
    	/*
    	 * If the remembered price is positive, the agent purchased the
         * device (if the price was negative, the agent made the device for
         * itself or sold the device).
    	 */
        if (devProp[device.type][device.use].devicePrices[devTrade] > 0) {
            bought++;
            paid += devProp[device.type][device.use].devicePrices[devTrade];
        }
    }
    double averagePrice;
    bool buy;
    if (bought >= glob.DEVICE_TRADE_MEMORY_LENGTH / 2) {
        averagePrice = paid / bought;
        buy = true;
    } else {
        averagePrice = 0.0;
        buy = false;
    }
    /*
     * buy is a boolean indicating whether or not the agent plans on
     * buying this device
     * averagePrice is the price (in utils) it expects to pay.
     */
    return pair<bool, double>(buy, averagePrice);
}


/**
 * When buying a device, the agent adds the lifetime of the new
 * device to its store, unless it has already agreed to sell the
 * device, in which case it gets the components of the device returned.
 */
void Agent::buysDevice(int deviceIndex, device_name_t device)
{
    resetDeviceGainAndCostMemory();
    if (devProp[device][deviceIndex].getDeviceExperience() < glob.MIN_HELD_DEVICE_EXPERIENCE) {
        devProp[device][deviceIndex].setDeviceExperience(glob.MIN_HELD_DEVICE_EXPERIENCE);
    }
    updateDeviceComponentExperience();
    /*
     * If the agent already committed to making this device for another
     * agent, it does not gain the device; instead, the agent gets to
     * reclaim the time, devices, and resources that were set aside to make
     * the device.
     */
    if (devProp[device][deviceIndex].devicesToMake > 0) {
        getBackDeviceComponents(device, deviceIndex);
    } else {
    	/*
    	 * If the agent had not committed to make this device, then it
         * immediately gains the lifetime of the device into its holdings.
    	 */
        devProp[device][deviceIndex].deviceHeld += glob.discoveredDevices[device][deviceIndex]->lifetime;
    }
}


/**
 * When selling a device, the agent gives up the lifetime of the
 * device, or sets aside the necessary time and components to make the
 * device.
 */
void Agent::sellsDevice(int deviceIndex, device_name_t device)
{
    resetDeviceGainAndCostMemory();
    /*
     * If the agent doesn't already hold enough minutes of the device to
     * give to the other agent, it must commit to making the device by
     * setting aside the device components.
     */
    if (devProp[device][deviceIndex].deviceHeld < glob.discoveredDevices[device][deviceIndex]->lifetime) {
        setAsideDeviceComponents(device, deviceIndex);
    } else {
    	/*
    	 * If the agent holds enough minutes of the device, it can immediately
         * forfeit those minutes and move on.
    	 */
        devProp[device][deviceIndex].deviceHeld -= glob.discoveredDevices[device][deviceIndex]->lifetime;
    }
}


/**
 * Calculates the utility loss of giving up resourceBundle.
 * \param resourceBundle a vector of number of units of resource given up
 * \return the utility loss of giving up resourceBundle
 */
double Agent::costOfResourceBundle(vector<int> &resourceBundle) const
{
    double cost = 0;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (resourceBundle[resId] > 0) {
            cost += barterUtility(resId, - resourceBundle[resId]);
        }
    }
    return cost;
}

/**
 * Calculates the utility gain of getting resourceBundle
 * \param resourceBundle a vector of number of units of resource gained
 * \return the utility gain of getting resourceBundle
 */
double Agent::gainOfResourceBundle(vector<int> &resourceBundle) const
{
    double gain = 0;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (resourceBundle[resId] > 0) {
            gain += barterUtility(resId, resourceBundle[resId]);
        }
    }
    return gain;
}

/**
 * Use map to memoize the used sum of certain indices of marginalUtilities.
 */
double Agent::myAccumulate(int resIndex, int change) const
{
    const vector<double> &margUtility = resProp[resIndex].marginalUtilities;
    int myHeld = resHeld(resIndex);
    pair<int, int> temp = make_pair(myHeld, change);
    double result;
    if (change > 0) {
        if (glob.memoMUs[group].count(temp) != 0) {
            result = glob.memoMUs[group][temp];
        } else {
            result = accumulate(margUtility.begin() + myHeld, margUtility.begin() + myHeld + change, 0.0);
            glob.memoMUs[group][temp] = result;
        }
    } else if (change < 0) {
        if (glob.memoMUs[group].count(temp) != 0) {
            result = glob.memoMUs[group][temp];
        } else {
            result = accumulate(margUtility.begin() + myHeld + change, margUtility.begin() + myHeld, 0.0);
            glob.memoMUs[group][temp] = result;
        }
    }
    return result;
}

/**
 * Calculates the sums of the marginal utilities of the units of a
 * given resource in a given range(essentially the integral of the MU
 * from what is currently held by the agent to the given point).
 * \param resIndex resource id
 * \param change the integral of the MU from what is currently held by the agent to the given point
 */
double Agent::barterUtility(int resIndex, int change) const
{
    if (resHeld(resIndex) < 0) {
        /*
         * NOTE: in the python version if myHeld is negative,
         * then the code would sum up an empty list, which produces the
         * value 0.  In C++, it is a memory allocation error because
         * the iterator is moved past the beginning of the list.
         */
        return 0.0;
    }
    const vector<double> &margUtility = resProp[resIndex].marginalUtilities;
    int margUtilLen = margUtility.size();
    int myHeld = resHeld(resIndex);
    double result = 0.0;
    if (change > 0) {
        if (myHeld + change < margUtilLen) {
            result = myAccumulate(resIndex, change);
        } else {
            if (myHeld >= margUtilLen) {
                return glob.MIN_RES_UTIL * (myHeld + change - margUtilLen);
            }
            result = accumulate(margUtility.begin() + myHeld, margUtility.begin() + margUtilLen, 0.0) + glob.MIN_RES_UTIL * (myHeld + change - margUtilLen);
        }
    } else if (change < 0) {
        if (myHeld + change < 0) {
            /*
             * NOTE: in the python version if myHeld - change is negative,
             * then the code would sum up an empty list, which produces the
             * value 0.  In C++, it is a memory allocation error because
             * the iterator is moved past the beginning of the list.
             */
            return 0.0;
        }
        if (myHeld < margUtilLen) {
            result = myAccumulate(resIndex, change);
        } else {
            if (myHeld + change >= margUtilLen) {
                return glob.MIN_RES_UTIL * (myHeld - margUtilLen);
            }
            result = accumulate(margUtility.begin() + myHeld + change, margUtility.begin() + margUtilLen, 0.0) + glob.MIN_RES_UTIL * (myHeld - margUtilLen);
        }
    }
    return result;
}

/**
 * When agents are deciding to make devices for themselves, they must
 * calculate which device of that type gives the most utility gain.
 * \return a struct contains maxGain, maxIndex, costOfMax, benefitOfMax
 */
MaxInfo Agent::calcMaxDeviceGainAndIndex(device_name_t deviceType)
{

    MaxInfo maxIF;
    vector<Device *> &consideredDevice = glob.discoveredDevices[deviceType];
    resetDeviceGainAndCostMemory();
    /*
     * deviceBenefits is a list of utility gains of obtaining the device of
     * the given device type for the resource with the corresponding resId.
     */
    vector<double> deviceBenefits(glob.NUM_RESOURCES, 0.0);
    /*
     * deviceCosts is a list of utility losses of making the device of
     * the given device type for the resource with the corresponding resId.
     */
    vector<double> deviceCosts(glob.NUM_RESOURCES, 0.0);
    /* deviceGains is a list of the benefits minus the costs. */
    vector<double> deviceGains(glob.NUM_RESOURCES, 0.0);
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {

        /*
         * If the agent knows how to make the device (i.e. has at least 1 experience)
         * the agent calculates the worst case time and
         * resources (i.e. the time and resources necessary if the agent
         * is completely unable to buy any components).
         */
        if (devProp[deviceType][resId].getDeviceExperience() >= 1.0) {
            pair < double, vector<int> > worstCaseConstruction =
                consideredDevice[resId]->worstCaseConstruction(*this);
            double necessaryTime = worstCaseConstruction.first;
            vector<int> necessaryRes = worstCaseConstruction.second;
            if (resBundleHeldCheck(necessaryRes) &&
                setAsideTime + necessaryTime < glob.DAY_LENGTH &&
                deviceCurrentlyHeldForResource(resId, deviceType) < glob.DAYS_OF_DEVICE_TO_HOLD * glob.DAY_LENGTH) {
                /*
                 * If the agent has enough time and resources (and doesn't
                 * already hold too much of the device), it calculates the
                 * cost and benefit of the device.
                 */
                deviceBenefits[resId] = consideredDevice[resId]->gainOverLifetime(*this);
                deviceCosts[resId] = consideredDevice[resId]->costs(*this);
                deviceGains[resId] = deviceBenefits[resId] - deviceCosts[resId];
            }
        }
    }

    vector<double>::iterator max =
        max_element(deviceGains.begin(), deviceGains.end());

    /*
     * maxGain is the largest utility gain available to this agent for this
     * device type.
     */
    maxIF.maxGain = *max;
    /* maxIdxOffset is the ID of the device for which the agent gets maxGain. */
    maxIF.maxIndex = max - deviceGains.begin();

    /* costOfMax is the cost of the device with ID of maxId. */
    maxIF.costOfMax = deviceCosts[maxIF.maxIndex];
    /* benefitOfMax is the benefit of the device with ID of maxId. */
    maxIF.benefitOfMax = deviceBenefits[maxIF.maxIndex];
    return maxIF;
}

/**
 * After trade decisions have been made, agents decide whether or
 * not to make devices for themselves by considering the potential
 * gains and potential costs.
 */
void Agent::personalDevices(device_name_t device)
{
    vector<Device *> &consideredDevice = glob.discoveredDevices[device];

    MaxInfo maxDeviceGain = calcMaxDeviceGainAndIndex(device);
    double maxGain 	= maxDeviceGain.maxGain;
    double maxIdxOffset = maxDeviceGain.maxIndex;
    double costOfMax 	= maxDeviceGain.costOfMax;
    double benefitOfMax = maxDeviceGain.benefitOfMax;
    /*
     * While the agent continues to gain utility from making devices,
     * it makes devices.
     */
    while (maxGain > 0) {
        devProp[device][maxIdxOffset].devicePrices.pop_back();
        // NOTE: this is very inefficient!  We may want to make this thing a dlist or
        // list or deque.
        /*
         * Because the agent is making this device for itself, it remembers
         * the price as a negative value .
         */
        devProp[device][maxIdxOffset].devicePrices.insert(devProp[device][maxIdxOffset].devicePrices.begin(), - costOfMax);

        /* payment is the cost of the device excluding the time cost. */
        double payment = costOfMax - 
            consideredDevice[maxIdxOffset]->expectedConstructionTime(*this) * endDayGPM;
        /* The agent's gain from selling/making devices this day is increased. */
        utilGainThroughDevSoldToday += benefitOfMax - payment;

        /*
         * The agent commits to making the device with ID maxId by setting
         * aside the time, components, and/or resources needed to make it.
         */
        setAsideDeviceComponents(device, maxIdxOffset);
        /* The agent immediately gains the lifetime of the device. */
        devProp[device][maxIdxOffset].deviceHeld += consideredDevice[maxIdxOffset]->lifetime;

        LOG(4) << "Agent " << name <<  " wants to make a " << device_names[device] << 
            " for " << maxIdxOffset << " (maxGain, costOfMax, benefitOfMax) = (" <<
            maxGain << ", " << costOfMax << ", " << benefitOfMax << ")";

        /* The agent recalculates benefits and losses and proceeds. */
        maxDeviceGain = calcMaxDeviceGainAndIndex(device);
        maxGain      = maxDeviceGain.maxGain;
        maxIdxOffset = maxDeviceGain.maxIndex;
        costOfMax    = maxDeviceGain.costOfMax;
        benefitOfMax = maxDeviceGain.benefitOfMax;
    }

    LOG(5) << "LIST OF DEVICES TO MAKE AT END OF PERSONALDEVICES:";
    device_name_t deviceList[] = {
        TOOL, DEVMACHINE, MACHINE, DEVFACTORY, FACTORY, INDUSTRY
    };
    /* Then, the agent makes all the devices it has committed to make. */
    for (int idx = 0; idx < 6; idx++) {
        device_name_t device = deviceList[idx];
        for (int resNum = 0; resNum < glob.NUM_RESOURCES; resNum++) {
            if (devProp[device][resNum].devicesToMake != 0) {
                LOG(4) << "for tool " << deviceList[idx] << " res " << resNum << " devicesToMake " << devProp[device][resNum].devicesToMake;
            }
        }
    }
}


/**
 * Every single device the agent has agreed or decide to make gets
 * made (starting with the least complex, and going to the most
 * complex, so that the complex devices will already have their
 * components built).
 */
void Agent::deviceProduction()
{
    device_name_t deviceList[] = {
        TOOL, DEVMACHINE, MACHINE, DEVFACTORY, FACTORY, INDUSTRY
    };

    /* First the agent updates its memory. */
    for (int idx = 0; idx < 6; idx++) {	// 6 = length of array above.
        device_name_t device = deviceList[idx];
        vector<DevProperties> &thisDev = devProp[device];
        for (int resNum = 0; resNum < glob.NUM_RESOURCES; resNum++) {
            thisDev[resNum].devicesRecentlyMade.pop_back();
            // TODO: this is not efficient for vectors.  Perhap use a deque?
            thisDev[resNum].devicesRecentlyMade.insert(thisDev[resNum].devicesRecentlyMade.begin(), 0);
        }
    }
    /* Then, the agent makes all the devices it has committed to make. */
    for (int idx = 0; idx < 6; idx++) {
        device_name_t device = deviceList[idx];
        for (int resNum = 0; resNum < glob.NUM_RESOURCES; resNum++) {
            if (devProp[device][resNum].devicesToMake != 0) {
                LOG(4) << "for device " << device << " res " << resNum << " devicesToMake " << devProp[device][resNum].devicesToMake;
            }
            for (int amt = 0; amt < devProp[device][resNum].devicesToMake; amt++) {
                makeDevice(resNum, device);
            }
            devProp[device][resNum].devicesToMake = 0;
        }
    }
}


/**
 * Calculate the total utility that the agent holds.
 */
void Agent::calcUtilityToday()
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        utilityToday += utilityHeld(resId);
    }
}

/**
 * Every day, every time a new type of device begins to be traded, the
 * agent's memory must be updated.
 */
void Agent::newDeviceTrade(device_name_t device)
{
    tradedDeviceWithThisRound.clear();
    resetDeviceGainAndCostMemory();
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        if (*max_element(agentDeviceTradeMemory[device][aId].begin(), agentDeviceTradeMemory[device][aId].end()) > 0) {
            agentDeviceTradeMemory[device][aId].pop_back();
            agentDeviceTradeMemory[device][aId].insert(agentDeviceTradeMemory[device][aId].begin(), 0);
        }
    }
}

/**
 * Any time an agent's holdings of resources or devices changes, the
 * memory that it has of the values of devices is no longer correct, so
 * these values must be reset.
 */
void Agent::resetDeviceGainAndCostMemory()
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        for (int dev = TOOL; dev <= DEVFACTORY; dev++) {
            DevProperties &thisDevProp = devProp[dev][resId];
            thisDevProp.gainOverDeviceLifeMemory = 0.0;
            thisDevProp.gainOverDeviceLifeMemoryValid = false;
            thisDevProp.costOfDeviceMemory = 0.0;
            thisDevProp.costOfDeviceMemoryValid = false;
            thisDevProp.worstCaseConstructionMemory =
                make_pair(0.0, vector<int>());
            thisDevProp.worstCaseConstructionMemoryValid = false;
        }
    }
}


/**
 * Agents keep track of certain day-to-day statistics, and at the end
 * of each day, these statistics must be reset to 0.
 */
void Agent::resetTodayStats()
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        resProp[resId].unitsGatheredToday = 0;
        for (int type = DEVMACHINE; type <= DEVFACTORY; type++) {
            devicesMadeWithDevDevicesToday[type][resId] = 0;
        }
        for (int type = TOOL; type <= INDUSTRY; type++) {
            unitsGatheredWithDeviceToday[type][resId] = 0;
        }
        for (int type = 0; type < NUM_DEVICE_TYPES; type++) {
            devProp[type][resId].devicesMadeToday = 0;
        }
        resProp[resId].beforeWorkMU = 0;
        resProp[resId].unitsGatheredEndWork = 0;
        resProp[resId].boughtEndWork = 0;
        resProp[resId].soldEndWork = 0;
        resProp[resId].boughtEndDay = 0;
        resProp[resId].soldEndDay = 0;
    }
    unitsSoldToday = 0;
    unitsSoldCrossGroupToday = 0;
    unitsSoldForDevicesToday = 0;
    unitsSoldCrossGroupForDevicesToday = 0;
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        timeSpentGatheringWithDeviceToday[i] = 0;
    }
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        timeSpentMakingDevicesToday[i] = 0;
    }
    timeSpentGatheringWithoutDeviceToday = 0;
    utilityToday = 0.0;
}


/**
 * Removes the agent from the simulation.
 */
void Agent::remove()
{
    initializeAll(name, personalValues);
    inSimulation = false;
}


/**
 * Find the minimum value of the held value over all resources.
 */
int Agent::calcMinHeld()
{
    int minHeld = resProp[0].getHeld();
    for (vector<ResProperties>::iterator it = resProp.begin() + 1;
        it < resProp.end(); it++) {
        if (it->getHeld() < minHeld) {
            minHeld = it->getHeld();
        }
    }
    return minHeld;
}


/**
 * Find the maximum value of the held value over all resources.
 */
int Agent::calcMaxHeld()
{
    int maxHeld = resProp[0].getHeld();
    for (vector<ResProperties>::iterator it = resProp.begin() + 1;
         it < resProp.end(); it++) {
        if (it->getHeld() > maxHeld) {
            maxHeld = it->getHeld();
        }
    }

    return maxHeld;
}

/**
 * Prints the information of agents, resources, and devices.
 */
void Agent::logAgentData()
{
    LOG(5) << "**************** AGENT " << name << " ******************** ";
    LOG(5) << " ---- scalars --- ";
    LOG(5) << "penalty " << penalty;
    LOG(5) << "endDayGPM " << endDayGPM;
    LOG(5) << "setAsideTime " << setAsideTime;
    LOG(5) << "overtime " << overtime;
    LOG(5) << "unitsSoldToday " << unitsSoldToday;
    LOG(5) << "unitsSoldForDevicesToday " << unitsSoldForDevicesToday;
    LOG(5) << "timeSpentGatheringWithoutDeviceToday " << timeSpentGatheringWithoutDeviceToday;
    LOG(5) << "utilityToday " << utilityToday;
    LOG(5) << "utilGainThroughDevSoldToday " << utilGainThroughDevSoldToday;
    LOG(5) << " --- resProps --- ";
    LOG(5) << "res     endDayUtil     held idleResource experience resSetAside unitsGath";
    for (int i = 0; i < glob.NUM_RESOURCES; i++) {
        char ostr[128];
        sprintf(ostr, "%2d\t%8.4f\t%3d\t%d\t%8.4f\t%4d\t%4d", i,
                resProp[i].endDayUtilities, resProp[i].getHeld(),
                resProp[i].idleResource, resProp[i].experience, resProp[i].resSetAside,
                resProp[i].unitsGatheredToday);
        LOG(5) << ostr;
    }
    LOG(5) << " --- devProps --- ";
    LOG(5) << "devExp  devHeld  idleDev devToMake devSetAsi gainOverDevLM Valid? costOfDevMem Valid? devMadeToday devMadeTot";
    for (int i = 0; i < glob.NUM_RESOURCES; i++) {
        for (int type = 0; type < NUM_DEVICE_TYPES; type++) {
            LOG(5) << " device " << i << ", type " << type;
            char ostr[128];
            sprintf(ostr, "%8.4f\t%8.4f\t%d\t%d\t%d\t%8.4f\t%1d\t%8.4f\t%1d\t%d\t%d",
                    devProp[type][i].getDeviceExperience(), devProp[type][i].deviceHeld, devProp[type][i].idleDevice,
                    devProp[type][i].devicesToMake, devProp[type][i].devicesSetAside, devProp[type][i].gainOverDeviceLifeMemory,
                    devProp[type][i].gainOverDeviceLifeMemoryValid, devProp[type][i].costOfDeviceMemory,
                    devProp[type][i].costOfDeviceMemoryValid, devProp[type][i].devicesMadeToday, devProp[type][i].devicesMadeTotal);
            LOG(5) << ostr;

#if 0
            LOG(5) << "dev " << i << ", " << type << " deviceExperience  " << devProp[type][i].getDeviceExperience();
            LOG(5) << "dev " << i << ", " << type << " deviceHeld " << devProp[type][i].deviceHeld;
            LOG(5) << "dev " << i << ", " << type << " idleDevice " << devProp[type][i].idleDevice;
            LOG(5) << "dev " << i << ", " << type << " devicesToMake " << devProp[type][i].devicesToMake;
            LOG(5) << "dev " << i << ", " << type << " devicesSetAside " << devProp[type][i].devicesSetAside;
            LOG(5) << "dev " << i << ", " << type << " gainOverDeviceLifeMemory " << devProp[type][i].gainOverDeviceLifeMemory;
            LOG(5) << "dev " << i << ", " << type << " gainOverDeviceLifeMemoryValid " << devProp[type][i].gainOverDeviceLifeMemoryValid;
            LOG(5) << "dev " << i << ", " << type << " costOfDeviceMemory " << devProp[type][i].costOfDeviceMemory;
            LOG(5) << "dev " << i << ", " << type << " costOfDeviceMemoryValid " << devProp[type][i].costOfDeviceMemoryValid;
            LOG(5) << "dev " << i << ", " << type << " worstCaseConstructionMemory " << " NOT SHOWN";
            LOG(5) << "dev " << i << ", " << type << " devicesMadeToday " << devProp[type][i].devicesMadeToday;
            LOG(5) << "dev " << i << ", " << type << " devicesMadeTotal " << devProp[type][i].devicesMadeTotal;
#endif
            LOG(5) << "other stuff not shown yet... ";
        }
    }

    //    LOG(5) << " --- agentDeviceTradeMemory ---";

}
