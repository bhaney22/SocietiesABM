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


#include <boost/foreach.hpp>
#include "statstracker.h"
#include "globals.h"
#include "agent.h"
#include "device.h"

using namespace std;

/**
 * DayExchangeStats constructor
 */
DayExchangeStats::DayExchangeStats()
{
    soldExchanges = vector<int>(glob.NUM_RESOURCES, 0);
    boughtExchanges = vector<int>(glob.NUM_RESOURCES, 0);
    for (int i = 0; i < NUM_DEVICE_TYPES; i++){
        deviceExchange deviceEx;
        deviceEx.sold = 0;
        deviceEx.bought = vector<int>(glob.NUM_RESOURCES, 0);
        deviceExchanges.push_back(deviceEx);
    }
}

/**
 * TradeStats constructor
 */
TradeStats::TradeStats()
{
    resTradeVolume.clear();
    resTradeVolumeCrossGroup.clear();
    resTradeVolumeByGroup.clear();
    resTradeVolumeWithinGroup.clear();

    resTradeForDeviceVolume.clear();
    resTradeForDeviceVolumeByGroup.clear();
    resTradeForDeviceVolumeCrossGroup.clear();
    resTradeForDeviceVolumeWithinGroup.clear();

    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        resTradeVolumeByGroup.push_back(glob.EMPTY_VECTOR_OF_INTS);
        resTradeVolumeWithinGroup.push_back(glob.EMPTY_VECTOR_OF_INTS);
        resTradeForDeviceVolumeByGroup.push_back(glob.EMPTY_VECTOR_OF_INTS);
        resTradeForDeviceVolumeWithinGroup.push_back(glob.EMPTY_VECTOR_OF_INTS);
    }
    if (glob.SAVE_TRADES) {
        dayResExchanges.clear();
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            dayResExchanges.push_back(DayExchangeStats());
        }
        resExchanges.clear();
    }
}

/**
 * Update the information about trading.
 */
void TradeStats::dailyUpdate()
{
    /*
     * For each agent add the units they sold.  Then append the total
     * amount sold by the society to resTradeVolume.
     */
    int tradeVolume = 0;
    int tradeVolumeCrossGroup = 0;
    vector<int> tradeVolumeByGroup = vector<int>(glob.NUM_AGENT_GROUPS, 0);
    vector<int> tradeVolumeWithinGroup = vector<int>(glob.NUM_AGENT_GROUPS, 0);
    BOOST_FOREACH(Agent *ag, glob.agent) {
        int soldToday = ag->unitsSoldToday;
        int soldCross = ag->unitsSoldCrossGroupToday;
        tradeVolume += soldToday;
        tradeVolumeByGroup[ag->group] += soldToday;
        tradeVolumeCrossGroup += soldCross;
        tradeVolumeWithinGroup[ag->group] += (soldToday - soldCross);
    }
    resTradeVolume.push_back(tradeVolume);
    resTradeVolumeCrossGroup.push_back(tradeVolumeCrossGroup);
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        resTradeVolumeByGroup[gId].push_back(tradeVolumeByGroup[gId]);
        resTradeVolumeWithinGroup[gId].push_back(tradeVolumeWithinGroup[gId]);
    }

    /*
     * For each agent add up the number of units they sold for a device that day.
     * Then append that to the resTradeForDeviceVolume
     */
    int tradeForDeviceVolume = 0;
    int tradeForDeviceVolumeCrossGroup = 0;
    vector<int> tradeForDeviceVolumeByGroup = vector<int>(glob.NUM_AGENT_GROUPS, 0);
    vector<int> tradeForDeviceVolumeWithinGroup = vector<int>(glob.NUM_AGENT_GROUPS, 0);
    BOOST_FOREACH(Agent *ag, glob.agent) {
        int soldToday = ag->unitsSoldForDevicesToday;
        int soldCross = ag->unitsSoldCrossGroupForDevicesToday;
        tradeForDeviceVolume += soldToday;
        tradeForDeviceVolumeByGroup[ag->group] += soldToday;
        tradeForDeviceVolumeCrossGroup += soldCross;
        tradeForDeviceVolumeWithinGroup[ag->group] += (soldToday - soldCross);
    }
    resTradeForDeviceVolume.push_back(tradeForDeviceVolume);
    resTradeForDeviceVolumeCrossGroup.push_back(tradeForDeviceVolumeCrossGroup);
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        resTradeForDeviceVolumeByGroup[gId].push_back(tradeForDeviceVolumeByGroup[gId]);
        resTradeForDeviceVolumeWithinGroup[gId].push_back(tradeForDeviceVolumeWithinGroup[gId]);
    }

    if (glob.SAVE_TRADES) {
        vector <vector <vector<int> > > newResExchanges;
        vector <vector <int> > dayResExchTmp(2);	// 2 vectors of ints.
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            // push on two vectors of integers
            dayResExchTmp[0] = dayResExchanges[resId].getSoldExchanges();
            dayResExchTmp[1] = dayResExchanges[resId].getBoughtExchanges();
            dayResExchanges[resId] = DayExchangeStats();  // reset for next time.
            newResExchanges.push_back(dayResExchTmp);
        }
        resExchanges.push_back(newResExchanges);
    }
}

