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

#pragma once

#include <app/CommandPathParams.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelDelegate.h>
#include <app/MessageDef/CommandDataIB.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace app {

class Command
{
public:
    enum class CommandState
    {
        Idle,                ///< Default state that the object starts out in, where no work has commenced
        AddingCommand,       ///< In the process of adding a command.
        AddedCommand,        ///< A command has been completely encoded and is awaiting transmission.
        AwaitingTimedStatus, ///< Sent a Timed Request and waiting for response.
        CommandSent,         ///< The command has been sent successfully.
        ResponseReceived,    ///< Received a response to our invoke and request and processing the response.
        AwaitingDestruction, ///< The object has completed its work and is awaiting destruction by the application.
    };

    /*
     * Destructor - as part of destruction, it will abort the exchange context
     * if a valid one still exists.
     *
     * See Abort() for details on when that might occur.
     */
    virtual ~Command() { Abort(); }

    CHIP_ERROR Finalize(System::PacketBufferHandle & commandPacket);

    virtual CHIP_ERROR AddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    virtual CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    virtual CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Gets the inner exchange context object, without ownership.
     *
     * @return The inner exchange context, might be nullptr if no
     *         exchange context has been assigned or the context
     *         has been released.
     */
    Messaging::ExchangeContext * GetExchangeContext() const { return mpExchangeCtx; }

protected:
    Command();

    /*
     * The actual closure of the exchange happens automatically in the exchange layer.
     * This function just sets the internally tracked exchange pointer to null to align
     * with the exchange layer so as to prevent further closure if Abort() is called later.
     */
    void Close();

    void MoveToState(const CommandState aTargetState);
    const char * GetStateStr() const;

    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    uint8_t mCommandIndex                      = 0;
    CommandState mState                        = CommandState::Idle;
    chip::System::PacketBufferTLVWriter mCommandMessageWriter;
    bool mBufferAllocated = false;

private:
    /*
     * This forcibly closes the exchange context if a valid one is pointed to. Such a situation does
     * not arise during normal message processing flows that all normally call Close() above. This can only
     * arise due to application-initiated destruction of the object when this object is handling receiving/sending
     * message payloads.
     */
    void Abort();

    friend class TestCommandInteraction;
};
} // namespace app
} // namespace chip
