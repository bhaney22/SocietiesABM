#ifndef _SOC_DEVMARKETPLACE_
#define _SOC_DEVMARKETPLACE_

#include "globals.h"
#include "utils.h"

class Agent;
class Utils;
class DevicePair;
using namespace std;

class DeviceMarketplace
{
private:
    device_name_t device;
    double epsilon;
    void genDevicePairs();

public:
    DeviceMarketplace() { epsilon = glob.TRADE_EPSILON; };
    void tradeDevices(device_name_t device);
};

class DevicePair
{
private:
    device_name_t deviceType;   //!< the device type being sold this round

    //! agentA and agentB will take turns being the deviceBuyer and deviceSeller
    Agent *agentA, *agentB, *deviceBuyer, *deviceSeller;

    int currentTradeAttempts;

    vector<int> deviceBuyerSelling;     //!< the menu of the MENU_SIZE resources that the deviceBuyer values the least
    vector<int> deviceSellerSelling;    //!< the list of devices that the device seller is able to make.

    int deviceBuyerPick;    //!< the ID of the device that will be bought

    /**
     * finalOffer is the final bundle of goods that will be sold by the
     * device buyer and received by the device seller; it is a list of
     * length NUM_RESOURCES, and each element is the amount of the resource
     * with the corresponding ID that will be bought/sold.
     */
    vector<int> finalOffer;

    /**
     * Util gains and losses are tracked to make sure that both agents
     * benefit from trade and to allow agents to store the gains and losses
     * in their own memory.
     */
    double deviceSellerUtilGain, deviceSellerUtilLoss, deviceBuyerUtilGain, deviceBuyerUtilLoss;

    vector<int> deviceBuyerAvailableRes();
    vector<int> deviceSellerAvailableDevices();
    void makePicks();
    void calcOffers();
    vector<int> calcDeviceSellerOffers();
    vector<int> calcDeviceBuyerOffers();
    void checkGains();
    void makeTrade();
    void switchAndTryAgain();
    void agentMemoryUpdate();
    void tradeStatsUpdate();

public:
    DevicePair(Agent *agentA, Agent *agentB, device_name_t deviceType);
    void pairDeviceTrade();
};


#endif
