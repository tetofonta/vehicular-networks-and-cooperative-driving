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

namespace plexe::vncd {

    void MyPlatooningApp::initialize(int stage) {
        BaseApp::initialize(stage);

        if(stage == 0 || stage > 1) return;

        this->protocol->registerApplication(PlatooningProtocol::BEACON_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));

        this->app_protocol = unique_ptr<PlatooningProtocol>(check_and_cast<PlatooningProtocol *>(this->protocol));
        this->app_protocol->startPlatoonAdvertisement();
        this->app_protocol->routePlatoonRequests(true);
    }

    void MyPlatooningApp::handleSelfMsg(omnetpp::cMessage *p_msg) {

    }

    void MyPlatooningApp::handleLowerMsg(omnetpp::cMessage *p_msg) {
        auto frame = unique_ptr<BaseFrame1609_4>(check_and_cast<BaseFrame1609_4*>(p_msg));
        cPacket * enc = frame->getEncapsulatedPacket();
        ASSERT2(enc, "received a BaseFrame1609_4 with nothing inside");

        EV_ERROR << frame << " " << enc << endl;
    }

}