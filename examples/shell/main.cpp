/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <shell/shell.h>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::Logging;

int cmd_rand(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "%d\n\r", GetRandU8());
    return 0;
}

int cmd_log(int argc, char ** argv)
{
    int i = 1;
    while (i < argc)
    {
        ChipLogProgress(chipTool, "%s", argv[i++]);
    }
    return 0;
}

static const struct chip_shell_cmd cmds_support[] = {
    { &cmd_rand, "rand", "Random number utilities" },
    { &cmd_log, "log", "Logging utilities" },
};

int main(void)
{
    chip_shell_register(cmds_support, ARRAY_SIZE(cmds_support));
    chip_shell_task(NULL);
}
