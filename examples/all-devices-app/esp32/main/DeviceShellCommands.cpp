/*
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <DeviceShellCommands.h>
#include <cstring>
#include <device-factory/DeviceFactory.h>
#include <lib/shell/streamer.h>

// Forward declaration of the function defined in main.cpp
void SetDeviceTypeAndRestart(const std::string & deviceType);

namespace chip {
namespace Shell {

Shell::Engine DeviceCommands::sSubShell;

void DeviceCommands::Register()
{
    static const shell_command_t subCommands[] = {
        { &SetDeviceTypeHandler, "set", "Usage: devtype set <device-type>" },
    };
    sSubShell.RegisterCommands(subCommands, MATTER_ARRAY_SIZE(subCommands));

    // Register the root `devtype` command in the top-level shell.
    static const shell_command_t devtypeCommand = { &DeviceHandler, "devtype", "Device type management commands" };

    Engine::Root().RegisterCommands(&devtypeCommand, 1);
}

CHIP_ERROR DeviceCommands::SetDeviceTypeHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        const auto supportedDeviceTypes = chip::app::NoHooksDeviceFactory::GetInstance().SupportedDeviceTypes();
        streamer_printf(streamer_get(), "Usage: devtype set <device-type>\r\n");
        streamer_printf(streamer_get(), "Example: devtype set contact-sensor\r\n");
        streamer_printf(streamer_get(), "Example: devtype set * (all bridged devices)\r\n");
        streamer_printf(streamer_get(), "Supported device types:\r\n");
        streamer_printf(streamer_get(), "  - *\r\n");
        for (const auto & deviceType : supportedDeviceTypes)
        {
            streamer_printf(streamer_get(), "  - %s\r\n", deviceType.c_str());
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const char * deviceType = argv[0];

    if (strcmp(deviceType, "*") != 0 && !chip::app::NoHooksDeviceFactory::GetInstance().IsValidDevice(deviceType))
    {
        streamer_printf(streamer_get(), "Unknown device type: %s\r\n", deviceType);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    streamer_printf(streamer_get(), "Device type set to: %s. Restarting...\r\n", deviceType);

    SetDeviceTypeAndRestart(std::string(deviceType));

    return CHIP_NO_ERROR;
}

} // namespace Shell
} // namespace chip
