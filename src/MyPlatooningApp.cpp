#include <MyPlatooningApp.h>
#include "plexe/apps/GeneralPlatooningApp.h"

#include "plexe/protocols/BaseProtocol.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"
#include "veins/base/utils/FindModule.h"
#include "plexe/scenarios/ManeuverScenario.h"
#include "maneuvers/MergeManeuver.h"
#include "maneuvers/IdleManeuver.h"

template<typename T>
unique_ptr<T> frame_cast(BaseFrame1609_4 *ptr) {
    if (T *p = dynamic_cast<T *>(ptr->getEncapsulatedPacket())) return unique_ptr<T>((T *) ptr->decapsulate());
    return unique_ptr<T>(nullptr);
}

namespace plexe::vncd {

    void MyPlatooningApp::initialize(int stage) {
        ApplicationAdapter::initialize(stage);
        if (stage == 0 || stage > 1) return;

        //Beacon_type altready registred by BaseApp
        this->protocol->registerApplication(PLATOON_NEGOTIATION_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"),
                                            gate("lowerControlIn"), gate("lowerControlOut"));
        this->app_protocol = check_and_cast<PlatooningProtocol *>(this->protocol);

        this->app_protocol->startPlatoonAdvertisement();
        this->app_protocol->routePlatoonRequests(true);
        this->app_protocol->setPlatoonAccepting(true);
        this->activeManeuver = new IdleManeuver(this);

        this->original_speed = this->traciVehicle->getSpeed();

        this->plexeTraciVehicle->setLaneChangeMode(DRIVER_CHOICE);
        this->plexeTraciVehicle->setCruiseControlDesiredSpeed(this->original_speed);

        this->evt_ManeuverEnd = make_unique<cMessage>("Maneuver End");
        this->state = APP_LEADER_IDLE;
    }

    void MyPlatooningApp::sendUnicast(omnetpp::cPacket *msg, int destination) {
        this->app_protocol->sendUnicast(msg, destination);
    }

    void MyPlatooningApp::handleSelfMsg(omnetpp::cMessage *p_msg) {
        if(p_msg == this->evt_ManeuverEnd.get()){
            if(this->getPlatoonRole() == PlatoonRole::LEADER) {
                this->state = APP_LEADER_IDLE;
                this->app_protocol->startPlatoonAdvertisement();
                this->app_protocol->routePlatoonRequests(true);
                this->app_protocol->setPlatoonAccepting(true);
            }
            else this->state = APP_FOLLOWER_IDLE;
            return;
        }

        auto msg = unique_ptr<cMessage>(p_msg);
        ApplicationAdapter::handleSelfMsg(msg.release());
    }

