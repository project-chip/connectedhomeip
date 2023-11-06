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
#include <lib/support/Span.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {

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
     * Expectation is that the caller has already validated the enable key before calling this.
     * Handles the test event trigger based on `eventTrigger` provided.
     *
     * @param[in] eventTrigger Event trigger to handle.
     *
     * @return CHIP_NO_ERROR on success or another CHIP_ERROR on failure
     */
    virtual CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) = 0;
};

} // namespace chip
