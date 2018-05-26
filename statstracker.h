
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
    vector<int> soldExchanges;
    /**
     * How much is bought for each resource.
     * length = NUM_RESOURCES
     */
    vector<int> boughtExchanges;
    /**
     * \struct deviceExchange
     * Records the number of exchanges for a type of device.
     */
    struct deviceExchange {
        int sold;               //!< the number of the type of device sold
        vector<int> bought;     //!< a vector of the number of the type of device bought for each resource
    };
    vector< deviceExchange > deviceExchanges; //!< a vector of deviceExchange struct. Indexed by device type

public:
    DayExchangeStats();
    /**
     * \return the vector of sold number for each resource
     */
    vector<int> getSoldExchanges() { return soldExchanges; }
    /**
     * \return the vector of bought number for each resource
     */
    vector<int> getBoughtExchanges() { return boughtExchanges; }
    void addToSoldExchanges(int resId, int newVal) { soldExchanges[resId] += newVal; }      //!< update soldExchange
    void addToBoughtExchanges(int resId, int newVal) { boughtExchanges[resId] += newVal; }  //!< update boughtExchange
};



class TradeStats
{
private:
    vector<int> resTradeVolume;				        //!< indexed by day #
    vector<vector<int> > resTradeVolumeByGroup;     //!< indexed by group # then day #
    vector<int> resTradeVolumeCrossGroup;           //!< index by day #
    vector<vector<int> > resTradeVolumeWithinGroup; //!< indexed by group # then day #

    vector<int> resTradeForDeviceVolume;		    //!< indexed by day #
    vector<vector<int> > resTradeForDeviceVolumeByGroup;    //!< indexed by group # then day #
    vector<int> resTradeForDeviceVolumeCrossGroup;          //!< indexed by day #
    vector<vector<int> > resTradeForDeviceVolumeWithinGroup;//!< indexed by group # then day #

    vector< DayExchangeStats > dayResExchanges;		//!< length NUM_RESOURCES
    vector< vector <vector <vector<int> > > >resExchanges;/**< indexed by day first, then resId
              then 0th is soldExchanges and 1st is boughtExchanges, both vectors of ints. */

public:
    TradeStats();
    void dailyUpdate();
    void newExchange(ResourcePair &pair);
// BRH 11.11.2017 New vectors to save individual trades in memory
	vector<int> tradeLog(); //!< laundry list of trade info

    vector<int> getResTradeVolume() { return resTradeVolume; };
    vector<vector<int> > getResTradeVolumeByGroup() { return resTradeVolumeByGroup; };
    vector<int> getResTradeVolumeCrossGroup() { return resTradeVolumeCrossGroup; };
    vector<vector<int> > getResTradeVolumeWithinGroup() { return resTradeVolumeWithinGroup; };

    vector<int> getResTradeForDeviceVolume() { return resTradeForDeviceVolume; };
    vector<vector<int> > getResTradeForDeviceVolumeByGroup() { return resTradeForDeviceVolumeByGroup; };
    vector<int> getResTradeForDeviceVolumeCrossGroup() { return resTradeForDeviceVolumeCrossGroup; };
    vector<vector<int> > getResTradeForDeviceVolumeWithinGroup() { return resTradeForDeviceVolumeWithinGroup; };
};



/**
 * ProductionStats
 */
class ProductionStats
{
    vector<int> resGathered;			                /**< indexed by day # */
    vector<vector<int> > resGatheredByAgent; 	        /**< indexed by agentId, then day */
    vector<vector<int> > resGatheredByGroup;            /**< indexed by groupId, then day */
    vector<vector<int> > numAgentsGatheringByRes;       /**< indexed by resId, then day */
    vector<vector<int> > resGatheredByRes;              /**< indexed by resId, then day */
    vector< vector < vector<int> > > resGatheredByResByAgent; /**< indexed by resId, then agentId, then day */
    vector< vector<int> > resGatheredByDevice;          /**< indexed by device_name_t (first 4), then day */
    vector< vector< vector<int> > > resGatheredByDeviceByRes; /**< indexed by device_name_t (first 4), then resId, then day. */
    vector< vector<int> > devicesMade;		            /**< indexed by device_name_t, then day */
    vector< vector< vector<int> > > devicesMadeByRes;   /**< indexed by device_name_t, then resId, then day */
    vector< vector< vector<int> > > devicesMadeByGroup; /**< indexed by agent type, then device_name_t, then day */
    /**
     * Number of devDevices made for this resource.
     * indexed by device_name_t (only for DEVMACHINE and DEVFACTORY), and then day
     */
    vector< vector<int> > devicesMadeWithDevDevice;
    /**
     * only for DEVMACHINE and DEVFACTORY
     * indexed by device_name_t, then resId, then day
     */
    vector< vector< vector <int> > > devicesMadeWithDevDeviceByRes;

