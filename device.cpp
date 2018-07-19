/**
 * Define the five types of devices: tool, macine, factory, industry, devMachine, devFactory.
 */
#include <sstream>
#include <algorithm>
#include <numeric>
#include <boost/foreach.hpp>
#include "device.h"
#include "agent.h"
using namespace std;

/**
 * Constructor.
 */
Device::Device(device_name_t componentType, device_name_t type, device_name_t canMake,
               device_name_t devDevice, device_name_t componentOf,
               vector<int> &components, int use, double deviceFactor, double lifetime)
{
    this->componentType = componentType;
    this->type = type;
    this->canMake = canMake;
    this->devDevice = devDevice;
    this->componentOf = componentOf;
    this->components = components;
    this->use = use;
    this->deviceFactor = deviceFactor;
    this->lifetime = lifetime;
    // what about necessaryResources?  Currently called in each subclass constructor, but could
    // be called here instead, perhaps...
}

/**
 * \return the number of agents that have enough experience in this device
 */
int Device::agentsKnown()
{
    int known = 0;
    for (int i = 0; i < glob.NUM_AGENTS; i++) {
        if (glob.agent[i]->devProp[type][use].getDeviceExperience() > 0) {
            known++;
        }
    }
    return known;
}

/**
 * \param agent
 * \return the sum of the costs of the components to the agent
 */
double Device::costs(Agent &agent)
{
    if (agent.devProp[type][use].costOfDeviceMemoryValid) {
        return agent.devProp[type][use].costOfDeviceMemory;
    } else {
        /* canBuy returns a boolean and a double */
        pair<bool, double> buy = agent.canBuy(*this);
        /*
         * canBuy is the boolean; it indicates whether or not the agent
         * believes it is able to buy the device
         */
        bool canBuy = buy.first;
        /*
         * If the agent believes it is able to buy the device, the price (in
         * utils) that it expects to pay is avgPrice
         * So, if the agent believes it is able to buy the device, the cost
         * is avgPrice.
         */
        double avgPrice = buy.second;
        if (canBuy) {
            return avgPrice;
            /*
             * If the agent does not believe that it can buy the device, it
             * calculates the cost of the device as the time required to make
             * this device plus the cost of all the components of this device
             * (so the cost function gets called recursively on lower and lower
             * order devices)
             */
        } else {
            /*
             * First the agent calculates the time required to make the
             * device based on the agent's experience and whether or not it
             * has a device-making device
             */
            double time;
            device_name_t bestDevDevice = agent.bestDevDevice(type, use);
            if (bestDevDevice != NO_DEVICE) {
                time = agent.deviceEffortCalc(use, type) /
                    glob.discoveredDevices[bestDevDevice][use]->deviceFactor;
            } else {
                time = agent.deviceEffortCalc(use, type);
            }
            /*
             * The initial cost is the required time multiplied by the gain
             * per minute that the agent was getting at the end of the last
             * work day
             */
            double cost = time * agent.endDayGPM;
            
            /*
             * Then, the agent adds the cost of each of the components of
             * this device.
             */
            for (int i = 0; i < (int) components.size(); i++) {
                int comp = components[i];
                cost += glob.discoveredDevices[componentType][comp]->costs(agent);
            }
            agent.devProp[type][use].costOfDeviceMemory = cost;
            agent.devProp[type][use].costOfDeviceMemoryValid = true;
            return cost;
        }
    }
}

/**
 * \param agent
 * \return The estimated construction time of the device and its components, if the agent is able to buy some components
 */
double Device::expectedConstructionTime(Agent &agent)
{
    /*
     * if the agent believes that it can buy the device, the expected time
     * to build the device is 0.
     */
    if (agent.canBuy(*this).first) {
        return 0.0;
    }
    /*
     * If the agent does not believe it can buy the device, it calculates
     * the time required to make the device based on its experience and 
     * whether or not it has a device-making device, then it recursively
     * adds the expected construction time of each of the components
     */
    device_name_t bestDevDevice = agent.bestDevDevice(type, use);
    double time;
    if (bestDevDevice != NO_DEVICE) {
        time = agent.deviceEffortCalc(use, type) / glob.discoveredDevices[bestDevDevice][use]->deviceFactor;
    } else {
        time = agent.deviceEffortCalc(use, type);
    }

    for (int i = 0; i < (int) components.size(); i++) {
        int comp = components[i];
        time += glob.discoveredDevices[componentType][comp]->expectedConstructionTime(agent);
    }
    return time;
}


