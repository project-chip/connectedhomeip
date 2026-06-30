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
# Parallel to all-devices-common/device-factory/enabled_devices.gni for the GN build.
#
# Callers must define ALL_DEVICES_COMMON_DIR before including this file.
#
# Exports:
#   ALL_DEVICES_DEVICE_SRCDIRS  — list of device module source directories
#   ALL_DEVICES_DEVICE_SOURCES  — list of device module source files (for non-component builds)
#
# After including this file, callers must append ${CMAKE_CURRENT_BINARY_DIR}
# to their include-directory list so that the generated
# app_config/enabled_devices.h is reachable as <app_config/enabled_devices.h>.

# ---------------------------------------------------------------------------
# Source files for devices and common interfaces (for non-component CMake builds).
# Excludes root-node specialization files (Thread/WiFi) which require platform
# specific selection.
# ---------------------------------------------------------------------------
set(ALL_DEVICES_DEVICE_SOURCES
    # keep-sorted: start
    "${ALL_DEVICES_COMMON_DIR}/oob-accessors/OOBDataSerializer.cpp"
    "${ALL_DEVICES_COMMON_DIR}/oob-accessors/boolean-state-sensor/BooleanStateSensorAccessor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/aggregator/AggregatorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/air-purifier/AirPurifierDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/air-purifier/impl/LoggingAirPurifierDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/air-quality-sensor/AirQualitySensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/boolean-state-sensor/BooleanStateSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/bridged-node/BridgedNodeDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/chime/ChimeDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/cooktop/CooktopDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/cooktop/impl/LoggingCooktopDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/device-energy-management/DeviceEnergyManagementDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/dimmable-light/DimmableLightDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/dimmable-light/impl/LoggingDimmableLightDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/dimmable-plug-in-unit/DimmablePlugInUnitDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/extractor-hood/ExtractorHoodDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/fan/FanDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/fan/impl/LoggingFanDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/flow-sensor/FlowSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/flow-sensor/impl/IncreasingFlowSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/generic-switch/GenericSwitchDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/humidity-sensor/HumiditySensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/humidity-sensor/impl/IncreasingHumiditySensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/light-sensor/LightSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/light-sensor/impl/IncreasingLightSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/mounted-dimmable-load-control/MountedDimmableLoadControlDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/mounted-on-off-control/MountedOnOffControlDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/network-infrastructure-manager/NetworkInfrastructureManagerDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/occupancy-sensor/OccupancySensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/occupancy-sensor/impl/LoggingOccupancySensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/on-off-light/impl/LoggingOnOffLightDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/on-off-light-switch/OnOffLightSwitchDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/on-off-plug-in-unit/OnOffPlugInUnitDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/oven/OvenDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/oven/impl/LoggingOvenDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/power-source/BatteryPowerSourceDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/power-source/impl/DecreasingBatteryPowerSourceDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/pressure-sensor/PressureSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/pressure-sensor/impl/IncreasingPressureSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/proximity-ranger/ProximityRangerDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/refrigerator/RefrigeratorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/refrigerator/impl/LoggingRefrigeratorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/proximity-ranger/impl/LoggingProximityRangerDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/proximity-ranger/impl/LoggingRangingAdapter.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/root-node/RootNodeDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/smoke-co-alarm/impl/LoggingOnlySmokeCoAlarmDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/smoke-co-alarm/SmokeCoAlarmDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/soil-sensor/SoilSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/soil-sensor/impl/IncreasingMoistureSoilSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/speaker/SpeakerDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/speaker/impl/LoggingSpeakerDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/temperature-controlled-cabinet/TemperatureControlledCabinetPart.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/temperature-controlled-cabinet/impl/LoggingTemperatureControlledCabinetPart.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/temperature-sensor/TemperatureSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/temperature-sensor/impl/IncreasingTemperatureSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/water-valve/WaterValveDevice.cpp"
    # keep-sorted: end

    # Baseline for devices (not real device types)
    # keep-sorted: start
    "${ALL_DEVICES_COMMON_DIR}/devices/capabilities/dimmable-load/DimmableLoadDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/capabilities/dimmable-load/impl/LoggingDimmableLoadDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/endpoint-id-allocator/DynamicEndpointIdAllocator.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/capabilities/fan-load/FanLoadDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/capabilities/fan-load/impl/LoggingFanLoadDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/interface/DeviceInterface.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/interface/SingleEndpointDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/capabilities/on-off-load/OnOffLoadDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/capabilities/on-off-load/impl/LoggingOnOffLoadDevice.cpp"
    # keep-sorted: end
)

# ---------------------------------------------------------------------------
# Source directories (unconditional — all device sources are always compiled;
# LTO eliminates unreachable device code when only a subset is registered).
# Derived automatically from ALL_DEVICES_DEVICE_SOURCES.
# ---------------------------------------------------------------------------
set(ALL_DEVICES_DEVICE_SRCDIRS "")
foreach(_src IN LISTS ALL_DEVICES_DEVICE_SOURCES)
    get_filename_component(_dir "${_src}" DIRECTORY)
    list(APPEND ALL_DEVICES_DEVICE_SRCDIRS "${_dir}")
endforeach()
list(REMOVE_DUPLICATES ALL_DEVICES_DEVICE_SRCDIRS)

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
        aggregator
        air-purifier
        air-quality-sensor
        bridged-node
        chime
        contact-sensor
        cooktop
        device-energy-management
        dimmable-light
        dimmable-plug-in-unit
        extractor-hood
        fan
        flow-sensor
        generic-switch
        humidity-sensor
        light-sensor
        mounted-dimmable-load-control
        mounted-on-off-control
        network-infrastructure-manager
        occupancy-sensor
        on-off-light
        on-off-light-switch
        on-off-plug-in-unit
        oven
        power-source
        pressure-sensor
        proximity-ranger
        rain-sensor
        refrigerator
        smoke-co-alarm
        soil-sensor
        speaker
        temperature-sensor
        water-freeze-detector
        water-leak-detector
        water-valve
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

# ---------------------------------------------------------------------------
# Source files for clusters that are not included in the default SDK build
# but are required by enabled devices (e.g. Binding cluster for client switches).
# ---------------------------------------------------------------------------
set(ALL_DEVICES_CLUSTER_SOURCES
    "${CHIP_ROOT}/src/app/clusters/bindings/BindingCluster.cpp"
    "${CHIP_ROOT}/src/app/clusters/bindings/BindingManager.cpp"
    "${CHIP_ROOT}/src/app/clusters/bindings/binding-table.cpp"
    "${CHIP_ROOT}/src/app/clusters/bindings/PendingNotificationMap.cpp"
)
