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
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app::Clusters;

/*
 * Enable temperature level delegate of temperature control cluster
 */
app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;
void emberAfTemperatureControlClusterInitCallback(EndpointId endpoint)
{
    TemperatureControl::SetInstance(&sAppSupportedTemperatureLevelsDelegate);
}

#ifdef ENABLE_CHIP_SHELL
const static std::map<std::string, uint8_t> map_cmd_errstate{
    { "no_error", (uint8_t) OperationalState::ErrorStateEnum::kNoError },
    { "unable_to_start_or_resume", (uint8_t) OperationalState::ErrorStateEnum::kUnableToStartOrResume },
    { "unable_to_complete_operation", (uint8_t) OperationalState::ErrorStateEnum::kUnableToCompleteOperation },
    { "command_invalid_in_state", (uint8_t) OperationalState::ErrorStateEnum::kCommandInvalidInState }
};

const static std::map<std::string, uint8_t> map_cmd_opstate{ { "stop", (uint8_t) OperationalState::OperationalStateEnum::kStopped },
                                                             { "run", (uint8_t) OperationalState::OperationalStateEnum::kRunning },
                                                             { "pause", (uint8_t) OperationalState::OperationalStateEnum::kPaused },
                                                             { "error",
                                                               (uint8_t) OperationalState::OperationalStateEnum::kError } };

static void InvalidStateHandler(void)
{
    ChipLogError(Shell, "Invalid State/Error to set");
    MATTER_CLI_LOG("Invalid. Supported commands are:\n");
    for (auto const & it : map_cmd_opstate)
    {
        MATTER_CLI_LOG(("\t opstate " + it.first + "\n").c_str());
    }
    for (auto const & it : map_cmd_errstate)
    {
        MATTER_CLI_LOG(("\t opstate error " + it.first + "\n").c_str());
    }
}

static CHIP_ERROR cliOpState(int argc, char * argv[])
{
    bool inputErr = false;
    if ((argc != 1) && (argc != 2))
    {
        inputErr = true;
        goto exit;
    }

    if (map_cmd_opstate.find(argv[0]) != map_cmd_opstate.end())
    {
        OperationalState::GetOperationalStateInstance()->SetOperationalState(map_cmd_opstate.at(argv[0]));
        ChipLogDetail(Shell, "OpSState : Set to %s state", argv[0]);
        if (!strcmp(argv[0], "error") && argc == 2)
        {
            OperationalState::Structs::ErrorStateStruct::Type err;
            if (map_cmd_errstate.find(argv[1]) != map_cmd_errstate.end())
            {
                ChipLogDetail(Shell, "OpSState_error : Set to %s state", argv[1]);
                err.errorStateID = map_cmd_errstate.at(argv[1]);
                OperationalState::GetOperationalStateInstance()->OnOperationalErrorDetected(err);
            }
            else
            {
                inputErr = true;
                goto exit;
            }
        }
    }
    else
    {
        inputErr = true;
    }
exit:
    if (inputErr)
    {
        InvalidStateHandler();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}
#endif /* ENABLE_CHIP_SHELL */

void LaundryWasherApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP laundry washer Demo App");
}

void LaundryWasherApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void LaundryWasherApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliOpState,
          .cmd_name = "opstate",
          .cmd_help = "Set the Operational State. Usage:[stop|run|pause|dock|error 'state'] " },
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
