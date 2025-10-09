/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "ButtonHandler.h"
#include <BindingHandler.h>
#include <LEDWidget.h>
#include <LightSwitch.h>
#include <app/clusters/identify-server/identify-server.h>
#include <cycfg.h>
#include <matter_config.h>
#include <wiced_device_config.h>

using namespace ::chip::DeviceLayer;
using namespace ::chip::app;

static bool sIsThreadProvisioned    = false;
static bool sIsThreadEnabled        = false;
static bool sIsThreadBLEAdvertising = false;
static bool sHaveBLEConnections     = false;

constexpr chip::EndpointId kLightDimmerSwitchEndpointId = 1;

static wiced_led_config_t chip_lighting_led_config[] = {
    {
        .led    = PLATFORM_LED_RED,
        .bright = 50,
    },
#if defined(PLATFORM_LED_YELLOW_ENABLED)
    {
        .led    = PLATFORM_LED_YELLOW,
        .bright = 50,
    },
#endif
};

void AppTaskMain(intptr_t args)
{
    wiced_result_t result;
    result = app_button_init();
    if (result != WICED_SUCCESS)
    {
        ChipLogError(Zcl, "ERROR app_button_init %d\n", result);
    }
    LEDWid().Init(chip_lighting_led_config, MATTER_ARRAY_SIZE(chip_lighting_led_config));
    LightSwitch::GetInstance().Init(kLightDimmerSwitchEndpointId);

    CYW30739MatterConfig::InitApp();
}

void OnIdentifyTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        break;
    }
}

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnIdentifyTriggerEffect,
};

void UpdateStatusLED()
{
#if defined(PLATFORM_LED_YELLOW_ENABLED)
    LEDWid().Set(false, PLATFORM_LED_YELLOW);
    // Status LED indicates:
    // - blinking 1 s - advertising, ready to commission
    // - blinking 200 ms - commissioning in progress
    // - constant lightning means commissioned with Thread network
    if (sIsThreadBLEAdvertising && !sHaveBLEConnections)
    {
        LEDWid().Blink(PLATFORM_LED_YELLOW, 50, 950);
    }
    else if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        LEDWid().Set(true, PLATFORM_LED_YELLOW);
    }
    else if (sHaveBLEConnections)
    {
        LEDWid().Blink(PLATFORM_LED_YELLOW, 50, 150);
    }
    else
    {
        /* back to default status */
        LEDWid().Set(false, PLATFORM_LED_YELLOW);
    }
#endif /* PLATFORM_LED_YELLOW_ENABLED */
}

void AppChipEventHandler(const ChipDeviceEvent * aEvent, intptr_t arg)
{
    switch (aEvent->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sIsThreadBLEAdvertising = true;
        sHaveBLEConnections     = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
        break;
    case DeviceEventType::kThreadStateChange:
        sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;
    case DeviceEventType::kThreadConnectivityChange:
        if (aEvent->ThreadConnectivityChange.Result == kConnectivity_Established)
            sHaveBLEConnections = true;
        break;
    default:
        if ((ConnectivityMgr().NumBLEConnections() == 0) && (!sIsThreadProvisioned || !sIsThreadEnabled))
        {
            printf("[Event] Commissioning with a Thread network has not been done. An error occurred\n");
            sIsThreadBLEAdvertising = false;
            sHaveBLEConnections     = false;
            UpdateStatusLED();
        }
        break;
    }
}

CHIP_ERROR StartBLEAdvertisingHandler()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    /// Don't allow on starting Matter service BLE advertising after Thread provisioning.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0)
    {
        printf("Matter service BLE advertising not started - device is already commissioned\n");
        return err;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        printf("BLE advertising is already enabled\n");
        return err;
    }
    else
    {
        printf("Start BLE advertising...\n");
        err = ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        if (err != CHIP_NO_ERROR)
        {
            printf("Enable BLE advertising failed\n");
            return err;
        }
    }

    printf("Enabling BLE advertising...\n");
    /* Check Commissioning Window*/
    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        printf("OpenBasicCommissioningWindow() failed\n");
        err = CHIP_ERROR_UNEXPECTED_EVENT;
    }

    return err;
}

CHIP_ERROR StopBLEAdvertisingHandler()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    printf("Stop BLE advertising...\n");
    err = ConnectivityMgr().SetBLEAdvertisingEnabled(false);
    if (err != CHIP_NO_ERROR)
    {
        printf("Disable BLE advertising failed\n");
        return err;
    }
    return err;
}
