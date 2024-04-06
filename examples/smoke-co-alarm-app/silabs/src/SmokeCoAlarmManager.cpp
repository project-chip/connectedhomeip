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

#include <app/clusters/smoke-co-alarm-server/SmokeCOTestEventTriggerHandler.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace chip::app::Clusters::SmokeCoAlarm;
using namespace chip::DeviceLayer;

SmokeCoAlarmManager SmokeCoAlarmManager::sAlarm;

static std::array<ExpressedStateEnum, SmokeCoAlarmServer::kPriorityOrderLength> sPriorityOrder = {
    ExpressedStateEnum::kSmokeAlarm,     ExpressedStateEnum::kInterconnectSmoke, ExpressedStateEnum::kCOAlarm,
    ExpressedStateEnum::kInterconnectCO, ExpressedStateEnum::kHardwareFault,     ExpressedStateEnum::kTesting,
    ExpressedStateEnum::kEndOfService,   ExpressedStateEnum::kBatteryAlert
};

CHIP_ERROR SmokeCoAlarmManager::Init()
{
    // Create cmsisos sw timer for alarm timer.
    mAlarmTimer = osTimerNew(TimerEventHandler, // timer callback handler
                             osTimerOnce,       // no timer reload (one-shot timer)
                             this,              // pass the app task obj context
                             NULL               // No osTimerAttr_t to provide.
    );

    if (mAlarmTimer == NULL)
    {
        SILABS_LOG("mAlarmTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    // read current State on endpoint one
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    mEndSelfTesting = false;

    return CHIP_NO_ERROR;
}

void SmokeCoAlarmManager::StartTimer(uint32_t aTimeoutMs)
{
    // Starts or restarts the function timer
    if (osTimerStart(mAlarmTimer, pdMS_TO_TICKS(aTimeoutMs)) != osOK)
    {
        SILABS_LOG("mAlarmTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void SmokeCoAlarmManager::CancelTimer(void)
{
    if (osTimerStop(mAlarmTimer) == osError)
    {
        SILABS_LOG("mAlarmTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void SmokeCoAlarmManager::TimerEventHandler(void * timerCbArg)
{
    // Get alarm obj context from timer id.
    SmokeCoAlarmManager * alarm = static_cast<SmokeCoAlarmManager *>(timerCbArg);

    // The timer event handler will be called in the context of the timer task
    // once mAlarmTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = alarm;
    if (alarm->mEndSelfTesting)
    {
        event.Handler = EndSelfTestingEventHandler;
    }
    AppTask::GetAppTask().PostEvent(&event);
}

void SmokeCoAlarmManager::SelfTestingEventHandler()
{
    SILABS_LOG("Start self-testing!");

    AlarmMgr().mEndSelfTesting = true;
    AlarmMgr().StartTimer(10000); // Self-test simulation in progress
}

void SmokeCoAlarmManager::EndSelfTestingEventHandler(AppEvent * aEvent)
{
    AlarmMgr().mEndSelfTesting = false;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    SmokeCoAlarmServer::Instance().SetTestInProgress(1, false);
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    SILABS_LOG("End self-testing!");
}

bool emberAfHandleEventTrigger(uint64_t eventTrigger)
{
    SmokeCOTrigger trigger = static_cast<SmokeCOTrigger>(eventTrigger);

    switch (trigger)
    {
    case SmokeCOTrigger::kForceSmokeCritical:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force smoke (critical)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kCritical), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceSmokeWarning:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force smoke (warning)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kWarning), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceSmokeInterconnect:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force smoke interconnect (warning)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, AlarmStateEnum::kWarning), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceCOCritical:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force CO (critical)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kCritical), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceCOWarning:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force CO (warning)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kWarning), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceCOInterconnect:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force CO (warning)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, AlarmStateEnum::kWarning), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceSmokeContaminationHigh:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force smoke contamination (critical)");
        SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kCritical);
        break;
    case SmokeCOTrigger::kForceSmokeContaminationLow:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force smoke contamination (warning)");
        SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kLow);
        break;
    case SmokeCOTrigger::kForceSmokeSensitivityHigh:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force smoke sensistivity (high)");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(1, SensitivityEnum::kHigh);
        break;
    case SmokeCOTrigger::kForceSmokeSensitivityLow:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force smoke sensitivity (low)");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(1, SensitivityEnum::kLow);
        break;
    case SmokeCOTrigger::kForceMalfunction:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force malfunction");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, true), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceLowBatteryWarning:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force low battery (warning)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kWarning), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceLowBatteryCritical:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force low battery (critical)");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kCritical), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceEndOfLife:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force end-of-life");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, EndOfServiceEnum::kExpired), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kForceSilence:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force silence");
        SmokeCoAlarmServer::Instance().SetDeviceMuted(1, MuteStateEnum::kMuted);
        break;
    case SmokeCOTrigger::kClearSmoke:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear smoke");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetSmokeState(1, AlarmStateEnum::kNormal), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kClearCO:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear CO");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetCOState(1, AlarmStateEnum::kNormal), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kClearSmokeInterconnect:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear smoke interconnect");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, AlarmStateEnum::kNormal), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kClearCOInterconnect:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear CO interconnect");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, AlarmStateEnum::kNormal), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kClearMalfunction:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear malfunction");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, false), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kClearEndOfLife:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear end-of-life");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, EndOfServiceEnum::kNormal), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kClearSilence:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear silence");
        SmokeCoAlarmServer::Instance().SetDeviceMuted(1, MuteStateEnum::kNotMuted);
        break;
    case SmokeCOTrigger::kClearBatteryLevelLow:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear low battery");
        VerifyOrReturnValue(SmokeCoAlarmServer::Instance().SetBatteryAlert(1, AlarmStateEnum::kNormal), true);
        SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
        break;
    case SmokeCOTrigger::kClearContamination:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Force SmokeContamination (warning)");
        SmokeCoAlarmServer::Instance().SetContaminationState(1, ContaminationStateEnum::kNormal);
        break;
    case SmokeCOTrigger::kClearSensitivity:
        ChipLogProgress(Support, "[Smoke-CO-Alarm-Test-Event] => Clear Smoke Sensitivity");
        SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(1, SensitivityEnum::kStandard);
        break;
    default:

        return false;
    }

    return true;
}
