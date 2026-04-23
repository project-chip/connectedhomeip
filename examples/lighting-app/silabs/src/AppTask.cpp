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
#include "CustomerAppTask.h"

#include "LEDWidget.h"
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
#include "RGBLEDWidget.h"
#endif

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <clusters/ColorControl/AttributeIds.h>
#include <clusters/LevelControl/AttributeIds.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

#include <app/ConcreteAttributePath.h>

#include <assert.h>

#include <cmsis_os2.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>

#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
#define LIGHT_LED 1
#else
#define LIGHT_LED 0
#endif

#define APP_FUNCTION_BUTTON 0
#define APP_LIGHT_SWITCH 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace ::chip::app::Clusters::OnOff;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

namespace {

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
RGBLEDWidget sLightLED;
#else
LEDWidget sLightLED;
#endif

// ---------------------------------------------------------------------------
// Lighting state. Lives here (TU-local) instead of on AppTask because the
// helpers below are not part of the override surface and AppTask is a
// singleton (CustomerAppTask::sAppTask).
// ---------------------------------------------------------------------------
AppTask::State_t sLightState = AppTask::kState_OffCompleted;
osTimerId_t sLightTimer      = nullptr;
bool sOffEffectArmed         = false;
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
uint8_t sCurrentLevel      = 254;
uint8_t sCurrentHue        = 0;
uint8_t sCurrentSaturation = 0;
uint16_t sCurrentX         = 0;
uint16_t sCurrentY         = 0;
uint16_t sCurrentCTMireds  = 250;
#endif

// ---------------------------------------------------------------------------
// Internal helpers (no override surface). Kept as TU-local free functions so
// they cannot be overridden by Derived classes; bodies access the state above
// directly.
// ---------------------------------------------------------------------------

// Forward-declare the event handler so PostLightActionRequest can take its address.
void LightActionEventHandler(AppEvent * aEvent);

// Forward-declare so OnLightActionCompleted (defined further below) can pass
// this pointer to PlatformMgr().ScheduleWork.
void UpdateClusterState(intptr_t context);

void StartLightTimer(uint32_t aTimeoutMs)
{
    if (osTimerStart(sLightTimer, pdMS_TO_TICKS(aTimeoutMs)) != osOK)
    {
        SILABS_LOG("sLightTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void CancelLightTimer()
{
    if (osTimerStop(sLightTimer) == osError)
    {
        SILABS_LOG("sLightTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void UpdateClusterState(intptr_t /* context */)
{
    uint8_t newValue = AppTask::GetAppTask().IsLightOn();

    Protocols::InteractionModel::Status status = OnOffServer::Instance().setOnOffValue(LIGHT_ENDPOINT, newValue, false);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        SILABS_LOG("ERR: updating on/off %x", to_underlying(status));
    }
}

void LightActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    AppTask::Action_t action;
    int32_t actor;
    uint8_t value  = aEvent->LightEvent.Value;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Light)
    {
        action = static_cast<AppTask::Action_t>(aEvent->LightEvent.Action);
        actor  = aEvent->LightEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = (CustomerAppTask::GetAppTask().IsLightOn()) ? AppTask::OFF_ACTION : AppTask::ON_ACTION;
        actor  = AppEvent::kEventType_Button;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = CustomerAppTask::GetAppTask().InitiateAction(actor, action, &value);

        if (!initiated)
        {
            SILABS_LOG("Action is already in progress or active.");
        }
    }
}

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
void PostLightControlActionRequest(int32_t aActor, AppTask::Action_t aAction, RGBLEDWidget::ColorData_t * aValue)
{
    AppEvent light_event;
    light_event.Type                     = AppEvent::kEventType_Light;
    light_event.LightControlEvent.Actor  = aActor;
    light_event.LightControlEvent.Action = aAction;
    light_event.LightControlEvent.Value  = *aValue;
    light_event.Handler                  = &CustomerAppTask::LightControlEventHandler;
    CustomerAppTask::GetAppTask().PostEvent(&light_event);
}
#endif

void OffEffectTimerEventHandler(AppEvent * /* aEvent */)
{
    if (!sOffEffectArmed)
    {
        return;
    }

    sOffEffectArmed = false;

    SILABS_LOG("OffEffect completed");

    int32_t actor = AppEvent::kEventType_Timer;
    uint8_t value = 0;
    CustomerAppTask::GetAppTask().InitiateAction(actor, AppTask::OFF_ACTION, &value);
}

void ActuatorMovementTimerEventHandler(AppEvent * /* aEvent */)
{
    AppTask::Action_t actionCompleted = AppTask::INVALID_ACTION;

    if (sLightState == AppTask::kState_OffInitiated)
    {
        sLightState     = AppTask::kState_OffCompleted;
        actionCompleted = AppTask::OFF_ACTION;
    }
    else if (sLightState == AppTask::kState_OnInitiated)
    {
        sLightState     = AppTask::kState_OnCompleted;
        actionCompleted = AppTask::ON_ACTION;
    }

    if (actionCompleted != AppTask::INVALID_ACTION)
    {
        CustomerAppTask::GetAppTask().OnLightActionCompleted(actionCompleted);
    }
}

OnOffEffect gEffect = {
    chip::EndpointId(LIGHT_ENDPOINT),
    &CustomerAppTask::OnTriggerOffWithEffect,
    EffectIdentifierEnum::kDelayedAllOff,
    to_underlying(DelayedAllOffEffectVariantEnum::kDelayedOffFastFade),
};

} // namespace

// ---------------------------------------------------------------------------
// AppTask member definitions (overridable APIs). Singleton provided by
// CustomerAppTask.cpp (AppTask::GetAppTask() returns CustomerAppTask::GetAppTask()).
// ---------------------------------------------------------------------------

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&CustomerAppTask::ButtonEventHandler);

    err = CustomerAppTask::GetAppTask().InitLight();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("InitLight() failed");
        appError(err);
    }

