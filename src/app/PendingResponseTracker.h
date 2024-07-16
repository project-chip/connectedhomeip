/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/**
 * @brief Interface for tracking responses to outbound InvokeRequests.
 *
 * This interface enables clients to:
 *   * Verify that received responses correspond to issued InvokeRequests.
 *   * Detect outstanding responses after the server indicates completion, helpful for identifying response omissions.
 */
class PendingResponseTracker
{
public:
    virtual ~PendingResponseTracker() = default;

    /**
     * Start tracking the given `aCommandRef`
     *
     * @return CHIP_ERROR_INVALID_ARGUMENT if `aCommandRef` is already being tracked.
     */
    virtual CHIP_ERROR Add(uint16_t aCommandRef) = 0;

    /**
     * Removes tracking for the given `aCommandRef`
     *
     * @return CHIP_ERROR_KEY_NOT_FOUND if aCommandRef is not currently tracked.
     */
    virtual CHIP_ERROR Remove(uint16_t aCommandRef) = 0;

    /**
     * Checks if the given `aCommandRef` is being tracked.
     */
    virtual bool IsTracked(uint16_t aCommandRef) = 0;

    /**
     * Returns the number of pending responses.
     */
    virtual size_t Count() = 0;

    /**
     * Removes a pending response command reference from the tracker.
     *
     * Deletes an element from the tracker (order not guaranteed). This function can be called
     * repeatedly to remove all tracked pending responses.
     *
     * @return NullOptional if the tracker is empty.
     * @return Optional containing the CommandReference of a removed pending response.
     */
    virtual Optional<uint16_t> PopPendingResponse() = 0;
};

} // namespace app
} // namespace chip
