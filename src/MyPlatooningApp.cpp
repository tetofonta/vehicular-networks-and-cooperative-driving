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
                protocol->registerApplication(0x1234, gate("lowerLayerIn"), gate("lowerLayerOut"),
                                              gate("lowerControlIn"), gate("lowerControlOut"));
                protocol->registerApplication(0x5678, gate("lowerLayerIn"), gate("lowerLayerOut"),
                                              gate("lowerControlIn"), gate("lowerControlOut"));
                this->app_protocol = (PlatooningProtocol *) this->protocol;
//                this->can_be_leader = uniform(0, 1) > 0.5;
                this->app_protocol->startPlatoonFormationAdvertisement();
                break;
            default:
                break;
        }
    }


    void MyPlatooningApp::handleSelfMsg(omnetpp::cMessage *p_msg) {
        BaseApp::handleSelfMsg(p_msg);

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

                int counts = 1;
                auto msg = new InternalListenTimeout();
                msg->setAddress(pkt->getAddress());
                if(this->events.contains(pkt->getAddress())){
                    cancelAndDelete( std::get<1>(this->events[pkt->getAddress()]));
                    counts = std::get<0>(this->events[pkt->getAddress()]) + 1;
                }

                if(counts >= 3){
                    this->events.erase(pkt->getAddress());
                    if (this->isPlatooningCompatible(pkt) && this->negotiationAddress == -1) { //todo
                        this->app_protocol->stopPlatoonFormationAdvertisement();
                        auto response = new PlatoonCreateRequest();
                        response->setType(PLATOON_CREATE_REQUEST);
//                    response->setLeaderExtraction(this->leaderExtraction);
                        response->setCoordinate(this->mobility->getPositionAt(simTime()).x);
                        this->negotiationAddress = pkt->getAddress();
                        this->app_protocol->startSendingUnicast(response, pkt->getAddress(), 0.1, 20);
                        delete response;
                    }
                } else {
                    this->events[pkt->getAddress()] = std::make_tuple(counts, msg);
                    scheduleAfter(5, msg);
                }
                break;
            }
            case 0x5678: {
                auto pkt = check_and_cast<PlatoonUnicast *>(frame->decapsulate());
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
                            this->isLeader = this->mobility->getPositionAt(simTime()).x >
                                             data->getCoordinate(); //todo use coordinates and heading
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

                            //WE ARE GOOD TO GO!

                            //leader this->isLeader
                            //speed: min(max_speed(a), max_speed(b))
                            //lane: min(lane(a), lane(b))

                            //se sono il leader e sono dietro -> overtake maneuver -> go to negotiated lane (rightmost)
                            //se non sono il leader -> sta fermo e non rompere
                            //se sono il leader e sono davanti -> sta fermo e non rompere
                            //leader send platoon_join_allowed
                            //follower -> join platoon maneuver

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
                BaseApp::handleLowerMsg(frame.release());
        }
    }

    bool MyPlatooningApp::isPlatooningCompatible(PlatoonSearchCAM *pkt) {
        if (this->mobility->getSpeed() < pkt->getPlatooning_speed_min()) return false;
        if (this->mobility->getSpeed() > pkt->getPlatooning_speed_max()) return false;
//        if (!(this->can_be_leader || pkt->getCan_be_leader())) return false;
        return true;
    }


}