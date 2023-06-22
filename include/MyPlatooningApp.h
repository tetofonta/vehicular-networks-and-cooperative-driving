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

namespace plexe::vncd {
    class MyPlatooningApp : public ApplicationAdapter {
    private:
        unique_ptr<PlatooningProtocol> app_protocol;

    public:
        MyPlatooningApp() = default;

    protected:
        void handleSelfMsg(cMessage* msg) override;
        void handleLowerMsg(cMessage* msg) override;
        void initialize(int stage) override;
        void sendUnicast(omnetpp::cPacket *msg, int destination) override;
    };

    Define_Module(MyPlatooningApp)
}


#endif //VNCD_PROJECT_MYPLATOONINGAPP_H