    void MyPlatooningApp::handleLowerMsg(omnetpp::cMessage *p_msg) {
        auto frame = unique_ptr<BaseFrame1609_4>(check_and_cast<BaseFrame1609_4 *>(p_msg));
        auto enc = frame->getEncapsulatedPacket();
        ASSERT2(enc, "received a BaseFrame1609_4 with nothing inside");

        if(enc->getKind() != PLATOON_NEGOTIATION_TYPE) return ApplicationAdapter::handleLowerMsg(frame.release());

        switch (this->state) {
            case APP_LEADER_IDLE: {
                if (auto adv = frame_cast<PlatoonAdvertiseBeacon>(frame.get())) {
                    if(!this->isLeader(adv->getCoords())){
                        if(this->someoneInFront()) break; //abort, incompatible.
                        this->app_protocol->stopPlatoonAdvertisement();
                    }

                    this->app_protocol->sendUnicast(
                            this->buildPlatoonCreateRequest().get(),
                            adv->getSenderAddress()
                    );

                    this->app_protocol->routePlatoonRequests(false);
                    this->state = APP_NEGOTIATING;
                    EV << "RECEIVED BEACON, GOING TO APP_NEGOTIATING " << adv->getPlatoon_id() << endl;
                    return;
                }
                if (auto req = frame_cast<PlatoonCreateRequest>(frame.get())) {
                    if(!this->isLeader(req->getCoord())){
                        this->app_protocol->stopPlatoonAdvertisement();
                    }
                    this->app_protocol->sendUnicast(
                            this->buildPlatoonCreateRequestACK(req.get()).get(),
                            req->getSenderAddress()
                    );

                    if(this->isLeader(req->getCoord()) || !this->someoneInFront()){
                        this->startMergeManeuver(req->getPlatoonId(), req->getLeaderId(), this->isLeader(req->getCoord()));
                        this->app_protocol->routePlatoonRequests(false);
                        this->app_protocol->setPlatoonAccepting(false);
                        this->state = APP_MANEUVERING;
                         EV << "RECEIVED REQUEST, GOING TO APP_MANEUVERING" << req->getPlatoonId() << endl;

                        return;
                    }

                    this->state = APP_LEADER_IDLE;
                    this->app_protocol->startPlatoonAdvertisement();
                    this->app_protocol->routePlatoonRequests(true);
                    this->app_protocol->setPlatoonAccepting(true);
                    EV << "RECEIVED REQUEST, ABORTING BECAUSE SOMEONE IS IN FRONT APP_LEADER_IDLE" << req->getPlatoonId() << endl;

                }
                break;
            }
            case APP_NEGOTIATING: {
                if (auto ack = frame_cast<PlatoonCreateRequestACK>(frame.get())) {
                    if(ack->getAccepted()){
                        this->app_protocol->setPlatoonAccepting(false);
                        this->startMergeManeuver(ack->getPlatoonId(), ack->getLeaderId(), this->isLeader(ack->getCoord()));
                        this->state = APP_MANEUVERING;
                        EV << "ACK ACCEPTED, APP_MANEUVERING" << ack->getPlatoonId() << this->isLeader(ack->getCoord()) << endl;

                        return;
                    }
                    this->state = APP_LEADER_IDLE;
                    this->app_protocol->routePlatoonRequests(true);
                    this->app_protocol->setPlatoonAccepting(true);
                    this->app_protocol->startPlatoonAdvertisement();
                    EV << "ACK REFUSED, APP_LEADER_IDLE" << ack->getPlatoonId() << endl;


                }
            }
            case APP_FOLLOWER_IDLE:
            case APP_MANEUVERING:
                break;
        }
    }

    unique_ptr<PlatoonCreateRequest> MyPlatooningApp::buildPlatoonCreateRequest() {
        auto pkt = std::make_unique<PlatoonCreateRequest>("Platoon Create Request", PLATOON_NEGOTIATION_TYPE);
        pkt->setLeaderId(this->positionHelper->getId());
        pkt->setPlatoonId(this->positionHelper->getPlatoonId());
        pkt->setCoord(this->mobility->getPositionAt(simTime()).x);
        return pkt;
    }

    unique_ptr<PlatoonCreateRequestACK> MyPlatooningApp::buildPlatoonCreateRequestACK(PlatoonCreateRequest *req) {
        auto pkt = std::make_unique<PlatoonCreateRequestACK>("Platoon Create Request ACK", PLATOON_NEGOTIATION_TYPE);
        pkt->setLeaderId(this->isLeader(req->getCoord()) ? this->positionHelper->getId() : -1);
        pkt->setPlatoonId(this->isLeader(req->getCoord()) ? this->positionHelper->getPlatoonId() : -1);
        pkt->setCoord(this->mobility->getPositionAt(simTime()).x);
        pkt->setAccepted(this->isLeader(req->getCoord()) || !this->someoneInFront());
        return pkt;
    }

    bool MyPlatooningApp::isLeader(double coord) {
        return this->mobility->getPositionAt(simTime()).x > coord;
    }

    MyPlatooningApp::~MyPlatooningApp(){
        if(this->evt_ManeuverEnd->isScheduled()) cancelEvent(this->evt_ManeuverEnd.get());
    }

    void MyPlatooningApp::startMergeManeuver(int platoon_id, int leader_id, bool leader){
        this->setActiveManeuver(std::make_unique<MergeManeuver>(this, this->evt_ManeuverEnd.get()));
        if (!leader) {
            JoinManeuverParameters params{
                    .platoonId = platoon_id,
                    .leaderId = leader_id,
                    .position = -1,
            };
            this->activeManeuver->startManeuver(&params);
        }
    }

    bool MyPlatooningApp::someoneInFront() {
//        return false;
        //prima di richiedere, e se devo andare avanti, controlla che non ci sia nessuno in mezzo alle palle
        double radar_distance = 1000, rel_speed = 0;
        this->plexeTraciVehicle->getRadarMeasurements(radar_distance, rel_speed);
        return radar_distance < 250 && this->app_protocol->front_distance < radar_distance;
    }
}