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
        ExpressedStateEnum::kTesting,       ExpressedStateEnum::kEndOfService,      ExpressedStateEnum::kSmokeAlarm,
        ExpressedStateEnum::kCOAlarm,       ExpressedStateEnum::kInterconnectSmoke, ExpressedStateEnum::kInterconnectCO,
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

    mEndSelfTesting = false;
    mEventTrigger   = 0;

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

    if (alarm->mEndSelfTesting)
    {
        event.Handler = EndSelfTestingEventHandler;
    }
    else if (alarm->mEventTrigger != 0)
    {
        event.Handler = TriggerEventHandler;
    }

    AppTask::GetAppTask().PostEvent(&event);
}

void SmokeCoAlarmManager::SelfTestingEventHandler(AppEvent * aEvent)
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    bool success = SmokeCoAlarmServer::Instance().SetTestInProgress(1, true);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (!success)
    {
        SILABS_LOG("Start self-testing failed");
        return;
    }

    SILABS_LOG("Start self-testing!");

    AlarmMgr().mEndSelfTesting = true;
    AlarmMgr().StartTimer(10000); // Self-test simulation in progress
}

void SmokeCoAlarmManager::EndSelfTestingEventHandler(AppEvent * aEvent)
{
    AlarmMgr().mEndSelfTesting = false;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    bool success = SmokeCoAlarmServer::Instance().SetTestInProgress(1, false);
    if (success)
    {
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kTesting, false);
        SILABS_LOG("End self-testing!");
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

void SmokeCoAlarmManager::TriggerEventHandler(AppEvent * aEvent)
{
    uint64_t eventTrigger    = AlarmMgr().mEventTrigger;
    AlarmMgr().mEventTrigger = 0;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    switch (eventTrigger)
    {
    case kTriggeredEvent_WarningSmokeAlarm:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kWarning),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kSmokeAlarm, true);
        break;

    case kTriggeredEvent_CriticalSmokeAlarm:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kCritical),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kSmokeAlarm, true);
        break;

    case kTriggeredEvent_SmokeAlarmClear:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kNormal),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kSmokeAlarm, false);
        break;

    case kTriggeredEvent_WarningCOAlarm:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kWarning),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kCOAlarm, true);
        break;

    case kTriggeredEvent_CriticalCOAlarm:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kCritical),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kCOAlarm, true);
        break;

    case kTriggeredEvent_COAlarmClear:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kNormal),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kCOAlarm, false);
        break;

    case kTriggeredEvent_WarningBatteryAlert:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kWarning),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kBatteryAlert, true);
        break;

    case kTriggeredEvent_CriticalBatteryAlert:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kCritical),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kBatteryAlert, true);
        break;

    case kTriggeredEvent_BatteryAlertClear:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kNormal),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kBatteryAlert, false);
        break;

    case kTriggeredEvent_HardwareFaultAlert:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, true),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kHardwareFault, true);
        break;

    case kTriggeredEvent_HardwareFaultAlertClear:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, false),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kHardwareFault, false);
        break;

    case kTriggeredEvent_EndofServiceAlert:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, EndOfServiceEnum::kExpired),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kEndOfService, true);
        break;

    case kTriggeredEvent_EndofServiceAlertClear:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, EndOfServiceEnum::kNormal),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kEndOfService, false);
        break;

    case kTriggeredEvent_DeviceMute:
        SmokeCoAlarmServer::Instance().SetDeviceMuted(1, MuteStateEnum::kMuted);
        break;

    case kTriggeredEvent_DeviceMuteClear:
        SmokeCoAlarmServer::Instance().SetDeviceMuted(1, MuteStateEnum::kNotMuted);
        break;

    case kTriggeredEvent_InterconnectSmokeAlarm:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, AlarmStateEnum::kWarning),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectSmoke, true);
        break;

    case kTriggeredEvent_InterconnectSmokeAlarmClear:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, AlarmStateEnum::kNormal),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectSmoke, false);
        break;

    case kTriggeredEvent_InterconnectCOAlarm:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, AlarmStateEnum::kWarning),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectCO, true);
        break;

    case kTriggeredEvent_InterconnectCOAlarmClear:
        VerifyOrReturn(SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, AlarmStateEnum::kNormal),
                       chip::DeviceLayer::PlatformMgr().UnlockChipStack());
        AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kInterconnectCO, false);
        break;

    case kTriggeredEvent_ContaminationStateHigh:
        SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kWarning);
        break;

    case kTriggeredEvent_ContaminationStateLow:
        SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kLow);
        break;

    case kTriggeredEvent_ContaminationStateClear:
        SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kNormal);
        break;

    case kTriggeredEvent_SensitivityLevelHigh:
        SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SensitivityEnum::kHigh);
        break;

    case kTriggeredEvent_SensitivityLevelLow:
        SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SensitivityEnum::kLow);
        break;

    case kTriggeredEvent_SensitivityLevelClear:
        SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SensitivityEnum::kStandard);
        break;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

bool SmokeCoAlarmManager::OnSelfTesting()
{
    AppEvent event;
    event.Handler = SelfTestingEventHandler;
    AppTask::GetAppTask().PostEvent(&event);

    return true;
}

bool SmokeCoAlarmManager::ManualSelfTesting()
{
    bool success = false;

    if ((mExpressedStateMask & 0b110010110) == 0)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        success = AlarmMgr().SetExpressedState(1, ExpressedStateEnum::kTesting, true);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

        if (success)
        {
            success = OnSelfTesting();
        }
    }

    return success;
}

bool SmokeCoAlarmManager::OnEventTriggerHandle(uint64_t eventTrigger)
{
    bool isValidCommand = false;

    switch (eventTrigger)
    {
    case kTriggeredEvent_WarningSmokeAlarm:
    case kTriggeredEvent_CriticalSmokeAlarm:
    case kTriggeredEvent_SmokeAlarmClear:
    case kTriggeredEvent_WarningCOAlarm:
    case kTriggeredEvent_CriticalCOAlarm:
    case kTriggeredEvent_COAlarmClear:
    case kTriggeredEvent_WarningBatteryAlert:
    case kTriggeredEvent_CriticalBatteryAlert:
    case kTriggeredEvent_BatteryAlertClear:
    case kTriggeredEvent_HardwareFaultAlert:
    case kTriggeredEvent_HardwareFaultAlertClear:
    case kTriggeredEvent_EndofServiceAlert:
    case kTriggeredEvent_EndofServiceAlertClear:
    case kTriggeredEvent_DeviceMute:
    case kTriggeredEvent_DeviceMuteClear:
    case kTriggeredEvent_InterconnectSmokeAlarm:
    case kTriggeredEvent_InterconnectSmokeAlarmClear:
    case kTriggeredEvent_InterconnectCOAlarm:
    case kTriggeredEvent_InterconnectCOAlarmClear:
    case kTriggeredEvent_ContaminationStateHigh:
    case kTriggeredEvent_ContaminationStateLow:
    case kTriggeredEvent_ContaminationStateClear:
    case kTriggeredEvent_SensitivityLevelHigh:
    case kTriggeredEvent_SensitivityLevelLow:
    case kTriggeredEvent_SensitivityLevelClear:
        isValidCommand           = true;
        AlarmMgr().mEventTrigger = eventTrigger;
        AlarmMgr().StartTimer(5000);
        break;
    }

    return isValidCommand;
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
