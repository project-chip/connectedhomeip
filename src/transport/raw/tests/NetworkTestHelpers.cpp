/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include "NetworkTestHelpers.h"

#include <inet/tests/TestInetCommon.h>

#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
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
