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
    double held;																																																				//JYC: changed to type double
public:
    /**
     * False if the agent worked on the resource during the day,
     * and True if the agent did not.
     */
    double idleResource;																																																//JYC: changed to type double
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
    double resSetAside;																																																	//JYC: changed to type double
    /**
     * Keeps track of the resources gathered on the current day
     */
    double unitsGatheredToday;																																													//JYC: changed to type double
    double timeSpentGatheringWithoutDeviceToday; //BRH NEW 05.29.2018

    double beforeWorkMU;        // *** the marginal utility after the first resource trade and before work
    double beforeWorkHeld;         // *** the number of units held of this resource before work																			//JYC: changed to type double
    double  unitsGatheredEndWork;  // *** the number of untis gathered after work before the second trade												//JYC: changed to type double
    double boughtEndWork, soldEndWork;     // *** the number of this resources bought and sold after work before the second trade	//JYC: changed to type double
    double boughtEndDay, soldEndDay;       // *** the number of this resources bought and sold at the end of the day								//JYC: changed to type double

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
    double getHeld() const { return held; }																																								//JYC: changed to type double
};


struct DevProperties
{
private:
    double deviceExperience;    //!< The amount of experience that the agent has in the device.
public:
    double minDeviceEffort;
    double maxDeviceEffort;
    double maxDeviceExperience;																																													//JYC: changed to type int

    vector<double> deviceEfforts;   //!< initialized to be an empty list in py

    double deviceHeld;		//!< Number of minutes of the device currently held by the agent
    bool idleDevice;		//!< Indicates whether or not the agent has built or used this device
    double devicesToMake;		//!< Keeps track of the devices that the agent has agreed to make during device trading.							//JYC: changed to type double
    /**
     * When trading devices, agents have to set aside devices so that
     * they will have enough to make the devices that they agree to
     * trade, this list keeps track of the number of devices set aside.
     */
    double devicesSetAside;																																															//JYC: changed to type double
    double gainOverDeviceLifeMemory;	//!< Memorizes gainOverDeviceLife
    bool gainOverDeviceLifeMemoryValid;
    double costOfDeviceMemory;		    //!< Memorizes cost
    bool costOfDeviceMemoryValid;
    pair < double, vector<double> > worstCaseConstructionMemory;	//!< Memorizes worstCaseConstruction										//JYC: changed to type double
    bool worstCaseConstructionMemoryValid;
    /**
     * This is to keep track of devices made on the current day.
     *  Primarily for statistical purposes
     */
    double devicesMadeToday;	//!< The total number of devices of this type made by agent.																		//JYC: changed to type double
    double devicesMadeTotal;																																														//JYC: changed to type double
    /**
     * Each element in these lists is the number of devices sold of the
     * corresponding resource.
     * Primarily for statistical purposes.
     */
    double devicesSoldTotal;
    /**
     * each element keeps track of the number of devices bought.
     * for statistical purposes
     */
    double devicesBoughtTotal;
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
    vector<double> devicesRecentlyMade;

    DevProperties();
    void calcDeviceEfforts();
    double getDeviceExperience() { return deviceExperience; };
    void setDeviceExperience(double newDE) { deviceExperience = newDE; };
};

#endif