/**
 * Add the new exchange happened.
 */
void TradeStats::newExchange(ResourcePair &pair)
{
    int resA = pair.getAPick();
    int resB = pair.getBPick();
    int numA = pair.getNumAPicked();
    int numB = pair.getNumBPicked();
    /*
     * dayResExchanges[resA].getSoldExchanges()[resB] is the number of units
     * of resA given up in exchange for resB.
     * dayResExchanges[resB].getBoughtExchanges()[resA] is the number of
     * units of resB given up in exchange for resA.
     */
    dayResExchanges[resA].addToSoldExchanges(resB, numA);
    dayResExchanges[resA].addToBoughtExchanges(resB, numB);
    dayResExchanges[resB].addToSoldExchanges(resA, numB);
    dayResExchanges[resB].addToBoughtExchanges(resA, numA);
}

/**
 * ProductionStats constructor
 */
ProductionStats::ProductionStats()
{
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        resGatheredByAgent.push_back(glob.EMPTY_VECTOR_OF_INTS);
        timeSpentGatheringWithoutDeviceByAgent.push_back(glob.EMPTY_VECTOR_OF_INTS);
    }

    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        numAgentsGatheringByRes.push_back(glob.EMPTY_VECTOR_OF_INTS);
        resGatheredByRes.push_back(glob.EMPTY_VECTOR_OF_INTS);
        resGatheredByResByAgent.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
        for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
            resGatheredByResByAgent[resId].push_back(glob.EMPTY_VECTOR_OF_INTS);
        }
    }

    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        resGatheredByGroup.push_back(glob.EMPTY_VECTOR_OF_INTS);
        devicesMadeByGroup.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
        timeSpentGatheringWithDeviceByGroup.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
        timeSpentMakingDevicesByGroup.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
        timeSpentGatheringWithoutDeviceByGroup.push_back(glob.EMPTY_VECTOR_OF_INTS);
        devicesMadeWithDevDeviceByGroup.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
        for (int type = TOOL; type < glob.getNumDeviceTypes(); type++) {
            devicesMadeByGroup[gId].push_back(glob.EMPTY_VECTOR_OF_INTS);
            timeSpentGatheringWithDeviceByGroup[gId].push_back(glob.EMPTY_VECTOR_OF_INTS);
            timeSpentMakingDevicesByGroup[gId].push_back(glob.EMPTY_VECTOR_OF_INTS);
            devicesMadeWithDevDeviceByGroup[gId].push_back(glob.EMPTY_VECTOR_OF_INTS);
        }
    }

    for (int type = TOOL; type < glob.getNumDeviceTypes(); type++) {
        devicesMade.push_back(glob.EMPTY_VECTOR_OF_INTS);
        devicesMadeByRes.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
        /*
         * For devicesMadeWithDevDevice & devicesMadeWithDevDeviceByRes:
         * Put in place holder vectors.  At this point it is making 6 but we really only need two.
         * In order to access the vectors with expected indexes (to simplify our lives), we chose
         * to waste the four empty vectors at the beginning instead of needing to do offsets
         * elsewhere to access the data correctly.
         */
        devicesMadeWithDevDevice.push_back(glob.EMPTY_VECTOR_OF_INTS);
        devicesMadeWithDevDeviceByRes.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);

        timeSpentMakingDevices.push_back(glob.EMPTY_VECTOR_OF_INTS);
        timeSpentMakingDevicesByAgent.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
        timeSpentGatheringWithDevice.push_back(glob.EMPTY_VECTOR_OF_INTS);
        timeSpentGatheringWithDeviceByAgent.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);


        // for each agent ID
        for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
            timeSpentGatheringWithDeviceByAgent[type].push_back(glob.EMPTY_VECTOR_OF_INTS);
            timeSpentMakingDevicesByAgent[type].push_back(glob.EMPTY_VECTOR_OF_INTS);
        }

        // for each resource ID
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            devicesMadeByRes[type].push_back(glob.EMPTY_VECTOR_OF_INTS);
        }

        // only types 0-3 (TOOL, MACHINE, FACTORY, INDUSTRY)
        if (type <= INDUSTRY) {
            resGatheredByDevice.push_back(glob.EMPTY_VECTOR_OF_INTS);
            percentResGatheredByDevice.push_back(vector<double>());
            percentResGatheredByDeviceByRes.push_back(vector<vector<double> >());
            resGatheredByDeviceByRes.push_back(glob.EMPTY_VECTOR_OF_VECTORS_OF_INTS);
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                percentResGatheredByDeviceByRes[type].push_back(vector<double>());
                resGatheredByDeviceByRes[type].push_back(glob.EMPTY_VECTOR_OF_INTS);
            }
        }
        // only types 4-5 (DEVMACHINE, DEVFACTORY)
        else if (type == DEVMACHINE || type == DEVFACTORY) {
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                devicesMadeWithDevDeviceByRes[type].push_back(glob.EMPTY_VECTOR_OF_INTS);
            }
        }
    }
}

