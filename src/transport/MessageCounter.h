/*
 *    Copyright (c) 2021 Project CHIP Authors
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
 *    @file
 *      This file defines the CHIP message counters.
 *
 */
#pragma once

#include <crypto/RandUtils.h>
#include <lib/core/CHIPError.h>

#include <stdint.h>

namespace chip {

/**
 * MessageCounter represents a local message counter. There are 2 types of message counter
 *
 * 1. Global unencrypted message counter
 * 2. Secure session message counter
 *
 * There will be separate implementations for each type
 */
class MessageCounter
{
public:
    static constexpr uint32_t kMessageCounterRandomInitMask = 0x0FFFFFFF; ///< 28-bit mask

    enum Type : uint8_t
    {
        GlobalUnencrypted,
        GlobalEncrypted,
        Session,
    };

    virtual ~MessageCounter() = default;

    virtual Type GetType() const                           = 0;
    virtual CHIP_ERROR AdvanceAndConsume(uint32_t & fetch) = 0; /** Advance the counter, and feed the new counter to fetch */

    // Note: this function must be called after Crypto is initialized. It can not be called from global variable constructor.
    static uint32_t GetDefaultInitialValuePredecessor() { return Crypto::GetRandU32() & kMessageCounterRandomInitMask; }
};

class GlobalUnencryptedMessageCounter : public MessageCounter
{
public:
    GlobalUnencryptedMessageCounter() : mLastUsedValue(0) {}

    void Init() { mLastUsedValue = GetDefaultInitialValuePredecessor(); }

    Type GetType() const override { return GlobalUnencrypted; }
    CHIP_ERROR AdvanceAndConsume(uint32_t & fetch) override
    {
        fetch = ++mLastUsedValue;
        return CHIP_NO_ERROR;
    }

private:
    uint32_t mLastUsedValue;
};

class LocalSessionMessageCounter : public MessageCounter
{
public:
    static constexpr uint32_t kMessageCounterMax = 0xFFFFFFFF;

    /**
     * Initialize a local message counter with random value between [1, 2^28]. This increases the difficulty of traffic analysis
     * attacks by making it harder to determine how long a particular session has been open. The initial counter is always 1 or
     * higher to guarantee first message is always greater than initial peer counter set to 0.
     *
     * The mLastUsedValue is the predecessor of the initial value, it will be advanced before using, so don't need to add 1 here.
     */
    LocalSessionMessageCounter() { mLastUsedValue = GetDefaultInitialValuePredecessor(); }

    Type GetType() const override { return Session; }
    CHIP_ERROR AdvanceAndConsume(uint32_t & fetch) override
    {
        if (mLastUsedValue == kMessageCounterMax)
        {
            return CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED;
        }

        fetch = ++mLastUsedValue;
        return CHIP_NO_ERROR;
    }

    // Test-only function to set the counter value
    void TestSetCounter(uint32_t value) { mLastUsedValue = value; }

private:
    uint32_t mLastUsedValue;
};

} // namespace chip
