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

    void Shutdown() override;

private:
    friend class TestCommandInteraction;
    friend class CommandHandlerHandleDeleter;
    CHIP_ERROR SendCommandResponse();
    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;

    uint32_t mPendingWorkCount = 0;

public:
    /**
     * @brief Holder class for CommandHandler
     * The user is expected to use this class when they need to process the command in background.
     */
    class CommandHandlerAsyncHolder
    {
    public:
        CommandHandlerAsyncHolder() = default;
        ~CommandHandlerAsyncHolder();

        /**
         * @brief Acquires a CommandHandler and mark it as pending for background processing.
         * This function should be called on CHIP main loop, this case should not be a issue since this function should be called
         * inside a command handler, which is already inside the CHIP mainloop.
         *   Destruct CommandHandlerAsyncHolder with active command handle will cause memory leak.
         * @return CHIP_ERROR_INCORRECT_STATE when the CommandHandlerAsyncHolder is already holding a command handler.
         */
        CHIP_ERROR AcquirePendingWork(CommandHandler * apCommandHandler);

        /**
         * @brief Acquires a CommandHandler and mark it as pending for background processing.
         * This function should be called on CHIP main loop, i.e. for example, the user can call PlatformMgr().ScheduleWork to run a
         * function on CHIP main loop.
         * The CommandHandler will be released regardless the return value of this function.
         * @return CHIP_ERROR_INCORRECT_STATE when the CommandHandlerAsyncHolder is not holding a command handler.
         */
        CHIP_ERROR FinishPendingWorkAndRelease();

        bool IsValid() { return mCommandHandler != nullptr; }

        CommandHandler * operator->() { return mCommandHandler; }

        CommandHandlerAsyncHolder(CommandHandlerAsyncHolder && handler)
        {
            mCommandHandler         = handler.mCommandHandler;
            handler.mCommandHandler = nullptr;
        }

    private:
        friend CommandHandler;
        CommandHandlerAsyncHolder(const CommandHandlerAsyncHolder &) = delete;

        CommandHandler * mCommandHandler = nullptr;
    };
    CHIP_ERROR FinishPendingWork();
};

} // namespace app
} // namespace chip
