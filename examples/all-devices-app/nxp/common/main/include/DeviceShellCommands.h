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

#pragma once

#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/shell/streamer.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Shell {

class DeviceCommands
{
public:
    DeviceCommands(const DeviceCommands &)             = delete;
    DeviceCommands(DeviceCommands &&)                  = delete;
    DeviceCommands & operator=(const DeviceCommands &) = delete;

    static DeviceCommands & GetInstance()
    {
        static DeviceCommands instance;
        return instance;
    }

    /// The callback runs on the Matter task. The type buffer is valid only for the call.
    using AddRuntimeDeviceCallback = CHIP_ERROR (*)(const char * type);

    /// Register once on the Matter task after server initialization.
    void Register(AddRuntimeDeviceCallback addDevice);

private:
    DeviceCommands() = default;

    static CHIP_ERROR DeviceHandler(int argc, char ** argv)
    {
        if (argc == 0)
        {
            sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
            return CHIP_NO_ERROR;
        }

        return sSubShell.ExecCommand(argc, argv);
    }

    static CHIP_ERROR SetDeviceTypeHandler(int argc, char ** argv);
    static CHIP_ERROR ListDeviceTypesHandler(int argc, char ** argv);

    static Shell::Engine sSubShell;
};

} // namespace Shell
} // namespace chip
