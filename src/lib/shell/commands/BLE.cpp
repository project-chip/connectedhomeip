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

#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif
#include <lib/shell/Engine.h>
#include <lib/shell/SubShellCommand.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

using chip::DeviceLayer::ConnectivityMgr;

namespace chip {
namespace Shell {

CHIP_ERROR BLEAdvertiseHandler(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();
    bool adv_enabled;

    VerifyOrReturnError(argc == 1, CHIP_ERROR_INVALID_ARGUMENT);

    adv_enabled = ConnectivityMgr().IsBLEAdvertisingEnabled();
    if (strcmp(argv[0], "start") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "BLE advertising already enabled\r\n");
        }
        else
        {
            streamer_printf(sout, "Starting BLE advertising\r\n");
            return ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        }
    }
    else if (strcmp(argv[0], "stop") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "Stopping BLE advertising\r\n");
            return ConnectivityMgr().SetBLEAdvertisingEnabled(false);
        }

        streamer_printf(sout, "BLE advertising already stopped\r\n");
    }
    else if (strcmp(argv[0], "state") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "BLE advertising is enabled\r\n");
        }
        else
        {
            streamer_printf(sout, "BLE advertising is disabled\r\n");
        }
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void RegisterBLECommands()
{
    static constexpr Command subCommands[] = {
        { &BLEAdvertiseHandler, "adv", "Manage BLE advertising. Usage: ble adv <start|stop|state>" },
    };

    static constexpr Command bleCommand = { &SubShellCommand<MATTER_ARRAY_SIZE(subCommands), subCommands>, "ble",
                                            "Bluetooth LE commands" };

    Engine::Root().RegisterCommands(&bleCommand, 1);
}

} // namespace Shell
} // namespace chip
