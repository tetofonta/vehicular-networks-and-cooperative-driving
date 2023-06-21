//
// Created by user on 5/23/23.
//

#ifndef VNCD_PROJECT_PROTOCOL_H
#define VNCD_PROJECT_PROTOCOL_H

#include <plexe/protocols/BaseProtocol.h>
#include "packets/platoonAdvertisementBeacon_m.h"
#include "messages/PlatoonAdvertisementListenTimeout_m.h"

using namespace std;
namespace plexe::vncd {

    class PlatooningProtocol : public BaseProtocol {

    private:
        unique_ptr<cMessage> evt_SendPlatoonAdvertiseBeacon;
        SimTime platoonAdvertiseBeaconInterval;

        void sendPlatoonAdvertisementBeacon();
        unique_ptr<BaseFrame1609_4> encapsulate(int destinationAddress, cPacket * pkt, int kind);
        unique_ptr<BaseFrame1609_4> buildPacket(int destinationAddress, PacketHeader * pkt, int kind);

        unique_ptr<PlatoonAdvertiseBeacon> createPlatoonAdvertisementBeacon();
        map<long, unique_ptr<PlatoonAdvertisementListenTimeout>> events;

        bool doRoutePlatoonRequests = false;
        double platooningFormationSpeedRange;

    protected:
        void handleSelfMsg(cMessage *msg) override;
        void handleLowerMsg(cMessage* msg) override;

        bool handlePlatoonAdvertisement(PlatoonAdvertiseBeacon * pkt);

        bool isPlatoonCompatible(PlatoonAdvertiseBeacon * pkt);
    public:
        PlatooningProtocol();
        ~PlatooningProtocol() override;

        void initialize(int stage) override;

        void sendBroadcast(PacketHeader * pkt, int kind);
        void sendUnicast(PacketHeader * pkt, int kind, int destinationAddress);

        void startPlatoonAdvertisement();
        void stopPlatoonAdvertisement();

        void routePlatoonRequests(bool state);
    };
}

#endif //VNCD_PROJECT_PROTOCOL_H
