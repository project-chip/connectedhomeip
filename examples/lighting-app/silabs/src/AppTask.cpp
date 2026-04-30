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

CustomerAppTask & appInstance()
{
    return CustomerAppTask::GetAppTask();
}

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
RGBLEDWidget sLightLED;
#else
LEDWidget sLightLED;
#endif

bool sLightOn           = false;
osTimerId_t sLightTimer = nullptr;

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
uint8_t sCurrentLevel      = 254;
uint8_t sCurrentHue        = 0;
uint8_t sCurrentSaturation = 0;
uint16_t sCurrentX         = 0;
uint16_t sCurrentY         = 0;
uint16_t sCurrentCTMireds  = 250;

bool VerifyColorControlValueSize(uint16_t size, size_t expectedBytes)
{
    if (size != expectedBytes)
    {
        ChipLogError(Zcl, "Wrong length for ColorControl value: %" PRIu16, size);
        return false;
    }
    return true;
}

void PostLightControlColorEvent(ColorAction_t action, const RGBLEDWidget::ColorData_t & colorData)
{
    AppEvent light_event{};
    light_event.Type                     = AppEvent::kEventType_Light;
    light_event.LightControlEvent.Actor  = static_cast<int32_t>(AppEvent::kEventType_Light);
    light_event.LightControlEvent.Action = static_cast<uint8_t>(action);
    light_event.LightControlEvent.Value  = colorData;
    light_event.Handler                  = &CustomerAppTask::LightControlEventHandler;
    appInstance().PostEvent(&light_event);
}
#endif

void OffEffectTimerEventHandler(AppEvent * /* aEvent */)
{
    sLightOn = false;
    sLightLED.Set(false);
}

OnOffEffect gEffect = {
    chip::EndpointId(LIGHT_ENDPOINT),
    &CustomerAppTask::OnTriggerOffWithEffect,
    EffectIdentifierEnum::kDelayedAllOff,
    to_underlying(DelayedAllOffEffectVariantEnum::kDelayedOffFastFade),
};

} // namespace

void AppTask::UpdateOnOffClusterState(intptr_t context)
{
    Protocols::InteractionModel::Status status =
        OnOffServer::Instance().setOnOffValue(LIGHT_ENDPOINT, static_cast<uint8_t>(context), false);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        SILABS_LOG("ERR: updating on/off %x", to_underlying(status));
    }
}

void AppTask::LightTimerEventHandler(void * /* timerCbArg */)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = nullptr;
    event.Handler            = &OffEffectTimerEventHandler;
    appInstance().PostEvent(&event);
}

void AppTask::LightActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        return;
    }


    sLightOn = !sLightOn;
    sLightLED.Set(sLightOn);

    if (osTimerIsRunning(sLightTimer))
    {
        if (osTimerStop(sLightTimer) == osError)
        {
            SILABS_LOG("sLightTimer stop() failed");
            appError(APP_ERROR_STOP_TIMER_FAILED);
        }
    }

#ifdef DISPLAY_ENABLED
    BaseApplication::GetLCD().WriteDemoUI(sLightOn);
#endif

    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateOnOffClusterState, static_cast<intptr_t>(sLightOn));
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
    case COLOR_ACTION_XY:
        sLightLED.SetColorFromXY(colorData.xy.x, colorData.xy.y);
        break;
    case COLOR_ACTION_HSV:
        sLightLED.SetColorFromHSV(colorData.hsv.h, colorData.hsv.s);
        break;
    case COLOR_ACTION_CT:
        sLightLED.SetColorFromCT(colorData.ct.ctMireds);
        break;
    default:
        ChipLogProgress(NotSpecified, "AppTask: unknown light action");
        break;
    }
}

#endif // SL_MATTER_RGB_LED_ENABLED

CHIP_ERROR AppTask::AppInit()
{
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&CustomerAppTask::ButtonEventHandler);

    CHIP_ERROR err = appInstance().InitLight();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("InitLight() failed");
        appError(err);
    }

    sLightLED.Init(LIGHT_LED);
    sLightLED.Set(sLightOn);

