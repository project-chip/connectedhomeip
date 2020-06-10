# TODO: Copyright header
cmake_minimum_required(VERSION 3.13.1)

set(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip)
get_filename_component(CHIP_ROOT ${CHIP_ROOT} REALPATH)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CHIP_ROOT}/config/nrf5_ncs/)

include(nrf5-app)

project(chip-nrf52840-lock-example)
target_sources(app PRIVATE main/main.cpp)
target_link_libraries(app PRIVATE ${CHIP_OUTPUT_DIR}/lib/libCHIP.a)