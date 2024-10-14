/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Provides a glue layer between Matter and NXP-SDK Low Power
 */

#if defined(chip_with_low_power) && (chip_with_low_power == 1)

#include <platform/CHIPDeviceLayer.h>
#include <platform/ThreadStackManager.h>

extern "C" void PWR_DisallowDeviceToSleep(void);
extern "C" void PWR_AllowDeviceToSleep(void);

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

extern "C" void App_AllowDeviceToSleep()
{
    ;
}

extern "C" void App_DisallowDeviceToSleep()
{
    ;
}

#endif