    /*
     * Index by agent type, then by device_name_t, then day.
     */
    vector< vector< vector <int> > > devicesMadeWithDevDeviceByGroup;
    /**
     * Each int is the total time spent by society without a device.
     * indexed by day
     */
    vector<int> timeSpentGatheringWithoutDevice;
    vector< vector<int> > timeSpentGatheringWithoutDeviceByAgent;   /**< indexed by agentId, then day */
    vector< vector<int > > timeSpentGatheringWithoutDeviceByGroup;  /**< indexed by groupId, then day */
    vector< vector<int> > timeSpentGatheringWithDevice;             /**< indexed by device_name_t (first 4), then day */
    /**
     * Total time this agent spent gathering with a device.
     * indexed by device_name_t (first 4), then agentId, then day.
     */
    vector< vector< vector<int> > > timeSpentGatheringWithDeviceByAgent;
    vector< vector< vector<int> > > timeSpentGatheringWithDeviceByGroup;         // *** indexed by device_name_t, then group id, then day
    /**
     * Time the society spent gathering devices each day.
     * indexed by device_name_t, then day
     */
    vector< vector<int> > timeSpentMakingDevices;
     /**
      * Time this agent spent gathering this device.
      * indexed by device_name_t, then agentId, then day.
      */
    vector< vector< vector<int> > > timeSpentMakingDevicesByAgent;
    vector< vector< vector<int> > > timeSpentMakingDevicesByGroup;      // *** indexed by device_name_t, then group id, then day
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

    vector<int> getResGathered() { return resGathered; };
    vector< vector < vector<int> > > getResGatheredByResByAgent() { return resGatheredByResByAgent; };
    vector<vector<int> > getResGatheredByRes() { return resGatheredByRes; };
    vector<vector<int> > getResGatheredByGroup() { return resGatheredByGroup; };

    vector< vector<int> > getDevicesMade() { return devicesMade; };
	vector< vector< vector<int> > > getDevicesMadeByRes() { return devicesMadeByRes; };     /*BRH 10.11.2017 */
    vector< vector< vector<int> > > getDevicesMadeByGroup() { return devicesMadeByGroup; };
    vector< vector<int> > getTimeSpentGatheringWithDevice() { return timeSpentGatheringWithDevice; };
    vector< vector<vector<int> > > getTimeSpentGatheringWithDeviceByGroup() { return timeSpentGatheringWithDeviceByGroup; };
    vector< int > getTimeSpentGatheringWithoutDevice() { return timeSpentGatheringWithoutDevice; };
    vector<vector<int> > getTimeSpentGatheringWithoutDeviceByGroup() { return timeSpentGatheringWithoutDeviceByGroup; };
    vector< vector<int> > getTimeSpentMakingDevices() { return timeSpentMakingDevices; };
    vector<vector<vector<int> > > getTimeSpentMakingDevicesByGroup() { return timeSpentMakingDevicesByGroup; };
    vector< vector<double> > getPercentResGatheredByDevice() { return percentResGatheredByDevice; };
    vector< vector<int> > getDevicesMadeWithDevDevice() { return devicesMadeWithDevDevice; };
    vector< vector<vector<int> > > getDevicesMadeWithDevDeviceByGroup() { return devicesMadeWithDevDeviceByGroup; };
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
    vector<int> sumRes;
    /**
     * The sum of the resources held by each agent, for each day.
     * Vectors are indexed by agentId and day.
     */
    vector< vector<int> > sumResByAgent;
    /**
     * The sum of the resources held by each group / type, for each day.
     * Vectors are indexed by groupId and day.
     */
    vector< vector<int> > sumResByGroup;

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
    vector< vector<int> > numberOfInventedDevices;

    void calcSumResourcesByAgentByGroup();
    void calcSumUtility();
    void calcNumDevicesInvented();

public:
    OtherStats();
    void dailyUpdate();
    int getSumResBack()  { return sumRes.back(); };
    int getSumUtilBack() { return sumUtil.back(); };
    vector<double> getSumUtil() { return sumUtil; };
    vector<vector<double> > getSumUtilByAgent() { return sumUtilByAgent; };
    vector<vector<double> > getSumUtilByGroup() { return sumUtilByGroup; };
    vector<int> getActiveAgents() { return activeAgents; };
    vector< vector<int> > getActiveGroupAgents() { return activeGroupAgents; };
    vector<int> getSumRes() { return sumRes; };
    vector<vector<int> > getSumResByGroup() { return sumResByGroup; };
    vector< vector<int> > getNumberOfInventedDevices() { return numberOfInventedDevices; };
    void getSumResByAgent();
    static const bool DEBUG = false;
    static const bool DEBUG_OTHERSTATS = false;
    static const bool ENTER_EXIT_METHODS = false;
};

#endif /* STATSTRACKER_H_ */
