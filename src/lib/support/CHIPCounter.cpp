/* See Project CHIP LICENSE file for licensing information. */

#include <support/CHIPCounter.h>

namespace chip {

MonotonicallyIncreasingCounter::MonotonicallyIncreasingCounter() : mCounterValue(0) {}

MonotonicallyIncreasingCounter::~MonotonicallyIncreasingCounter() {}

CHIP_ERROR
MonotonicallyIncreasingCounter::Init(uint32_t aStartValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mCounterValue = aStartValue;

    return err;
}

CHIP_ERROR
MonotonicallyIncreasingCounter::Advance()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mCounterValue++;

    return err;
}

uint32_t MonotonicallyIncreasingCounter::GetValue()
{
    return mCounterValue;
}

} // namespace chip
