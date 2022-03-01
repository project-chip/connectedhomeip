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

#include "AppTask.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "ButtonManager.h"
#include "LEDWidget.h"
#include "LightingManager.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include "ThreadUtil.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/attribute-storage.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <platform/CHIPDeviceLayer.h>

#include <lib/support/ErrorStr.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemClock.h>

#include <logging/log.h>
#include <zephyr.h>

#include <algorithm>

LOG_MODULE_DECLARE(app);

namespace {

constexpr int kAppEventQueueSize      = 10;
constexpr uint8_t kButtonPushEvent    = 1;
constexpr uint8_t kButtonReleaseEvent = 0;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));

LEDWidget sStatusLED;

Button sFactoryResetButton;
Button sLightingButton;
Button sThreadStartButton;

bool sIsThreadProvisioned = false;
bool sIsThreadEnabled     = false;
bool sIsThreadAttached    = false;
bool sHaveBLEConnections  = false;

} // namespace

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR ret;

    // Initialize status LED
    LEDWidget::InitGpio(SYSTEM_STATE_LED_PORT);
    sStatusLED.Init(SYSTEM_STATE_LED_PIN);

    InitButtons();

    // Init lighting manager
    ret = LightingMgr().Init(LIGHTING_PWM_DEVICE, LIGHTING_PWM_CHANNEL);
    if (ret != CHIP_NO_ERROR)
    {
        LOG_ERR("Failed to int lighting manager");
        return ret;
    }

    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Init ZCL Data Model and start server
    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    ret = chip::Server::GetInstance().AddTestCommissioning();
    if (ret != CHIP_NO_ERROR)
    {
        LOG_ERR("Failed to add test pairing");
        return ret;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartApp()
{
    CHIP_ERROR err = Init();

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppTask.Init() failed");
        return err;
    }

    AppEvent event = {};

    while (true)
    {
        int ret = k_msgq_get(&sAppEventQueue, &event, K_MSEC(10));

        while (!ret)
        {
            DispatchEvent(&event);
            ret = k_msgq_get(&sAppEventQueue, &event, K_NO_WAIT);
        }

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).

        if (PlatformMgr().TryLockChipStack())
        {
            sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
            sIsThreadAttached    = ConnectivityMgr().IsThreadAttached();
            sHaveBLEConnections  = (ConnectivityMgr().NumBLEConnections() != 0);
            PlatformMgr().UnlockChipStack();
        }

        if (sIsThreadProvisioned && sIsThreadEnabled)
        {
            if (sIsThreadAttached)
            {
                sStatusLED.Blink(950, 50);
            }
            else
            {
                sStatusLED.Blink(100, 100);
            }
        }
        else
        {
            sStatusLED.Blink(50, 950);
        }

        sStatusLED.Animate();
    }
}

void AppTask::LightingActionButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = LightingActionEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action = LightingManager::INVALID_ACTION;
    int32_t actor                    = 0;

    if (aEvent->Type == AppEvent::kEventType_Lighting)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightingEvent.Action);
        actor  = aEvent->LightingEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = LightingMgr().IsTurnedOn() ? LightingManager::OFF_ACTION : LightingManager::ON_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != LightingManager::INVALID_ACTION && !LightingMgr().InitiateAction(action, actor, 0, NULL))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::FactoryResetButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = FactoryResetHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FactoryResetHandler(AppEvent * aEvent)
{
    LOG_INF("Factory Reset triggered.");
    chip::Server::GetInstance().ScheduleFactoryReset();
}

void AppTask::StartThreadButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartThreadHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::StartThreadHandler(AppEvent * aEvent)
{
    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        StartDefaultThreadNetwork();
        LOG_INF("Device is not commissioned to a Thread network. Starting with the default configuration.");
    }
    else
    {
        LOG_INF("Device is commissioned to a Thread network.");
    }
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        LOG_INF("Turn On Action has been initiated");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been initiated");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been initiated");
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        LOG_INF("Turn On Action has been completed");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been completed");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been completed");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::PostLightingActionRequest(LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type                 = AppEvent::kEventType_Lighting;
    event.LightingEvent.Action = aAction;
    event.Handler              = LightingActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(AppEvent * aEvent)
{
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT) != 0)
    {
        LOG_INF("Failed to post event to app task event queue");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        LOG_INF("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState()
{
    uint8_t onoff = LightingMgr().IsTurnedOn();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &onoff,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating on/off cluster failed: %x", status);
    }

    uint8_t level = LightingMgr().GetLevel();

    status = emberAfWriteAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &level,
                                   ZCL_INT8U_ATTRIBUTE_TYPE);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating level cluster failed: %x", status);
    }
}

void AppTask::InitButtons(void)
{
    sFactoryResetButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_1, FactoryResetButtonEventHandler);
    sLightingButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_1, LightingActionButtonEventHandler);
    sThreadStartButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_2, StartThreadButtonEventHandler);

    ButtonManagerInst().AddButton(sFactoryResetButton);
    ButtonManagerInst().AddButton(sLightingButton);
    ButtonManagerInst().AddButton(sThreadStartButton);
}
