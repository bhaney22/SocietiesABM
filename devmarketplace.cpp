/*
 * societies  Simulates societies and thier growth based on trade,
 * experience and inventions.
 * Copyright (C) 2011 Tony Ditta, Michael Koster

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. 

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. 

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


/**
 * The devmarketplace module contains the definitions of the
 * DeviceMarketplace class and the DevicePair class -- both of which help
 * control device trading
 */

#include <algorithm>
#include <vector>
#include <cmath>
#include <limits>
#include <list>
#include <numeric>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>   //TODO: for test. Need to be deleted
#include "device.h"
#include "devmarketplace.h"
#include "agent.h"
#include "logging.h"

using namespace std;

/**
 * Agents trade devices of type device.
 */
void DeviceMarketplace::tradeDevices(device_name_t device)
{
    this->device = device;
    BOOST_FOREACH(Agent *ag, glob.agent) {
        ag->newDeviceTrade(device);
    }

    if (util.deviceExists(device)) {
        for (int round = 0; round < glob.DEVICE_TRADE_ROUNDS; round++) {
            genDevicePairs();
        }
    }
}


/**
 * An agent is randomly selected and chooses the agent it most prefers
 * to trade devices with, if that agent has already traded this round or
 * if the two agents have already traded this day, the agent selects its
 * next preference and so on. If the agent has no preferred agents, all
 * the preferred agents have already traded this round, or the agent has
 * already been paired with all its preference today, the agent gets a
 * random partner.
 */
void DeviceMarketplace::genDevicePairs()
{
    Agent *partner;
    vector<Agent *> agentList;
    vector<DevicePair> pairs;

    BOOST_FOREACH(Agent *ag, glob.agent) {
        agentList.push_back(ag);
        ag->deviceBoughtThisRound.clear();
    }

    /*
     * While there are agents who have not traded, agents get paired up.
     */
    while (agentList.size() > 1) {
        bool traded = false;
        /* The currentTrader is randomly selected from the list of agents
         * that haven't traded.
         */
#ifdef DONT_RANDOMIZE
        int loc = 0;
#else
        int loc = glob.random_int(0, agentList.size()); // grab random agent location
#endif
        vector<Agent *>::iterator it = agentList.begin(); // iterator for the agent list
        advance(it, loc);	     // cannot use ++ or + loc with list iterators.
        Agent *currentTrader = *it;  // Agent * is the value at the iterator.
        agentList.erase(it);         // remove agent at index 'it' from the list

        /*
         * The agent returns an ordered list of the agents that it wants to
         * trade with (the first agent in the list is the most preferred).
         */
        vector<int> prefDeviceTraders = currentTrader->preferredDeviceTraders(device);
        /*
         * Check each agent in currentTrader's desired trade partners to see
         * if that agent is available for trade.
         */
        for (unsigned prefIdx = 0; prefIdx < prefDeviceTraders.size(); prefIdx++) {

            /*
             * If the preferred agent has not traded this round and hasn't 
             * traded this device with currentTrader today, they immediately
             * get paired up.
             */
            int pref = prefDeviceTraders[prefIdx];
            /* check if glob.agent[pref] is in agentList */
            bool tmpNotTraded =
                (find(agentList.begin(), agentList.end(),
                      glob.agent[pref]) == agentList.end());
            
            vector<Agent *>::iterator hasTradedThisRound =
                find(currentTrader->tradedDeviceWithThisRound.begin(),
                     currentTrader->tradedDeviceWithThisRound.end(),
                     glob.agent[pref]);
            vector<Agent *>::iterator endOfCurrentTraderVect =
                currentTrader->tradedDeviceWithThisRound.end();
            /*
             * if preferred agent has not traded yet this round
             * if preferred agent has not traded this device today with the
             * current agent.
             */
            if (! tmpNotTraded && (hasTradedThisRound == endOfCurrentTraderVect)) {
                // set partner = to partner we are working with at the moment
                it = agentList.begin();
                hasTradedThisRound = find(agentList.begin(), agentList.end(),
                                          glob.agent[pref]);
                advance(it, distance(agentList.begin(), hasTradedThisRound));
                partner = *it;
                // remove the partner from the viable partners list
                agentList.erase(it);
                traded = true;
                break;
            }
        }

        /*
         * If none of the preferred agents are available, currentTrader
         * searches for an agent that it hasn't traded the current device
         * with today.
         */
        if (! traded) {
            for (unsigned partnersIdx = 0; partnersIdx < agentList.size(); partnersIdx++) {
                Agent *partners = agentList[partnersIdx];
                if (find(currentTrader->tradedDeviceWithThisRound.begin(),
                         currentTrader->tradedDeviceWithThisRound.end(),
                         partners) == currentTrader->tradedDeviceWithThisRound.end()) {
                    it = agentList.begin();
                    advance(it, partnersIdx);
                    partner = *it;
                    agentList.erase(it);
                    traded = true;
                    break;
                }
            }
        }

        /*
         * If no partner has been found for currentTrader yet, a partner is
         * assigned randomly.
         */
        if (! traded) {
            loc = glob.random_int(0, agentList.size());
            it = agentList.begin();
            advance(it, loc);	// cannot use ++ or + loc with list iterators.
            partner = *it;
            agentList.erase(it);
        }

        // add the pairs to the end of the pairs list.
        pairs.push_back(DevicePair(currentTrader, partner, device));
    }

    for (unsigned eachPair = 0; eachPair < pairs.size(); eachPair++){
        pairs[eachPair].pairDeviceTrade();
    }

}

