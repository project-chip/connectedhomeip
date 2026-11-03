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

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../.. REALPATH)
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/silabs)

# west passes -DUSE_CCACHE=0 as UNINITIALIZED; normalize for connectedhomeip_PRE_CMAKE.
if("$CACHE{USE_CCACHE}" STREQUAL "0" OR "${USE_CCACHE}" STREQUAL "0")
    set(USE_CCACHE "0" CACHE STRING "Disable ccache for all sysbuild images" FORCE)
endif()

# Forward board overlay (custom partitions) to MCUboot; plain Zephyr sysbuild has no Partition Manager.
if(EXISTS ${APP_DIR}/boards/${BOARD}.overlay)
    set(mcuboot_EXTRA_DTC_OVERLAY_FILE ${APP_DIR}/boards/${BOARD}.overlay
        CACHE INTERNAL "App board overlay for MCUboot partitions")
else()
    unset(mcuboot_EXTRA_DTC_OVERLAY_FILE CACHE)
endif()

# Do not use a sysbuild/mcuboot/ directory: it becomes APPLICATION_CONFIG_DIR and replaces MCUboot's prj.conf.
set(_mcuboot_extra_conf ${APP_DIR}/sysbuild/mcuboot.conf)
if(EXISTS ${APP_DIR}/sysbuild/mcuboot_${BOARD}.conf)
    list(APPEND _mcuboot_extra_conf ${APP_DIR}/sysbuild/mcuboot_${BOARD}.conf)
endif()
set(mcuboot_EXTRA_CONF_FILE ${_mcuboot_extra_conf}
    CACHE INTERNAL "MCUboot Kconfig fragments from lighting-app")

find_package(Sysbuild REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sysbuild LANGUAGES)
