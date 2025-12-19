/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <lib/shell/streamer.h>

void InitServerWithDeviceType(std::string deviceType);

namespace chip {
namespace Shell {

Shell::Engine DeviceCommands::sSubShell;

void DeviceCommands::Register()
{
    static const shell_command_t subCommands[] = {
        { &SetDeviceTypeHandler, "set", "Usage: device set <device-type>" },
    };
    sSubShell.RegisterCommands(subCommands, MATTER_ARRAY_SIZE(subCommands));

    // Register the root `device` command in the top-level shell.
    static const shell_command_t deviceCommand = { &DeviceHandler, "device", "Device type management commands" };

    Engine::Root().RegisterCommands(&deviceCommand, 1);
}

CHIP_ERROR DeviceCommands::SetDeviceTypeHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        streamer_printf(streamer_get(), "Usage: device set <device-type>\r\n");
        streamer_printf(streamer_get(), "Example: device set contact-sensor\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const char * deviceType = argv[0];

    streamer_printf(streamer_get(), "Device type set to: %s\r\n", deviceType);

    InitServerWithDeviceType(std::string(deviceType));

    return CHIP_NO_ERROR;
}

} // namespace Shell
} // namespace chip

// C-style API implementation
extern "C" {
const char * GetStoredDeviceType()
{
    const std::string & deviceType = chip::Shell::DeviceCommands::GetInstance().GetStoredDeviceType();
    return deviceType.empty() ? nullptr : deviceType.c_str();
}
}
