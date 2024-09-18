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
#include "BLEApplicationManager.h"
#include "CHIPDeviceManager.h"
#include "LockManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/data-model/Nullable.h>
#include <app/util/attribute-storage.h>

#if !CHIP_CONFIG_ENABLE_ICD_SERVER
#include "ICDUtil.h"
#endif

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

#ifndef APP_DEVICE_TYPE_ENDPOINT
#define APP_DEVICE_TYPE_ENDPOINT 1
#endif

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::Protocols::InteractionModel;

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
        DoorLockServer::Instance().SetLockState(1, DlLockState::kLocked, OperationSourceEnum::kManual, NullNullable, NullNullable,
                                                NullNullable, NullNullable);
    }
    else if (!strcmp(argv[0], "unlocked"))
    {
        ChipLogDetail(Shell, "Lock : lock set to %s state", argv[0]);
        DoorLockServer::Instance().SetLockState(1, DlLockState::kUnlocked, OperationSourceEnum::kManual, NullNullable, NullNullable,
                                                NullNullable, NullNullable);
    }
    else if (!strcmp(argv[0], "unlatched"))
    {
        ChipLogDetail(Shell, "Lock : lock set to %s state", argv[0]);
        DoorLockServer::Instance().SetLockState(1, DlLockState::kUnlatched, OperationSourceEnum::kManual, NullNullable,
                                                NullNullable, NullNullable, NullNullable);
    }
    else if (!strcmp(argv[0], "notfullylocked"))
    {
        ChipLogDetail(Shell, "Lock : lock set to %s state", argv[0]);
        DoorLockServer::Instance().SetLockState(1, DlLockState::kNotFullyLocked, OperationSourceEnum::kManual, NullNullable,
                                                NullNullable, NullNullable, NullNullable);
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

    /* BLEApplicationManager implemented per platform or left blank */
    chip::NXP::App::BleAppMgr().Init();
}

void LockApp::AppTask::PostInitMatterStack()
{
#if !CHIP_CONFIG_ENABLE_ICD_SERVER
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
#endif
}

void LockApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliDoorLock,
          .cmd_name = "doorlock",
          .cmd_help = "Set the Door Lock State or trigger Lock Jammed Alarm. "
                      "Usage:[open|closed|jammedalarm|locked|unlocked|notfullylocked|unlatched] " },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

bool LockApp::AppTask::CheckStateClusterHandler(void)
{
    Nullable<DoorLock::DlLockState> state(DlLockState::kUnlocked);
    DoorLock::Attributes::LockState::Get(APP_DEVICE_TYPE_ENDPOINT, state);
    return (state.Value() == DlLockState::kUnlocked);
}

CHIP_ERROR LockApp::AppTask::ProcessSetStateClusterHandler(void)
{
    Nullable<DoorLock::DlLockState> state;
    DoorLock::Attributes::LockState::Get(APP_DEVICE_TYPE_ENDPOINT, state);
    auto newState = (state.Value() == DlLockState::kUnlocked) ? DlLockState::kLocked : DlLockState::kUnlocked;
    auto status   = DoorLock::Attributes::LockState::Set(APP_DEVICE_TYPE_ENDPOINT, newState);

    VerifyOrReturnError(status == Status::Success, CHIP_ERROR_WRITE_FAILED);

    return CHIP_NO_ERROR;
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
