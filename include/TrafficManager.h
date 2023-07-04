#ifndef PLATOONSTRAFFICMANAGER_H_
#define PLATOONSTRAFFICMANAGER_H_

#include "plexe/mobility/TraCIBaseTrafficManager.h"

using namespace std;

namespace plexe::vncd {

    class TrafficManager : public TraCIBaseTrafficManager {

    public:
        virtual void initialize(int stage);

        TrafficManager() {
            platoonInsertDistance = 0;
            platoonInsertHeadway = 0;
            insertStartTime = SimTime(0);
            platoonLeaderHeadway = 0;
            platoonAdditionalDistance = 0;
            nCars = 0;
            nLanes = 0;
            insertedCars = 0;
        }
        ~TrafficManager() override;

    private:
        //message reference to start car insertion
        unique_ptr <cMessage> evt_startInsertion;
        SimTime insertStartTime;

        // total number of vehicles to be injected
        int nCars;
        int insertedCars;
        // number of lanes
        int nLanes;
        // insert distance
        double platoonInsertDistance;
        // insert headway
        double platoonInsertHeadway;
        // headway for leader vehicles
        double platoonLeaderHeadway;
        // additional distance between consecutive platoons
        double platoonAdditionalDistance;
        // sumo vehicle type of platooning cars
        std::string platooningVType;

        int prev_lane = -1;

    protected:
        void insertCars();

        void handleSelfMsg(cMessage *msg) override;
    };

} // namespace plexe

#endif
