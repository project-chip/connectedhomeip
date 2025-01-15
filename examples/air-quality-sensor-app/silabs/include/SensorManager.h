/*
 *
 *    Copyright (c) 2019-2024 Google LLC.
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

#pragma once

#include "AppEvent.h"
#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>
#include <stdbool.h>
#include <stdint.h>

class SensorManager
{
public:
    CHIP_ERROR Init();
    static SensorManager & SensorMgr() { return sSensorManager; }

private:
    SensorManager()  = default;
    ~SensorManager() = default;

    osTimerId_t mSensorTimer;

    // Reads new generated sensor value, stores it, and updates local Air Quality attribute
    static void SensorTimerEventHandler(void * arg);

    static SensorManager sSensorManager;
};
