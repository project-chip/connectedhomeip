#
#    Copyright (c) 2026 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

cmake_minimum_required(VERSION 3.20)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(TELINK_COMMON ${CHIP_ROOT}/examples/platform/telink REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(ALL_DEVICES_COMMON_DIR ${CHIP_ROOT}/examples/all-devices-app/all-devices-common REALPATH)

include(${TELINK_COMMON}/common.cmake)
include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${ALL_DEVICES_COMMON_DIR}/device-factory/enabled_devices.cmake)

add_compile_definitions(IDENTIFY_CLUSTER_DISABLED=1)

project(chip-telink-all-devices-app-example)

target_include_directories(app PRIVATE
                           include
                           ${GEN_DIR}/app-common
                           ${TELINK_COMMON}/common/include
                           ${TELINK_COMMON}/util/include
                           ${TELINK_COMMON}/app/include
                           ${TELINK_COMMON}/zephyr_ext
                           ${CHIP_ROOT}/examples/all-devices-app/all-devices-common
                           ${CHIP_ROOT}/src/data-model-providers/codedriven
                           ${CHIP_ROOT}/src/data-model-providers
                           ${CMAKE_CURRENT_BINARY_DIR}) # for generated app_config/enabled_devices.h

target_sources(app PRIVATE
               # Code-driven data model
               ${CHIP_ROOT}/src/data-model-providers/codedriven/CodeDrivenDataModelProvider.cpp
               ${CHIP_ROOT}/src/data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.cpp
               ${CHIP_ROOT}/src/app/server-cluster/ServerClusterInterfaceRegistry.cpp
               ${CHIP_ROOT}/src/app/StorageDelegateWrapper.cpp
               ${CHIP_ROOT}/src/app/persistence/DefaultAttributePersistenceProvider.cpp

               # all-devices common sources (interfaces, root node base, supported devices)
               ${ALL_DEVICES_DEVICE_SOURCES}
               ${ALL_DEVICES_CLUSTER_SOURCES})

target_sources(app PRIVATE
               src/AppTask.cpp
               src/DeviceTypeShell.cpp
               ${TELINK_COMMON}/common/src/AllDevicesServer.cpp
               ${TELINK_COMMON}/common/src/mainCommon.cpp
               ${TELINK_COMMON}/common/src/AppTaskCommon.cpp
               ${TELINK_COMMON}/util/src/LEDManager.cpp
               ${TELINK_COMMON}/util/src/ButtonManager.cpp
               ${TELINK_COMMON}/util/src/ThreadUtil.cpp
               ${TELINK_COMMON}/util/src/PWMManager.cpp
               ${TELINK_COMMON}/util/src/ColorFormat.cpp
               ${TELINK_COMMON}/zephyr_ext/zephyr_key_matrix.c
               ${TELINK_COMMON}/zephyr_ext/zephyr_key_pool.c
               ${TELINK_COMMON}/zephyr_ext/zephyr_led_pool.c
               ${TELINK_COMMON}/zephyr_ext/zephyr_ws2812.c
               ${TELINK_COMMON}/zephyr_ext/zephyr_pm_observer.c)

if(CONFIG_NET_L2_OPENTHREAD)
  target_sources(app PRIVATE
    ${CHIP_ROOT}/examples/all-devices-app/all-devices-common/device/types/root-node/ThreadRootNode.cpp
    ${CHIP_ROOT}/examples/all-devices-app/all-devices-common/device/types/root-node/ThreadRootNode.h
  )
elseif(CONFIG_CHIP_WIFI)
  target_sources(app PRIVATE
    ${CHIP_ROOT}/examples/all-devices-app/all-devices-common/device/types/root-node/WifiRootNode.cpp
    ${CHIP_ROOT}/examples/all-devices-app/all-devices-common/device/types/root-node/WifiRootNode.h
  )
endif()

if(CONFIG_PWM)
	target_sources(app PRIVATE ${TELINK_COMMON}/zephyr_ext/zephyr_pwm_pool.c)
endif()

if(CONFIG_BOOTLOADER_MCUBOOT)
  target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()

if(CONFIG_MCUMGR_TRANSPORT_BT)
  zephyr_library_link_libraries(MCUBOOT_BOOTUTIL)
  target_sources(app PRIVATE ${TELINK_COMMON}/util/src/DFUOverSMP.cpp)
endif()

if (CONFIG_CHIP_PW_RPC)
  include(${TELINK_COMMON}/Rpc.cmake)
endif()
