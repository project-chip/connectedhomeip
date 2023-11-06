/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <AppTask.h>

#include <BLConfig.h>

#include <plat.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

volatile int apperror_cnt;
// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    ChipLogProgress(NotSpecified, "!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (true)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

extern "C" int START_ENTRY(void)
{
    platform_port_init();

    Internal::BLConfig::Init();

    ChipLogProgress(NotSpecified, "==================================================");
    ChipLogProgress(NotSpecified, "bouffalolab chip-lighting-example, built at " __DATE__ " " __TIME__);
    ChipLogProgress(NotSpecified, "==================================================");

    ChipLogProgress(NotSpecified, "Init CHIP Memory");
    chip::Platform::MemoryInit(NULL, 0);

    ChipLogProgress(NotSpecified, "Starting App Task");
    StartAppTask();

    ChipLogProgress(NotSpecified, "Starting OS Scheduler...");
    vTaskStartScheduler();

    // Should never get here.
    ChipLogError(NotSpecified, "Starting OS Scheduler failed");
    appError(0);

    return 0;
}
