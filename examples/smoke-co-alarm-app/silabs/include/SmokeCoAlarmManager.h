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

    CHIP_ERROR Init();

    /**
     * @brief Execute the self-test process and attribute changes
     *
     */
    bool StartSelfTesting();

    /**
     * @brief Execute the HandleEventTrigger process
     *
     */
    bool StartHandleEventTrigger(uint64_t eventTrigger);

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

    bool mSelfTesting;
    bool mHandleEventTrigger;
    uint64_t mEventTrigger;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void SelfTestingTimerEventHandler(AppEvent * aEvent);
    static void EventTriggerTimerEventHandler(AppEvent * aEvent);

    static SmokeCoAlarmManager sAlarm;
};

inline SmokeCoAlarmManager & AlarmMgr(void)
{
    return SmokeCoAlarmManager::sAlarm;
}
