cmake_minimum_required(VERSION 2.8.9)
project (hello)

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
  "${PROJECT_SOURCE_DIR}/config/version.hpp.in"
  "${PROJECT_SOURCE_DIR}/config/version.hpp"
  )


message("Using TARGET_ARCH = " ${TARGET_ARCH} ", libs/inc taken from ../_install_" ${TARGET_ARCH})

include_directories("${PROJECT_SOURCE_DIR}/config" "${PROJECT_SOURCE_DIR}/debug" "${PROJECT_SOURCE_DIR}/drv")
include_directories("${PROJECT_SOURCE_DIR}/libs/jsonCpp/json")

include_directories("../_install_${TARGET_ARCH}/include")
link_directories("../_install_${TARGET_ARCH}/lib")

add_subdirectory(libs)
add_definitions(-std=c++0x -fpermissive)

file(GLOB SOURCES "debug/*.cpp" "drv/*.cpp" "./*.cpp")

add_executable(hello ${SOURCES})
target_link_libraries(hello serDeser jsonCpp rabbitmq pthread cli)