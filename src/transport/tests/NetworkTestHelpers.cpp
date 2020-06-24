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

#include "NetworkTestHelpers.h"

#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

namespace chip {
namespace Test {

CHIP_ERROR IOContext::Init(nlTestSuite * suite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mSuite = suite;

    // Initialize the CHIP System Layer.
    err = mSystemLayer.Init(NULL);
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = mInetLayer.Init(mSystemLayer, NULL);
    SuccessOrExit(err);

exit:
    return err;
}

// Shutdown all layers, finalize operations
CHIP_ERROR IOContext::Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mSuite         = NULL;

    err = mSystemLayer.Shutdown();
    SuccessOrExit(err);

    err = mInetLayer.Shutdown();
    SuccessOrExit(err);

exit:
    return err;
}

void IOContext::DriveIO()
{
    NL_TEST_ASSERT(mSuite, mSystemLayer.State() == System::kLayerState_Initialized);
    NL_TEST_ASSERT(mSuite, mInetLayer.State == Inet::InetLayer::kState_Initialized);

    // Set the select timeout to 100ms
    struct timeval aSleepTime;
    aSleepTime.tv_sec  = 0;
    aSleepTime.tv_usec = 100 * 1000;

    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    mSystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);
    mInetLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &aSleepTime);
    if (selectRes < 0)
    {
        printf("select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        NL_TEST_ASSERT(mSuite, false);
        return;
    }

    mSystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    mInetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
}

void IOContext::DriveIOUntil(unsigned maxWaitMs, std::function<bool(void)> completionFunction)
{
    uint64_t mStartTime = mSystemLayer.GetClock_MonotonicMS();

    while (true)
    {
        DriveIO(); // at least one IO loop is guaranteed

        if (completionFunction() || ((mSystemLayer.GetClock_MonotonicMS() - mStartTime) >= maxWaitMs))
        {
            break;
        }
    }
}

} // namespace Test
} // namespace chip
