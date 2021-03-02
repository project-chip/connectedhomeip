/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/shell/shell.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <inttypes.h>
#include <stdarg.h>

#include "ChipShellMbedCollection.h"

#include <inet/InetError.h>
#include <inet/InetLayer.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::System;
using namespace chip::Inet;

System::Layer gSystemLayer;
Inet::InetLayer gInet;

static chip::Shell::Shell sShellDateSubcommands;
static chip::Shell::Shell sShellNetworkSubcommands;
static chip::Shell::Shell sShellSocketSubcommands;

int cmd_date_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_date_help(int argc, char ** argv)
{
    sShellDateSubcommands.ForEachCommand(cmd_date_help_iterator, nullptr);
    return 0;
}

int cmd_date_dispatch(int argc, char ** argv)
{
    uint16_t year;
    uint8_t month, dayOfMonth;
    uint8_t hour, minute, second;

    uint64_t currTimeMS = 0;

    streamer_t * sout = streamer_get();

    if (0 == argc)
    {
        if (Layer::GetClock_RealTimeMS(currTimeMS) != CHIP_SYSTEM_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: Date/Time was not set\r\n");
            return CHIP_ERROR_ACCESS_DENIED;
        }

        SecondsSinceEpochToCalendarTime(currTimeMS / 1000, year, month, dayOfMonth, hour, minute, second);
        streamer_printf(sout, "%04" PRIu16 "-%02" PRIu8 "-%02" PRIu8 " %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8 "\n\r", year, month,
                        dayOfMonth, hour, minute, second);
        return CHIP_NO_ERROR;
    }

    return sShellDateSubcommands.ExecCommand(argc, argv);
}

int cmd_date_set(int argc, char ** argv)
{
    uint16_t year;
    uint8_t month, dayOfMonth;
    uint8_t hour, minute, seconds;

    uint32_t newCurrTime = 0;
    streamer_t * sout    = streamer_get();

    int ret          = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc == 2, error = CHIP_ERROR_INVALID_ARGUMENT);

    ret = sscanf(argv[0], "%4" SCNu16 "-%2" SCNu8 "-%2" SCNu8, &year, &month, &dayOfMonth);
    if (3 != ret)
    {
        streamer_printf(sout, "ERROR: Date is in wrong format! Please use 'YYYY-MM-DD HH:MM:SS' format.\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    ret = sscanf(argv[1], "%2" SCNu8 ":%2" SCNu8 ":%2" SCNu8, &hour, &minute, &seconds);
    if (3 != ret)
    {
        streamer_printf(sout, "ERROR: Time is in wrong format! Please use 'YYYY-MM-DD HH:MM:SS' format.\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    if (!CalendarTimeToSecondsSinceEpoch(year, month, dayOfMonth, hour, minute, seconds, newCurrTime))
    {
        streamer_printf(sout, "ERROR: Wrond date and/or time values\r\n");
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT;);
    }

    error = Layer::SetClock_RealTime(static_cast<uint64_t>(newCurrTime) * UINT64_C(1000000));

exit:
    return error;
}

int cmd_device_test_config(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    chip::DeviceLayer::ConfigurationMgrImpl().RunConfigUnitTest();
exit:
    return error;
}

/* Network commands */
int cmd_network_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = sShellNetworkSubcommands.ExecCommand(argc, argv);

exit:
    return error;
}

int cmd_network_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_network_help(int argc, char ** argv)
{
    sShellNetworkSubcommands.ForEachCommand(cmd_network_help_iterator, nullptr);
    return 0;
}

int cmd_network_interface(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    InterfaceIterator intIterator;
    InterfaceAddressIterator addrIterator;
    char intName[IF_NAMESIZE];
    InterfaceId intId;
    IPAddress addr;
    INET_ERROR err;

    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(sout, "    Current interface:\n");
    for (; intIterator.HasCurrent(); intIterator.Next())
    {
        intId = intIterator.GetInterface();
        if (intId == INET_NULL_INTERFACEID)
        {
            streamer_printf(sout, "ERROR: get interface failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }

        err = intIterator.GetInterfaceName(intName, sizeof(intName));
        if (err != INET_NO_ERROR)
        {
            streamer_printf(sout, "ERROR: get interface name failed\r\n");
            ExitNow(error = CHIP_ERROR_INTERNAL;);
        }
        printf("     interface id: %d, interface name: %s\n", intId, intName);

        //         printf("     interface id: 0x%" PRIxPTR ", interface name: %s, interface state: %s, %s multicast, %s broadcast
        //         addr\n",
        // #if CHIP_SYSTEM_CONFIG_USE_LWIP
        //                reinterpret_cast<uintptr_t>(intId),
        // #else
        //                static_cast<uintptr_t>(intId),
        // #endif
        //                intName, intIterator.IsUp() ? "UP" : "DOWN", intIterator.SupportsMulticast() ? "supports" : "no",
        //                intIterator.HasBroadcastAddress() ? "has" : "no");
    }

exit:
    return error;
}

/* Socket commands */
int cmd_socket_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = sShellSocketSubcommands.ExecCommand(argc, argv);

exit:
    return error;
}

int cmd_socket_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_socket_help(int argc, char ** argv)
{
    sShellSocketSubcommands.ForEachCommand(cmd_socket_help_iterator, nullptr);
    return 0;
}

static const shell_command_t cmds_date_root = { &cmd_date_dispatch, "date", "Display the current time, or set the system date." };

static const shell_command_t cmds_date[] = { { &cmd_date_set, "set", "Set date/time using 'YYYY-MM-DD HH:MM:SS' format" },
                                             { &cmd_date_help, "help", "Display help for each subcommand" } };

static const shell_command_t cmds_test_config = { &cmd_device_test_config, "testconfig",
                                                  "Test the configuration implementation. Usage: device testconfig" };

static const shell_command_t cmds_network_root = { &cmd_network_dispatch, "network", "Network interface layer commands" };

static const shell_command_t cmds_network[] = { { &cmd_network_interface, "interface",
                                                  "Display current network interface details" },
                                                { &cmd_network_help, "help", "Display help for each network subcommands" } };

static const shell_command_t cmds_socket_root = { &cmd_socket_dispatch, "socket", "Socket layer commands" };

static const shell_command_t cmds_socket[] = { { &cmd_socket_help, "help", "Display help for each socket subcommands" } };

void cmd_mbed_utils_init()
{
    gSystemLayer.Init(nullptr);
    gInet.Init(gSystemLayer, nullptr);

    sShellDateSubcommands.RegisterCommands(cmds_date, ArraySize(cmds_date));
    sShellNetworkSubcommands.RegisterCommands(cmds_network, ArraySize(cmds_network));
    sShellSocketSubcommands.RegisterCommands(cmds_socket, ArraySize(cmds_socket));
    shell_register(&cmds_date_root, 1);
    shell_register(&cmds_test_config, 1);
    shell_register(&cmds_network_root, 1);
    shell_register(&cmds_socket_root, 1);
}
