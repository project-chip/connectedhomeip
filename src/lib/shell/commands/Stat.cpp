/*
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>
#include <system/SystemStats.h>

using namespace chip;

namespace chip {
namespace Shell {
namespace {

Shell::Engine sSubShell;

CHIP_ERROR StatPeakHandler(int argc, char ** argv)
{
    auto labels     = System::Stats::GetStrings();
    auto watermarks = System::Stats::GetHighWatermarks();

    for (int i = 0; i < System::Stats::kNumEntries; i++)
    {
        streamer_printf(streamer_get(), "%s: %i\r\n", labels[i], static_cast<int>(watermarks[i]));
    }

    if (DeviceLayer::GetDiagnosticDataProvider().SupportsWatermarks())
    {
        uint64_t heapWatermark;
        ReturnErrorOnFailure(DeviceLayer::GetDiagnosticDataProvider().GetCurrentHeapHighWatermark(heapWatermark));
        streamer_printf(streamer_get(), "Heap allocated bytes: %u\r\n", static_cast<unsigned>(heapWatermark));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR StatResetHandler(int argc, char ** argv)
{
    auto current    = System::Stats::GetResourcesInUse();
    auto watermarks = System::Stats::GetHighWatermarks();

    for (int i = 0; i < System::Stats::kNumEntries; i++)
    {
        watermarks[i] = current[i];
    }

    if (DeviceLayer::GetDiagnosticDataProvider().SupportsWatermarks())
    {
        ReturnErrorOnFailure(DeviceLayer::GetDiagnosticDataProvider().ResetWatermarks());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR StatHandler(int argc, char ** argv)
{
    return sSubShell.ExecCommand(argc, argv);
}
} // namespace

void RegisterStatCommands()
{
    // Register subcommands of the `stat` commands.
    static const shell_command_t subCommands[] = {
        { &StatPeakHandler, "peak", "Print peak usage of system resources. Usage: stat peak" },
        { &StatResetHandler, "reset", "Reset peak usage of system resources. Usage: stat reset" },
    };

    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    // Register the root `stat` command in the top-level shell.
    static const shell_command_t statCommand = { &StatHandler, "stat", "Statistics commands" };

    Engine::Root().RegisterCommands(&statCommand, 1);
}

} // namespace Shell
} // namespace chip
