//
// Created by user on 6/18/23.
//

#ifndef VNCD_PROJECT_MERGEMANEUVER_H
#define VNCD_PROJECT_MERGEMANEUVER_H


#include <plexe/apps/BaseApp.h>
#include <PlatoonSearchCAM_m.h>
#include <InternalListenTimeout_m.h>
#include <plexe/maneuver/JoinManeuver.h>
#include <plexe/apps/GeneralPlatooningApp.h>
#include "Protocol.h"

using namespace veins;

namespace plexe::vncd {

    class MergeManeuver : public MergeAtBack {
    public:
        explicit MergeManeuver(GeneralPlatooningApp * app): MergeAtBack(app) {};
    };
}

#endif //VNCD_PROJECT_MERGEMANEUVER_H
