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

Command::Command(Messaging::ExchangeManager * apExchangeMgr)
{
    mpExchangeMgr = apExchangeMgr;
}

CHIP_ERROR Command::AllocateBuffers()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandList::Builder commandListBuilder;

    if (!mBuffersAllocated) {
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

        mBuffersAllocated = true;
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

        CommandDataElement::Parser commandElement;

        err = commandElement.Init(commandListReader);
        SuccessOrExit(err);

        err = ProcessCommandDataElement(commandElement);
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

CHIP_ERROR Command::PrepareCommand(const CommandPathParams & aCommandPathParams, bool aIsStatus)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataElement::Builder commandDataElement;

    err = AllocateBuffers();
    SuccessOrExit(err);

    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrExit(mState == CommandState::Idle, err = CHIP_ERROR_INCORRECT_STATE);

    commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();
    err                = commandDataElement.GetError();
    SuccessOrExit(err);

    err = ConstructCommandPath(aCommandPathParams, commandDataElement);
    SuccessOrExit(err);

    if (!aIsStatus)
    {
        err = commandDataElement.GetWriter()->StartContainer(TLV::ContextTag(CommandDataElement::kCsTag_Data),
                                                             TLV::kTLVType_Structure, mDataElementContainerType);
    }

    MoveToState(CommandState::AddingCommand);
    
exit:
    return err;
}

TLV::TLVWriter * Command::GetCommandDataElementTLVWriter()
{
    if (mState != CommandState::AddingCommand) {
       return nullptr;
    }
    else { 
        return mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuilder().GetWriter();
    }
}

CHIP_ERROR Command::Finalize(System::PacketBufferHandle & commandPacket)
{
    VerifyOrReturnError(mState == CommandState::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    return mCommandMessageWriter.Finalize(&commandPacket);
}

CHIP_ERROR Command::FinishCommand(bool aIsStatus)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mState == CommandState::AddingCommand, err = CHIP_ERROR_INCORRECT_STATE);

    CommandDataElement::Builder commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuilder();
    if (!aIsStatus)
    {
        ReturnErrorOnFailure(commandDataElement.GetWriter()->EndContainer(mDataElementContainerType));
    }

    ReturnErrorOnFailure(commandDataElement.EndOfCommandDataElement().GetError());
    ReturnErrorOnFailure(mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList().GetError());
    ReturnErrorOnFailure(mInvokeCommandBuilder.EndOfInvokeCommand().GetError());
    
    MoveToState(CommandState::AddedCommand);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Command::ConstructCommandPath(const CommandPathParams & aCommandPathParams,
                                         CommandDataElement::Builder aCommandDataElement)
{
    CommandPath::Builder commandPath = aCommandDataElement.CreateCommandPathBuilder();
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
    // (signaled by setting it to null, then we need to forcibly
    // tear it down.
    //
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }
}

void Command::Close()
{
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

void Command::ClearState(void)
{
    MoveToState(CommandState::Idle);
}

} // namespace app
} // namespace chip
