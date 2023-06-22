//
// Created by user on 5/11/23.
//

#ifndef VNCD_PROJECT_MYPLATOONINGAPP_H
#define VNCD_PROJECT_MYPLATOONINGAPP_H

#include <plexe/apps/BaseApp.h>
#include <plexe/maneuver/JoinManeuver.h>
#include <plexe/apps/GeneralPlatooningApp.h>
#include "Protocol.h"
#include "ApplicationAdapter.h"
#include "packets/platoonCreateRequest_m.h"
#include "packets/platoonCreateRequestACK_m.h"

typedef enum {
    APP_LEADER_IDLE,
    APP_FOLLOWER_IDLE,
    APP_NEGOTIATING,
    APP_MANEUVERING
} vehicle_state_t;

namespace plexe::vncd {
    class MyPlatooningApp : public ApplicationAdapter {
    private:
        unique_ptr<PlatooningProtocol> app_protocol;
        unique_ptr<cMessage> evt_ManeuverEnd;
        vehicle_state_t state;

        unique_ptr<PlatoonCreateRequest> buildPlatoonCreateRequest();
        unique_ptr<PlatoonCreateRequestACK> buildPlatoonCreateRequestACK(PlatoonCreateRequest * req);
        bool isLeader(double coord);
        double original_speed;

    public:
        MyPlatooningApp() = default;
        ~MyPlatooningApp();

    protected:
        void handleSelfMsg(cMessage* msg) override;
        void handleLowerMsg(cMessage* msg) override;
        void initialize(int stage) override;
        void sendUnicast(omnetpp::cPacket *msg, int destination) override;
        void startMergeManeuver(int platoon_id, int leader_id, bool leader);
    };

    Define_Module(MyPlatooningApp)
}


#endif //VNCD_PROJECT_MYPLATOONINGAPP_H
