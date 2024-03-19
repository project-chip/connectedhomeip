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

#include "static-supported-temperature-levels.h"
#include <operational-state-delegate-impl.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>

using namespace chip::Shell;
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app::Clusters;

app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;

CHIP_ERROR cliOpState(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Target State is missing");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!strcmp(argv[0], "stop"))
    {
        ChipLogDetail(Shell, "OpSState : Set to %s state", argv[0]);
        OperationalState::GetOperationalStateInstance()->SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kStopped));
    }
    else if (!strcmp(argv[0], "run"))
    {
        ChipLogDetail(Shell, "OpSState : Set to %s state", argv[0]);
        OperationalState::GetOperationalStateInstance()->SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kRunning));
    }
    else if (!strcmp(argv[0], "pause"))
    {
        ChipLogDetail(Shell, "OpSState : Set to %s state", argv[0]);
        OperationalState::GetOperationalStateInstance()->SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kPaused));
    }
    else if (!strcmp(argv[0], "error"))
    {
        OperationalState::Structs::ErrorStateStruct::Type err;
        ChipLogDetail(Shell, "OpSState : Set to %s state", argv[0]);
        if (!strcmp(argv[1], "no_error"))
        {
            ChipLogDetail(Shell, "OpSState_error : Error: %s state", argv[1]);
            err.errorStateID = (uint8_t) OperationalState::ErrorStateEnum::kNoError;
        }
        else if (!strcmp(argv[1], "unable_to_start_or_resume"))
        {
            ChipLogDetail(Shell, "OpSState_error : Error: %s state", argv[1]);
            err.errorStateID = (uint8_t) OperationalState::ErrorStateEnum::kUnableToStartOrResume;
        }
        else if (!strcmp(argv[1], "unable_to_complete_operation"))
        {
            ChipLogDetail(Shell, "OpSState_error : Error: %s state", argv[1]);
            err.errorStateID = (uint8_t) OperationalState::ErrorStateEnum::kUnableToCompleteOperation;
        }
        else if (!strcmp(argv[1], "command_invalid_in_state"))
        {
            ChipLogDetail(Shell, "OpSState_error : Error: %s state", argv[1]);
            err.errorStateID = (uint8_t) OperationalState::ErrorStateEnum::kCommandInvalidInState;
        }
        else
        {
            ChipLogError(Shell, "Invalid Error State to set");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        OperationalState::GetOperationalStateInstance()->OnOperationalErrorDetected(err);
        OperationalState::GetOperationalStateInstance()->SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kError));
    }
    else
    {
        ChipLogError(Shell, "Invalid State to set");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

void LaundryWasherApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP laundry washer Demo App");
}

void LaundryWasherApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());

    app::Clusters::TemperatureControl::SetInstance(&sAppSupportedTemperatureLevelsDelegate);
}

void LaundryWasherApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliOpState, .cmd_name = "opstate", .cmd_help = "Set the Operational State" },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
LaundryWasherApp::AppTask & LaundryWasherApp::AppTask::GetDefaultInstance()
{
    static LaundryWasherApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return LaundryWasherApp::AppTask::GetDefaultInstance();
}
