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
get_filename_component(NLIO_ROOT ${CHIP_ROOT}/third_party/nlio/repo/include REALPATH)
get_filename_component(NRFCONNECT_COMMON ${CHIP_ROOT}/examples/platform/nrfconnect REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

include(${CHIP_ROOT}/config/nrfconnect/app/check-nrfconnect-version.cmake)

# Load NCS/Zephyr build system
set(CONF_FILE ${CHIP_ROOT}/config/nrfconnect/app/sample-defaults.conf prj.conf)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf)
  list(APPEND CONF_FILE boards/${BOARD}.conf)
endif()

option(BUILD_WITH_DFU "Build target with Device Firmware Upgrade support" OFF)
if(BUILD_WITH_DFU)
    if(${BOARD} STREQUAL "nrf5340dk_nrf5340_cpuapp")
        list(INSERT OVERLAY_CONFIG 0 ${CHIP_ROOT}/config/nrfconnect/app/overlay-multi_image_smp_dfu_support.conf)
        set(mcuboot_OVERLAY_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/configuration/mcuboot_multi_image_dfu.conf CACHE INTERNAL "")
    else()
        list(INSERT OVERLAY_CONFIG 0 ${CHIP_ROOT}/config/nrfconnect/app/overlay-single_image_smp_dfu_support.conf)
        set(mcuboot_OVERLAY_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/configuration/mcuboot_single_image_dfu.conf CACHE INTERNAL "")
    endif()
    list(INSERT OVERLAY_CONFIG 0 ${CHIP_ROOT}/config/nrfconnect/app/overlay-mcuboot_qspi_nor_support.conf)
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static.yml)
endif()

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)

project(chip-nrfconnect-lock-example)

include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/config/nrfconnect/app/flashing.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_include_directories(app PRIVATE
                           main/include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/lock-app
                           ${NLIO_ROOT}
                           ${NRFCONNECT_COMMON}/util/include
                           ${NRFCONNECT_COMMON}/app/include)

target_sources(app PRIVATE
               main/AppTask.cpp
               main/BoltLockManager.cpp
               main/main.cpp
               main/ZclCallbacks.cpp
               ${GEN_DIR}/lock-app/zap-generated/callback-stub.cpp
               ${GEN_DIR}/lock-app/zap-generated/IMClusterCommandHandler.cpp
               ${NRFCONNECT_COMMON}/util/LEDWidget.cpp
               ${NRFCONNECT_COMMON}/util/ThreadUtil.cpp)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../lock-common/lock-app.zap
)

if(BUILD_WITH_DFU)
    target_sources(app PRIVATE ${NRFCONNECT_COMMON}/util/DFUOverSMP.cpp)
endif()
