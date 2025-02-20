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

#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/SubShellCommand.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Shell {

static CHIP_ERROR FactoryResetHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Performing factory reset ... \r\n");
    DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
    return CHIP_NO_ERROR;
}

void RegisterDeviceCommands()
{
    static constexpr Command subCommands[] = {
        { &FactoryResetHandler, "factoryreset", "Performs device factory reset" },
    };

    static constexpr Command deviceComand = { &SubShellCommand<MATTER_ARRAY_SIZE(subCommands), subCommands>, "device",
                                              "Device management commands" };

    Engine::Root().RegisterCommands(&deviceComand, 1);
}

} // namespace Shell
} // namespace chip
