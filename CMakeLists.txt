
cmake_minimum_required(VERSION 2.8)
project(libemulex)
set(libed2k_version 0.0.1)
set(out_dir "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/module")
####### Build customization

# Override from command line "CMake -D<OPTION>=TRUE/FALSE/0/1/ON/OFF"
option (PRODUCTION  "Build project in production mode." FALSE)
option (BUILD_TESTS "Build unit tests." TRUE)
option (BUILD_TOOLS "Build tools." TRUE)
option (BUILD_SHARED "Build shared library." TRUE)
option (DISABLE_DHT "Enable KAD support" FALSE)
option (UPNP_VERBOSE "Verbose output for UPnP" FALSE)
option (DHT_VERBOSE "Verbose output for DHT" FALSE)
option (DEPS_DIR "the deps prefix location" FALSE)

if (NOT DEPS_DIR)
    set(DEPS_DIR "/usr/local/")
endif()

include(cmake/Environment.cmake)
include(cmake/Linux.cmake)
include(cmake/Windows.cmake)
include(cmake/Android.cmake)
include(cmake/Darwin.cmake)
include(cmake/Targets.cmake)
