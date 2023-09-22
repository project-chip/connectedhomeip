/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "PumpManager.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
constexpr EndpointId kPccClusterEndpoint   = 1;
constexpr EndpointId kOnOffClusterEndpoint = 1;

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
LEDWidget sPumpStateLED;
#endif
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(StartActionEventHandler);
#endif
    InitCommonParts();

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    sPumpStateLED.Init(GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios));
    sPumpStateLED.Set(!PumpMgr().IsStopped());
#endif

    PumpMgr().Init();
    PumpMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    return CHIP_NO_ERROR;
}

void AppTask::StartActionEventHandler(AppEvent * aEvent)
{
    PumpManager::Action_t action = PumpManager::INVALID_ACTION;
    int32_t actor                = 0;

    if (aEvent->Type == AppEvent::kEventType_Start)
    {
        action = static_cast<PumpManager::Action_t>(aEvent->StartEvent.Action);
        actor  = aEvent->StartEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = PumpMgr().IsStopped() ? PumpManager::START_ACTION : PumpManager::STOP_ACTION;
        actor  = static_cast<uint8_t>(AppEvent::kEventType_Button);
    }

    if (action != PumpManager::INVALID_ACTION && !PumpMgr().InitiateAction(actor, action))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::ActionInitiated(PumpManager::Action_t action, int32_t actor)
{
    // If the action has been initiated by the pump, update the pump trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (action == PumpManager::START_ACTION)
    {
        LOG_INF("Pump Start Action has been initiated");
    }
    else if (action == PumpManager::STOP_ACTION)
    {
        LOG_INF("Pump Stop Action has been initiated");
    }

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    sPumpStateLED.Blink(50, 50);
#endif
}

void AppTask::ActionCompleted(PumpManager::Action_t action, int32_t actor)
{
    // If the action has been completed by the pump, update the pump trait.
    // Turn on the pump state LED if in a STARTED state OR
    // Turn off the pump state LED if in a STOPPED state.
    if (action == PumpManager::START_ACTION)
    {
        LOG_INF("Pump Start Action has been completed");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sPumpStateLED.Set(true);
#endif
    }
    else if (action == PumpManager::STOP_ACTION)
    {
        LOG_INF("Pump Stop Action has been completed");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sPumpStateLED.Set(false);
#endif
    }

    if (actor == static_cast<uint8_t>(AppEvent::kEventType_Button))
    {
        GetAppTask().UpdateClusterState();
    }
}

void AppTask::PostStartActionRequest(int32_t actor, PumpManager::Action_t action)
{
    AppEvent event;
    event.Type              = AppEvent::kEventType_Start;
    event.StartEvent.Actor  = actor;
    event.StartEvent.Action = action;
    event.Handler           = StartActionEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::UpdateClusterState() {}
