/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "SmokeCoAlarmManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace ::chip::app::Clusters::SmokeCoAlarm;
using namespace ::chip::DeviceLayer;

SmokeCoAlarmManager SmokeCoAlarmManager::sAlarm;

TimerHandle_t sAlarmTimer;

CHIP_ERROR SmokeCoAlarmManager::Init()
{
    mExpressedStatePriority = {
        ExpressedStateEnum::kEndOfService,  ExpressedStateEnum::kSmokeAlarm,        ExpressedStateEnum::kCOAlarm,
        ExpressedStateEnum::kTesting,       ExpressedStateEnum::kInterconnectSmoke, ExpressedStateEnum::kInterconnectCO,
        ExpressedStateEnum::kHardwareFault, ExpressedStateEnum::kBatteryAlert,      ExpressedStateEnum::kNormal
    };

    // Create FreeRTOS sw timer for alarm timer.
    sAlarmTimer = xTimerCreate("alarmTmr",       // Just a text name, not used by the RTOS kernel
                               pdMS_TO_TICKS(1), // == default timer period
                               false,            // no timer reload (==one-shot)
                               (void *) this,    // init timer id = alarm obj context
                               TimerEventHandler // timer callback handler
    );

    if (sAlarmTimer == NULL)
    {
        SILABS_LOG("sAlarmTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    // read current State on endpoint one
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    ExpressedStateEnum currentExpressedState;
    bool success = SmokeCoAlarmServer::Instance().GetExpressedState(1, currentExpressedState);
    if (success)
    {
        mExpressedStateMask |= (1 << to_underlying(currentExpressedState));
    }

    AlarmStateEnum currentSmokeState;
    success = SmokeCoAlarmServer::Instance().GetSmokeState(1, currentSmokeState);
    if (success && (currentSmokeState != AlarmStateEnum::kNormal))
    {
        mExpressedStateMask |= (1 << to_underlying(ExpressedStateEnum::kSmokeAlarm));
    }

    AlarmStateEnum currentCOState;
    success = SmokeCoAlarmServer::Instance().GetCOState(1, currentCOState);
    if (success && (currentCOState != AlarmStateEnum::kNormal))
    {
        mExpressedStateMask |= (1 << to_underlying(ExpressedStateEnum::kCOAlarm));
    }

    AlarmStateEnum currentBatteryAlert;
    success = SmokeCoAlarmServer::Instance().GetBatteryAlert(1, currentBatteryAlert);
    if (success && (currentBatteryAlert != AlarmStateEnum::kNormal))
    {
        mExpressedStateMask |= (1 << to_underlying(ExpressedStateEnum::kBatteryAlert));
    }

    bool currentHardwareFaultAlert;
    success = SmokeCoAlarmServer::Instance().GetHardwareFaultAlert(1, currentHardwareFaultAlert);
    if (success && (currentHardwareFaultAlert != false))
    {
        mExpressedStateMask |= (1 << to_underlying(ExpressedStateEnum::kHardwareFault));
    }

    EndOfServiceEnum currentEndOfServiceAlert;
    success = SmokeCoAlarmServer::Instance().GetEndOfServiceAlert(1, currentEndOfServiceAlert);
    if (success && (currentEndOfServiceAlert != EndOfServiceEnum::kNormal))
    {
        mExpressedStateMask |= (1 << to_underlying(ExpressedStateEnum::kEndOfService));
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    return CHIP_NO_ERROR;
}

void SmokeCoAlarmManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sAlarmTimer))
    {
        SILABS_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ms if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sAlarmTimer, pdMS_TO_TICKS(aTimeoutMs), pdMS_TO_TICKS(100)) != pdPASS)
    {
        SILABS_LOG("sAlarmTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void SmokeCoAlarmManager::CancelTimer(void)
{
    if (xTimerStop(sAlarmTimer, pdMS_TO_TICKS(0)) == pdFAIL)
    {
        SILABS_LOG("sAlarmTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void SmokeCoAlarmManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get alarm obj context from timer id.
    SmokeCoAlarmManager * alarm = static_cast<SmokeCoAlarmManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sAlarmTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = alarm;
    if (alarm->mSelfTesting)
    {
        event.Handler = SelfTestingTimerEventHandler;
    }
    else if (alarm->mHandleEventTrigger)
    {
        event.Handler = EventTriggerTimerEventHandler;
    }
    AppTask::GetAppTask().PostEvent(&event);
}

void SmokeCoAlarmManager::SelfTestingTimerEventHandler(AppEvent * aEvent)
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();

    bool success = SmokeCoAlarmServer::Instance().SetTestInProgress(1, true);
    if (success)
    {
        SILABS_LOG("Start self-testing!");

        SILABS_LOG("End self-testing!");
    }

    SmokeCoAlarmServer::Instance().SetTestInProgress(1, false);
    AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kTesting, false);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

void SmokeCoAlarmManager::EventTriggerTimerEventHandler(AppEvent * aEvent)
{
    SmokeCoAlarmManager * alarm = static_cast<SmokeCoAlarmManager *>(aEvent->TimerEvent.Context);
    bool success                = true;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    switch (alarm->mEventTrigger)
    {
    case 0xffffffff00000090:
        success = SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kSmokeAlarm, true);
        }
        break;

    case 0xffffffff000000a0:
        success = SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kSmokeAlarm, false);
        }
        break;

    case 0xffffffff00000091:
        success = SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kCOAlarm, true);
        }
        break;

    case 0xffffffff000000a1:
        success = SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kCOAlarm, false);
        }
        break;

    case 0xffffffff00000095:
        success = SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kBatteryAlert, true);
        }
        break;

    case 0xffffffff000000a5:
        success = SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kBatteryAlert, false);
        }
        break;

    case 0xffffffff00000093:
        success = SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, true);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kHardwareFault, true);
        }
        break;

    case 0xffffffff000000a3:
        success = SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, false);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kHardwareFault, false);
        }
        break;

    case 0xffffffff0000009a:
        success = SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, EndOfServiceEnum::kExpired);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kEndOfService, true);
        }
        break;

    case 0xffffffff000000aa:
        success = SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, EndOfServiceEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kEndOfService, false);
        }
        break;

    case 0xffffffff0000009b:
        success = SmokeCoAlarmServer::Instance().SetDeviceMuted(1, MuteStateEnum::kMuted);
        break;

    case 0xffffffff000000ab:
        success = SmokeCoAlarmServer::Instance().SetDeviceMuted(1, MuteStateEnum::kNotMuted);
        break;

    case 0xffffffff00000092:
        success = SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectSmoke, true);
        }
        break;

    case 0xffffffff000000a2:
        success = SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectSmoke, false);
        }
        break;

    case 0xffffffff00000094:
        success = SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectCO, true);
        }
        break;

    case 0xffffffff000000a4:
        success = SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectCO, false);
        }
        break;

    case 0xffffffff00000096:
        success = SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kWarning);
        break;

    case 0xffffffff00000097:
        success = SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kLow);
        break;

    case 0xffffffff000000a6:
        success = SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kNormal);
        break;

    case 0xffffffff00000098:
        success = SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SensitivityEnum::kHigh);
        break;

    case 0xffffffff00000099:
        success = SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SensitivityEnum::kLow);
        break;

    case 0xffffffff000000a8:
        success = SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SensitivityEnum::kStandard);
        break;

    default:
        break;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

bool SmokeCoAlarmManager::StartSelfTesting()
{
    AlarmMgr().mSelfTesting = true;
    AlarmMgr().StartTimer(10);

    return true;
}

bool SmokeCoAlarmManager::StartHandleEventTrigger(uint64_t eventTrigger)
{
    AlarmMgr().mHandleEventTrigger = true;
    AlarmMgr().mEventTrigger       = eventTrigger;
    AlarmMgr().StartTimer(1000);

    return true;
}

bool SmokeCoAlarmManager::SetExpressedState(EndpointId endpointId, ExpressedStateEnum expressedState, bool isSet)
{
    bool success = false;

    if (isSet)
    {
        mExpressedStateMask |= (1 << to_underlying(expressedState));
    }
    else
    {
        mExpressedStateMask &= ~(1 << to_underlying(expressedState));
    }

    for (auto it : mExpressedStatePriority)
    {
        if (mExpressedStateMask & (1 << to_underlying(it)))
        {
            success = SmokeCoAlarmServer::Instance().SetExpressedState(endpointId, it);
            break;
        }
    }

    return success;
}
