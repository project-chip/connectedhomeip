/*
 *
 *    Copyright (c) 2024-2025 Project CHIP Authors
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
#include "ICDUtil.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/InteractionModelEngine.h>
#include <platform/CHIPDeviceLayer.h>

// TODO: Ideally we should not depend on the codegen integration
// It would be best if we could use generic cluster API instead
#include <app/clusters/boolean-state-server/CodegenIntegration.h>

#ifndef APP_DEVICE_TYPE_ENDPOINT
#define APP_DEVICE_TYPE_ENDPOINT 1
#endif

using namespace chip::app::Clusters;

void ContactSensorApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Contact Sensor Demo App");
}

void ContactSensorApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

ContactSensorApp::AppTask & ContactSensorApp::AppTask::GetDefaultInstance()
{
    static ContactSensorApp::AppTask sAppTask;
    return sAppTask;
}

bool ContactSensorApp::AppTask::CheckStateClusterHandler(void)
{
    auto booleanState = BooleanState::FindClusterOnEndpoint(APP_DEVICE_TYPE_ENDPOINT);
    VerifyOrReturnError(booleanState != nullptr, false);
    bool val = booleanState->GetStateValue();
    return val;
}

CHIP_ERROR ContactSensorApp::AppTask::ProcessSetStateClusterHandler(void)
{
    auto booleanState = BooleanState::FindClusterOnEndpoint(APP_DEVICE_TYPE_ENDPOINT);
    VerifyOrReturnError(booleanState != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    bool val = booleanState->GetStateValue();
    booleanState->SetStateValue(!val);

    return CHIP_NO_ERROR;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return ContactSensorApp::AppTask::GetDefaultInstance();
}
