cmake_minimum_required(VERSION 3.22)
project(VNCD_PROJECT)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/omnetpp ${PROJECT_SOURCE_DIR}/cmake/veins ${PROJECT_SOURCE_DIR}/cmake/plexe)

set(OMNETPP_ROOT /opt/omnetpp)
set(VEINS_ROOT ${PROJECT_SOURCE_DIR}/../veins)
set(PLEXE_ROOT ${PROJECT_SOURCE_DIR}/../plexe)

find_package(OmnetPP 6.0 REQUIRED)
find_package(Veins REQUIRED)
find_package(Plexe REQUIRED)

include(GenerateOppMessage)
include(AddOppRun)
include_directories(${OMNETPP_INCLUDE_DIR} ${VEINS_INCLUDE_DIR} ${PLEXE_INCLUDE_DIR} include ${CMAKE_BINARY_DIR}/msg)

set(SOURCES
        src/MyPlatooningApp.cpp
        include/MyPlatooningApp.h
        src/Scenario.cpp
        include/Scenario.h
        src/TrafficManager.cpp
        include/TrafficManager.h
        src/PlatooningProtocol.cpp
        include/Protocol.h
        src/maneuvers/MergeManeuver.cpp
        include/maneuvers/MergeManeuver.h
        src/ApplicationAdapter.cpp
        include/ApplicationAdapter.h
        src/maneuvers/IdleManeuver.cpp
        include/maneuvers/IdleManeuver.h)

add_library(libvncd SHARED ${SOURCES})
set_target_properties(libvncd PROPERTIES NED_FOLDERS ./networks:${VEINS_NED_DIR}:${PLEXE_NED_DIR})
target_link_libraries(libvncd ${PLEXE_LINK_LIBRARY})
generate_opp_message(msg/packets/packetHeader.msg TARGET libvncd)
generate_opp_message(msg/packets/platoonAdvertisementBeacon.msg TARGET libvncd)
generate_opp_message(msg/messages/PlatoonAdvertisementListenTimeout.msg TARGET libvncd)
generate_opp_message(msg/packets/platoonCreateRequest.msg TARGET libvncd)
generate_opp_message(msg/packets/platoonCreateRequestACK.msg TARGET libvncd)

add_opp_run(vncd DEPENDENCY libvncd WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(runmaker COMMAND sh ${PROJECT_SOURCE_DIR}/makerun.sh "${OMNETPP_ROOT}" "${PROJECT_SOURCE_DIR}/networks:${VEINS_NED_DIR}:${PLEXE_NED_DIR}" "$<TARGET_FILE:libvncd>")
