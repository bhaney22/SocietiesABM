/**
 *
 * societies  Simulates societies and their growth based on trade,
 * experience and inventions.
 * XXX Fix this comment
 *
 * Copyright (C) 2013 Tony Ditta, Michael Koster, Victor Norman, Becky
 * Haney, Jiaming Jiang

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. 

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/**
 * Define the properties of resources and devices specified in glob.initializeAgents().
 */

#include "properties.h"
#include "globals.h"
#include <cmath>
#include <cassert>


/**
 * ResProperties constructor.
 * An agent will create an object of this class for a single resource.
 * The object contains all the properties for that resource.
 */
ResProperties::ResProperties()
{
    endDayUtilities = 0.0;
    steepness = 0.0;
    scaling = 0.0;
    assert(marginalUtilities.empty());
    marginalUtilities.clear();
    averageLifetime = 0.0;

    held = 0;
    idleResource = true;
    experience = 0.0;

    minResEffort = 0.0;
    maxResEffort = 0.0;
    maxResExperience = 0.0;

    assert(resEfforts.empty());
    resEfforts.clear();

    resSetAside = 0;
    unitsGatheredToday = 0;
    unitsGatheredEndWork = 0;

    beforeWorkHeld = 0;
    boughtEndWork = 0;
    soldEndWork = 0;
    boughtEndDay = 0;
    soldEndDay = 0;
    beforeWorkMU = 0.0;
}

/**
 * Calculate the efforts needed to gather a resource.
 */
void ResProperties::calcResEfforts()
{
    assert(resEfforts.empty());
    resEfforts.clear();
    for (int potentialExp = 0; potentialExp < int(maxResExperience); potentialExp++) {
        resEfforts.push_back(maxResEffort -
                (maxResEffort - minResEffort)
                * exp(- sqrt(maxResEffort) * 2
                * exp(-(maxResEffort - minResEffort)
                      * potentialExp / maxResExperience)));
    }
}


/**
 * Calculate the initial values of marginal utilities based off of
 * the preferences of the agents of the given resource.
 */
void ResProperties::calcMarginalUtilities()
{
    double potentialHeld = 1.0;
    double util = scaling * pow(potentialHeld, (1.0 / steepness));
    vector<double> utils(1, 0.0);
    // First, calculate the outputs of the utility function and store the
    // results in a list
    while (util - utils[utils.size() - 1] > glob.MIN_RES_UTIL) {
        utils.push_back(util);
        potentialHeld += 1.0;
        util = scaling * pow(potentialHeld, (1.0 / steepness));
    }
    // The marginal utility, then, is the difference between consecutive
    // values in the utility list.
    assert(marginalUtilities.empty());
    marginalUtilities.clear();
    for (int i = 1; i < (int) utils.size(); i++) {
        marginalUtilities.push_back(utils[i] - utils[i - 1]);
    }
}

/**
 * mutator method for held.
 */
void ResProperties::setHeld(int newHeld) {
    held = newHeld;
}

/**
 * DevProperties constructor.
 * An agent will create an object of this class for each device and for each resource of that device.
 * It contains all the properties of that device's resource.
 */
DevProperties::DevProperties()
{
    deviceExperience = 0.0;

    minDeviceEffort = 0.0;
    maxDeviceEffort = 0.0;
    maxDeviceExperience = 0.0; 

    assert(deviceEfforts.empty());
    deviceEfforts.clear();

    deviceHeld = 0;
    idleDevice = true;
    devicesToMake = 0;
    devicesSetAside = 0;
    gainOverDeviceLifeMemory = 0.0;
    gainOverDeviceLifeMemoryValid = false;
    costOfDeviceMemory = 0.0;
    costOfDeviceMemoryValid = false;
    worstCaseConstructionMemory = make_pair(0.0, vector<int>(glob.NUM_RESOURCES, 0));
    worstCaseConstructionMemoryValid = false;

    devicesMadeToday = 0;
    devicesMadeTotal = 0;
    
    devicesSoldTotal = 0;
    devicesBoughtTotal = 0;
    deviceMinutesUsedTotal = 0;
    deviceMinutesDecayTotal = 0;

    devicePrices = vector<double>(glob.DEVICE_TRADE_MEMORY_LENGTH, 0.0);
    devicesRecentlyMade = vector<int>(glob.DEVICE_PRODUCTION_MEMORY_LENGTH, 0);
}

void DevProperties::calcDeviceEfforts()
{
    assert(deviceEfforts.empty());
    deviceEfforts.clear();
    for (int potentialExp = 0; potentialExp < int(maxDeviceExperience); potentialExp++) {
        deviceEfforts.push_back( maxDeviceEffort -
                (maxDeviceEffort - minDeviceEffort)
                * exp(-maxDeviceEffort * 2
                * exp(-(maxDeviceEffort - minDeviceEffort) / 2
                * potentialExp / maxDeviceExperience)));
    }
}
