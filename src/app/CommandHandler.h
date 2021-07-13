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
 *      This file defines object for a CHIP IM Invoke Command Handler
 *
 */

#pragma once

#include <memory>

#include <app/Command.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

class CommandHandler : public Command
{
public:
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate) override;
    CHIP_ERROR OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                      const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload);
    CHIP_ERROR AddStatusCode(const CommandPathParams & aCommandPathParams,
                             const Protocols::SecureChannel::GeneralStatusCode aGeneralCode, const Protocols::Id aProtocolId,
                             const Protocols::InteractionModel::ProtocolCode aProtocolCode) override;

private:
    friend class TestCommandInteraction;
    friend class CommandHandlerHandleDeleter;
    CHIP_ERROR SendCommandResponse();
    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;
    static void FinishPendingWork(System::Layer * aLayer, void * aAppState, CHIP_ERROR aError);

    struct CommandHandlerHandleDeleter
    {
        void operator()(CommandHandler * apCommandHandler) const;
    };

    uint32_t mPendingWorkCount = 0;

public:
    using CommandHandlerAsyncHandle = std::unique_ptr<CommandHandler, CommandHandlerHandleDeleter>;
    /**
     * @brief Usually, the command handlers do things very fast, and a synchronous handler is OK, however, some commands might
     * require some background work, thus we need to mark the current CommandHander as an async command handler.
     * - Releasing the CommandHandlerAsyncHandle means the owner has added the necessary responses (status code or response command,
     * determined by what has been put put in the response by cluster handler), and the response will be sent by interaction model.
     * - Releasing the CommandHandlerAsyncHandle before encoding a response might result in malformed response.
     * - If PreparePendingWork is never called, interaction model engine will try to send response (and close CommandHandler) after
     * the handler function returned.
     *
     * @return A valid CommandHandlerAsyncHandle if the CommandHandler is initialized, or nullptr otherwise.
     */
    CommandHandlerAsyncHandle PreparePendingWork();
};

} // namespace app
} // namespace chip