    sLightLED.Init(LIGHT_LED);
    sLightLED.Set(IsLightOn());

// Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI(IsLightOn());
#ifdef QR_CODE_ENABLED
#ifdef SL_WIFI
    if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (!ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        GetLCD().ShowQRCode(true);
    }
#endif // QR_CODE_ENABLED
#endif

    return err;
}

CHIP_ERROR AppTask::InitLight()
{
    sLightTimer = osTimerNew(&CustomerAppTask::LightTimerEventHandler, osTimerOnce, nullptr, nullptr);

    if (sLightTimer == nullptr)
    {
        SILABS_LOG("sLightTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    bool currentLedState;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OnOffServer::Instance().getOnOffValue(LIGHT_ENDPOINT, &currentLedState);

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    app::DataModel::Nullable<uint8_t> brightness;
    uint16_t currentx, currenty, currentctmireds;
    uint8_t currenthue, currentsaturation;

    if (Clusters::LevelControl::Attributes::CurrentLevel::Get(LIGHT_ENDPOINT, brightness) ==
            Protocols::InteractionModel::Status::Success &&
        !brightness.IsNull())
    {
        sCurrentLevel = brightness.Value();
    }

    if (Clusters::ColorControl::Attributes::CurrentX::Get(LIGHT_ENDPOINT, &currentx) ==
        Protocols::InteractionModel::Status::Success)
    {
        sCurrentX = currentx;
    }
    if (Clusters::ColorControl::Attributes::CurrentY::Get(LIGHT_ENDPOINT, &currenty) ==
        Protocols::InteractionModel::Status::Success)
    {
        sCurrentY = currenty;
    }
    if (Clusters::ColorControl::Attributes::CurrentHue::Get(LIGHT_ENDPOINT, &currenthue) ==
        Protocols::InteractionModel::Status::Success)
    {
        sCurrentHue = currenthue;
    }
    if (Clusters::ColorControl::Attributes::CurrentSaturation::Get(LIGHT_ENDPOINT, &currentsaturation) ==
        Protocols::InteractionModel::Status::Success)
    {
        sCurrentSaturation = currentsaturation;
    }
    if (Clusters::ColorControl::Attributes::ColorTemperatureMireds::Get(LIGHT_ENDPOINT, &currentctmireds) ==
        Protocols::InteractionModel::Status::Success)
    {
        sCurrentCTMireds = currentctmireds;
    }
#endif

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    sLightState     = currentLedState ? kState_OnCompleted : kState_OffCompleted;
    sOffEffectArmed = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(&CustomerAppTask::AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = GetAppTask().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    GetAppTask().StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");

    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, nullptr, osWaitForever);
        while (eventReceived == osOK)
        {
            GetAppTask().DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, nullptr, 0);
        }
    }
}

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
void AppTask::LightControlEventHandler(AppEvent * aEvent)
{
    uint8_t light_action                = aEvent->LightControlEvent.Action;
    RGBLEDWidget::ColorData_t colorData = aEvent->LightControlEvent.Value;
    PlatformMgr().LockChipStack();
    Protocols::InteractionModel::Status status;
    app::DataModel::Nullable<uint8_t> currentlevel;
    status = LevelControl::Attributes::CurrentLevel::Get(LIGHT_ENDPOINT, currentlevel);
    PlatformMgr().UnlockChipStack();
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to get CurrentLevel attribute"));
    if (status == Protocols::InteractionModel::Status::Success && !currentlevel.IsNull())
    {
        sLightLED.SetLevel(currentlevel.Value());
    }
    switch (light_action)
    {
    case COLOR_ACTION_XY: {
        sLightLED.SetColorFromXY(colorData.xy.x, colorData.xy.y);
    }
    break;
    case COLOR_ACTION_HSV: {
        sLightLED.SetColorFromHSV(colorData.hsv.h, colorData.hsv.s);
    }
    break;
    case COLOR_ACTION_CT: {
        sLightLED.SetColorFromCT(colorData.ct.ctMireds);
    }
    break;
    default:
        ChipLogProgress(NotSpecified, "AppTask: unknown light action");
        break;
    }
}
#endif

