/**
 * Utils check what phase (trading, making device, etc) the agents are currently in
 * and call the corresponding functions to execute the phase.
 */
#include <vector>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <cassert>
#include <stdlib.h>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "globals.h"
#include "utils.h"
#include "agent.h"
#include "device.h"
#include "marketplace.h"
#include "statstracker.h"
#include "devmarketplace.h"
#include "logging.h"

using namespace std;

/**
 * Print the number of agents, number of days, nunber of resources of this run.
 */
void Utils::printStartConditions()
{
    cout << "Num days: " << glob.NUM_DAYS << endl;
	cout << "Num agents: " << glob.NUM_AGENTS << endl;
    cout << "Num resources: " << glob.NUM_RESOURCES << endl;
}

/**
 * Let the agents start working.
 */
void Utils::agentsWork()
{
    LOG(2) << "Entering agents work phase";
    BOOST_FOREACH(Agent *agent, glob.agent) {
        if (agent->inSimulation) {
            agent->workDay();
        }
    }
}

/**
 * Let the agents start trading resources.
 */
void Utils::agentsTrade()
{
    if (glob.TRADE_EXISTS) {
        LOG(2) << "Entering agents trade phase";
        glob.resourceMarket->tradeResources();
    } else {
        LOG(2) << "Skipping disabled agents trade phase";
    }
}

/**
 * Used below for shuffling the list.
 */
static int my_random_int_from_0(int upper)
{
    return glob.random_int(0, upper);
}

/**
 * Agents are selected in random order to invent devices.
 */
void Utils::agentsInvent()
{
    if (! glob.DEVICES_EXIST) {
        LOG(2) << "Skipping agents invent phase, DEVICES_EXIST being disabled";
        return;
    }
    LOG(2) << "Entering agents invent phase";
    vector<Agent *> inventors;
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        inventors.push_back(glob.agent[aId]);
        if (glob.agent[aId]->inSimulation) {
            glob.agent[aId]->updateDeviceComponentExperience();
        }
    }
    // shuffle the inventors list, and then call toolInvention() on each.
#ifndef DONT_RANDOMIZE
    random_shuffle(inventors.begin(), inventors.end(), my_random_int_from_0);
#endif
    BOOST_FOREACH(Agent *agent, inventors) {
        if (agent->inSimulation) {
            agent->toolInvention();
        }
    }
}

/**
 * At the end of each day, update the status.
 */
void Utils::endDay()
{
    LOG(2) << "Entering end day checks phase";
    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
        if (glob.agent[aId]->inSimulation) {
            glob.agent[aId]->endDayChecks();

            glob.agent[aId]->logAgentData();
            //if (aId == 0) {
            //    saveEndDayData();
            //}
        }
    }
}

/**
 * At the end of each day, unused resources and devices will decay.
 */
void Utils::endDayDecay()
{
    LOG(2) << "Entering day decay phase";
    BOOST_FOREACH(Agent *agent, glob.agent) {
        if (agent->inSimulation) {
            agent->decay();
        }
    }

    BOOST_FOREACH(Agent *agent, glob.agent) {
        assert(agent->calcMinHeld() >= 0);
    }
}


/**
 * From most complex to least complex, agents trade devices, then decide
 * to make devices for themselves, then trade, and so on
 */
void Utils::agentsTradeDevices()
{
    /* Shouldn't trade when TRADE_EXISTS is turned off. */
    if (glob.TRADE_EXISTS) {
        LOG(2) << "Entering trade devices phase";
        BOOST_FOREACH(Agent *agent, glob.agent) {
            if (agent->overtime < glob.DAY_LENGTH) {
                agent->utilGainThroughDevSoldToday = 0.0;
            }
        }
        if (! glob.DEVICES_EXIST) {
            return;
        }
        glob.resourceMarket->tradeResources();
        device_name_t devarr[] = { INDUSTRY, FACTORY, DEVFACTORY, MACHINE, DEVMACHINE, TOOL };
        for (int i = 0; i < 6; i++) {  // 6 is length of array above.
            device_name_t types = devarr[i];

            glob.deviceMarket->tradeDevices(types);

            BOOST_FOREACH(Agent *agent, glob.agent) {
                if (agent->inSimulation) {
                    agent->personalDevices(types);
                }
            }
            glob.resourceMarket->tradeResources();
        }
    } else {
        LOG(2) << "Skipping disabled agents trade phase";
    }
}

/**
 * Let agents start producing some of the devices they already invented.
 */
void Utils::agentsProduceDevices()
{
    LOG(2) << "Entering produce devices phase";
    if (! glob.DEVICES_EXIST) {
        return;
    }
    BOOST_FOREACH(Agent *agent, glob.agent) {
        if (agent->inSimulation) {
            agent->deviceProduction();
        }
    }
}

/*
 * Check if any device has been made / exists.
 */
bool Utils::deviceExists(device_name_t devType)
{
    bool exist = false;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (glob.discoveredDevices[devType][resId] != NULL &&
            glob.discoveredDevices[devType][resId]->agentsKnown() != 0) {
            exist = true;
            break;
        }
    }
    return exist;
}


