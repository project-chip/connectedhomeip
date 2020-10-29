/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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

#include <support/PersistedCounter.h>

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

    virtual ~MessageCounter() = 0;

    virtual Type GetType()     = 0;
    virtual void Reset()       = 0;
    virtual uint32_t Value()   = 0; /** Get current value */
    virtual uint32_t Advance() = 0; /** Advance the counter and get new value */
};

inline MessageCounter::~MessageCounter() {}

class GlobalUnencryptedMessageCounter : public MessageCounter
{
public:
    GlobalUnencryptedMessageCounter();
    ~GlobalUnencryptedMessageCounter() override {}

    Type GetType() override { return GlobalUnencrypted; }
    void Reset() override
    { /* null op */
    }
    uint32_t Value() override { return value; }
    uint32_t Advance() override { return ++value; }

private:
    uint32_t value;
};

class GlobalEncryptedMessageCounter : public MessageCounter
{
public:
    GlobalEncryptedMessageCounter() {}
    ~GlobalEncryptedMessageCounter() override {}

    CHIP_ERROR Init();
    Type GetType() override { return GlobalEncrypted; }
    void Reset() override
    { /* null op */
    }
    uint32_t Value() override { return persisted.GetValue(); }
    uint32_t Advance() override { return static_cast<uint32_t>(persisted.Advance()); }

private:
#if CONFIG_DEVICE_LAYER
    PersistedCounter persisted;
#else
    struct FakePersistedCounter {
        FakePersistedCounter() : value(0) {}
        CHIP_ERROR Init(chip::Platform::PersistedStorage::Key aId, uint32_t aEpoch) { return CHIP_NO_ERROR; }

        uint32_t GetValue() { return value; }
        uint32_t Advance() { return ++value; }
    private:
        uint32_t value;
    } persisted;
#endif
};

class LocalSessionMessageCounter : public MessageCounter
{
public:
    LocalSessionMessageCounter() : value(1) {}
    ~LocalSessionMessageCounter() override {}

    Type GetType() override { return Session; }
    void Reset() override { value = 0; }
    uint32_t Value() override { return value; }
    uint32_t Advance() override { return ++value; }

private:
    uint32_t value;
};

} // namespace chip
