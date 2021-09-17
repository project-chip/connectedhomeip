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
#include <app/MessageDef/CommandPath.h>
#include <app/MessageDef/StatusElement.h>

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
         * OnResponse will be called when the response from server arrived.
         *  - When StatusElement is received and it is Success, aData will be nullptr.
         *  - When command specific response is received, aData will be a valid TLVReader initialized with a struct of command
         * specify response.
         *
         * @param[in] apCommandSender: The command sender object that initialized the command transaction continue to exist after
         * this call completes.
         * @param[in] aPath: The command path field in invoke command response.
         * @param[in] aData: The command data, will be nullptr if the server returns a StatusElement.
         */
        virtual void OnResponse(const CommandSender * apCommandSender, const CommandPath::Type & aPath, TLV::TLVReader * aData) {}

        /**
         * OnError will be called when error occurred when receiving / handling response from server:
         * - CHIP_ERROR_TIMEOUT: The CommandSender failed to receive a expected response.
         * - TLV Related erros: The CommandSender received a malformed response from server.
         * - CHIP_ERROR_IM: The CommandSender received a legal response from server, but the protocol code indicates a error status
         * presended in aInteractionModelStatus.
         *
         * @param[in] apCommandSender: The command sender object that initialized the command transaction,  continue to exist after
         * this call completes.
         * @param[in] aInteractionModelStatus: The interaction model protocol status code, will be Failure if the CommandSender
         * failed to recieve the response (aError != CHIP_NO_ERROR). This argument will never be Success when this function is
         * called.
         * @param[in] aProtocolError: The error occurred during the invoke command interaction, will be CHIP_NO_ERROR when the
         * CommandSender received a valid response from server.
         */
        virtual void OnError(const CommandSender * apCommandSender, Protocols::InteractionModel::Status aInteractionModelStatus,
                             CHIP_ERROR aProtocolError)
        {}

        /**
         * OnFinal will be called when CommandSender has finished all the works, and it is safe to release the
         * allocated CommandSender object if needed.
         *
         * This function is marked as must be implemented to make application aware of releasing the object.
         *
         * After the command being sent, this function will always be called exactly once for a single command sender.
         *
         * @param[in] apCommandSender: The command sender object of the terminated invoke command transaction.
         */
        virtual void OnFinal(CommandSender * apCommandSender) = 0;
    };

    CommandSender(Callback * apCallback) : mpCallback(apCallback){};

    ~CommandSender() { AbortExistingExchangeContext(); }

    // TODO: issue #6792 - the secure session parameter should be made non-optional and passed by reference.
    // Once SendCommandRequest returns successfully, the CommandSender will
    // handle calling Shutdown on itself once it decides it's done with waiting
    // for a response (i.e. times out or gets a response).
    //
    // Client can specify the maximum time to wait for response (in milliseconds) via timeout parameter.
    // Default timeout value will be used otherwise.
    //
    // If SendCommandRequest is never called, or the call fails, the API
    // consumer is responsible for calling Shutdown on the CommandSender.
    CHIP_ERROR SendCommandRequest(NodeId aNodeId, FabricIndex aFabricIndex, Optional<SessionHandle> secureSession,
                                  uint32_t timeout = kImMessageTimeoutMsec);

private:
    // ExchangeDelegate interface implementation.  Private so people won't
    // accidentally call it on us when we're not being treated as an actual
    // ExchangeDelegate.
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    // CommandSender does not have "Shutdown" method, the user will destruct it directly.
    void ShutdownInternal();

    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;

    Callback * mpCallback = nullptr;
};

} // namespace app
} // namespace chip