/**
 * Print the total number of utils and units of resources.
 */
void Utils::printSumUtilAndRes()
{
    cout << "SumUtil:" << glob.otherStats->getSumUtilBack() << endl;
    cout << "SumRes:" << glob.otherStats->getSumResBack() << endl;
}

/**
 * At the end of each day, compile the stats of the day
 * \param dayNumber the current day in the simulation
 */
void Utils::dayAnalysis(int dayNumber)
{
    LOG(2) << "Entering day analysis phase";
    BOOST_FOREACH(Agent *agent, glob.agent) {
        agent->calcUtilityToday();
    }

    // Update each statistics module in turn
    glob.tradeStats->dailyUpdate();
    glob.productionStats->dailyUpdate();
    glob.otherStats->dailyUpdate();
    glob.otherStats->getSumResByAgent();

    BOOST_FOREACH(Agent *agent, glob.agent) {
        agent->resetTodayStats();
    }
}

/**
 * Decide if to save the current day status, to remove any resources or agents.
 */
void Utils::removeOrSave(int dayNumber)
{
    LOG(2) << "Entering remove or save phase";
    if (glob.SAVE_DAY_STATUS) {
        if (dayNumber == glob.DAY_FOR_SAVE) {
            saveDayStatus(dayNumber);
        }
    }
    if (glob.REMOVE_RES) {
        if (dayNumber == glob.REMOVE_RES_DAY) {
            glob.removeRes(glob.RES_TO_REMOVE, dayNumber);
        }
    }
    if (glob.REMOVE_AGENT) {
        if (dayNumber == glob.REMOVE_AGENT_DAY) {
            glob.removeAgent(glob.AGENT_TO_REMOVE, dayNumber);
        }
    }
}


/**
 * Call saveResults at the end of each simulation to save the results.
 */
void Utils::endSim()
{
    printStartConditions();
    printSumUtilAndRes();
    if (glob.SAVE_DAY_STATUS) {
        cout << "Your status on day " << glob.DAY_FOR_SAVE << " is saved in " <<
            glob.DAY_STATUS_SAVE_FOLDER << endl;
    }
    if (glob.END_SAVE) {
        saveResults();
        cout << "Your results have been saved in " << glob.SAVE_FOLDER << endl;
        cout << "The simulation was named " << glob.SIM_NAME << endl;
    } else {
        // TODO: we are not doing this in C++...  remove this else part?
        /*
        while (options.graph):
            ag.showGraph()
            command = raw_input('Enter c to continue (anything else to quit):')
            if command.lower() == 'c':
                ag.showGraph()
            else:
                break
                    }
        */
    }
}

/**
 * Used to sort a vector of vectors. Only sort on the first item.
 */
bool mySort(const vector<double>& i, const vector<double>& j)
{
    return i[0] < j[0];
}


/**
 * Compute the five quartiles of elements of each index position.
 * \param data a vector of a vector of double
 * \return five vectors of double with min, 25%, median, 75%, max.
 */
vector<vector<double> > Utils::calcQuartiles(vector<vector<double> > data)
{
    vector<double> min;     // a vector of each index's minimum value of all sub-vectors in data
    vector<double> med;     // a vector of each index's median value of all sub-vectors in data
    vector<double> max;     // a vector of each index's maximum value of all sub-vectors in data
    vector<vector<double> > copy = data;    // make a copy so the original data will not be changed

    bool even;
    if (copy.size() % 2 == 0) {
        even = true;
    } else {
        even = false;
    }

    /*
     * Find the min, median, and max.
     */
    for ( unsigned int m = 0; m < copy[0].size(); m++) {
        if ( m != 0 ) {
            for (unsigned int i = 0; i < copy.size(); i++) {
                swap(copy[i][m], copy[i][0]);
            }
        }
        sort(copy.begin(), copy.end(), mySort);
        min.push_back(copy[0][0]);
        if (even) {
            med.push_back( (copy[copy.size()/2-1][0] + copy[copy.size()/2][0]) / 2.0 );
        } else {
            med.push_back(copy[copy.size() / 2][0]);
        }
        max.push_back(copy[copy.size()-1][0]);
    }

    /*
     * Find the first and third quartile.
     */
    vector<double> qOne;
    vector<double> qThree;
    vector<double> aboveMedian;
    vector<double> belowMedian;
    for (int dayNum = 0; dayNum < glob.NUM_DAYS; dayNum++) {
        aboveMedian.clear();
        belowMedian.clear();
        for (unsigned int vecId = 0; vecId < data.size(); vecId++) {
            if (data[vecId][dayNum] >= med[dayNum]) {
                aboveMedian.push_back(data[vecId][dayNum]);
            }
            if (data[vecId][dayNum] <= med[dayNum]) {
                belowMedian.push_back(data[vecId][dayNum]);
            }
        }

        sort(belowMedian.begin(), belowMedian.end());
        if (belowMedian.size() % 2 == 0) {
            qOne.push_back( (belowMedian[belowMedian.size()/2-1] + belowMedian[belowMedian.size()/2]) / 2.0 );
        } else {
            qOne.push_back( belowMedian[belowMedian.size() / 2] );
        }
        sort(aboveMedian.begin(), aboveMedian.end());
        if (aboveMedian.size() % 2 == 0) {
            qThree.push_back( (aboveMedian[aboveMedian.size()/2-1] + aboveMedian[aboveMedian.size()/2]) / 2.0 );
        } else {
            qThree.push_back( aboveMedian[aboveMedian.size() / 2] );
        }
    }

    vector<vector<double> > result;
    result.push_back(min);
    result.push_back(qOne);
    result.push_back(med);
    result.push_back(qThree);
    result.push_back(max);

    copy.clear();

    return result;
}

