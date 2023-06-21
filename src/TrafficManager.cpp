#include <TrafficManager.h>

namespace plexe::vncd {

    Define_Module(TrafficManager);

    void TrafficManager::initialize(int stage) {
        TraCIBaseTrafficManager::initialize(stage);
        if (stage > 0) return;

        //stage 0
        this->nCars = par("nCars");
        this->nLanes = par("nLanes");
        this->insertStartTime = SimTime(par("insertStartTime").doubleValue());
        this->platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();

        this->platooningVType = par("platooningVType").stdstringValue();

        this->evt_startInsertion = make_unique<cMessage>();
        scheduleAt(this->insertStartTime, this->evt_startInsertion.get());
    }

    void TrafficManager::handleSelfMsg(cMessage *msg) {
        TraCIBaseTrafficManager::handleSelfMsg(msg);
        if (this->evt_startInsertion.get() == msg) this->insertCars();
    }

    /**
     * Inserts a car with random speed configured by omnetpp.ini with its own platoon
     */
    void TrafficManager::insertCars() {
        struct Vehicle cur_vehicle{
                .id = findVehicleTypeIndex(this->platooningVType),
                .lane = (int) uniform(0, this->nLanes),
                .position = 5.0f,
                .speed = (float) par("platoonInsertSpeed").doubleValue(),
        };

        VehicleInfo vehicleInfo{
                .controller = ACC, //just not make crashes!
                .distance = 1,
                .headway = this->platoonLeaderHeadway,
                .id = this->insertedCars,
                .platoonId = this->insertedCars++,
                .position = (int) cur_vehicle.position
        };

        PlatoonInfo info{
                .speed = cur_vehicle.speed,
                .lane = cur_vehicle.lane,
        };

        positions.addVehicleToPlatoon(vehicleInfo.id, vehicleInfo);
        positions.setPlatoonInformation(vehicleInfo.platoonId, info);
        this->addVehicleToQueue(0, cur_vehicle);

        if (--this->nCars)
            scheduleAfter(par("insertDelay").doubleValue(), this->evt_startInsertion.get());
    }

    TrafficManager::~TrafficManager() {
        if(this->evt_startInsertion->isScheduled())
            cancelEvent(this->evt_startInsertion.get());
    }

} // namespace plexe
