/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AppTask.h"

#if CONFIG_LOW_POWER
#include "PWR_Interface.h"
#endif

#include <platform/CHIPDeviceLayer.h>

void ContactSensorApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Contact Sensor Demo App");
}

#if CONFIG_LOW_POWER
void ContactSensorApp::AppTask::AppMatter_DisallowDeviceToSleep()
{
    PWR_DisallowDeviceToSleep();
}

void ContactSensorApp::AppTask::AppMatter_AllowDeviceToSleep()
{
    PWR_AllowDeviceToSleep();
}
#endif

ContactSensorApp::AppTask & ContactSensorApp::AppTask::GetDefaultInstance()
{
    static ContactSensorApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return ContactSensorApp::AppTask::GetDefaultInstance();
}
