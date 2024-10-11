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

#pragma once

#include "controller/InvokeInteraction.h"
#include "controller/ReadInteraction.h"
#include <app/clusters/refrigerator-alarm-server/refrigerator-alarm-server.h>
#include <protocols/interaction_model/StatusCode.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAlarm;

#if CONFIG_ENABLE_CHIP_SHELL
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellManualRefrigeratorAlarmStateSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

#if CONFIG_ENABLE_CHIP_SHELL

CHIP_ERROR ManualRefrigeratorAlarmCommandHelpHandler(int argc, char ** argv)
{
    sShellManualRefrigeratorAlarmStateSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualRefrigeratorAlarmCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualRefrigeratorAlarmCommandHelpHandler(argc, argv);
    }

    return sShellManualRefrigeratorAlarmStateSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ManualRefrigeratorAlarmDoorOpenCommandHandler(int argc, char ** argv)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Protocols::InteractionModel::Status status;
    RefrigeratorAlarmServer & serverInstance = RefrigeratorAlarmServer::Instance();

    status = serverInstance.SetMaskValue(1, 0);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetStateValue(1, 1);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetSupportedValue(1, 0);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualRefrigeratorAlarmDoorOpenCommandHandler Failed!\r\n");
    }

    return err;
}

CHIP_ERROR ManualRefrigeratorAlarmDoorCloseCommandHandler(int argc, char ** argv)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Protocols::InteractionModel::Status status;
    RefrigeratorAlarmServer & serverInstance = RefrigeratorAlarmServer::Instance();

    status = serverInstance.SetMaskValue(1, 1);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetStateValue(1, 0);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetSupportedValue(1, 1);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualRefrigeratorAlarmDoorCloseCommandHandler Failed!\r\n");
    }

    return err;
}

CHIP_ERROR ManualRefrigeratorAlarmSuppressCommandHandler(int argc, char ** argv)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Protocols::InteractionModel::Status status;
    RefrigeratorAlarmServer & serverInstance = RefrigeratorAlarmServer::Instance();

    status = serverInstance.SetSupportedValue(1, 1);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    status = serverInstance.SetStateValue(1, 0);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ManualRefrigeratorAlarmDoorCloseCommandHandler Failed!\r\n");
    }

    return err;
}
#endif // CONFIG_ENABLE_CHIP_SHELL
