/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @file
 *
 * @brief
 *   Class declarations for a monotonically-increasing counter that is periodically
 *   saved in the platform's persistent storage.
 */

#pragma once

#include <lib/support/CHIPCounter.h>
#include <lib/support/CodeUtils.h>
#include <platform/PersistedStorage.h>

namespace chip {

/**
 * @class LifetimePersistedCounter
 *
 * @brief
 *   A class for managing a counter as an integer value intended to persist
 *   across reboots.
 *
 * Counter values are always set to start at a multiple of a bootup value
 * "epoch".
 *
 * Example:
 *
 * - Assuming epoch is 100 via LifetimePersistedCounter::Init(_, 100) and GetValue +
 *   AdvanceValue is called, we get the following outputs:
 *
 *   - Output: 0, 1, 2, 3, 4
 *
 */

template <typename T>
class LifetimePersistedCounter : public MonotonicallyIncreasingCounter<T>
{
public:
    LifetimePersistedCounter() : mId(chip::Platform::PersistedStorage::kEmptyKey) {}
    ~LifetimePersistedCounter() override = default;

    /**
     *  @brief
     *    Initialize a LifetimePersistedCounter object.
     *
     *  @param[in] aId     The identifier of this LifetimePersistedCounter instance.
     *
     *  @return CHIP_ERROR_INVALID_ARGUMENT if aId is NULL
     *          CHIP_ERROR_INVALID_STRING_LENGTH if aId is longer than
     *          CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH.
     *          CHIP_ERROR_INVALID_INTEGER_VALUE if aEpoch is 0.
     *          CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR Init(const chip::Platform::PersistedStorage::Key aId)
    {
        mId = aId;
        T startValue;

        // Read our previously-stored starting value.
        ReturnErrorOnFailure(ReadStartValue(startValue));

        // This will set the starting value, after which we're ready.
        return MonotonicallyIncreasingCounter<T>::Init(startValue);
    }

    /**
     *  @brief
     *  Increment the counter and write to persisted storage if we've completed
     *  the current epoch.
     *
     *  @return Any error returned by a write to persisted storage.
     */
    CHIP_ERROR Advance() override
    {
        VerifyOrReturnError(mId != chip::Platform::PersistedStorage::kEmptyKey, CHIP_ERROR_INCORRECT_STATE);

        ReturnErrorOnFailure(MonotonicallyIncreasingCounter<T>::Advance());

        return chip::Platform::PersistedStorage::Write(mId, MonotonicallyIncreasingCounter<T>::GetValue());
    }

private:
    /**
     *  @brief
     *    Read our starting counter value (if we have one) in from persistent storage.
     *
     *  @param[in,out] aStartValue  The value read out.
     *
     *  @return Any error returned by a read from persistent storage.
     */
    CHIP_ERROR ReadStartValue(T & aStartValue)
    {
        aStartValue = 0;

        CHIP_ERROR err = chip::Platform::PersistedStorage::Read(mId, aStartValue);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // No previously-stored value, no worries, the counter is initialized to zero.
            // Suppress the error.
            err = CHIP_NO_ERROR;
        }
        return err;
    }

    chip::Platform::PersistedStorage::Key mId; // start value is stored here
};

} // namespace chip