/**
 * \param an agent
 * \return a pair of (timeNeeded, necessaryRes) where the necessary time and resources to make the device and its components, if the agent is not able to buy any components.
 */
pair<double, vector<int> > Device::worstCaseConstruction(Agent &agent)
{
    /*
     * If the memory of this calculation that the agent has is valid, return
     * the value in memory
     */
    if (agent.devProp[type][use].worstCaseConstructionMemoryValid) {
        return agent.devProp[type][use].worstCaseConstructionMemory;
    } else {
        /*
         * highOrderNeeded will be a list of the number of typeToCheck
         * devices needed for the corresponding resId
         */
        vector<int> highOrderNeeded(glob.NUM_RESOURCES, 0);
        /*
         * Begin by requiring one device of the type that is currently under
         * consideration
         */
        highOrderNeeded[use] = 1;
        /* typeToCheck begins as the type of device under consideration */
        device_name_t typeToCheck = type;
        /* compType is the component type of typeToCheck */
        device_name_t compType = componentType;
        /*
         * timeNeeded will be the total, worst case time for this device and
         * its components.
         */
        double timeNeeded = 0.0;
        if (devDevice != NO_DEVICE &&
            agent.devProp[devDevice][use].deviceHeld > 0.0) {
            timeNeeded = agent.deviceEffortCalc(use, type) /
                glob.discoveredDevices[devDevice][use]->deviceFactor;
        } else {
            timeNeeded = agent.deviceEffortCalc(use, type);
        }

        /*
         * Loop through so that typeToCheck and compType become lower and
         * lower order device types; stop when there are no more lower order
         * devices to consider.
         */
        while (compType != NO_DEVICE) {
            /*
             * Given the number of typeToCheck devices in highOrderNeeded,
             * compNeeded is the number of compType devices needed for each
             * corresponding resId.
             */
            vector<int> compNeeded(glob.NUM_RESOURCES, 0);
            /*
             * Begin by simply adding the components of devices in
             * highOrderNeeded
             */
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                if (highOrderNeeded[resId] > 0) {
                    for (int i = 0; i < (int) glob.discoveredDevices[typeToCheck][resId]->components.size(); i++) {
                        int compId = glob.discoveredDevices[typeToCheck][resId]->components[i];
                        compNeeded[compId] += highOrderNeeded[resId];
                    }
                }
            }
            
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                if (compNeeded[resId] > 0) {
                    /*
                     * Remove devices from compNeeded if enough of the
                     * device is held by the agent; the number removed is
                     * the number of devices held (i.e. lifetime held
                     * divided by lifetime per device)
                     */
                    compNeeded[resId] = max(0, compNeeded[resId] -
                                                (int(agent.devProp[compType][resId].deviceHeld)
                                                        / int(glob.discoveredDevices[compType][resId]->lifetime)));
                }
            }
            
            /*
             * The time needed to make each of these components is added to
             * timeNeeded
             */
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                if (compNeeded[resId] > 0) {
                    device_name_t devDevice = glob.discoveredDevices[compType][resId]->devDevice;
                    /*
                     * If the device-making device is not invented or if
                     * the agent doesn't hold any, the devDeviceFactor
                     * is 1 (i.e. no effect on production)
                     */
                    double devDeviceFactor = 1.0;
                    if (devDevice != NO_DEVICE &&
                        agent.devProp[devDevice][resId].deviceHeld > 0.0) {
                        devDeviceFactor = glob.discoveredDevices[devDevice][resId]->deviceFactor;
                    }
                    for (int i = 0; i < compNeeded[resId]; i++) {
                        timeNeeded += agent.tempDeviceEffortCalc(resId, compType, i) / devDeviceFactor;
                    }
                }
            }
            
            /*
             * Once held devices have been removed from compNeeded, the
             * values in highOrderNeeded become the values from compNeeded,
             * and the loop starts again.
             */
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                highOrderNeeded[resId] = compNeeded[resId];
            }
            // typeToCheck and compType move one order down			
            typeToCheck = compType;
            compType = glob.discoveredDevices[typeToCheck][use]->componentType;
        }
        
        /*
         * When compType is None, typeToCheck is 'Tool', so the
         * highOrderNeeded list is the list of the number of tools needed
         * for each corresponding resId, so the necessary resources for this
         * device is the sum of necessary resources for each tool
         */
        vector<int> necessaryRes(glob.NUM_RESOURCES, 0);
        for (int resIdA = 0; resIdA < glob.NUM_RESOURCES; resIdA++) {
            if (highOrderNeeded[resIdA] > 0) {
                vector<int> thisToolNeeds = glob.discoveredDevices[TOOL][resIdA]->necessaryResources;
                for (int resIdB = 0; resIdB < glob.NUM_RESOURCES; resIdB++) {
                    necessaryRes[resIdB] += highOrderNeeded[resIdA] * thisToolNeeds[resIdB];
                }
            }
        }

        // Agents remember this calculated list
        agent.devProp[type][use].worstCaseConstructionMemory = make_pair(timeNeeded, necessaryRes);
        agent.devProp[type][use].worstCaseConstructionMemoryValid = true;
        return pair<double, vector<int> >(timeNeeded, necessaryRes);
    }
}

