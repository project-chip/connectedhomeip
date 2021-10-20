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
#include "CommandHandler.h"
#include "CommandSender.h"
#include "InteractionModelEngine.h"

#include <app/AppBuildConfig.h>
#include <lib/core/CHIPTLVDebug.hpp>

namespace chip {
namespace app {

Command::Command() {}

CHIP_ERROR Command::AllocateBuffer()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandList::Builder commandListBuilder;

    if (!mBufferAllocated)
    {
        mCommandMessageWriter.Reset();

        System::PacketBufferHandle commandPacket = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
        VerifyOrExit(!commandPacket.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        mCommandMessageWriter.Init(std::move(commandPacket));
        err = mInvokeCommandBuilder.Init(&mCommandMessageWriter);
        SuccessOrExit(err);

        commandListBuilder = mInvokeCommandBuilder.CreateCommandListBuilder();
        err                = commandListBuilder.GetError();
        SuccessOrExit(err);

        mCommandIndex = 0;

        mBufferAllocated = true;
    }

exit:
    return err;
}

CHIP_ERROR Command::ProcessCommandMessage(System::PacketBufferHandle && payload, CommandRoleId aCommandRoleId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVReader reader;
    chip::TLV::TLVReader commandListReader;
    InvokeCommand::Parser invokeCommandParser;
    CommandList::Parser commandListParser;

    reader.Init(std::move(payload));
    err = reader.Next();
    SuccessOrExit(err);

    err = invokeCommandParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeCommandParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif
    err = invokeCommandParser.GetCommandList(&commandListParser);
    SuccessOrExit(err);

    commandListParser.GetReader(&commandListReader);

    while (CHIP_NO_ERROR == (err = commandListReader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == commandListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == commandListReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        CommandDataIB::Parser commandElement;

        err = commandElement.Init(commandListReader);
        SuccessOrExit(err);

        err = ProcessCommandDataIB(commandElement);
        SuccessOrExit(err);
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

CHIP_ERROR Command::PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Builder commandDataIB;

    err = AllocateBuffer();
    SuccessOrExit(err);

    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrExit(mState == CommandState::Idle, err = CHIP_ERROR_INCORRECT_STATE);

    commandDataIB = mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataIBBuilder();
    err           = commandDataIB.GetError();
    SuccessOrExit(err);

    err = ConstructCommandPath(aCommandPathParams, commandDataIB);
    SuccessOrExit(err);

    if (aStartDataStruct)
    {
        err = commandDataIB.GetWriter()->StartContainer(TLV::ContextTag(CommandDataIB::kCsTag_Data), TLV::kTLVType_Structure,
                                                        mDataElementContainerType);
    }

    MoveToState(CommandState::AddingCommand);

exit:
    return err;
}

TLV::TLVWriter * Command::GetCommandDataIBTLVWriter()
{
    if (mState != CommandState::AddingCommand)
    {
        return nullptr;
    }
    else
    {
        return mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataIBBuilder().GetWriter();
    }
}

CHIP_ERROR Command::Finalize(System::PacketBufferHandle & commandPacket)
{
    VerifyOrReturnError(mState == CommandState::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    return mCommandMessageWriter.Finalize(&commandPacket);
}

CHIP_ERROR Command::FinishCommand(bool aEndDataStruct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mState == CommandState::AddingCommand, err = CHIP_ERROR_INCORRECT_STATE);

    CommandDataIB::Builder commandDataIB = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataIBBuilder();
    if (aEndDataStruct)
    {
        ReturnErrorOnFailure(commandDataIB.GetWriter()->EndContainer(mDataElementContainerType));
    }

    ReturnErrorOnFailure(commandDataIB.EndOfCommandDataIB().GetError());
    ReturnErrorOnFailure(mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList().GetError());
    ReturnErrorOnFailure(mInvokeCommandBuilder.EndOfInvokeCommand().GetError());

    MoveToState(CommandState::AddedCommand);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Command::ConstructCommandPath(const CommandPathParams & aCommandPathParams, CommandDataIB::Builder aCommandDataIB)
{
    CommandPath::Builder commandPath = aCommandDataIB.CreateCommandPathBuilder();
    if (aCommandPathParams.mFlags.Has(CommandPathFlags::kEndpointIdValid))
    {
        commandPath.EndpointId(aCommandPathParams.mEndpointId);
    }

    if (aCommandPathParams.mFlags.Has(CommandPathFlags::kGroupIdValid))
    {
        commandPath.GroupId(aCommandPathParams.mGroupId);
    }

    commandPath.ClusterId(aCommandPathParams.mClusterId).CommandId(aCommandPathParams.mCommandId).EndOfCommandPath();

    return commandPath.GetError();
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

    case CommandState::CommandSent:
        return "CommandSent";

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
