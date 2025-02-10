/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "AppTask.h"
#include "AppConfig.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "LEDWidget.h"
#include "qrcodegen.h"
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <assert.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/ASR/NetworkCommissioningDriver.h>

#include "init_Matter.h"
#ifdef CONFIG_ENABLE_CHIP_SHELL
#include "matter_shell.h"
#endif
#include <app-common/zap-generated/attributes/Accessors.h>

namespace {
lega_thread_t sAppTaskHandle;
} // namespace

LEDWidget lightLED;

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

AppTask AppTask::sAppTask;

namespace {
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain /* Endpoint Id */,
                                      &(NetworkCommissioning::ASRWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
}

static DeviceCallbacks EchoCallbacks;

CHIP_ERROR AppTask::StartAppTask()
{
    // Start App task.
    OSStatus result = lega_rtos_create_thread(&sAppTaskHandle, 2, APP_TASK_NAME, (lega_thread_function_t) AppTaskMain,
                                              APP_TASK_STACK_SIZE, (lega_thread_arg_t) this);
    return (result == kNoErr) ? CHIP_NO_ERROR : CHIP_APPLICATION_ERROR(0x02);
}

bool IsLightOn()
{
    Protocols::InteractionModel::Status status;
    bool on = true;
    status  = app::Clusters::OnOff::Attributes::OnOff::Get(1, &on);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ASR_LOG("Error Read OnOff Attribute 0x%02x\n", to_underlying(status));
    }

    return on != false;
}

uint8_t GetLightLevel()
{
    Protocols::InteractionModel::Status status;
    app::DataModel::Nullable<uint8_t> currentLevel;

    status = app::Clusters::LevelControl::Attributes::CurrentLevel::Get(1, currentLevel);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ASR_LOG("Error Read CurrentLevel Attribute 0x%02x\n", to_underlying(status));
        return -1;
    }

    if (currentLevel.IsNull())
    {
        ASR_LOG("Error currentLevel is null\n");
        return -1;
    }

    return currentLevel.Value();
}

void led_startup_status()
{
    uint8_t currentLevel;

    if (IsLightOn() == true)
    {
        currentLevel = GetLightLevel();
        lightLED.Set(1);
        lightLED.SetBrightness(currentLevel);
    }
}

void AppTask::AppTaskMain(void * pvParameter)
{
    ASR_LOG("App Task started");

    if (MatterInitializer::Init_Matter_Stack(MATTER_DEVICE_NAME) != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    if (deviceMgr.Init(&EchoCallbacks) != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    if (MatterInitializer::Init_Matter_Server() != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    NetWorkCommissioningInstInit();

    ASR_LOG("Current Firmware Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    ConfigurationMgr().LogDeviceConfig();

    // Print setup info
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

#ifdef LIGHT_SELECT_RGB
    lightLED.RGB_init();
#else
    lightLED.Init(LIGHT_LED);
#endif

    /* get led onoff status and level value */
    led_startup_status();
#ifdef CONFIG_ENABLE_CHIP_SHELL
    RegisterLightCommands();
#endif
    /* Delete task */
    lega_rtos_delete_thread(NULL);
}
