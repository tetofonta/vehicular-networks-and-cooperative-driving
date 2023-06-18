//
// Created by user on 5/11/23.
//

#ifndef VNCD_PROJECT_MYPLATOONINGAPP_H
#define VNCD_PROJECT_MYPLATOONINGAPP_H

#include <plexe/apps/BaseApp.h>
#include <PlatoonSearchCAM_m.h>
#include <InternalListenTimeout_m.h>
#include <plexe/maneuver/JoinManeuver.h>
#include <plexe/apps/GeneralPlatooningApp.h>
#include "Protocol.h"

typedef enum{
    PLATOON_CREATE_REQUEST = 0,
    PLATOON_CREATE_ANSWER
} unicast_type_t;


namespace plexe::vncd {
    class MyPlatooningApp : public GeneralPlatooningApp {
    private:
        PlatooningProtocol * app_protocol = nullptr;
        long negotiationAddress = -1;
        bool isLeader = false;
        bool isPlatooningCompatible(PlatoonSearchCAM * pkt);
        std::map<long, std::tuple<int, InternalListenTimeout *>> events;
    public:
        MyPlatooningApp() = default;

    protected:
        void handleSelfMsg(cMessage* msg) override;
        void handleLowerMsg(cMessage* msg) override;
        virtual void initialize(int stage) override;

        void onPlatoonBeacon(const PlatooningBeacon *pb) override;
    };

    Define_Module(MyPlatooningApp)
}


#endif //VNCD_PROJECT_MYPLATOONINGAPP_H