/**
 * \return The necessary resources for this device assuming no components are owned by the agent and none of the components can be purchased.
 */
vector<int> Device::necessaryRes()
{
    vector<int> necessaryRes(glob.NUM_RESOURCES, 0);
    for (int i = 0; i < (int) components.size(); i++) {
        for (int j = 0; j < glob.NUM_RESOURCES; j++) {
            necessaryRes[j] += glob.discoveredDevices[componentType][components[i]]->necessaryResources[j];
        }
    }

    return necessaryRes;
}

/**
 * Put all components in the form of [comp1, comp2, ..., ]
 */
string Device::componentsAsString()
{
    string res = "[";
    BOOST_FOREACH(int i, components) {
        std::stringstream str;
        str << i;
        res.append(str.str());
        res.append(", ");
    }
    res.append("]");
    return res;
}

/**
 * DevDevices speed up the production of other devices.
 * \param componentType the type of component device
 * \param type the type of this device
 * \param canMake the type of device it can make
 * \param devDevice ???
 * \param componentOf the type of device it can be component of
 * \param components a vector of its components
 * \param use
 * \param deviceFactor
 * \param lifetime
 */
DevDevice::DevDevice(device_name_t componentType, device_name_t type, device_name_t canMake,
                     device_name_t devDevice, device_name_t componentOf, vector<int> &components,
                     int use, double deviceFactor, double lifetime) :
    Device(componentType, type, canMake, devDevice, componentOf, components, use, deviceFactor, lifetime)
{
    necessaryResources = necessaryRes();	// both inherited from Device.
}

/**
 * In order to consider a device-making device, the agent must not hold
 * more than DAYS_OF_DEVICE_TO_HOLD worth of the device-making device, and
 * must hold at least MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION of the
 * devices that this device-making device can make
 */ 
bool DevDevice::wantsToConsiderDevDevice(Agent &agent)
{
    // NOTE: could perhaps optimize this by doing these
    // computations only if the first class of the return part below is true.
    int numAgentCurrentlyHeld, sum;
    if (agent.devDeviceHeldForRes(type, use) < glob.DAYS_OF_DEVICE_TO_HOLD * glob.DAY_LENGTH) {
        numAgentCurrentlyHeld = int(agent.devProp[type][use].deviceHeld / lifetime);
        sum = accumulate(agent.devProp[canMake][use].devicesRecentlyMade.begin(),
                             agent.devProp[canMake][use].devicesRecentlyMade.end(),
                             0);
    } else {
        return false;
    }
    return (sum > glob.MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION * (numAgentCurrentlyHeld + 1));
}

