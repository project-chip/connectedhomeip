#
# Copyright (c) 2026 Project CHIP Authors
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
cmake_minimum_required(VERSION 3.20)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(ALL_DEVICES_COMMON_DIR ${CHIP_ROOT}/examples/all-devices-app/all-devices-common REALPATH)

# You may point this at a file that defines GetAppTask() to return a subclass of AppTask.
if(NOT DEFINED APP_TASK_INSTANCE_SOURCE)
    set(APP_TASK_INSTANCE_SOURCE "")
endif()

# Device selection: -DALL_DEVICES_ENABLED_DEVICES="on-off-light;..." (default: on-off-light).
# Set to an empty string to build all of the devices.
if(NOT DEFINED ALL_DEVICES_ENABLED_DEVICES)
    set(ALL_DEVICES_ENABLED_DEVICES "on-off-light")
endif()
include(${ALL_DEVICES_COMMON_DIR}/device-factory/enabled_devices.cmake)

# Count how many are ALL_DEVICES_ENABLE_<NAME>=1 to size AppTask's device array
get_cmake_property(_all_devices_vars VARIABLES)
set(ALL_DEVICES_ENABLED_DEVICE_COUNT 0)
foreach(_var IN LISTS _all_devices_vars)
    if(_var MATCHES "^ALL_DEVICES_ENABLE_" AND ${_var})
        math(EXPR ALL_DEVICES_ENABLED_DEVICE_COUNT "${ALL_DEVICES_ENABLED_DEVICE_COUNT} + 1")
    endif()
endforeach()

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/zephyr/chip-module)

# Zephyr auto-discovers boards in  boards/
# This allows discovery in boards/vendor/
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/board_files.cmake)
if(DEFINED BOARD)
    all_devices_board_name(${BOARD} ALL_DEVICES_BOARD_NAME)

    all_devices_find_board_file(${CMAKE_CURRENT_SOURCE_DIR}/boards ${ALL_DEVICES_BOARD_NAME}.overlay
                                 ALL_DEVICES_NESTED_BOARD_OVERLAY EXCLUDE_FLAT)
    if(ALL_DEVICES_NESTED_BOARD_OVERLAY)
        list(APPEND DTC_OVERLAY_FILE ${ALL_DEVICES_NESTED_BOARD_OVERLAY})
        set(DTC_OVERLAY_FILE "${DTC_OVERLAY_FILE}" CACHE STRING "Application devicetree overlays" FORCE)
    endif()
    message(STATUS "DEBUG BOARD=${BOARD} ALL_DEVICES_BOARD_NAME=${ALL_DEVICES_BOARD_NAME} ALL_DEVICES_NESTED_BOARD_OVERLAY=${ALL_DEVICES_NESTED_BOARD_OVERLAY} EXTRA_DTC_OVERLAY_FILE=${EXTRA_DTC_OVERLAY_FILE}")

    all_devices_find_board_file(${CMAKE_CURRENT_SOURCE_DIR}/boards ${ALL_DEVICES_BOARD_NAME}.conf
                                 ALL_DEVICES_NESTED_BOARD_CONF EXCLUDE_FLAT)
    if(ALL_DEVICES_NESTED_BOARD_CONF)
        list(APPEND EXTRA_CONF_FILE ${ALL_DEVICES_NESTED_BOARD_CONF})
    endif()

    # Zephyr also auto-discovers socs/${SOC}.conf, but only directly in socs/.
    # Allow grouping those under a vendor subfolder too (e.g. socs/vendor/).
    all_devices_find_soc_for_board(${ALL_DEVICES_BOARD_NAME} ALL_DEVICES_SOC_NAME)
    if(ALL_DEVICES_SOC_NAME)
        all_devices_find_board_file(${CMAKE_CURRENT_SOURCE_DIR}/socs ${ALL_DEVICES_SOC_NAME}.conf
                                     ALL_DEVICES_NESTED_SOC_CONF EXCLUDE_FLAT)
        if(ALL_DEVICES_NESTED_SOC_CONF)
            list(APPEND EXTRA_CONF_FILE ${ALL_DEVICES_NESTED_SOC_CONF})
        endif()
    endif()
