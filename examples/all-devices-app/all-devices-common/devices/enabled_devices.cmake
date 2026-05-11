#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
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

# Authoritative device configuration for the CMake (ESP32) build.
# Parallel to all-devices-common/devices/enabled_devices.gni for the GN build.
#
# Callers must define ALL_DEVICES_COMMON_DIR before including this file.
#
# Exports:
#   ALL_DEVICES_DEVICE_SRCDIRS  — list of device module source directories
#
# After including this file, callers must append ${CMAKE_CURRENT_BINARY_DIR}
# to their include-directory list so that the generated
# app_config/enabled_devices.h is reachable as <app_config/enabled_devices.h>.

# ---------------------------------------------------------------------------
# Source directories (unconditional — all device sources are always compiled;
# LTO eliminates unreachable device code when only a subset is registered).
# ---------------------------------------------------------------------------
set(ALL_DEVICES_DEVICE_SRCDIRS
    # keep-sorted: start
    "${ALL_DEVICES_COMMON_DIR}/devices/boolean-state-sensor"
    "${ALL_DEVICES_COMMON_DIR}/devices/chime"
    "${ALL_DEVICES_COMMON_DIR}/devices/dimmable-light"
    "${ALL_DEVICES_COMMON_DIR}/devices/dimmable-light/impl"
    "${ALL_DEVICES_COMMON_DIR}/devices/interface"
    "${ALL_DEVICES_COMMON_DIR}/devices/occupancy-sensor"
    "${ALL_DEVICES_COMMON_DIR}/devices/occupancy-sensor/impl"
    "${ALL_DEVICES_COMMON_DIR}/devices/on-off-light"
    "${ALL_DEVICES_COMMON_DIR}/devices/root-node"
    "${ALL_DEVICES_COMMON_DIR}/devices/soil-sensor"
    "${ALL_DEVICES_COMMON_DIR}/devices/soil-sensor/impl"
    "${ALL_DEVICES_COMMON_DIR}/devices/speaker"
    "${ALL_DEVICES_COMMON_DIR}/devices/speaker/impl"
    "${ALL_DEVICES_COMMON_DIR}/devices/temperature-sensor"
    "${ALL_DEVICES_COMMON_DIR}/devices/temperature-sensor/impl"
    # keep-sorted: end
)

# ---------------------------------------------------------------------------
# Device selection.
# ALL_DEVICES_ENABLED_DEVICES: semicolon-separated list of device keys to
# enable.  Empty (the default) means all devices are enabled.
# ---------------------------------------------------------------------------
if(NOT DEFINED ALL_DEVICES_ENABLED_DEVICES)
    set(ALL_DEVICES_ENABLED_DEVICES "")
endif()

# Determine whether every device is enabled.
if(ALL_DEVICES_ENABLED_DEVICES)
    set(_all_devices_all OFF)
else()
    set(_all_devices_all ON)
endif()

# Initialise all per-device variables to 0, then set the enabled ones to 1.
# The macro name is derived from the registry key: "foo-bar" → ALL_DEVICES_ENABLE_FOO_BAR.
#
# IMPORTANT:
#   - Keep list in sync with enabled_devices.gni
#   - ensure enabled_devices_config.h.in contains required ALL_DEVICES_ENABLE* defines
#   - Update scripts/build/build/targets.py to include the new device
foreach(_key
        # keep-sorted: start
        chime
        contact-sensor
        dimmable-light
        occupancy-sensor
        on-off-light
        soil-sensor
        speaker
        temperature-sensor
        water-leak-detector
        # keep-sorted: end
    )
    string(REPLACE "-" "_" _suffix "${_key}")
    string(TOUPPER "${_suffix}" _suffix)
    if(_all_devices_all)
        set("ALL_DEVICES_ENABLE_${_suffix}" 1)
    else()
        set("ALL_DEVICES_ENABLE_${_suffix}" 0)
    endif()
endforeach()

if(NOT _all_devices_all)
    foreach(_device ${ALL_DEVICES_ENABLED_DEVICES})
        string(REPLACE "-" "_" _suffix "${_device}")
        string(TOUPPER "${_suffix}" _suffix)
        set("ALL_DEVICES_ENABLE_${_suffix}" 1)
    endforeach()
endif()

# ---------------------------------------------------------------------------
# Generate app_config/enabled_devices.h.
# Guarded by CMAKE_BUILD_EARLY_EXPANSION: during the ESP-IDF component
# requirements phase, CMake processes this file in a temporary build tree.
# configure_file() must only run during the real configure pass.
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
# Compute the application output name.
# Mirrors the three-way logic in enabled_devices.gni.
# ALL_DEVICES_APP_NAME may be passed as a -D flag to override the default.
# ---------------------------------------------------------------------------
if(NOT DEFINED ALL_DEVICES_APP_NAME)
    set(ALL_DEVICES_APP_NAME "")
endif()

if(ALL_DEVICES_APP_NAME)
    set(ALL_DEVICES_COMPUTED_NAME "${ALL_DEVICES_APP_NAME}")
elseif(ALL_DEVICES_ENABLED_DEVICES)
    set(ALL_DEVICES_COMPUTED_NAME "example-device-app")
else()
    set(ALL_DEVICES_COMPUTED_NAME "all-devices-app")
endif()

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/enabled_devices_config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/app_config/enabled_devices.h"
    )
endif()
