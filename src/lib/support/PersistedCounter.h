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

/**
 * @file
 *
 * @brief
 *   Class declarations for a monotonically-increasing counter that is periodically
 *   saved to the provided storage.
 */

#pragma once

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {

/**
 * @class PersistedCounter
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
 * - Assuming epoch is 100 via PersistedCounter::Init(_, 100) and GetValue +
 *   AdvanceValue is called, we get the following outputs:
 *
 *   - Output: 0, 1, 2, 3, 4  <reboot/reinit>
 *   - Output: 100, 101, 102, 103, 104, 105 <reboot/reinit>
 *   - Output: 200, 201, 202, ...., 299, 300, 301, 302 <reboot/reinit>
 *   - Output: 400, 401 ...
 *
 */
template <typename T>
class PersistedCounter : public MonotonicallyIncreasingCounter<T>
{
public:
    PersistedCounter() : mKey(StorageKeyName::Uninitialized()) {}
    ~PersistedCounter() override {}

    /**
     *  @brief
     *    Initialize a PersistedCounter object.
     *
     *  @param[in] aStorage the storage to use for the counter values.
     *  @param[in] aKey the key to use for storing the counter values.
     *  @param[in] aEpoch  On bootup, values we vend will start at a
     *                     multiple of this parameter.
     *
     *  @return CHIP_ERROR_INVALID_ARGUMENT if aStorageDelegate or aKey is NULL
     *          CHIP_ERROR_INVALID_INTEGER_VALUE if aEpoch is 0.
     *          CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR Init(PersistentStorageDelegate * aStorage, StorageKeyName aKey, T aEpoch)
    {
        VerifyOrReturnError(aStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(aKey.IsInitialized(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(aEpoch > 0, CHIP_ERROR_INVALID_INTEGER_VALUE);

        mStorage = aStorage;
        mKey     = aKey;
        mEpoch   = aEpoch;

        T startValue;

        // Read our previously-stored starting value.
        ReturnErrorOnFailure(ReadStartValue(startValue));

#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
        // Compiler should optimize these branches.
        if (is_same_v<decltype(T), uint64_t>)
        {
            ChipLogDetail(EventLogging, "PersistedCounter::Init() aEpoch 0x%" PRIx64 " startValue 0x%" PRIx64, aEpoch, startValue);
        }
        else if (is_same_v<decltype(T), uint32_t>)
        {
            ChipLogDetail(EventLogging, "PersistedCounter::Init() aEpoch 0x%" PRIx32 " startValue 0x%" PRIx32,
                          static_cast<uint32_t>(aEpoch), static_cast<uint32_t>(startValue));
        }
#endif

        ReturnErrorOnFailure(PersistNextEpochStart(startValue + aEpoch));

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
        VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mKey.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        ReturnErrorOnFailure(MonotonicallyIncreasingCounter<T>::Advance());

        if (MonotonicallyIncreasingCounter<T>::GetValue() >= mNextEpoch)
        {
            // Value advanced past the previously persisted "start point".
            // Ensure that a new starting point is persisted.
            ReturnErrorOnFailure(PersistNextEpochStart(mNextEpoch + mEpoch));

            // Advancing the epoch should have ensured that the current value
            // is valid
            VerifyOrReturnError(MonotonicallyIncreasingCounter<T>::GetValue() < mNextEpoch, CHIP_ERROR_INTERNAL);
        }
        return CHIP_NO_ERROR;
    }

private:
    /**
     *  @brief
     *    Write out the counter value to persistent storage.
     *
     *  @param[in] aStartValue  The counter value to write out.
     *
     *  @return Any error returned by a write to persistent storage.
     */
    CHIP_ERROR PersistNextEpochStart(T aStartValue)
    {
        mNextEpoch = aStartValue;
#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
        // Compiler should optimize these branches.
        if (is_same_v<decltype(T), uint64_t>)
        {
            ChipLogDetail(EventLogging, "PersistedCounter::WriteStartValue() aStartValue 0x%" PRIx64, aStartValue);
        }
        else
        {
            ChipLogDetail(EventLogging, "PersistedCounter::WriteStartValue() aStartValue 0x%" PRIx32,
                          static_cast<uint32_t>(aStartValue));
        }
#endif

        T valueLE = Encoding::LittleEndian::HostSwap<T>(aStartValue);
        return mStorage->SyncSetKeyValue(mKey.KeyName(), &valueLE, sizeof(valueLE));
    }

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
        T valueLE     = 0;
        uint16_t size = sizeof(valueLE);

        VerifyOrReturnError(mKey.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        CHIP_ERROR err = mStorage->SyncGetKeyValue(mKey.KeyName(), &valueLE, size);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // No previously-stored value, no worries, the counter is initialized to zero.
            // Suppress the error.
            err = CHIP_NO_ERROR;
        }
        else
        {
            // TODO: Figure out how to avoid a bootloop here.  Maybe we should just
            // init to 0?  Or a random value?
            ReturnErrorOnFailure(err);
        }

        if (size != sizeof(valueLE))
        {
            // TODO: Again, figure out whether this could lead to bootloops.
            return CHIP_ERROR_INCORRECT_STATE;
        }

        aStartValue = Encoding::LittleEndian::HostSwap<T>(valueLE);

#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
        ChipLogDetail(EventLogging, "PersistedCounter::ReadStartValue() aStartValue 0x%x", aStartValue);
#endif

        return CHIP_NO_ERROR;
    }

    PersistentStorageDelegate * mStorage = nullptr; // start value is stored here
    StorageKeyName mKey;
    T mEpoch     = 0; // epoch modulus value
    T mNextEpoch = 0; // next epoch start
};

} // namespace chip
