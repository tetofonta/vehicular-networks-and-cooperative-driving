cmake_minimum_required(VERSION 3.22)
project(VNCD_PROJECT)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/omnetpp ${PROJECT_SOURCE_DIR}/cmake/veins ${PROJECT_SOURCE_DIR}/cmake/plexe)
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
        )

add_library(libvncd SHARED ${SOURCES})
set_target_properties(libvncd PROPERTIES NED_FOLDERS networks:${VEINS_NED_DIR}:${PLEXE_NED_DIR})
target_link_libraries(libvncd ${PLEXE_LINK_LIBRARY})
generate_opp_message(msg/PlatoonSearchCAM.msg TARGET libvncd)
generate_opp_message(msg/PlatoonUnicast.msg TARGET libvncd)
generate_opp_message(msg/PlatoonCreateRequest.msg TARGET libvncd)
generate_opp_message(msg/PlatoonCreateAnswer.msg TARGET libvncd)

add_opp_run(vncd DEPENDENCY libvncd WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})