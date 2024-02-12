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
#include <app/clusters/dishwasher-alarm-server/dishwasher-alarm-server.h>
#include <protocols/interaction_model/StatusCode.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DishwasherAlarm;

#if CONFIG_ENABLE_CHIP_SHELL
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellManualDishWasherAlarmStateSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

#if CONFIG_ENABLE_CHIP_SHELL

CHIP_ERROR ManualDishWasherAlarmCommandHelpHandler(int argc, char ** argv)
{
    sShellManualDishWasherAlarmStateSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualDishWasherAlarmCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualDishWasherAlarmCommandHelpHandler(argc, argv);
    }

    return sShellManualDishWasherAlarmStateSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ManualDishWasherAlarmSetRaiseCommandHandler(int argc, char ** argv)
{
    if (argc != 0)
    {
        return ManualDishWasherAlarmCommandHelpHandler(argc, argv);
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    Protocols::InteractionModel::Status status;
    DishwasherAlarmServer & serverInstance = DishwasherAlarmServer::Instance();

    BitMask<AlarmMap> supported;                       // Set dishwasher alarm supported value
    supported.SetField(AlarmMap::kInflowError, 1);     // 0x01, 1
    supported.SetField(AlarmMap::kDrainError, 1);      // 0x02, 2
    supported.SetField(AlarmMap::kDoorError, 1);       // 0x04, 4
    supported.SetField(AlarmMap::kTempTooLow, 1);      // 0x08, 8
    supported.SetField(AlarmMap::kWaterLevelError, 1); // 0x20, 32

    BitMask<AlarmMap> mask;                       // Set dishwasher alarm mask value
    mask.SetField(AlarmMap::kInflowError, 1);     // 0x01, 1
    mask.SetField(AlarmMap::kDrainError, 1);      // 0x02, 2
    mask.SetField(AlarmMap::kDoorError, 1);       // 0x04, 4
    mask.SetField(AlarmMap::kTempTooLow, 1);      // 0x08, 8
    mask.SetField(AlarmMap::kWaterLevelError, 1); // 0x20, 32

    BitMask<AlarmMap> state;                  // Set dishwasher alarm state value
    state.SetField(AlarmMap::kDrainError, 1); // 0x02, 2
    state.SetField(AlarmMap::kDoorError, 1);  // 0x04, 4
    state.SetField(AlarmMap::kTempTooLow, 1); // 0x08, 8

    status = serverInstance.SetSupportedValue(1, supported); // 0x2F, 47
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetMaskValue(1, mask); // 0x2F, 47
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetStateValue(1, state); // 0x0E, 14
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualDishWasherAlarmSetRaiseCommandHandler Failed!\r\n");
    }

    return err;
}

CHIP_ERROR ManualDishWasherAlarmSetLowerCommandHandler(int argc, char ** argv)
{
    if (argc != 0)
    {
        return ManualDishWasherAlarmCommandHelpHandler(argc, argv);
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    Protocols::InteractionModel::Status status;
    DishwasherAlarmServer & serverInstance = DishwasherAlarmServer::Instance();

    BitMask<AlarmMap> supported;                       // Set dishwasher alarm supported value
    supported.SetField(AlarmMap::kInflowError, 1);     // 0x01, 1
    supported.SetField(AlarmMap::kDrainError, 1);      // 0x02, 2
    supported.SetField(AlarmMap::kDoorError, 1);       // 0x04, 4
    supported.SetField(AlarmMap::kTempTooLow, 1);      // 0x08, 8
    supported.SetField(AlarmMap::kWaterLevelError, 1); // 0x20, 32

    BitMask<AlarmMap> mask;                       // Set dishwasher alarm mask value
    mask.SetField(AlarmMap::kInflowError, 1);     // 0x01, 1
    mask.SetField(AlarmMap::kDrainError, 1);      // 0x02, 2
    mask.SetField(AlarmMap::kDoorError, 1);       // 0x04, 4
    mask.SetField(AlarmMap::kTempTooLow, 1);      // 0x08, 8
    mask.SetField(AlarmMap::kWaterLevelError, 1); // 0x20, 32

    status = serverInstance.SetSupportedValue(1, supported); // 0x2F, 47
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetMaskValue(1, mask); // 0x2F, 47
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetStateValue(1, 0); // Set dishwasher alarm state value 0x00, 0
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualDishWasherAlarmSetLowerCommandHandler Failed!\r\n");
    }

    return err;
}
#endif // CONFIG_ENABLE_CHIP_SHELL