/**
 * DevicePair constructor.
 * \param agentA an agent
 * \param agentB another agent other than agentA
 * \param deviceType the type of device the two agents want to trade
 */
DevicePair::DevicePair(Agent *agentA, Agent *agentB, device_name_t deviceType)
{
    this->deviceType = deviceType;
    this->agentA = agentA;
    this->agentB = agentB;
    this->deviceBuyer = agentA;
    this->deviceSeller = agentB;
    currentTradeAttempts = 0;
    deviceBuyerSelling.clear();
    deviceSellerSelling.clear();
    this->deviceBuyerPick = -1;
    finalOffer.clear();
    this->deviceSellerUtilGain = 0;
    this->deviceSellerUtilLoss = 0;
    this->deviceBuyerUtilGain = 0;
    this->deviceBuyerUtilLoss = 0;
}

/**
 * The agents begin trading devices by creating the list of offers
 * made by the deviceSeller (who offers devices) and deviceBuyer (who
 * offers a menu of resources)
 */
void DevicePair::pairDeviceTrade()
{
    if (currentTradeAttempts == 0) {
        // Each agent notes that it has traded with the other agent this round
        deviceSeller->tradedDeviceWithThisRound.push_back(deviceBuyer);
        deviceBuyer->tradedDeviceWithThisRound.push_back(deviceSeller);
    }
    if (deviceBuyer->inSimulation && deviceSeller->inSimulation) {
        double tempMax = 0.0;
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            if (deviceSeller->devProp[deviceType][resId].getDeviceExperience() > tempMax) {
                tempMax = deviceSeller->devProp[deviceType][resId].getDeviceExperience();
            }
        }
        if (currentTradeAttempts < glob.DEVICE_TRADE_ATTEMPTS && tempMax >= 1.0) {
            // One agent is the device buyer, the other agent is the device seller
            deviceBuyerSelling = deviceBuyerAvailableRes();
            // The device seller offers the devices that it knows how to build and
            // for which it has adequate time and resources
            deviceSellerSelling = deviceSellerAvailableDevices();
            if (deviceSellerSelling.size() > 0 && deviceBuyerSelling.size() > 0) {
                // The agents are given another chance to trade,
                // switching roles of device seller and buyer.
                makePicks();
            } else {
                switchAndTryAgain();
            }
        } else if (currentTradeAttempts < glob.DEVICE_TRADE_ATTEMPTS) {
            // The agents are given another chance to trade,
            // switching roles of device seller and buyer.
            switchAndTryAgain();
        }
    }
}

