
#ifndef STATSTRACKER_H_
#define STATSTRACKER_H_
/*
 * societies  Simulates societies and thier growth based on trade, experience and inventions.
 * Copyright (C) 2011 Tony Ditta, Michael Koster
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <iostream>
#include <vector>
#include "globals.h"
#include "marketplace.h"
using namespace std;

class Device;

class DayExchangeStats
{
private:
    /**
     * How much is sold for each resource.
     * length = NUM_RESOURCES
     */
    vector<double> soldExchanges;																																												//JYC: changed to type double
    /**
     * How much is bought for each resource.
     * length = NUM_RESOURCES
     */
    vector<double> boughtExchanges;																																											//JYC: changed to type double
    /**
     * \struct deviceExchange
     * Records the number of exchanges for a type of device.
     */
    struct deviceExchange {
        double sold;               //!< the number of the type of device sold																														//JYC: changed to type double
        vector<double> bought;     //!< a vector of the number of the type of device bought for each resource													//JYC: changed to type double
    };
    vector< deviceExchange > deviceExchanges; //!< a vector of deviceExchange struct. Indexed by device type

public:
    DayExchangeStats();
    /**
     * \return the vector of sold number for each resource
     */
    vector<double> getSoldExchanges() { return soldExchanges; }
    /**
     * \return the vector of bought number for each resource
     */
    vector<double> getBoughtExchanges() { return boughtExchanges; }
    void addToSoldExchanges(int resId, int newVal) { soldExchanges[resId] += newVal; }      //!< update soldExchange
    void addToBoughtExchanges(int resId, int newVal) { boughtExchanges[resId] += newVal; }  //!< update boughtExchange
};



class TradeStats
{
private:
    vector<double> resTradeVolume;				        //!< indexed by day #																															//JYC: changed to type double
    vector<vector<double> > resTradeVolumeByGroup;     //!< indexed by group # then day #																					//JYC: changed to type double
    vector<double> resTradeVolumeCrossGroup;           //!< index by day #																													//JYC: changed to type double
    vector<vector<double> > resTradeVolumeWithinGroup; //!< indexed by group # then day #																				//JYC: changed to type double

    vector<double> resTradeForDeviceVolume;		    //!< indexed by day #																													//JYC: changed to type double
    vector<vector<double> > resTradeForDeviceVolumeByGroup;    //!< indexed by group # then day #																	//JYC: changed to type double
    vector<double> resTradeForDeviceVolumeCrossGroup;          //!< indexed by day #																								//JYC: changed to type double
    vector<vector<double> > resTradeForDeviceVolumeWithinGroup;//!< indexed by group # then day #																//JYC: changed to type double

    vector< DayExchangeStats > dayResExchanges;		//!< length NUM_RESOURCES
    vector< vector <vector <vector<double> > > >resExchanges;/**< indexed by day first, then resId																		//JYC: changed to type double
              then 0th is soldExchanges and 1st is boughtExchanges, both vectors of ints. */

public:
    TradeStats();
    void dailyUpdate();
    void newExchange(ResourcePair &pair);
// BRH 11.11.2017 New vectors to save individual trades in memory
	vector<int> tradeLog(); //!< laundry list of trade info

    vector<double> getResTradeVolume() { return resTradeVolume; };																															//JYC: changed to type double
    vector<vector<double> > getResTradeVolumeByGroup() { return resTradeVolumeByGroup; };																			//JYC: changed to type double
    vector<double> getResTradeVolumeCrossGroup() { return resTradeVolumeCrossGroup; };																				//JYC: changed to type double
    vector<vector<double> > getResTradeVolumeWithinGroup() { return resTradeVolumeWithinGroup; };															//JYC: changed to type double

    vector<double> getResTradeForDeviceVolume() { return resTradeForDeviceVolume; };																						//JYC: changed to type double
    vector<vector<double> > getResTradeForDeviceVolumeByGroup() { return resTradeForDeviceVolumeByGroup; };										//JYC: changed to type double
    vector<double> getResTradeForDeviceVolumeCrossGroup() { return resTradeForDeviceVolumeCrossGroup; };											//JYC: changed to type double
    vector<vector<double> > getResTradeForDeviceVolumeWithinGroup() { return resTradeForDeviceVolumeWithinGroup; };						//JYC: changed to type double
};



/**
 * ProductionStats
 */
