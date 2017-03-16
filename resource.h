/**
 * societies  Simulates societies and thier growth based on trade, experience and $
 * Copyright (C) 2011 Tony Ditta, Michael Koster

 * This program is free software; you can redistribute it and/or modify it under t$

 * This program is distributed in the hope that it will be useful, but WITHOUT ANY$

 * You should have received a copy of the GNU General Public License along with th$

 * The resource module includes the definition of the Resource class.
*/

#ifndef _SOC_RESOURCE_H_
#define _SOC_RESOURCE_H_

class Resource
{
public:

    bool inSimulation;
    int ID;

    Resource(int id);
    /**
     * A larger decay slowness means a smaller chance of decaying at the end of
     * each day.
     */
    void remove() { inSimulation = false; }
};


#endif
