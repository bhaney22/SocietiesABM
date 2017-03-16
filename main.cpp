/**
 * The main file to start running the program.
 * To run it, type ./societies
 * Can also use -p, -z, ... to specify the options. See options.cpp for more information on te options.
 */
#include <stdlib.h>
#include "globals.h"
#include "utils.h"
#include "options.h"
#include "logging.h"
#include "statstracker.h"

Globals glob;
Utils util;

void runSimulation()
{
    glob.perRunInitialization();

    glob.startTimer();

    for (int i = glob.START_DAY; i < glob.NUM_DAYS; i++) {
        LOG(1) << "Day " << i;
        glob.currentDay = i;
        util.agentsTradeDevices();
        util.agentsProduceDevices();
        util.agentsTrade();
        util.agentsWork();
        util.agentsTrade();
        util.agentsInvent();
        util.endDay();
        util.dayAnalysis(i);
        util.endDayDecay();
        util.removeOrSave(i);
    }

    glob.endTimer();

    LOG(1) << "Ending simulation";
    util.endSim();

    cout << "The total number of seconds is " << (glob.endTime - glob.startTime) << endl;
}


int main(int argc, char *argv[])
{
    glob = Globals();
    /* check if parse_args run correctly. */
    int result = parse_args(argc, argv);
    if (result < 0) {
        cerr << "Error in parse_args: exiting now." << endl;
    	return 0;
    }

    glob.initGlobalStructures();
    glob.setAdvancedOptions();

    if (glob.noRunSet) {
        glob.printConfig();
        exit(0);
    }

    runSimulation();
}