/**
 * For a device-making device, the gain over lifetime is the amount of
 * time saved by using the full lifetime of the device-making device to
 * make devices versus making that same number of devices without the
 * device-making device times the endDayGPM.
 * \return gain over life time of a DevDevice
 */
double DevDevice::gainOverLifetime(Agent &agent)
{
    if ((! wantsToConsiderDevDevice(agent)) || (! glob.res[use].inSimulation)) {
        return 0;
    } else {
        // Simulates making the same number of tools with and without a
        // devDevice, the time saved multiplied by the gain per minute
        // that the agent had at the end of the last work day is the
        // gain associated with this device.
        if (agent.devProp[type][use].gainOverDeviceLifeMemoryValid) {
            return agent.devProp[type][use].gainOverDeviceLifeMemory;
        } else {
            double devDeviceHeld = agent.devProp[type][use].deviceHeld;
            double heldDeviceUseTime = 0.0;
            double deviceExperienceGained = 0.0;
            while (devDeviceHeld - heldDeviceUseTime > 0.0) {
                heldDeviceUseTime += agent.tempDeviceEffortCalc(use, canMake, deviceExperienceGained) / deviceFactor;
                deviceExperienceGained += glob.EXPERIENCE_FOR_MAKING[canMake] / deviceFactor;
            }
            double deviceExperienceGainedWithHeld = deviceExperienceGained;
            int devicesMadeWithDevDevice = 0;
            double deviceUseTime = 0.0;
            while (lifetime - deviceUseTime > 0.0) {
                deviceUseTime += agent.tempDeviceEffortCalc(use, canMake, deviceExperienceGained) / deviceFactor;
                deviceExperienceGained += 1.0 / deviceFactor;
                devicesMadeWithDevDevice += 1;
            }
            int devicesMadeWithoutDevDevice = 0;
            double noDeviceUseTime = 0.0;
            deviceExperienceGained = deviceExperienceGainedWithHeld;
            while (devicesMadeWithoutDevDevice < devicesMadeWithDevDevice) {
                noDeviceUseTime += agent.tempDeviceEffortCalc(use, canMake, deviceExperienceGained);
                deviceExperienceGained += 1.0;
                devicesMadeWithoutDevDevice += 1;
            }
            double timeGained = noDeviceUseTime - deviceUseTime;
            agent.devProp[type][use].gainOverDeviceLifeMemory = timeGained * agent.endDayGPM;
            agent.devProp[type][use].gainOverDeviceLifeMemoryValid = true;

            return timeGained * agent.endDayGPM;
        }
    }
}

/**
 * DevMachine speeds up the production of tools.
 */
DevMachine::DevMachine(vector<int> &components, int use) :
    DevDevice(TOOL, DEVMACHINE, TOOL, NO_DEVICE, DEVFACTORY, components, use,
              glob.DEV_MACHINE_FACTOR, glob.DEV_MACHINE_LIFETIME)
{
    // TODO: Check with BRH: componentOf argument is DEVFACTORY.  Is that correct?
    // Looks like it is unset in the python code.  Perhaps should be NO_DEVICE.
    necessaryResources = necessaryRes();   // both inherited from Device.
}

/**
 * DevFactory speeds up the production of machines.
 */
DevFactory::DevFactory(vector<int> &components, int use) :
    DevDevice(DEVMACHINE, DEVFACTORY, MACHINE, NO_DEVICE, NO_DEVICE, components, use,
              glob.DEV_FACTORY_FACTOR, glob.DEV_FACTORY_LIFETIME)
{
    necessaryResources = necessaryRes();   // both inherited from Device.
}

/**
 * The most basic type of device.
 */
Tool::Tool(vector<int> &components, int use) :
    Device(NO_DEVICE, TOOL, NO_DEVICE, DEVMACHINE, MACHINE, components, use,
           glob.TOOL_FACTOR, glob.TOOL_LIFETIME)
{
    this->necessaryResources = this->necessaryRes();
}

