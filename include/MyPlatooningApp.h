//
// Created by user on 5/11/23.
//

#ifndef VNCD_PROJECT_MYPLATOONINGAPP_H
#define VNCD_PROJECT_MYPLATOONINGAPP_H

#include <plexe/apps/BaseApp.h>
#include <PlatoonSearchCAM_m.h>
#include <InternalListenTimeout_m.h>
#include "Protocol.h"

typedef enum{
    PLATOON_CREATE_REQUEST = 0,
    PLATOON_CREATE_ANSWER
} unicast_type_t;

namespace plexe::vncd {
    class MyPlatooningApp : public BaseApp {
    private:
        PlatooningProtocol * app_protocol = nullptr;
//        bool can_be_leader;
        long negotiationAddress = -1;
        bool isLeader = false;
//        int leaderExtraction = -1;

        bool isPlatooningCompatible(PlatoonSearchCAM * pkt);

        std::map<long, std::tuple<int, InternalListenTimeout *>> events;

    public:
        MyPlatooningApp() = default;

    protected:
        void handleSelfMsg(cMessage* msg) override;
        void handleLowerMsg(cMessage* msg) override;
        virtual void initialize(int stage) override;

    };

    Define_Module(MyPlatooningApp)
}


#endif //VNCD_PROJECT_MYPLATOONINGAPP_H
