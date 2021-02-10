/* See Project CHIP LICENSE file for licensing information. */

#include "PersistedCounter.h"

#include <platform/PersistedStorage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <stdlib.h>
#include <string.h>

namespace chip {

PersistedCounter::PersistedCounter() : mId(chip::Platform::PersistedStorage::kEmptyKey), mEpoch(0), mNextEpoch(0) {}

PersistedCounter::~PersistedCounter() {}

CHIP_ERROR
PersistedCounter::Init(const chip::Platform::PersistedStorage::Key aId, uint32_t aEpoch)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(aEpoch > 0, err = CHIP_ERROR_INVALID_INTEGER_VALUE);

    // Store the ID.
    mId    = aId;
    mEpoch = aEpoch;

    uint32_t startValue;

    // Read our previously-stored starting value.
    err = ReadStartValue(startValue);
    SuccessOrExit(err);

#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
    ChipLogDetail(EventLogging, "PersistedCounter::Init() aEpoch 0x%x startValue 0x%x", aEpoch, startValue);
#endif

    err = PersistNextEpochStart(startValue + aEpoch);
    SuccessOrExit(err);

    // This will set the starting value, after which we're ready.
    err = MonotonicallyIncreasingCounter::Init(startValue);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR
PersistedCounter::Advance()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mId != chip::Platform::PersistedStorage::kEmptyKey, err = CHIP_ERROR_INCORRECT_STATE);

    err = MonotonicallyIncreasingCounter::Advance();
    SuccessOrExit(err);

    if (GetValue() >= mNextEpoch)
    {
        // Value advanced past the previously persisted "start point".
        // Ensure that a new starting point is persisted.
        err = PersistNextEpochStart(mNextEpoch + mEpoch);
        SuccessOrExit(err);

        // Advancing the epoch should have ensured that the current value
        // is valid
        VerifyOrExit(GetValue() < mNextEpoch, err = CHIP_ERROR_INTERNAL);
    }
exit:
    return err;
}

CHIP_ERROR
PersistedCounter::PersistNextEpochStart(uint32_t aStartValue)
{
    mNextEpoch = aStartValue;
#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
    ChipLogDetail(EventLogging, "PersistedCounter::WriteStartValue() aStartValue 0x%x", aStartValue);
#endif

    return chip::Platform::PersistedStorage::Write(mId, aStartValue);
}

CHIP_ERROR
PersistedCounter::ReadStartValue(uint32_t & aStartValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    aStartValue = 0;

    err = chip::Platform::PersistedStorage::Read(mId, aStartValue);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // No previously-stored value, no worries, the counter is initialized to zero.
        // Suppress the error.
        err = CHIP_NO_ERROR;
    }
    else
    {
        SuccessOrExit(err);
    }

#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
    ChipLogDetail(EventLogging, "PersistedCounter::ReadStartValue() aStartValue 0x%x", aStartValue);
#endif

exit:
    return err;
}

} // namespace chip
