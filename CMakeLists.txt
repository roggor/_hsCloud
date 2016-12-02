cmake_minimum_required(VERSION 2.8.9)
project (hsCloudBB)

option (DEBUG_ON_MY "Turns on debugs" ON)

set (VERSION_MAJOR 1)
set (VERSION_MINOR1 0)
set (VERSION_MINOR2 0)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR1}.${VERSION_MINOR2})

#set default TARGET_ARCH
if(NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "arm")
    add_definitions(-DTARGET_ARM)
    message("TARGET_ARCH not set! " ${TARGET_ARCH} " used as default.")
endif()

configure_file (
  "${PROJECT_SOURCE_DIR}/main/version.hpp.in"
  "${PROJECT_SOURCE_DIR}/main/version.hpp"
  )


message("Using TARGET_ARCH = " ${TARGET_ARCH} ", libs/inc taken from ../../_install_" ${TARGET_ARCH})

add_definitions(-DSQLITE_OMIT_LOAD_EXTENSION)


include_directories("${PROJECT_SOURCE_DIR}/main" "${PROJECT_SOURCE_DIR}/drv")
include_directories("${PROJECT_SOURCE_DIR}/libs/jsonCpp/json")
include_directories("${PROJECT_SOURCE_DIR}/libs/serDeser")

include_directories("../../_install_${TARGET_ARCH}/include")
link_directories("../../_install_${TARGET_ARCH}/lib")

add_subdirectory(libs)
add_definitions(-std=c++0x -fpermissive)

file(GLOB SOURCES "main/*.cpp" "drv/*.cpp" "libs/sqlite/*.c" "./*.cpp")

add_executable(hsCloudBB ${SOURCES})
target_link_libraries(hsCloudBB serDeser jsonCpp rabbitmq pthread cli websockets)