/**
 * The device buyer offers a menu of the MENU_SIZE resources that it
 * values the least.
 * \return devBuyerSelling
 */
vector<int> DevicePair::deviceBuyerAvailableRes()
{
    vector< pair <double, int> > deviceBuyerEndDay;
    pair<double, int> resUtilAndId;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (deviceBuyer->resProp[resId].getHeld() > 0) {
            resUtilAndId = make_pair(deviceBuyer->resProp[resId].endDayUtilities, resId);
            deviceBuyerEndDay.push_back(resUtilAndId);
        }
    }
    sort(deviceBuyerEndDay.begin(), deviceBuyerEndDay.end());
    vector<int> devBuyerSelling;
    int menuSize = min(glob.MENU_SIZE, (int)deviceBuyerEndDay.size());
    for (int i = 0; i < menuSize; i++) {
        devBuyerSelling.push_back(deviceBuyerEndDay[i].second);
    }
    return devBuyerSelling;
}


/**
 * The device seller offers the devices in which it has experience, the
 * necessary time for construction, and the necessary resources for
 * construction.
 * \return a vector of available devices for selling
 */
vector<int> DevicePair::deviceSellerAvailableDevices()
{
    vector<int> availableDevices;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (deviceSeller->devProp[deviceType][resId].getDeviceExperience() >= 1) {
            pair< double, vector<int> > necessaryTimeAndRes = glob.discoveredDevices[deviceType][resId]->worstCaseConstruction(*deviceSeller);
            double necessaryTime = necessaryTimeAndRes.first;
            vector<int> necessaryRes = necessaryTimeAndRes.second;
            if (find(deviceSeller->deviceBoughtThisRound.begin(),
                     deviceSeller->deviceBoughtThisRound.end(),
                     resId) == (deviceSeller->deviceBoughtThisRound).end() &&
                deviceSeller->resBundleHeldCheck(necessaryRes) &&
                deviceSeller->setAsideTime + necessaryTime < glob.DAY_LENGTH) {
                availableDevices.push_back(resId);
            }
        }
    }
    return availableDevices;
}


/**
 * The device buyer chooses the device that it values the most.
 */
void DevicePair::makePicks ()
{
    double deviceBuyerMaxUtil = 0;
    int deviceBuyerMaxUtilIndex = -1;
    for (unsigned i = 0; i < deviceSellerSelling.size(); i++) {	// Use an iterator instead
        int resId = deviceSellerSelling[i];
        // First, the agent considers the value of the device to itself; if
        // this utility value is greater than the current maximum, the max
        // gets updated.
        if (deviceBuyer->buyerDeviceValue(resId, deviceType) > deviceBuyerMaxUtil) {
            deviceBuyerMaxUtil = deviceBuyer->buyerDeviceValue(resId, deviceType);
            deviceBuyerMaxUtilIndex = resId;
        }
        // If the agent has agreed to make this device in a previous trading
        // round, the agent also considers the cost of making the device
        // itself as a potential benefit of obtaining this device through
        // purchasing it.
        if (deviceBuyer->devProp[deviceType][resId].devicesToMake > 0 &&
            glob.discoveredDevices[deviceType][resId]->costs(*deviceBuyer) > deviceBuyerMaxUtil) {
            deviceBuyerMaxUtil = glob.discoveredDevices[deviceType][resId]->costs(*deviceBuyer);
            deviceBuyerMaxUtilIndex = resId;
        }
    }
    this->deviceBuyerUtilGain = deviceBuyerMaxUtil;
    this->deviceBuyerPick = deviceBuyerMaxUtilIndex;
    // If the deviceBuyer wants one of the deviceSeller's devices, they go 
    // on to calculate the resource bundle that will be traded.
    if (deviceBuyerPick != -1) {
        deviceSellerUtilLoss = deviceSeller->sellerDeviceValue(deviceBuyerPick, deviceType);
        calcOffers();
    } else {
        // The agents are given another chance to trade, switching roles of
        // device seller and buyer. 
        switchAndTryAgain();
    }
}