/**
 * Print the first line of a csv file with day numbers.
 */
void Utils::headerByDay(ofstream &file, string filePath)
{
    if ( !boost::filesystem::exists(filePath)) {
        file.open(filePath.c_str());
        file << ",";
        for (int i = 0; i < glob.NUM_DAYS; i++) {
            file << (i + 1) << ",";
        }
        file << "\n";
        file.close();
    }
}

/**
 * BRH 2013.05.23: This graph shows the gini coefficient.  The formula used is can be found at
 * http://en.wikipedia.org/wiki/Gini_coefficient.  The python adaption was corrected to the one in
 * http://econpy.googlecode.com/svn/trunk/pytrix/utilities.py
 *
 * First and last day don't have a gini, so leave as blank.
 */
void Utils::saveGini()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/gini.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    /*
     * Save the gini of all agents into the same file.
     */
    file << "giniPerAgent_"<< glob.SIM_NAME << ",,";   // Leave the first day blank
    vector<double> orderedUtils;
    vector<double> y;
    vector<vector<double> > sumUtilByAgent = glob.otherStats->getSumUtilByAgent();
    for (int dayNum = 1; dayNum < glob.NUM_DAYS-1; dayNum++) {//for every day...
        orderedUtils.clear();
        for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {//for every agent
            if (glob.agent[aId]->inSimulation) {
                // takes a four-day average (one before, two after) of utility to calculate Gini coefficient
                orderedUtils.push_back(accumulate(sumUtilByAgent[aId].begin() + dayNum - 1,
                                                sumUtilByAgent[aId].begin() + dayNum + 2, 0.0));// create vector of utilities per agent
            }
        }
        sort(orderedUtils.begin(), orderedUtils.end());//orders utilities smallest to largest
        y.clear();
        for (unsigned int i = 0; i < orderedUtils.size(); i++) {
            y.push_back(accumulate(orderedUtils.begin(), orderedUtils.begin() + i + 1, 0));// seems to be sum of utilities up to a point, cumulative histogram type thing.
        }
        double B = accumulate(y.begin(), y.end(), 0.0) / (y[y.size()-1] * double(orderedUtils.size())); // arbitrary variable used in final calc.
        file << (1.0 + (1.0 / double(orderedUtils.size())) - (2.0 * B)) << ",";// final calculation.
    }
    file << ",\n";      // leave the last day blank

    /*
     * Save the gini of each group into the same file.
     */
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
	file << "giniByGoup " << gId << ",,";
        for (int dayNum = 1; dayNum < glob.NUM_DAYS-1; dayNum++) {
            orderedUtils.clear();
            for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
                if (glob.agent[aId]->group == gId) {
                    if (glob.agent[aId]->inSimulation) {
                        orderedUtils.push_back(accumulate(sumUtilByAgent[aId].begin() + dayNum - 1,
                                                        sumUtilByAgent[aId].begin() + dayNum + 2, 0.0));
                    }
                }
            }
            sort(orderedUtils.begin(), orderedUtils.end());
            y.clear();
            for (unsigned int i = 0; i < orderedUtils.size(); i++) {
                y.push_back(accumulate(orderedUtils.begin(), orderedUtils.begin() + i + 1, 0));
            }
            double B = accumulate(y.begin(), y.end(), 0.0) / (y[y.size()-1] * double(orderedUtils.size()));
            file << (1.0 + (1.0 / double(orderedUtils.size())) - (2.0 * B)) << ",";
        }
        file << ",\n";      // leave the last day blank
    }
    file << "\n";
    file.close();

    orderedUtils.clear();
    y.clear();
    sumUtilByAgent.clear();
}

/**
 * Save the min, first quartile, median, third quartile and max of HHIs into csv files.
 */
