find_path(VEINS_ROOT NAMES bin/veins_launchd print-veins-version PATHS ENV PATH PATH_SUFFIXES .. DOC "Path to Veins root directory")
set(VEINS_INCLUDE_DIR ${VEINS_ROOT}/src)
set(VEINS_LINK_DBG_LIBRARY ${VEINS_ROOT}/out/clang-debug/src/libveins_dbg.so)
set(VEINS_LINK_REL_LIBRARY ${VEINS_ROOT}/out/clang-release/src/libveins.so)

if(${CMAKE_BUILD_TYPE} EQUAL Release)
    set(VEINS_LINK_LIBRARY ${VEINS_LINK_REL_LIBRARY})
else()
    set(VEINS_LINK_LIBRARY ${VEINS_LINK_DBG_LIBRARY})
endif()

set(VEINS_NED_DIR ${VEINS_ROOT}/src/veins/)

execute_process(COMMAND python ${VEINS_ROOT}/print-veins-version OUTPUT_VARIABLE VEINS_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Veins
        FOUND_VAR VEINS_FOUND
        VERSION_VAR VEINS_VERSION
        REQUIRED_VARS VEINS_ROOT VEINS_INCLUDE_DIR VEINS_LINK_LIBRARY VEINS_NED_DIR)
