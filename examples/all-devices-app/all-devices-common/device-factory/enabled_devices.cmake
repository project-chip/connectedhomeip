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
    "${ALL_DEVICES_COMMON_DIR}/device/types/aggregator/Aggregator.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/air-purifier/AirPurifier.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/air-purifier/impl/LoggingAirPurifier.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/air-quality-sensor/AirQualitySensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/boolean-state-sensor/BooleanStateSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/bridged-node/BridgedNode.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/chime/Chime.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/cooktop/Cooktop.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/cooktop/impl/LoggingCooktop.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/device-energy-management/EnergyManagement.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/dimmable-light/DimmableLight.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/dimmable-light/impl/LoggingDimmableLight.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/dimmable-plug-in-unit/DimmablePlugInUnit.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/dishwasher/Dishwasher.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/extractor-hood/ExtractorHood.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/fan/Fan.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/fan/impl/LoggingFan.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/flow-sensor/FlowSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/flow-sensor/impl/IncreasingFlowSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/generic-switch/GenericSwitch.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/humidity-sensor/HumiditySensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/humidity-sensor/impl/IncreasingHumiditySensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/laundry-dryer/LaundryDryer.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/laundry-washer/LaundryWasher.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/light-sensor/LightSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/light-sensor/impl/IncreasingLightSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/microwave-oven/MicrowaveOven.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/mounted-dimmable-load-control/MountedDimmableLoadControl.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/mounted-on-off-control/MountedOnOffControl.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/network-infrastructure-manager/NetworkInfrastructureManager.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/occupancy-sensor/OccupancySensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/occupancy-sensor/impl/LoggingOccupancySensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/on-off-light/impl/LoggingOnOffLight.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/on-off-light-switch/OnOffLightSwitch.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/on-off-plug-in-unit/OnOffPlugInUnit.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/oven/Oven.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/oven/impl/LoggingOven.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/power-source/BatteryPowerSource.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/power-source/impl/DecreasingBatteryPowerSource.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/pressure-sensor/PressureSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/pressure-sensor/impl/IncreasingPressureSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/proximity-ranger/ProximityRanger.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/refrigerator/Refrigerator.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/refrigerator/impl/LoggingRefrigerator.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/proximity-ranger/impl/LoggingProximityRanger.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/proximity-ranger/impl/LoggingRangingAdapter.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/root-node/RootNode.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/smoke-co-alarm/impl/LoggingOnlySmokeCoAlarm.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/smoke-co-alarm/SmokeCoAlarm.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/soil-sensor/SoilSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/soil-sensor/impl/IncreasingMoistureSoilSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/speaker/Speaker.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/speaker/impl/LoggingSpeaker.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/temperature-controlled-cabinet/TemperatureControlledCabinetPart.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/temperature-controlled-cabinet/impl/LoggingTemperatureControlledCabinetPart.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/temperature-sensor/TemperatureSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/temperature-sensor/impl/IncreasingTemperatureSensor.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/water-valve/WaterValve.cpp"
    # keep-sorted: end

    # Baseline for devices (not real device types)
    # keep-sorted: start
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/dimmable-load/DimmableLoad.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/dimmable-load/impl/LoggingDimmableLoad.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/fan-load/FanLoad.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/fan-load/impl/LoggingFanLoad.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/on-off-load/OnOffLoad.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/on-off-load/impl/LoggingOnOffLoad.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/operational-state/impl/LoggingOperationalStateDelegate.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/operational-state/impl/LoggingRvcOperationalStateDelegate.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/api/allocator/DynamicEndpointIdAllocator.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/api/Interface.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/api/SingleEndpoint.cpp"
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
        dishwasher
        extractor-hood
        fan
        flow-sensor
        generic-switch
        humidity-sensor
        laundry-dryer
        laundry-washer
        light-sensor
        microwave-oven
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
        robotic-vacuum-cleaner
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
