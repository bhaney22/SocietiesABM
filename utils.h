/**
 * The utils module includes commonly-used functions for use during simulations.
 */

#ifndef _SOC_UTILS_H_
#define _SOC_UTILS_H_

#include "globals.h"

class Utils {
public:
	void printStartConditions();
	void agentsWork();
	void agentsTrade();
	void agentsInvent();
	void endDay();
	void endDayDecay();
	void agentsTradeDevices();
	void agentsProduceDevices();
	bool deviceExists(device_name_t deviceType);
	void printSumUtilAndRes();
	void dayAnalysis(int dayNumber);
	void removeOrSave(int dayNumber);
	void endSim();
	void headerByDay(ofstream &file, string filePath);
	vector<vector<double> > calcQuartiles(vector<vector<double> > data);
	void firstRunCheck(ofstream &file, string filePath);
	void saveResults();
	void saveGini();
	void saveHHIQuartiles();
	void saveTotalUtility();
	void saveMeanUtility();
	void saveUnitsGathered();
	void saveUnitsHeld();
	void saveUnitsTraded();
	void saveDeviceMade();
	void saveTotalTimeUsage();
	void saveDevicePercents();
	void saveDevDevice();
	void saveDeviceComplexity();
	void saveUseMatrix();
	void saveTradeFlows();
	void saveDeviceRecipes();
	void saveUniqueKey();
	void saveOutput();
	void saveEndDayData();

     void removeAgent(int agentNumber, int day);
     void removeRes(int resNumber, int day);

	void saveDayStatus(int day);		// NOT IMPLEMENtED YET.
	void loadDayStatus();		// NOT IMPLEMENtED YET.
};

extern Utils util;
extern OtherStats oStats;
#endif
