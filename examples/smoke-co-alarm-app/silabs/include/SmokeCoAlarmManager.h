/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    CHIP_ERROR Init();

    /**
     * @brief Execute the self-test process
     *
     */
    void SelfTestingEventHandler();

private:
    friend SmokeCoAlarmManager & AlarmMgr(void);

    bool mEndSelfTesting;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void EndSelfTestingEventHandler(AppEvent * aEvent);

    static SmokeCoAlarmManager sAlarm;
};

inline SmokeCoAlarmManager & AlarmMgr(void)
{
    return SmokeCoAlarmManager::sAlarm;
}
