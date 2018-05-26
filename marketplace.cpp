/*
 * societies  Simulates societies and thier growth based on trade, experience and inventions.
 * Copyright (C) 2011 Tony Ditta, Michael Koster
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */


/**
 * The marketplace module contains the implementations of the Marketplace class
 * and the ResourcePair class -- both of which help control resource trading. 
 */

#include <list>
#include <set>
#include <algorithm> 	// for min(), set_difference.
#include <fstream>
#include <cmath>
#include <cassert>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include "marketplace.h"
#include "agent.h"
#include "statstracker.h"
#include "logging.h"


/**
 * Call genPairs() for glob.RES_TRADE_ROUNDS times.
 */
void ResourceMarketplace::tradeResources()
{
    for (int roundNumber = 0; roundNumber < glob.RES_TRADE_ROUNDS; roundNumber++) {
        genPairs();
    }
}


/**
 * Two agents are randomly chosen to be a trading pair.
 */
void ResourceMarketplace::genPairs()
{
    list<Agent *> agentList;	// use a doubly-linked list that supports quick deletion from in the middle.
    agentList.assign(glob.agent.begin(), glob.agent.end());

    vector<ResourcePair> pairs;	
    while (agentList.size() > 1) {
#ifdef DONT_RANDOMIZE
        int loc = 0;
#else
        int loc = glob.random_int(0, agentList.size());
#endif
        list<Agent *>::iterator it = agentList.begin();
        advance(it, loc);	// cannot use ++ or + loc with list iterators.
        Agent *firstAgent = *it;
        agentList.erase(it);
        loc = glob.random_int(0, agentList.size());
        it = agentList.begin();
        advance(it, loc);
        Agent *secondAgent = *it;
        agentList.erase(it);
        pairs.push_back(ResourcePair(firstAgent, secondAgent));
    }

    assert( (agentList.size() == 0) || (agentList.size() == 1));

    BOOST_FOREACH(ResourcePair pair, pairs) {
        pair.pairTrade();
    }
}

/**
 * ResourcePair constructor.
 * \param agentA the first agent of the pair
 * \param agentB the second agent of the pair, cannot be the same as agentA
 */
ResourcePair::ResourcePair(Agent *agentA, Agent *agentB)
{
    this->agentA = agentA;
    this->agentB = agentB;
    assert(orderedPossiblePairs.empty());
    currentTradeAttempts = 0;
    tradeRatio = 0;
    aPick = 0;
    bPick = 0;
    numAPicked = 0;
    numBPicked = 0;

    epsilon = glob.TRADE_EPSILON;
}

/**
 * Used below to sort a vector of pairs from smallest to largest.
 * We only sort on the first item.
 */
bool sortpairs(const pair<double, int>& i, const pair<double, int>& j)
{
    return i.first < j.first;
}

/**
 * Each agent calculates the average marginal utility of all resources
 * it holds, and offers the MENU_SIZE resources that it values the least.
 */
void ResourcePair::pairTrade()
{
    if (! (agentA->inSimulation && agentB->inSimulation)) {
        return;
    }
    if (currentTradeAttempts >= glob.RES_TRADE_ATTEMPTS) {
        return;
    }
    /* aEnd and bEnd are lists of ordered pairs; the first element
     * of each ordered pair is the current MU that the agent has for
     * the given resource, and the second element of each ordered
     * pair is the number ID of the resource.
     */
    vector< pair<double, int> > aEnd, bEnd;
    for (int resId = 0; resId < glob.NUM_RESOURCES; resId++) {
        if (agentA->getHeld(resId) > 0) {
            aEnd.push_back(pair<double,int>(agentA->resProp[resId].endDayUtilities,
                                            resId));
        }
        if (agentB->getHeld(resId) > 0) {
            bEnd.push_back(pair<double,int>(agentB->resProp[resId].endDayUtilities,
                                            resId));
        }
    }

    // These lists are sorted on the first element (MU)
    sort(aEnd.begin(), aEnd.end(), sortpairs);
    sort(bEnd.begin(), bEnd.end(), sortpairs);

    set<int>tempASelling, tempBSelling;

    int aMenuSize = min(glob.MENU_SIZE, (int) aEnd.size());
    int bMenuSize = min(glob.MENU_SIZE, (int) bEnd.size());
    for (int eachMenuItem = 0; eachMenuItem < aMenuSize; eachMenuItem++) {
        tempASelling.insert(aEnd[eachMenuItem].second);  // second is resId
    }
    for (int eachMenuItem = 0; eachMenuItem < bMenuSize; eachMenuItem++) {
        tempBSelling.insert(bEnd[eachMenuItem].second);  // second is resId
    }
    
    // To avoid unnecessary computation, the set of B's offers is 
    // removed from A's offers and vice versa (that way, both agents
    // aren't trying to sell the same resources).

    // see http://stackoverflow.com/questions/283977/c-stl-set-difference
    // this->aSelling = tempASelling - tempBSelling;
    set_difference(tempASelling.begin(), tempASelling.end(),
                   tempBSelling.begin(), tempBSelling.end(),
                   inserter(this->aSelling, this->aSelling.end()));

    // this->bSelling = tempBSelling - tempASelling;
    set_difference(tempBSelling.begin(), tempBSelling.end(),
                   tempASelling.begin(), tempASelling.end(),
                   inserter(this->bSelling, this->bSelling.end()));

    // If the agents both have something to offer, they begin the trade.
    if (! aSelling.empty() && ! bSelling.empty()) {
        makePicks();
    }
}


