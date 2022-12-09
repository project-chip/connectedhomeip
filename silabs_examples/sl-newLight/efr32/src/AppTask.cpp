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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "LEDWidget.h"
#ifdef RGB_LED_ENABLED
#include "led_widget_rgb.h"
#endif //RGB_LED_ENABLED

#include "sl_simple_led_instances.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include <assert.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>


#define SYSTEM_STATE_LED &sl_led_led0
#define LIGHT_LED &sl_led_led1
#define APP_FUNCTION_BUTTON &sl_button_btn0
#define APP_LIGHT_SWITCH &sl_button_btn1

using namespace chip;
using namespace ::chip::DeviceLayer;

namespace {


#ifdef RGB_LED_ENABLED
#define LIGHT_LED_RGB &sl_led_rgb_pwm
LEDWidgetRGB sLightLED;
#else 
LEDWidget sLightLED;
#endif

EmberAfIdentifyEffectIdentifier sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;



/**********************************************************
 * Identify Callbacks
 *********************************************************/

namespace {
void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;
}
} // namespace

void OnTriggerIdentifyEffect(Identify * identify)
{
    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mCurrentEffectIdentifier == EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE)
    {
        ChipLogProgress(Zcl, "IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE - Not supported, use effect varriant %d",
                        identify->mEffectVariant);
        sIdentifyEffect = static_cast<EmberAfIdentifyEffectIdentifier>(identify->mEffectVariant);
    }

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
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnTriggerIdentifyEffect,
};

/**********************************************************
 * OffWithEffect Callbacks
 *********************************************************/

void OnTriggerOffWithEffect(OnOffEffect * effect)
{
    chip::app::Clusters::OnOff::OnOffEffectIdentifier effectId = effect->mEffectIdentifier;
    uint8_t effectVariant                                      = effect->mEffectVariant;

    // Uses printouts until we can support the effects
    if (effectId == EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DELAYED_ALL_OFF)
    {
        if (effectVariant == EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_FADE_TO_OFF_IN_0P8_SECONDS)
        {
            ChipLogProgress(Zcl, "EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_FADE_TO_OFF_IN_0P8_SECONDS");
        }
        else if (effectVariant == EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_NO_FADE)
        {
            ChipLogProgress(Zcl, "EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_NO_FADE");
        }
        else if (effectVariant ==
                 EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_50_PERCENT_DIM_DOWN_IN_0P8_SECONDS_THEN_FADE_TO_OFF_IN_12_SECONDS)
        {
            ChipLogProgress(Zcl,
                            "EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_50_PERCENT_DIM_DOWN_IN_0P8_SECONDS_THEN_FADE_TO_OFF_"
                            "IN_12_SECONDS");
        }
    }
    else if (effectId == EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DYING_LIGHT)
    {
        if (effectVariant ==
            EMBER_ZCL_ON_OFF_DYING_LIGHT_EFFECT_VARIANT_20_PERCENTER_DIM_UP_IN_0P5_SECONDS_THEN_FADE_TO_OFF_IN_1_SECOND)
        {
            ChipLogProgress(
                Zcl, "EMBER_ZCL_ON_OFF_DYING_LIGHT_EFFECT_VARIANT_20_PERCENTER_DIM_UP_IN_0P5_SECONDS_THEN_FADE_TO_OFF_IN_1_SECOND");
        }
    }
}

OnOffEffect gEffect = {
    chip::EndpointId{ 1 },
    OnTriggerOffWithEffect,
    EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DELAYED_ALL_OFF,
    static_cast<uint8_t>(EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_FADE_TO_OFF_IN_0P8_SECONDS),
};

} // namespace

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;


CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "Lighting-App");
#endif

    err = BaseApplication::Init(&gIdentify);

    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("BaseApplication::Init() failed");
        appError(err);
    }
    SILABS_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    err = LightMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("LightMgr::Init() failed");
        appError(err);
    }
    LightMgr().SetCallbacks(ActionInitiated, ActionCompleted);



#if defined(RGB_LED_ENABLED)
    LEDWidgetRGB::InitGpioRGB();
    sLightLED.Init(LIGHT_LED_RGB);
#else 
    sLightLED.Init(LIGHT_LED);
#endif

    sLightLED.Set(LightMgr().IsLightOn());

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
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

    SILABS_LOG("App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
        sLightLED.Animate();
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


void AppTask::LightActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    LightingManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Light)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightEvent.Action);
        actor  = aEvent->LightEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LightMgr().IsLightOn())
        {
            action = LightingManager::OFF_ACTION;

        }
        else
        {
            action = LightingManager::ON_ACTION;
 
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = LightMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            SILABS_LOG("Action is already in progress or active.");
        }
    }
}
void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    bool lightOn = aAction == LightingManager::ON_ACTION;
    SILABS_LOG("Turning light %s", (lightOn) ? "On" : "Off")
    sLightLED.Set(lightOn);

#ifdef DISPLAY_ENABLED
    sAppTask.GetLCD().WriteDemoUI(lightOn);
#endif

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // action has been completed on the light
    if (aAction == LightingManager::ON_ACTION)
    {
        SILABS_LOG("Light ON")
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        SILABS_LOG("Light OFF")
    }
    if (sAppTask.mSyncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::LightControlEventHandler(AppEvent * aEvent)
{
    /* 1. Unpack the AppEvent */
    uint8_t light_action = aEvent->LightControlEvent.Action;
    uint8_t value = aEvent->LightControlEvent.Value;

    /* 2. Excute the control command. */
    if (light_action == LightingManager::MOVE_TO_LEVEL)
    {
#ifdef RGB_LED_ENABLED
        sLightLED.SetLevel(value);
#endif //RGB_LED_ENABLED
        SILABS_LOG("Level set to: %d.", value);
    }
    else if (light_action == LightingManager::MOVE_TO_HUE)
    {
#ifdef RGB_LED_ENABLED
        sLightLED.SetHue(value);
#endif //RGB_LED_ENABLED
        SILABS_LOG("Light LED hue set.");
    }
    else if (light_action == LightingManager::MOVE_TO_SAT)
    {
#ifdef RGB_LED_ENABLED
        sLightLED.SetSaturation(value);
#endif //RGB_LED_ENABLED
        SILABS_LOG("Light LED saturation set.");
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
        button_event.Handler = LightActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (buttonHandle == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        sAppTask.PostEvent(&button_event);
    }
}



void AppTask::PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type              = AppEvent::kEventType_Light;
    event.LightEvent.Actor  = aActor;
    event.LightEvent.Action = aAction;
    event.Handler           = LightActionEventHandler;
    PostEvent(&event);
}
void AppTask::PostLightControlActionRequest(int32_t aActor, LightingManager::Action_t aAction, uint8_t value)
{
    AppEvent light_event                    = {};
    light_event.Type                        = AppEvent::kEventType_Light;
    light_event.LightControlEvent.Actor     = aActor;
    light_event.LightControlEvent.Action    = aAction;
    light_event.LightControlEvent.Value     = value;
    light_event.Handler                     = LightControlEventHandler;
    PostEvent(&light_event);
}
void AppTask::UpdateClusterState(intptr_t context)
{
    uint8_t newValue = LightMgr().IsLightOn();
    // write the new on/off value
    EmberAfStatus status = OnOffServer::Instance().setOnOffValue(1, newValue, false);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        SILABS_LOG("ERR: updating on/off %x", status);
    }
}
