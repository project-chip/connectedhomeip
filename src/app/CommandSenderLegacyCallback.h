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

#include <app/ConcreteCommandPath.h>
#include <app/MessageDef/StatusIB.h>
#include <lib/core/TLV.h>

namespace chip {
namespace app {

class CommandSender;

/**
 * @brief Legacy callbacks for CommandSender
 *
 * This class exists for legacy purposes. If you are developing a new callback implementation,
 * please use `CommandSender::ExtendableCallback`.
 */
class CommandSenderLegacyCallback
{
public:
    virtual ~CommandSenderLegacyCallback() = default;

    /**
     * OnResponse will be called when a successful response from server has been received and processed.
     * Specifically:
     *  - When a status code is received and it is IM::Success, aData will be nullptr.
     *  - When a data response is received, aData will point to a valid TLVReader initialized to point at the struct container
     *    that contains the data payload (callee will still need to open and process the container).
     *
     * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
     * receives an OnDone call to destroy the object.
     *
     * @param[in] apCommandSender The command sender object that initiated the command transaction.
     * @param[in] aPath           The command path field in invoke command response.
     * @param[in] aStatusIB       It will always have a success status. If apData is null, it can be any success status,
     *                            including possibly a cluster-specific one. If apData is not null it aStatusIB will always
     *                            be a generic SUCCESS status with no-cluster specific information.
     * @param[in] apData          The command data, will be nullptr if the server returns a StatusIB.
     */
    virtual void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, const StatusIB & aStatusIB,
                            TLV::TLVReader * apData)
    {}

    /**
     * OnError will be called when an error occurs *after* a successful call to SendCommandRequest(). The following
     * errors will be delivered through this call in the aError field:
     *
     * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
     * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
     * - CHIP_ERROR encapsulating a StatusIB: If we got a non-path-specific or path-specific
     *   status response from the server.  In that case, constructing a
     *   StatusIB from the error can be used to extract the status.
     *      - Note: a CommandSender using `CommandSender::Callback` only supports sending
     *        a single InvokeRequest. As a result, only one path-specific error is expected
     *        to ever be sent to the OnError callback.
     * - CHIP_ERROR*: All other cases.
     *
     * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
     * receives an OnDone call to destroy and free the object.
     *
     * @param[in] apCommandSender The command sender object that initiated the command transaction.
     * @param[in] aError          A system error code that conveys the overall error code.
     */
    virtual void OnError(const CommandSender * apCommandSender, CHIP_ERROR aError) {}

    /**
     * OnDone will be called when CommandSender has finished all work and it is safe to destroy and free the
     * allocated CommandSender object.
     *
     * This function will:
     *      - Always be called exactly *once* for a given CommandSender instance.
     *      - Be called even in error circumstances.
     *      - Only be called after a successful call to SendCommandRequest returns, if SendCommandRequest is used.
     *      - Always be called before a successful return from SendGroupCommandRequest, if SendGroupCommandRequest is used.
     *
     * This function must be implemented to destroy the CommandSender object.
     *
     * @param[in] apCommandSender   The command sender object of the terminated invoke command transaction.
     */
    virtual void OnDone(CommandSender * apCommandSender) = 0;
};

} // namespace app
} // namespace chip