/**
 * The only difference between tool costs and other device costs is that the
 * components of a tool are resources, so their cost is directly in
 * utility (instead of in lower-order devices).
 * \return the cost to the agent (in terms of utility) of giving up time to make the tool and the resource components of this tool.
 */
double Tool::costs(Agent &agent)
{
    if (agent.devProp[type][use].costOfDeviceMemoryValid) {
        return agent.devProp[type][use].costOfDeviceMemory;
    } else {
        pair< bool, double > buy = agent.canBuy(*this);
        bool canBuy = buy.first;
        double avgPrice = buy.second;
        if (canBuy) {
            agent.devProp[type][use].costOfDeviceMemory = avgPrice;
            agent.devProp[type][use].costOfDeviceMemoryValid = true;
            return avgPrice;
        } else {
            device_name_t bestDevDevice = agent.bestDevDevice(type, use);
            double time;
            if (bestDevDevice != NO_DEVICE) {
                time = agent.deviceEffortCalc(use, type) / glob.discoveredDevices[bestDevDevice][use]->deviceFactor;
            } else {
                time = agent.deviceEffortCalc(use, type);
            }
            double cost = time * agent.endDayGPM;
            for (int i = 0; i < (int) components.size(); i++) {
                int comp = components[i];
                cost += agent.tempUtilCalc(comp, -1);
            }
            agent.devProp[type][use].costOfDeviceMemory = cost;
            agent.devProp[type][use].costOfDeviceMemoryValid = true;
            return cost;
        }
    }
}

/**
 * The estimated construction time of a tool (does not include
 * construction time of components because components are resources).
 * \return the estimated construction time of a tool
 */
double Tool::expectedConstructionTime(Agent &agent)
{
    if (agent.canBuy(*this).first) {
        return 0.0;
    } else {
        device_name_t bestDevDevice = agent.bestDevDevice(type, use);	// jj29: bestDevDeviceType?
        double time;
        if (bestDevDevice != NO_DEVICE) {
            time = agent.deviceEffortCalc(use, type) /
                glob.discoveredDevices[bestDevDevice][use]->deviceFactor;
        } else {
            time = agent.deviceEffortCalc(use, type);
        }
        return time;
    }
}

/**
 * Simulates using the device after all higher order devices have
 * been used and calculates the utility benefit.
 * \return gain over life time of a Tool
 */
double Tool::gainOverLifetime(Agent &agent)
{
    double utilTool = 0.0;
    if (agent.deviceHeldForRes(use) > glob.DAYS_OF_DEVICE_TO_HOLD * glob.DAY_LENGTH ||
        (! glob.res[use].inSimulation)) {
        utilTool = 0;
    } else if (agent.devProp[type][use].gainOverDeviceLifeMemoryValid) {
        utilTool = agent.devProp[type][use].gainOverDeviceLifeMemory;
    } else {
        double toolHeld = agent.devProp[TOOL][use].deviceHeld;
        double machineHeld = agent.devProp[MACHINE][use].deviceHeld;
        double factoryHeld = agent.devProp[FACTORY][use].deviceHeld;
        double industryHeld = agent.devProp[INDUSTRY][use].deviceHeld;
        double machineFactor = 0.0, factoryFactor = 0.0, industryFactor = 0.0;
        if (machineHeld > 0) {
            machineFactor = glob.discoveredDevices[MACHINE][use]->deviceFactor;
        }
        if (factoryHeld > 0) {
            factoryFactor = glob.discoveredDevices[FACTORY][use]->deviceFactor;
        }
        if (industryHeld > 0) {
            industryFactor = glob.discoveredDevices[INDUSTRY][use]->deviceFactor;
        }
        double deviceUseTime = 0.0;
        int unitsMade = 0;
        double experienceGained = 0.0;

        double t1 = lifetime + toolHeld + machineHeld + factoryHeld + industryHeld;
        double t2 = toolHeld + machineHeld + factoryHeld + industryHeld;
        double t3 = machineHeld + factoryHeld + industryHeld;
        double t4 = factoryHeld + industryHeld;
        while (t1 - deviceUseTime > 0.0) {
            if (deviceUseTime > t2) {
                utilTool += agent.tempUtilCalc(use, unitsMade);
            }
            if (deviceUseTime >= t3) {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) /
                    deviceFactor;
                experienceGained += 1.0 / deviceFactor;
            }
            else if (deviceUseTime >= t4) {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) /
                    machineFactor;
                experienceGained += 1.0 / machineFactor;
            }
            else if (deviceUseTime >= industryHeld) {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) /
                    factoryFactor;
                experienceGained += 1.0 / factoryFactor;
            }
            else {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) /
                    industryFactor;
                experienceGained += 1.0 / industryFactor;
            }
            unitsMade++;
        }
        agent.devProp[type][use].gainOverDeviceLifeMemory = utilTool;
        agent.devProp[type][use].gainOverDeviceLifeMemoryValid = true;
    }
    return utilTool;
}

