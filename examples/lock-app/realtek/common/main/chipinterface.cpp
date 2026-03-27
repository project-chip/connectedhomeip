/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTask.h"
#include "CHIPDeviceManager.h"
#include <stdlib.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

extern "C" void ChipTestShutdown(void)
{
    ChipLogProgress(DeviceLayer, "Lock APP Demo! Shutdown Now!");
    CHIPDeviceManager::GetInstance().Shutdown();
}

bool lowPowerClusterSleep()
{
    return true;
}

extern "C" void InitGPIO(void)
{
    GetAppTask().InitGpio();
}

extern "C" void ChipTest(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "GetAppTask().StartAppTask() failed");
        return;
    }
}