void Utils::saveHHIQuartiles()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/HHI.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    string quartileNames[] = { "min", "q1", "median", "q3", "max" };
    vector<vector<int> > resGatheredByRes = glob.productionStats->getResGatheredByRes();
    vector<vector<vector<int> > > resGatheredByResByAgent = glob.productionStats->getResGatheredByResByAgent();

    /*
     * Save HHI for all agents
     */
    vector<vector<double> > data;
    vector<double> HHIpoints;
    double HHI = 0;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        HHIpoints.clear();
        for (int dayNum = 0; dayNum < glob.NUM_DAYS; dayNum++) {
            HHI = 0;
            if (resGatheredByRes[resId][dayNum] > 0) {
                for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
                    HHI += pow((double(resGatheredByResByAgent[resId][aId][dayNum]) / double(resGatheredByRes[resId][dayNum])), 2);
                }
            }
            HHIpoints.push_back(HHI);
        }
        data.push_back(HHIpoints);
    }
    vector<vector<double> > q = calcQuartiles(data);
    for (unsigned int eachQuartile = 0; eachQuartile < q.size(); eachQuartile++) {
        file << "HHI_" << quartileNames[eachQuartile] << "_ByAgents" << glob.SIM_NAME << ",";
        for (int dayNum = 0; dayNum < glob.NUM_DAYS; dayNum++) {
            file << q[eachQuartile][dayNum] << ",";
        }
        file << "\n";
    }

    /*
     * Save HHI for each group's agents.
     */
    vector<vector<vector<double> > > dataGroup;
    data.clear();
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
            HHIpoints.clear();
            for (int dayNum = 0; dayNum < glob.NUM_DAYS; dayNum++) {
                HHI = 0;
                if (resGatheredByRes[resId][dayNum] > 0) {
                    for (int aId = 0; aId < glob.NUM_AGENTS; aId++) {
                        if (glob.agent[aId]->group == gId) {
                            HHI += pow((double(resGatheredByResByAgent[resId][aId][dayNum]) / double(resGatheredByRes[resId][dayNum])), 2);
                        }
                    }
                }
                HHIpoints.push_back(HHI);
            }
            data.push_back(HHIpoints);

        }
        dataGroup.push_back(data);
    }
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        q = calcQuartiles(dataGroup[gId]);

        for (unsigned int eachQuartile = 0; eachQuartile < q.size(); eachQuartile++) {
            file << "HHI_" << quartileNames[eachQuartile] << "_ByGroup_" << gId << glob.SIM_NAME << ",";
            for (int dayNum = 0; dayNum < glob.NUM_DAYS; dayNum++) {
                file << q[eachQuartile][dayNum] << ",";
            }
            file << "\n";
        }
    }
    file << "\n";
    file.close();

    resGatheredByRes.clear();
    resGatheredByResByAgent.clear();
    data.clear();
    HHIpoints.clear();
    dataGroup.clear();
}

/**
 * Save the total utility for each day into a csv file.
 */
void Utils::saveTotalUtility()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/totalUtility.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    file << "sumUtil,";
    vector<double> sumUtil = glob.otherStats->getSumUtil();
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << (sumUtil[i]) << ",";
    }
    file.close();

    sumUtil.clear();
}

/**
 * Save the mean utility by all agents and by each group of each day into a csv file.
 */
void Utils::saveMeanUtility()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/meanUtility.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    /* mean utility per agent */
    file << "meanUtilPerAgent,";
    vector<double> sumUtil = glob.otherStats->getSumUtil();
    vector<int> activeAgents = glob.otherStats->getActiveAgents();
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << (sumUtil[i] / activeAgents[i]) << ",";
    }
    file << "\n";

    /* mean utility per group */
    vector<vector<double> > sumUtilByGroup = glob.otherStats->getSumUtilByGroup();
    vector<vector<int> > activeGroupAgents = glob.otherStats->getActiveGroupAgents();
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        file << "meanUtilByGroup_" << i << "_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double)sumUtilByGroup[i][j] / (double)activeGroupAgents[i][j]) << ",";
        }
        file << "\n";
    }
    file << "\n";
    file.close();

    sumUtil.clear();
    activeAgents.clear();
    activeGroupAgents.clear();
    sumUtilByGroup.clear();
}

/**
 * Save the mean units gathered by all agents and by each group of each day into a csv file.
 */
void Utils::saveUnitsGathered()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/unitsGathered.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    vector<int> resGath = glob.productionStats->getResGathered();
    vector<int> activeAgents = glob.otherStats->getActiveAgents();

    /* unitsGathered per agent */
    file << "unitsGatheredPerActiveAgent_" << glob.SIM_NAME << ",";
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << ( (double) resGath[i] / (double) activeAgents[i] ) << ",";
    }
    file << "\n";

    /* unitsGathered per group */
    vector<vector<int> > resGatheredByGroup = glob.productionStats->getResGatheredByGroup();
    vector<vector<int> > activeGroupAgents = glob.otherStats->getActiveGroupAgents();
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        file << "unitsGatheredByGroup_" << i << "_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double)resGatheredByGroup[i][j] / (double)activeGroupAgents[i][j]) << ",";
        }
        file << "\n";
    }
    file << "\n";
    file.close();

    resGath.clear();
    activeAgents.clear();
    activeGroupAgents.clear();
    resGatheredByGroup.clear();
}

/**
 * Save the mean units held by all agents and by each group of each day into a csv file.
 */