endif()

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# ot-config re-lists zephyr_interface's INTERFACE_COMPILE_DEFINITIONS, including
# $<COMPILE_LANGUAGE:CXX> generator expressions. openthread/CMakeLists.txt uses
# ot-config's defines to build a plain custom target, where COMPILE_LANGUAGE is
# invalid. Strip it so LLVM/Clang builds configure cleanly.
if(CONFIG_OPENTHREAD AND TARGET ot-config AND TARGET zephyr_interface)
    get_target_property(OT_CONFIG_DEFS ot-config INTERFACE_COMPILE_DEFINITIONS)
    if(OT_CONFIG_DEFS)
        list(FILTER OT_CONFIG_DEFS EXCLUDE REGEX "TARGET_PROPERTY:zephyr_interface,INTERFACE_COMPILE_DEFINITIONS")
    else()
        set(OT_CONFIG_DEFS "")
    endif()
    get_target_property(ZEPHYR_INTERFACE_DEFS zephyr_interface INTERFACE_COMPILE_DEFINITIONS)
    if(ZEPHYR_INTERFACE_DEFS)
        list(FILTER ZEPHYR_INTERFACE_DEFS EXCLUDE REGEX "COMPILE_LANGUAGE:CXX")
    else()
        set(ZEPHYR_INTERFACE_DEFS "")
    endif()
    set_target_properties(ot-config PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "${OT_CONFIG_DEFS};${ZEPHYR_INTERFACE_DEFS}")
endif()

project(matter-all-devices-zephyr)

include(${CHIP_ROOT}/config/zephyr/app/enable-gnu-std.cmake)

target_compile_options(app PRIVATE -Werror -Wno-error=format)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)
endif()

target_compile_definitions(app PRIVATE ALL_DEVICES_ENABLED_DEVICE_COUNT=${ALL_DEVICES_ENABLED_DEVICE_COUNT})

# Extension Point for a custom app task
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/custom/CMakeLists.txt)
#    include(${CMAKE_CURRENT_SOURCE_DIR}/custom/CMakeLists.txt)
# endif()

if(APP_TASK_INSTANCE_SOURCE STREQUAL "")
    set(APP_TASK_INSTANCE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/AppTaskInstance.cpp)
endif()

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    src
    include
    ${ALL_DEVICES_COMMON_DIR}
    ${CHIP_ROOT}/src/data-model-providers/codedriven
    ${CHIP_ROOT}/src/data-model-providers
    ${CMAKE_CURRENT_BINARY_DIR} # for generated app_config/enabled_devices.h
)

target_sources(app PRIVATE
    CHIPProjectConfig.h
    include/AppTask.h
    src/AppTask.cpp
    ${APP_TASK_INSTANCE_SOURCE}
    include/Buttons.h
    src/Buttons.cpp
    src/main.cpp

    # Code-driven data model
    ${CHIP_ROOT}/src/data-model-providers/codedriven/CodeDrivenDataModelProvider.cpp
    ${CHIP_ROOT}/src/data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.cpp
    ${CHIP_ROOT}/src/app/server-cluster/ServerClusterInterfaceRegistry.cpp
    ${CHIP_ROOT}/src/app/StorageDelegateWrapper.cpp
    ${CHIP_ROOT}/src/app/persistence/DefaultAttributePersistenceProvider.cpp

    # all-devices-common
    ${ALL_DEVICES_DEVICE_SOURCES}
    ${ALL_DEVICES_CLUSTER_SOURCES}
)

if(CONFIG_ALL_DEVICES_ENABLE_THREAD)
    target_sources(app PRIVATE
        ${ALL_DEVICES_COMMON_DIR}/device/types/root-node/ThreadRootNode.h
        ${ALL_DEVICES_COMMON_DIR}/device/types/root-node/ThreadRootNode.cpp
    )
endif()

# TODO wifi

if(CONFIG_CHIP_OTA_REQUESTOR)

    target_sources(app PRIVATE
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/BDXDownloader.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestor.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestorDriver.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestorStorage.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/OTARequestorAttributes.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/OTARequestorCluster.cpp
    )

    include(${CHIP_ROOT}/config/zephyr/ota-image.cmake)

    if(CONFIG_MCUBOOT_SIGNATURE_KEY_FILE STREQUAL "")
        set(ALL_DEVICES_OTA_BASE_NAME "zephyr")
    else()
        set(ALL_DEVICES_OTA_BASE_NAME "zephyr.signed")
    endif()

    if(CONFIG_CHIP_OTA_IMAGE_BUILD)
        chip_ota_image(chip-ota-image
            INPUT_FILES ${PROJECT_BINARY_DIR}/zephyr/${ALL_DEVICES_OTA_BASE_NAME}.bin
            OUTPUT_FILE ${PROJECT_BINARY_DIR}/zephyr/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME}
        )
    endif()
endif()
