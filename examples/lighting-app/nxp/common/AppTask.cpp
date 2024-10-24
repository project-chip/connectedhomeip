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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>

#ifndef APP_DEVICE_TYPE_ENDPOINT
#define APP_DEVICE_TYPE_ENDPOINT 1
#endif

using namespace chip::app::Clusters;

void LightingApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Lighting Demo App");
}

LightingApp::AppTask & LightingApp::AppTask::GetDefaultInstance()
{
    static LightingApp::AppTask sAppTask;
    return sAppTask;
}

bool LightingApp::AppTask::CheckStateClusterHandler(void)
{
    bool val = false;
    OnOff::Attributes::OnOff::Get(APP_DEVICE_TYPE_ENDPOINT, &val);
    return val;
}

CHIP_ERROR LightingApp::AppTask::ProcessSetStateClusterHandler(void)
{
    bool val = false;
    OnOff::Attributes::OnOff::Get(APP_DEVICE_TYPE_ENDPOINT, &val);
    auto status = OnOff::Attributes::OnOff::Set(APP_DEVICE_TYPE_ENDPOINT, (bool) !val);

    VerifyOrReturnError(status == chip::Protocols::InteractionModel::Status::Success, CHIP_ERROR_WRITE_FAILED);

    return CHIP_NO_ERROR;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LightingApp::AppTask::GetDefaultInstance();
}
