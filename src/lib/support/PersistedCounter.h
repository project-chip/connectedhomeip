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
#include <limits>

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
        if constexpr (std::is_same_v<decltype(startValue), uint64_t>)
        {
            ChipLogDetail(EventLogging, "PersistedCounter::Init() aEpoch 0x" ChipLogFormatX64 " startValue 0x" ChipLogFormatX64,
                          ChipLogValueX64(aEpoch), ChipLogValueX64(startValue));
        }
        else if (std::is_same_v<decltype(startValue), uint32_t>)
        {
            ChipLogDetail(EventLogging, "PersistedCounter::Init() aEpoch 0x%" PRIx32 " startValue 0x%" PRIx32,
                          static_cast<uint32_t>(aEpoch), static_cast<uint32_t>(startValue));
        }
#endif

        ReturnErrorOnFailure(PersistNextEpochStart(static_cast<T>(startValue + aEpoch)));

        // This will set the starting value, after which we're ready.
        return MonotonicallyIncreasingCounter<T>::Init(startValue);
    }

    /**
     *  @brief Increment the counter by N and write to persisted storage if we've completed the current epoch.
     *
     *  @param value value of N
     *
     *  @return Any error returned by a write to persisted storage.
     */
    CHIP_ERROR AdvanceBy(T value) override
    {
        VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mKey.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        // If value is 0, we do not need to do anything
        VerifyOrReturnError(value > 0, CHIP_NO_ERROR);

        // We should update the persisted epoch value if :
        // 1- Sum of the current counter and value is greater or equal to the mNextEpoch.
        //    This is the standard operating case.
        // 2- Increasing the current counter by value would cause a roll over. This would cause the current value to be < to the
        //    mNextEpoch so we force an update.
        bool shouldDoEpochUpdate = ((MonotonicallyIncreasingCounter<T>::GetValue() + value) >= mNextEpoch) ||
            (MonotonicallyIncreasingCounter<T>::GetValue() > std::numeric_limits<T>::max() - value);

        ReturnErrorOnFailure(MonotonicallyIncreasingCounter<T>::AdvanceBy(value));

        if (shouldDoEpochUpdate)
        {
            // Since AdvanceBy allows the counter to be increased by an arbitrary value, it is possible that the new counter value
            // is greater than mNextEpoch + mEpoch. As such, we want the next Epoch value to be calculated from the new current
            // value.
            PersistAndVerifyNextEpochStart(MonotonicallyIncreasingCounter<T>::GetValue());
        }

        return CHIP_NO_ERROR;
    }

    /**
     *  @brief Increment the counter and write to persisted storage if we've completed the current epoch.
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
            ReturnErrorOnFailure(PersistAndVerifyNextEpochStart(mNextEpoch));
        }

        return CHIP_NO_ERROR;
    }

private:
    CHIP_ERROR PersistAndVerifyNextEpochStart(T refEpoch)
    {
        // Value advanced past the previously persisted "start point".
        // Ensure that a new starting point is persisted.
        ReturnErrorOnFailure(PersistNextEpochStart(static_cast<T>(refEpoch + mEpoch)));

        // Advancing the epoch should have ensured that the current value is valid
        VerifyOrReturnError(static_cast<T>(MonotonicallyIncreasingCounter<T>::GetValue() + mEpoch) == mNextEpoch,
                            CHIP_ERROR_INTERNAL);

        // Previous check did not take into consideration that the counter value can be equal to the max counter value or
        // rollover.
        // TODO(#33175): PersistedCounter allows rollover so this check is incorrect. We need a Counter class that adequatly
        // manages rollover behavior for counters that cannot rollover.
        // VerifyOrReturnError(MonotonicallyIncreasingCounter<T>::GetValue() < mNextEpoch, CHIP_ERROR_INTERNAL);

        return CHIP_NO_ERROR;
    }

    /**
     *  @brief
     *    Write out the counter value to persistent storage.
     *
     *  @param[in] aStartValue  The counter value to write out.
     *
     *  @return Any error returned by a write to persistent storage.
     */
    CHIP_ERROR
    PersistNextEpochStart(T aStartValue)
    {
        mNextEpoch = aStartValue;
#if CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
        if constexpr (std::is_same_v<decltype(aStartValue), uint64_t>)
        {
            ChipLogDetail(EventLogging, "PersistedCounter::WriteStartValue() aStartValue 0x" ChipLogFormatX64,
                          ChipLogValueX64(aStartValue));
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
        T valueLE     = GetInitialCounterValue();
        uint16_t size = sizeof(valueLE);

        // clang-tidy claims that we're returning without writing to 'aStartValue',
        // assign 0 to supppress the warning. In case of error, the value wont't be
        // used anyway.
        aStartValue = 0;

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
        if constexpr (std::is_same_v<decltype(aStartValue), uint64_t>)
        {
            ChipLogDetail(EventLogging, "PersistedCounter::ReadStartValue() aStartValue 0x" ChipLogFormatX64,
                          ChipLogValueX64(aStartValue));
        }
        else
        {
            ChipLogDetail(EventLogging, "PersistedCounter::ReadStartValue() aStartValue 0x%" PRIx32,
                          static_cast<uint32_t>(aStartValue));
        }
#endif

        return CHIP_NO_ERROR;
    }

    /**
     * @brief Get the Initial Counter Value
     *
     * By default, persisted counters start off at 0.
     */
    virtual inline T GetInitialCounterValue() { return 0; }

    PersistentStorageDelegate * mStorage = nullptr; // start value is stored here
    StorageKeyName mKey;
    T mEpoch     = 0; // epoch modulus value
    T mNextEpoch = 0; // next epoch start
};

} // namespace chip
