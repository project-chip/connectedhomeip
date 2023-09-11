/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

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
    void StartSelfTesting();

private:
    friend SmokeCoAlarmManager & AlarmMgr(void);

    static SmokeCoAlarmManager sAlarm;
};

inline SmokeCoAlarmManager & AlarmMgr(void)
{
    return SmokeCoAlarmManager::sAlarm;
}
