#
#    Copyright (c) 2020 Project CHIP Authors
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
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(NRFCONNECT_COMMON ${CHIP_ROOT}/examples/platform/nrfconnect REALPATH)
get_filename_component(CHEF ${CMAKE_CURRENT_SOURCE_DIR}/../ REALPATH)

include(${CHIP_ROOT}/config/nrfconnect/app/check-nrfconnect-version.cmake)

set(CONF_FILE prj.conf)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "Vendor ID " ${CONFIG_CHIP_DEVICE_VENDOR_ID})
message(STATUS "Product ID " ${CONFIG_CHIP_DEVICE_PRODUCT_ID})
message(STATUS "Product Name " ${CONFIG_CHIP_DEVICE_PRODUCT_NAME})
message(STATUS "SW Version String" ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING})

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf)
  list(APPEND CONF_FILE boards/${BOARD}.conf)
endif()

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

get_filename_component(GEN_DIR ${CHEF}/out/${CONFIG_CHEF_DEVICE_TYPE}/zap-generated REALPATH)

project(chip-nrfconnect-chef-example)

include(${CHIP_ROOT}/config/nrfconnect/app/check-sysbuild-use.cmake)
include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)

target_include_directories(app PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common
                           ${GEN_DIR}
                           ${CHEF}
                           ${CHEF}/common
                           ${CHEF}/common/clusters/
                           ${GEN_DIR}/../
                           ${CHIP_ROOT}/src
                           ${CHIP_ROOT}/examples/shell/shell_common/include
                           ${NRFCONNECT_COMMON}/util/include
                           ${NRFCONNECT_COMMON}/app/include
)

if (CONFIG_CHIP_LIB_SHELL)
  target_sources(app PRIVATE
    ${CHEF}/shell_common/globals.cpp
    ${CHEF}/shell_common/cmd_misc.cpp
    ${CHEF}/shell_common/cmd_otcli.cpp
  )

  target_include_directories(app PRIVATE
    ${CHEF}/shell_common/include
  )
endif()

target_sources(app PRIVATE
    ${CHEF}/common/chef-air-quality.cpp
    ${CHEF}/common/chef-concentration-measurement.cpp
    ${CHEF}/common/chef-fan-control-manager.cpp
    ${CHEF}/common/chef-laundry-washer-controls-delegate-impl.cpp
    ${CHEF}/common/chef-laundry-washer-mode.cpp
    ${CHEF}/common/chef-dishwasher-alarm-delegate-impl.cpp
    ${CHEF}/common/chef-dishwasher-mode-delegate-impl.cpp
    ${CHEF}/common/chef-operational-state-delegate-impl.cpp
    ${CHEF}/common/chef-rvc-mode-delegate.cpp
    ${CHEF}/common/chef-rvc-operational-state-delegate.cpp
    ${CHEF}/common/clusters/audio-output/AudioOutputManager.cpp
    ${CHEF}/common/clusters/channel/ChannelManager.cpp
    ${CHEF}/common/clusters/door-lock/chef-doorlock-stubs.cpp
    ${CHEF}/common/clusters/door-lock/chef-lock-endpoint.cpp
    ${CHEF}/common/clusters/door-lock/chef-lock-manager.cpp
    ${CHEF}/common/clusters/keypad-input/KeypadInputManager.cpp
    ${CHEF}/common/clusters/low-power/LowPowerManager.cpp
    ${CHEF}/common/clusters/media-input/MediaInputManager.cpp
    ${CHEF}/common/clusters/media-playback/MediaPlaybackManager.cpp
    ${CHEF}/common/clusters/refrigerator-and-temperature-controlled-cabinet-mode/tcc-mode.cpp
    ${CHEF}/common/clusters/resource-monitoring/chef-resource-monitoring-delegates.cpp
    ${CHEF}/common/clusters/switch/SwitchEventHandler.cpp
    ${CHEF}/common/clusters/switch/SwitchManager.cpp
    ${CHEF}/common/clusters/temperature-control/static-supported-temperature-levels.cpp
    ${CHEF}/common/clusters/target-navigator/TargetNavigatorManager.cpp
    ${CHEF}/common/clusters/wake-on-lan/WakeOnLanManager.cpp
    ${CHEF}/common/clusters/window-covering/chef-window-covering.cpp
    ${CHEF}/common/stubs.cpp
    ${CHEF}/nrfconnect/main.cpp
)

