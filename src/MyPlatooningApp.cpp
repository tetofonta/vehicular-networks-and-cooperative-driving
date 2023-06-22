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

template<typename T, typename U>
unique_ptr<T> unique_dynamic_cast(U *ptr) {
    if (T *p = dynamic_cast<T *>(ptr)) return unique_ptr<T>((T *) ptr);
    return nullptr;
}

namespace plexe::vncd {

    void MyPlatooningApp::initialize(int stage) {
        ApplicationAdapter::initialize(stage);
        if (stage == 0 || stage > 1) return;

        //Beacon_type altready registred by BaseApp
        this->app_protocol = unique_ptr<PlatooningProtocol>(check_and_cast<PlatooningProtocol *>(this->protocol));

        if (this->positionHelper->getId() == 0) {
            this->app_protocol->startPlatoonAdvertisement();
        }
        this->app_protocol->routePlatoonRequests(true);
        this->activeManeuver = new MergeAtBack(this);
    }

    void MyPlatooningApp::sendUnicast(omnetpp::cPacket *msg, int destination) {
        this->app_protocol->sendUnicast(msg, destination);
    }

    void MyPlatooningApp::handleSelfMsg(omnetpp::cMessage *p_msg) {
        auto msg = unique_ptr<cMessage>(p_msg);
        ApplicationAdapter::handleSelfMsg(msg.release());
    }

    void MyPlatooningApp::handleLowerMsg(omnetpp::cMessage *p_msg) {
        auto frame = unique_ptr<BaseFrame1609_4>(check_and_cast<BaseFrame1609_4 *>(p_msg));
        auto enc = frame->getEncapsulatedPacket();
        ASSERT2(enc, "received a BaseFrame1609_4 with nothing inside");

        switch (enc->getKind()) {
            case 12345: { //BEACON_TYPE
                if (auto platoon_advertisement = dynamic_cast<PlatoonAdvertiseBeacon *>(enc)) {
                    frame->decapsulate();
                    if (getPlatoonRole() != PlatoonRole::LEADER) return;
                    if (isInManeuver()) return;

                    JoinManeuverParameters params{
                            .platoonId = platoon_advertisement->getPlatoon_id(),
                            .leaderId = (int) platoon_advertisement->getSenderAddress(),
                            .position = -1,
                    };
                    this->activeManeuver->startManeuver(&params);
                }
                break;
            }
        }

        if(frame->getEncapsulatedPacket()) ApplicationAdapter::handleLowerMsg(frame.release());
    }

}