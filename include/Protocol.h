//
// Created by user on 5/23/23.
//

#ifndef VNCD_PROJECT_PROTOCOL_H
#define VNCD_PROJECT_PROTOCOL_H

#include <plexe/protocols/BaseProtocol.h>
#include <PlatoonUnicast_m.h>

namespace plexe::vncd {

    class PlatooningProtocol : public BaseProtocol {

    private:
        cPacket * sending = nullptr;

        cMessage * platoonFormationAdvertisement = nullptr;

//        bool can_be_leader;

        cMessage * platoonUnicast = nullptr;
        PlatoonUnicast * currentSendingPacket = nullptr;
        double currentSendingDelay;
        int currentSendingRetries = -1;

        void sendPlatoonAdvertisementBeacon();

        void sendPacket(cPacket * pkt);

    protected:
        void handleSelfMsg(cMessage *msg) override;

    public:
        PlatooningProtocol();

        ~PlatooningProtocol() override;

        void initialize(int stage) override;

        void startPlatoonFormationAdvertisement();
        void stopPlatoonFormationAdvertisement();

        void startSendingUnicast(PlatoonUnicast * packet, long address, double delay, int retries);
        void stopSendingUnicast();
    };
}

#endif //VNCD_PROJECT_PROTOCOL_H
