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

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>

namespace chip {
namespace Test {

CHIP_ERROR IOContext::Init(nlTestSuite * suite)
{
    CHIP_ERROR err = Platform::MemoryInit();

    gSystemLayer.Init();

    InitNetwork();

    mSuite       = suite;
    mSystemLayer = &gSystemLayer;
    mInetLayer   = &gInet;

    return err;
}

// Shutdown all layers, finalize operations
CHIP_ERROR IOContext::Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ShutdownNetwork();
    gSystemLayer.Shutdown();
    Platform::MemoryShutdown();

    return err;
}

void IOContext::DriveIO()
{
    // Set the select timeout to 100ms
    constexpr uint32_t kSleepTimeMilliseconds = 100;
    ServiceEvents(kSleepTimeMilliseconds);
}

void IOContext::DriveIOUntil(unsigned maxWaitMs, std::function<bool(void)> completionFunction)
{
    uint64_t mStartTime = mSystemLayer->GetClock().GetMonotonicMilliseconds();

    while (true)
    {
        DriveIO(); // at least one IO loop is guaranteed

        if (completionFunction() || ((mSystemLayer->GetClock().GetMonotonicMilliseconds() - mStartTime) >= maxWaitMs))
        {
            break;
        }
    }
}

} // namespace Test
} // namespace chip
