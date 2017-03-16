#include <boost/foreach.hpp>

#include "device.h"
#include "agent.h"
#include "statstrackers.h"

TradeStats::TradeStats () {
    resTradeVolume = new int [glob.NUM_DAYS];
    resTradeForDeviceVolume = new int [glob.NUM_DAYS];
}

void TradeStats::dailyUpdate(const int dayNum) {
    int tradeVolume = 0;
    int tradeForDeviceVolume = 0;

    BOOST_FOREACH(Agent *agent, glob.agent) {
        tradeVolume += agent->unitsSoldToday;
        tradeForDeviceVolume += agent->unitsSoldForDevicesToday;
    }

    resTradeVolume[dayNum] = tradeVolume;
    resTradeForDeviceVolume[dayNum] = tradeForDeviceVolume;
}

ExtractionStats::ExtractionStats () {
    resExtracted = new int [glob.NUM_DAYS];
    
    resExtractedByAgent = new int* [glob.NUM_AGENTS];
    for (int i = 0; i < glob.NUM_AGENTS; i++) {
        resExtractedByAgent[i] = new int [glob.NUM_DAYS];
    timeSpentExtractingWithoutDevice = new double [glob.NUM_DAYS];
    }

    resExtractedByResByAgent = new int** [glob.NUM_RESOURCES];
    for (int i = 0; i < glob.NUM_RESOURCES; i++) {
        resExtractedByResByAgent[i] = new int* [glob.NUM_AGENTS];
        for (int j = 0; j < glob.NUM_AGENTS; j++) {
            resExtractedByResByAgent[i][j] = new int [glob.NUM_DAYS];
        }
    }

    timeSpentExtractingWithoutDeviceByAgent = new double* [glob.NUM_AGENTS];
    for (int i = 0; i < glob.NUM_AGENTS; i++) {
        timeSpentExtractingWithoutDeviceByAgent[i] = new double[glob.NUM_DAYS];
    }

    resExtractedByRes = new int* [glob.NUM_RESOURCES];
    for (int i = 0; i < glob.NUM_RESOURCES; i++) {
        resExtractedByRes[i] = new int [glob.NUM_DAYS];
    }

    resExtractedByDevice = new int* [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        resExtractedByDevice[i] = new int [glob.NUM_DAYS];
    }

    resExtractedByDeviceByRes = new int** [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        resExtractedByDeviceByRes[i] = new int* [glob.NUM_RESOURCES];
        for (int j = 0; j < glob.NUM_RESOURCES; j++) {
            resExtractedByDeviceByRes[i][j] = new int [glob.NUM_DAYS];
        }
    }

    percentResExtractedByDevice = new double* [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        percentResExtractedByDevice[i] = new double [glob.NUM_DAYS];
    }

    percentResExtractedByDeviceByRes = new double** [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        percentResExtractedByDeviceByRes[i] = new double* [glob.NUM_RESOURCES];
        for (int j = 0; j < glob.NUM_RESOURCES; j++) {
            percentResExtractedByDeviceByRes[i][j] = new double [glob.NUM_DAYS];
        }
    }

    timeSpentExtractingWithDevice = new double* [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        timeSpentExtractingWithDevice[i] = new double [glob.NUM_DAYS];
    }

    timeSpentExtractingWithDeviceByAgent = new double** [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        timeSpentExtractingWithDeviceByAgent[i] = new double* [glob.NUM_AGENTS];
        for (int j = 0; j < glob.NUM_AGENTS; j++) {
            timeSpentExtractingWithDeviceByAgent[i][j] = new double [glob.NUM_DAYS];
        }
    }
}

void ExtractionStats::dailyUpdate(const int dayNum) {
    int totalExtracted = 0;
    int agentExtracted;

    BOOST_FOREACH(Agent *agent, glob.agent) {
        agentExtracted = 0;
        for (vector<ResProperties>::iterator it = agent->resProp.begin();
             it < agent->resProp.end(); it++) {
            agentExtracted += it->unitsExtractedToday;
        }
        resExtractedByAgent[agent->name][dayNum] = agentExtracted;
        totalExtracted += agentExtracted;
    }

    resExtracted[dayNum] = totalExtracted;

    int sumResExtracted;
    int agentResExtracted;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        sumResExtracted = 0;
        BOOST_FOREACH(Agent *agent, glob.agent) {
            agentResExtracted = agent->resProp[resId].unitsExtractedToday;
            resExtractedByResByAgent[resId][agent->name][dayNum] = agentResExtracted;
            sumResExtracted += agentResExtracted;
        }
        resExtractedByRes[resId][dayNum] = sumResExtracted;
    }

    int sumDeviceExtracted;
    for (int devType = 0; devType < NUM_DEVICE_TYPES; devType++) {
        sumDeviceExtracted = 0;
        BOOST_FOREACH(Agent *agent, glob.agent) {
            for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
                sumDeviceExtracted += agent->unitsExtractedWithDeviceToday[devType][resId];
            }
        }
        resExtractedByDevice[devType][dayNum] = sumDeviceExtracted;
    }
            
}

ProductionStats::ProductionStats () {
    devicesMade = new int* [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        devicesMade[i] = new int [glob.NUM_DAYS];
    }

    devicesMadeByRes = new int** [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        devicesMadeByRes[i] = new int* [glob.NUM_RESOURCES];
        for (int j = 0; j < glob.NUM_RESOURCES; j++) {
            devicesMadeByRes[i][j] = new int [glob.NUM_DAYS];
        }
    }

    timeSpentMakingDevices = new double [glob.NUM_DAYS];

    
    timeSpentMakingDevicesByAgent = new double* [glob.NUM_AGENTS];
    for (int i = 0; i < glob.NUM_AGENTS; i++) {
        timeSpentMakingDevicesByAgent[i] = new double [glob.NUM_DAYS];
    }

    /* devicesMadeWithDevDevice and devicesMadeWithDevDeviceByRes are indexed by DEVMACHINE through DEVFACTORY.
       We'll still make 6 vectors, but will only fill in the vectors inside of
       those for the last 2. */
    devicesMadeWithDevDevice = new int* [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        devicesMadeWithDevDevice[i] = new int [glob.NUM_DAYS];
    }

    devicesMadeWithDevDeviceByRes = new int** [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        devicesMadeWithDevDeviceByRes[i] = new int* [glob.NUM_RESOURCES];
        for (int j = 0; j < glob.NUM_RESOURCES; j++) {
           devicesMadeWithDevDeviceByRes[i][j] = new int [glob.NUM_DAYS];
        }
    }
}



OtherStats::OtherStats() {
    activeAgents = new int [glob.NUM_DAYS];

    sumRes = new int [glob.NUM_DAYS];

    sumResByAgent = new int* [glob.NUM_AGENTS];
    for (int i = 0; i < glob.NUM_AGENTS; i++) {
        sumResByAgent[i] = new int [glob.NUM_DAYS];
    }

    sumUtil = new double [glob.NUM_DAYS];

    sumUtilByAgent = new double* [glob.NUM_AGENTS];
    for (int i = 0; i < glob.NUM_AGENTS; i++) {
        sumUtilByAgent[i] = new double [glob.NUM_DAYS];
    }

    numberOfInventedDevices = new int* [NUM_DEVICE_TYPES];
    for (int i = 0; i < NUM_DEVICE_TYPES; i++) {
        numberOfInventedDevices[i] = new int [glob.NUM_DAYS];
    }
}
