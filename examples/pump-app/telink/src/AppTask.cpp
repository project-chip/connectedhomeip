/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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
#include "LEDManager.h"
#include "PumpManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
constexpr EndpointId kPccClusterEndpoint   = 1;
constexpr EndpointId kOnOffClusterEndpoint = 1;
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    SetExampleButtonCallbacks(StartActionEventHandler);
    InitCommonParts();

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, !PumpMgr().IsStopped());

    PumpMgr().Init();
    PumpMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    return CHIP_NO_ERROR;
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

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, 50, 50);
}

void AppTask::ActionCompleted(PumpManager::Action_t action, int32_t actor)
{
    // If the action has been completed by the pump, update the pump trait.
    // Turn on the pump state LED if in a STARTED state OR
    // Turn off the pump state LED if in a STOPPED state.
    if (action == PumpManager::START_ACTION)
    {
        LOG_INF("Pump Start Action has been completed");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, true);
    }
    else if (action == PumpManager::STOP_ACTION)
    {
        LOG_INF("Pump Stop Action has been completed");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, false);
    }

    if (actor == static_cast<uint8_t>(AppEvent::kEventType_Button))
    {
        GetAppTask().UpdateClusterState();
    }
}

void AppTask::PostStartActionRequest(int32_t actor, PumpManager::Action_t action)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_DeviceAction;
    event.DeviceEvent.Actor  = actor;
    event.DeviceEvent.Action = action;
    event.Handler            = StartActionEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::StartActionEventHandler(AppEvent * aEvent)
{
    PumpManager::Action_t action = PumpManager::INVALID_ACTION;
    int32_t actor                = 0;

    if (aEvent->Type == AppEvent::kEventType_DeviceAction)
    {
        action = static_cast<PumpManager::Action_t>(aEvent->DeviceEvent.Action);
        actor  = aEvent->DeviceEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = PumpMgr().IsStopped() ? PumpManager::START_ACTION : PumpManager::STOP_ACTION;
        actor  = static_cast<uint8_t>(AppEvent::kEventType_Button);
    }

    if (action != PumpManager::INVALID_ACTION && !PumpMgr().InitiateAction(actor, action))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::UpdateClusterState()
{
    // Write the new values
    bool onOffState = !PumpMgr().IsStopped();

    Protocols::InteractionModel::Status status = Clusters::OnOff::Attributes::OnOff::Set(kOnOffClusterEndpoint, onOffState);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        LOG_ERR("ERR: Updating On/Off state  %x", to_underlying(status));
    }

    int16_t maxPressure    = PumpMgr().GetMaxPressure();
    uint16_t maxSpeed      = PumpMgr().GetMaxSpeed();
    uint16_t maxFlow       = PumpMgr().GetMaxFlow();
    int16_t minConstPress  = PumpMgr().GetMinConstPressure();
    int16_t maxConstPress  = PumpMgr().GetMaxConstPressure();
    int16_t minCompPress   = PumpMgr().GetMinCompPressure();
    int16_t maxCompPress   = PumpMgr().GetMaxCompPressure();
    uint16_t minConstSpeed = PumpMgr().GetMinConstSpeed();
    uint16_t maxConstSpeed = PumpMgr().GetMaxConstSpeed();
    uint16_t minConstFlow  = PumpMgr().GetMinConstFlow();
    uint16_t maxConstFlow  = PumpMgr().GetMaxConstFlow();
    int16_t minConstTemp   = PumpMgr().GetMinConstTemp();
    int16_t maxConstTemp   = PumpMgr().GetMaxConstTemp();

    LOG_INF("UpdateClusterState:\n maxPressure = %d,\t maxSpeed = %d,\t maxFlow = %d\n minConstPress = %d,\t "
            "maxConstPress = %d\n minCompPress = %d,\t maxCompPress = %d\n minConstSpeed = %d,\t maxConstSpeed = %d\n"
            "minConstFlow = %d,\t maxConstFlow = %d\n minConstTemp = %d,\t maxConstTemp = %d",
            maxPressure, maxSpeed, maxFlow, minConstPress, maxConstPress, minCompPress, maxCompPress, minConstSpeed, maxConstSpeed,
            minConstFlow, maxConstFlow, minConstTemp, maxConstTemp);
}

void AppTask::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
    ledManager.linkLed(LedManager::EAppLed_App0, 1);
#else
    ledManager.linkLed(LedManager::EAppLed_App0, 0);
#endif // CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
}
