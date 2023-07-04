//
// Created by user on 6/18/23.
//

#ifndef VNCD_PROJECT_MERGEMANEUVER_H
#define VNCD_PROJECT_MERGEMANEUVER_H


#include <plexe/apps/BaseApp.h>
#include <plexe/maneuver/JoinManeuver.h>
#include <plexe/apps/GeneralPlatooningApp.h>
#include "Protocol.h"

using namespace veins;

namespace plexe::vncd {

    struct MergeManeuverParameters {
        JoinManeuverParameters params;
        double distance;
    };

    class MergeManeuver : public MergeAtBack {
    private:
        cMessage * evt_ManeuverEnd;
        SimTime start_time;
        double startDistance;
        simsignal_t maneuverSpeedSignal;
        simsignal_t platoonSizeSignal;

    public:
        explicit MergeManeuver(GeneralPlatooningApp * app, cMessage * evt_maneuver, simsignal_t maneuverSpeedSignal, simsignal_t platoonSizeSignal): MergeAtBack(app){
            this->evt_ManeuverEnd = evt_maneuver;
            this->maneuverSpeedSignal = maneuverSpeedSignal;
            this->platoonSizeSignal = platoonSizeSignal;
        }

        void handleJoinFormationAck(const JoinFormationAck* msg) override;
        void handleJoinFormation(const JoinFormation* msg) override;
        void startManeuver(const void *parameters) override;
    };
}

#endif //VNCD_PROJECT_MERGEMANEUVER_H
