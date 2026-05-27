/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "AppKeys.h"

#include <app/server/Server.h>
#include <cstring>
#include <devices/device-factory/DeviceFactory.h>
#include <lib/shell/streamer.h>
#include <platform/silabs/SilabsConfig.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

namespace chip {
namespace Shell {

Shell::Engine DeviceCommands::sSubShell;

void DeviceCommands::Register()
{
    static const shell_command_t subCommands[] = {
        { &SetDeviceTypeHandler, "set", "Usage: devtype set <device-type>" },
        { &GetDeviceTypeHandler, "get", "Usage: devtype get" },
        { &ListDeviceTypesHandler, "list", "Usage: devtype list" },
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
        const auto supportedDeviceTypes = chip::app::DeviceFactory::GetInstance().SupportedDeviceTypes();
        streamer_printf(streamer_get(), "Usage: devtype set <device-type>\r\n");
        streamer_printf(streamer_get(), "Supported device types:\r\n");
        for (const auto & deviceType : supportedDeviceTypes)
        {
            streamer_printf(streamer_get(), "  - %s\r\n", deviceType.c_str());
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const char * deviceType = argv[0];

    if (!chip::app::DeviceFactory::GetInstance().IsValidDevice(deviceType))
    {
        streamer_printf(streamer_get(), "Invalid device type: %s\r\n", deviceType);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    PersistentStorageDelegate & storage = Server::GetInstance().GetPersistentStorage();
    CHIP_ERROR err = storage.SyncSetKeyValue(kDeviceTypeKey, deviceType, static_cast<uint16_t>(strlen(deviceType)));
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Failed to save device type to KVS: %" CHIP_ERROR_FORMAT "\r\n", err.Format());
        return err;
    }

    streamer_printf(streamer_get(), "Device type set to: %s. Rebooting %d seconds...\r\n", deviceType, SL_KVS_SAVE_DELAY_SECONDS);

    // Reboot the device after a short delay...
    // Silabs KVS has a  delay to commit, so wait a bit
    return DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Milliseconds32(SL_KVS_SAVE_DELAY_SECONDS * 1000 + 100),
        [](System::Layer *, void *) { chip::DeviceLayer::Silabs::GetPlatform().SoftwareReset(); }, nullptr);
}

CHIP_ERROR DeviceCommands::GetDeviceTypeHandler(int argc, char ** argv)
{
    PersistentStorageDelegate & storage = Server::GetInstance().GetPersistentStorage();
    char storedDeviceType[64]           = {};
    uint16_t storedLen                  = sizeof(storedDeviceType);
    CHIP_ERROR err                      = storage.SyncGetKeyValue(kDeviceTypeKey, storedDeviceType, storedLen);

    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Failed to get stored device type: %" CHIP_ERROR_FORMAT " (using default: %s)\r\n",
                        err.Format(), chip::app::DeviceFactory::GetInstance().GetDefaultDevice().c_str());
        return CHIP_NO_ERROR;
    }

    if (storedLen <= 0 || storedLen > sizeof(storedDeviceType))
    {
        streamer_printf(streamer_get(), "Invalid device type length: %d (using default: %s)\r\n", storedLen,
                        chip::app::DeviceFactory::GetInstance().GetDefaultDevice().c_str());
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    streamer_printf(streamer_get(), "Current device type: %.*s\r\n", static_cast<int>(storedLen), storedDeviceType);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommands::ListDeviceTypesHandler(int argc, char ** argv)
{
    const auto supportedDeviceTypes = chip::app::DeviceFactory::GetInstance().SupportedDeviceTypes();
    streamer_printf(streamer_get(), "Supported device types:\r\n");
    for (const auto & deviceType : supportedDeviceTypes)
    {
        streamer_printf(streamer_get(), "  - %s\r\n", deviceType.c_str());
    }
    return CHIP_NO_ERROR;
}

} // namespace Shell
} // namespace chip