// Update the LCD with the stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI(sLightOn);
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

    sLightOn = currentLedState;

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(&AppTask::AppTaskMain);
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

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_LIGHT_SWITCH && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = &CustomerAppTask::LightActionEventHandler;
        appInstance().PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        appInstance().PostEvent(&button_event);
    }
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

    if (osTimerStart(sLightTimer, pdMS_TO_TICKS(offEffectDuration)) != osOK)
    {
        SILABS_LOG("sLightTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void AppTask::DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    switch (clusterId)
    {
    case OnOff::Id:
        if (attributeId == OnOff::Attributes::OnOff::Id && value != nullptr && size == sizeof(uint8_t))
        {
#ifdef SL_MATTER_ENABLE_AWS
            ChipLogProgress(Zcl, "sending light state update");
            MatterAwsSendMsg("light/state", (const char *) (value ? (*value ? "on" : "off") : "invalid"));
#endif // SL_MATTER_ENABLE_AWS
            sLightOn = (*value != 0);
            sLightLED.Set(sLightOn);
            if (sLightOn && osTimerIsRunning(sLightTimer))
            {
                if (osTimerStop(sLightTimer) == osError)
                {
                    SILABS_LOG("sLightTimer stop() failed");
                    appError(APP_ERROR_STOP_TIMER_FAILED);
                }
            }
        }
        break;

    case LevelControl::Id:
        if (attributeId == LevelControl::Attributes::CurrentLevel::Id && value != nullptr && size == sizeof(uint8_t))
        {
            sLightLED.SetLevel(*value);
        }
        break;

    case ColorControl::Id:
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
        switch (attributeId)
        {
        case ColorControl::Attributes::CurrentX::Id:
        case ColorControl::Attributes::CurrentY::Id:
            VerifyOrReturn(VerifyColorControlValueSize(size, sizeof(uint16_t)));
            {
                RGBLEDWidget::ColorData_t colorData;
                colorData.xy = { sCurrentX, sCurrentY };
                const uint16_t newVal = *reinterpret_cast<uint16_t *>(value);
                bool const changed =
                    (attributeId == ColorControl::Attributes::CurrentX::Id) ? (colorData.xy.x != newVal) : (colorData.xy.y != newVal);
                if (changed)
                {
                    if (attributeId == ColorControl::Attributes::CurrentX::Id)
                    {
                        colorData.xy.x = newVal;
                    }
                    else
                    {
                        colorData.xy.y = newVal;
                    }
                    PostLightControlColorEvent(COLOR_ACTION_XY, colorData);
                }
            }
            break;

        case ColorControl::Attributes::CurrentHue::Id:
        case ColorControl::Attributes::CurrentSaturation::Id:
            VerifyOrReturn(VerifyColorControlValueSize(size, sizeof(uint8_t)));
            {
                RGBLEDWidget::ColorData_t colorData;
                colorData.hsv = { sCurrentHue, sCurrentSaturation };
                bool changed = false;
                if (attributeId == ColorControl::Attributes::CurrentHue::Id)
                {
                    if (colorData.hsv.h != *value)
                    {
                        colorData.hsv.h = *value;
                        sCurrentHue     = *value;
                        changed         = true;
                    }
                }
                else if (colorData.hsv.s != *value)
                {
                    colorData.hsv.s    = *value;
                    sCurrentSaturation = *value;
                    changed            = true;
                }
                if (changed)
                {
                    PostLightControlColorEvent(COLOR_ACTION_HSV, colorData);
                }
            }
            break;

        case ColorControl::Attributes::ColorTemperatureMireds::Id:
            VerifyOrReturn(VerifyColorControlValueSize(size, sizeof(uint16_t)));
            {
                const uint16_t newMireds = *reinterpret_cast<uint16_t *>(value);
                if (sCurrentCTMireds != newMireds)
                {
                    RGBLEDWidget::ColorData_t colorData;
                    colorData.ct.ctMireds = newMireds;
                    sCurrentCTMireds      = newMireds;
                    PostLightControlColorEvent(COLOR_ACTION_CT, colorData);
                }
            }
            break;

        default:
            break;
        }
#endif // SL_MATTER_RGB_LED_ENABLED
        break;

    case Identify::Id:
        if (value != nullptr && size == sizeof(uint8_t))
        {
            ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                            ChipLogValueMEI(attributeId), type, *value, size);
        }
        break;

    default:
        break;
    }
}
