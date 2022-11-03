/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "launch.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "heap_trace.h"
#include "sdkconfig.h"

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
#if CONFIG_HEAP_TRACING_STANDALONE || CONFIG_HEAP_TASK_TRACKING
    idf::chip::RegisterHeapTraceCommands();
#endif // CONFIG_HEAP_TRACING_STANDALONE || CONFIG_HEAP_TASK_TRACKING
    xTaskCreate(&MatterShellTask, "chip_cli", 2048, NULL, 5, NULL);
}

} // namespace chip