/**
 * Each agent creates a list of resource pairs (one that it would give up
 * and one that it would receive) that it would like to bring to
 * trade. They are ordered in preference by the ratio of utility received
 * over utility given up.
 */
void ResourcePair::makePicks()
{
    /* possiblePairs is a list of ordered pairs; each element is an ordered
     * pair of resources: the first element in the ordered pair is a
     * resource that A is offering and the second element in the ordered
     * pair is a resource that B is offering.
     */
    vector<ResPair> possiblePairs;
    // aRatios and bRatios are lists of the ratios of the marginal utilities
    // of the pair found at the corresponding index of possiblePairs
    list<double> aRatios;
    list<double> bRatios;

    // TODO: I really have to wonder if all this rigamarole below with
    // advance(), distance(), etc., is worth it.  Is it really that
    // important to use a list here.  If we use a vector or an array we can
    // calculate all this things so much more cleanly...
    BOOST_FOREACH(int aOffered, aSelling) {
        BOOST_FOREACH(int bOffered, bSelling) {
            possiblePairs.push_back(ResPair(aOffered, bOffered));
            aRatios.push_back(agentA->resProp[bOffered].endDayUtilities /
                              agentA->tempUtilCalc(aOffered, -1));
            bRatios.push_back(agentB->resProp[aOffered].endDayUtilities /
                              agentB->tempUtilCalc(bOffered, -1));
        }
    }
    
    // The agents take turns 'picking' which pair they would like to trade
    // (i.e. the pair of resources for which the ratio of MU is the highest).
    for (int eachPairIdx = 0; eachPairIdx < glob.RES_TRADE_ATTEMPTS; eachPairIdx++) {
        if (possiblePairs.size() > 0) {
            if ((eachPairIdx % 2) == (currentTradeAttempts % 2)) {
                // aFav is the index of the max element in aRatios.
                list<double>::iterator maxARatiosIt = max_element(aRatios.begin(), aRatios.end());
                int aFav = distance(aRatios.begin(), maxARatiosIt);
                orderedPossiblePairs.push(possiblePairs[aFav]);
                possiblePairs.erase(possiblePairs.begin() + aFav);
                aRatios.erase(maxARatiosIt);
                list<double>::iterator it = bRatios.begin();
                advance(it, aFav);
                bRatios.erase(it);
            } else {
                // bFav is the index of the max element in bRatios.
                list<double>::iterator maxBRatiosIt = max_element(bRatios.begin(), bRatios.end());
                int bFav = distance(bRatios.begin(), maxBRatiosIt);
                orderedPossiblePairs.push(possiblePairs[bFav]);
                possiblePairs.erase(possiblePairs.begin() + bFav);
                list<double>::iterator it = aRatios.begin();
                advance(it, bFav);
                aRatios.erase(it);
                bRatios.erase(maxBRatiosIt);
            }
        }
    }
    if (! orderedPossiblePairs.empty()) {
        attemptPossibleTrades();
    } 
}

/**
 * The resource that A offered becomes B's pick, and the resource that
 * B offered becomes A's pick, and they attempt a trade.
 */
void ResourcePair::attemptPossibleTrades()
{
    if (currentTradeAttempts < glob.RES_TRADE_ATTEMPTS) {
        setPicks(orderedPossiblePairs.front().second, orderedPossiblePairs.front().first);
        orderedPossiblePairs.pop();
        calcRatio();
    }
}

/**
 * Per unit of bPick, agentA calculates how much aPick it wants to buy and
 * agentB calculates how much aPick it is willing to sell.
 */
