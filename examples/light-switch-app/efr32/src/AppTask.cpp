/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**********************************************************
 * Includes
 *********************************************************/

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "LEDWidget.h"
#include "binding-handler.h"
#include "sl_simple_led_instances.h"

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/EFR32/freertos_bluetooth.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <app/clusters/identify-server/identify-server.h>

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define SYSTEM_STATE_LED &sl_led_led0

#define APP_FUNCTION_BUTTON &sl_button_btn0
#define APP_LIGHT_SWITCH &sl_button_btn1

using namespace chip;
using namespace ::chip::DeviceLayer;

namespace {

/**********************************************************
 * Variable declarations
 *********************************************************/

EmberAfIdentifyEffectIdentifier sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;

bool mCurrentButtonState = false;

/**********************************************************
 * Identify Callbacks
 *********************************************************/

namespace {
void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(Zcl, "Trigger Identify Complete");
    sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;

#if CHIP_DEVICE_CONFIG_ENABLE_SED == 1
    AppTask::GetAppTask().StopStatusLEDTimer();
#endif
}
} // namespace

void OnTriggerIdentifyEffect(Identify * identify)
{
    ChipLogProgress(Zcl, "Trigger Identify Effect");
    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mCurrentEffectIdentifier == EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE)
    {
        ChipLogProgress(Zcl, "IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE - Not supported, use effect varriant %d",
                        identify->mEffectVariant);
        sIdentifyEffect = static_cast<EmberAfIdentifyEffectIdentifier>(identify->mEffectVariant);
    }

#if CHIP_DEVICE_CONFIG_ENABLE_SED == 1
    AppTask::GetAppTask().StartStatusLEDTimer();
#endif

    switch (sIdentifyEffect)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
    }
}

Identify gIdentify = {
    chip::EndpointId{ 1 },
    AppTask::GetAppTask().OnIdentifyStart,
    AppTask::GetAppTask().OnIdentifyStop,
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnTriggerIdentifyEffect,
};

} // namespace

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

/**********************************************************
 * AppTask Definitions
 *********************************************************/

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "Light Switch");
#endif

    err = BaseApplication::Init(&gIdentify);
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("BaseApplication::Init() failed");
        appError(err);
    }

    // Configure Bindings - TODO ERROR PROCESSING
    err = InitBindingHandler();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("InitBindingHandler() failed");
        appError(err);
    }

    return err;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    QueueHandle_t sAppEventQueue = *(static_cast<QueueHandle_t *>(pvParameter));

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_DEVICE_CONFIG_ENABLE_SED) && CHIP_DEVICE_CONFIG_ENABLE_SED)
    sAppTask.StartStatusLEDTimer();
#endif

    EFR32_LOG("App Task started");
    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

void AppTask::OnIdentifyStart(Identify * identify)
{
    ChipLogProgress(Zcl, "onIdentifyStart");

#if CHIP_DEVICE_CONFIG_ENABLE_SED == 1
    sAppTask.StartStatusLEDTimer();
#endif
}

void AppTask::OnIdentifyStop(Identify * identify)
{
    ChipLogProgress(Zcl, "onIdentifyStop");

#if CHIP_DEVICE_CONFIG_ENABLE_SED == 1
    sAppTask.StopStatusLEDTimer();
#endif
}

void AppTask::SwitchActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        BindingCommandData * data = Platform::New<BindingCommandData>();
        data->clusterId           = chip::app::Clusters::OnOff::Id;

        if (mCurrentButtonState)
        {
            mCurrentButtonState = false;
            data->commandId     = chip::app::Clusters::OnOff::Commands::Off::Id;
        }
        else
        {
            data->commandId     = chip::app::Clusters::OnOff::Commands::On::Id;
            mCurrentButtonState = true;
        }

#ifdef DISPLAY_ENABLED
        sAppTask.GetLCD().WriteDemoUI(mCurrentButtonState);
#endif

        DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    }
}

void AppTask::ButtonEventHandler(const sl_button_t * buttonHandle, uint8_t btnAction)
{
    if (buttonHandle == NULL)
    {
        return;
    }

    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (buttonHandle == APP_LIGHT_SWITCH && btnAction == SL_SIMPLE_BUTTON_PRESSED)
    {
        button_event.Handler = SwitchActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (buttonHandle == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        sAppTask.PostEvent(&button_event);
    }
}
