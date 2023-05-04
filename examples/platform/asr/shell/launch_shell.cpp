/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "launch_shell.h"

#include "lega_rtos_api.h"

#include <lib/shell/Engine.h>

namespace {

void MatterShellTask(void * args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}

} // namespace

namespace chip {

void LaunchShell()
{
    chip::Shell::Engine::Root().Init();

    lega_rtos_create_thread(NULL, 1, "matter_shell", (lega_thread_function_t) MatterShellTask, 2048, 0);
}

} // namespace chip
