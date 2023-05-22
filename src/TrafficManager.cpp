//
// Copyright (C) 2014-2023 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <TrafficManager.h>

namespace plexe::vncd {

    Define_Module(TrafficManager);

    void TrafficManager::initialize(int stage) {

        TraCIBaseTrafficManager::initialize(stage);

        if (stage == 0) {

            nCars = par("nCars");
            nLanes = par("nLanes");
            insertStartTime = SimTime(par("insertStartTime").doubleValue());
            platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();

            platoonInsertDistance = par("platoonInsertDistance").doubleValue();
            platoonInsertHeadway = par("platoonInsertHeadway").doubleValue();
            platoonAdditionalDistance = par("platoonAdditionalDistance").doubleValue();
            platooningVType = par("platooningVType").stdstringValue();

            insertPlatoonMessage = new cMessage("");
            scheduleAt(insertStartTime, insertPlatoonMessage);
        }
    }

    void TrafficManager::scenarioLoaded() {
        automated.id = findVehicleTypeIndex(platooningVType);
        automated.lane = -1;
        automated.position = 0;
    }

    void TrafficManager::handleSelfMsg(cMessage *msg) {

        TraCIBaseTrafficManager::handleSelfMsg(msg);

        if (msg == insertPlatoonMessage) {
            insertCars();
        }
    }

    void TrafficManager::insertCars() {

        automated.position = 5;
        automated.lane = (int) uniform(0, 2);
        automated.speed = par("platoonInsertSpeed").doubleValue();

        //define the vehicle information as the leader of a 1 car platoon
        VehicleInfo vehicleInfo{
                .controller = ACC,
                .distance = 2,
                .headway = platoonLeaderHeadway,
                .id = insertedCars,
                .platoonId = insertedCars++,
                .position = automated.position
        };

        //Define the platoon information for the current vehicle
        PlatoonInfo info{
                .speed = automated.speed,
                .lane = automated.lane,
        };

        positions.addVehicleToPlatoon(vehicleInfo.id, vehicleInfo);
        positions.setPlatoonInformation(vehicleInfo.platoonId, info);
        this->addVehicleToQueue(0, automated);

        if(--this->nCars)
            scheduleAfter(SimTime(par("insertDelay")), insertPlatoonMessage);
    }

    TrafficManager::~TrafficManager() {
        cancelAndDelete(insertPlatoonMessage);
        insertPlatoonMessage = nullptr;
    }

} // namespace plexe
