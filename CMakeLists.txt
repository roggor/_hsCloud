cmake_minimum_required(VERSION 2.8.9)
project (hello)

option (DEBUG_ON_MY "Turns on debugs" ON)

set (VERSION_MAJOR 1)
set (VERSION_MINOR1 0)
set (VERSION_MINOR2 0)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR1}.${VERSION_MINOR2})

configure_file (
  "${PROJECT_SOURCE_DIR}/version.hpp.in"
  "${PROJECT_SOURCE_DIR}/version.hpp"
  )

#set default TARGET_ARCH
if(NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "arm")
    message("TARGET_ARCH not set! " ${TARGET_ARCH} " used as default.")
endif()

include_directories("${PROJECT_SOURCE_DIR}/drv" "${PROJECT_SOURCE_DIR}/jsonCpp/json" "${PROJECT_SOURCE_DIR}/libExample" "../_install_${TARGET_ARCH}/include")
link_directories("../_install_${TARGET_ARCH}/lib")

message("!!! Using TARGET_ARCH = " ${TARGET_ARCH} ", libs/inc taken from ../_install_" ${TARGET_ARCH})

add_subdirectory(libExample)
add_definitions(-std=c++0x)

file(GLOB SOURCES "drv/*.cpp" "jsonCpp/*.cpp" "./*.cpp")

add_executable(hello ${SOURCES})
target_link_libraries(hello exampleLib rabbitmq pthread)