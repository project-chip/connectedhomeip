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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#include <platform/CHIPDeviceLayer.h>

#include <AppTask.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

extern "C" size_t GetHeapTotalSize(void);

// ================================================================================
// Logging Support
// ================================================================================

void appError(int err)
{
    APP_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    assert(true);
}

// ================================================================================
// Main Code
// ================================================================================

int main(void)
{
    int err;

    APP_LOG("==================================================");
    APP_LOG("chip-linux-lock-app starting");
    APP_LOG("==================================================");

    APP_LOG("Init CHIP Stack");
    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        APP_LOG("PlatformMgr().InitChipStack() failed");
        appError(err);
    }

    APP_LOG("Starting Platform Manager Event Loop");
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        APP_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(err);
    }

    APP_LOG("Starting App Task");
    err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        APP_LOG("GetAppTask().Init() failed");
        appError(err);
    }

    APP_LOG("Starting OS scheduler");
    chip_os_sched_start();

    // Should never get here.
    APP_LOG("FATAL: OS scheduler exited");
    appError(err);
}
