/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#include "AppEvent.h"
#include "binding-handler.h"

#include <app/util/attribute-storage.h>

#include "AppTaskCommon.cpp" // include the common file that is used for all examples

namespace {
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;
} // namespace

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err;

    LOG_INF("SW Version: %u, %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initialize status LED
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    LEDWidget::InitGpio(LEDS_PORT);
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
    sStatusLED.Init(SYSTEM_STATE_LED);

    UpdateStatusLED();
#endif

    InitButtons();

    // Initialize function button timer
    k_timer_init(&sFactoryResetTimer, &AppTask::FactoryResetTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFactoryResetTimer, this);

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

#if CONFIG_CHIP_OTA_REQUESTOR
    InitBasicOTARequestor();
#endif

    // We only have network commissioning on endpoint 0.
    // Set up a valid Network Commissioning cluster on endpoint 0 is done in
    // src/platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.cpp
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    ConfigurationMgr().LogDeviceConfig();

    // Configure Bindings
    err = InitBindingHandlers();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitBindingHandlers fail");
        return err;
    }

    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = ConnectivityMgr().SetBLEDeviceName("TelinkMinApp");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(new AppFabricTableDelegate);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppFabricTableDelegate fail");
        return err;
    }

    return CHIP_NO_ERROR;
}


void AppTask::InitButtons(void)
{
#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
    sFactoryResetButton.Configure(BUTTON_PORT, BUTTON_PIN_1, FactoryResetButtonEventHandler);
    sBleAdvStartButton.Configure(BUTTON_PORT, BUTTON_PIN_4, StartBleAdvButtonEventHandler);
#else
    sFactoryResetButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_1, FactoryResetButtonEventHandler);
    sBleAdvStartButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_2, StartBleAdvButtonEventHandler);
#endif

    ButtonManagerInst().AddButton(sFactoryResetButton);
    ButtonManagerInst().AddButton(sBleAdvStartButton);
}
