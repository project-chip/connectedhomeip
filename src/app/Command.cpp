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
#include <lib/support/logging/Trace.h>

namespace chip {
namespace app {

CHIP_ERROR Command::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate)
{
    Logging::LogTracer tracer(__func__);

    // Error if already initialized.
    VerifyOrReturnError(apExchangeMgr != nullptr, tracer.LogError(CHIP_ERROR_INCORRECT_STATE));
    VerifyOrReturnError(mpExchangeMgr == nullptr, tracer.LogError(CHIP_ERROR_INCORRECT_STATE));

    mpExchangeMgr = apExchangeMgr;
    mpDelegate    = apDelegate;

    ReturnTracedErrorOnFailure(Reset(), tracer);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Command::Reset()
{
    Logging::LogTracer tracer(__func__);

    AbortExistingExchangeContext();

    mCommandMessageWriter.Reset();

    System::PacketBufferHandle commandPacket = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!commandPacket.IsNull(), tracer.LogError(CHIP_ERROR_NO_MEMORY));

    mCommandMessageWriter.Init(std::move(commandPacket));

    ReturnTracedErrorOnFailure(mInvokeCommandBuilder.Init(&mCommandMessageWriter), tracer);

    CommandList::Builder commandListBuilder = mInvokeCommandBuilder.CreateCommandListBuilder();

    ReturnTracedErrorOnFailure(commandListBuilder.GetError(), tracer);

    MoveToState(CommandState::Initialized);
    mCommandIndex = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Command::ProcessCommandMessage(System::PacketBufferHandle && payload, CommandRoleId aCommandRoleId)
{
    Logging::LogTracer tracer(__func__);

    chip::System::PacketBufferTLVReader reader;
    reader.Init(std::move(payload));
    ReturnTracedErrorOnFailure(reader.Next(), tracer);

    InvokeCommand::Parser invokeCommandParser;
    ReturnTracedErrorOnFailure(invokeCommandParser.Init(reader), tracer);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnTracedErrorOnFailure(invokeCommandParser.CheckSchemaValidity(), tracer);
#endif

    CommandList::Parser commandListParser;
    ReturnTracedErrorOnFailure(invokeCommandParser.GetCommandList(&commandListParser), tracer);

    chip::TLV::TLVReader commandListReader;
    commandListParser.GetReader(&commandListReader);

    CHIP_ERROR err;
    while (CHIP_NO_ERROR == (err = commandListReader.Next()))
    {
        VerifyOrReturnError(chip::TLV::AnonymousTag == commandListReader.GetTag(), tracer.LogError(CHIP_ERROR_INVALID_TLV_TAG));
        VerifyOrReturnError(chip::TLV::kTLVType_Structure == commandListReader.GetType(),
                            tracer.LogError(CHIP_ERROR_WRONG_TLV_TYPE));

        CommandDataElement::Parser commandElement;

        ReturnTracedErrorOnFailure(commandElement.Init(commandListReader), tracer);
        ReturnTracedErrorOnFailure(ProcessCommandDataElement(commandElement), tracer);
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        return CHIP_NO_ERROR;
    }
    else if (CHIP_NO_ERROR != err)
    {
        return tracer.LogError(err);
    }

    return CHIP_NO_ERROR;
}

void Command::Shutdown()
{
    VerifyOrReturn(mState != CommandState::Uninitialized);
    AbortExistingExchangeContext();
    ShutdownInternal();
}

void Command::ShutdownInternal()
{
    mCommandMessageWriter.Reset();

    mpExchangeMgr = nullptr;
    mpExchangeCtx = nullptr;
    mpDelegate    = nullptr;
    ClearState();

    mCommandIndex = 0;
}

CHIP_ERROR Command::PrepareCommand(const CommandPathParams & aCommandPathParams, bool aIsStatus)
{
    Logging::LogTracer tracer(__func__);

    VerifyOrReturnError(mState == CommandState::Initialized || mState == CommandState::AddCommand,
                        tracer.LogError(CHIP_ERROR_INCORRECT_STATE));

    CommandDataElement::Builder commandDataElement;
    commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();

    ReturnTracedErrorOnFailure(commandDataElement.GetError(), tracer);

    ReturnTracedErrorOnFailure(ConstructCommandPath(aCommandPathParams, commandDataElement), tracer);

    if (!aIsStatus)
    {
        ReturnTracedErrorOnFailure(commandDataElement.GetWriter()->StartContainer(TLV::ContextTag(CommandDataElement::kCsTag_Data),
                                                                                  TLV::kTLVType_Structure,
                                                                                  mDataElementContainerType),
                                   tracer);
    }

    return CHIP_NO_ERROR;
}

TLV::TLVWriter * Command::GetCommandDataElementTLVWriter()
{
    return mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuilder().GetWriter();
}

CHIP_ERROR Command::FinishCommand(bool aIsStatus)
{
    Logging::LogTracer tracer(__func__);

    CommandDataElement::Builder commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuilder();
    if (!aIsStatus)
    {
        ReturnTracedErrorOnFailure(commandDataElement.GetWriter()->EndContainer(mDataElementContainerType), tracer);
    }

    commandDataElement.EndOfCommandDataElement();
    ReturnTracedErrorOnFailure(commandDataElement.GetError(), tracer);

    MoveToState(CommandState::AddCommand);

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

CHIP_ERROR Command::AbortExistingExchangeContext()
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

CHIP_ERROR Command::FinalizeCommandsMessage(System::PacketBufferHandle & commandPacket)
{
    Logging::LogTracer tracer(__func__);

    VerifyOrReturnError(mState == CommandState::AddCommand, tracer.LogError(CHIP_ERROR_INCORRECT_STATE));

    CommandList::Builder commandListBuilder;
    commandListBuilder = mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList();

    ReturnTracedErrorOnFailure(commandListBuilder.GetError(), tracer);

    mInvokeCommandBuilder.EndOfInvokeCommand();
    ReturnTracedErrorOnFailure(mInvokeCommandBuilder.GetError(), tracer);

    ReturnTracedErrorOnFailure(mCommandMessageWriter.Finalize(&commandPacket), tracer);

    return CHIP_NO_ERROR;
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