/**
 * Update the information about production.
 */
void ProductionStats::dailyUpdate()
{
    calcResGatheredAndResGatheredByAgentByGroup();
    calcResGatheredByResByAgentAndNumAgentsGetheringByRes();
    calcResGatheredByDevice();
    calcResGatheredByDeviceByRes();
    calcDevicesMade();
    calcDevicesMadeByRes();
    calcDevicesMadeWithDevDevicesByRes();
    calcTimeSpentGatheringWithoutDevice();
    calcTimeSpentGatheringWithDeviceAndTimeSpentMakingDevicesByAgent();
    calcPercentResGatheredByDevice();
    calcPercentResGatheredByDeviceByRes();
}


/**
 * Calculate and update the sum of gathered resoures by all agents,
 *      the number of gathered resources by each agent,
 *      the sum of gathered resources by each group of agents on each day.
 */
void ProductionStats::calcResGatheredAndResGatheredByAgentByGroup()
{
    /*
     * Go through each agent and add up the amount of each resource that was gathered that day.
     * This information will be added to lists by total of all agents and individually
     */
    int sumGathered = 0;
    vector<int> temp = vector<int>(glob.NUM_AGENT_GROUPS, 0);   // index by type, today's unitsGathered by each type
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        int totalResThisAgentGathered = 0;
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            totalResThisAgentGathered += glob.agent[aId]->getUnitsGatheredToday(resId);
        }
        resGatheredByAgent[aId].push_back(totalResThisAgentGathered);
        sumGathered += totalResThisAgentGathered;
        temp[glob.agent[aId]->group] += totalResThisAgentGathered;
    }
    resGathered.push_back(sumGathered);
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        resGatheredByGroup[gId].push_back(temp[gId]);
    }
}

