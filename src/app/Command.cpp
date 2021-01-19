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

CHIP_ERROR Command::Init(Messaging::ExchangeManager * apExchangeMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    if (mpExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mpExchangeMgr = apExchangeMgr;
    mpExchangeCtx = nullptr;

    err = Reset();
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR Command::Reset()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ClearExistingExchangeContext();

    if (mCommandMessageBuf.IsNull())
    {
        // TODO: Calculate the packet buffer size
        mCommandMessageBuf = System::PacketBuffer::New();
        VerifyOrExit(!mCommandMessageBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    }

    mCommandMessageWriter.Init(std::move(mCommandMessageBuf));
    err = mInvokeCommandBuilder.Init(&mCommandMessageWriter);
    SuccessOrExit(err);

    mInvokeCommandBuilder.CreateCommandListBuilder();
    MoveToState(kState_Initialized);

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

    err = invokeCommandParser.CheckSchemaValidity();
    SuccessOrExit(err);

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

        err = commandElement.CheckSchemaValidity();
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

void Command::Shutdown()
{
    VerifyOrExit(mState != kState_Uninitialized, );
    mCommandMessageWriter.Reset();
    mCommandMessageBuf = nullptr;

    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }
    mpExchangeMgr = nullptr;
    MoveToState(kState_Uninitialized);

exit:
    return;
}

chip::TLV::TLVWriter & Command::CreateCommandDataElementTLVWriter()
{
    mCommandDataBuf = chip::System::PacketBuffer::New();
    if (mCommandDataBuf.IsNull())
    {
        ChipLogDetail(DataManagement, "Unable to allocate packet buffer");
    }

    mCommandDataWriter.Init(mCommandDataBuf.Retain());

    return mCommandDataWriter;
}

CHIP_ERROR Command::AddCommand(chip::EndpointId aEndpintId, chip::GroupId aGroupId, chip::ClusterId aClusterId,
                               chip::CommandId aCommandId, uint8_t aFlags)
{
    CommandParams commandParams;

    memset(&commandParams, 0, sizeof(CommandParams));

    commandParams.EndpointId = aEndpintId;
    commandParams.GroupId    = aGroupId;
    commandParams.ClusterId  = aClusterId;
    commandParams.CommandId  = aCommandId;
    commandParams.Flags      = aFlags;

    return AddCommand(commandParams);
}

CHIP_ERROR Command::AddCommand(CommandParams & aCommandParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    const uint8_t * apCommandData;
    uint32_t apCommandLen;

    apCommandData = mCommandDataBuf->Start();
    apCommandLen  = mCommandDataBuf->DataLength();

    if (apCommandLen > 0)
    {
        VerifyOrExit(apCommandLen > 2, err = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(apCommandData[0] == chip::TLV::kTLVType_Structure, err = CHIP_ERROR_INVALID_ARGUMENT);

        apCommandData += 1;
        apCommandLen -= 1;
    }

    {
        CommandDataElement::Builder commandDataElement =
            mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();
        CommandPath::Builder commandPath = commandDataElement.CreateCommandPathBuilder();
        if (aCommandParams.Flags & kCommandPathFlag_EndpointIdValid)
        {
            commandPath.EndpointId(aCommandParams.EndpointId);
        }

        if (aCommandParams.Flags & kCommandPathFlag_GroupIdValid)
        {
            commandPath.GroupId(aCommandParams.GroupId);
        }

        commandPath.NamespacedClusterId(aCommandParams.ClusterId).CommandId(aCommandParams.CommandId).EndOfCommandPath();

        err = commandPath.GetError();
        SuccessOrExit(err);

        if (apCommandLen > 0)
        {
            // Copy the application data into a new TLV structure field contained with the
            // command structure.  NOTE: The TLV writer will take care of moving the app data
            // to the correct location within the buffer.
            err = mInvokeCommandBuilder.GetWriter()->PutPreEncodedContainer(
                chip::TLV::ContextTag(CommandDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure, apCommandData, apCommandLen);
            SuccessOrExit(err);
        }
        commandDataElement.EndOfCommandDataElement();

        err = commandDataElement.GetError();
        SuccessOrExit(err);
    }
    MoveToState(kState_AddCommand);

exit:
    mCommandDataBuf = nullptr;
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR Command::AddStatusCode(const uint16_t aGeneralCode, const uint32_t aProtocolId, const uint16_t aProtocolCode,
                                  const chip::ClusterId aNamespacedClusterId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusElement::Builder statusElementBuilder;

    err = statusElementBuilder.Init(mInvokeCommandBuilder.GetWriter());
    SuccessOrExit(err);

    statusElementBuilder.EncodeStatusElement(aGeneralCode, aProtocolId, aProtocolCode, aProtocolCode).EndOfStatusElement();
    err = statusElementBuilder.GetError();

    MoveToState(kState_AddCommand);

exit:
    ChipLogFunctError(err);
    return err;
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

    mInvokeCommandBuilder.EndOfInvokeCommand();
    err = mInvokeCommandBuilder.GetError();
    SuccessOrExit(err);

    err = mCommandMessageWriter.Finalize(&mCommandMessageBuf);
    SuccessOrExit(err);

    mCommandMessageBuf->EnsureReservedSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);

exit:
    ChipLogFunctError(err);
    return err;
}

#if CHIP_DETAIL_LOGGING
const char * Command::GetStateStr() const
{
    switch (mState)
    {
    case kState_Uninitialized:
        return "Uninitialized";

    case kState_Initialized:
        return "Initialized";

    case kState_AddCommand:
        return "AddCommand";

    case kState_Sending:
        return "Sending";
    }
    return "N/A";
}
#endif // CHIP_DETAIL_LOGGING

void Command::MoveToState(const CommandState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "ICR moving to [%10.10s]", GetStateStr());
}

void Command::ClearState(void)
{
    MoveToState(kState_Uninitialized);
}

} // namespace app
} // namespace chip
