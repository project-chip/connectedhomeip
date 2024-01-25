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
 *   * Detect outstanding requests after the server indicates completion.
 *     This helps identify potential response omissions.
 */
class SentRequestTracker
{
public:
    virtual ~SentRequestTracker() = default;

    /**
     * @brief Adds a request to the tracker, associated with CommandReference.
     *
     * @param [in] aCommandRef The CommandReference associated with the request to be tracked.
     * @return CHIP_NO_ERROR on success.
     * @return CHIP_ERROR_INVALID_ARGUMENT if aCommandRef is already being tracked.
     */
    virtual CHIP_ERROR AddCommand(uint16_t aCommandRef) = 0;
    /**
     * @brief Removes the request associated with the CommandReference from the tracker.
     *
     * @param [in] aCommandRef The CommandReference of the request to remove.
     * @return CHIP_NO_ERROR on success.
     * @return CHIP_ERROR_KEY_NOT_FOUND if aCommandRef is not found in the tracker.
     */
    virtual CHIP_ERROR RemoveCommand(uint16_t aCommandRef) = 0;
    /**
     * @brief Indicates whether a request associated with a CommandReference is currently being tracked.
     *
     * @param [in] aCommandRef The CommandReference associated with the request to check.
     * @return True if the request is being tracked, False otherwise.
     */
    virtual bool IsCommandTracked(uint16_t aCommandRef) = 0;
    /**
     * @brief Returns the number of requests currently being tracked.
     */
    virtual size_t Count() = 0;
    /**
     * @brief Pops a CommandReference associated with a pending request.
     *
     * Used after the server indicates it has finished sending responses to the client.
     * Enables the client to report an error to upper layers if a response for the associated
     * request was not received.
     *
     * @return NullOptional if no more pending commands exist.
     * @return Optional containing the CommandReference of a request that was not previously
     *         removed from the tracker. This request is removed from the tracker.
     */
    virtual Optional<uint16_t> PopCommand() = 0;
};

} // namespace app
} // namespace chip