//
// Created by user on 6/22/23.
//

#ifndef VNCD_PROJECT_IDLEMANEUVER_H
#define VNCD_PROJECT_IDLEMANEUVER_H

#include <plexe/maneuver/JoinManeuver.h>

namespace plexe::vncd{

    class IdleManeuver : public Maneuver {
    public:
        explicit IdleManeuver(GeneralPlatooningApp *app) : Maneuver(app) {};

        void startManeuver(const void* parameters) override{}
        void onManeuverMessage(const ManeuverMessage* mm) override{}
        void onPlatoonBeacon(const PlatooningBeacon* pb) override{}
        void onFailedTransmissionAttempt(const ManeuverMessage* mm) override{}
        void abortManeuver() override{}
    };
}

#endif //VNCD_PROJECT_IDLEMANEUVER_H