/**
 * The device seller calculates the smallest bundle of resources that
 * it would accept for its device, then the device buyer calculates the
 * largest bundle it would be willing to give up.
 * Then, they split the difference using arithmetic mean based on their device trade power.
 */
void DevicePair::calcOffers()
{
    vector<int> deviceSellerOffers = calcDeviceSellerOffers();
    vector<int> deviceBuyerOffers = calcDeviceBuyerOffers();
    
    finalOffer.clear();
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        int x = (int) round((deviceSellerOffers[resId] * deviceBuyer->devTradePower
                            + deviceBuyerOffers[resId] * deviceSeller->devTradePower)
                            / (deviceBuyer->devTradePower + deviceSeller->devTradePower));
        finalOffer.push_back(x);
        if (finalOffer[resId] > deviceBuyer->resHeld(resId)) {
            finalOffer[resId] = deviceBuyer->resHeld(resId);
        }
    }
    checkGains();
}


/**
 * From the menu of resources, the device seller picks resources (one unit
 * at a time) that it values the most until the utility gain from the
 * resources overcomes the utility lost through making the device
 * (i.e. the reservation price). Thus, the seller is creating the
 * smallest possible bundle of resources that it would accept in
 * exchange for the device it is selling.
 * \return deviceSellerOffers
 */
vector<int> DevicePair::calcDeviceSellerOffers()
{
    /*
     * deviceSellerMarginalUtilities will be a list where each element is
     * 0 unless the resource with the corresponding resId is begin offered
     * by the device buyer; in that case, the element is equal to the MU for
     * that resource.
     */
    vector<double> deviceSellerMarginalUtilities;
    /*
     * deviceSellerOffers will be a list where each element is the number of
     * units demanded by the device seller in the resource with the
     * corresponding resId.
     */
    vector<int> deviceSellerOffers;
    /* The maximum available MU. */
    double deviceSellerMUMax = 0.0;
    /* The id of the resource with the highest available MU. */
    int deviceSellerMUMaxId = -1;

    double MU = 0.0;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
	// Test if resId is in deviceBuyerSelling        
        if (find(deviceBuyerSelling.begin(), deviceBuyerSelling.end(), resId)
            != deviceBuyerSelling.end()) {
            MU = deviceSeller->utilCalc(resId);
        } else {
            MU = 0.0;
        }
        deviceSellerMarginalUtilities.push_back(MU);
        if (MU > deviceSellerMUMax) {
            deviceSellerMUMax = MU;
            deviceSellerMUMaxId = resId;
        }
        deviceSellerOffers.push_back(0);
    }
    
    /* One unit at a time, the device seller adds units to its bundle until
     * the utility gain from that bundle exceeds the utility loss from
     * selling the device. */
    double deviceSellerUtilGain = 0.0;
    while (deviceSellerUtilGain < deviceSellerUtilLoss && deviceSellerMUMax > 0) {
        deviceSellerUtilGain += deviceSeller -> tempUtilCalc(deviceSellerMUMaxId, deviceSellerOffers[deviceSellerMUMaxId]);
        deviceSellerOffers[deviceSellerMUMaxId]++;
        if (deviceBuyer->resHeld(deviceSellerMUMaxId) > deviceSellerOffers[deviceSellerMUMaxId]) {
            MU = deviceSeller->tempUtilCalc(deviceSellerMUMaxId, deviceSellerOffers[deviceSellerMUMaxId]);
        } else {
            MU = 0.0;
        }
        deviceSellerMarginalUtilities[deviceSellerMUMaxId] = MU;
        vector<double>::iterator maxPointer =
                        max_element(deviceSellerMarginalUtilities.begin(), deviceSellerMarginalUtilities.end());
        deviceSellerMUMax = *maxPointer;
        deviceSellerMUMaxId = maxPointer - deviceSellerMarginalUtilities.begin();
    }

    return deviceSellerOffers;
}