/**
 * Calculate and update the number of resources gathered for each resource by each agent,
 *     and the number of resources gathered for each resource by all agents.
 */
void ProductionStats::calcResGatheredByResByAgentAndNumAgentsGetheringByRes()
{
    /*
     * For every resource begin to go through and if see if an agent gathered any today.
     * Add each amount gathered to sumResGathered
     * Append each agents individual amount to the resGatheredByResByAgent
     * If a resource was gathered by that agent add 1 to agentsMakingRes.
     * After all agents have been checked for a given resource, append agentsMakingRes to numAgentsGatheringbyRes
     * For each resource take the total gathered and append that to resGatheredByRes
     */
	 /* BRH 10.2.2017 testing */
	bool DEBUG_PRODUCTIONSTATS = false;
	 
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        int numAgentsGatheringRes = 0;
        int totalAmountOfThisResGathered = 0;

        for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
            int agentTotalThisResGathered =
                glob.agent[aId]->getUnitsGatheredToday(resId);
            resGatheredByResByAgent[resId][aId].push_back(agentTotalThisResGathered);
            totalAmountOfThisResGathered += agentTotalThisResGathered;
				if (agentTotalThisResGathered > 0) {
                numAgentsGatheringRes += 1;
				}
            /**
             * DEBUG_PRODUCTIONSTATS: not exist in python version
             */
            if (DEBUG_PRODUCTIONSTATS) {
                cout << "Resource Id: " << resId << " Agent Id: " << aId << endl;
                cout << "ResourcesGatheredByAgent: "
                     << agentTotalThisResGathered << endl;
            }
        }
        if (DEBUG_PRODUCTIONSTATS) {
            cout << "Number of Agents Gathering Res " << resId << ": "
                 << numAgentsGatheringRes << endl;
        }
        numAgentsGatheringByRes[resId].push_back(numAgentsGatheringRes);

        if (DEBUG_PRODUCTIONSTATS) {
            cout << "Total Amount of Res " << resId << " Gathered: "
                 << totalAmountOfThisResGathered << endl;
        }
        resGatheredByRes[resId].push_back(totalAmountOfThisResGathered);
    }
}

/**
 * Calculate and update the units gathered for all resourcess for each device.
 */
void ProductionStats::calcResGatheredByDevice()
{
    /*
     * We want to know the total amount of resources gathered for each device.
     * For each device set sum=0
     * Go through each agent look if they gathered with that device today.
     * After going through all agents for a given device, add total to resGatheredByDevice
     */

    for (int tempType = TOOL; tempType <= INDUSTRY; tempType++) {
        int sumGathered = 0;
        for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                sumGathered += glob.agent[aId]->getUnitsGatheredWithDevice(tempType, resId);
                if (DEBUG_PRODUCTIONSTATS) {
                    cout << "deviceType: " << tempType << " agentId: " << aId
                         << " resourceID: " << resId << endl;
                    cout << " Sum Resources Gathered With Devices: "
                         << sumGathered << endl;
                }
            }
        }
        resGatheredByDevice[tempType].push_back(sumGathered);
    }
}

/**
 * Calculate and update the units gathered by each device for each resource.
 */
void ProductionStats::calcResGatheredByDeviceByRes()
{
    /*
     * We want to know which devices are gathering how much of each resource.
     *
     *	Rearranged for resId line and the for type lines
     *		- We did this because when we need to push_back vectors the correct number of vectors.
     *		- Type gets called first before resId.  The flip flop makes sense.
     */

    for (int type = 0; type < glob.getNumResGatherDev(); type++) {
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            int sumGathered = 0;
            for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
                sumGathered += glob.agent[aId]->getUnitsGatheredWithDevice(type, resId);
            }
            if (DEBUG_PRODUCTIONSTATS) {
                cout << "Number Of Resources Gathered With Device Type" << type
                     << " in order to gain resource " << resId << endl;
            }
            resGatheredByDeviceByRes[type][resId].push_back(sumGathered);
        }
    }
}

