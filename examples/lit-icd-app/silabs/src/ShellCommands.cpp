/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#if defined(ENABLE_CHIP_SHELL)

#include "ShellCommands.h"
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;

namespace LitICDCommands {

using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

/**
 * @brief configures lit icd matter shell
 */
void RegisterSwitchCommands()
{
    // Blank structure for now
    static const shell_command_t sLitICDCommand = {};

    Engine::Root().RegisterCommands(&sLitICDCommand, 1);
}

} // namespace LitICDCommands

#endif // ENABLE_CHIP_SHELL
