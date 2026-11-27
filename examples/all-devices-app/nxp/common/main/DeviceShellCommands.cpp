/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2025-2026 NXP
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

#include "DeviceShellCommands.h"

#if CONFIG_CHIP_LIB_SHELL

#include <device-factory/DeviceFactory.h>
#include <lib/shell/streamer.h>

namespace chip {
namespace Shell {

Shell::Engine DeviceCommands::sSubShell;

namespace {

DeviceCommands::AddRuntimeDeviceCallback gAddRuntimeDevice = nullptr;

} // namespace

void DeviceCommands::Register(AddRuntimeDeviceCallback addDevice)
{
    VerifyOrReturn(addDevice != nullptr);
    static bool registered = false;
    VerifyOrReturn(!registered);

    static const shell_command_t subCommands[] = {
        { &SetDeviceTypeHandler, "set", "Usage: devtype set <type> (append a device immediately)" },
        { &ListDeviceTypesHandler, "list", "Usage: devtype list (show supported device types)" },
    };
    gAddRuntimeDevice = addDevice;
    sSubShell.RegisterCommands(subCommands, MATTER_ARRAY_SIZE(subCommands));

    // Register the root `devtype` command in the top-level shell.
    static const shell_command_t devtypeCommand = { &DeviceHandler, "devtype",
                                                    "list supported types; set <type> appends a device immediately" };
    Engine::Root().RegisterCommands(&devtypeCommand, 1);
    registered = true;
}

CHIP_ERROR DeviceCommands::SetDeviceTypeHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        streamer_printf(streamer_get(), "Usage: devtype set <type>\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    VerifyOrReturnError(gAddRuntimeDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return gAddRuntimeDevice(argv[0]);
}

CHIP_ERROR DeviceCommands::ListDeviceTypesHandler(int argc, char **)
{
    if (argc != 0)
    {
        streamer_printf(streamer_get(), "Usage: devtype list\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    streamer_printf(streamer_get(), "Supported device types (compiled into this firmware):\r\n");
    for (const auto & type : chip::app::NoHooksDeviceFactory::GetInstance().SupportedDeviceTypes())
    {
        streamer_printf(streamer_get(), "  %s\r\n", type.c_str());
    }
    return CHIP_NO_ERROR;
}

} // namespace Shell
} // namespace chip

#endif // CONFIG_CHIP_LIB_SHELL
