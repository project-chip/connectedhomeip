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
#include <app/clusters/dishwasher-alarm-server/dishwasher-alarm-server.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <operational-state-delegate-impl.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app::Clusters;
using namespace app::Clusters::DishwasherAlarm;

extern void MatterDishwasherAlarmServerInit();


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

CHIP_ERROR cliSetAlarmState(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Target State is missing");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!strcmp(argv[0], "set"))
    {
        BitMask<AlarmMap> state;
        if (!strcmp(argv[1], "InflowError"))
        {
            ChipLogDetail(Shell, "Set Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kInflowError, 1);
        }
        else if (!strcmp(argv[1], "DrainError"))
        {
            ChipLogDetail(Shell, "Set Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kDrainError, 1);
        }
        else if (!strcmp(argv[1], "DoorError"))
        {
            ChipLogDetail(Shell, "Set Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kDoorError, 1);
        }
        else if (!strcmp(argv[1], "TempTooLow"))
        {
            ChipLogDetail(Shell, "Set Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kTempTooLow, 1);
        }
        else if (!strcmp(argv[1], "WaterLevelError"))
        {
            ChipLogDetail(Shell, "Set Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kWaterLevelError, 1);
        }
        else
        {
            ChipLogError(Shell, "Invalid Alarm State to set");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        DishwasherAlarmServer::Instance().SetStateValue(1, state);
    }
    else if (!strcmp(argv[0], "clear"))
    {
        BitMask<AlarmMap> state;
        if (!strcmp(argv[1], "InflowError"))
        {
            ChipLogDetail(Shell, "Clear Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kInflowError, 0);
        }
        else if (!strcmp(argv[1], "DrainError"))
        {
            ChipLogDetail(Shell, "Clear Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kDrainError, 0);
        }
        else if (!strcmp(argv[1], "DoorError"))
        {
            ChipLogDetail(Shell, "Clear Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kDoorError, 0);
        }
        else if (!strcmp(argv[1], "TempTooLow"))
        {
            ChipLogDetail(Shell, "Clear Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kTempTooLow, 0);
        }
        else if (!strcmp(argv[1], "WaterLevelError"))
        {
            ChipLogDetail(Shell, "Clear Dishwasher Alarm State: %s state", argv[1]);
            state.SetField(AlarmMap::kWaterLevelError, 0);
        }
        else
        {
            ChipLogError(Shell, "Invalid Alarm State to set");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
         DishwasherAlarmServer::Instance().SetStateValue(1, state);
    }
    else
    {
        ChipLogError(Shell, "Invalid Command");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

void DishwasherApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Dish Washer Demo App");
}

void DishwasherApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void DishwasherApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliOpState, .cmd_name = "opstate", .cmd_help = "Set the Operational State, Usage: opstate run|stop|pause|error no_error/unable_to_start_or_resume/unable_to_complete_operation/command_invalid_in_state" },
        { .cmd_func = cliSetAlarmState, .cmd_name = "alarmstate", .cmd_help = "Set/Clear the Dishwasher Alarm State, Usage: alarmstate set/clear InflowError|DrainError|DoorError|TempTooLow|WaterLevelError" },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

void DishwasherApp::AppTask::PostInitMatterServerInstance() 
{
    MatterDishwasherAlarmServerInit();
}

// This returns an instance of this class.
DishwasherApp::AppTask & DishwasherApp::AppTask::GetDefaultInstance()
{
    static DishwasherApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return DishwasherApp::AppTask::GetDefaultInstance();
}
