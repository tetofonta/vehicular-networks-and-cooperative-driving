#include <Protocol.h>

#include "plexe/protocols/BaseProtocol.h"
#include <packets/platoonAdvertisementBeacon_m.h>
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

    void PlatooningProtocol::initialize(int stage) {
        BaseProtocol::initialize(stage);

        if(stage > 0) return;

        this->platoonAdvertiseBeaconInterval = SimTime(par("platoonAdvertisementInterval").doubleValue());
        this->evt_SendPlatoonAdvertiseBeacon = make_unique<cMessage>("Send Advertise Beacon");
        this->evt_SendPlatooonBeacon = make_unique<cMessage>("Send Platoon Beacon");
        this->platooningFormationSpeedRange = par("platooningFormationSpeedRange").doubleValue();
        this->maxPlatoonSize = par("maxPlatoonSize").intValue();
        scheduleAfter(uniform(0.001, this->beaconingInterval), evt_SendPlatooonBeacon.get());
    }

    void PlatooningProtocol::handleSelfMsg(cMessage *p_msg) {
        if(this->evt_SendPlatoonAdvertiseBeacon.get() == p_msg){
            sendPlatoonAdvertisementBeacon();
            scheduleAfter(this->platoonAdvertiseBeaconInterval, this->evt_SendPlatoonAdvertiseBeacon.get());
            return;
        }

        if(this->evt_SendPlatooonBeacon.get() == p_msg){
            this->sendPlatooningMessage(-1);
            scheduleAfter(this->beaconingInterval, this->evt_SendPlatooonBeacon.get());
            return;
        }

        if(auto interval = dynamic_cast<PlatoonAdvertisementListenTimeout *>(p_msg)){
            EV << "Platoon " << interval->getPlatoon() << " no heard for a long time..." << endl;
            this->events.erase(interval->getPlatoon());
        }
    }

    unique_ptr<BaseFrame1609_4> PlatooningProtocol::encapsulate(int destinationAddress, cPacket * pkt){
        auto wsm = veins::make_unique<BaseFrame1609_4>("", pkt->getKind());
        wsm->setRecipientAddress(destinationAddress);
        wsm->setChannelNumber(static_cast<int>(Channel::cch));
        wsm->setUserPriority(priority);
        wsm->encapsulate(pkt);
        return wsm;
    }
    unique_ptr<BaseFrame1609_4> PlatooningProtocol::buildPacket(int destinationAddress, cPacket *pkt) {
        if(auto unicast_packet = dynamic_cast<PacketHeader *>(pkt))
            unicast_packet->setSenderAddress(this->positionHelper->getLeaderId());
        return this->encapsulate(destinationAddress, pkt);
    }

    void PlatooningProtocol::sendBroadcast(cPacket * pkt){
        Enter_Method_Silent();
        auto frame = this->buildPacket(-1, pkt->dup());
        this->sendTo(frame.release(), PlexeRadioInterfaces::ALL);
    }
    void PlatooningProtocol::sendUnicast(cPacket * pkt, int destinationAddress){
        Enter_Method_Silent();
        auto frame = this->buildPacket(destinationAddress, pkt->dup());
        this->sendTo(frame.release(), PlexeRadioInterfaces::VEINS_11P);
    }

    void PlatooningProtocol::startPlatoonAdvertisement(){
        Enter_Method_Silent();
        if(this->evt_SendPlatoonAdvertiseBeacon->isScheduled()) return;
        scheduleAfter(uniform(0.01, 2*this->platoonAdvertiseBeaconInterval), this->evt_SendPlatoonAdvertiseBeacon.get());
    }
    void PlatooningProtocol::stopPlatoonAdvertisement(){
        Enter_Method_Silent();
        if(!this->evt_SendPlatoonAdvertiseBeacon->isScheduled()) return;
        cancelEvent(this->evt_SendPlatoonAdvertiseBeacon.get());
    }
    unique_ptr<PlatoonAdvertiseBeacon> PlatooningProtocol::createPlatoonAdvertisementBeacon(){
        auto beacon = make_unique<PlatoonAdvertiseBeacon>( "PlatoonAdvertiseBeacon", PLATOON_NEGOTIATION_TYPE);
        beacon->setLane(this->traciVehicle->getLaneIndex());
        beacon->setPlatoon_id(this->positionHelper->getPlatoonId());
        beacon->setPlatoon_speed(this->mobility->getSpeed());
        beacon->setPlatoon_size(this->positionHelper->getPlatoonSize());
        return beacon;
    }
    void PlatooningProtocol::sendPlatoonAdvertisementBeacon() {
        this->sendBroadcast(this->createPlatoonAdvertisementBeacon().release());
    }

    void PlatooningProtocol::routePlatoonRequests(bool state){
        this->doRoutePlatoonRequests = state;
    }
    bool PlatooningProtocol::isPlatoonCompatible(PlatoonAdvertiseBeacon *pkt){
        if(pkt->getLane() != this->traciVehicle->getLaneIndex()) return false;

        auto max_speed_delta = std::min(pkt->getPlatoon_speed(), this->traciVehicle->getSpeed()) * this->platooningFormationSpeedRange;
        if(std::abs(pkt->getPlatoon_speed() - this->traciVehicle->getSpeed()) < max_speed_delta) return false;

        if(this->maxPlatoonSize < pkt->getPlatoon_size() + this->positionHelper->getPlatoonSize()) return false;

        return true;
    }
    bool PlatooningProtocol::handlePlatoonAdvertisement(PlatoonAdvertiseBeacon *pkt) {
        if(!this->doRoutePlatoonRequests) return false;
        if(!this->isPlatoonCompatible(pkt)){
            return false;
            EV_ERROR << "PLATOON INCOMPATIBLE :(" << endl;
        }

        if(!this->events.contains(pkt->getPlatoon_id())){
            EV << "Heard message " << pkt << " once" << endl;
            auto interval = make_unique<PlatoonAdvertisementListenTimeout>("Timeout platoon");
            interval->setPlatoon(pkt->getPlatoon_id());
            interval->setCount(1);
            scheduleAfter(5, interval.get());
            this->events[pkt->getPlatoon_id()] = std::move(interval);
            return false;
        }

        auto interval = std::move(this->events[pkt->getPlatoon_id()]);
        cancelEvent(interval.get());
        interval->setCount(interval->getCount() + 1);
        EV << "Heard message " << pkt << " " << (int) interval->getCount() << " times" << endl;

        if(interval->getCount() >= 3){
            this->events.erase(pkt->getPlatoon_id());
            EV << "routing" << pkt  << endl;
            return true;
        }

        scheduleAfter(5, interval.get());
        this->events[pkt->getPlatoon_id()] = std::move(interval);
        return false;
    }

    PlatooningProtocol::PlatooningProtocol() {}
    PlatooningProtocol::~PlatooningProtocol() {}

    void PlatooningProtocol::handleLowerMsg(cMessage * p_msg){
        auto frame = unique_ptr<BaseFrame1609_4>(check_and_cast<BaseFrame1609_4*>(p_msg));
        cPacket * enc = frame->getEncapsulatedPacket();
        ASSERT2(enc, "received a BaseFrame1609_4 with nothing inside");

        if(auto platoon_beacon = dynamic_cast<PlatoonAdvertiseBeacon*>(enc))
            if (!handlePlatoonAdvertisement(platoon_beacon)) return;

        BaseProtocol::handleLowerMsg(frame.release());
    }
}