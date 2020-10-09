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
#include "PersistedCounter.h"

#include <platform/PersistedStorage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <stdlib.h>
#include <string.h>

namespace chip {

PersistedCounter::PersistedCounter() : mId(chip::Platform::PersistedStorage::kEmptyKey), mEpoch(0), mNextEpoch(0) {}

PersistedCounter::~PersistedCounter()
{
    return;
}

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
