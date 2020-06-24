cmake_minimum_required(VERSION 3.4.1)

add_library(native-lib SHARED src/main/cpp/native-lib.cpp)
find_library(log-lib log)

target_link_libraries(native-lib ${log-lib}  ${CMAKE_HOME_DIRECTORY}/../../../setup_payload/libSetupPayload.a)
target_include_directories(native-lib PRIVATE ${CMAKE_HOME_DIRECTORY}/../../../setup_payload ${CMAKE_HOME_DIRECTORY}/../../../lib ${CMAKE_HOME_DIRECTORY}/../../..)
add_definitions(-DCHIP_SYSTEM_CONFIG_USE_SOCKETS)
