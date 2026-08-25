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
cmake_minimum_required(VERSION 3.20)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/connectedhomeip REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(ZEPHYR_PLATFORM_COMMON ${CHIP_ROOT}/examples/platform/silabs/zephyr REALPATH)
get_filename_component(SILABS_DISHWASHER_COMMON ${CHIP_ROOT}/examples/dishwasher-app/silabs REALPATH)
get_filename_component(ENERGY_MGMT_DIR ${CHIP_ROOT}/examples/energy-management REALPATH)


# Need this as wiseconnect abstraction is causing a build failure
add_definitions(-DMBEDTLS_X509_REMOVE_INFO )

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/silabs)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

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

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    foreach(_mbedtls_lib mbedtls mbedx509 tfpsacrypto builtin p256-m pqcp extras platform utilities)
        if(TARGET ${_mbedtls_lib})
            target_compile_options(${_mbedtls_lib} PRIVATE -Wno-documentation -Wno-c99-extensions)
        endif()
    endforeach()

    if(CONFIG_OPENTHREAD)
        foreach(_ot_lib openthread-ftd openthread-mtd openthread-radio openthread-cli-ftd openthread-cli-mtd)
            if(TARGET ${_ot_lib})
                target_compile_options(${_ot_lib} PRIVATE -Wno-c99-extensions)
            endif()
        endforeach()
    endif()
endif()

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=format)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)
endif()

project(matter-zephyr-dishwasher)

include(${CHIP_ROOT}/config/silabs/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# Prefer examples/energy-management electrical-sensor headers over the older
# FreeRTOS dishwasher copies of the same filenames (ElectricalSensorManager.h,
# PowerTopologyDelegateImpl.h) so shared FakeReadings/DEM APIs resolve correctly.
target_include_directories(app
    PRIVATE
    include
    ${ENERGY_MGMT_DIR}/device-energy-management/include
    ${ENERGY_MGMT_DIR}/electrical-sensor/include
    ${ENERGY_MGMT_DIR}/utils/include
    ${ENERGY_MGMT_DIR}/energy-management-triggers
    ${SILABS_DISHWASHER_COMMON}/include
    ${ZEPHYR_PLATFORM_COMMON}
)

target_sources(app
    PRIVATE
    src/AppTask.cpp
    src/DeviceCallbacks.cpp
    ../include/CHIPProjectConfig.h
    ${ZEPHYR_PLATFORM_COMMON}/main.cpp
    ${ZEPHYR_PLATFORM_COMMON}/AppTaskBase.cpp
    ${ZEPHYR_PLATFORM_COMMON}/AppTaskZephyr.cpp
    ${ZEPHYR_PLATFORM_COMMON}/CHIPDeviceManager.cpp
    ${ZEPHYR_PLATFORM_COMMON}/CommonDeviceCallbacks.cpp
    ${ZEPHYR_PLATFORM_COMMON}/DataModelCallbacks.cpp

    # Dishwasher-specific logic (FreeRTOS AppTask/DataModelCallbacks are not used;
    # Zephyr owns those entry points under src/).
    ${SILABS_DISHWASHER_COMMON}/src/DataModelHelper.cpp
    ${SILABS_DISHWASHER_COMMON}/src/DeviceEnergyManager.cpp
    ${SILABS_DISHWASHER_COMMON}/src/DishwasherManager.cpp
    ${SILABS_DISHWASHER_COMMON}/src/operational-state-delegate-impl.cpp

    # Shared energy-management (replaces stale energy-management-app/... paths in BUILD.gn)
    ${ENERGY_MGMT_DIR}/device-energy-management/src/DeviceEnergyManagementDelegateImpl.cpp
    ${ENERGY_MGMT_DIR}/device-energy-management/src/DeviceEnergyManagementManager.cpp
    ${ENERGY_MGMT_DIR}/device-energy-management/src/device-energy-management-mode.cpp
    ${ENERGY_MGMT_DIR}/electrical-sensor/src/ElectricalEnergyMeasurementDelegateImpl.cpp
    ${ENERGY_MGMT_DIR}/electrical-sensor/src/ElectricalPowerMeasurementDelegateImpl.cpp
    ${ENERGY_MGMT_DIR}/electrical-sensor/src/ElectricalSensorManager.cpp
    ${ENERGY_MGMT_DIR}/electrical-sensor/src/PowerTopologyDelegateImpl.cpp
    ${ENERGY_MGMT_DIR}/utils/src/EnergyTimeUtils.cpp

    # Test-event triggers (SystemLayer timers; need GetESManager from AppTask)
    ${ENERGY_MGMT_DIR}/energy-management-triggers/DEMTestEventTriggers.cpp
    ${ENERGY_MGMT_DIR}/energy-management-triggers/EnergyReportingEventTriggers.cpp
    ${ENERGY_MGMT_DIR}/energy-management-triggers/FakeReadings.cpp
)

if(CONFIG_CHIP_FACTORY_DATA)
    target_sources(app PRIVATE
                    ${CHIP_ROOT}/examples/platform/silabs/provision/ProvisionStorageZephyr.cpp
                    ${CHIP_ROOT}/examples/platform/silabs/provision/ProvisionStorageCustom.cpp
    )
    target_include_directories(app PRIVATE
                    ${CHIP_ROOT}/third_party/silabs/matter_support/provision
    )
    target_compile_definitions(app PRIVATE
        SL_PROVISION_GENERATOR=0
    )
endif()

if(CONFIG_CHIP_OTA_REQUESTOR)
    target_sources(app PRIVATE
                    ${ZEPHYR_PLATFORM_COMMON}/ota_requestor/source/OTARequestorInitiatorCommon.cpp
                    ${ZEPHYR_PLATFORM_COMMON}/ota_requestor/source/OTARequestorInitiatorZephyr.cpp
    )
    target_include_directories(app PRIVATE
                    ${ZEPHYR_PLATFORM_COMMON}/ota_requestor/include/
    )
endif()

if(CONFIG_CHIP_WIFI)
  chip_configure_data_model(app INCLUDE_SERVER
      ZAP_FILE ${SILABS_DISHWASHER_COMMON}/data_model/dishwasher-wifi-app.zap
  )
else()
  chip_configure_data_model(app INCLUDE_SERVER
      ZAP_FILE ${SILABS_DISHWASHER_COMMON}/data_model/dishwasher-thread-app.zap
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

include(${CHIP_ROOT}/config/silabs/app/zephyr-post-build.cmake)
