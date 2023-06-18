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
#include "MergeManeuver.h"
#include <PlatoonCreateRequest_m.h>
#include <PlatoonCreateAnswer_m.h>
#include <PlatoonSearchCAM_m.h>

namespace plexe::vncd {

    void MyPlatooningApp::initialize(int stage) {
        BaseApp::initialize(stage);

        switch (stage) {
            case 0:
                break;
            case 1:
                protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"),
                                                 gate("lowerControlIn"), gate("lowerControlOut"));
                protocol->registerApplication(0x1234, gate("lowerLayerIn"), gate("lowerLayerOut"),
                                              gate("lowerControlIn"), gate("lowerControlOut"));
                protocol->registerApplication(0x5678, gate("lowerLayerIn"), gate("lowerLayerOut"),
                                              gate("lowerControlIn"), gate("lowerControlOut"));

                findHost()->subscribe(Mac1609_4::sigRetriesExceeded, this);

                this->app_protocol = (PlatooningProtocol *) this->protocol;
                this->app_protocol->startPlatoonFormationAdvertisement();
                this->scenario = FindModule<BaseScenario *>::findSubModule(omnetpp::cModule::getParentModule());

                this->mergeManeuver = new MergeManeuver(this);
                this->joinManeuver = new JoinAtBack(this);

                break;
            default:
                break;
        }
    }


    void MyPlatooningApp::handleSelfMsg(omnetpp::cMessage *p_msg) {
        if(this->mergeManeuver) this->mergeManeuver->handleSelfMsg(p_msg);
        if(this->joinManeuver) this->joinManeuver->handleSelfMsg(p_msg);
        GeneralPlatooningApp::handleSelfMsg(p_msg);

        auto internalTimeout = dynamic_cast<InternalListenTimeout *>(p_msg);
        if(internalTimeout != NULL){
            auto msg = std::make_unique<InternalListenTimeout>(*internalTimeout);
            this->events.erase(msg->getAddress());
        }

        auto msg = std::make_unique<cMessage>(*p_msg);


    }

    void MyPlatooningApp::handleLowerMsg(omnetpp::cMessage *msg) {
        auto frame = make_unique<BaseFrame1609_4>(*check_and_cast<BaseFrame1609_4 *>(msg));
        cPacket *enc = frame->getEncapsulatedPacket();
        ASSERT2(enc, "received a BaseFrame1609_4s with nothing inside");

        switch (enc->getKind()) {
            case 0x1234: {
                auto pkt = check_and_cast<PlatoonSearchCAM *>(frame->decapsulate());
                EV << "PLATOON SEARCH CAM RECEIVED: SPEEDS: " << this->mobility->getSpeed() << " " << pkt->getPlatooning_speed_min() << " " << pkt->getPlatooning_speed_max() << endl;
                if(!this->isPlatooningCompatible(pkt)) break;
                EV << "PLATOON IS COMPATIBLE" << endl;

                int counts = 1;
                auto msg = new InternalListenTimeout();
                msg->setAddress(pkt->getAddress());
                if(this->events.contains(pkt->getAddress())){
                    cancelAndDelete( std::get<1>(this->events[pkt->getAddress()]));
                    counts = std::get<0>(this->events[pkt->getAddress()]) + 1;
                    EV << "HEARD MESSAGE " << counts << " TIMES" << endl;
                }

                if(counts >= 3){
                    EV << "HEARD MESSAGE TOO MANY TIMES" << endl;
                    this->events.erase(pkt->getAddress());
                    if (this->negotiationAddress == -1) {
                        EV << "STARTING NEGOTIATION" << endl;
                        this->app_protocol->stopPlatoonFormationAdvertisement();
                        auto response = new PlatoonCreateRequest();
                        response->setType(PLATOON_CREATE_REQUEST);
                        response->setCoordinate(this->mobility->getPositionAt(simTime()).x);
                        response->setPlatooning_speed_min(pkt->getPlatooning_speed_min());
                        response->setPlatooning_speed_max(pkt->getPlatooning_speed_max());
                        response->setLane(pkt->getLane());

                        response->setPlatoon_id(this->positionHelper->getPlatoonId());
                        response->setLeader_id(this->positionHelper->getLeaderId());

                        this->negotiationAddress = pkt->getAddress();
                        this->app_protocol->startSendingUnicast(response, pkt->getAddress(), 0.1, 20);
                        EV << "REQ SENT" << endl;
                        delete response;
                    }
                } else {
                    this->events[pkt->getAddress()] = std::make_tuple(counts, msg);
                    scheduleAfter(5, msg);
                }
                break;
            }
            case 0x5678: {
                if(isInManeuver()) break;
                auto pkt = check_and_cast<PlatoonUnicast *>(frame->decapsulate());
                if(pkt->getDestinationAddress() != this->mobility->getId()) break;
                switch (pkt->getType()) {
                    case PLATOON_CREATE_REQUEST: {
                        auto data = check_and_cast<PlatoonCreateRequest *>(pkt);
                        auto response = new PlatoonCreateAnswer();
                        if ((this->negotiationAddress == -1 || this->negotiationAddress == data->getSenderAddress()) &&
                            std::abs(this->mobility->getPositionAt(simTime()).x - data->getCoordinate()) >= 5) {
                            this->app_protocol->stopPlatoonFormationAdvertisement();
                            this->negotiationAddress = data->getSenderAddress();
                            response->setType(PLATOON_CREATE_ANSWER);
                            response->setAccepted(true);
                            response->setCoordinate(this->mobility->getPositionAt(simTime()).x);
                            response->setPlatooning_speed_min(data->getPlatooning_speed_min());
                            response->setPlatooning_speed_max(data->getPlatooning_speed_max());
                            response->setLane(data->getLane());
                            this->isLeader = this->mobility->getPositionAt(simTime()).x >
                                             data->getCoordinate(); //todo use coordinates and heading

                            if(this->isLeader){
                                response->setPlatoon_id(this->positionHelper->getPlatoonId());
                                response->setLeader_id(this->positionHelper->getLeaderId());
                                this->traciVehicle->setColor(TraCIColor(255, 255, 0, 255));
                                this->traciVehicle->setMaxSpeed(data->getPlatooning_speed_min());
                                this->plexeTraciVehicle->setActiveController(DRIVER);
                                this->setPlatoonRole(PlatoonRole::LEADER);
                            } else {
                                response->setPlatoon_id(-1);
                                response->setLeader_id(-1);
                                this->traciVehicle->setMaxSpeed(data->getPlatooning_speed_max());
                                this->traciVehicle->setColor(TraCIColor(255, 0, 255, 255));
                                this->setPlatoonRole(PlatoonRole::LEADER);
                                this->startMergeManeuver(data->getPlatoon_id(), data->getLeader_id(), -1);
                                EV << this->mobility->getId() << " STARTED MANEUVER" << endl;
                            }

                            this->app_protocol->startSendingUnicast(response, data->getSenderAddress(), 0.1, 4);
                        } else {
                            response->setType(PLATOON_CREATE_ANSWER);
                            response->setAccepted(false);
                            this->app_protocol->startSendingUnicast(response, data->getSenderAddress(), 0.1, 4);
                        }
                        delete response;
                        break;
                    }
                    case PLATOON_CREATE_ANSWER: {
                        auto data = check_and_cast<PlatoonCreateAnswer *>(pkt);
                        if (data->getAccepted()) {
                            this->isLeader = this->mobility->getPositionAt(simTime()).x >
                                             data->getCoordinate(); //todo use coordinates and heading
                            this->app_protocol->stopSendingUnicast();

                            if(this->isLeader){
                                this->traciVehicle->setColor(TraCIColor(255, 255, 0, 255));
                                this->traciVehicle->setMaxSpeed(data->getPlatooning_speed_min());
                                this->setPlatoonRole(PlatoonRole::LEADER);
                            } else {
                                this->traciVehicle->setMaxSpeed(data->getPlatooning_speed_max());
                                this->traciVehicle->setColor(TraCIColor(255, 0, 255, 255));
                                this->setPlatoonRole(PlatoonRole::LEADER);
                                this->startMergeManeuver(data->getPlatoon_id() != -1 ? data->getPlatoon_id() : this->positionHelper->getPlatoonId(), data->getLeader_id() != -1 ? data->getLeader_id() : this->positionHelper->getLeaderId(), -1);
                                EV << this->mobility->getId() << " STARTED MANEUVER" << endl;
                            }
                        } else {
                            this->app_protocol->stopSendingUnicast();
                            this->negotiationAddress = -1;
                            this->app_protocol->startPlatoonFormationAdvertisement();
                        }
                    }
                }
                break;
            }
            default:
                GeneralPlatooningApp::handleLowerMsg(frame.release());
        }
    }

    bool MyPlatooningApp::isPlatooningCompatible(PlatoonSearchCAM *pkt) {
        if (this->mobility->getSpeed() < pkt->getPlatooning_speed_min()) return false;
        if (this->mobility->getSpeed() > pkt->getPlatooning_speed_max()) return false;
        return true;
    }

    void MyPlatooningApp::onPlatoonBeacon(const PlatooningBeacon * pb){
//        GeneralPlatooningApp::onPlatoonBeacon(pb); //bug because join maneuver e mergeManeuver possono non essere definiti.
        if(this->mergeManeuver) this->mergeManeuver->onPlatoonBeacon(pb);
        if(this->joinManeuver) this->joinManeuver->onPlatoonBeacon(pb);
        BaseApp::onPlatoonBeacon(pb);
    }

}