void ResourcePair::calcRatio()
{
    double aRatio = agentA->utilCalc(aPick) / agentA->tempUtilCalc(bPick, -1);
    double bRatio = agentB->tempUtilCalc(aPick, -1) / agentB->utilCalc(bPick);
    // The geometric mean of the two ratios is selected (this assumes the
    // agents have bargained equally well).
    setTradeRatio(exp((aRatio*log(agentB->resTradePower) + bRatio*log(agentB->resTradePower)) / (agentA->resTradePower+agentB->resTradePower)));
    aMakesOffer();
}

/**
 * Round a double to an int.
 */
static int myround(double val)
{
    return (int) floor(val + 0.5);
}
    
/**
 * Using the trade ratio, Agent A offers the trade that it would
 * benefit from the most.
 */
void ResourcePair::aMakesOffer()
{
    double tradeRatio = this->tradeRatio;
    Agent *agentA = this->agentA;
    Agent *agentB = this->agentB;
    int aPick = this->aPick;
    int bPick = this->bPick;
    
    /* The offers begin with the smallest possible offer (the amount offered
     * of one resource is 1 unit, and the amount offered of the other
     * resource is 1 or greater).
     *
     * Because the ratios of marginal utilities will be continuous and
     * units of resources are discrete, some rounding must occur; so, an
     * exact amount offered at the given ratio is calculated, and the
     * actual amount (in discrete units) is actually offered.
     */
    int numAPicked, numBPicked;
    double exactNumAPicked;
    if (tradeRatio <= 1.0) {
        numBPicked = myround(1.0 / tradeRatio);
    } else { /* tradeRatio > 1 */
        numBPicked = 1;
    }
    exactNumAPicked = numBPicked * tradeRatio;
    numAPicked = myround(exactNumAPicked);

    // aGains is the utility surpluses for each offer.
    vector<double> aGains;
    aGains.push_back(agentA->barterUtility(aPick, numAPicked) -
                     agentA->barterUtility(bPick, - numBPicked));
    vector< pair<int, int> > offers;
    offers.push_back(pair<int,int>(numAPicked, numBPicked));
    // While both agents have enough resources and agent A would benefit
    // from trade, A continues to consider larger and larger offers.
    while ( (numBPicked + 1 <= agentA->resHeld(bPick)) &&
            (numAPicked + 1 <= agentB->resHeld(aPick)) &&
            (agentA->barterUtility(aPick, myround(exactNumAPicked + tradeRatio)) -
            agentA->barterUtility(bPick, - numBPicked - 1)) > epsilon) {
        aGains.push_back(agentA->barterUtility(aPick, numAPicked) -
                         agentA->barterUtility(bPick, - numBPicked));
        offers.push_back(pair<int,int>(numAPicked, numBPicked));
        numBPicked++;
        exactNumAPicked = numBPicked * tradeRatio;
        numAPicked = myround(exactNumAPicked);
    }
    
    // A picks the offer that benefits it the most.
    int bestOffer = max_element(aGains.begin(), aGains.end()) - aGains.begin();  // convert to index.
    numAPicked = offers[bestOffer].first;
    numBPicked = offers[bestOffer].second;
    
    // If both agents would benefit from this trade, then they swap resources.
    if ((numAPicked <= agentB->resHeld(aPick)) && (numBPicked <= agentA->resHeld(bPick)) &&
        (agentB->barterUtility(bPick, numBPicked) - agentB->barterUtility(aPick, - numAPicked) > epsilon) &&
        (agentA->barterUtility(aPick, numAPicked) - agentA->barterUtility(bPick, - numBPicked) > epsilon)) {
        setNumPicked(numAPicked, numBPicked);
        makeTrade();
    } else {
        // If agentB wouldn't benefit from the trade, then agentB gets a
        // chance to make an offer.
        bMakesCounterOffer();
    }
}


/**
 * Using the trade ratio, Agent B offers the trade that it would
 * benefit from the most.
 */
