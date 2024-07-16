/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "controller/InvokeInteraction.h"
#include "controller/ReadInteraction.h"
#include "operational-state-delegate-impl.h"
#include "oven-operational-state-delegate.h"
#include "rvc-operational-state-delegate-impl.h"

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;

#if CONFIG_ENABLE_CHIP_SHELL
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellManualOperationalStateSubCommands;
Engine sShellManualRVCOperationalStateSubCommands;
Engine sShellManualOvenCavityOperationalStateSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

#if CONFIG_ENABLE_CHIP_SHELL
/********************************************************
 * Operational State Functions
 *********************************************************/

CHIP_ERROR ManualOperationalStateCommandHelpHandler(int argc, char ** argv)
{
    sShellManualOperationalStateSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualOperationalStateCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualOperationalStateCommandHelpHandler(argc, argv);
    }

    return sShellManualOperationalStateSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ManualOperationalStateSetStateCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualOperationalStateCommandHelpHandler(argc, argv);
    }
    uint32_t state = atoi(argv[0]);

    CHIP_ERROR err;
    err = GetOperationalStateInstance()->SetOperationalState(state);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualOperationalStateSetStateCommandHandler Failed!\r\n");
    }

    return err;
}

CHIP_ERROR ManualOperationalStateSetErrorCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualOperationalStateCommandHelpHandler(argc, argv);
    }

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint32_t error = atoi(argv[0]);

    switch (error)
    {
    case to_underlying(OperationalState::ErrorStateEnum::kNoError):                   // 0x00, 0
    case to_underlying(OperationalState::ErrorStateEnum::kUnableToStartOrResume):     // 0x01, 1
    case to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation): // 0x02, 2
    case to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState):     // 0x03, 3
        err.errorStateID = error;
        break;
    default:
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kUnknownEnumValue); // 0x04, 4
        break;
    }

    GetOperationalStateInstance()->OnOperationalErrorDetected(err);

    return CHIP_NO_ERROR;
}

/********************************************************
 * RVC Operational State Functions
 *********************************************************/

CHIP_ERROR ManualRVCOperationalStateCommandHelpHandler(int argc, char ** argv)
{
    sShellManualRVCOperationalStateSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualRVCOperationalStateCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualRVCOperationalStateCommandHelpHandler(argc, argv);
    }

    return sShellManualRVCOperationalStateSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ManualRVCOperationalStateSetStateCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualRVCOperationalStateCommandHelpHandler(argc, argv);
    }
    uint32_t state = atoi(argv[0]);

    CHIP_ERROR err;
    err = RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(state);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualRVCOperationalStateSetStateCommandHandler Failed!\r\n");
    }

    return err;
}

CHIP_ERROR ManualRVCOperationalStateSetErrorCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualRVCOperationalStateCommandHelpHandler(argc, argv);
    }

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint32_t error = atoi(argv[0]);

    switch (error)
    {
    case to_underlying(OperationalState::ErrorStateEnum::kNoError):                     // 0x00, 0
    case to_underlying(OperationalState::ErrorStateEnum::kUnableToStartOrResume):       // 0x01, 1
    case to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation):   // 0x02, 2
    case to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState):       // 0x03, 3
    case to_underlying(RvcOperationalState::ErrorStateEnum::kFailedToFindChargingDock): // 0x40, 64
    case to_underlying(RvcOperationalState::ErrorStateEnum::kStuck):                    // 0x41, 65
    case to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinMissing):           // 0x42, 66
    case to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinFull):              // 0x43, 67
    case to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankEmpty):           // 0x44, 68
    case to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankMissing):         // 0x45, 69
    case to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankLidOpen):         // 0x46, 70
    case to_underlying(RvcOperationalState::ErrorStateEnum::kMopCleaningPadMissing):    // 0x47, 71
        err.errorStateID = error;
        break;
    default:
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kUnknownEnumValue); // 0x04, 4
        break;
    }

    RvcOperationalState::GetRvcOperationalStateInstance()->OnOperationalErrorDetected(err);

    return CHIP_NO_ERROR;
}

/********************************************************
 * Oven Cavity Operational State Functions
 *********************************************************/

CHIP_ERROR ManualOvenCavityOperationalStateCommandHelpHandler(int argc, char ** argv)
{
    sShellManualOvenCavityOperationalStateSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualOvenCavityOperationalStateCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualOvenCavityOperationalStateCommandHelpHandler(argc, argv);
    }

    return sShellManualOvenCavityOperationalStateSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ManualOvenCavityOperationalStateSetStateCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualOvenCavityOperationalStateCommandHelpHandler(argc, argv);
    }
    uint32_t state = atoi(argv[0]);

    CHIP_ERROR err;
    err = OvenCavityOperationalState::GetOperationalStateInstance()->SetOperationalState(state);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualOvenCavityOperationalStateSetStateCommandHandler Failed!\r\n");
    }

    return err;
}

CHIP_ERROR ManualOvenCavityOperationalStateSetErrorCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualOvenCavityOperationalStateCommandHelpHandler(argc, argv);
    }

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint32_t error = atoi(argv[0]);

    switch (error)
    {
    case to_underlying(OvenCavityOperationalState::ErrorStateEnum::kNoError):                   // 0x00, 0
    case to_underlying(OvenCavityOperationalState::ErrorStateEnum::kUnableToStartOrResume):     // 0x01, 1
    case to_underlying(OvenCavityOperationalState::ErrorStateEnum::kUnableToCompleteOperation): // 0x02, 2
    case to_underlying(OvenCavityOperationalState::ErrorStateEnum::kCommandInvalidInState):     // 0x03, 3
        err.errorStateID = error;
        break;
    default:
        err.errorStateID = to_underlying(OvenCavityOperationalState::ErrorStateEnum::kUnknownEnumValue); // 0x04, 4
        break;
    }

    OvenCavityOperationalState::GetOperationalStateInstance()->OnOperationalErrorDetected(err);

    return CHIP_NO_ERROR;
}
#endif // CONFIG_ENABLE_CHIP_SHELL
