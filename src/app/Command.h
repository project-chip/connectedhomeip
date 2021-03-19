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

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/BitFlags.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <app/InteractionModelDelegate.h>
#include <app/MessageDef/CommandDataElement.h>
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>

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
        Uninitialized = 0, //< The invoke command message has not been initialized
        Initialized,       //< The invoke command message has been initialized and is ready
        AddCommand,        //< The invoke command message has added Command
        Sending,           //< The invoke command message  has sent out the invoke command
    };

    enum class CommandPathFlags : uint8_t
    {
        kEndpointIdValid = 0x01, /**< Set when the EndpointId field is valid */
        kGroupIdValid    = 0x02, /**< Set when the GroupId field is valid */
    };

    /**
     * Encapsulates arguments to be passed into SendCommand().
     *
     */
    struct CommandParams
    {
        CommandParams(chip::EndpointId endpointId, chip::GroupId groupId, chip::ClusterId clusterId, chip::CommandId commandId,
                      const BitFlags<CommandPathFlags> & flags) :
            EndpointId(endpointId),
            GroupId(groupId), ClusterId(clusterId), CommandId(commandId), Flags(flags)
        {}

        chip::EndpointId EndpointId;
        chip::GroupId GroupId;
        chip::ClusterId ClusterId;
        chip::CommandId CommandId;
        BitFlags<CommandPathFlags> Flags;
    };

    /**
     *  Initialize the Command object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          CommandState::NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate);

    /**
     *  Shutdown the CommandSender. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();

    /**
     * Finalize Command Message TLV Builder and finalize command message
     *
     * @return CHIP_ERROR
     *
     */
    CHIP_ERROR FinalizeCommandsMessage();

    chip::TLV::TLVWriter & CreateCommandDataElementTLVWriter();
    CHIP_ERROR AddCommand(chip::EndpointId aEndpintId, chip::GroupId aGroupId, chip::ClusterId aClusterId,
                          chip::CommandId aCommandId, BitFlags<CommandPathFlags> Flags);
    CHIP_ERROR AddCommand(CommandParams & aCommandParams);
    CHIP_ERROR AddStatusCode(const uint16_t aGeneralCode, Protocols::Id aProtocolId, const uint16_t aProtocolCode,
                             const chip::ClusterId aClusterId);

    /**
     * Gets the inner exchange context object, without ownership.
     *
     * @return The inner exchange context, might be nullptr if no
     *         exchange context has been assigned or the context
     *         has been released.
     */
    const Messaging::ExchangeContext * GetExchangeContext() const { return mpExchangeCtx; }

    CHIP_ERROR Reset();

    virtual ~Command() = default;

    bool IsFree() const { return (nullptr == mpExchangeCtx); };
    virtual CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) = 0;

protected:
    CHIP_ERROR ClearExistingExchangeContext();
    void MoveToState(const CommandState aTargetState);
    CHIP_ERROR ProcessCommandMessage(System::PacketBufferHandle && payload, CommandRoleId aCommandRoleId);
    void ClearState();
    const char * GetStateStr() const;

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;
    chip::System::PacketBufferHandle mCommandMessageBuf;
    CommandId mCommandIndex = 0;

private:
    chip::System::PacketBufferHandle mpBufHandle;
    InvokeCommand::Builder mInvokeCommandBuilder;
    CommandState mState;

    chip::System::PacketBufferHandle mCommandDataBuf;
    chip::System::PacketBufferTLVWriter mCommandMessageWriter;
    chip::System::PacketBufferTLVWriter mCommandDataWriter;
};
} // namespace app
} // namespace chip
