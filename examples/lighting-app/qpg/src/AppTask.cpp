/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#if !defined(GP_APP_DIVERSITY_POWERCYCLECOUNTING)
#error This application requires powercycle counting.
#endif

#include "gpSched.h"
#include "powercycle_counting.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "ota.h"

#include "ButtonHandler.h"
#include "StatusLed.h"
#include "qPinCfg.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/persistence/DeferredAttributePersistenceProvider.h>
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>

static uint8_t countdown = 0;

constexpr EndpointId kLightEndpointId = 1;

// Define a custom attribute persister which makes actual write of the ColorX attribute value
// to the non-volatile storage only when it has remained constant for 5 seconds. This is to reduce
// the flash wearout when the attribute changes frequently as a result of MoveToLevel command.
// DeferredAttribute object describes a deferred attribute, but also holds a buffer with a value to
// be written, so it must live so long as the DeferredAttributePersistenceProvider object.
//
DeferredAttribute gPersisters[] = {
    DeferredAttribute(
        ConcreteAttributePath(kLightEndpointId, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::CurrentHue::Id)),
    DeferredAttribute(ConcreteAttributePath(kLightEndpointId, Clusters::ColorControl::Id,
                                            Clusters::ColorControl::Attributes::CurrentSaturation::Id)),
    DeferredAttribute(
        ConcreteAttributePath(kLightEndpointId, Clusters::LevelControl::Id, Clusters::LevelControl::Attributes::CurrentLevel::Id))

};

// Deferred persistence will be auto-initialized as soon as the default persistence is initialized
DefaultAttributePersistenceProvider gSimpleAttributePersistence;
DeferredAttributePersistenceProvider gDeferredAttributePersister(gSimpleAttributePersistence,
                                                                 Span<DeferredAttribute>(gPersisters, 3),
                                                                 System::Clock::Milliseconds32(5000));

const uint8_t StatusLedGpios[] = QPINCFG_STATUS_LED;
const ButtonConfig_t buttons[] = QPINCFG_BUTTONS;

AppTask AppTask::sAppTask;

/**********************************************************
 * OffWithEffect Callbacks
 *********************************************************/

void OnTriggerOffWithEffect(OnOffEffect * effect)
{
    auto effectId      = effect->mEffectIdentifier;
    auto effectVariant = effect->mEffectVariant;

    // Uses print outs until we can support the effects
    if (effectId == Clusters::OnOff::EffectIdentifierEnum::kDelayedAllOff)
    {
        auto typedEffectVariant = static_cast<Clusters::OnOff::DelayedAllOffEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == Clusters::OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade)
        {
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffFastFade");
        }
        else if (typedEffectVariant == Clusters::OnOff::DelayedAllOffEffectVariantEnum::kNoFade)
        {
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kNoFade");
        }
        else if (typedEffectVariant == Clusters::OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade)
        {
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade");
        }
    }
    else if (effectId == Clusters::OnOff::EffectIdentifierEnum::kDyingLight)
    {
        auto typedEffectVariant = static_cast<Clusters::OnOff::DyingLightEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == Clusters::OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff)
        {
            ChipLogProgress(Zcl, "DyingLightEffectVariantEnum::kDyingLightFadeOff");
        }
    }
}

OnOffEffect gEffect = {
    chip::EndpointId{ 1 },
    OnTriggerOffWithEffect,
    Clusters::OnOff::EffectIdentifierEnum::kDelayedAllOff,
    to_underlying(Clusters::OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade),
};

CHIP_ERROR AppTask::Init()
{
    // Initialize common code in base class
    CHIP_ERROR err = BaseAppTask::Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "BaseAppTask::Init() failed");
        return err;
    }

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(AppTask::InitServerWrapper, 0);

    // Setup powercycle reset expired handler
    gpAppFramework_SetResetExpiredHandler(AppTask::PowerCycleExpiredHandlerWrapper);

    // Setup button handler
    ButtonHandler_Init(buttons, Q_ARRAY_SIZE(buttons), BUTTON_LOW, AppTask::ButtonEventHandlerWrapper);

    // Setup light
    err = LightingMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "LightingMgr().Init() failed");
        return err;
    }
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    return err;
}

void AppTask::InitServer(intptr_t arg)
{
    BaseAppTask::InitServer((intptr_t) &gSimpleAttributePersistence);

    app::SetAttributePersistenceProvider(&gDeferredAttributePersister);

    // Open commissioning after boot if no fabric was available
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
    {
        ChipLogProgress(NotSpecified, "No fabrics, starting commissioning.");
        PlatformMgr().ScheduleWork(OpenCommissioning, 0);
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action;

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        // Toggle light
        if (LightingMgr().IsTurnedOn())
        {
            action = LightingManager::OFF_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 0, 0);
    }
    if (aEvent->Type == AppEvent::kEventType_Level && aEvent->ButtonEvent.Action != 0)
    {
        // Toggle Dimming of light between 2 fixed levels
        uint8_t val = 0x0;
        val         = LightingMgr().GetLevel() == 0x40 ? 0xfe : 0x40;
        action      = LightingManager::LEVEL_ACTION;

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 1, &val);
    }
}

bool AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    // Call base class ButtonEventHandler
    bool eventHandled = BaseAppTask::ButtonEventHandler(btnIdx, btnPressed);
    if (eventHandled)
    {
        return true;
    }

    // Only go ahead if button index has a supported value
    if (btnIdx != APP_SWITCH_BUTTON && btnIdx != APP_LEVEL_BUTTON)
    {
        return false;
    }

    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_SWITCH_BUTTON && btnPressed == true)
    {
        // Hand off to Light handler - On/Off light
        button_event.Handler = LightingActionEventHandler;
    }
    else if (btnIdx == APP_LEVEL_BUTTON)
    {
        // Hand off to Light handler - Change level of light
        button_event.Type    = AppEvent::kEventType_Level;
        button_event.Handler = LightingActionEventHandler;
    }
    else
    {
        return false;
    }

    sAppTask.PostEvent(&button_event);

    return true;
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction)
{
    // Placeholder for light action
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light goes on");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light goes off ");
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // Placeholder for light action completed
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light On Action has been completed");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light Off Action has been completed");
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

/**
 * Update cluster status after application level changes
 */
void AppTask::UpdateClusterState(void)
{
    SystemLayer().ScheduleLambda([] {
        ChipLogProgress(NotSpecified, "UpdateClusterState");

        // Write the new on/off value
        Protocols::InteractionModel::Status status =
            Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, LightingMgr().IsTurnedOn());

        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating on/off %x", to_underlying(status));
        }

        // Write new level value
        status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, LightingMgr().GetLevel());
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating level %x", to_underlying(status));
        }
    });
}

static void NextCountdown(void)
{
    if (countdown > 0)
    {
        LightingMgr().InitiateAction((countdown % 2 == 0) ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION, 0, 0, 0);
        countdown--;
        gpSched_ScheduleEvent(1000000UL, NextCountdown);
    }
    else
    {
        SystemLayer().ScheduleLambda([] { ConfigurationMgr().InitiateFactoryReset(); });
    }
}

void AppTask::PowerCycleExpiredHandler(uint8_t resetCounts)
{
    if (resetCounts == 10)
    {
        ChipLogProgress(NotSpecified, "Factory Reset Triggered!");
        countdown = 5;
        NextCountdown();
    }
}
