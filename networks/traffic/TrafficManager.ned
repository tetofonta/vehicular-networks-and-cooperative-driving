package org.car2x.plexe.vncd.traffic;

import org.car2x.plexe.mobility.BTraCIBaseTrafficManager;

simple TrafficManager extends BTraCIBaseTrafficManager {

    parameters:
        //platoon insertion time
        double insertStartTime @unit("s");
        volatile double insertDelay @unit("s");


        //platoon insertion speed
        volatile double platoonInsertSpeed @unit("mps");
        //how many vehicles to inject
        int nCars;
        //number of lanes
        int nLanes;
        //sumo vehicle type for platooning cars
        string platooningVType;
        //insert distance and headway. distance is computed as:
        //dist = insertHeadway * insertSpeed + insertDistance
        double platoonInsertDistance @unit("m") = default(5m);
        double platoonInsertHeadway @unit("s") = default(0s);
        double platoonLeaderHeadway @unit("s") = default(1.2s);
        double platoonAdditionalDistance @unit("m") = default(0m);
        @class(plexe::vncd::TrafficManager);
}
