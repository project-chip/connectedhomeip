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

#include <core/CHIPCore.h>
#include <support/Base64.h>
#include <support/CHIPArgParser.hpp>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::Shell;

void cmd_misc_init();
void cmd_base64_init();
void cmd_device_init();
void cmd_otcli_init();

int main(void)
{
    cmd_misc_init();
    cmd_base64_init();
    cmd_device_init();
    cmd_otcli_init();

    shell_task(NULL);
}