/**
 * \return The necessary resources for this tool.
 */
vector<int> Tool::necessaryRes()
{
    vector<int> resourceCount(glob.NUM_RESOURCES, 0);	// initialize to 0 by default.
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (find(components.begin(), components.end(), resId) != components.end()) {
            resourceCount[resId] = 1;
        }
    }
    return resourceCount;
}


Machine::Machine(vector<int> &components, int use) :
    Device(TOOL, MACHINE, NO_DEVICE, DEVFACTORY, FACTORY, components, use, glob.MACHINE_FACTOR,
           glob.MACHINE_LIFETIME)
{
    // BRH: canMake is NO_DEVICE?...  seems weird.  It is unset in python.
    necessaryResources = necessaryRes();   // both inherited from Device.
}

/**
 * Simulates using the device after all higher order devices have
 * been used and calculates the utility benefit.
 * \return gain over life time of a machine
 */
double Machine::gainOverLifetime(Agent &agent)
{
    double utilMachine = 0.0;
    if (agent.deviceHeldForRes(use) > glob.DAYS_OF_DEVICE_TO_HOLD * glob.DAY_LENGTH ||
        (! glob.res[use].inSimulation)) {
        utilMachine = 0;
    }
    else if (agent.devProp[type][use].gainOverDeviceLifeMemoryValid) {
        utilMachine = agent.devProp[type][use].gainOverDeviceLifeMemory;
    }
    else {
        double machineHeld = agent.devProp[MACHINE][use].deviceHeld;
        double factoryHeld = agent.devProp[FACTORY][use].deviceHeld;
        double industryHeld = agent.devProp[INDUSTRY][use].deviceHeld;
        double factoryFactor = 0.0, industryFactor = 0.0;
        if (factoryHeld > 0) {
            factoryFactor = glob.discoveredDevices[FACTORY][use]->deviceFactor;
        }
        if (industryHeld > 0) {
            industryFactor = glob.discoveredDevices[INDUSTRY][use]->deviceFactor;
        }
        double deviceUseTime = 0;
        int unitsMade = 0;
        double experienceGained = 0.0;

        double t1 = lifetime + machineHeld + factoryHeld + industryHeld;
        double t2 = machineHeld + factoryHeld + industryHeld;
        double t3 = factoryHeld + industryHeld;
        while (t1 - deviceUseTime > 0) {
            if (deviceUseTime > t2) {
                utilMachine += agent.tempUtilCalc(use, unitsMade);
            }
            if (deviceUseTime < industryHeld) {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) /
                    industryFactor;
                experienceGained += 1.0 / industryFactor;
            }
            else if (deviceUseTime < t3) {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) /
                    factoryFactor;
                experienceGained += 1.0 / factoryFactor;
            } else {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) / 
                    deviceFactor;
                experienceGained += 1.0 / deviceFactor;
            }
            unitsMade++;
        }
        agent.devProp[type][use].gainOverDeviceLifeMemory = utilMachine;
        agent.devProp[type][use].gainOverDeviceLifeMemoryValid = true;
    }
    return utilMachine;
}


