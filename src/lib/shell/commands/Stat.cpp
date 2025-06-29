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
#include <lib/shell/SubShellCommand.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>
#include <system/SystemStats.h>

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif // CHIP_HAVE_CONFIG_H

#if CHIP_CRYPTO_MBEDTLS
#include <mbedtls/memory_buffer_alloc.h>
#endif

using namespace chip;

namespace chip {
namespace Shell {
namespace {

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

#if CHIP_CRYPTO_MBEDTLS && defined(MBEDTLS_MEMORY_DEBUG)
    size_t maxUsed   = 0;
    size_t maxBlocks = 0;

    mbedtls_memory_buffer_alloc_max_get(&maxUsed, &maxBlocks);

    streamer_printf(streamer_get(), "mbedTLS heap allocated bytes: %u\r\n", static_cast<unsigned>(maxUsed));
#endif

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

#if CHIP_CRYPTO_MBEDTLS && defined(MBEDTLS_MEMORY_DEBUG)
    mbedtls_memory_buffer_alloc_max_reset();
#endif

    return CHIP_NO_ERROR;
}

} // namespace

void RegisterStatCommands()
{
    static constexpr Command subCommands[] = {
        { &StatPeakHandler, "peak", "Print peak usage of system resources" },
        { &StatResetHandler, "reset", "Reset peak usage of system resources" },
    };

    static constexpr Command statCommand = { &SubShellCommand<MATTER_ARRAY_SIZE(subCommands), subCommands>, "stat",
                                             "Statistics commands" };

    Engine::Root().RegisterCommands(&statCommand, 1);
}

} // namespace Shell
} // namespace chip
