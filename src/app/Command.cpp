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
#include <core/CHIPTLVDebug.hpp>

namespace chip {
namespace app {

CHIP_ERROR Command::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(apExchangeMgr != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mpExchangeMgr = apExchangeMgr;
    mpDelegate    = apDelegate;
    err           = Reset();
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR Command::Reset()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandList::Builder commandListBuilder;
    ClearExistingExchangeContext();

    if (mCommandMessageBuf.IsNull())
    {
        // TODO: Calculate the packet buffer size
        mCommandMessageBuf = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
        VerifyOrExit(!mCommandMessageBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    }

    mCommandMessageWriter.Init(std::move(mCommandMessageBuf));
    err = mInvokeCommandBuilder.Init(&mCommandMessageWriter);
    SuccessOrExit(err);

    commandListBuilder = mInvokeCommandBuilder.CreateCommandListBuilder();
    err                = commandListBuilder.GetError();
    SuccessOrExit(err);
    MoveToState(CommandState::Initialized);

    mCommandIndex = 0;

exit:
    ChipLogFunctError(err);
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
    ChipLogFunctError(err);
    return err;
}

void Command::Shutdown()
{
    VerifyOrExit(mState != CommandState::Uninitialized, );
    mCommandMessageWriter.Reset();
    mCommandMessageBuf = nullptr;

    ClearExistingExchangeContext();

    mpExchangeMgr = nullptr;
    mpDelegate    = nullptr;
    ClearState();

    mCommandIndex = 0;
exit:
    return;
}

CHIP_ERROR Command::PrepareCommand(const CommandPathParams * const apCommandPathParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CommandDataElement::Builder commandDataElement =
        mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();
    err = commandDataElement.GetError();
    SuccessOrExit(err);

    if (apCommandPathParams != nullptr)
    {
        err = ConstructCommandPath(*apCommandPathParams, commandDataElement);
        SuccessOrExit(err);
    }

    err = commandDataElement.GetWriter()->StartContainer(TLV::ContextTag(CommandDataElement::kCsTag_Data), TLV::kTLVType_Structure,
                                                         mDataElementContainerType);
exit:
    ChipLogFunctError(err);
    return err;
}

TLV::TLVWriter * Command::GetCommandDataElementTLVWriter()
{
    return mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuilder().GetWriter();
}

CHIP_ERROR Command::FinishCommand()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CommandDataElement::Builder commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuilder();
    err                                            = commandDataElement.GetWriter()->EndContainer(mDataElementContainerType);
    SuccessOrExit(err);
    commandDataElement.EndOfCommandDataElement();
    err = commandDataElement.GetError();
    SuccessOrExit(err);
    MoveToState(CommandState::AddCommand);

exit:
    ChipLogFunctError(err);
    return err;
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

CHIP_ERROR Command::ClearExistingExchangeContext()
{
    // Discard any existing exchange context. Effectively we can only have one Echo exchange with
    // a single node at any one time.
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Command::FinalizeCommandsMessage()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandList::Builder commandListBuilder;
    VerifyOrExit(mState == CommandState::AddCommand, err = CHIP_ERROR_INCORRECT_STATE);
    commandListBuilder = mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList();
    err                = commandListBuilder.GetError();
    SuccessOrExit(err);

    mInvokeCommandBuilder.EndOfInvokeCommand();
    err = mInvokeCommandBuilder.GetError();
    SuccessOrExit(err);

    err = mCommandMessageWriter.Finalize(&mCommandMessageBuf);
    SuccessOrExit(err);
exit:
    ChipLogFunctError(err);
    return err;
}

const char * Command::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case CommandState::Uninitialized:
        return "Uninitialized";

    case CommandState::Initialized:
        return "Initialized";

    case CommandState::AddCommand:
        return "AddCommand";

    case CommandState::Sending:
        return "Sending";
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
    MoveToState(CommandState::Uninitialized);
}

} // namespace app
} // namespace chip