Factory::Factory(vector<int> &components, int use) :
    Device(MACHINE, FACTORY, NO_DEVICE, NO_DEVICE, INDUSTRY, components, use,
           glob.FACTORY_FACTOR, glob.FACTORY_LIFETIME)
{
    // TODO: BRH: canMake is NO_DEVICE for Factory?  Seems weird.  Please verify.
    necessaryResources = necessaryRes();   // both inherited from Device.
}

/**
 * been used and calculates the utility benefit.
 * \return gain over life time of a factory
 */
double Factory::gainOverLifetime(Agent &agent)
{
    double utilFactory = 0.0;
    if (agent.deviceHeldForRes(use) > glob.DAYS_OF_DEVICE_TO_HOLD * glob.DAY_LENGTH ||
        (! glob.res[use].inSimulation)) {
        utilFactory = 0;
    } else if (agent.devProp[type][use].gainOverDeviceLifeMemoryValid) {
        utilFactory = agent.devProp[type][use].gainOverDeviceLifeMemory;
    } else {
        double factoryHeld = agent.devProp[FACTORY][use].deviceHeld;
        double industryHeld = agent.devProp[INDUSTRY][use].deviceHeld;
        double industryFactor = 0.0;    // TODO: device.py doesn't have this. Then what's the value of it when used later?
        if (industryHeld > 0) {
            industryFactor = glob.discoveredDevices[INDUSTRY][use]->deviceFactor;
        }
        double deviceUseTime = 0;
        int unitsMade = 0;
        double experienceGained = 0;
        double t1 = lifetime + factoryHeld + industryHeld;
        double t2 = factoryHeld + industryHeld;
        while (t1 - deviceUseTime > 0) {
            if (deviceUseTime > t2) {
                utilFactory += agent.tempUtilCalc(use, unitsMade);
            }
            if (deviceUseTime < industryHeld) {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) /
                    industryFactor;
                experienceGained += 1.0 / industryFactor;
            } else {
                deviceUseTime += agent.tempEffortCalc(use, experienceGained) / 
                    deviceFactor;
                experienceGained += 1.0 / deviceFactor;
            }
            unitsMade++;
        }
        agent.devProp[type][use].gainOverDeviceLifeMemory = utilFactory;
        agent.devProp[type][use].gainOverDeviceLifeMemoryValid = true;
    }
    return utilFactory;
}


Industry::Industry(vector<int> &components, int use) :
    Device(FACTORY, INDUSTRY, NO_DEVICE, NO_DEVICE, NO_DEVICE, components, use,
           glob.INDUSTRY_FACTOR, glob.INDUSTRY_LIFETIME)
{
    // TODO: BRH: canMake is NO_DEVICE for Industry?  Seems weird.  Please verify.
    necessaryResources = necessaryRes();   // both inherited from Device.
}

/**
 * \return gain over life time of an industry
 */
double Industry::gainOverLifetime(Agent &agent)
{
    double utilIndustry = 0.0;
    if (agent.deviceHeldForRes(use) > glob.DAYS_OF_DEVICE_TO_HOLD * glob.DAY_LENGTH ||
        (! glob.res[use].inSimulation)) {
        utilIndustry = 0;
    } else if (agent.devProp[type][use].gainOverDeviceLifeMemoryValid) {
        utilIndustry = agent.devProp[type][use].gainOverDeviceLifeMemory;
    } else {
        double industryHeld = agent.devProp[INDUSTRY][use].deviceHeld;
        double deviceUseTime = 0;
        int unitsMade = 0;
        double experienceGained = 0.0;
        double t1 = lifetime + industryHeld;
        while (t1 - deviceUseTime > 0) {
            if (deviceUseTime > industryHeld) {
                utilIndustry += agent.tempUtilCalc(use, unitsMade);
            }
            deviceUseTime += agent.tempEffortCalc(use, experienceGained) / deviceFactor;
            experienceGained += 1.0 / deviceFactor;
            unitsMade++;
        }
        agent.devProp[type][use].gainOverDeviceLifeMemory = utilIndustry;
        agent.devProp[type][use].gainOverDeviceLifeMemoryValid = true;
    }
    return utilIndustry;
}