/**
 * Looking to identify how many of each type of device was made today.
 * For each type in devicesMade, set made =0 and look in agent
 * Set the sum of each agent =0 and look at the each resource for each agent
 * If they invented that device for that resource add that to agent tempSum
 * After all resources have been checked for that device and that agent add the total to made
 * Once all agents have been checked append made to the devicesMade list for the given device
 */
void ProductionStats::calcDevicesMade()
{
    vector<vector<int> > temp;  //*< indexed by agent type, then device type */
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        temp.push_back(glob.EMPTY_VECTOR_OF_INTS);
        temp[gId] = vector<int>(glob.getNumDeviceTypes(), 0);
    }

    for (int type = 0; type < glob.getNumDeviceTypes(); type++) {
        int tempDevicesMade = 0;
        for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
            int tempSum = 0;
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                tempSum += glob.agent[aId]->devProp[type][resId].devicesMadeToday;
            }
            tempDevicesMade += tempSum;
            temp[glob.agent[aId]->group][type] += tempSum;
        }
        devicesMade[type].push_back(tempDevicesMade);
    }
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int type = 0; type < glob.getNumDeviceTypes(); type++) {
            devicesMadeByGroup[gId][type].push_back(temp[gId][type]);
        }
    }
}

/**
 * Similar process but now answering the question:  How many of a certain
 * device has been made for a given resource. 
 */
void ProductionStats::calcDevicesMadeByRes()
{
    for (int type = 0; type < glob.getNumDeviceTypes(); type++) {
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            int tempMade = 0;
            for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
                tempMade += glob.agent[aId]->getDevicesMadeToday(resId, type);
                if (DEBUG_PRODUCTIONSTATS) {
                    cout << "Agent " << aId << " Made device " << type
                         << " from resource" << resId << endl;
                }
            }
            devicesMadeByRes[type][resId].push_back(tempMade);
        }
    }
}

/**
 * This calculated the number of devices made by device making devices.
 * We want to know the total of devDevices made and the total devDevices
 * made for each resource.
 */
void ProductionStats::calcDevicesMadeWithDevDevicesByRes()
{
    vector<vector<int> > temp;  //*< indexed by agent type, then device type */
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        temp.push_back(glob.EMPTY_VECTOR_OF_INTS);
        temp[gId] = vector<int>(2, 0);  // only DEVMACHINE and DEVFACTORY;
    }
    for (int type = DEVMACHINE; type <= DEVFACTORY; type++) {
        int devMadeWithDevice = 0;
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            // this gets reset for each resource and set the inner vector
            int devMadeWithDeviceByRes = 0;
            for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
                // The next two lines look up the same thing, but one will
                // be adding for each type and the other is adding for each
                // type and resource.
                devMadeWithDeviceByRes +=
                    glob.agent[aId]->devicesMadeWithDevDevicesToday[type][resId];
                devMadeWithDevice +=
                    glob.agent[aId]->devicesMadeWithDevDevicesToday[type][resId];
                temp[glob.agent[aId]->group][type-4] += glob.agent[aId]->devicesMadeWithDevDevicesToday[type][resId];

                if (DEBUG_PRODUCTIONSTATS) {
                    cout << "Devices Made With DevDevice:  Type: " << type
                         << "  resID: " << resId << endl;
                    cout << "devMadeWithDeviceByRes   Type: " << type
                         << "  resID: " << resId << endl;
                    cout << "devMadeWithDevice   Type: " << type
                         << "  resID: " << resId << endl;
                    cout << "agentID: " << aId << "  resID: " << resId
                         << " type: " << type << endl;
                }
            }
            devicesMadeWithDevDeviceByRes[type][resId].push_back(devMadeWithDeviceByRes);
        }
        devicesMadeWithDevDevice[type].push_back(devMadeWithDevice);
    }
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int type =DEVMACHINE; type <= DEVFACTORY; type++) {
            devicesMadeWithDevDeviceByGroup[gId][type].push_back(temp[gId][type]);
        }
    }
}


