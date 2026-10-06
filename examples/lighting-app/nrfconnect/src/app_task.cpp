/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

#include "app_task.h"

#include "app/matter_init.h"
#include "app/task_executor.h"

#if defined(CONFIG_PWM)
#include "pwm/pwm_device.h"
#endif

#include "clusters/identify.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/persistence/DeferredAttributePersistenceProvider.h>
#include <app/server/Server.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::DeviceLayer;

namespace {
constexpr EndpointId kLightEndpointId = 1;
constexpr uint8_t kDefaultMinLevel    = 0;
constexpr uint8_t kDefaultMaxLevel    = 254;

Nrf::Matter::IdentifyCluster sIdentifyCluster(kLightEndpointId, true, []() {
    Nrf::PostTask([] { Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Set(false); });
#if defined(CONFIG_PWM)
    Nrf::PostTask([] { AppTask::Instance().GetPWMDevice().ApplyLevel(); });
#endif
});

#if defined(CONFIG_PWM)
const struct pwm_dt_spec sLightPwmDevice = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1));
#endif

/* Define a custom attribute persister which makes actual write of the CurrentLevel attribute value
 * to the non-volatile storage only when it has remained constant for 5 seconds. This is to reduce
 * the flash wearout when the attribute changes frequently as a result of MoveToLevel command.
 * DeferredAttribute object describes a deferred attribute, but also holds a buffer with a value to
 * be written, so it must live so long as the DeferredAttributePersistenceProvider object.
 */
DeferredAttribute gCurrentLevelPersister(ConcreteAttributePath(kLightEndpointId, Clusters::LevelControl::Id,
                                                               Clusters::LevelControl::Attributes::CurrentLevel::Id));

/* Deferred persistence will be auto-initialized as soon as the default persistence is initialized */
DefaultAttributePersistenceProvider gSimpleAttributePersistence;
DeferredAttributePersistenceProvider gDeferredAttributePersister(gSimpleAttributePersistence,
                                                                 Span<DeferredAttribute>(&gCurrentLevelPersister, 1),
                                                                 System::Clock::Milliseconds32(5000));

#define APPLICATION_BUTTON_MASK DK_BTN2_MSK
} /* namespace */

void AppTask::LightingActionEventHandler(const LightingEvent & event)
{
#if defined(CONFIG_PWM)
    Nrf::PWMDevice::Action_t action = Nrf::PWMDevice::INVALID_ACTION;
    int32_t actor                   = 0;
    if (event.Actor == LightingActor::Button)
    {
        action = Instance().mPWMDevice.IsTurnedOn() ? Nrf::PWMDevice::OFF_ACTION : Nrf::PWMDevice::ON_ACTION;
        actor  = static_cast<int32_t>(event.Actor);
    }

    if (action == Nrf::PWMDevice::INVALID_ACTION || !Instance().mPWMDevice.InitiateAction(action, actor, NULL))
    {
        LOG_INF("An action could not be initiated.");
    }
#else
    Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Set(!Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).GetState());
#endif
}

void AppTask::ButtonEventHandler(Nrf::ButtonState state, Nrf::ButtonMask hasChanged)
{
    if ((APPLICATION_BUTTON_MASK & hasChanged) & state)
    {
        Nrf::PostTask([] {
            LightingEvent event;
            event.Actor = LightingActor::Button;
            LightingActionEventHandler(event);
        });
    }
}

#if defined(CONFIG_PWM)
void AppTask::ActionInitiated(Nrf::PWMDevice::Action_t action, int32_t actor)
{
    if (action == Nrf::PWMDevice::ON_ACTION)
    {
        LOG_INF("Turn On Action has been initiated");
    }
    else if (action == Nrf::PWMDevice::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been initiated");
    }
    else if (action == Nrf::PWMDevice::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been initiated");
    }
}

void AppTask::ActionCompleted(Nrf::PWMDevice::Action_t action, int32_t actor)
{
    if (action == Nrf::PWMDevice::ON_ACTION)
    {
        LOG_INF("Turn On Action has been completed");
    }
    else if (action == Nrf::PWMDevice::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been completed");
    }
    else if (action == Nrf::PWMDevice::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been completed");
    }

    if (actor == static_cast<int32_t>(LightingActor::Button))
    {
        Instance().UpdateClusterState();
    }
}
#endif /* CONFIG_PWM */

void AppTask::UpdateClusterState()
{
    SystemLayer().ScheduleLambda([this] {
#if defined(CONFIG_PWM)
        /* write the new on/off value */
        Protocols::InteractionModel::Status status =
            Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, mPWMDevice.IsTurnedOn());
#else
        Protocols::InteractionModel::Status status =
            Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).GetState());
#endif
        if (status != Protocols::InteractionModel::Status::Success)
        {
            LOG_ERR("Updating on/off cluster failed: %x", to_underlying(status));
        }

#if defined(CONFIG_PWM)
        /* write the current level */
        status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, mPWMDevice.GetLevel());
#else
        /* write the current level */
        if (Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).GetState())
        {
            status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, 100);
        }
        else
        {
            status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, 0);
        }
#endif

        if (status != Protocols::InteractionModel::Status::Success)
        {
            LOG_ERR("Updating level cluster failed: %x", to_underlying(status));
        }
    });
}

void AppTask::InitPWMDDevice()
{
#if defined(CONFIG_PWM)
    /* Initialize lighting device (PWM) */
    uint8_t minLightLevel = kDefaultMinLevel;
    Clusters::LevelControl::Attributes::MinLevel::Get(kLightEndpointId, &minLightLevel);

    uint8_t maxLightLevel = kDefaultMaxLevel;
    Clusters::LevelControl::Attributes::MaxLevel::Get(kLightEndpointId, &maxLightLevel);

    Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::Type currentLevel;
    Clusters::LevelControl::Attributes::CurrentLevel::Get(kLightEndpointId, currentLevel);

    int ret = mPWMDevice.Init(&sLightPwmDevice, minLightLevel, maxLightLevel, currentLevel.ValueOr(kDefaultMaxLevel));
    if (ret != 0)
    {
        LOG_ERR("Failed to initialize PWD device.");
    }

    mPWMDevice.SetCallbacks(ActionInitiated, ActionCompleted);
#endif
}

CHIP_ERROR AppTask::Init()
{
    /* Initialize Matter stack */
    ReturnErrorOnFailure(Nrf::Matter::PrepareServer(Nrf::Matter::InitData{ .mPostServerInitClbk = []() {
        app::SetAttributePersistenceProvider(&gDeferredAttributePersister);
        return gSimpleAttributePersistence.Init(Nrf::Matter::GetPersistentStorageDelegate());
    } }));

    if (!Nrf::GetBoard().Init(ButtonEventHandler))
    {
        LOG_ERR("User interface initialization failed.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Register Matter event handler that controls the connectivity status LED based on the captured Matter network
     * state. */
    ReturnErrorOnFailure(Nrf::Matter::RegisterEventHandler(Nrf::Board::DefaultMatterEventHandler, 0));

    return Nrf::Matter::StartServer();
}

CHIP_ERROR AppTask::StartApp()
{
    ReturnErrorOnFailure(Init());

    while (true)
    {
        Nrf::DispatchNextTask();
    }

    return CHIP_NO_ERROR;
}
