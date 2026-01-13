#
# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/connectedhomeip REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(ZEPHYR_PLATFORM_COMMON ${CHIP_ROOT}/examples/platform/silabs/zephyr REALPATH)
get_filename_component(SILABS_LIGHTING_COMMON ${CHIP_ROOT}/examples/lighting-app/silabs REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON_DIR ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)


# Need this as wiseconnect abstraction is causing a build failure
add_definitions(-DMBEDTLS_X509_REMOVE_INFO )

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/silabs)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)
target_compile_options(app PRIVATE -Werror -Wno-error=format)

project(matter-zephyr-light)

include(${CHIP_ROOT}/config/silabs/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_include_directories(app
    PRIVATE
    include
    ${ZEPHYR_PLATFORM_COMMON}
)

target_sources(app
    PRIVATE
    src/AppTask.cpp
    ../include/CHIPProjectConfig.h
    ${ZEPHYR_PLATFORM_COMMON}/main.cpp
    ${ZEPHYR_PLATFORM_COMMON}/AppTaskBase.cpp
    ${ZEPHYR_PLATFORM_COMMON}/AppTaskZephyr.cpp
    ${ZEPHYR_PLATFORM_COMMON}/CHIPDeviceManager.cpp
    ${ZEPHYR_PLATFORM_COMMON}/CommonDeviceCallbacks.cpp
)

# TODO Implement OTA support
if(CONFIG_CHIP_OTA_REQUESTOR)
    # target_sources(app PRIVATE
    #                 ${EXAMPLE_PLATFORM_SILABS_COMMON_DIR}/ota_requestor/source/OTARequestorInitiatorCommon.cpp
    #                 ${EXAMPLE_PLATFORM_SILABS_COMMON_DIR}/ota_requestor/source/OTARequestorInitiatorZephyr.cpp
    # )
    # target_include_directories(app PRIVATE
    #                 ${EXAMPLE_PLATFORM_SILABS_COMMON_DIR}/ota_requestor/include/
    # )
endif()

if(CONFIG_CHIP_WIFI)
  chip_configure_data_model(app
      ZAP_FILE ${SILABS_LIGHTING_COMMON}/data_model/lighting-wifi-app.zap
  )
else()
  chip_configure_data_model(app
      ZAP_FILE ${SILABS_LIGHTING_COMMON}/data_model/lighting-thread-app.zap
  )
endif()

if(CONFIG_CHIP_LIB_SHELL)
    target_compile_definitions(app PRIVATE ENABLE_CHIP_SHELL)
    target_include_directories(app PRIVATE 
                               ${CHIP_ROOT}/examples/shell/shell_common/include
                               ${ZEPHYR_PLATFORM_COMMON}/include
    )
    target_sources(app PRIVATE
                   ${ZEPHYR_PLATFORM_COMMON}/AppCLIZephyr.cpp
                   ${CHIP_ROOT}/examples/shell/shell_common/cmd_misc.cpp
                   ${CHIP_ROOT}/examples/shell/shell_common/cmd_otcli.cpp
                   ${CHIP_ROOT}/examples/shell/shell_common/cmd_server.cpp
    )
endif()