/**
 * The goal is to save the time each agent spent gathering without devices
 * and the amount of time society spent gathering without devices.
 */
void ProductionStats::calcTimeSpentGatheringWithoutDevice()
{
    int withoutDeviceTotal = 0;
    vector<int> temp = vector<int>(glob.NUM_AGENT_GROUPS, 0);
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        int withoutDeviceEachAgent = glob.agent[aId]->timeSpentGatheringWithoutDeviceToday;
        withoutDeviceTotal += withoutDeviceEachAgent;
        timeSpentGatheringWithoutDeviceByAgent[aId].push_back(withoutDeviceEachAgent);
        temp[glob.agent[aId]->group] += withoutDeviceEachAgent;
    }
    timeSpentGatheringWithoutDevice.push_back(withoutDeviceTotal);
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        timeSpentGatheringWithoutDeviceByGroup[gId].push_back(temp[gId]);
    }
}


/**
 * This keeps 4 statistics:
 * Amount of time each agent spends gathering resources with a device
 * Amount of time the population spends gathering resources with a device
 * Amount of time each agent spends making devices
 * Amount of time the population spends making devices
 */
void ProductionStats::calcTimeSpentGatheringWithDeviceAndTimeSpentMakingDevicesByAgent()
{
    /* NOTE: python code only computes timeSpentGathering* for TOOL
     * -> INDUSTRY, not the last two.  This code does not make that
     * distinction. So be careful when using it in save data.
     */
    vector<vector<int> > tempTimeSpentGathering, tempTimeSpentMaking;  //*< indexed by agent type, then device type */
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        tempTimeSpentGathering.push_back(glob.EMPTY_VECTOR_OF_INTS);
        tempTimeSpentGathering[gId] = vector<int>(glob.getNumDeviceTypes(), 0);

        tempTimeSpentMaking.push_back(glob.EMPTY_VECTOR_OF_INTS);
        tempTimeSpentMaking[gId] = vector<int>(glob.getNumDeviceTypes(), 0);
    }

    for (int type = 0; type < glob.getNumDeviceTypes(); type++) {
        int totalTimeWithDevice = 0;
        int totalTimeSpentMakingDevices = 0;
        for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
            // Grab the amount of time this agent spent gathering
            // with/making devices for this type. 
            int timeWithDeviceByAgent = glob.agent[aId]->timeSpentGatheringWithDeviceToday[type];
            int timeMakingDevicesByAgent = glob.agent[aId]->timeSpentMakingDevicesToday[type];

            // add this agents time spent with the previous agents
            totalTimeWithDevice += timeWithDeviceByAgent;
            totalTimeSpentMakingDevices += timeMakingDevicesByAgent;

            // save this individual agent's time data in a vector.
            timeSpentGatheringWithDeviceByAgent[type][aId].push_back(timeWithDeviceByAgent);
            timeSpentMakingDevicesByAgent[type][aId].push_back(timeMakingDevicesByAgent);
            tempTimeSpentGathering[glob.agent[aId]->group][type] += timeWithDeviceByAgent;
            tempTimeSpentMaking[glob.agent[aId]->group][type] += timeMakingDevicesByAgent;
        }
        // save the time data of the population as a whole.
        timeSpentGatheringWithDevice[type].push_back(totalTimeWithDevice);
        timeSpentMakingDevices[type].push_back(totalTimeSpentMakingDevices);
    }
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int type = 0; type < glob.getNumDeviceTypes(); type++) {
            timeSpentGatheringWithDeviceByGroup[gId][type].push_back(tempTimeSpentGathering[gId][type]);
            timeSpentMakingDevicesByGroup[gId][type].push_back(tempTimeSpentMaking[gId][type]);
        }
    }
}

