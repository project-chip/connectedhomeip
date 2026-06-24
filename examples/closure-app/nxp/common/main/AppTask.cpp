/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023-2024 NXP
 *    Copyright 2025 NXP
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

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include "BLEApplicationManager.h"
#endif

#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"
#include "ClosureManager.h"

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

static CHIP_ERROR cliClosure(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Invalid Argument");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!strcmp(argv[0], "open"))
    {
        ChipLogDetail(Shell, "####Closure Open");

        ClosureManager::GetInstance().InvokeOpenCommand();
    }
    else if (!strcmp(argv[0], "close"))
    {
        ChipLogDetail(Shell, "####Closure Close");

        ClosureManager::GetInstance().InvokeCloseCommand();
    }
	else if (!strcmp(argv[0], "stop"))
    {
        ChipLogDetail(Shell, "####Closure Stop");

        ClosureManager::GetInstance().InvokeStopCommand();
    }
	else if (!strcmp(argv[0], "unlatch"))
    {
        ChipLogDetail(Shell, "####Closure Unlatch");

        ClosureManager::GetInstance().InvokeCloseCommand();
    }
    else
    {
        ChipLogError(Shell, "Invalid State to set");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

void ClosureApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Closure Demo App");
}

void ClosureApp::AppTask::PostInitMatterStack()
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#ifdef APP_BT_DEVICE_NAME
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(APP_BT_DEVICE_NAME);
#endif
    /* BLEApplicationManager implemented per platform or left blank */
    chip::NXP::App::BleAppMgr().Init();
#endif
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void ClosureApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliClosure,
          .cmd_name = "closure",
          .cmd_help = "open/close/stop/unlatch the closure, open|close|stop|unlatch " },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

void ClosureApp::AppTask::PostInitMatterServerInstance()
{
    ChipLogDetail(NotSpecified, "ApplicationInit Start()");
    ClosureManager::GetInstance().Init();
    ChipLogDetail(NotSpecified, "ApplicationInit Complete()");
}

// This returns an instance of this class.
ClosureApp::AppTask & ClosureApp::AppTask::GetDefaultInstance()
{
    static ClosureApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return ClosureApp::AppTask::GetDefaultInstance();
}
