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

# ****************************************************************
# Common Application Files
# ****************************************************************

if (NOT DEFINED EXAMPLE_PLATFORM_NXP_COMMON_DIR)
    get_filename_component(EXAMPLE_PLATFORM_NXP_COMMON_DIR ${CHIP_ROOT}/examples/platform/nxp/common REALPATH)
endif()

if(NOT DEFINED EXAMPLE_NXP_PLATFORM_DIR)
    get_filename_component(EXAMPLE_NXP_PLATFORM_DIR ${CHIP_ROOT}/examples/platform/nxp/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME} REALPATH)
endif()

if (CONFIG_CHIP_APP_COMMON)
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_task/source/AppTaskBase.cpp
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_callbacks/source/CommonDeviceCallbacks.cpp
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_manager/source/CHIPDeviceManager.cpp
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/icd/source/ICDUtil.cpp
    )
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_config
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_task/include
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_callbacks/include
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_manager/include
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/factory_data/include
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/icd/include
    )
    target_compile_definitions(app
        PRIVATE
        APP_QUEUE_TICKS_TO_WAIT=${CONFIG_CHIP_APP_QUEUE_TICKS_TO_WAIT}
    )

    if (CONFIG_APP_FREERTOS_OS)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_task/source/AppTaskFreeRTOS.cpp
        )
    else()
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_task/source/AppTaskZephyr.cpp
        )
    endif()
endif()

if (CONFIG_CHIP_APP_CLUSTERS)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/clusters/include
    )
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/clusters/source/ZclCallbacks.cpp
    )
endif()

if (CONFIG_CHIP_APP_ASSERT)
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_assert/source/AppAssert.cpp
    )
endif()

if (CONFIG_CHIP_APP_BLE_MANAGER)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_ble/include
    )
    if (CONFIG_CHIP_APP_BLE_MANAGER_CUSTOM)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_ble/source/BleZephyrManagerApp.cpp
        )
    else()
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_ble/source/BLEApplicationManagerEmpty.cpp
        )
    endif()
endif()

if (CONFIG_DIAG_LOGS_DEMO)
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/diagnostic_logs/source/DiagnosticLogsDemo.cpp
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/diagnostic_logs/source/DiagnosticLogsProviderDelegateImpl.cpp
    )
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/diagnostic_logs/include
    )
endif()

if (CONFIG_CHIP_APP_FACTORY_DATA)
    if (CONFIG_CHIP_APP_FACTORY_DATA_IMPL_PLATFORM)
        target_sources(app PRIVATE
            ${EXAMPLE_NXP_PLATFORM_DIR}/factory_data/source/AppFactoryDataExample.cpp
        )
        if (CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA)
            target_compile_definitions(app PRIVATE
                ENABLE_SECURE_WHOLE_FACTORY_DATA
            )
        endif()
    elseif (CONFIG_CHIP_APP_FACTORY_DATA_IMPL_COMMON)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/factory_data/source/AppFactoryDataDefaultImpl.cpp
        )
    endif()
endif()

if (CONFIG_CHIP_APP_LED_WIDGET)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/led_widget/include
    )
endif()

if (CONFIG_CHIP_APP_LOW_POWER)
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/low_power/source/LowPower.cpp
    )
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/low_power/include
    )
endif()

# Button module empty would be chosen in case the app does not support button.
target_include_directories(app PRIVATE
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/include
)

if (CONFIG_CHIP_APP_BUTTON_REGISTRATION_DEFAULT)
    set(button_registration_source ButtonRegistrationDefault.cpp)
elseif(CONFIG_CHIP_APP_BUTTON_REGISTRATION_APP_AND_BLE)
    set(button_registration_source ButtonRegistrationAppAndBle.cpp)
elseif(CONFIG_CHIP_APP_BUTTON_REGISTRATION_APP_ONLY)
    set(button_registration_source ButtonRegistrationAppOnly.cpp)
else ()
    set(button_registration_source ButtonRegistrationEmpty.cpp)
endif()

