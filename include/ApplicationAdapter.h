//
// Created by stefano on 22/06/23.
//

#ifndef VNCD_PROJECT_APPLICATIONADAPTER_H
#define VNCD_PROJECT_APPLICATIONADAPTER_H

#include <plexe/apps/BaseApp.h>
#include <plexe/maneuver/JoinManeuver.h>
#include "Protocol.h"
#include <plexe/apps/GeneralPlatooningApp.h>

namespace plexe::vncd {
    class ApplicationAdapter : public GeneralPlatooningApp {
    protected:
    public:
        ApplicationAdapter(): GeneralPlatooningApp(){};
        ~ApplicationAdapter() override = default;
        void initialize(int stage) override;
        void startJoinManeuver(int platoonId, int leaderId, int position);
        void startMergeManeuver(int platoonId, int leaderId, int position);
        void onPlatoonBeacon(const PlatooningBeacon * pb) override;
        void onManeuverMessage(const ManeuverMessage * pb);
        void receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details);

    protected:
        void handleSelfMsg(cMessage* msg) override;
    };

}


#endif //VNCD_PROJECT_APPLICATIONADAPTER_H
