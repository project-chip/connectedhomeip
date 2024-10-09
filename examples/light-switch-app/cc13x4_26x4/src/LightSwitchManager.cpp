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

#include "LightSwitchManager.h"
#include "BindingHandler.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/switch-server/switch-server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace ::chip::DeviceLayer;

LightSwitchManager LightSwitchManager::sLightSwitch;

TimerHandle_t sSwitchTimer;

CHIP_ERROR LightSwitchManager::Init()
{
    // Create FreeRTOS sw timer for light timer.
    sSwitchTimer = xTimerCreate("switchTmr",      // Just a text name, not used by the RTOS kernel
                                1,                // == default timer period (mS)
                                false,            // no timer reload (==one-shot)
                                (void *) this,    // init timer id = light obj context
                                TimerEventHandler // timer callback handler
    );

    if (sSwitchTimer == NULL)
    {
        PLAT_LOG("sSwitchTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    // Configure Bindings
    CHIP_ERROR error = InitBindingHandler();
    if (error != CHIP_NO_ERROR)
    {
        PLAT_LOG("InitBindingHandler() failed!");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    mState = kState_SwitchOffCompleted;

    return CHIP_NO_ERROR;
}

void LightSwitchManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightSwitchManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;

    State_t new_state;

    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = chip::app::Clusters::OnOff::Id;

    // need to create logic to separate this
    data->isGroup = false;

    // Initiate Switch On/Off Action only when the previous one is complete.
    if ((mState == kState_SwitchOffCompleted || mState == kState_SwitchOnCompleted) && aAction == SWITCH_ON_ACTION)
    {
        action_initiated = true;
        new_state        = kState_SwitchOnInitiated;
        data->commandId  = chip::app::Clusters::OnOff::Commands::On::Id;
    }
    else if ((mState == kState_SwitchOffCompleted || mState == kState_SwitchOnCompleted) && aAction == SWITCH_OFF_ACTION)
    {
        action_initiated = true;
        new_state        = kState_SwitchOffInitiated;
        data->commandId  = chip::app::Clusters::OnOff::Commands::Off::Id;
    }

    if (action_initiated)
    {
        StartTimer(ACTUATOR_MOVEMENT_PERIOD_MS);
        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
    }
    // Platform::Delete(data);
    return action_initiated;
}

void LightSwitchManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sSwitchTimer))
    {
        PLAT_LOG("app timer already started!");
        CancelTimer();
    }
    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sSwitchTimer, pdMS_TO_TICKS(aTimeoutMs), 100) != pdPASS)
    {
        PLAT_LOG("sSwitchTimer timer start() failed");
    }
}

void LightSwitchManager::CancelTimer(void)
{
    if (xTimerStop(sSwitchTimer, 0) == pdFAIL)
    {
        PLAT_LOG("sSwitchTimer stop() failed");
    }
}

void LightSwitchManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get light obj context from timer id.
    LightSwitchManager * lightswitch = static_cast<LightSwitchManager *>(pvTimerGetTimerID(xTimer));
    // The timer event handler will be called in the context of the timer task
    // once sLightTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type                     = AppEvent::kEventType_AppEvent;
    event.LightSwitchEvent.Context = lightswitch;
    event.Handler                  = ActuatorMovementTimerEventHandler;
    AppTask::GetAppTask().PostEvent(&event);
}

void LightSwitchManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted  = INVALID_ACTION;
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->clusterId           = chip::app::Clusters::OnOff::Id;

    LightSwitchManager * lightswitch = static_cast<LightSwitchManager *>(aEvent->LightSwitchEvent.Context);
    if (lightswitch->mState == kState_SwitchOffInitiated)
    {
        lightswitch->mState = kState_SwitchOffCompleted;
        actionCompleted     = SWITCH_OFF_ACTION;
        data->commandId     = chip::app::Clusters::OnOff::Commands::Off::Id;
    }
    else if (lightswitch->mState == kState_SwitchOnInitiated)
    {
        lightswitch->mState = kState_SwitchOnCompleted;
        actionCompleted     = SWITCH_ON_ACTION;
        data->commandId     = chip::app::Clusters::OnOff::Commands::On::Id;
    }

    if (actionCompleted != INVALID_ACTION)
    {
        DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));

        if (lightswitch->mActionCompleted_CB)
        {
            lightswitch->mActionCompleted_CB(actionCompleted);
        }
    }
}

void LightSwitchManager::IdentifyEventHandler()
{
    AppEvent event;
    event.Type = AppEvent::kEventType_Identify;
    AppTask::GetAppTask().PostEvent(&event);
}
