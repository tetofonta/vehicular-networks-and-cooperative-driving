//
// Created by stefano on 22/06/23.
//
#include <ApplicationAdapter.h>
#include <veins/modules/mac/ieee80211p/Mac1609_4.h>
using namespace std;
namespace plexe::vncd {

    void ApplicationAdapter::initialize(int stage) {
        BaseApp::initialize(stage);

        if (stage == 1) {
            this->protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"),
                                                gate("lowerControlIn"), gate("lowerControlOut"));
            findHost()->subscribe(Mac1609_4::sigRetriesExceeded, this);

            if (this->positionHelper->isLeader()) setPlatoonRole(PlatoonRole::LEADER);
            else setPlatoonRole(PlatoonRole::FOLLOWER);
            this->scenario = FindModule<BaseScenario *>::findSubModule(getParentModule());
        }
    }

    void ApplicationAdapter::handleSelfMsg(cMessage *msg) {
        if (this->activeManeuver && this->activeManeuver->handleSelfMsg(msg)) return;
        BaseApp::handleSelfMsg(msg);
    }

    void ApplicationAdapter::startJoinManeuver(int platoonId, int leaderId, int position){
        throw new cRuntimeError("Please use activeManeuver->startManeuver()");
    }

    void ApplicationAdapter::startMergeManeuver(int platoonId, int leaderId, int position) {
        throw new cRuntimeError("Please use activeManeuver->startManeuver()");
    }

    void ApplicationAdapter::onPlatoonBeacon(const PlatooningBeacon *pb) {
        EV << "platoonbeacon" << pb << endl;
        if(this->activeManeuver) this->activeManeuver->onPlatoonBeacon(pb);
        BaseApp::onPlatoonBeacon(pb);
    }

    void ApplicationAdapter::onManeuverMessage(const ManeuverMessage *pb) {
        EV << "maneuvermessage" << pb << endl;
        auto msg = unique_ptr<const ManeuverMessage>(pb);
        if(this->activeManeuver) this->activeManeuver->onManeuverMessage(msg.get());
    }

    void ApplicationAdapter::receiveSignal(omnetpp::cComponent *src, omnetpp::simsignal_t id, omnetpp::cObject *value,
                                           omnetpp::cObject *details) {
        if (id == Mac1609_4::sigRetriesExceeded) {
            auto frame = check_and_cast<BaseFrame1609_4*>(value);
            auto mm = check_and_cast<ManeuverMessage*>(frame->getEncapsulatedPacket());
            if (this->activeManeuver) this->activeManeuver->onFailedTransmissionAttempt(mm);
        }
    }

    void ApplicationAdapter::setActiveManeuver(unique_ptr<plexe::Maneuver> maneuver) {
        this->cur_maneuver = std::move(maneuver);
        this->activeManeuver = this->cur_maneuver.get();
    }

}