message(STATUS ${CHEF}/devices/${CONFIG_CHEF_DEVICE_TYPE}.zap)
chip_configure_data_model(app
    ZAP_FILE ${CHEF}/devices/${CONFIG_CHEF_DEVICE_TYPE}.zap
)

include(${CHIP_ROOT}/config/nrfconnect/app/flashing.cmake)


if (CONFIG_CHIP_PW_RPC)

# Make all targets created below depend on zephyr_interface to inherit MCU-related compilation flags
link_libraries($<BUILD_INTERFACE:zephyr_interface>)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)

pw_set_module_config(pw_rpc_CONFIG pw_rpc.disable_global_mutex_config)
pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.nrfconnect)
set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

add_subdirectory(third_party/connectedhomeip/examples/platform/nrfconnect/pw_sys_io)
add_subdirectory(third_party/connectedhomeip/third_party/nanopb/repo)
add_subdirectory(third_party/connectedhomeip/third_party/pigweed/repo)

pw_proto_library(attributes_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/attributes_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/attributes_service.options
  PREFIX
    attributes_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(boolean_state_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/boolean_state_service.proto
  PREFIX
    boolean_state_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(descriptor_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/descriptor_service.proto
  PREFIX
    descriptor_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(device_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/device_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/device_service.options
  PREFIX
    device_service
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(ot_cli_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/ot_cli_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/ot_cli_service.options
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  PREFIX
    ot_cli_service
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(thread_service
  SOURCES
    ${CHIP_ROOT}/examples/common/pigweed/protos/thread_service.proto
  INPUTS
    ${CHIP_ROOT}/examples/common/pigweed/protos/thread_service.options
  STRIP_PREFIX
    ${CHIP_ROOT}/examples/common/pigweed/protos
  PREFIX
    thread_service
  DEPS
    pw_protobuf.common_proto
)

target_sources(app PRIVATE
  ../../common/pigweed/RpcService.cpp
  ../../common/pigweed/nrfconnect/PigweedLoggerMutex.cpp
  ${NRFCONNECT_COMMON}/Rpc.cpp
  ${NRFCONNECT_COMMON}/util/PigweedLogger.cpp
)

target_include_directories(app PRIVATE
  ${PIGWEED_ROOT}/pw_sys_io/public
  ${CHIP_ROOT}/src/lib/support
  ${CHIP_ROOT}/src/system
  ${NRFCONNECT_COMMON}
  ${NRFCONNECT_COMMON}/util/include
  ../../common
  ../../common/pigweed
  ../../common/pigweed/nrfconnect)

target_compile_options(app PRIVATE
                       "-DPW_RPC_ATTRIBUTE_SERVICE=1"
                       "-DPW_RPC_BOOLEAN_STATE_SERVICE=1"
                       "-DPW_RPC_DESCRIPTOR_SERVICE=1"
                       "-DPW_RPC_DEVICE_SERVICE=1"
                       "-DPW_RPC_THREAD_SERVICE=1"
)

target_link_libraries(app PRIVATE
  attributes_service.nanopb_rpc
  boolean_state_service.nanopb_rpc
  descriptor_service.nanopb_rpc
  device_service.nanopb_rpc
  thread_service.nanopb_rpc
  pw_checksum
  pw_hdlc
  pw_log
  pw_rpc.server
  pw_sys_io
)

endif(CONFIG_CHIP_PW_RPC)