/**
 * Of all the resources gathered what percent of them were gathered with a device?
 */
void ProductionStats::calcPercentResGatheredByDevice()
{
    // Used enums because the resGatheredByDevice is only filled with the first 4.
    // Agent class does not fill vector indexes 5 and 6.
    for (int type = TOOL; type <= INDUSTRY; type++) {
        if (DEBUG_PRODUCTIONSTATS) {
            cout << "Device type: " << type << endl;
        }
        double percentGathered = double(resGatheredByDevice[type].back()) / double(resGathered.back());

        percentResGatheredByDevice[type].push_back(percentGathered);
        if (DEBUG_PRODUCTIONSTATS) {
            cout << "ResGatheredByDevice for type: " << type << " =  "
                 << resGatheredByDevice[type].back() << endl;
        }
        if (DEBUG_PRODUCTIONSTATS) {
            cout << "ResGathered =  " << resGathered.back() << endl;
        }

    }
}

/**
 * What percent of each resource was gathered by a device.
 * Indexed by only the devices that gather resources.
 */
void ProductionStats::calcPercentResGatheredByDeviceByRes()
{
    for (int type = TOOL; type <= INDUSTRY; type++) {
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            double percentGatheredBDBR = 0;
            if (resGatheredByRes[resId].back() > 0) {
                percentGatheredBDBR = double(resGatheredByDeviceByRes[type][resId].back()) /
                    (resGatheredByRes[resId].back());
                percentResGatheredByDeviceByRes[type][resId].push_back(percentGatheredBDBR);
                if (DEBUG_PRODUCTIONSTATS) {
                    cout << "% of res " << resId << "gathered by device "
                         << type << " : " << endl;
                }

            } else {
                percentResGatheredByDeviceByRes[type][resId].push_back(0.0);
            }
        }
    }
}



/******************************************************************
 *
 * 					OtherStats
 *
 ******************************************************************/

/**
 * OtherStats Default Constructor:
 * 		This default constructor creates all the necessary data structures (primarily
 * 		vectors and nested vectors with type int at their lowest level). The structure
 * 		of the loops in this constructor allows us to reduce the total number of
 * 		loops the program must complete in order to gather statistics.
 */
OtherStats::OtherStats()
{
    activeAgents = vector<int>();
    sumRes = vector<int>();
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        sumResByAgent.push_back(vector<int>());
    }
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        sumResByGroup.push_back(vector<int>());
        activeGroupAgents.push_back(vector<int>());
    }

    sumUtil = vector<double>();
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        sumUtilByAgent.push_back(vector<double>());
    }
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        sumUtilByGroup.push_back(vector<double>());
    }

    for (int i = 0; i < glob.getNumDeviceTypes(); i++) {
        numberOfInventedDevices.push_back(glob.EMPTY_VECTOR_OF_INTS);
    }
}

/**
 * Calls functions to handle updating of resource stats, total utility,
 * statistics about devices in existence.
 */
void OtherStats::dailyUpdate()
{
    activeAgents.push_back(glob.activeAgents);		// For saving results and graphing.
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        activeGroupAgents[i].push_back(glob.NUM_ACTIVE_AGENTS_IN_GROUP[i]);
    }
    calcSumResourcesByAgentByGroup();
    calcSumUtility();
    calcNumDevicesInvented();
}

/**
 * Updates totals of resources held by agent and by population
 */
