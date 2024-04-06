/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stdint.h>

#include <app/TestEventTriggerDelegate.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

/**
 * @brief User handler for handling the test event trigger
 *
 * @note If TestEventTrigger is enabled, it needs to be implemented in the app
 *
 * @param eventTrigger Event trigger to handle
 *
 * @warning *** DO NOT USE FOR STANDARD CLUSTER EVENT TRIGGERS ***
 *
 * TODO(#31723): Rename `emberAfHandleEventTrigger` to `SilabsHandleGlobalTestEventTrigger`
 *
 * @retval true on success
 * @retval false if error happened
 */
bool emberAfHandleEventTrigger(uint64_t eventTrigger);

namespace chip {

class SilabsTestEventTriggerDelegate : public TestEventTriggerDelegate, TestEventTriggerHandler
{
public:
    explicit SilabsTestEventTriggerDelegate(const ByteSpan & enableKey) : mEnableKey(enableKey)
    {
        VerifyOrDie(AddHandler(this) == CHIP_NO_ERROR);
    }

    /**
     * @brief Checks to see if `enableKey` provided matches value chosen by the manufacturer.
     * @param enableKey Buffer of the key to verify.
     * @return True or False.
     */
    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;

    /**
     * @brief Delegates handling to global `emberAfHandleEventTrigger` function. DO NOT EXTEND.
     *
     * @param eventTrigger - trigger to process.
     * @return CHIP_NO_ERROR if properly handled, else another CHIP_ERROR.
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        // WARNING: LEGACY SUPPORT ONLY, DO NOT EXTEND FOR STANDARD CLUSTERS
        return (emberAfHandleEventTrigger(eventTrigger)) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    }

private:
    ByteSpan mEnableKey;
};

} // namespace chip