/**
 * From the menu of resources, the device buyer picks resources (one unit
 * at a time) that it values the least until the utility gain from the
 * resources is just below the utility gained from receiving the device
 * (i.e. the reservation price). Thus, the buyer is creating the largest
 * possible bundle of resources that it would be willing to give away in
 * exchange for the device it is buying.
 * \return deviceBuyerOffers
 */
vector<int> DevicePair::calcDeviceBuyerOffers()
{
    double max_double = numeric_limits<double>::max();

    // deviceBuyerMarginalUtilities will be a list where each element is 0 
    // unless the resource with the corresponding resId is begin offered by 
    // the device buyer; in that case, the element is equal to the MU for
    // that resource.
    vector<double> deviceBuyerMarginalUtilities;
    // deviceBuyerOffers will be a list where each element is the number of
    // units offered by the device seller in the resource with the
    // corresponding resId.
    vector<int> deviceBuyerOffers;
    double deviceBuyerMUMin = max_double;
    int deviceBuyerMUMinId = -1;
    double MU = 0.0;

    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
	// Test if resId is in deviceBuyerSelling
        if (find(deviceBuyerSelling.begin(), deviceBuyerSelling.end(), resId)
            != deviceBuyerSelling.end()) {
            MU = deviceBuyer->tempUtilCalc(resId, -1);
        } else {
            MU = max_double;
        }
        deviceBuyerMarginalUtilities.push_back(MU);
        if (MU < deviceBuyerMUMin) {
            deviceBuyerMUMin = MU;
            deviceBuyerMUMinId = resId;
        }
        deviceBuyerOffers.push_back(0);
    }

    /*
     * One unit at a time, the device buyer adds units to its bundle until
     * the utility gain from that bundle exceeds the utility gain from
     * buying the device.
     */
    double deviceBuyerUtilLoss = 0.0;
    while (deviceBuyerUtilLoss < deviceBuyerUtilGain &&
           deviceBuyerMUMin < max_double) {
        deviceBuyerOffers[deviceBuyerMUMinId]++;
        deviceBuyerUtilLoss += deviceBuyer->tempUtilCalc(deviceBuyerMUMinId,
                                        - deviceBuyerOffers[deviceBuyerMUMinId]);
        if (deviceBuyer->resHeld(deviceBuyerMUMinId) > deviceBuyerOffers[deviceBuyerMUMinId]) {
            MU = deviceBuyer->tempUtilCalc(deviceBuyerMUMinId, -deviceBuyerOffers[deviceBuyerMUMinId] - 1);
        } else {
            MU = max_double;
        }
        deviceBuyerMarginalUtilities[deviceBuyerMUMinId] = MU;
        vector<double>::iterator minPointer =
            min_element(deviceBuyerMarginalUtilities.begin(), deviceBuyerMarginalUtilities.end());
        deviceBuyerMUMin = *minPointer;
        deviceBuyerMUMinId = minPointer - deviceBuyerMarginalUtilities.begin();
    }
    // The device buyer removes the least valuable unit from it's bundle so
    // that the utility of the offered bundle is now as close to the utility
    // of the device as possible without exceeding the value of the device.
    deviceBuyerOffers[deviceBuyerMUMinId]--;
    
    return deviceBuyerOffers;
}

/**
 * If both agents gain from the potential trade, they trade.
 */
void DevicePair::checkGains()
{
    deviceBuyerUtilLoss = deviceBuyer->costOfResourceBundle(finalOffer);
    deviceSellerUtilGain = deviceSeller->gainOfResourceBundle(finalOffer);
    if (deviceBuyerUtilLoss < deviceBuyerUtilGain &&
        deviceSellerUtilLoss < deviceSellerUtilGain) {
        makeTrade();
    } else {
        switchAndTryAgain();
    }
}


/**
 * The device seller sells the device, and the device buyer gives up
 * the agreed resources.
 */
