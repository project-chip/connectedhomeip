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
     * @brief Adds a pending response to the tracker. Tracked using CommandReference.
     *
     * @param [in] aCommandRef The CommandReference associated with the response for which we are waiting.
     * @return CHIP_NO_ERROR on success.
     * @return CHIP_ERROR_INVALID_ARGUMENT if aCommandRef is already being tracked.
     */
    virtual CHIP_ERROR AddPendingResponse(uint16_t aCommandRef) = 0;

    /**
     * @brief Removes the pending response from the tracker as a response is received.
     *
     * Responses are tracked using CommandReference.
     *
     * @param [in] aCommandRef The CommandReference of the response that is no longer pending.
     * @return CHIP_NO_ERROR on success.
     * @return CHIP_ERROR_KEY_NOT_FOUND if aCommandRef is not found in the tracker. This indicates the tracker was not waiting for a
     * response associated with this CommandReference.
     */
    virtual CHIP_ERROR ResponseReceived(uint16_t aCommandRef) = 0;

    /**
     * @brief Indicates whether a response associated with a CommandReference is currently pending.
     *
     * @param [in] aCommandRef The CommandReference associated with the response to check if response
     *             is pending.
     * @return True if the response is pending, False otherwise.
     */
    virtual bool IsResponsePending(uint16_t aCommandRef) = 0;

    /**
     * @brief Returns the number of pending responses.
     */
    virtual size_t Count() = 0;

    /**
     * @brief Pops a CommandReference associated with a pending response.
     *
     * Used after the server indicates that it has finished sending responses to the client.
     * Enables the client to report an error to upper layers if a response for the associated command was not received.
     *
     * @return NullOptional if no more pending commands exist.
     * @return Optional containing the CommandReference of a request that was not previously
     *         removed from the tracker. This request is removed from the tracker.
     */
    virtual Optional<uint16_t> PopPendingResponse() = 0;
};

} // namespace app
} // namespace chip
