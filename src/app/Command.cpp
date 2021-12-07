/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines Base class for a CHIP IM Command
 *
 */

#include "Command.h"
#include <app/AppBuildConfig.h>
#include <lib/core/CHIPTLVDebug.hpp>

namespace chip {
namespace app {

Command::Command() {}

CHIP_ERROR Command::Finalize(System::PacketBufferHandle & commandPacket)
{
    VerifyOrReturnError(mState == CommandState::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    return mCommandMessageWriter.Finalize(&commandPacket);
}

void Command::Abort()
{
    //
    // If the exchange context hasn't already been gracefully closed
    // (signaled by setting it to null), then we need to forcibly
    // tear it down.
    //
    if (mpExchangeCtx != nullptr)
    {
        // We (or more precisely our subclass) might be a delegate for this
        // exchange, and we don't want the OnExchangeClosing notification in
        // that case.  Null out the delegate to avoid that.
        //
        // TODO: This makes all sorts of assumptions about what the delegate is
        // (notice the "might" above!) that might not hold in practice.  We
        // really need a better solution here....
        mpExchangeCtx->SetDelegate(nullptr);
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }
}

void Command::Close()
{
    //
    // Shortly after this call to close and when handling an inbound message, it's entirely possible
    // for this object (courtesy of its derived class) to be destroyed
    // *before* the call unwinds all the way back to ExchangeContext::HandleMessage.
    //
    // As part of tearing down the exchange, there is logic there to invoke the delegate to notify
    // it of impending closure - which is this object, which just got destroyed!
    //
    // So prevent a use-after-free, set delegate to null.
    //
    // For more details, see #10344.
    //
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->SetDelegate(nullptr);
    }

    mpExchangeCtx = nullptr;
}

const char * Command::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case CommandState::Idle:
        return "Idle";

    case CommandState::AddingCommand:
        return "AddingCommand";

    case CommandState::AddedCommand:
        return "AddedCommand";

    case CommandState::AwaitingTimedStatus:
        return "AwaitingTimedStatus";

    case CommandState::CommandSent:
        return "CommandSent";

    case CommandState::ResponseReceived:
        return "ResponseReceived";

    case CommandState::AwaitingDestruction:
        return "AwaitingDestruction";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void Command::MoveToState(const CommandState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "ICR moving to [%10.10s]", GetStateStr());
}

} // namespace app
} // namespace chip
