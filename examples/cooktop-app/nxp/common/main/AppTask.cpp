/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023-2024 NXP
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
#include "CHIPDeviceManager.h"
#include "ICDUtil.h"
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>

using namespace chip::Shell;
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app::Clusters;

void CooktopApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Cooktop Demo App");
}

void CooktopApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());

}

void CooktopApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
#endif
}

// This returns an instance of this class.
CooktopApp::AppTask & CooktopApp::AppTask::GetDefaultInstance()
{
    static CooktopApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return CooktopApp::AppTask::GetDefaultInstance();
}
