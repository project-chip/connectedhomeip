/* See Project CHIP LICENSE file for licensing information. */

#include "NetworkTestHelpers.h"

#include <inet/tests/TestInetCommon.h>

#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

namespace chip {
namespace Test {

CHIP_ERROR IOContext::Init(nlTestSuite * suite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    gSystemLayer.Init(nullptr);

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

    return err;
}

void IOContext::DriveIO()
{
    // Set the select timeout to 100ms
    struct timeval aSleepTime;
    aSleepTime.tv_sec  = 0;
    aSleepTime.tv_usec = 100 * 1000;

    ServiceEvents(aSleepTime);
}

void IOContext::DriveIOUntil(unsigned maxWaitMs, std::function<bool(void)> completionFunction)
{
    uint64_t mStartTime = mSystemLayer->GetClock_MonotonicMS();

    while (true)
    {
        DriveIO(); // at least one IO loop is guaranteed

        if (completionFunction() || ((mSystemLayer->GetClock_MonotonicMS() - mStartTime) >= maxWaitMs))
        {
            break;
        }
    }
}

} // namespace Test
} // namespace chip
