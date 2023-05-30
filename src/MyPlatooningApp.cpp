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

        switch(stage){
            case 0:
                break;
            case 1:
                protocol->registerApplication(0x1234, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
                protocol->registerApplication(0x5678, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
                this->app_protocol = (PlatooningProtocol *) this->protocol;
                this->can_be_leader = uniform(0, 1) > 0.5;
                this->app_protocol->startPlatoonFormationAdvertisement(can_be_leader);
                break;
            default:
                break;
        }
    }


    void MyPlatooningApp::handleSelfMsg(omnetpp::cMessage *p_msg) {
        BaseApp::handleSelfMsg(p_msg);
        auto msg = std::make_unique<cMessage>(*p_msg);
    }

    void MyPlatooningApp::handleLowerMsg(omnetpp::cMessage *msg) {
        auto frame = make_unique<BaseFrame1609_4>(*check_and_cast<BaseFrame1609_4 *>(msg));
        cPacket *enc = frame->getEncapsulatedPacket();
        ASSERT2(enc, "received a BaseFrame1609_4s with nothing inside");

        switch (enc->getKind()) {
            case 0x1234: {
                auto pkt = check_and_cast<PlatoonSearchCAM *>(frame->decapsulate());
                if (this->isPlatooningCompatible(pkt) && this->negotiationAddress == -1){ //todo
                    this->app_protocol->stopPlatoonFormationAdvertisement();
                    auto response = new PlatoonCreateRequest();
                    response->setType(PLATOON_CREATE_REQUEST);
                    this->leaderExtraction = this->can_be_leader ? (int) uniform(0, 1024) : -1;
                    response->setLeaderExtraction(this->leaderExtraction);
                    this->negotiationAddress = pkt->getAddress();
                    this->app_protocol->startSendingUnicast(response, pkt->getAddress(), 0.1, 20);
                    delete response;
                }
                break;
            }
            case 0x5678: {
                auto pkt = check_and_cast<PlatoonUnicast *>(frame->decapsulate());
                switch(pkt->getType()){
                    case PLATOON_CREATE_REQUEST: {
                        auto data = check_and_cast<PlatoonCreateRequest *>(pkt);
                        auto response = new PlatoonCreateAnswer();
                        if(this->negotiationAddress == -1 || this->negotiationAddress == data->getSenderAddress()){
                            this->app_protocol->stopPlatoonFormationAdvertisement();
                            if(this->negotiationAddress == -1){
                                this->leaderExtraction = this->can_be_leader ? (int) uniform(0, 1024) : -1;
                            }
                            this->negotiationAddress = data->getSenderAddress();
                            response->setType(PLATOON_CREATE_ANSWER);
                            response->setLeaderExtraction(this->leaderExtraction);
                            response->setAccepted(true);
                            this->isLeader = response->getLeaderExtraction() > data->getLeaderExtraction();
                            this->app_protocol->startSendingUnicast(response, data->getSenderAddress(), 0.1, 4);
                        } else {
                            response->setType(PLATOON_CREATE_ANSWER);
                            response->setAccepted(false);
                            this->app_protocol->startSendingUnicast(response, data->getSenderAddress(), 0.1, 4);
                        }
                        break;
                    }
                    case PLATOON_CREATE_ANSWER: {
                        auto data = check_and_cast<PlatoonCreateAnswer *>(pkt);
                        if(data->getAccepted()){
                            this->isLeader = this->leaderExtraction > data->getLeaderExtraction();
                            this->app_protocol->stopSendingUnicast();

                            

                        } else {
                            this->app_protocol->stopSendingUnicast();
                            this->negotiationAddress = -1;
                            this->app_protocol->startPlatoonFormationAdvertisement(this->can_be_leader);
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
        if(this->mobility->getSpeed() < pkt->getPlatooning_speed_min()) return false;
        if(this->mobility->getSpeed() > pkt->getPlatooning_speed_max()) return false;
        if(!(this->can_be_leader || pkt->getCan_be_leader())) return false;
        return true;
    }


}