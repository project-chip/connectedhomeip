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

#include "heap_trace.h"

#include <string.h>

#include <core/CHIPError.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/shell/streamer.h>
#include <lib/support/CodeUtils.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_heap_task_info.h"
#include "esp_heap_trace.h"

using chip::Shell::Engine;
using chip::Shell::PrintCommandHelp;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;

namespace {

constexpr size_t kNumHeapTraceRecords = 100;
constexpr size_t kNumHeapTasks        = 20;
constexpr size_t kNumHeapBlocks       = 20;

heap_trace_record_t sTraceRecords[kNumHeapTraceRecords];

Engine sShellHeapSubCommands;

int HeapTraceHelpHandler(int argc, char ** argv)
{
    sShellHeapSubCommands.ForEachCommand(PrintCommandHelp, nullptr);
    return 0;
}

#if CONFIG_HEAP_TRACING_STANDALONE
int HeapTraceResetHandler(int argc, char ** argv)
{
    ESP_ERROR_CHECK(heap_trace_stop());
    ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
    return 0;
}

int HeapTraceDumpHandler(int argc, char ** argv)
{
    heap_trace_dump();
    streamer_printf(streamer_get(), "Free heap %d/%d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT),
                    heap_caps_get_total_size(MALLOC_CAP_8BIT));
    return 0;
}
#endif // CONFIG_HEAP_TRACING_STANDALONE

#if CONFIG_HEAP_TASK_TRACKING
int HeapTraceTaskHandler(int argc, char ** argv)
{
    // static storage is required for task memory info;
    static size_t numTotals = 0;
    static heap_task_totals_t sTotals[kNumHeapTasks];
    static heap_task_block_t sBlocks[kNumHeapBlocks];

    heap_task_info_params_t heap_info;
    memset(&heap_info, 0, sizeof(heap_info));
    heap_info.caps[0]    = MALLOC_CAP_8BIT; // Gets heap with CAP_8BIT capabilities
    heap_info.mask[0]    = MALLOC_CAP_8BIT;
    heap_info.caps[1]    = MALLOC_CAP_32BIT; // Gets heap info with CAP_32BIT capabilities
    heap_info.mask[1]    = MALLOC_CAP_32BIT;
    heap_info.tasks      = NULL; // Passing NULL captures heap info for all tasks
    heap_info.num_tasks  = 0;
    heap_info.totals     = sTotals; // Gets task wise allocation details
    heap_info.num_totals = &numTotals;
    heap_info.max_totals = kNumHeapTasks;  // Maximum length of "sTotals"
    heap_info.blocks     = sBlocks;        // Gets block wise allocation details. For each block, gets owner task, address and size
    heap_info.max_blocks = kNumHeapBlocks; // Maximum length of "sBlocks"

    heap_caps_get_per_task_info(&heap_info);

    for (int i = 0; i < *heap_info.num_totals; i++)
    {
        streamer_printf(streamer_get(), "Task: %s -> CAP_8BIT: %zu CAP_32BIT: %zu\n",
                        heap_info.totals[i].task ? pcTaskGetTaskName(heap_info.totals[i].task) : "Pre-Scheduler allocs",
                        heap_info.totals[i].size[0],  // Heap size with CAP_8BIT capabilities
                        heap_info.totals[i].size[1]); // Heap size with CAP32_BIT capabilities
    }
    streamer_printf(streamer_get(), "Free heap %d/%d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT),
                    heap_caps_get_total_size(MALLOC_CAP_8BIT));

    return 0;
}
#endif

int HeapTraceDispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        HeapTraceHelpHandler(argc, argv);
        return CHIP_NO_ERROR;
    }

    return sShellHeapSubCommands.ExecCommand(argc, argv);
}

} // namespace

namespace idf {
namespace chip {

void RegisterHeapTraceCommands()
{
    static const shell_command_t sHeapSubCommands[] = {
        { &HeapTraceHelpHandler, "help", "Usage: heap-trace <subcommand>" },
#if CONFIG_HEAP_TRACING_STANDALONE
        { &HeapTraceResetHandler, "reset", "Reset the heap trace baseline" },
        { &HeapTraceDumpHandler, "dump", "Dump the last collected heap trace" },
#endif // CONFIG_HEAP_TRACING_STANDALONE
#if CONFIG_HEAP_TASK_TRACKING
        { &HeapTraceTaskHandler, "task", "Dump heap usage of each task" },
#endif // CONFIG_HEAP_TASK_TRACKING
    };
    sShellHeapSubCommands.RegisterCommands(sHeapSubCommands, ArraySize(sHeapSubCommands));

#if CONFIG_HEAP_TRACING_STANDALONE
    ESP_ERROR_CHECK(heap_trace_init_standalone(sTraceRecords, kNumHeapTraceRecords));
    ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
#endif // CONFIG_HEAP_TRACING_STANDALONE

    static const shell_command_t sHeapCommand = { &HeapTraceDispatch, "heap-trace", "Heap debug tracing" };
    Engine::Root().RegisterCommands(&sHeapCommand, 1);
}

} // namespace chip
} // namespace idf
