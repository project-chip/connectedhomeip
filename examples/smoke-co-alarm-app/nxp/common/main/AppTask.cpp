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
#include <app/clusters/smoke-co-alarm-server/smoke-co-alarm-server.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;

static CHIP_ERROR cliSelfTest(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Invalid Argument");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!strcmp(argv[0], "start"))
    {
        ChipLogDetail(Shell, "Start self test");
        SmokeCOAlarmApp::AppTask::GetDefaultInstance().AlarmSelfTestHandler();
    }
    else
    {
        ChipLogError(Shell, "Invalid command");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}


void SmokeCOAlarmApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Smoke CO Alarm Demo App");
}

void SmokeCOAlarmApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
	if (AlarmMgr().Init() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Init AlarmMgr failed");
    }
}

void SmokeCOAlarmApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliSelfTest,
          .cmd_name = "selftest",
          .cmd_help = "start alarm self test, usage: selftest start " },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
SmokeCOAlarmApp::AppTask & SmokeCOAlarmApp::AppTask::GetDefaultInstance()
{
    static SmokeCOAlarmApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return SmokeCOAlarmApp::AppTask::GetDefaultInstance();
}

void SmokeCOAlarmApp::AppTask::AlarmSelfTestHandler(void)
{

    bool success = SmokeCoAlarmServer::Instance().RequestSelfTest(1);

    if (!success)
    {
        ChipLogError(DeviceLayer, "Manual self-test failed");
    }
}
