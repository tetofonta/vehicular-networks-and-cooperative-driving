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

    class MergeManeuver : public MergeAtBack {
    private:
        cMessage * evt_ManeuverEnd;

    public:
        explicit MergeManeuver(GeneralPlatooningApp * app, cMessage * evt_maneuver): MergeAtBack(app){
            this->evt_ManeuverEnd = evt_maneuver;
        }

        void handleJoinFormationAck(const JoinFormationAck* msg) override;
        void handleJoinFormation(const JoinFormation* msg) override;
    };
}

#endif //VNCD_PROJECT_MERGEMANEUVER_H
