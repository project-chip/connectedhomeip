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
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>
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
    enum class CommandRoleId
    {
        SenderId  = 0,
        HandlerId = 1,
    };

    enum class CommandState
    {
        Idle,                ///< Default state that the object starts out in, where no work has commenced
        AddingCommand,       ///< In the process of adding a command.
        AddedCommand,        ///< A command has been completely encoded and is awaiting transmission.
        CommandSent,         ///< The command has been sent successfully.
        AwaitingDestruction, ///< The object has completed its work and is awaiting destruction by the application.
    };

    /*
     * Destructor - as part of destruction, it will abort the exchange context
     * if a valid one still exists.
     *
     * See Abort() for details on when that might occur.
     */
    virtual ~Command() { Abort(); }

    /*
     * A set of methods to construct command request or response payloads
     */
    CHIP_ERROR PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct = true);
    TLV::TLVWriter * GetCommandDataIBTLVWriter();
    CHIP_ERROR FinishCommand(bool aEndDataStruct = true);
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

    virtual CHIP_ERROR ProcessCommandDataIB(CommandDataIB::Parser & aCommandElement) = 0;

protected:
    Command();

    /*
     * Allocates a packet buffer used for encoding an invoke request/response payload.
     *
     * This can be called multiple times safely, as it will only allocate the buffer once for the lifetime
     * of this object.
     */
    CHIP_ERROR AllocateBuffer();

    /*
     * The actual closure of the exchange happens automatically in the exchange layer.
     * This function just sets the internally tracked exchange pointer to null to align
     * with the exchange layer so as to prevent further closure if Abort() is called later.
     */
    void Close();

    void MoveToState(const CommandState aTargetState);
    CHIP_ERROR ProcessCommandMessage(System::PacketBufferHandle && payload, CommandRoleId aCommandRoleId);
    CHIP_ERROR ConstructCommandPath(const CommandPathParams & aCommandPathParams, CommandDataIB::Builder aCommandDataIB);
    const char * GetStateStr() const;

    InvokeCommand::Builder mInvokeCommandBuilder;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    uint8_t mCommandIndex                      = 0;
    CommandState mState                        = CommandState::Idle;
    chip::System::PacketBufferTLVWriter mCommandMessageWriter;

private:
    /*
     * This forcibly closes the exchange context if a valid one is pointed to. Such a situation does
     * not arise during normal message processing flows that all normally call Close() above. This can only
     * arise due to application-initiated destruction of the object when this object is handling receiving/sending
     * message payloads.
     */
    void Abort();

    friend class TestCommandInteraction;
    TLV::TLVType mDataElementContainerType = TLV::kTLVType_NotSpecified;
    bool mBufferAllocated                  = false;
};
} // namespace app
} // namespace chip
