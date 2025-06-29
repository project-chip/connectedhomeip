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
#include <system/SystemClock.h>
#include <crypto/RandUtils.h>

#include "FreeRTOS.h"
#include "task.h"
#include "si_chrono.h"
#include "nvm3_default.h"
#include "nvm3_default_config.h"

using chip::DeviceLayer::ConnectivityMgr;


namespace chip {
namespace Shell {

CHIP_ERROR DebugRunHandler(int argc, char ** argv)
{
    ChipLogProgress(Crypto, "~~~ Debug!");
    return CHIP_NO_ERROR;
}

void RegisterDebugCommands()
{
    static constexpr Command subCommands[] = {
        { &DebugRunHandler, "run", "Debug Run" },
    };

    static constexpr Command debugCommand = { &SubShellCommand<ArraySize(subCommands), subCommands>, "debug", "Debug commands" };

    Engine::Root().RegisterCommands(&debugCommand, 1);
}

} // namespace Shell
} // namespace chip
