/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file defines objects for a CHIP IM Invoke Command Sender
 *
 */

#pragma once

#include <type_traits>

#include <app/data-model/Encode.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

#include <app/Command.h>
#include <app/MessageDef/CommandPathIB.h>
#include <app/MessageDef/StatusIB.h>

#define COMMON_STATUS_SUCCESS 0

namespace chip {
namespace app {

class CommandSender final : public Command, public Messaging::ExchangeDelegate
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;

        /**
         * OnResponse will be called when a successful response from server has been received and processed. Specifically:
         *  - When a status code is received and it is IM::Success, aData will be nullptr.
         *  - When a data response is received, aData will point to a valid TLVReader initialized to point at the struct container
         *    that contains the data payload (callee will still need to open and process the container).
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] apCommandSender: The command sender object that initiated the command transaction.
         * @param[in] aPath: The command path field in invoke command response.
         * @param[in] aData: The command data, will be nullptr if the server returns a StatusIB.
         */
        virtual void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, TLV::TLVReader * aData) {}

        /**
         * OnError will be called when an error occurr *after* a successful call to SendCommandRequest(). The following
         * errors will be delivered through this call in the aError field:
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR_IM_STATUS_CODE_RECEIVED: An invoke response containing a status code denoting an error was received.
         *                  When the protocol ID in the received status is IM, aInteractionModelStatus will contain the IM status
         *                  code. Otherwise, aInteractionModelStatus will always be set to IM::Status::Failure.
         * - CHIP_ERROR*: All other cases.
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy and free the object.
         *
         * @param[in] apCommandSender: The command sender object that initiated the command transaction.
         * @param[in] aInteractionModelStatus: Contains an IM status code. This SHALL never be IM::Success, and will contain a valid
         * server-side emitted error if aProtocolError == CHIP_ERROR_IM_STATUS_CODE_RECEIVED.
         * @param[in] aError: A system error code that conveys the overall error code.
         */
        virtual void OnError(const CommandSender * apCommandSender, Protocols::InteractionModel::Status aInteractionModelStatus,
                             CHIP_ERROR aError)
        {}

        /**
         * OnDone will be called when CommandSender has finished all work and is safe to destory and free the
         * allocated CommandSender object.
         *
         * This function will:
         *      - Always be called exactly *once* for a given CommandSender instance.
         *      - Be called even in error circumstances.
         *      - Only be called after a successful call to SendCommandRequest as been made.
         *
         * This function must be implemented to destroy the CommandSender object.
         *
         * @param[in] apCommandSender: The command sender object of the terminated invoke command transaction.
         */
        virtual void OnDone(CommandSender * apCommandSender) = 0;
    };

    /*
     * Constructor.
     *
     * The callback passed in has to outlive this CommandSender object.
     */
    CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr);

    /**
     * API for adding a data request.  The template parameter T is generally
     * expected to be a ClusterName::Commands::CommandName::Type struct, but any
     * object that can be encoded using the DataModel::Encode machinery and
     * exposes the right command id will work.
     *
     * @param [in] aRequestCommandPath the path of the command being requested.
     * @param [in] aData the data for the request.
     */
    template <typename CommandDataT>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData)
    {
        ReturnErrorOnFailure(PrepareCommand(aCommandPath, /* aStartDataStruct = */ false));
        TLV::TLVWriter * writer = GetCommandDataElementTLVWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, TLV::ContextTag(CommandDataElement::kCsTag_Data), aData));
        return FinishCommand(/* aEndDataStruct = */ false);
    }

    // TODO: issue #6792 - the secure session parameter should be made non-optional and passed by reference.
    //
    // Sends a queued up command request to the target encapsulated by the secureSession handle.
    //
    // Upon successful return from this call, all subsequent errors that occur during this interaction
    // will be conveyed through the OnError callback above. In addition, upon completion of work regardless of
    // whether it was successful or not, the OnDone callback will be invoked to indicate completion of work on this
    // object and to indicate to the application that it can destory and free this object.
    //
    // Applications can, however, destroy this object at any time after this call, except while handling
    // an OnResponse or OnError callback, and it will safely clean-up.
    //
    // If this call returns failure, the callback's OnDone will never be called; the client is responsible
    // for destroying this object on failure.
    //
    // Client can specify the maximum time to wait for response (in milliseconds) via timeout parameter.
    // Default timeout value will be used otherwise.
    //
    CHIP_ERROR SendCommandRequest(NodeId aNodeId, FabricIndex aFabricIndex, Optional<SessionHandle> secureSession,
                                  uint32_t timeout = kImMessageTimeoutMsec);

private:
    // ExchangeDelegate interface implementation.  Private so people won't
    // accidentally call it on us when we're not being treated as an actual
    // ExchangeDelegate.
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    //
    // Called internally to signal the completion of all work on this object, gracefully close the
    // exchange (by calling into the base class) and finally, signal to the application that it's
    // safe to release this object.
    //
    void Close();

    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;

    Callback * mpCallback                      = nullptr;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
};

} // namespace app
} // namespace chip
