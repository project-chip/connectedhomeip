#
#    Copyright (c) 2022 Project CHIP Authors
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
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON_DIR ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)

include(${CHIP_ROOT}/config/nrfconnect/app/check-nrfconnect-version.cmake)

if(DEFINED CONF_FILE AND NOT CONF_FILE STREQUAL "prj.conf")
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static_dfu.yml)
endif()

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)

project(chip-nrfconnect-all-clusters-minimal-app-example)

include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/config/nrfconnect/app/flashing.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_include_directories(app PRIVATE
                           main/include
                           ${ALL_CLUSTERS_COMMON_DIR}/include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/all-clusters-minimal-app
                           ${NRFCONNECT_COMMON}/util/include)

target_sources(app PRIVATE
               main/AppTask.cpp
               main/main.cpp
               ${ALL_CLUSTERS_COMMON_DIR}/src/smco-stub.cpp
               ${ALL_CLUSTERS_COMMON_DIR}/src/static-supported-modes-manager.cpp
               ${ALL_CLUSTERS_COMMON_DIR}/src/bridged-actions-stub.cpp
               ${ALL_CLUSTERS_COMMON_DIR}/src/binding-handler.cpp
               ${NRFCONNECT_COMMON}/util/LEDWidget.cpp)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../all-clusters-common/all-clusters-minimal-app.zap
)

if(CONFIG_CHIP_OTA_REQUESTOR OR CONFIG_MCUMGR_TRANSPORT_BT)
    target_sources(app PRIVATE ${NRFCONNECT_COMMON}/util/OTAUtil.cpp)
endif()

if (CONFIG_SHELL AND BOARD STREQUAL "nrf52840dongle_nrf52840")
  target_sources(app PRIVATE ${NRFCONNECT_COMMON}/util/DFUTrigger.cpp)
endif()
