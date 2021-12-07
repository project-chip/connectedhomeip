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
#include <lib/support/PersistedCounter.h>

namespace chip {

/**
 * MessageCounter represents a local message counter. There are 3 types
 * of message counter
 *
 * 1. Global unencrypted message counter
 * 2. Global encrypted message counter
 * 3. Session message counter
 *
 * There will be separate implementations for each type
 */
class MessageCounter
{
public:
    enum Type : uint8_t
    {
        GlobalUnencrypted,
        GlobalEncrypted,
        Session,
    };

    virtual ~MessageCounter() = default;

    virtual Type GetType()       = 0;
    virtual uint32_t Value()     = 0; /** Get current value */
    virtual CHIP_ERROR Advance() = 0; /** Advance the counter */
};

class GlobalUnencryptedMessageCounter : public MessageCounter
{
public:
    GlobalUnencryptedMessageCounter() : value(0) {}

    void Init();

    Type GetType() override { return GlobalUnencrypted; }
    uint32_t Value() override { return value; }
    CHIP_ERROR Advance() override
    {
        ++value;
        return CHIP_NO_ERROR;
    }

private:
    uint32_t value;
};

class GlobalEncryptedMessageCounter : public MessageCounter
{
public:
    GlobalEncryptedMessageCounter() {}

    CHIP_ERROR Init();
    Type GetType() override { return GlobalEncrypted; }
    uint32_t Value() override { return persisted.GetValue(); }
    CHIP_ERROR Advance() override { return persisted.Advance(); }

private:
#if CONFIG_DEVICE_LAYER
    PersistedCounter persisted;
#else
    struct FakePersistedCounter
    {
        FakePersistedCounter() : value(0) {}
        CHIP_ERROR Init(chip::Platform::PersistedStorage::Key aId, uint32_t aEpoch) { return CHIP_NO_ERROR; }

        uint32_t GetValue() { return value; }
        CHIP_ERROR Advance()
        {
            ++value;
            return CHIP_NO_ERROR;
        }

    private:
        uint32_t value;
    } persisted;
#endif
};

class LocalSessionMessageCounter : public MessageCounter
{
public:
    static constexpr uint32_t kInitialValue                 = 1;         ///< Used for initializing peer counter
    static constexpr uint32_t kMessageCounterRandomInitMask = 0x0FFFFFF; ///< 28-bit mask

    /**
     * Initialize a local message counter with random value between [0, 2^28-1]. This increases the difficulty of traffic analysis
     * attacks by making it harder to determine how long a particular session has been open.
     */
    LocalSessionMessageCounter() { value = Crypto::GetRandU32() & kMessageCounterRandomInitMask; }

    Type GetType() override { return Session; }
    uint32_t Value() override { return value; }
    CHIP_ERROR Advance() override
    {
        ++value;
        return CHIP_NO_ERROR;
    }

private:
    uint32_t value;
};

} // namespace chip