void DevicePair::makeTrade()
{
    deviceSeller->sellsDevice(deviceBuyerPick, deviceType);
    deviceBuyer->buysDevice(deviceBuyerPick, deviceType);
    
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (finalOffer[resId] > 0) {
            deviceBuyer->sells(resId, finalOffer[resId]);
            deviceSeller->buys(resId, finalOffer[resId]);

            // set the data for saving end day results
            deviceBuyer->resProp[resId].soldEndDay += finalOffer[resId];
            deviceSeller->resProp[resId].boughtEndDay += finalOffer[resId];
        }
        LOG(4) << "agent " << deviceBuyer->name << " gave up " <<
            finalOffer[resId] << " of resource " << resId;
    }
    LOG(4) << "agent " << deviceSeller->name << " gave up " <<
        deviceType << " for " << deviceBuyerPick;
    LOG(4) << "Just now, agent " << deviceSeller ->name <<" gained " <<
        deviceSellerUtilGain << " and lost " << deviceSellerUtilLoss;
    LOG(4) << "Furthermore, agent " << deviceBuyer->name << " gained " <<
        deviceBuyerUtilGain << " and lost " << deviceBuyerUtilLoss;

    agentMemoryUpdate();
    tradeStatsUpdate();
    /* The agents are given another chance to trade. */
    currentTradeAttempts++;	
    pairDeviceTrade();
}



/**
 * The agents get another trade attempt -- switching roles of buyer
 * and seller.
 */
void DevicePair::switchAndTryAgain()
{
    currentTradeAttempts++;
    if (deviceBuyer == agentA) {
        deviceBuyer = agentB;
        deviceSeller = agentA;
    } else {
        deviceBuyer = agentA;
        deviceSeller = agentB;
    }
    pairDeviceTrade();
}


/**
 * Agents remember the devices they've bought and the surplus
 * from the trades.
 */
void DevicePair::agentMemoryUpdate()
{
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        deviceBuyer->resProp[resId].endDayUtilities = deviceBuyer->utilCalc(resId);
        deviceSeller->resProp[resId].endDayUtilities = deviceSeller->utilCalc(resId);
    }
    deviceSeller->agentDeviceTradeMemory[deviceType][deviceBuyer->name][0]
        += deviceSellerUtilGain - deviceSellerUtilLoss;
    deviceBuyer->agentDeviceTradeMemory[deviceType][deviceSeller->name][0]
        += deviceBuyerUtilGain - deviceBuyerUtilLoss;
    deviceBuyer->devProp[deviceType][deviceBuyerPick].devicePrices.pop_back();
    vector<double>::iterator it = deviceBuyer->devProp[deviceType][deviceBuyerPick].devicePrices.begin();
    deviceBuyer->devProp[deviceType][deviceBuyerPick].devicePrices.insert(it, deviceBuyerUtilLoss);
    deviceBuyer->deviceBoughtThisRound.push_back(deviceBuyerPick);
    deviceSeller->devProp[deviceType][deviceBuyerPick].devicePrices.pop_back();
    it = deviceSeller->devProp[deviceType][deviceBuyerPick].devicePrices.begin();
    deviceSeller->devProp[deviceType][deviceBuyerPick].devicePrices.insert(it, -deviceSellerUtilGain + deviceSellerUtilLoss);
}


/**
 * Agent stats are updated to be saved by the statsTracker classes.
 */
void DevicePair::tradeStatsUpdate()
{
    deviceBuyer->unitsSoldForDevicesToday += accumulate(finalOffer.begin(), finalOffer.end(), 0);
    if (deviceBuyer->group != deviceSeller->group) {
        deviceBuyer->unitsSoldCrossGroupForDevicesToday += accumulate(finalOffer.begin(), finalOffer.end(), 0);;
    }
    deviceBuyer->devProp[deviceType][deviceBuyerPick].devicesBoughtTotal++;
    deviceSeller->devProp[deviceType][deviceBuyerPick].devicesSoldTotal++;
}
