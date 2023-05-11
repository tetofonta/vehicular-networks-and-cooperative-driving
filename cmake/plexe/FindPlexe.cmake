find_path(PLEXE_ROOT NAMES bin/plexe_run PATHS ENV PATH PATH_SUFFIXES .. DOC "Path to Plexe root directory")

set(PLEXE_INCLUDE_DIR ${PLEXE_ROOT}/src)
set(PLEXE_LINK_DBG_LIBRARY ${PLEXE_ROOT}/out/clang-debug/src/libplexe_dbg.so)
set(PLEXE_LINK_REL_LIBRARY ${PLEXE_ROOT}/out/clang-release/src/libplexe.so)
set(PLEXE_RUN_BIN ${PLEXE_ROOT}/bin/plexe_run)

if(${CMAKE_BUILD_TYPE} EQUAL Release)
    set(PLEXE_LINK_LIBRARY ${PLEXE_LINK_REL_LIBRARY})
else()
    set(PLEXE_LINK_LIBRARY ${PLEXE_LINK_DBG_LIBRARY})
endif()

set(PLEXE_NED_DIR ${PLEXE_ROOT}/src/plexe/)

execute_process(COMMAND python ${PLEXE_ROOT}/print-plexe-version OUTPUT_VARIABLE PLEXE_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Plexe
        FOUND_VAR PLEXE_FOUND
        VERSION_VAR PLEXE_VERSION
        REQUIRED_VARS PLEXE_ROOT PLEXE_INCLUDE_DIR PLEXE_LINK_LIBRARY PLEXE_NED_DIR PLEXE_RUN_BIN)
