//
// Created by user on 5/23/23.
//

#ifndef VNCD_PROJECT_PROTOCOL_H
#define VNCD_PROJECT_PROTOCOL_H

#include <plexe/protocols/BaseProtocol.h>
#include "packets/platoonAdvertisementBeacon_m.h"
#include "messages/PlatoonAdvertisementListenTimeout_m.h"

static short PLATOON_NEGOTIATION_TYPE = 23344;

using namespace std;
namespace plexe::vncd {

    class PlatooningProtocol : public BaseProtocol {

    private:
        unique_ptr<cMessage> evt_SendPlatoonAdvertiseBeacon;
        unique_ptr<cMessage> evt_SendPlatooonBeacon;
        SimTime platoonAdvertiseBeaconInterval;

        void sendPlatoonAdvertisementBeacon();
        unique_ptr<BaseFrame1609_4> encapsulate(int destinationAddress, cPacket * pkt);
        unique_ptr<BaseFrame1609_4> buildPacket(int destinationAddress, cPacket * pkt);

        unique_ptr<PlatoonAdvertiseBeacon> createPlatoonAdvertisementBeacon();
        map<long, unique_ptr<PlatoonAdvertisementListenTimeout>> events;

        bool doRoutePlatoonRequests = false;
        bool doAcceptPlatoons = false;
        double platooningFormationSpeedRange;
        uint8_t maxPlatoonSize;

    protected:
        void handleSelfMsg(cMessage *msg) override;
        void handleLowerMsg(cMessage* msg) override;
        bool handlePlatoonAdvertisement(PlatoonAdvertiseBeacon * pkt);
        bool isPlatoonCompatible(PlatoonAdvertiseBeacon * pkt);
        void updateDistances(PlatoonAdvertisementListenTimeout * interval);

    public:
        int front_platoon_id = -1;
        int back_platoon_id = -1;
        double front_distance = 10000;
        double back_distance = 10000;
        //todo fai getter... non avevo voglia!

        PlatooningProtocol();
        ~PlatooningProtocol() override;

        void initialize(int stage) override;

        void sendBroadcast(cPacket * pkt);
        void sendUnicast(cPacket * pkt, int destinationAddress);

        void startPlatoonAdvertisement();
        void stopPlatoonAdvertisement();

        void routePlatoonRequests(bool state);
        void setPlatoonAccepting(bool val);

    };
}

#endif //VNCD_PROJECT_PROTOCOL_H
