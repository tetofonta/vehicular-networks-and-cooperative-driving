#include <Protocol.h>
#include "PlatoonSearchCAM_m.h"

#include "plexe/protocols/BaseProtocol.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"
#include "veins/base/utils/FindModule.h"
#include "plexe/scenarios/ManeuverScenario.h"
#include "plexe/messages/InterferingBeacon_m.h"
#include "veins/modules/messages/PhyControlMessage_m.h"
#include "plexe/driver/Veins11pRadioDriver.h"

namespace plexe::vncd {

    Define_Module(PlatooningProtocol)

    void PlatooningProtocol::initialize(int stage)
    {
        BaseProtocol::initialize(stage);

        if (stage == 0) {

            // get gates
            lowerLayerIn = findGate("lowerLayerIn");
            lowerLayerOut = findGate("lowerLayerOut");

            // get traci interface
            mobility = veins::TraCIMobilityAccess().get(getParentModule());
            traci = mobility->getCommandInterface();
            traciVehicle = mobility->getVehicleCommandInterface();


            if (Veins11pRadioDriver* driver = FindModule<Veins11pRadioDriver*>::findSubModule(getParentModule())) {
                driver->registerNode(getParentModule()->getIndex() + 1e6);
            }

            // random start time
            SimTime beginTime = SimTime(uniform(0.001, beaconingInterval));
            if (beaconingInterval > 0) scheduleAt(simTime() + beaconingInterval + beginTime, sendBeacon);


        }
    }

    void PlatooningProtocol::handleSelfMsg(cMessage* msg)
    {
        BaseProtocol::handleSelfMsg(msg);

        if (msg == this->sendBeacon) {
            sendPlatooningMessage(-1);
            scheduleAt(simTime() + beaconingInterval, sendBeacon);
        } else if (msg == this->platoonFormationAdvertisement){
            this->sendPlatoonAdvertisementBeacon(this->can_be_leader);
            scheduleAfter(1, this->platoonFormationAdvertisement);
        } else if (msg == this->platoonUnicast){

            auto pkt = this->currentSendingPacket->dup();
            this->sendPacket(pkt);

            if(this->currentSendingRetries == -1 || this->currentSendingRetries-- > 0) {
                scheduleAfter(this->currentSendingDelay, this->platoonUnicast);
            }else {
//                delete this->platoonUnicast;
//                delete this->currentSendingPacket;
//                this->platoonUnicast = nullptr;
//                this->currentSendingPacket = nullptr;
            }
        }
    }

    void PlatooningProtocol::startPlatoonFormationAdvertisement(bool can_be_leader) {
        Enter_Method_Silent();
        this->can_be_leader = can_be_leader;
        this->platoonFormationAdvertisement = new cMessage("sendAdvertisement");
        scheduleAfter(uniform(0.001, 1), this->platoonFormationAdvertisement);
    }

    void PlatooningProtocol::stopPlatoonFormationAdvertisement() {
        Enter_Method_Silent();
        if(this->platoonFormationAdvertisement == nullptr) return; //already stopped, do nothing
        cancelAndDelete(this->platoonFormationAdvertisement);
        this->platoonFormationAdvertisement = nullptr;
    }

    void PlatooningProtocol::sendPacket(omnetpp::cPacket *pkt) {
        auto frame = new BaseFrame1609_4("", pkt->getKind());
        frame->setRecipientAddress(LAddress::L2BROADCAST());
        frame->setUserPriority(priority+1);
        frame->setChannelNumber(static_cast<int>(Channel::cch));

        frame->encapsulate(pkt);
        this->sendTo(frame, PlexeRadioInterfaces::ALL);
    }

    void PlatooningProtocol::sendPlatoonAdvertisementBeacon(bool can_be_leader) {
        auto msg = new PlatoonSearchCAM("MSG_ORGY_SEARCH");
        msg->setPlatooning_speed_max(this->mobility->getSpeed() * 1.05);
        msg->setPlatooning_speed_min(this->mobility->getSpeed() * 0.95);
        msg->setCan_be_leader(can_be_leader);
        msg->setKind(0x1234);
        msg->setLane(this->mobility->getVehicleCommandInterface()->getLaneIndex());
        msg->setAddress(this->mobility->getId());

        this->sendPacket(msg);
    }

    void PlatooningProtocol::startSendingUnicast(PlatoonUnicast *packet, long address, double delay, int retries) {
        Enter_Method_Silent();
        packet->setSenderAddress(this->mobility->getId());
        packet->setDestinationAddress(address);
        packet->setKind(0x5678);

        this->currentSendingDelay = delay;
        this->currentSendingPacket = packet->dup();
        this->currentSendingRetries = retries;

        this->platoonUnicast = new cMessage("Send unicast");
        scheduleAfter(uniform(0.001, 2*delay), this->platoonUnicast);
    }

    void PlatooningProtocol::stopSendingUnicast() {
        Enter_Method_Silent();
        if(this->platoonUnicast == nullptr) return; //already stopped, do nothing
        cancelAndDelete(this->platoonUnicast);
        this->platoonUnicast = nullptr;
        this->currentSendingPacket = nullptr;
        this->currentSendingRetries = 0;
    }

    PlatooningProtocol::PlatooningProtocol()
    {
    }

    PlatooningProtocol::~PlatooningProtocol()
    {
    }
}