class ProductionStats
{
    vector<double> resGathered;			                /**< indexed by day # */																															//JYC: changed to type double
    vector<vector<double> > resGatheredByAgent; 	        /**< indexed by agentId, then day */																						//JYC: changed to type double
    vector<vector<double> > resGatheredByGroup;            /**< indexed by groupId, then day */																					//JYC: changed to type double
    vector<vector<double> > numAgentsGatheringByRes;       /**< indexed by resId, then day */																					//JYC: changed to type double
    vector<vector<double> > resGatheredByRes;              /**< indexed by resId, then day */																							//JYC: changed to type double
    vector< vector < vector<double> > > resGatheredByResByAgent; /**< indexed by resId, then agentId, then day */												//JYC: changed to type double
    vector< vector<double> > resGatheredByDevice;          /**< indexed by device_name_t (first 4), then day */															//JYC: changed to type double
    vector< vector< vector<double> > > resGatheredByDeviceByRes; /**< indexed by device_name_t (first 4), then resId, then day. */					//JYC: changed to type double
    vector< vector<double> > devicesMade;		            /**< indexed by device_name_t, then day */																				//JYC: changed to type double
    vector< vector< vector<double> > > devicesMadeByRes;   /**< indexed by device_name_t, then resId, then day */												//JYC: changed to type double
    vector< vector< vector<double> > > devicesMadeByGroup; /**< indexed by agent type, then device_name_t, then day */								//JYC: changed to type double
    /**
     * Number of devDevices made for this resource.
     * indexed by device_name_t (only for DEVMACHINE and DEVFACTORY), and then day
     */
    vector< vector<double> > devicesMadeWithDevDevice;																																					//JYC: changed to type double
    /**
     * only for DEVMACHINE and DEVFACTORY
     * indexed by device_name_t, then resId, then day
     */
    vector< vector< vector <double> > > devicesMadeWithDevDeviceByRes;																													//JYC: changed to type double

    /*
     * Index by agent type, then by device_name_t, then day.
     */
    vector< vector< vector <double> > > devicesMadeWithDevDeviceByGroup;																												//JYC: changed to type double
    /**
     * Each int is the total time spent by society without a device.
     * indexed by day
     */
    vector<double> timeSpentGatheringWithoutDevice;																																						//JYC: changed to type double
    vector< vector<double> > timeSpentGatheringWithoutDeviceByAgent;   /**< indexed by agentId, then day */													//JYC: changed to type double
    vector< vector<double> > timeSpentGatheringWithoutDeviceByGroup;  /**< indexed by groupId, then day */													//JYC: changed to type double
    vector< vector<double> > timeSpentGatheringWithDevice;             /**< indexed by device_name_t (first 4), then day */										//JYC: changed to type double
    /**
     * Total time this agent spent gathering with a device.
     * indexed by device_name_t (first 4), then agentId, then day.
     */
    vector< vector< vector<double> > > timeSpentGatheringWithDeviceByAgent;			//JYC: changed to type double
    vector< vector< vector<double> > > timeSpentGatheringWithDeviceByGroup;         // *** indexed by device_name_t, then group id, then day	 //JYC: changed to type double
    
    //BRH Two new stats for tracking time by resource
    vector< vector<double> > timeSpentGatheringWithoutDeviceByRes;        //[resId][day]																								//JYC: changed to type double
    vector< vector< vector<double> > > timeSpentGatheringWithDeviceByRes; //[type][resId][day]																				//JYC: changed to type double
    /**
     * Time the society spent gathering devices each day.
     * indexed by device_name_t, then day
     */
    vector< vector<double> > timeSpentMakingDevices;																																							//JYC: changed to type double
     /**
      * Time this agent spent gathering this device.
      * indexed by device_name_t, then agentId, then day.
      */
    vector< vector< vector<double> > > timeSpentMakingDevicesByAgent;																															//JYC: changed to type double
    vector< vector< vector<double> > > timeSpentMakingDevicesByGroup;      // *** indexed by device_name_t, then group id, then day				//JYC: changed to type double
    /**
     * Resources gathered with device / all resources gathered
     * indexed by device_name_t (first 4), then day.
     */
    vector< vector<double> > percentResGatheredByDevice;
     /**
      * Amount of resources gathered with this / all of this specific resource gathered
      * indexed by device_name_t (first 4), then resId, then day.
      */
    vector< vector< vector<double> > > percentResGatheredByDeviceByRes;

    void calcResGatheredAndResGatheredByAgentByGroup();
    void calcResGatheredByResByAgentAndNumAgentsGetheringByRes();
    void calcResGatheredByDevice();
    void calcResGatheredByDeviceByRes();
    void calcDevicesMade();
    void calcDevicesMadeByRes();
    void calcDevicesMadeWithDevDevicesByRes();
    void calcTimeSpentGatheringWithoutDevice();
    void calcTimeSpentGatheringWithDeviceAndTimeSpentMakingDevicesByAgent();
    void calcPercentResGatheredByDevice();
    void calcPercentResGatheredByDeviceByRes();

public:
    ProductionStats();
    void dailyUpdate ();