void Utils::saveUnitsHeld()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/unitsHeld.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    /* unitsHeld per agent */
    file << "unitsHeldPerActiveAgent_" << glob.SIM_NAME << ",";
    vector<int> sumRes = glob.otherStats->getSumRes();
    vector<int> activeAgents = glob.otherStats->getActiveAgents();
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << ( (double) sumRes[i] / (double) activeAgents[i] ) << ",";
    }
    file << "\n";

    /* unitsHeld per group */
    vector<vector<int> > resHeldByGroup = glob.otherStats->getSumResByGroup();
    vector<vector<int> > activeGroupAgents = glob.otherStats->getActiveGroupAgents();
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        file << "unitsHeldByGroup_" << i << "_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double)resHeldByGroup[i][j] / (double)activeGroupAgents[i][j]) << ",";
        }
        file << "\n";
    }
    file << "\n";
    file.close();

    sumRes.clear();
    activeAgents.clear();
    activeGroupAgents.clear(); 
    resHeldByGroup.clear();
}

/**
 * Save the number of all resources traded for each device and for resources 
 *     per agent, per group, across group, and within each group into a csv file.
 */
void Utils::saveUnitsTraded()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/unitsTraded.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    vector<int> activeAgents = glob.otherStats->getActiveAgents();

    /* unitsTradedForDevice per agent */
    vector<int> resTradeForDeviceVolume = glob.tradeStats->getResTradeForDeviceVolume();
    file << "unitsTradedForDevicePerActiveAgent_" << glob.SIM_NAME << ",";
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << ( (double) resTradeForDeviceVolume[i] / (double) activeAgents[i] ) << ",";
    }
    file << "\n";

    /* unitsTradedForDevice per group */
    vector<vector<int> > resTradedForDeviceVolumeByGroup = glob.tradeStats->getResTradeForDeviceVolumeByGroup();
    vector<vector<int> > activeGroupAgents = glob.otherStats->getActiveGroupAgents();
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        file << "unitsTradedForDeviceByGroup_" << i << "_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double)resTradedForDeviceVolumeByGroup[i][j] / (double)activeGroupAgents[i][j]) << ",";
        }
        file << "\n";
    }

    /* unitsTradedForDevice in total when trading agents are from different groups */
    vector<int> resTradeForDeviceVolumeCrossGroup = glob.tradeStats->getResTradeForDeviceVolumeCrossGroup();
    file << "unitsTradedForDeviceCrossGroup_" << glob.SIM_NAME << ",";
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << ( (double) resTradeForDeviceVolumeCrossGroup[i] / (double) activeAgents[i] ) << ",";
    }
    file << "\n";

    /* unitsTradedForDevice when trading agents are from the same groups */
    vector<vector<int> > resTradedForDeviceVolumeWithinGroup = glob.tradeStats->getResTradeForDeviceVolumeWithinGroup();
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        file << "unitsTradedForDeviceWithinGroup " << i << "_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double)resTradedForDeviceVolumeWithinGroup[i][j] / (double)activeGroupAgents[i][j]) << ",";
        }
        file << "\n";
    }

    /* unitsTradedForRes per agent */
    file << "unitsTradedForResPerActiveAgent_" << glob.SIM_NAME << ",";
    vector<int> resTradeVolume = glob.tradeStats->getResTradeVolume();
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << ( (double) resTradeVolume[i] / (double) activeAgents[i] ) << ",";
    }
    file << "\n";

    /* unitsTradedForRes per group */
    vector<vector<int> > resTradeVolumeByGroup = glob.tradeStats->getResTradeVolumeByGroup();
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        file << "unitsTradedForResByGroup_" << i << "_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double)resTradeVolumeByGroup[i][j] / (double)activeGroupAgents[i][j]) << ",";
        }
        file << "\n";
    }

    /* unitsTradedForRes when trading agents are from different groups */
    file << "unitsTradedForResCrossGroup_" << glob.SIM_NAME << ",";
    vector<int> resTradeVolumeCrossGroup = glob.tradeStats->getResTradeVolumeCrossGroup();
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << ( (double) resTradeVolumeCrossGroup[i] / (double) activeAgents[i] ) << ",";
    }
    file << "\n";

    /* unitsTradedForRes when trading agents are from the same group */
    vector<vector<int> > resTradeVolumeWithinGroup = glob.tradeStats->getResTradeVolumeWithinGroup();
    for (int i = 0; i < glob.NUM_AGENT_GROUPS; i++) {
        file << "unitsTradedForResWithinGroup " << i << "_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double)resTradeVolumeWithinGroup[i][j] / (double)activeGroupAgents[i][j]) << ",";
        }
        file << "\n";
    }

    file << "\n";
    file.close();

    activeAgents.clear();
    activeGroupAgents.clear(); 
    resTradeForDeviceVolume.clear();
    resTradedForDeviceVolumeByGroup.clear();
    resTradeForDeviceVolumeCrossGroup.clear();
    resTradeVolume.clear();
    resTradeVolumeByGroup.clear();
    resTradeVolumeCrossGroup.clear();
}

/**
 * Save the number of made devices for each device type per agent and per group.
 */
