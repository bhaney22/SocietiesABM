#include <iostream>
#include <vector>
#include "globals.h"
using namespace std;

class Agent;

class Device
{
public:

    device_name_t componentType;    //!< The type of device that combines to build this device
    int use;                        //!< The resId for which this device is useful
    double lifetime;                //!< Number of minutes that one device of this type can be used
    /**
     * The value by which this device divides resource
     * extraction time (for resource-extracting devices) or
     * device production time (for device-making devices)
     */
    double deviceFactor;
    /**
     * List of the IDs of the components of componentType that
     * get combined to make this device
     */
    vector<int> components;
    /**
     * A list of the quantities of resources necessary to
     * make this device assuming the agent isn't able to buy
     * any of the components and doesn't own any of the
     * components
     */
    vector<int> necessaryResources;
    device_name_t type;     
    /**
     * The type of device that this device is able to make
     * (only relevant for device-making devices).
     */
    device_name_t canMake;
    /**
     * The type of device-making device that speeds construction
     * of this device
     */
    device_name_t devDevice;
    device_name_t componentOf;

    Device(device_name_t componentType, device_name_t type,
           device_name_t canMake, device_name_t devDevice, device_name_t componentOf,
           vector<int> &components, int use, double deviceFactor, double lifetime);
    int agentsKnown();
    virtual double costs(Agent &agent);
    virtual double expectedConstructionTime(Agent &agent);
    pair<double, vector<int> > worstCaseConstruction(Agent &agent);
    virtual vector<int> necessaryRes();
    virtual double gainOverLifetime(Agent &agent) = 0;

    string componentsAsString();
};


class DevDevice : public Device
{
public:
    DevDevice(device_name_t componentType, device_name_t type,
              device_name_t canMake, device_name_t devDevice, device_name_t componentOf,
              vector<int> &components, int use, double deviceFactor, double lifetime);
    bool wantsToConsiderDevDevice(Agent &agent);
    double gainOverLifetime(Agent &agent);
};


class DevMachine : public DevDevice
{
public:
    DevMachine(vector<int> &components, int use);
};


class DevFactory : public DevDevice
{
public:
    DevFactory(vector<int> &components, int use);
};


class Tool : public Device
{
public:
    Tool(vector<int> &components, int use);
    double costs(Agent &agent);
    double expectedConstructionTime(Agent &agent);
    double gainOverLifetime(Agent &agent);
    vector<int> necessaryRes();	
};


class Machine : public Device
{
public:
    Machine(vector<int> &components, int use);
    double gainOverLifetime(Agent &agent);
};


class Factory : public Device
{
public:
    Factory(vector<int> &components, int use);
    double gainOverLifetime(Agent &agent);
};


class Industry : public Device
{
public:
    Industry(vector<int> &components, int use);
    double gainOverLifetime(Agent &agent);
};

// TODO: I moved componentOf up to Device.
// Tony: please make sure it and canMake are set to correct values in all cases.
// As long as Industry and DevFactory are componentOf None
// or componentOf -1 or something similar, then all devices
// have componentOf, and it should be moved to the super class