void OtherStats::calcSumResourcesByAgentByGroup()
{
    if (ENTER_EXIT_METHODS) {
        cout << "[statsTracker.cpp] OtherStats() - Enter" << endl;
    }

    int sumResources = 0;
    vector<int> temp = vector<int>(glob.NUM_AGENT_GROUPS, 0);
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        int totalHeldByThisAgent = 0;
        for (int resP = 0; resP < glob.NUM_RESOURCES; resP++) {
            if (DEBUG_OTHERSTATS) {
                cout << "[statsTracker.cpp] OtherStats() - getting NumResources, aId = "
                     << aId << " resId = " << resP << endl;
            }

            totalHeldByThisAgent += glob.agent[aId]->getHeld(resP);
        }
        sumResByAgent[aId].push_back(totalHeldByThisAgent);
        sumResources += totalHeldByThisAgent;
        temp[glob.agent[aId]->group] += totalHeldByThisAgent;
    }
    sumRes.push_back(sumResources);
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        sumResByGroup[gId].push_back(temp[gId]);
    }
    if (ENTER_EXIT_METHODS) {
        cout << "[statsTracker.cpp] OtherStats() - Exit" << endl;
    }

}

/**
 * Updates total utility owned by population per day
 */
void OtherStats::calcSumUtility()
{
    double sumUtility = 0;
    vector<double> sumUtilityByGroup = vector<double>(glob.NUM_AGENT_GROUPS, 0.0);
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        int agentUtility = 0;
        agentUtility = glob.agent[aId]->utilityToday;
        sumUtilByAgent[aId].push_back(agentUtility);
        sumUtilityByGroup[glob.agent[aId]->group] += agentUtility;
        sumUtility += agentUtility;
    }
    sumUtil.push_back(sumUtility);
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        sumUtilByGroup[gId].push_back(sumUtilityByGroup[gId]);
    }
}

/**
 * Updates totals devices in existence
 */
void OtherStats::calcNumDevicesInvented()
{
    for (int i = 0; i < glob.getNumDeviceTypes(); i++) {
        int knownDevices = 0;
        for (int j = 0; j < glob.NUM_RESOURCES; j++) {
            if ((glob.discoveredDevices[i][j] != 0)
                && (glob.discoveredDevices[i][j]->agentsKnown() != 0)) {
                knownDevices += 1;
            }
        }
        numberOfInventedDevices[i].push_back(knownDevices);
    }

    if (DEBUG) {
        cout << "Number of Invented Devices (By Type, By Day): ";
        cout << "TOOL: " << numberOfInventedDevices[0].back() << ", ";
        cout << "MACHINE: " << numberOfInventedDevices[1].back() << ", ";
        cout << "FACTORY: " << numberOfInventedDevices[2].back() << ", ";
        cout << "INDUSTRY: " << numberOfInventedDevices[3].back() << ", ";
        cout << "DEVMACHINE: " << numberOfInventedDevices[4].back() << ", ";
        cout << "DEVFACTORY: " << numberOfInventedDevices[5].back() << endl;
    }
}

/**
 * Update the sum of resources held by each agent.
 */
void OtherStats::getSumResByAgent()
{
    int tempDayNum = 0;
    int tempAgentId = 0;

    // We have a "2D" vector vvi (vector of vector of int's) that we need to iterate through.  We
    // need to set the iterator for each vector.
    vector<int>::iterator dayIT;
    vector<vector<int> >::iterator agentIT;

    //	For every agent reset the day number equal to 0
    for (agentIT = sumResByAgent.begin(); agentIT != sumResByAgent.end(); agentIT++) {
        tempDayNum = 0;
        //  For every day look up how many resources this agent owns
        for (dayIT = (*agentIT).begin(); dayIT != (*agentIT).end(); dayIT++) {
            if (DEBUG) {
                cout << "Sum Res By Agent " << tempAgentId << ": "
                     << sumResByAgent[tempAgentId][tempDayNum]
                     << " on day #" << tempDayNum << endl;
            }
            tempDayNum++;
        }
        tempAgentId++;
    }
}