void Utils::saveDeviceMade()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/deviceMade.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    vector<int> activeAgents = glob.otherStats->getActiveAgents();
    string devicesStr[] = { "TOOL", "MACHINE", "FACTORY", "INDUSTRY", "DEVMACHINE", "DEVFACTORY" };

    /* device made per agent */
    vector< vector<int> > devicesMade = glob.productionStats->getDevicesMade();
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        file << devicesStr[i] << "MadePerActiveAgent_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double) devicesMade[i][j] / (double) activeAgents[j] ) << ",";
        }
        file << "\n";
    }

    /* device made per goup */
    vector< vector< vector <int> > > devicesMadeByGroup = glob.productionStats->getDevicesMadeByGroup();
    vector<vector<int> > activeGroupAgents = glob.otherStats->getActiveGroupAgents();
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
            file << devicesStr[i] << "MadeByGroup" << gId << "_" << glob.SIM_NAME << ",";
            for (int j = 0; j < glob.NUM_DAYS; j++) {
                file << ( (double) devicesMadeByGroup[gId][i][j] / (double) activeGroupAgents[gId][j] ) << ",";
            }
            file << "\n";
        }
    }
    file << "\n";
    file.close();

    activeAgents.clear();
    activeGroupAgents.clear(); 
    devicesMade.clear();
    devicesMadeByGroup.clear();
}


/**
 * Save the number of minutes used to gather resources with each type of device per agent and per group,
 *     and the number of minutes used to make devices per agent and per group,
 *     and the number of minutes used to gather resources without devices per agent and per group.
 */
void Utils::saveTotalTimeUsage()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/totalTimeUsage.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    vector<int> activeAgents = glob.otherStats->getActiveAgents();
    string devicesStr[] = { "TOOL", "MACHINE", "FACTORY", "INDUSTRY", "DEVMACHINE", "DEVFACTORY" };

    /* timeSpentGatheringWithDevice per agent */
    vector< vector<int> > timeSpentGatheringWithDevice = glob.productionStats->getTimeSpentGatheringWithDevice();
    for (int i = 0; i < NUM_DEVICE_TYPES - 2; i++) {
        file << devicesStr[i] << "_timeGatheringWithDevicePerActiveAgent_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double) timeSpentGatheringWithDevice[i][j] / (double) activeAgents[j] ) << ",";
        }
        file << "\n";
    }

    /* timeSpentGatheringWithDevice per group */
    vector< vector<vector<int> > > timeSpentGatheringWithDeviceByGroup = glob.productionStats->getTimeSpentGatheringWithDeviceByGroup();
    vector<vector<int> > activeGroupAgents = glob.otherStats->getActiveGroupAgents();
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int i = 0; i < NUM_DEVICE_TYPES - 2; i++) {
            file << devicesStr[i] << "_timeGatheringWithDeviceByGroup " << gId << "_" << glob.SIM_NAME << ",";
            for (int j = 0; j < glob.NUM_DAYS; j++) {
                file << ( (double) timeSpentGatheringWithDeviceByGroup[gId][i][j] / (double) activeGroupAgents[gId][j] ) << ",";
            }
            file << "\n";
        }
    }
    file << "\n";       // add an extra line so more readable.

    /* timeMakingDevices per agent */
    vector< vector<int> > timeSpentMakingDevices = glob.productionStats->getTimeSpentMakingDevices();
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        file << devicesStr[i] << " timeMakingDevicesPerActiveAgent_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << ( (double) timeSpentMakingDevices[i][j] / (double) activeAgents[j] ) << ",";
        }
        file << "\n";
    }

    /* timeMakingDevices per group */
    vector< vector<vector<int> > > timeSpentMakingDevicesByGroup = glob.productionStats->getTimeSpentMakingDevicesByGroup();
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
            file << devicesStr[i] << " timeMakingDevicesByGroup " << gId << "_" << glob.SIM_NAME << ",";
            for (int j = 0; j < glob.NUM_DAYS; j++) {
                file << ( (double) timeSpentMakingDevicesByGroup[gId][i][j] / (double) activeGroupAgents[gId][j] ) << ",";
            }
            file << "\n";
        }
    }
    file << "\n";       // add an extra line so more readable.

    /* timeGatheringWithoutDevice per agent */
    file << "timeGatheringWithoutDevicePerActiveAgent_" << glob.SIM_NAME << ",";
    vector<int> timeSpentGatheringWithoutDevice = glob.productionStats->getTimeSpentGatheringWithoutDevice();
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        file << ( (double) timeSpentGatheringWithoutDevice[i] / (double) activeAgents[i] ) << ",";
    }
    file << "\n";

    /* timeGatheringWithoutDevice per group */
    vector<vector<int> > timeSpentGatheringWithoutDeviceByGroup = glob.productionStats->getTimeSpentGatheringWithoutDeviceByGroup();
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        file << "timeGatheringWithoutDeviceByGroup " << gId << "_" << glob.SIM_NAME << ",";
        for (int i = 0; i < glob.NUM_DAYS; i++) {
            file << ( (double) timeSpentGatheringWithoutDeviceByGroup[gId][i] / (double)activeGroupAgents[gId][i] ) << ",";
        }
        file << "\n";
    }
    file << "\n\n";       // add two extra lines so more readable.
    file.close();

    activeAgents.clear();
    activeGroupAgents.clear(); 
    timeSpentGatheringWithDevice.clear();
    timeSpentGatheringWithDeviceByGroup.clear();
    timeSpentMakingDevices.clear();
    timeSpentMakingDevicesByGroup.clear();
    timeSpentGatheringWithoutDevice.clear();
    timeSpentGatheringWithoutDeviceByGroup.clear();
}

