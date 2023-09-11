/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "NetworkTestHelpers.h"

#include <inet/tests/TestInetCommon.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Test {

CHIP_ERROR IOContext::Init()
{
    CHIP_ERROR err = Platform::MemoryInit();
    chip::DeviceLayer::SetConfigurationMgr(&chip::DeviceLayer::ConfigurationMgrImpl());

    InitSystemLayer();
    InitNetwork();

    mSystemLayer        = &gSystemLayer;
    mTCPEndPointManager = &gTCP;
    mUDPEndPointManager = &gUDP;

    return err;
}

// Shutdown all layers, finalize operations
void IOContext::Shutdown()
{
    ShutdownNetwork();
    ShutdownSystemLayer();
    Platform::MemoryShutdown();
}

void IOContext::DriveIO()
{
    // Set the select timeout to 10ms
    constexpr uint32_t kSleepTimeMilliseconds = 10;
    ServiceEvents(kSleepTimeMilliseconds);
}

void IOContext::DriveIOUntil(System::Clock::Timeout maxWait, std::function<bool(void)> completionFunction)
{
    System::Clock::Timestamp startTime = System::SystemClock().GetMonotonicTimestamp();

    while (true)
    {
        DriveIO(); // at least one IO loop is guaranteed

        if (completionFunction() || ((System::SystemClock().GetMonotonicTimestamp() - startTime) >= maxWait))
        {
            break;
        }
    }
}

} // namespace Test
} // namespace chip
