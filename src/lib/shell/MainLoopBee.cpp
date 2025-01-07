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

#include "streamer.h"
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#include <ctype.h>
#include <string.h>

#include <openthread/error.h>
#include <openthread/cli.h>
#include "chip_porting.h"

using chip::FormatCHIPError;
using chip::Platform::MemoryAlloc;
using chip::Platform::MemoryFree;
using chip::Shell::Engine;
using chip::Shell::streamer_get;

typedef otError (*cmd)(void *, uint8_t, char **);

otError matter_cmd_handler(void *aContext, uint8_t argc, char *argv[])
{
    Engine::Root().ExecCommand(argc, argv);
    return OT_ERROR_NONE;
}

otCliCommand bee_cmd[] = {
    {"kv",(cmd)matter_kvs_cmd_handle},
    {"matter",matter_cmd_handler},
};

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    otCliSetUserCommands(bee_cmd, sizeof(bee_cmd)/sizeof(bee_cmd[0]), NULL);
}

} // namespace Shell
} // namespace chip