/**
 * Save the percent of resources gathered by each type of device.
 */
void Utils::saveDevicePercents()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/devicePercents.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    string devicesStr[] = { "TOOL", "MACHINE", "FACTORY", "INDUSTRY", "DEVMACHINE", "DEVFACTORY" };
    vector< vector<double> > percentResGatheredByDevice = glob.productionStats->getPercentResGatheredByDevice();
    for (int i = 0; i < NUM_DEVICE_TYPES - 2; i++) {
        file << devicesStr[i] << "percentResGatheredWith_" << glob.SIM_NAME << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << percentResGatheredByDevice[i][j] << ",";
        }
        file << "\n";
    }
    file << "\n";
    file.close();
    percentResGatheredByDevice.clear();
}

/**
 * Save the percent of made devices by DEVMACHINE and DEVFACTORY by all agents and by each group's all agents.
 */
void Utils::saveDevDevice()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/devDeviceUse.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    int totalMade = 0;
    string devicesStr[] = { "TOOL", "MACHINE", "FACTORY", "INDUSTRY", "DEVMACHINE", "DEVFACTORY" };

    /* devices made with devDevice by total agents */
    vector< vector<int> > devicesMade = glob.productionStats->getDevicesMade();
    vector< vector<int> > devicesMadeWithDevDevice = glob.productionStats->getDevicesMadeWithDevDevice();
    for (int i = 4; i < NUM_DEVICE_TYPES; i++) {
        file << devicesStr[i] << "Use_" << glob.SIM_NAME  << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            if (devicesStr[i] == "DEVMACHINE") {
                totalMade = devicesMade[0][j];
            } else if (devicesStr[i] == "DEVFACTORY") {
                totalMade = devicesMade[1][j];
            }
            if (totalMade > 0) {
                file << ( (double) devicesMadeWithDevDevice[i][j] / (double) totalMade ) << ",";
            } else {
                file << 0 << ",";
            }
        }
        file << "\n";
    }

    /* devices made with devDevice by each group */
    vector< vector< vector <int> > > devicesMadeByGroup = glob.productionStats->getDevicesMadeByGroup();
    vector<vector<vector<int> > > devicesMadeWithDevDeviceByGroup = glob.productionStats->getDevicesMadeWithDevDeviceByGroup();
    for (int gId = 0; gId < glob.NUM_AGENT_GROUPS; gId++) {
        for (int i = 4; i < NUM_DEVICE_TYPES; i++) {
            file << "Group " << gId << "_" << devicesStr[i] << " Use_" << glob.SIM_NAME  << ",";
            for (int j = 0; j < glob.NUM_DAYS; j++) {
                if (devicesStr[i] == "DEVMACHINE") {
                    totalMade = devicesMadeByGroup[gId][0][j];
                } else if (devicesStr[i] == "DEVFACTORY") {
                    totalMade = devicesMadeByGroup[gId][1][j];
                }
                if (totalMade > 0) {
                    file << ( (double) devicesMadeWithDevDeviceByGroup[gId][i][j] / (double) totalMade ) << ",";
                } else {
                    file << 0 << ",";
                }
            }
            file << "\n";
        }
    }
    file << "\n";
    file.close();

    devicesMade.clear();
    devicesMadeWithDevDevice.clear();
    devicesMadeByGroup.clear();
    devicesMadeWithDevDeviceByGroup.clear();
}

/**
 * save deviceComplexity for each day
 */
void Utils::saveDeviceComplexity()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/complexity.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    file << "complexity_" << glob.SIM_NAME << ",";
    vector< vector<double> > percentResGatheredByDevice = glob.productionStats->getPercentResGatheredByDevice();
    double complexityToday;
    for (int i = 0; i < glob.NUM_DAYS; i++) {
        complexityToday = 0.0;
        for (int j = 0; j < 4; j++) {
            complexityToday += percentResGatheredByDevice[j][i] * glob.RES_IN_DEV[j];
        }
        file << complexityToday << ",";
    }
    file << "\n";
    file.close();

    percentResGatheredByDevice.clear();
}


/**
 * save the number of discovered / invented devices by all agents for each day
 */
