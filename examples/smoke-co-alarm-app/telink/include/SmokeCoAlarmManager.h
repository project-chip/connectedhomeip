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
