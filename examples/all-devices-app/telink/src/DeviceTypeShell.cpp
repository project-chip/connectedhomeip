/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AllDevicesServer.h"

#if CONFIG_CHIP_LIB_SHELL

#include <device-factory/DeviceFactory.h>
#include <platform/CHIPDeviceLayer.h>

#include <zephyr/shell/shell.h>

#include <string>

namespace {

std::string gRequestedDeviceType;
bool gStartRequested = false;

void StartAllDevicesServerWork(intptr_t)
{
    CHIP_ERROR err = chip::app::all_devices::StartAllDevicesServer(gRequestedDeviceType);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to start all-devices server: %" CHIP_ERROR_FORMAT, err.Format());
        gStartRequested = false;
    }
}

int cmd_devtype_set(const struct shell * shell, size_t argc, char ** argv)
{
    if (argc != 2)
    {
        shell_print(shell, "Usage: devtype set <device-type>");
        return -EINVAL;
    }

    if (chip::app::all_devices::IsServerStarted())
    {
        shell_print(shell, "Server already started. Factory reset is required to change device type.");
        return -EALREADY;
    }

    if (gStartRequested)
    {
        shell_print(shell, "Server start is already scheduled.");
        return -EALREADY;
    }

    const std::string deviceType(argv[1]);
    auto & factory = chip::app::DeviceFactory::GetInstance();

    if (!factory.IsValidDevice(deviceType))
    {
        shell_print(shell, "Invalid device type: %s", deviceType.c_str());
        shell_print(shell, "Supported device types:");

        for (const auto & item : factory.SupportedDeviceTypes())
        {
            shell_print(shell, "  %s", item.c_str());
        }

        return -EINVAL;
    }

    gRequestedDeviceType = deviceType;
    gStartRequested      = true;

    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(StartAllDevicesServerWork, 0);
    if (err != CHIP_NO_ERROR)
    {
        gStartRequested = false;
        shell_print(shell, "Failed to schedule server start: %" CHIP_ERROR_FORMAT, err.Format());
        return -EIO;
    }

    shell_print(shell, "Starting device type: %s", deviceType.c_str());

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_devtype,
                               SHELL_CMD_ARG(set, NULL, "Set device type: devtype set <device-type>", cmd_devtype_set, 2, 0),
                               SHELL_SUBCMD_SET_END);

} // namespace

SHELL_CMD_REGISTER(devtype, &sub_devtype, "All-devices device type commands", NULL);

#endif // CONFIG_CHIP_LIB_SHELL