void Utils::saveDiscoveredDevices()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/deviceDiscovered_.csv";

    headerByDay(file, filePath);
    file.open(filePath.c_str(), ios::app);

    vector< vector<int> > numberOfInventedDevices = glob.otherStats->getNumberOfInventedDevices();
    string devicesStr[] = { "TOOL", "MACHINE", "FACTORY", "INDUSTRY", "DEVMACHINE", "DEVFACTORY" };
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        file << devicesStr[i] << "Discovered_" << glob.SIM_NAME  << ",";
        for (int j = 0; j < glob.NUM_DAYS; j++) {
            file << numberOfInventedDevices[i][j] << ",";
        }
        file << "\n";
    }
    file << "\n";
    file.close();

    numberOfInventedDevices.clear();
}


/**
 * Save results into csv files -- part of graph.py in python.
 * Different from saveResults() in utils.py
 */
void Utils::saveResults()
{
    /*
     * Save the config files into configFiles folder
     */
    int command;
    string conf = glob.SAVE_FOLDER + "/configFiles";
    string commandLine;
    if (glob.configAgentCSV != "") {
        commandLine = "cp " + glob.configAgentFilename + " "
                + glob.configAgentCSV + " "
                + glob.configFilename + " " + conf;
    } else {
        commandLine = "cp " + glob.configAgentFilename + " " + glob.configFilename + " " + conf;
    }
    command = system(commandLine.c_str());

    /*
     * Save the data into each file.
     */
    saveGini();
    saveHHIQuartiles();
    saveTotalUtility();
    saveMeanUtility();
    saveUnitsGathered();
    saveUnitsHeld();
    saveUnitsTraded();
    saveDeviceMade();
    saveTotalTimeUsage();
    saveDevicePercents();
    saveDevDevice();
    saveDeviceComplexity();
    saveDiscoveredDevices();
}

/**
 * Save the number of units held, marginal utilities, and average marginal utility ratio
 *     for each resource by agent[0] before it starts work and after the second trade,
 *     and the number of units bought and sold for each resource by agent[0] during the second trade,
 *     and the number of units gathered for each resource by agent[0] during work
 *     into file agentsData.csv under the saving directory specified when start running.
 */
void Utils::saveEndDayData()
{
    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/agentsData.csv";
    if ( !boost::filesystem::exists(filePath)) {
        file.open(filePath.c_str());
        file << "Day,resId,held(before work),MU(before work),avgMURatio(before work),gathered(after work),bought,sold,held(after trade),MU(after trade),avgMURatio(after trade)\n";
        file.close();
    }
    file.open(filePath.c_str(), ios::app);

    // For now, only do one agent
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        ResProperties &resPr = glob.agent[0]->resProp[resId];
        double avgMURatioBeforeWork = 0;
        double avgMURatioAfterTrade = 0;
        for (int i = 0; i < glob.NUM_RESOURCES; i++) {
            if (i != resId) {
                avgMURatioBeforeWork += (resPr.beforeWorkMU / glob.agent[0]->resProp[i].beforeWorkMU);
                avgMURatioAfterTrade += (resPr.endDayUtilities / glob.agent[0]->resProp[i].endDayUtilities);
            }
        }
        file << glob.currentDay << "," << resId << ","
                << resPr.beforeWorkHeld << "," << resPr.beforeWorkMU << "," << avgMURatioBeforeWork << "," << resPr.unitsGatheredEndWork << ","
                << (resPr.boughtEndDay - resPr.boughtEndWork) << "," << (resPr.soldEndDay - resPr.soldEndWork) << ","
                << resPr.getHeld() << "," << resPr.endDayUtilities << "," << avgMURatioAfterTrade << ",\n";
    }
    file.close();
}

void Utils::removeAgent(int agentNumber, int day)
{
    glob.agent[agentNumber]->remove();
    glob.activeAgents--;
    glob.NUM_ACTIVE_AGENTS_IN_GROUP[glob.agent[agentNumber]->group]--;
    cout << "current number of agents in group " << glob.agent[agentNumber]->group << " = " << glob.NUM_ACTIVE_AGENTS_IN_GROUP[glob.agent[agentNumber]->group] << endl;
    cout << "Agent " << agentNumber << " has been removed on day " << day << endl;
}

void Utils::removeRes(int resNumber, int day)
{
    glob.res[resNumber].remove();
    for (unsigned int i = 0; i < glob.agent.size(); i++) {
        for (int device = 0; device < 6; device++) {
            glob.agent[i]->devProp[device][resNumber].deviceHeld = 0;
        }
    }
    if (glob.ELIMINATE_RESERVES) {
        for (unsigned int i = 0; i < glob.agent.size(); i++) {
            glob.agent[i]->resProp[resNumber].setHeld(0);
            glob.agent[i]->resProp[resNumber].experience = 0;
        }
    }
    cout << "Res " << resNumber << " has been removed on day " << day << endl;
}

/* TODO NOT IMPLEMENTED YET */
void Utils::saveDayStatus(int day)
{
    cout << "saveDayStatus: NOT IMPLEMENTED YET!" << endl;
}

/* TODO NOT IMPLEMENTED YET */
void Utils::loadDayStatus()
{
    cout << "loadDayStatus: NOT IMPLEMENTED YET!" << endl;
}