    vector<double> getResGathered() { return resGathered; };
    vector< vector < vector<double> > > getResGatheredByResByAgent() { return resGatheredByResByAgent; };
    vector<vector<double> > getResGatheredByRes() { return resGatheredByRes; };
    vector<vector<double> > getResGatheredByGroup() { return resGatheredByGroup; };

    vector< vector<double> > getDevicesMade() { return devicesMade; };
	vector< vector< vector<double> > > getDevicesMadeByRes() { return devicesMadeByRes; };     /*BRH 10.11.2017 */
    vector< vector< vector<double> > > getDevicesMadeByGroup() { return devicesMadeByGroup; };
    vector< vector<double> > getTimeSpentGatheringWithDevice() { return timeSpentGatheringWithDevice; };
    vector<vector< vector<double> > > getTimeSpentGatheringWithDeviceByRes() { return timeSpentGatheringWithDeviceByRes; };
    vector< vector<vector<double> > > getTimeSpentGatheringWithDeviceByGroup() { return timeSpentGatheringWithDeviceByGroup; };
    vector<double> getTimeSpentGatheringWithoutDevice() { return timeSpentGatheringWithoutDevice; };
    vector<vector<double> > getTimeSpentGatheringWithoutDeviceByRes() { return timeSpentGatheringWithoutDeviceByRes; };
    vector<vector<double> > getTimeSpentGatheringWithoutDeviceByGroup() { return timeSpentGatheringWithoutDeviceByGroup; };
    vector< vector<double> > getTimeSpentMakingDevices() { return timeSpentMakingDevices; };
    vector<vector<vector<double> > > getTimeSpentMakingDevicesByGroup() { return timeSpentMakingDevicesByGroup; };
    vector< vector<double> > getPercentResGatheredByDevice() { return percentResGatheredByDevice; };
    vector< vector<double> > getDevicesMadeWithDevDevice() { return devicesMadeWithDevDevice; };
    vector< vector<vector<double> > > getDevicesMadeWithDevDeviceByGroup() { return devicesMadeWithDevDeviceByGroup; };
    static const bool DEBUG_PRODUCTIONSTATS = false;
};


class OtherStats
{
    /**
     * The number of agents that are active on a given day.
     * vector is indexed by day.
     */
    vector<int> activeAgents;

    /**
     * The number of agents in each group that are active on a given day.
     * First indexed by group number, then by day.
     */
    vector< vector<int> > activeGroupAgents;
    /**
     * The sum of the resources held (I think) by all agents for each day.
     * Vector is indexed by day.
     */
    vector<double> sumRes;																															//JYC: changed to type double
    /**
     * The sum of the resources held by each agent, for each day.
     * Vectors are indexed by agentId and day.
     */
    vector< vector<double> > sumResByAgent;																							//JYC: changed to type double
    /**
     * The sum of the resources held by each group / type, for each day.
     * Vectors are indexed by groupId and day.
     */
    vector< vector<double> > sumResByGroup;																							//JYC: changed to type double

    /**
     * The sum of the utilityToday values for agents, for each day.
     * Vector is indexed by day.
     */
    vector<double> sumUtil;
    /**
     * The utilityToday values for each agent, for each day.
     * Indexed by agentId and then day.
     */
    vector< vector<double> > sumUtilByAgent;
    /**
     * The utilityToday values for each group / type, for each day.
     * Indexed by groupId and then day.
     */
    vector< vector<double> > sumUtilByGroup;

    /**
     * Outer vector: Each type of device
     * Inner vector: The number of this type of device that is known
     */
    vector< vector<double> > numberOfInventedDevices;																				//JYC: changed to type double

    void calcSumResourcesByAgentByGroup();
    void calcSumUtility();
    void calcNumDevicesInvented();

public:
    OtherStats();
    void dailyUpdate();
    double getSumResBack()  { return sumRes.back(); };																						//JYC: changed to type double
    double getSumUtilBack() { return sumUtil.back(); };																						//JYC: changed to type double
    vector<double> getSumUtil() { return sumUtil; };																							//JYC: changed to type double
    vector<vector<double> > getSumUtilByAgent() { return sumUtilByAgent; };
    vector<vector<double> > getSumUtilByGroup() { return sumUtilByGroup; };
    vector<int> getActiveAgents() { return activeAgents; };
    vector< vector<int> > getActiveGroupAgents() { return activeGroupAgents; };
    vector<double> getSumRes() { return sumRes; };
    vector<vector<double> > getSumResByGroup() { return sumResByGroup; };										//JYC: changed to type double
    vector< vector<double> > getNumberOfInventedDevices() { return numberOfInventedDevices; };	//JYC: changed to type double
    void getSumResByAgent();
    static const bool DEBUG = false;
    static const bool DEBUG_OTHERSTATS = false;
    static const bool ENTER_EXIT_METHODS = false;
};

#endif /* STATSTRACKER_H_ */