void ResourcePair::bMakesCounterOffer()
{
    double tradeRatio = this->tradeRatio;
    Agent *agentA = this->agentA;
    Agent *agentB = this->agentB;
    int aPick = this->aPick;
    int bPick = this->bPick;

    /* 
     * The offers begin with the smallest possible offer (the amount offered
     * of one resource is 1 unit, and the amount offered of the other
     * resource is 1 or greater).
     *
     * Because the ratios of marginal utilities will be continuous and units
     * of resources are discrete, some rounding must occur; so, an exact
     * amount offered at the given ratio is calculated, and the actual
     * amount (in discrete units) is actually offered.
     */
    int numAPicked, numBPicked;
    double exactNumAPicked;
    if (tradeRatio <= 1) {
        numBPicked = myround(1.0 / tradeRatio);
    } else { // tradeRatio > 1
        numBPicked = 1;
    }
    exactNumAPicked = numBPicked * tradeRatio;
    numAPicked = myround(exactNumAPicked);
    
    // bGains is the utility surpluses for each offer.
    vector<double> bGains;
    bGains.push_back(agentB->barterUtility(bPick, numBPicked) -
                     agentB->barterUtility(aPick, - numAPicked));
    vector< pair<int, int> > offers;
    offers.push_back(pair<int,int>(numAPicked, numBPicked));
    
    // While both agents have enough resources and agent B would benefit
    // from trade, B continues to consider larger and larger offers.
    while (numAPicked + 1 <= agentB->resHeld(aPick) &&
           numBPicked + 1 <= agentA->resHeld(bPick) &&
           agentB->barterUtility(bPick, numBPicked + 1) -
           agentB->barterUtility(aPick, - myround(exactNumAPicked + tradeRatio)) > epsilon) {
        bGains.push_back(agentB->barterUtility(bPick, numBPicked) -
                         agentB->barterUtility(aPick, - numAPicked));
        offers.push_back(pair<int,int>(numAPicked, numBPicked));
        numBPicked++;
        exactNumAPicked = numBPicked * tradeRatio;
        numAPicked = myround(exactNumAPicked);
    }
    
    // B picks the offer that benefits it the most
    int bestOffer = max_element(bGains.begin(), bGains.end()) - bGains.begin();
    numAPicked = offers[bestOffer].first;
    numBPicked = offers[bestOffer].second;
    
    // If both agents would benefit from this trade, then they swap resources
    if (numBPicked <= agentA->resHeld(bPick) && numAPicked <= agentB->resHeld(aPick) &&
        agentA->barterUtility(aPick, numAPicked) - agentA->barterUtility(bPick, - numBPicked) > epsilon &&
        agentB->barterUtility(bPick, numBPicked) - agentB->barterUtility(aPick, - numAPicked) > epsilon) {
        setNumPicked(numAPicked, numBPicked);
        makeTrade();
    }
    else if (! orderedPossiblePairs.empty()) {
        // If there are still possible trades to be made, the agents continue
        currentTradeAttempts++;
        attemptPossibleTrades();
    }
}

/**
 * Add a log file to capture trade data -- BRH (2015?)
 * Quit calling this function as soon as the next function (save vector in memory) is working.11.11.2017
 */
 //***************** SOON TO BE DELETED FUNCTION ************************
void ResourcePair::writeTradeData()
{
    /* calculate the average holdings of resource A & B across all agents. */ 
    int totalHeldResA = 0;
    int totalHeldResB = 0;
    for (int i = 0; i < glob.NUM_AGENTS; i++) {
        totalHeldResA += glob.agent[i]->resProp[aPick].getHeld();
        totalHeldResB += glob.agent[i]->resProp[bPick].getHeld();
    }
    double avgHeldResA = (double) totalHeldResA / (double) glob.NUM_AGENTS;
    double avgHeldResB = (double) totalHeldResB / (double) glob.NUM_AGENTS;

    /* Calculate the MU that any agent (I use agentA) would place on holding that many units of Resources A & B. */
    double MUResA = agentA->resProp[aPick].marginalUtilities[avgHeldResA];
    double MUResB = agentA->resProp[bPick].marginalUtilities[avgHeldResB];

    /* Calculate the ratio of MUResA/MUResB for agentA and B */
    double MUResAAgentA = agentA->resProp[aPick].marginalUtilities[agentA->resProp[aPick].getHeld()];
    double MUResBAgentA = agentA->resProp[bPick].marginalUtilities[agentA->resProp[bPick].getHeld()];
    double MURatioAgentA = MUResAAgentA / MUResBAgentA;

    /*
     * Calculate "MyTradeRatio" which is the price using the ratio of MU of
     * how much the agents holds of the two resources at the time
     * the two agents meet (Epstein & Axtell, p. 103). It seems to be very close, but
     * not the same as the TradeRatio used above.
     */
    double MUResAAgentB = agentB->resProp[aPick].marginalUtilities[agentB->resProp[aPick].getHeld()];
    double MUResBAgentB = agentB->resProp[bPick].marginalUtilities[agentB->resProp[bPick].getHeld()];
    double MURatioAgentB = MUResAAgentB / MUResBAgentB;
    double myTradeRatio = sqrt(MURatioAgentA*MURatioAgentB);

    double actualPrice = double(numAPicked) / double(numBPicked);
    double equilPrice = double(MUResA) / double(MUResB);
    double ratio = actualPrice / equilPrice;

    ofstream file;
    string filePath = glob.SAVE_FOLDER + "/trades.csv";
    if ( !boost::filesystem::exists(filePath)) {
        file.open(filePath.c_str());
        string header[] = {"UniqueKey","Config","Run","Day", "Round",
                "Agent A", "Res A", "1.#ResA Bought", "Avg #ResA Held", "2.MU ResA",
                "Agent B", "ResB", "3.#ResB Bought", "Avg #ResB Held","MU ResB",
                "5.ActualPrice (1./3.)","6.EquilPrice (2./4.)","Ratio (5./6.)",
                "MURatioAgentA","MURatioAgentB","MyTradeRatio","TradeRatio"
        };
        for (int i = 0; i < 22;  i++) {
            file << header[i] << ",";
        }
        file << "\n";
        file.close();
    }
    file.open(filePath.c_str(), ios::app);
			/* ORDER: UniqueKey, Config, Run (SIMNAME for now), Day, ... */
			file << glob.UniqueKey << ",";
			file << glob.configName << "," ;
			file << glob.SIM_NAME << "," ;
			file << glob.currentDay << "," << currentTradeAttempts << ","
            << agentA->name << "," << aPick << "," << numAPicked << "," << avgHeldResA << "," << MUResA << ","
            << agentB->name << "," << bPick << "," << numBPicked << "," << avgHeldResB << "," << MUResB << ","
            << actualPrice << "," << equilPrice << "," << ratio << ","
            << MURatioAgentA << "," << MURatioAgentB << "," << myTradeRatio << "," << tradeRatio << "\n";
    file.close();
}