bool AppTask::IsLightOn() const
{
    return (sLightState == AppTask::kState_OnCompleted);
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_LIGHT_SWITCH && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = &LightActionEventHandler;
        CustomerAppTask::GetAppTask().PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        CustomerAppTask::GetAppTask().PostEvent(&button_event);
    }
}

void AppTask::OnLightActionInitiated(AppTask::Action_t aAction, int32_t aActor, uint8_t * aValue)
{
    if (aAction == LEVEL_ACTION)
    {
        VerifyOrReturn(aValue != nullptr);
        sLightLED.SetLevel(*aValue);
    }
    else
    {
        bool lightOn = aAction == ON_ACTION;
        SILABS_LOG("Turning light %s", (lightOn) ? "On" : "Off")

        sLightLED.Set(lightOn);

#ifdef DISPLAY_ENABLED
        GetLCD().WriteDemoUI(lightOn);
#endif

        if (aActor == AppEvent::kEventType_Button)
        {
            mSyncClusterToButtonAction = true;
        }
    }
}

void AppTask::OnLightActionCompleted(AppTask::Action_t aAction)
{
    if (aAction == ON_ACTION)
    {
        SILABS_LOG("Light ON")
    }
    else if (aAction == OFF_ACTION)
    {
        SILABS_LOG("Light OFF")
    }

    if (mSyncClusterToButtonAction)
    {
        // UpdateClusterState is a TU-local helper (no CRTP override); pass it directly to ScheduleWork.
        TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState,
                                                                               reinterpret_cast<intptr_t>(nullptr));
        mSyncClusterToButtonAction = false;
    }
}

bool AppTask::InitiateAction(int32_t aActor, AppTask::Action_t aAction, uint8_t * aValue)
{
    bool action_initiated = false;
    AppTask::State_t new_state;

    if (((sLightState == kState_OffCompleted) || sOffEffectArmed) && aAction == ON_ACTION)
    {
        action_initiated = true;
        new_state        = kState_OnInitiated;
        if (sOffEffectArmed)
        {
            CancelLightTimer();
            sOffEffectArmed = false;
        }
    }
    else if (sLightState == kState_OnCompleted && aAction == OFF_ACTION && sOffEffectArmed == false)
    {
        action_initiated = true;
        new_state        = kState_OffInitiated;
    }
    else if (aAction == LEVEL_ACTION)
    {
        action_initiated = true;
    }

    if (action_initiated && (aAction == ON_ACTION || aAction == OFF_ACTION))
    {
        StartLightTimer(ACTUATOR_MOVEMENT_PERIOD_MS);
        sLightState = new_state;
    }

    if (action_initiated)
    {
        CustomerAppTask::GetAppTask().OnLightActionInitiated(aAction, aActor, aValue);
    }

    return action_initiated;
}

void AppTask::LightTimerEventHandler(void * /* timerCbArg */)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = nullptr;

    if (sOffEffectArmed)
    {
        event.Handler = &OffEffectTimerEventHandler;
    }
    else
    {
        event.Handler = &ActuatorMovementTimerEventHandler;
    }
    CustomerAppTask::GetAppTask().PostEvent(&event);
}

