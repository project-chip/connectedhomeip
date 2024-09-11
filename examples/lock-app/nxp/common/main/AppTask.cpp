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
#include <app/clusters/door-lock-server/door-lock-server.h>
#include "LockManager.h"
#include <app/data-model/Nullable.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;

#ifdef ENABLE_CHIP_SHELL

static CHIP_ERROR cliDoorLock(int argc, char * argv[])
{
    bool inputErr = false;
    if ((argc != 1) && (argc != 2))
    {
        inputErr = true;
        goto exit;
    }

	if (!strcmp(argv[0], "open"))
    {
        ChipLogDetail(Shell, "Door : Set to %s state", argv[0]);
		DoorLockServer::Instance().SetDoorState(1, DoorStateEnum::kDoorOpen);
    }
    else if (!strcmp(argv[0], "closed"))
    {
        ChipLogDetail(Shell, "Door : Set to %s state", argv[0]);
		DoorLockServer::Instance().SetDoorState(1, DoorStateEnum::kDoorClosed);
    }
    else if (!strcmp(argv[0], "jammedalarm"))
    {
        ChipLogDetail(Shell, "Send out Lock Jammed Alarm...");
		DoorLockServer::Instance().SendLockAlarmEvent(1, AlarmCodeEnum::kLockJammed);
    }
	else if (!strcmp(argv[0], "locked"))
    {
        ChipLogDetail(Shell, "Lock : lock set to %s state", argv[0]);
		DoorLockServer::Instance().SetLockState(1, DlLockState::kLocked, OperationSourceEnum::kManual, NullNullable, NullNullable, NullNullable, NullNullable);
    }
	else if (!strcmp(argv[0], "unlocked"))
    {
        ChipLogDetail(Shell, "Lock : lock set to %s state", argv[0]);
		DoorLockServer::Instance().SetLockState(1, DlLockState::kUnlocked, OperationSourceEnum::kManual, NullNullable, NullNullable, NullNullable, NullNullable);
    }
	else if (!strcmp(argv[0], "unlatched"))
    {
        ChipLogDetail(Shell, "Lock : lock set to %s state", argv[0]);
		DoorLockServer::Instance().SetLockState(1, DlLockState::kUnlatched, OperationSourceEnum::kManual, NullNullable, NullNullable, NullNullable, NullNullable);
    }
	else if (!strcmp(argv[0], "notfullylocked"))
    {
        ChipLogDetail(Shell, "Lock : lock set to %s state", argv[0]);
		DoorLockServer::Instance().SetLockState(1, DlLockState::kNotFullyLocked, OperationSourceEnum::kManual, NullNullable, NullNullable, NullNullable, NullNullable);
    }
    else
    {
        inputErr = true;
    }
exit:
    if (inputErr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}
#endif /* ENABLE_CHIP_SHELL */

void LockApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Lock Demo App");
}

void LockApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void LockApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliDoorLock,
          .cmd_name = "doorlock",
          .cmd_help = "Set the Door Lock State or trigger Lock Jammed Alarm. Usage:[open|closed|jammedalarm|locked|unlocked|notfullylocked|unlatched] " },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
LockApp::AppTask & LockApp::AppTask::GetDefaultInstance()
{
    static LockApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LockApp::AppTask::GetDefaultInstance();
}

