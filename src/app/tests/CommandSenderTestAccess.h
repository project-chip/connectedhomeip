/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/CommandSender.h>

namespace chip {
namespace Test {

/**
 * @brief Class acts as an accessor to private methods of the CommandSender class without needing to give friend access to
 *        each individual test.
 *        This is not a Global API and should only be used for (Unit) Testing.
 */
class CommandSenderTestAccess
{

public:
    CommandSenderTestAccess(app::CommandSender * aCommandSender) : mpCommandSender(aCommandSender) {}

    Messaging::ExchangeHolder & GetExchangeCtx() { return mpCommandSender->mExchangeCtx; }

    void FlushNoCommandResponse() { mpCommandSender->FlushNoCommandResponse(); }
    void SetFinishedCommandCount(uint16_t aFinishedCommandCount) { mpCommandSender->mFinishedCommandCount = aFinishedCommandCount; }
    void MoveToStateAddedCommand() { mpCommandSender->MoveToState(app::CommandSender::State::AddedCommand); }

    CHIP_ERROR Finalize(System::PacketBufferHandle & commandPacket) { return mpCommandSender->Finalize(commandPacket); }
    CHIP_ERROR ProcessInvokeResponse(System::PacketBufferHandle && payload, bool & moreChunkedMessages)
    {
        return mpCommandSender->ProcessInvokeResponse(std::move(payload), moreChunkedMessages);
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload)
    {
        return mpCommandSender->OnMessageReceived(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }

private:
    app::CommandSender * mpCommandSender = nullptr;
};

} // namespace Test
} // namespace chip
