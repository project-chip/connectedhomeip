/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "EventHandlerLibShell.h"
#include "AppTask.h"
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"

#include "app/server/Server.h"
#include "platform/CHIPDeviceLayer.h"
#include <lib/support/CodeUtils.h>

constexpr uint8_t kRefEndpointId = 1;

using namespace chip;
using namespace chip::app;
using namespace Clusters::RefrigeratorAlarm;
using namespace Clusters::TemperatureControl;
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellRefrigeratorSubCommands;
Engine sShellRefrigeratorEventSubCommands;
Engine sShellRefrigeratorEventAlarmDoorSubCommands;

/********************************************************
 * Refrigerator shell functions
 *********************************************************/

CHIP_ERROR RefrigeratorHelpHandler(int argc, char ** argv)
{
    sShellRefrigeratorSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR RefrigeratorCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return RefrigeratorHelpHandler(argc, argv);
    }

    return sShellRefrigeratorSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * Event shell functions
 *********************************************************/

CHIP_ERROR EventHelpHandler(int argc, char ** argv)
{
    sShellRefrigeratorEventSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventRefrigeratorCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return EventHelpHandler(argc, argv);
    }

    return sShellRefrigeratorEventSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * Alarm shell functions
 *********************************************************/

CHIP_ERROR AlarmHelpHandler(int argc, char ** argv)
{
    sShellRefrigeratorEventAlarmDoorSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR RefrigeratorDoorEventHandler(int argc, char ** argv)
{

    if (argc == 0)
    {
        return AlarmHelpHandler(argc, argv);
    }
    if (argc >= 2)
    {
        ChipLogError(Shell, "Too many arguments");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if the argument is a valid integer
    if (argv[0] == nullptr || !std::all_of(argv[0], argv[0] + strlen(argv[0]), ::isdigit))
    {
        ChipLogError(Shell, "Invalid argument: Input must be a valid integer.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Convert the argument to an integer
    int value = std::stoi(argv[0]); // Safe to use now, as we validated the input earlier

    RefrigeratorAlarmEventData * data = Platform::New<RefrigeratorAlarmEventData>();
    data->eventId                     = Events::Notify::Id;
    data->doorState                   = static_cast<AlarmBitmap>(value);

    DeviceLayer::PlatformMgr().ScheduleWork(EventWorkerFunction, reinterpret_cast<intptr_t>(data));

    return CHIP_NO_ERROR;
}

/**
 * @brief configures Refrigerator matter shell
 *
 */

CHIP_ERROR RegisterRefrigeratorEvents()
{
    static const shell_command_t sRefrigeratorSubCommands[] = {
        { &RefrigeratorHelpHandler, "help", "Usage: refrigeratoralarm <subcommand>" },
        { &EventRefrigeratorCommandHandler, "event", " Usage: refrigeratoralarm event <subcommand>" }
    };

    static const shell_command_t sRefrigeratorEventSubCommands[] = {
        { &EventHelpHandler, "help", "Usage : refrigeratoralarm event <subcommand>" },
        { &RefrigeratorDoorEventHandler, "door-state-change", "Sends door state change event to Refrigerator app" }
    };

    static const shell_command_t sRefrigeratorEventAlarmDoorSubCommands[] = {
        { &AlarmHelpHandler, "help", "Usage : Refrigerator event to change door state" }
    };

    static const shell_command_t sRefrigeratorCommand = { &RefrigeratorCommandHandler, "refrigeratoralarm",
                                                          "refrigerator alarm commands. Usage: refrigeratoralarm <subcommand>" };

    sShellRefrigeratorEventAlarmDoorSubCommands.RegisterCommands(sRefrigeratorEventAlarmDoorSubCommands,
                                                                 MATTER_ARRAY_SIZE(sRefrigeratorEventAlarmDoorSubCommands));
    sShellRefrigeratorEventSubCommands.RegisterCommands(sRefrigeratorEventSubCommands,
                                                        MATTER_ARRAY_SIZE(sRefrigeratorEventSubCommands));
    sShellRefrigeratorSubCommands.RegisterCommands(sRefrigeratorSubCommands, MATTER_ARRAY_SIZE(sRefrigeratorSubCommands));

    Engine::Root().RegisterCommands(&sRefrigeratorCommand, 1);

    return CHIP_NO_ERROR;
}

void EventWorkerFunction(intptr_t context)
{
    VerifyOrReturn(reinterpret_cast<void *>(context) != nullptr, ChipLogError(Shell, "EventWorkerFunction - Invalid work data"));
    EventData * data = reinterpret_cast<EventData *>(context);

    switch (data->eventId)
    {
    case Events::Notify::Id: {
        RefrigeratorAlarmEventData * alarmData = reinterpret_cast<RefrigeratorAlarmEventData *>(context);
        RefrigeratorAlarmServer::Instance().SetStateValue(kRefEndpointId, alarmData->doorState);
        break;
    }

    default: {
        ChipLogError(Zcl, "Invalid Event Id %s, line %d", __func__, __LINE__);
        break;
    }
    }
    // Free memory
    Platform::Delete(data);
}
