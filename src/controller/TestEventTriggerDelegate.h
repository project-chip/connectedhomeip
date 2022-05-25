/*
 *    Copyright (c) 2022 Project CHIP Authors
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
    /**
     * This API can be used to retrieve the test enable key. The enable key is copied into `buffer`
     * if the size is large enough.
     *
     * @param[out] enableKey Buffer to put the test enable key into
     *
     * @return CHIP_ERROR_BUFFER_TOO_SMALL the provided MutableByteSpan is not big enough.
     */
    // TODO should I just use MutableByteSpan instead?
    virtual CHIP_ERROR GetEnableKey(MutableByteSpan enableKey) = 0;

    /**
     * Expectation is that the caller has already validated the test enable key before calling this.
     * Configures the test event trigger based on `eventTrigger` provided.
     *
     * @param[in] eventTrigger Event trigger to configure
     *
     * @return CHIP_ERROR_INVALID_ARGUMENT when eventTrigger is not a valid test event trigger.
     */
    virtual CHIP_ERROR ConfigureTestEventTrigger(uint64_t eventTrigger) = 0;

    /**
     * Get the count of all configured test event triggers.
     *
     * @return count of configured event triggers.
     */
    virtual size_t ConfiguredEventTriggerCount() = 0;
};

#if 0
// This is a lighter weight interface from General Diagnostics Cluster perspective placing all the burden on each
// implementation to perform required validation.
class TestEventTriggerDelegate
{
public:
    virtual CHIP_ERROR ConfigureTestEventTrigger(const ByteSpan testEventKey, uint64_t eventTrigger) = 0;
    virtual size_t ConfiguredEventTriggerCount() = 0;
};
#endif 

} // namespace chip
