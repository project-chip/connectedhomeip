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
#include "binding-handler.h"
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_TBR
#include <openthread/cli.h>
#include "platform/OpenThread/GenericThreadBorderRouterDelegate.h"
#include <app/clusters/thread-border-router-management-server/thread-border-router-management-server.h>
#endif

#ifndef APP_DEVICE_TYPE_ENDPOINT
#define APP_DEVICE_TYPE_ENDPOINT 1
#endif

using namespace chip;
using namespace chip::app::Clusters;

void LightSwitchComboApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Light Switch Combo Demo App");
}

void LightSwitchComboApp::AppTask::PostInitMatterServerInstance()
{
#if CHIP_DEVICE_CONFIG_ENABLE_TBR
    Thread::OperationalDataset dataset;
    auto * persistentStorage = &Server::GetInstance().GetPersistentStorage();
    static ThreadBorderRouterManagement::GenericOpenThreadBorderRouterDelegate sThreadBRDelegate(persistentStorage);
    if (sThreadBRDelegate.GetDataset(dataset, ThreadBorderRouterManagement::Delegate::DatasetType::kActive) != CHIP_NO_ERROR) {
        // Init the dafault dataset
        ChipLogProgress(DeviceLayer, "Initialize the default dataset");
        chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
        otCliInputLine((char*)"dataset init new");
        otCliInputLine((char*)"dataset");
        otCliInputLine((char*)"dataset commit active");
        chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
    }
    ChipLogProgress(DeviceLayer, "Activate the OT BSS");
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
    // Activate the ot bss
    otCliInputLine((char*)"ifconfig up");
    otCliInputLine((char*)"thread start");
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
#endif
}

LightSwitchComboApp::AppTask & LightSwitchComboApp::AppTask::GetDefaultInstance()
{
    static LightSwitchComboApp::AppTask sAppTask;
    return sAppTask;
}

bool LightSwitchComboApp::AppTask::CheckStateClusterHandler(void)
{
    bool val = false;
    OnOff::Attributes::OnOff::Get(APP_DEVICE_TYPE_ENDPOINT, &val);
    return val;
}

CHIP_ERROR LightSwitchComboApp::AppTask::ProcessSetStateClusterHandler(void)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::OnOff::Commands::Toggle::Id;
    data->clusterId           = chip::app::Clusters::OnOff::Id;
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));

    return CHIP_NO_ERROR;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LightSwitchComboApp::AppTask::GetDefaultInstance();
}
