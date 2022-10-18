/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "EventHandler.h"

#if defined(ENABLE_CHIP_SHELL)
#include "AppTask.h"
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"

#define ENDPOINT_1 1

#include <lib/support/CodeUtils.h>

#include "app/server/Server.h"
#include "platform/CHIPDeviceLayer.h"

using namespace chip;
using namespace chip::app;
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellDoorlockSubCommands;
Engine sShellDoorlockEventSubCommands;
Engine sShellDoorlockEventAlarmSubCommands;
Engine sShellDoorlockEventDoorStateSubCommands;

/********************************************************
 * Doorlock shell functions
 *********************************************************/

CHIP_ERROR DoorlockHelpHandler(int argc, char ** argv)
{
    sShellDoorlockSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DoorlockCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return DoorlockHelpHandler(argc, argv);
    }

    return sShellDoorlockSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * Event shell functions
 *********************************************************/

CHIP_ERROR EventHelpHandler(int argc, char ** argv)
{
    sShellDoorlockEventSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventDoorlockCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return EventHelpHandler(argc, argv);
    }

    return sShellDoorlockEventSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * Alarm shell functions
 *********************************************************/

CHIP_ERROR AlarmHelpHandler(int argc, char ** argv)
{
    sShellDoorlockEventAlarmSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AlarmEventHandler(int argc, char ** argv)
{

    if (argc == 0)
    {
        return AlarmHelpHandler(argc, argv);
    }
    if (argc >= 2)
    {
        ChipLogError(Zcl, "Too many arguments provided to function %s, line %d", __func__, __LINE__);
        return APP_ERROR_TOO_MANY_SHELL_ARGUMENTS;
    }

    AlarmEventData * data = Platform::New<AlarmEventData>();
    data->alarmCode       = static_cast<DlAlarmCode>(atoi(argv[0]));

    DeviceLayer::PlatformMgr().ScheduleWork(AlarmEventWorkerFunction, reinterpret_cast<intptr_t>(data));

    return CHIP_NO_ERROR;
}

/********************************************************
 * Door state shell functions
 *********************************************************/

CHIP_ERROR DoorStateHelpHandler(int argc, char ** argv)
{
    sShellDoorlockEventDoorStateSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DoorStateEventHandler(int argc, char ** argv)
{

    if (argc == 0)
    {
        return DoorStateHelpHandler(argc, argv);
    }
    if (argc >= 2)
    {
        ChipLogError(Zcl, "Too many arguments provided to function %s, line %d", __func__, __LINE__);
        return APP_ERROR_TOO_MANY_SHELL_ARGUMENTS;
    }

    DoorStateEventData * data = Platform::New<DoorStateEventData>();
    data->doorState           = static_cast<DlDoorState>(atoi(argv[0]));

    DeviceLayer::PlatformMgr().ScheduleWork(DoorStateEventWorkerFunction, reinterpret_cast<intptr_t>(data));

    return CHIP_NO_ERROR;
}

/**
 * @brief configures lock matter shell
 *
 */

CHIP_ERROR RegisterLockEvents()
{
    static const shell_command_t sDoorlockSubCommands[] = { { &DoorlockHelpHandler, "help", "Usage: doorlock <subcommand>" },
                                                            { &EventDoorlockCommandHandler, "event",
                                                              " Usage: doorlock event <subcommand>" } };

    static const shell_command_t sDoorlockEventSubCommands[] = {
        { &EventHelpHandler, "help", "Usage : doorlock event <subcommand>" },
        { &AlarmEventHandler, "lock-alarm", "Sends lock alarm event to lock app" },
        { &DoorStateEventHandler, "door-state-change", "Sends door state change event to lock app" }
    };

    static const shell_command_t sDoorlockEventAlarmSubCommands[] = { { &AlarmHelpHandler, "help",
                                                                        "Usage : doorlock event lock-alarm AlarmCode" } };

    static const shell_command_t sDoorlockEventDoorStateSubCommands[] = {
        { &DoorStateHelpHandler, "help", "Usage : doorlock event door-state-change DoorState" }
    };

    static const shell_command_t sDoorLockCommand = { &DoorlockCommandHandler, "doorlock",
                                                      "doorlock commands. Usage: doorlock <subcommand>" };

    sShellDoorlockEventAlarmSubCommands.RegisterCommands(sDoorlockEventAlarmSubCommands, ArraySize(sDoorlockEventAlarmSubCommands));

    sShellDoorlockEventDoorStateSubCommands.RegisterCommands(sDoorlockEventDoorStateSubCommands,
                                                             ArraySize(sDoorlockEventDoorStateSubCommands));

    sShellDoorlockEventSubCommands.RegisterCommands(sDoorlockEventSubCommands, ArraySize(sDoorlockEventSubCommands));
    sShellDoorlockSubCommands.RegisterCommands(sDoorlockSubCommands, ArraySize(sDoorlockSubCommands));

    Engine::Root().RegisterCommands(&sDoorLockCommand, 1);

    return CHIP_NO_ERROR;
}

void AlarmEventWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "AlarmEventWorkerFunction - Invalid work data"));

    AlarmEventData * data = reinterpret_cast<AlarmEventData *>(context);

    DoorLockServer::Instance().SendLockAlarmEvent(ENDPOINT_1, data->alarmCode);
}

void DoorStateEventWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "DoorStateEventWorkerFunction - Invalid work data"));

    DoorStateEventData * data = reinterpret_cast<DoorStateEventData *>(context);

    DoorLockServer::Instance().SetDoorState(ENDPOINT_1, data->doorState);
}

#endif // ENABLE_CHIP_SHELL