void AppTask::OnTriggerOffWithEffect(OnOffEffect * effect)
{
    auto effectId              = effect->mEffectIdentifier;
    auto effectVariant         = effect->mEffectVariant;
    uint32_t offEffectDuration = 0;

    if (effectId == EffectIdentifierEnum::kDelayedAllOff)
    {
        auto typedEffectVariant = static_cast<DelayedAllOffEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == DelayedAllOffEffectVariantEnum::kDelayedOffFastFade)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffFastFade");
        }
        else if (typedEffectVariant == DelayedAllOffEffectVariantEnum::kNoFade)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kNoFade");
        }
        else if (typedEffectVariant == DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade)
        {
            offEffectDuration = 12800;
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade");
        }
    }
    else if (effectId == EffectIdentifierEnum::kDyingLight)
    {
        auto typedEffectVariant = static_cast<DyingLightEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == DyingLightEffectVariantEnum::kDyingLightFadeOff)
        {
            offEffectDuration = 1500;
            ChipLogProgress(Zcl, "DyingLightEffectVariantEnum::kDyingLightFadeOff");
        }
    }

    sOffEffectArmed = true;
    StartLightTimer(offEffectDuration);
}

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
bool AppTask::InitiateLightCtrlAction(int32_t aActor, AppTask::Action_t aAction, uint32_t aAttributeId, uint8_t * value)
{
    bool action_initiated = false;
    VerifyOrReturnError(aAction == COLOR_ACTION_XY || aAction == COLOR_ACTION_HSV || aAction == COLOR_ACTION_CT, action_initiated);
    RGBLEDWidget::ColorData_t colorData;
    switch (aAction)
    {
    case COLOR_ACTION_XY:
        colorData.xy = { sCurrentX, sCurrentY };

        if (aAttributeId == ColorControl::Attributes::CurrentX::Id)
        {
            VerifyOrReturnValue(colorData.xy.x != *reinterpret_cast<uint16_t *>(value), action_initiated);
            colorData.xy.x   = *reinterpret_cast<uint16_t *>(value);
            action_initiated = true;
        }
        else if (aAttributeId == ColorControl::Attributes::CurrentY::Id)
        {
            VerifyOrReturnValue(colorData.xy.y != *reinterpret_cast<uint16_t *>(value), action_initiated);
            colorData.xy.y   = *reinterpret_cast<uint16_t *>(value);
            action_initiated = true;
        }
        break;

    case COLOR_ACTION_HSV:
        colorData.hsv = { sCurrentHue, sCurrentSaturation };

        if (aAttributeId == ColorControl::Attributes::CurrentHue::Id)
        {
            VerifyOrReturnValue(colorData.hsv.h != *value, action_initiated);
            colorData.hsv.h  = *value;
            sCurrentHue      = *value;
            action_initiated = true;
        }
        else if (aAttributeId == ColorControl::Attributes::CurrentSaturation::Id)
        {
            VerifyOrReturnValue(colorData.hsv.s != *value, action_initiated);
            colorData.hsv.s    = *value;
            sCurrentSaturation = *value;
            action_initiated   = true;
        }
        break;

    case COLOR_ACTION_CT:
        colorData.ct.ctMireds = sCurrentCTMireds;
        VerifyOrReturnValue(colorData.ct.ctMireds != *reinterpret_cast<uint16_t *>(value), action_initiated);
        colorData.ct.ctMireds = *reinterpret_cast<uint16_t *>(value);
        action_initiated      = true;
        break;

    default:
        break;
    }
    PostLightControlActionRequest(aActor, aAction, &colorData);
    return action_initiated;
}
#endif

void AppTask::DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value)
{
    [[maybe_unused]] EndpointId endpointId = attributePath.mEndpointId;
    ClusterId clusterId                    = attributePath.mClusterId;
    AttributeId attributeId                = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    switch (clusterId)
    {
    case OnOff::Id:
        if (attributeId == OnOff::Attributes::OnOff::Id && value != nullptr)
        {
            CustomerAppTask::GetAppTask().InitiateAction(AppEvent::kEventType_Light,
                                                         *value ? AppTask::ON_ACTION : AppTask::OFF_ACTION, value);
        }
        break;

    case LevelControl::Id:
        if (attributeId == LevelControl::Attributes::CurrentLevel::Id && value != nullptr)
        {
            CustomerAppTask::GetAppTask().InitiateAction(AppEvent::kEventType_Light, AppTask::LEVEL_ACTION, value);
        }
        break;

    case ColorControl::Id:
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
        switch (attributeId)
        {
        case ColorControl::Attributes::CurrentX::Id:
        case ColorControl::Attributes::CurrentY::Id:
            CustomerAppTask::GetAppTask().InitiateLightCtrlAction(AppEvent::kEventType_Light, AppTask::COLOR_ACTION_XY, attributeId,
                                                                  value);
            break;

        case ColorControl::Attributes::CurrentHue::Id:
        case ColorControl::Attributes::CurrentSaturation::Id:
            CustomerAppTask::GetAppTask().InitiateLightCtrlAction(AppEvent::kEventType_Light, AppTask::COLOR_ACTION_HSV, attributeId,
                                                                  value);
            break;

        case ColorControl::Attributes::ColorTemperatureMireds::Id:
            if (size != sizeof(uint16_t))
            {
                ChipLogError(Zcl, "Wrong length for ColorControl value: %" PRIu16, size);
                return;
            }
            CustomerAppTask::GetAppTask().InitiateLightCtrlAction(AppEvent::kEventType_Light, AppTask::COLOR_ACTION_CT, attributeId,
                                                                  value);
            break;

        default:
            break;
        }
#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
        break;

    case Identify::Id:
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
        break;

    default:
        break;
    }
}
