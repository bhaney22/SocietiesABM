#ifndef _SOC_MARKETPLACE_
#define _SOC_MARKETPLACE_

#include <queue>
#include <set>
#include "globals.h"

class Agent;

using namespace std;

class ResourceMarketplace
{
private:
    void genPairs();

public:
    ResourceMarketplace() { }
    void tradeResources();
};


class ResourcePair
{
private:
    
    /**
     * A pair of two indeces of the two traading agents.
     */
    typedef pair<int,int> ResPair;
    
    Agent *agentA;      // *** one the agents that is picked to trade
    Agent *agentB;      // *** the other agent that is picked to trade

    double epsilon;

    int currentTradeAttempts;     // *** keep track of how much times they have tried to trade
    /**
     * orderedPossiblePairs is a list of order pairs of resource IDs that 
     * the agents have ordered representing the resources that they want to
     * trade.  We use a queue to implement this as we always add to the end
     * and remove from the front.
     */
    queue<ResPair> orderedPossiblePairs;

    /**
     * Resource IDs that correspond to the resource that
     * agent A is buying and agent B is buying.
     */
    int aPick, bPick;

    /**
     * The number of units of their picked
     * resource that the agents are going to get in the trade.
     */
    int numAPicked, numBPicked;

    /**
     * The 'price' of aPick in terms of bPick (i.e. the number
     * of units of bPick that will be traded for 1 unit of aPick).
     */
    double tradeRatio;

    set<int> aSelling;      // *** a set of resource IDs that agentA offers to sell
    set<int> bSelling;      // *** a set of resource IDs that agentB offers to sell

    void setPicks(int aPick, int bPick) {
        this->aPick = aPick; this->bPick = bPick;
    }
    void setNumPicked(int numAPicked, int numBPicked) {
        this->numAPicked = numAPicked; this->numBPicked = numBPicked;
    }
    void setTradeRatio(double tradeRatio) { this->tradeRatio = tradeRatio; }
    void makePicks();
    void attemptPossibleTrades();
    void calcRatio();
    void aMakesOffer();
    void bMakesCounterOffer();
    void makeTrade();
    void tradingStatsUpdate();
    void writeTradeData();
	void saveAgentTradeData();

public:
    ResourcePair(Agent *agentA, Agent *agentB);
    int getAPick() { return aPick; }
    int getBPick() { return bPick; }
    int getNumAPicked() { return numAPicked; }
    int getNumBPicked() { return numBPicked; }
    void pairTrade();
};
#endif
