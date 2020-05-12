/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <support/PersistedCounter.h>
#include <platform/PersistedStorage.h>

#include <stdlib.h>
#include <string.h>

namespace chip {

PersistedCounter::PersistedCounter(void) :
    MonotonicallyIncreasingCounter(), mId(chip::Platform::PersistedStorage::kEmptyKey), mStartingCounterValue(0), mEpoch(0)
{}

PersistedCounter::~PersistedCounter(void)
{
    return;
}

CHIP_ERROR
PersistedCounter::Init(const chip::Platform::PersistedStorage::Key aId, uint32_t aEpoch)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Store the ID.
    mId = aId;

    // Check and store the epoch.
    VerifyOrExit(aEpoch > 0, err = CHIP_ERROR_INVALID_INTEGER_VALUE);
    mEpoch = aEpoch;

    // Read our previously-stored starting value.
    err = ReadStartValue(mStartingCounterValue);
    SuccessOrExit(err);

#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
    ChipLogDetail(EventLogging, "PersistedCounter::Init() aEpoch 0x%x mStartingCounterValue 0x%x", aEpoch, mStartingCounterValue);
#endif

    // Write out the counter value with which we'll start next time we
    // boot up.
    err = WriteStartValue(mStartingCounterValue + mEpoch);
    SuccessOrExit(err);

    // This will set the starting value, after which we're ready.
    err = MonotonicallyIncreasingCounter::Init(mStartingCounterValue);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR
PersistedCounter::Advance(void)
{
    return IncrementCount();
}

CHIP_ERROR
PersistedCounter::AdvanceEpochRelative(uint32_t aValue)
{
    CHIP_ERROR ret;

    mStartingCounterValue = (aValue / mEpoch) * mEpoch;         // Start of enclosing epoch
    mCounterValue         = mStartingCounterValue + mEpoch - 1; // Move to end of enclosing epoch
    ret                   = IncrementCount();                   // Force to next epoch
#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
    ChipLogError(EventLogging, "Advanced counter to 0x%x (relative to 0x%x)", mCounterValue, aValue);
#endif

    return ret;
}

bool PersistedCounter::GetNextValue(uint32_t & aValue)
{
    bool startNewEpoch = false;

    // Increment aValue.
    aValue++;

    // If we've exceeded the value with which we started by aEpoch or
    // more, we need to start a new epoch.
    if ((aValue - mStartingCounterValue) >= mEpoch)
    {
        aValue        = mStartingCounterValue + mEpoch;
        startNewEpoch = true;
    }

    return startNewEpoch;
}

CHIP_ERROR
PersistedCounter::IncrementCount(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Get the incremented value.
    if (GetNextValue(mCounterValue))
    {
        // Started a new epoch, so write out the next one.
        err = WriteStartValue(mCounterValue + mEpoch);
        SuccessOrExit(err);

        mStartingCounterValue = mCounterValue;
    }

exit:
    return err;
}

CHIP_ERROR
PersistedCounter::WriteStartValue(uint32_t aStartValue)
{
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
