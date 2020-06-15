# TODO: Copyright header
cmake_minimum_required(VERSION 3.13.1)

set(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip)
get_filename_component(CHIP_ROOT ${CHIP_ROOT} REALPATH)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CHIP_ROOT}/config/nrfconnect/)

include(nrfconnect-app)

project(chip-nrf52840-lock-example)
target_sources(app PRIVATE
               main/AppTask.cpp
               main/BoltLockManager.cpp
               main/main.cpp
               main/LEDWidget.cpp)