target_sources(app PRIVATE
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/source/${button_registration_source}
)

if (NOT CONFIG_CHIP_APP_BUTTON_REGISTRATION_EMPTY)
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/source/ButtonManager.cpp
    )
endif()

if (CONFIG_CHIP_APP_BUTTON)
    if (CONFIG_CHIP_APP_BUTTON_APP)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/source/ButtonApp.cpp
        )
    endif()
    if (CONFIG_CHIP_APP_BUTTON_BLE)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/source/ButtonBle.cpp
        )
    endif()
    if (CONFIG_CHIP_APP_BUTTON_WITH_TIMER)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/source/ButtonWithTimer.cpp
        )
    endif()
endif()

if (CONFIG_CHIP_APP_CLI)
    target_compile_definitions(app PRIVATE
        ENABLE_CHIP_SHELL
    )
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_cli/include
        ${CHIP_ROOT}/examples/shell/shell_common/include
    )
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_cli/source/AppCLIBase.cpp
        ${CHIP_ROOT}/examples/shell/shell_common/cmd_misc.cpp
        ${CHIP_ROOT}/examples/shell/shell_common/cmd_otcli.cpp
        ${CHIP_ROOT}/examples/shell/shell_common/cmd_server.cpp
    )
    if (CONFIG_APP_FREERTOS_OS)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_cli/source/AppCLIFreeRTOS.cpp
        )
    else()
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_cli/source/AppCLIZephyr.cpp
        )
    endif()
endif()

if (CONFIG_CHIP_APP_OPERATIONAL_KEYSTORE)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/operational_keystore/include
    )
    if (CONFIG_CHIP_APP_OPERATIONAL_KEYSTORE_S200)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/operational_keystore/source/OperationalKeystoreS200.cpp
        )
    elseif (CONFIG_CHIP_APP_OPERATIONAL_KEYSTORE_EMPTY)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/operational_keystore/source/OperationalKeystoreEmpty.cpp
        )
    endif()
endif()

if (CONFIG_CHIP_APP_OTA_REQUESTOR)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/include
    )
    if (CONFIG_CHIP_APP_OTA_REQUESTOR_INITIATOR_FREERTOS)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/source/OTARequestorInitiatorCommon.cpp
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/source/OTARequestorInitiator.cpp
        )
    elseif (CONFIG_CHIP_APP_OTA_REQUESTOR_INITIATOR_ZEPHYR)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/source/OTARequestorInitiatorCommon.cpp
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/source/OTARequestorInitiatorZephyr.cpp
        )
    elseif (CONFIG_CHIP_APP_OTA_REQUESTOR_INITIATOR_MULTI_IMAGE)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/source/OTARequestorInitiatorMultiImage.cpp
        )
    endif()
    if (CONFIG_CHIP_APP_PLATFORM_OTA_UTILS)
        target_sources(app PRIVATE
            # Use the example provided by mcxw71_k32w1 platform until a common solution is proposed.
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/../mcxw71_k32w1/ota/OtaUtils.cpp
        )
    endif()
endif()

if (CONFIG_CHIP_APP_RPC)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/rpc/include
    )
endif()

if (CONFIG_CHIP_APP_UI_FEEDBACK)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ui_feedback/include
    )
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ui_feedback/source/LedManager.cpp
    )
    if (CONFIG_CHIP_APP_PLATFORM_LED_ON_OFF)
        if(EXISTS ${EXAMPLE_NXP_PLATFORM_DIR}/util/LedOnOff.cpp)
            target_sources(app PRIVATE
                ${EXAMPLE_NXP_PLATFORM_DIR}/util/LedOnOff.cpp
            )
        else()
            target_sources(app PRIVATE
                # Use the example provided by mcxw71_k32w1 platform until a common solution is proposed.
                ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/../mcxw71_k32w1/util/LedOnOff.cpp
            )
        endif()
    endif()
endif()

if (CONFIG_CHIP_APP_WIFI_CONNECT)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/wifi_connect/include
    )
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/wifi_connect/source/WifiConnect.cpp
    )
endif()