/**
 * Agents trade the resources they have agreed to trade.
 */
void ResourcePair::makeTrade()
{
    LOG(1) << "Agent " << agentA->name << " is about to gain " <<
        agentA->barterUtility(aPick, numAPicked) << " and lose " <<
        agentA->barterUtility(bPick, - numBPicked) << " by selling " <<
        numBPicked << " units of resource " << bPick;

    LOG(1) << "Agent " << agentB->name << " is about to gain " <<
        agentB->barterUtility(bPick, numBPicked) << " and lose " <<
        agentB->barterUtility(aPick, - numAPicked) << " by selling " <<
        numAPicked << " units of resource " << aPick;

	//Write trade to tradeLog in memory
	vector<int> tradeInfo(11);
		tradeInfo[0]=agentA->name; //* A sells to B
		tradeInfo[1]=agentB->name;
		tradeInfo[2]=bPick;
		tradeInfo[3]=numBPicked;
		tradeInfo[4]=agentA->barterUtility(bPick, - numBPicked); //SellerLosesMU
		tradeInfo[5]=agentB->barterUtility(bPick, - numBPicked); //BuyerGainsMU
		tradeInfo[6]=aPick;
		tradeInfo[7]=numAPicked;
		tradeInfo[8]=agentA->barterUtility(aPick, - numAPicked); //SellerGainsMU
		tradeInfo[9]=agentB->barterUtility(aPick, - numAPicked); //BuyerLosesMU
	
    // Agents give up and gain the resources at the decided amounts.
    agentB->sells(aPick, numAPicked);
    agentA->buys(aPick, numAPicked);
    agentA->sells(bPick, numBPicked);
    agentB->buys(bPick, numBPicked);

    // Update utilities for each resource.
    agentA->resProp[bPick].endDayUtilities = agentA->utilCalc(bPick);
    agentA->resProp[aPick].endDayUtilities = agentA->utilCalc(aPick);
    agentB->resProp[aPick].endDayUtilities = agentB->utilCalc(aPick);
    agentB->resProp[bPick].endDayUtilities = agentB->utilCalc(bPick);
    tradingStatsUpdate();

    // The agents are given a chance to trade again.
    currentTradeAttempts++;
    pairTrade();

    // Set end day resources data
    agentA->resProp[aPick].boughtEndDay += numAPicked;
    agentA->resProp[bPick].soldEndDay += numBPicked;
    agentB->resProp[aPick].soldEndDay += numAPicked;
    agentB->resProp[bPick].boughtEndDay += numBPicked;

//    writeTradeData();  /*BRH 11.11.2017 Instead of writing a file every day, save a matrix and write it all at once at end of sim. */

}

/**
 * Update the stats after trading.
 */
void ResourcePair::tradingStatsUpdate()
{
    agentA->unitsSoldToday += numBPicked;
    agentB->unitsSoldToday += numAPicked;
    if (agentA->group != agentB->group) {
        agentA->unitsSoldCrossGroupToday += numBPicked;
        agentB->unitsSoldCrossGroupToday += numAPicked;
    }
    if (glob.SAVE_TRADES) {
    	glob.tradeStats->newExchange(*this);
    }
}







