/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
