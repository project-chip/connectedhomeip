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
 *   saved in the platform's persistent storage.
 */

#ifndef PERSISTED_COUNTER_H
#define PERSISTED_COUNTER_H

#include <platform/PersistedStorage.h>
#include <support/CHIPCounter.h>

namespace chip {

/**
 * @class PersistedCounter
 *
 * @brief
 *   A class for managing a counter as an integer value intended to persist
 *   across reboots.
 */

class PersistedCounter : public MonotonicallyIncreasingCounter
{
public:
    PersistedCounter(void);
    virtual ~PersistedCounter(void);

    /**
     *  @brief
     *    Initialize a PersistedCounter object.
     *
     *  @param[in] aId     The identifier of this PersistedCounter instance.
     *  @param[in] aEpoch  On bootup, values we vend will start at a
     *                     multiple of this parameter.
     *
     *  @return CHIP_ERROR_INVALID_ARGUMENT if aId is NULL
     *          CHIP_ERROR_INVALID_STRING_LENGTH if aId is longer than
     *          CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH.
     *          CHIP_ERROR_INVALID_INTEGER_VALUE if aEpoch is 0.
     *          CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR Init(const chip::Platform::PersistedStorage::Key aId, uint32_t aEpoch);

    /**
     *  @brief
     *  Increment the counter and write to persisted storage if we've completed
     *  the current epoch.
     *
     *  @return Any error returned by a write to persisted storage.
     */
    CHIP_ERROR Advance(void) override;

    /*
     *  @brief
     *    Advance the counter to start of the epoch following the provided
     *    value.
     *
     *  @return Any error returned by a write to persistent storage.
     */
    CHIP_ERROR AdvanceEpochRelative(uint32_t aValue);

private:
    /**
     *  @brief
     *  Get the next value of the counter, based on aValue.
     *
     *  @param[inout] aValue.  The value to be incremented.
     *
     *  @return true if incrementing aValue started a new epoch, false otherwise.
     */
    bool GetNextValue(uint32_t & aValue);

    /**
     *  @brief
     *    Increment the value of the counter by one.  May perform a write to
     *    persistent storage.
     *
     *  @return Any error returned by a write to persistent storage,
     *          CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR IncrementCount(void);

    /**
     *  @brief
     *    Write out the counter value to persistent storage.
     *
     *  @param[in] aStartValue  The counter value to write out.
     *
     *  @return Any error returned by a write to persistent storage.
     */
    CHIP_ERROR WriteStartValue(uint32_t aStartValue);

    /**
     *  @brief
     *    Read our starting counter value (if we have one) in from persistent storage.
     *
     *  @param[inout] aStartValue  The value read out.
     *
     *  @return Any error returned by a read from persistent storage.
     */
    CHIP_ERROR ReadStartValue(uint32_t & aStartValue);

    chip::Platform::PersistedStorage::Key mId;
    uint32_t mStartingCounterValue;
    uint32_t mEpoch;
};

} // namespace chip

#endif // PERSISTED_COUNTER_H
