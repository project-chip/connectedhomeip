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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <app/clusters/smoke-co-alarm-server/smoke-co-alarm-server.h>

#include <lib/core/CHIPError.h>

class SmokeCoAlarmManager
{
public:
    using ExpressedStateEnum = chip::app::Clusters::SmokeCoAlarm::ExpressedStateEnum;

    enum TriggeredEvent_t
    {
        kTriggeredEvent_WarningSmokeAlarm           = 0xffffffff00000090,
        kTriggeredEvent_CriticalSmokeAlarm          = 0xffffffff0000009c,
        kTriggeredEvent_SmokeAlarmClear             = 0xffffffff000000a0,
        kTriggeredEvent_WarningCOAlarm              = 0xffffffff00000091,
        kTriggeredEvent_CriticalCOAlarm             = 0xffffffff0000009d,
        kTriggeredEvent_COAlarmClear                = 0xffffffff000000a1,
        kTriggeredEvent_WarningBatteryAlert         = 0xffffffff00000095,
        kTriggeredEvent_CriticalBatteryAlert        = 0xffffffff0000009e,
        kTriggeredEvent_BatteryAlertClear           = 0xffffffff000000a5,
        kTriggeredEvent_HardwareFaultAlert          = 0xffffffff00000093,
        kTriggeredEvent_HardwareFaultAlertClear     = 0xffffffff000000a3,
        kTriggeredEvent_EndofServiceAlert           = 0xffffffff0000009a,
        kTriggeredEvent_EndofServiceAlertClear      = 0xffffffff000000aa,
        kTriggeredEvent_DeviceMute                  = 0xffffffff0000009b,
        kTriggeredEvent_DeviceMuteClear             = 0xffffffff000000ab,
        kTriggeredEvent_InterconnectSmokeAlarm      = 0xffffffff00000092,
        kTriggeredEvent_InterconnectSmokeAlarmClear = 0xffffffff000000a2,
        kTriggeredEvent_InterconnectCOAlarm         = 0xffffffff00000094,
        kTriggeredEvent_InterconnectCOAlarmClear    = 0xffffffff000000a4,
        kTriggeredEvent_ContaminationStateHigh      = 0xffffffff00000096,
        kTriggeredEvent_ContaminationStateLow       = 0xffffffff00000097,
        kTriggeredEvent_ContaminationStateClear     = 0xffffffff000000a6,
        kTriggeredEvent_SensitivityLevelHigh        = 0xffffffff00000098,
        kTriggeredEvent_SensitivityLevelLow         = 0xffffffff00000099,
        kTriggeredEvent_SensitivityLevelClear       = 0xffffffff000000a8,
    } TriggeredEvent;

    CHIP_ERROR Init();

    /**
     * @brief Execute the self-test process and attribute changes
     *
     */
    bool OnSelfTesting();

    /**
     * @brief Execute the self-test process manually
     *
     */
    bool ManualSelfTesting();

    /**
     * @brief Execute the HandleEventTrigger process
     *
     */
    bool OnEventTriggerHandle(uint64_t eventTrigger);

    /**
     * @brief Updates the expressed state with new value
     *
     * @param endpointId ID of the endpoint
     * @param expressedState expressed state
     * @param isSet true on set, false on unset
     * @return true on success, false on failure
     */
    bool SetExpressedState(chip::EndpointId endpointId, ExpressedStateEnum expressedState, bool isSet);

private:
    friend SmokeCoAlarmManager & AlarmMgr(void);

    /**
     * @brief Record expressed state in decreasing priority
     *
     */
    std::array<ExpressedStateEnum, 9> mExpressedStatePriority;

    /**
     * @brief Expression status record values
     *
     */
    int mExpressedStateMask = 1;

    bool mEndSelfTesting;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void SelfTestingEventHandler(AppEvent * aEvent);
    static void EndSelfTestingEventHandler(AppEvent * aEvent);

    static SmokeCoAlarmManager sAlarm;
};

inline SmokeCoAlarmManager & AlarmMgr(void)
{
    return SmokeCoAlarmManager::sAlarm;
}
