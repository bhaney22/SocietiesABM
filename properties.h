#ifndef _SOC_PROPERTIES_H_
#define _SOC_PROPERTIES_H_

#include <vector>
using namespace std;

struct ResProperties
{
    /**
     * Keeps track of the marginal utility of a resource at the end
     * of the day for quick reference during tool trading.
     */
    double endDayUtilities;

    /**
     * The marginal utility of the associated resource at the amount
     * held of that resource.
     */
    vector<double> marginalUtilities;
private:
    /**
     * Keeps track of the number of units of all resources this agent is
     * holding.
     */
    int held;
public:
    /**
     * False if the agent worked on the resource during the day,
     * and True if the agent did not.
     */
    int idleResource;
    /**
     * the amount of experience the agent has producing the
     * corresponding resource (note that this experience can be reduced by
     * a penalty if the agent doesn't work on a resource during a day).
     */
    double experience;
    /**
     * When trading devices, agents have to set aside resources
     * so that they will have enough to make the devices that they agree to
     * trade. This variable keeps track of the number of units set aside.
     */
    int resSetAside;
    /**
     * Keeps track of the resources gathered on the current day
     */
    int unitsGatheredToday;

    double beforeWorkMU;        // *** the marginal utility after the first resource trade and before work
    int beforeWorkHeld;         // *** the number of units held of this resource before work
    int  unitsGatheredEndWork;  // *** the number of untis gathered after work before the second trade
    int boughtEndWork, soldEndWork;     // *** the number of this resources bought and sold after work before the second trade
    int boughtEndDay, soldEndDay;       // *** the number of this resources bought and sold at the end of the day

    double steepness;
    double scaling;
    double averageLifetime;
    double minResEffort;
    double maxResEffort;
    double maxResExperience;
    /**
     * The effort function of the agent (maps experience to
     * number of minutes required to extract a resource.)
     */
    vector<double> resEfforts;

    ResProperties();
    void calcResEfforts();
    void calcMarginalUtilities();
    void setHeld(int newHeld);
    int getHeld() const { return held; }
};


struct DevProperties
{
private:
    double deviceExperience;    //!< The amount of experience that the agent has in the device.
public:
    double minDeviceEffort;
    double maxDeviceEffort;
    double maxDeviceExperience;

    vector<double> deviceEfforts;   //!< initialized to be an empty list in py

    double deviceHeld;		//!< Number of minutes of the device currently held by the agent
    bool idleDevice;		//!< Indicates whether or not the agent has built or used this device
    int devicesToMake;		//!< Keeps track of the devices that the agent has agreed to make during device trading.
    /**
     * When trading devices, agents have to set aside devices so that
     * they will have enough to make the devices that they agree to
     * trade, this list keeps track of the number of devices set aside.
     */
    int devicesSetAside;
    double gainOverDeviceLifeMemory;	//!< Memorizes gainOverDeviceLife
    bool gainOverDeviceLifeMemoryValid;
    double costOfDeviceMemory;		    //!< Memorizes cost
    bool costOfDeviceMemoryValid;
    pair < double, vector<int> > worstCaseConstructionMemory;	//!< Memorizes worstCaseConstruction
    bool worstCaseConstructionMemoryValid;
    /**
     * This is to keep track of devices made on the current day.
     *  Primarily for statistical purposes
     */
    int devicesMadeToday;	//!< The total number of devices of this type made by agent.
    int devicesMadeTotal;
    /**
     * Each element in these lists is the number of devices sold of the
     * corresponding resource.
     * Primarily for statistical purposes.
     */
    int devicesSoldTotal;
    /**
     * each element keeps track of the number of devices bought.
     * for statistical purposes
     */
    int devicesBoughtTotal;
    /**
     * This element keeps track of the time used, time in minutes.
     * Primarily for statistical purposes
     */
    double deviceMinutesUsedTotal;
    /**
     * This element keep the total of decay minutes.
     * Primarily for statistical purposes.
     */
    double deviceMinutesDecayTotal;
    /**
     * Each element in these lists is a list of the five most recent
     * prices (in terms of utility given up) of the device of the
     * corresponding resource.
     */
    vector<double> devicePrices;
    /**
     * This list keeps track of devices that have been made
     * within the set memory length.
     */
    vector<int> devicesRecentlyMade;

    DevProperties();
    void calcDeviceEfforts();
    double getDeviceExperience() { return deviceExperience; };
    void setDeviceExperience(double newDE) { deviceExperience = newDE; };
};

#endif

