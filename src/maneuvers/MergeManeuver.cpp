//
// Created by user on 6/18/23.
//
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

#include "maneuvers/MergeManeuver.h"

namespace plexe::vncd{

    void MergeManeuver::handleJoinFormationAck(const JoinFormationAck *msg) {
        MergeAtBack::handleJoinFormationAck(msg);
        this->app->scheduleAfter(0, this->evt_ManeuverEnd);
    }

    void MergeManeuver::handleJoinFormation(const JoinFormation *msg) {
        MergeAtBack::handleJoinFormation(msg);
        this->app->scheduleAfter(0, this->evt_ManeuverEnd);
    }

}