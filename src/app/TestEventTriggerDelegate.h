/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/Span.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {

class TestEventTriggerHandler : public IntrusiveListNodeBase<IntrusiveMode::AutoUnlink>
{
public:
    virtual ~TestEventTriggerHandler() = default;
    /**
     * Handles the test event trigger based on `eventTrigger` provided.
     *
     * @param[in] eventTrigger Event trigger to handle.
     *
     * @return CHIP_NO_ERROR on success or another CHIP_ERROR on failure
     */
    virtual CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) = 0;
};

class TestEventTriggerDelegate
{
public:
    /* Expected byte size of the EnableKey */
    static constexpr size_t kEnableKeyLength = 16;

    virtual ~TestEventTriggerDelegate() {}
    /**
     * Checks to see if `enableKey` provided matches value chosen by the manufacturer.
     *
     * @param[in] enableKey Buffer of the key to verify.
     */
    virtual bool DoesEnableKeyMatch(const ByteSpan & enableKey) const = 0;

    /**
     * Handles the test event trigger based on `eventTrigger` provided.
     *
     * By default, this iterates over handlers added via `AddEventTriggerHandler`.
     *
     * If more specific behavior is desired, the method can be completely overridden.
     *
     * @param[in] eventTrigger Event trigger to handle.
     *
     * @return CHIP_NO_ERROR on success or another CHIP_ERROR on failure
     */
    virtual CHIP_ERROR HandleEventTriggers(uint64_t eventTrigger)
    {
        CHIP_ERROR last_error = CHIP_ERROR_INVALID_ARGUMENT;
        for (TestEventTriggerHandler & handler : mHandlerList)
        {
            last_error = handler.HandleEventTrigger(eventTrigger);
            if (last_error == CHIP_NO_ERROR)
            {
                break;
            }
        }

        return last_error;
    }

    CHIP_ERROR AddHandler(TestEventTriggerHandler * handler)
    {
        VerifyOrReturnError(!mHandlerList.Contains(handler), CHIP_ERROR_INVALID_ARGUMENT);
        mHandlerList.PushBack(handler);
        return CHIP_NO_ERROR;
    }

    void RemoveHandler(TestEventTriggerHandler * handler)
    {
        VerifyOrReturn(mHandlerList.Contains(handler));
        mHandlerList.Remove(handler);
    }

    void ClearAllHandlers() { mHandlerList.Clear(); }

protected:
    IntrusiveList<TestEventTriggerHandler, IntrusiveMode::AutoUnlink> mHandlerList;
};

/**
 * @brief TestEventTriggerDelegate that owns its enable key in RAM.
 *
 */
class SimpleTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    SimpleTestEventTriggerDelegate() { memset(&mEnableKey[0], 0, sizeof(mEnableKey)); }

    /**
     * @brief Initialize the delegate with a key
     *
     * @param enableKey - EnableKey to use for this instance.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT if enableKey is wrong size.
     */
    CHIP_ERROR Init(ByteSpan enableKey)
    {
        VerifyOrReturnError(enableKey.size() == sizeof(mEnableKey), CHIP_ERROR_INVALID_ARGUMENT);
        MutableByteSpan ourEnableKeySpan(mEnableKey);
        return CopySpanToMutableSpan(enableKey, ourEnableKeySpan);
    }

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override { return enableKey.data_equal(ByteSpan(mEnableKey)); }

private:
    uint8_t mEnableKey[TestEventTriggerDelegate::kEnableKeyLength];
};

} // namespace chip
