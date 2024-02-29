/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements unit tests for CHIP Interaction Model Command Interaction
 *
 */

#include <cinttypes>

#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/Encode.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

using TestContext = chip::Test::AppContext;
using namespace chip::Protocols;

namespace {

void CheckForInvalidAction(nlTestSuite * apSuite, chip::Test::MessageCapturer & messageLog)
{
    NL_TEST_ASSERT(apSuite, messageLog.MessageCount() == 1);
    NL_TEST_ASSERT(apSuite, messageLog.IsMessageType(0, chip::Protocols::InteractionModel::MsgType::StatusResponse));
    CHIP_ERROR status;
    NL_TEST_ASSERT(apSuite,
                   chip::app::StatusResponse::ProcessStatusResponse(std::move(messageLog.MessagePayload(0)), status) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, status == CHIP_IM_GLOBAL_STATUS(InvalidAction));
}

} // anonymous namespace

namespace chip {

namespace {
bool isCommandDispatched      = false;
size_t commandDispatchedCount = 0;

bool sendResponse = true;
bool asyncCommand = false;

// Allow us to do test asserts from arbitrary places.
nlTestSuite * gSuite = nullptr;

constexpr EndpointId kTestEndpointId                      = 1;
constexpr ClusterId kTestClusterId                        = 3;
constexpr CommandId kTestCommandIdWithData                = 4;
constexpr CommandId kTestCommandIdNoData                  = 5;
constexpr CommandId kTestCommandIdCommandSpecificResponse = 6;
constexpr CommandId kTestCommandIdFillResponseMessage     = 7;
constexpr CommandId kTestNonExistCommandId                = 0;

const app::CommandHandler::TestOnlyMarker kCommandHandlerTestOnlyMarker;
const app::CommandSender::TestOnlyMarker kCommandSenderTestOnlyMarker;
} // namespace

namespace app {

CommandHandler::Handle asyncCommandHandle;

struct ForcedSizeBuffer
{
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> mBuffer;

    ForcedSizeBuffer(uint32_t size)
    {
        if (mBuffer.Alloc(size))
        {
            // No significance with using 0x12, just using a value.
            memset(mBuffer.Get(), 0x12, size);
        }
    }

    // No significance with using 0x12 as the CommandId, just using a value.
    static constexpr chip::CommandId GetCommandId() { return 0x12; }
    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
    {
        VerifyOrReturnError(mBuffer, CHIP_ERROR_NO_MEMORY);

        TLV::TLVType outerContainerType;
        ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, outerContainerType));
        ReturnErrorOnFailure(app::DataModel::Encode(aWriter, TLV::ContextTag(1), ByteSpan(mBuffer.Get(), mBuffer.AllocatedSize())));
        return aWriter.EndContainer(outerContainerType);
    }
};

enum class ForcedSizeBufferLengthHint
{
    kSizeBetween0and255,
    kSizeGreaterThan255,
};

InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aRequestCommandPath)
{
    // Mock cluster catalog, only support commands on one cluster on one endpoint.
    using InteractionModel::Status;

    if (aRequestCommandPath.mEndpointId != kTestEndpointId)
    {
        return Status::UnsupportedEndpoint;
    }

    if (aRequestCommandPath.mClusterId != kTestClusterId)
    {
        return Status::UnsupportedCluster;
    }

    if (aRequestCommandPath.mCommandId == kTestNonExistCommandId)
    {
        return Status::UnsupportedCommand;
    }

    return Status::Success;
}

void DispatchSingleClusterCommand(const ConcreteCommandPath & aRequestCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    ChipLogDetail(Controller, "Received Cluster Command: Endpoint=%x Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  aRequestCommandPath.mEndpointId, ChipLogValueMEI(aRequestCommandPath.mClusterId),
                  ChipLogValueMEI(aRequestCommandPath.mCommandId));

    // Duplicate what our normal command-field-decode code does, in terms of
    // checking for a struct and then entering it before getting the fields.
    if (aReader.GetType() != TLV::kTLVType_Structure)
    {
        apCommandObj->AddStatus(aRequestCommandPath, Protocols::InteractionModel::Status::InvalidAction);
        return;
    }

    TLV::TLVType outerContainerType;
    CHIP_ERROR err = aReader.EnterContainer(outerContainerType);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = aReader.Next();
    if (aRequestCommandPath.mCommandId == kTestCommandIdNoData)
    {
        NL_TEST_ASSERT(gSuite, err == CHIP_ERROR_END_OF_TLV);
    }
    else
    {
        NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, aReader.GetTag() == TLV::ContextTag(1));
        bool val;
        err = aReader.Get(val);
        NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, val);
    }

    err = aReader.ExitContainer(outerContainerType);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    if (asyncCommand)
    {
        asyncCommandHandle = apCommandObj;
        asyncCommand       = false;
    }

    if (sendResponse)
    {
        if (aRequestCommandPath.mCommandId == kTestCommandIdNoData || aRequestCommandPath.mCommandId == kTestCommandIdWithData)
        {
            apCommandObj->AddStatus(aRequestCommandPath, Protocols::InteractionModel::Status::Success);
        }
        else
        {
            const CommandHandler::InvokeResponseParameters prepareParams(aRequestCommandPath);
            const ConcreteCommandPath responseCommandPath = aRequestCommandPath;
            apCommandObj->PrepareInvokeResponseCommand(responseCommandPath, prepareParams);
            chip::TLV::TLVWriter * writer = apCommandObj->GetCommandDataIBTLVWriter();
            writer->PutBoolean(chip::TLV::ContextTag(1), true);
            apCommandObj->FinishCommand();
        }
    }

    chip::isCommandDispatched = true;
    commandDispatchedCount++;
}

class MockCommandSenderCallback : public CommandSender::Callback
{
public:
    void OnResponse(chip::app::CommandSender * apCommandSender, const chip::app::ConcreteCommandPath & aPath,
                    const chip::app::StatusIB & aStatus, chip::TLV::TLVReader * aData) override
    {
        IgnoreUnusedVariable(apCommandSender);
        IgnoreUnusedVariable(aData);
        ChipLogDetail(Controller, "Received Cluster Command: Cluster=%" PRIx32 " Command=%" PRIx32 " Endpoint=%x", aPath.mClusterId,
                      aPath.mCommandId, aPath.mEndpointId);
        onResponseCalledTimes++;
    }
    void OnError(const chip::app::CommandSender * apCommandSender, CHIP_ERROR aError) override
    {
        ChipLogError(Controller, "OnError happens with %" CHIP_ERROR_FORMAT, aError.Format());
        mError = aError;
        onErrorCalledTimes++;
        mError = aError;
    }
    void OnDone(chip::app::CommandSender * apCommandSender) override { onFinalCalledTimes++; }

    void ResetCounter()
    {
        onResponseCalledTimes = 0;
        onErrorCalledTimes    = 0;
        onFinalCalledTimes    = 0;
    }

    int onResponseCalledTimes = 0;
    int onErrorCalledTimes    = 0;
    int onFinalCalledTimes    = 0;
    CHIP_ERROR mError         = CHIP_NO_ERROR;
} mockCommandSenderDelegate;

class MockCommandSenderExtendableCallback : public CommandSender::ExtendableCallback
{
public:
    void OnResponse(CommandSender * apCommandSender, const CommandSender::ResponseData & aResponseData) override
    {
        IgnoreUnusedVariable(apCommandSender);
        ChipLogDetail(Controller, "Received response for command: Cluster=%" PRIx32 " Command=%" PRIx32 " Endpoint=%x",
                      aResponseData.path.mClusterId, aResponseData.path.mCommandId, aResponseData.path.mEndpointId);
        onResponseCalledTimes++;
    }
    void OnNoResponse(CommandSender * commandSender, const CommandSender::NoResponseData & aNoResponseData) override
    {
        ChipLogError(Controller, "NoResponse received for command associated with CommandRef %u", aNoResponseData.commandRef);
        onNoResponseCalledTimes++;
    }
    void OnError(const CommandSender * apCommandSender, const CommandSender::ErrorData & aErrorData) override
    {
        ChipLogError(Controller, "OnError happens with %" CHIP_ERROR_FORMAT, aErrorData.error.Format());
        mError = aErrorData.error;
        onErrorCalledTimes++;
    }
    void OnDone(CommandSender * apCommandSender) override { onFinalCalledTimes++; }

    void ResetCounter()
    {
        onResponseCalledTimes   = 0;
        onNoResponseCalledTimes = 0;
        onErrorCalledTimes      = 0;
        onFinalCalledTimes      = 0;
    }

    int onResponseCalledTimes   = 0;
    int onNoResponseCalledTimes = 0;
    int onErrorCalledTimes      = 0;
    int onFinalCalledTimes      = 0;
    CHIP_ERROR mError           = CHIP_NO_ERROR;
} mockCommandSenderExtendedDelegate;

class MockCommandHandlerCallback : public CommandHandler::Callback
{
public:
    void OnDone(CommandHandler & apCommandHandler) final { onFinalCalledTimes++; }
    void DispatchCommand(CommandHandler & apCommandObj, const ConcreteCommandPath & aCommandPath, TLV::TLVReader & apPayload) final
    {
        DispatchSingleClusterCommand(aCommandPath, apPayload, &apCommandObj);
    }
    InteractionModel::Status CommandExists(const ConcreteCommandPath & aCommandPath)
    {
        return ServerClusterCommandExists(aCommandPath);
    }

    void ResetCounter() { onFinalCalledTimes = 0; }

    int onFinalCalledTimes = 0;
} mockCommandHandlerDelegate;

class TestCommandInteraction
{
public:
    static void TestCommandSenderWithWrongState(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithWrongState(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderWithSendCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendEmptyCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderExtendableApiWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderExtendableApiWithProcessReceivedMsgContainingInvalidCommandRef(nlTestSuite * apSuite,
                                                                                                void * apContext);
    static void TestCommandHandlerWithProcessReceivedNotExistCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerEncodeSimpleCommandData(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerCommandDataEncoding(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerCommandEncodeFailure(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage1(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage2(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage3(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage4(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerInvalidMessageSync(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerCommandEncodeExternalFailure(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerEncodeSimpleStatusCode(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendEmptyResponse(nlTestSuite * apSuite, void * apContext);

    static void TestCommandHandlerWithProcessReceivedEmptyDataMsg(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerRejectMultipleIdenticalCommands(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerRejectsMultipleCommandsWithIdenticalCommandRef(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerRejectMultipleCommandsWhenHandlerOnlySupportsOne(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerAcceptMultipleCommands(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsStatusResponse(nlTestSuite * apSuite,
                                                                                                  void * apContext);
    static void TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponsePrimative(nlTestSuite * apSuite,
                                                                                                         void * apContext);
    static void TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponse(nlTestSuite * apSuite,
                                                                                                void * apContext);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    static void TestCommandHandlerReleaseWithExchangeClosed(nlTestSuite * apSuite, void * apContext);
#endif

    static void TestCommandSenderLegacyCallbackUnsupportedCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderExtendableCallbackUnsupportedCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderLegacyCallbackBuildingBatchCommandFails(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderExtendableCallbackBuildingBatchDuplicateCommandRefFails(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderExtendableCallbackBuildingBatchCommandSuccess(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderCommandSuccessResponseFlow(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderCommandAsyncSuccessResponseFlow(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderCommandFailureResponseFlow(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderCommandSpecificResponseFlow(nlTestSuite * apSuite, void * apContext);

    static void TestCommandSenderAbruptDestruction(nlTestSuite * apSuite, void * apContext);

    static size_t GetNumActiveHandlerObjects()
    {
        return chip::app::InteractionModelEngine::GetInstance()->mCommandHandlerObjs.Allocated();
    }

private:
    /**
     * With the introduction of batch invoke commands, CommandHandler keeps track of incoming
     * ConcreteCommandPath and the associated CommandRef. Normally this is populated as a part
     * of OnMessageReceived from the incoming request. For some unit tests were we want to just
     * test some of the processing messages we need to inject commands into the table of
     * ConcreteCommandPath/CommandRef pairs.
     */
    class CommandHandlerWithOutstandingCommand : public app::CommandHandler
    {
    public:
        CommandHandlerWithOutstandingCommand(CommandHandler::Callback * apCallback, const ConcreteCommandPath & aRequestCommandPath,
                                             const Optional<uint16_t> & aRef) :
            CommandHandler(apCallback)
        {
            GetCommandPathRegistry().Add(aRequestCommandPath, aRef);
        }
    };

    // Generate an invoke request.  If aCommandId is kTestCommandIdWithData, a
    // payload will be included.  Otherwise no payload will be included.
    static void GenerateInvokeRequest(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                      bool aIsTimedRequest, CommandId aCommandId, ClusterId aClusterId = kTestClusterId,
                                      EndpointId aEndpointId = kTestEndpointId);
    // Generate an invoke response.  If aCommandId is kTestCommandIdWithData, a
    // payload will be included.  Otherwise no payload will be included.
    static void GenerateInvokeResponse(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                       CommandId aCommandId, ClusterId aClusterId = kTestClusterId,
                                       EndpointId aEndpointId = kTestEndpointId, Optional<uint16_t> aCommandRef = NullOptional);
    static void AddInvokeRequestData(nlTestSuite * apSuite, void * apContext, CommandSender * apCommandSender,
                                     CommandId aCommandId = kTestCommandIdWithData);
    static void AddInvalidInvokeRequestData(nlTestSuite * apSuite, void * apContext, CommandSender * apCommandSender,
                                            CommandId aCommandId = kTestCommandIdWithData);
    static void AddInvokeResponseData(nlTestSuite * apSuite, void * apContext, CommandHandler * apCommandHandler,
                                      bool aNeedStatusCode, CommandId aResponseCommandId = kTestCommandIdWithData,
                                      CommandId aRequestCommandId = kTestCommandIdWithData);
    static uint32_t GetAddResponseDataOverheadSizeForPath(nlTestSuite * apSuite, const ConcreteCommandPath & aRequestCommandPath,
                                                          ForcedSizeBufferLengthHint aBufferSizeHint);
    static void FillCurrentInvokeResponseBuffer(nlTestSuite * apSuite, CommandHandler * apCommandHandler,
                                                const ConcreteCommandPath & aRequestCommandPath, uint32_t aSizeToLeaveInBuffer);
    static void ValidateCommandHandlerEncodeInvokeResponseMessage(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode);
};

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

CommandPathParams MakeTestCommandPath(CommandId aCommandId = kTestCommandIdWithData)
{
    return CommandPathParams(kTestEndpointId, 0, kTestClusterId, aCommandId, (chip::app::CommandPathFlags::kEndpointIdValid));
}

void TestCommandInteraction::GenerateInvokeRequest(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                                   bool aIsTimedRequest, CommandId aCommandId, ClusterId aClusterId,
                                                   EndpointId aEndpointId)

{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    err = invokeRequestMessageBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeRequestMessageBuilder.SuppressResponse(true).TimedRequest(aIsTimedRequest);
    InvokeRequests::Builder & invokeRequests = invokeRequestMessageBuilder.CreateInvokeRequests();
    NL_TEST_ASSERT(apSuite, invokeRequestMessageBuilder.GetError() == CHIP_NO_ERROR);

    CommandDataIB::Builder & commandDataIBBuilder = invokeRequests.CreateCommandData();
    NL_TEST_ASSERT(apSuite, invokeRequests.GetError() == CHIP_NO_ERROR);

    CommandPathIB::Builder & commandPathBuilder = commandDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, commandDataIBBuilder.GetError() == CHIP_NO_ERROR);

    commandPathBuilder.EndpointId(aEndpointId).ClusterId(aClusterId).CommandId(aCommandId).EndOfCommandPathIB();
    NL_TEST_ASSERT(apSuite, commandPathBuilder.GetError() == CHIP_NO_ERROR);

    if (aCommandId == kTestCommandIdWithData)
    {
        chip::TLV::TLVWriter * pWriter = commandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kFields)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    commandDataIBBuilder.EndOfCommandDataIB();
    NL_TEST_ASSERT(apSuite, commandDataIBBuilder.GetError() == CHIP_NO_ERROR);

    invokeRequests.EndOfInvokeRequests();
    NL_TEST_ASSERT(apSuite, invokeRequests.GetError() == CHIP_NO_ERROR);

    invokeRequestMessageBuilder.EndOfInvokeRequestMessage();
    NL_TEST_ASSERT(apSuite, invokeRequestMessageBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::GenerateInvokeResponse(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                                    CommandId aCommandId, ClusterId aClusterId, EndpointId aEndpointId,
                                                    Optional<uint16_t> aCommandRef)

{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    err = invokeResponseMessageBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeResponseMessageBuilder.SuppressResponse(true);
    InvokeResponseIBs::Builder & invokeResponses = invokeResponseMessageBuilder.CreateInvokeResponses();
    NL_TEST_ASSERT(apSuite, invokeResponseMessageBuilder.GetError() == CHIP_NO_ERROR);

    InvokeResponseIB::Builder & invokeResponseIBBuilder = invokeResponses.CreateInvokeResponse();
    NL_TEST_ASSERT(apSuite, invokeResponses.GetError() == CHIP_NO_ERROR);

    CommandDataIB::Builder & commandDataIBBuilder = invokeResponseIBBuilder.CreateCommand();
    NL_TEST_ASSERT(apSuite, commandDataIBBuilder.GetError() == CHIP_NO_ERROR);

    CommandPathIB::Builder & commandPathBuilder = commandDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, commandDataIBBuilder.GetError() == CHIP_NO_ERROR);

    commandPathBuilder.EndpointId(aEndpointId).ClusterId(aClusterId).CommandId(aCommandId).EndOfCommandPathIB();
    NL_TEST_ASSERT(apSuite, commandPathBuilder.GetError() == CHIP_NO_ERROR);

    if (aCommandId == kTestCommandIdWithData)
    {
        chip::TLV::TLVWriter * pWriter = commandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kFields)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    if (aCommandRef.HasValue())
    {
        NL_TEST_ASSERT(apSuite, commandDataIBBuilder.Ref(aCommandRef.Value()) == CHIP_NO_ERROR);
    }

    commandDataIBBuilder.EndOfCommandDataIB();
    NL_TEST_ASSERT(apSuite, commandDataIBBuilder.GetError() == CHIP_NO_ERROR);

    invokeResponseIBBuilder.EndOfInvokeResponseIB();
    NL_TEST_ASSERT(apSuite, invokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);

    invokeResponses.EndOfInvokeResponses();
    NL_TEST_ASSERT(apSuite, invokeResponses.GetError() == CHIP_NO_ERROR);

    invokeResponseMessageBuilder.EndOfInvokeResponseMessage();
    NL_TEST_ASSERT(apSuite, invokeResponseMessageBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::AddInvokeRequestData(nlTestSuite * apSuite, void * apContext, CommandSender * apCommandSender,
                                                  CommandId aCommandId)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    auto commandPathParams = MakeTestCommandPath(aCommandId);

    err = apCommandSender->PrepareCommand(commandPathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = apCommandSender->GetCommandDataIBTLVWriter();

    err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = apCommandSender->FinishCommand();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::AddInvalidInvokeRequestData(nlTestSuite * apSuite, void * apContext, CommandSender * apCommandSender,
                                                         CommandId aCommandId)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    auto commandPathParams = MakeTestCommandPath(aCommandId);

    err = apCommandSender->PrepareCommand(commandPathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = apCommandSender->GetCommandDataIBTLVWriter();

    err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    apCommandSender->MoveToState(CommandSender::State::AddedCommand);
}

void TestCommandInteraction::AddInvokeResponseData(nlTestSuite * apSuite, void * apContext, CommandHandler * apCommandHandler,
                                                   bool aNeedStatusCode, CommandId aResponseCommandId, CommandId aRequestCommandId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    constexpr EndpointId kTestEndpointId   = 1;
    constexpr ClusterId kTestClusterId     = 3;
    ConcreteCommandPath requestCommandPath = { kTestEndpointId, kTestClusterId, aRequestCommandId };
    if (aNeedStatusCode)
    {
        apCommandHandler->AddStatus(requestCommandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        const CommandHandler::InvokeResponseParameters prepareParams(requestCommandPath);
        ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, aResponseCommandId };
        err = apCommandHandler->PrepareInvokeResponseCommand(responseCommandPath, prepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        chip::TLV::TLVWriter * writer = apCommandHandler->GetCommandDataIBTLVWriter();

        err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = apCommandHandler->FinishCommand();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

uint32_t TestCommandInteraction::GetAddResponseDataOverheadSizeForPath(nlTestSuite * apSuite,
                                                                       const ConcreteCommandPath & aRequestCommandPath,
                                                                       ForcedSizeBufferLengthHint aBufferSizeHint)
{
    BasicCommandPathRegistry<4> mBasicCommandPathRegistry;
    CommandHandler commandHandler(kCommandHandlerTestOnlyMarker, &mockCommandHandlerDelegate, &mBasicCommandPathRegistry);
    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = mBasicCommandPathRegistry.Add(requestCommandPath1, MakeOptional<uint16_t>(static_cast<uint16_t>(1)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = mBasicCommandPathRegistry.Add(requestCommandPath2, MakeOptional<uint16_t>(static_cast<uint16_t>(2)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandHandler.AllocateBuffer();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    uint32_t remainingSizeBefore = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();

    // When ForcedSizeBuffer exceeds 255, an extra byte is needed for length, affecting the overhead size required by
    // AddResponseData. In order to have this accounted for in overhead calculation we set the length to be 256.
    uint32_t sizeOfForcedSizeBuffer = aBufferSizeHint == ForcedSizeBufferLengthHint::kSizeGreaterThan255 ? 256 : 0;
    err                             = commandHandler.AddResponseData(aRequestCommandPath, ForcedSizeBuffer(sizeOfForcedSizeBuffer));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    uint32_t remainingSizeAfter = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    uint32_t delta              = remainingSizeBefore - remainingSizeAfter - sizeOfForcedSizeBuffer;

    return delta;
}

void TestCommandInteraction::FillCurrentInvokeResponseBuffer(nlTestSuite * apSuite, CommandHandler * apCommandHandler,
                                                             const ConcreteCommandPath & aRequestCommandPath,
                                                             uint32_t aSizeToLeaveInBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = apCommandHandler->AllocateBuffer();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    uint32_t remainingSize = apCommandHandler->mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();

    // AddResponseData's overhead calculation depends on the size of ForcedSizeBuffer. If the buffer exceeds 255 bytes, an extra
    // length byte is required. Since tests using FillCurrentInvokeResponseBuffer currently end up with sizeToFill > 255, we
    // inform the calculation of this expectation. Nonetheless, we also validate this assumption for correctness.
    ForcedSizeBufferLengthHint bufferSizeHint = ForcedSizeBufferLengthHint::kSizeGreaterThan255;
    uint32_t overheadSizeNeededForAddingResponse =
        GetAddResponseDataOverheadSizeForPath(apSuite, aRequestCommandPath, bufferSizeHint);
    NL_TEST_ASSERT(apSuite, remainingSize > (aSizeToLeaveInBuffer + overheadSizeNeededForAddingResponse));
    uint32_t sizeToFill = remainingSize - aSizeToLeaveInBuffer - overheadSizeNeededForAddingResponse;

    // Validating assumption. If this fails, it means overheadSizeNeededForAddingResponse is likely too large.
    NL_TEST_ASSERT(apSuite, sizeToFill >= 256);

    err = apCommandHandler->AddResponseData(aRequestCommandPath, ForcedSizeBuffer(sizeToFill));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::TestCommandSenderWithWrongState(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);
}

void TestCommandInteraction::TestCommandHandlerWithWrongState(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                       = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ConcreteCommandPath requestCommandPath  = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    CommandHandlerWithOutstandingCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);

    const CommandHandler::InvokeResponseParameters prepareParams(requestCommandPath);
    err = commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    TestExchangeDelegate delegate;

    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mResponseSender.SetExchangeContext(exchange);

    err = commandHandler.StartSendingCommandResponses();

    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    //
    // Ordinarily, the ExchangeContext will close itself upon sending the final message / error'ing out on a responder exchange
    // when unwinding back from an OnMessageReceived callback. Since that isn't the case in this artificial setup here
    // (where we created a responder exchange that's not responding to anything), we need
    // to explicitly close it out. This is not expected in normal application logic.
    //
    exchange->Close();
}

void TestCommandInteraction::TestCommandSenderWithSendCommand(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    GenerateInvokeResponse(apSuite, apContext, buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    err                      = commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, moreChunkedMessages == false);
}

void TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                       = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ConcreteCommandPath requestCommandPath  = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    CommandHandlerWithOutstandingCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mResponseSender.SetExchangeContext(exchange);

    const CommandHandler::InvokeResponseParameters prepareParams(requestCommandPath);
    err = commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.FinishCommand();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.StartSendingCommandResponses();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandHandler.Close();
    ctx.GetLoopback().Reset();
}

void TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateInvokeResponse(apSuite, apContext, buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    err                      = commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, moreChunkedMessages == false);
}

void TestCommandInteraction::TestCommandSenderExtendableApiWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);

    uint16_t mockCommandRef = 1;
    pendingResponseTracker.Add(mockCommandRef);
    commandSender.mFinishedCommandCount = 1;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateInvokeResponse(apSuite, apContext, buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    err                      = commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, moreChunkedMessages == false);

    commandSender.FlushNoCommandResponse();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderExtendedDelegate.onResponseCalledTimes == 1 &&
                       mockCommandSenderExtendedDelegate.onFinalCalledTimes == 0 &&
                       mockCommandSenderExtendedDelegate.onNoResponseCalledTimes == 0 &&
                       mockCommandSenderExtendedDelegate.onErrorCalledTimes == 0);
}

void TestCommandInteraction::TestCommandSenderExtendableApiWithProcessReceivedMsgContainingInvalidCommandRef(nlTestSuite * apSuite,
                                                                                                             void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);

    uint16_t mockCommandRef = 1;
    pendingResponseTracker.Add(mockCommandRef);
    commandSender.mFinishedCommandCount = 1;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    uint16_t invalidResponseCommandRef = 2;
    GenerateInvokeResponse(apSuite, apContext, buf, kTestCommandIdWithData, kTestClusterId, kTestEndpointId,
                           MakeOptional(invalidResponseCommandRef));
    bool moreChunkedMessages = false;
    err                      = commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_KEY_NOT_FOUND);
    NL_TEST_ASSERT(apSuite, moreChunkedMessages == false);

    commandSender.FlushNoCommandResponse();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderExtendedDelegate.onResponseCalledTimes == 0 &&
                       mockCommandSenderExtendedDelegate.onFinalCalledTimes == 0 &&
                       mockCommandSenderExtendedDelegate.onNoResponseCalledTimes == 1 &&
                       mockCommandSenderExtendedDelegate.onErrorCalledTimes == 0);
}

void TestCommandInteraction::ValidateCommandHandlerEncodeInvokeResponseMessage(nlTestSuite * apSuite, void * apContext,
                                                                               bool aNeedStatusCode)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandlerWithOutstandingCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);

    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mResponseSender.SetExchangeContext(exchange);

    AddInvokeResponseData(apSuite, apContext, &commandHandler, aNeedStatusCode);
    err = commandHandler.FinalizeLastInvokeResponseMessage();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message. Since that isn't the case in this artificial setup here
    // (where we created a responder exchange that's not responding to anything), we need to explicitly close it out. This is not
    // expected in normal application logic.
    //
    exchange->Close();
}

void TestCommandInteraction::TestCommandHandlerEncodeSimpleCommandData(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple command data and command path
    ValidateCommandHandlerEncodeInvokeResponseMessage(apSuite, apContext, false /*aNeedStatusCode=false*/);
}

struct Fields
{
    static constexpr chip::CommandId GetCommandId() { return 4; }
    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
    {
        TLV::TLVType outerContainerType;
        ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, outerContainerType));
        ReturnErrorOnFailure(aWriter.PutBoolean(TLV::ContextTag(1), true));
        return aWriter.EndContainer(outerContainerType);
    }
};

struct BadFields
{
    static constexpr chip::CommandId GetCommandId() { return 4; }
    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
    {
        TLV::TLVType outerContainerType;
        uint8_t data[36] = { 0 };
        ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, outerContainerType));
        // Just encode something bad to return a failure state here.
        for (uint8_t i = 1; i < UINT8_MAX; i++)
        {
            ReturnErrorOnFailure(app::DataModel::Encode(aWriter, TLV::ContextTag(i), ByteSpan(data)));
        }
        return aWriter.EndContainer(outerContainerType);
    }
};

void TestCommandInteraction::TestCommandHandlerCommandDataEncoding(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    auto path               = MakeTestCommandPath();
    auto requestCommandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    CommandHandlerWithOutstandingCommand commandHandler(nullptr, requestCommandPath, /* aRef = */ NullOptional);

    commandHandler.AddResponse(requestCommandPath, Fields());
    err = commandHandler.FinalizeLastInvokeResponseMessage();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::TestCommandHandlerCommandEncodeFailure(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    auto path               = MakeTestCommandPath();
    auto requestCommandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    CommandHandlerWithOutstandingCommand commandHandler(nullptr, requestCommandPath, NullOptional);

    commandHandler.AddResponse(requestCommandPath, BadFields());
    err = commandHandler.FinalizeLastInvokeResponseMessage();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

/**
 * Helper macro we can use to pretend we got a reply from the server in cases
 * when the reply was actually dropped due to us not wanting the client's state
 * machine to advance.
 *
 * When this macro is used, the client has sent a message and is waiting for an
 * ack+response, and the server has sent a response that got dropped and is
 * waiting for an ack (and maybe a response).
 *
 * What this macro then needs to do is:
 *
 * 1. Pretend that the client got an ack (and clear out the corresponding ack
 *    state).
 * 2. Pretend that the client got a message from the server, with the id of the
 *    message that was dropped, which requires an ack, so the client will send
 *    that ack in its next message.
 *
 * This is a macro so we get useful line numbers on assertion failures
 */
#define PretendWeGotReplyFromServer(aSuite, aContext, aClientExchange)                                                             \
    {                                                                                                                              \
        Messaging::ReliableMessageMgr * localRm    = (aContext).GetExchangeManager().GetReliableMessageMgr();                      \
        Messaging::ExchangeContext * localExchange = aClientExchange;                                                              \
        NL_TEST_ASSERT(aSuite, localRm->TestGetCountRetransTable() == 2);                                                          \
                                                                                                                                   \
        localRm->ClearRetransTable(localExchange);                                                                                 \
        NL_TEST_ASSERT(aSuite, localRm->TestGetCountRetransTable() == 1);                                                          \
                                                                                                                                   \
        localRm->EnumerateRetransTable([localExchange](auto * entry) {                                                             \
            localExchange->SetPendingPeerAckMessageCounter(entry->retainedBuf.GetMessageCounter());                                \
            return Loop::Break;                                                                                                    \
        });                                                                                                                        \
    }

// Command Sender sends invoke request, command handler drops invoke response, then test injects status response message with
// busy to client, client sends out a status response with invalid action.
void TestCommandInteraction::TestCommandInvalidMessage1(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    asyncCommand = false;

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err                                                 = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 0 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    StatusResponseMessage::Builder response;
    response.Init(&writer);
    response.Status(Protocols::InteractionModel::Status::Busy);
    NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);
    Test::MessageCapturer messageLog(ctx);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(apSuite, ctx, commandSender.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;

    err = commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_IM_GLOBAL_STATUS(Busy));
    NL_TEST_ASSERT(apSuite, mockCommandSenderDelegate.mError == CHIP_IM_GLOBAL_STATUS(Busy));
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);
    NL_TEST_ASSERT(apSuite, commandSender.GetInvokeResponseMessageCount() == 0);

    ctx.DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    CheckForInvalidAction(apSuite, messageLog);
    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects unknown message to client,
// client sends out status response with invalid action.
void TestCommandInteraction::TestCommandInvalidMessage2(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    asyncCommand = false;

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err                                                 = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 0 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    ReportDataMessage::Builder response;
    response.Init(&writer);
    NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);
    Test::MessageCapturer messageLog(ctx);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(apSuite, ctx, commandSender.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;

    err = commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INVALID_MESSAGE_TYPE);
    NL_TEST_ASSERT(apSuite, mockCommandSenderDelegate.mError == CHIP_ERROR_INVALID_MESSAGE_TYPE);
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);

    ctx.DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    CheckForInvalidAction(apSuite, messageLog);
    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects malformed invoke response
// message to client, client sends out status response with invalid action.
void TestCommandInteraction::TestCommandInvalidMessage3(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    asyncCommand = false;

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err                                                 = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 0 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    InvokeResponseMessage::Builder response;
    response.Init(&writer);
    NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandResponse);
    Test::MessageCapturer messageLog(ctx);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(apSuite, ctx, commandSender.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;

    err = commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_END_OF_TLV);
    NL_TEST_ASSERT(apSuite, mockCommandSenderDelegate.mError == CHIP_ERROR_END_OF_TLV);
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);

    ctx.DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    CheckForInvalidAction(apSuite, messageLog);
    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects malformed status response to
// client, client responds to the status response with invalid action.
void TestCommandInteraction::TestCommandInvalidMessage4(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    asyncCommand = false;

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err                                                 = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 0 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    StatusResponseMessage::Builder response;
    response.Init(&writer);
    NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);
    Test::MessageCapturer messageLog(ctx);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(apSuite, ctx, commandSender.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;

    err = commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_END_OF_TLV);
    NL_TEST_ASSERT(apSuite, mockCommandSenderDelegate.mError == CHIP_ERROR_END_OF_TLV);
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);

    ctx.DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    CheckForInvalidAction(apSuite, messageLog);
    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Command Sender sends malformed invoke request, handler fails to process it and sends status report with invalid action
void TestCommandInteraction::TestCommandHandlerInvalidMessageSync(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    chip::isCommandDispatched = false;
    AddInvalidInvokeRequestData(apSuite, apContext, &commandSender);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !chip::isCommandDispatched);
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);
    NL_TEST_ASSERT(apSuite, mockCommandSenderDelegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandHandlerCommandEncodeExternalFailure(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    auto path               = MakeTestCommandPath();
    auto requestCommandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    CommandHandlerWithOutstandingCommand commandHandler(nullptr, requestCommandPath, NullOptional);

    err = commandHandler.AddResponseData(requestCommandPath, BadFields());
    NL_TEST_ASSERT(apSuite, err != CHIP_NO_ERROR);
    commandHandler.AddStatus(requestCommandPath, Protocols::InteractionModel::Status::Failure);
    err = commandHandler.FinalizeLastInvokeResponseMessage();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::TestCommandHandlerEncodeSimpleStatusCode(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple status code and command path
    ValidateCommandHandlerEncodeInvokeResponseMessage(apSuite, apContext, true /*aNeedStatusCode=true*/);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    TestExchangeDelegate delegate;
    commandHandler.mResponseSender.SetExchangeContext(ctx.NewExchangeToAlice(&delegate));
    // Use some invalid endpoint / cluster / command.
    GenerateInvokeRequest(apSuite, apContext, commandDatabuf, /* aIsTimedRequest = */ false, 0xEF /* command */,
                          0xADBE /* cluster */, 0xDE /* endpoint */);

    // TODO: Need to find a way to get the response instead of only check if a function on key path is called.
    // We should not reach CommandDispatch if requested command does not exist.
    chip::isCommandDispatched = false;
    commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), false);
    NL_TEST_ASSERT(apSuite, !chip::isCommandDispatched);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedEmptyDataMsg(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    bool allBooleans[] = { true, false };
    for (auto messageIsTimed : allBooleans)
    {
        for (auto transactionIsTimed : allBooleans)
        {
            app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
            System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

            TestExchangeDelegate delegate;
            auto exchange = ctx.NewExchangeToAlice(&delegate, false);
            commandHandler.mResponseSender.SetExchangeContext(exchange);

            chip::isCommandDispatched = false;
            GenerateInvokeRequest(apSuite, apContext, commandDatabuf, messageIsTimed, kTestCommandIdNoData);
            Protocols::InteractionModel::Status status =
                commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), transactionIsTimed);
            if (messageIsTimed != transactionIsTimed)
            {
                NL_TEST_ASSERT(apSuite, status == Protocols::InteractionModel::Status::TimedRequestMismatch);
            }
            else
            {
                NL_TEST_ASSERT(apSuite, status == Protocols::InteractionModel::Status::Success);
            }
            NL_TEST_ASSERT(apSuite, chip::isCommandDispatched == (messageIsTimed == transactionIsTimed));

            //
            // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
            // OnMessageReceived callback and not having sent a subsequent message (as is the case when calling ProcessInvokeRequest
            // above, which doesn't actually send back a response in these cases). Since that isn't the case in this artificial
            // setup here (where we created a responder exchange that's not responding to anything), we need to explicitly close it
            // out. This is not expected in normal application logic.
            //
            exchange->Close();
        }
    }
}

void TestCommandInteraction::TestCommandSenderLegacyCallbackUnsupportedCommand(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender, kTestNonExistCommandId);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Because UnsupportedCommand is a path specific error we will expect it to come via on response when using Extended Path.
void TestCommandInteraction::TestCommandSenderExtendableCallbackUnsupportedCommand(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderExtendedDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender, kTestNonExistCommandId);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderExtendedDelegate.onResponseCalledTimes == 1 &&
                       mockCommandSenderExtendedDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderExtendedDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderLegacyCallbackBuildingBatchCommandFails(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;
    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(0);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(0);

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    err = commandSender.SetCommandSenderConfig(config);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    // Even though we got an error saying invalid argument we are going to attempt
    // to add two commands.

    auto commandPathParams = MakeTestCommandPath();
    err                    = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    err                           = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(1);
    err = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderExtendableCallbackBuildingBatchDuplicateCommandRefFails(nlTestSuite * apSuite,
                                                                                                      void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    err = commandSender.SetCommandSenderConfig(config);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(0);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(0);
    auto commandPathParams = MakeTestCommandPath();
    err                    = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    err                           = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(0);
    err = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderExtendableCallbackBuildingBatchCommandSuccess(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    err = commandSender.SetCommandSenderConfig(config);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // The specific values chosen here are arbitrary. This test primarily verifies that we can
    // use a larger command reference value followed by a smaller one for subsequent command.
    uint16_t firstCommandRef  = 40;
    uint16_t secondCommandRef = 2;
    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(firstCommandRef);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(firstCommandRef);
    auto commandPathParams = MakeTestCommandPath();
    err                    = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    err                           = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(secondCommandRef);
    finishCommandParams.SetCommandRef(secondCommandRef);
    err = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    writer = commandSender.GetCommandDataIBTLVWriter();
    err    = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderCommandSuccessResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, commandSender.GetInvokeResponseMessageCount() == 0);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 1 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);
    NL_TEST_ASSERT(apSuite, commandSender.GetInvokeResponseMessageCount() == 1);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderCommandAsyncSuccessResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    asyncCommand = true;
    err          = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 0 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 1);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 2);

    // Decrease CommandHandler refcount and send response
    asyncCommandHandle = nullptr;

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 1 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderCommandSpecificResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender, kTestCommandIdCommandSpecificResponse);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 1 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderCommandFailureResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender, kTestNonExistCommandId);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderAbruptDestruction(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    //
    // Don't send back a response, just keep the CommandHandler
    // hanging to give us enough time to do what we want with the CommandSender object.
    //
    sendResponse = false;

    mockCommandSenderDelegate.ResetCounter();

    {
        app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

        AddInvokeRequestData(apSuite, apContext, &commandSender, kTestCommandIdCommandSpecificResponse);
        err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        //
        // No callbacks should be invoked yet - let's validate that.
        //
        NL_TEST_ASSERT(apSuite,
                       mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 0 &&
                           mockCommandSenderDelegate.onErrorCalledTimes == 0);

        NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);
    }

    //
    // Upon the sender being destructed by the application, our exchange should get cleaned up too.
    //
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
}

void TestCommandInteraction::TestCommandHandlerRejectMultipleIdenticalCommands(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    isCommandDispatched = false;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.SetCommandSenderConfig(configParameters));

    // Command ID is not important here, since the command handler should reject the commands without handling it.
    auto commandPathParams = MakeTestCommandPath(kTestCommandIdCommandSpecificResponse);

    for (uint16_t i = 0; i < 2; i++)
    {
        app::CommandSender::PrepareCommandParameters prepareCommandParams;
        prepareCommandParams.SetStartDataStruct(true);
        prepareCommandParams.SetCommandRef(i);
        NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.PrepareCommand(commandPathParams, prepareCommandParams));
        NL_TEST_ASSERT(apSuite,
                       CHIP_NO_ERROR == commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));

        app::CommandSender::FinishCommandParameters finishCommandParams;
        finishCommandParams.SetEndDataStruct(true);
        finishCommandParams.SetCommandRef(i);
        NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.FinishCommand(finishCommandParams));
    }

    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderExtendedDelegate.onResponseCalledTimes == 0 &&
                       mockCommandSenderExtendedDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderExtendedDelegate.onErrorCalledTimes == 1);
    NL_TEST_ASSERT(apSuite, !chip::isCommandDispatched);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

void TestCommandInteraction::TestCommandHandlerRejectsMultipleCommandsWithIdenticalCommandRef(nlTestSuite * apSuite,
                                                                                              void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    isCommandDispatched = false;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.SetCommandSenderConfig(configParameters));

    uint16_t numberOfCommandsToSend = 2;
    {
        CommandPathParams requestCommandPaths[] = {
            MakeTestCommandPath(kTestCommandIdWithData),
            MakeTestCommandPath(kTestCommandIdCommandSpecificResponse),
        };

        uint16_t hardcodedCommandRef = 0;

        for (uint16_t i = 0; i < numberOfCommandsToSend; i++)
        {
            app::CommandSender::PrepareCommandParameters prepareCommandParams;
            prepareCommandParams.SetStartDataStruct(true);
            prepareCommandParams.SetCommandRef(i);
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.PrepareCommand(requestCommandPaths[i], prepareCommandParams));
            NL_TEST_ASSERT(apSuite,
                           CHIP_NO_ERROR == commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));
            // TODO fix this comment
            // We are taking advantage of the fact that the commandRef was set into finishCommandParams during PrepareCommand
            // But setting it to a different value here, we are overriding what it was supposed to be.
            app::CommandSender::FinishCommandParameters finishCommandParams;
            finishCommandParams.SetEndDataStruct(true);
            finishCommandParams.SetCommandRef(hardcodedCommandRef);
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.TestOnlyFinishCommand(finishCommandParams));
        }
    }

    BasicCommandPathRegistry<4> mBasicCommandPathRegistry;
    CommandHandler commandHandler(kCommandHandlerTestOnlyMarker, &mockCommandHandlerDelegate, &mBasicCommandPathRegistry);
    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mResponseSender.SetExchangeContext(exchange);

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    err = commandSender.Finalize(commandDatabuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    mockCommandHandlerDelegate.ResetCounter();
    commandDispatchedCount = 0;

    InteractionModel::Status status = commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), false);
    NL_TEST_ASSERT(apSuite, status == InteractionModel::Status::InvalidAction);

    NL_TEST_ASSERT(apSuite, commandDispatchedCount == 0);

    //
    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message (as is the case when calling ProcessInvokeRequest
    // above, which doesn't actually send back a response in these cases). Since that isn't the case in this artificial
    // setup here (where we created a responder exchange that's not responding to anything), we need to explicitly close it
    // out. This is not expected in normal application logic.
    //
    exchange->Close();
}

#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

void TestCommandInteraction::TestCommandHandlerRejectMultipleCommandsWhenHandlerOnlySupportsOne(nlTestSuite * apSuite,
                                                                                                void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    isCommandDispatched = false;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.SetCommandSenderConfig(configParameters));

    uint16_t numberOfCommandsToSend = 2;
    {
        CommandPathParams requestCommandPaths[] = {
            MakeTestCommandPath(kTestCommandIdWithData),
            MakeTestCommandPath(kTestCommandIdCommandSpecificResponse),
        };

        for (uint16_t i = 0; i < numberOfCommandsToSend; i++)
        {
            app::CommandSender::PrepareCommandParameters prepareCommandParams;
            prepareCommandParams.SetStartDataStruct(true);
            prepareCommandParams.SetCommandRef(i);
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.PrepareCommand(requestCommandPaths[i], prepareCommandParams));
            NL_TEST_ASSERT(apSuite,
                           CHIP_NO_ERROR == commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));
            app::CommandSender::FinishCommandParameters finishCommandParams;
            finishCommandParams.SetEndDataStruct(true);
            finishCommandParams.SetCommandRef(i);
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.FinishCommand(finishCommandParams));
        }
    }

    CommandHandler commandHandler(&mockCommandHandlerDelegate);
    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mResponseSender.SetExchangeContext(exchange);

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    err = commandSender.Finalize(commandDatabuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    mockCommandHandlerDelegate.ResetCounter();
    sendResponse           = true;
    commandDispatchedCount = 0;

    InteractionModel::Status status = commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), false);
    NL_TEST_ASSERT(apSuite, status == InteractionModel::Status::InvalidAction);

    NL_TEST_ASSERT(apSuite, commandDispatchedCount == 0);

    //
    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message (as is the case when calling ProcessInvokeRequest
    // above, which doesn't actually send back a response in these cases). Since that isn't the case in this artificial
    // setup here (where we created a responder exchange that's not responding to anything), we need to explicitly close it
    // out. This is not expected in normal application logic.
    //
    exchange->Close();
}

void TestCommandInteraction::TestCommandHandlerAcceptMultipleCommands(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    isCommandDispatched = false;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &ctx.GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.SetCommandSenderConfig(configParameters));

    uint16_t numberOfCommandsToSend = 2;
    {
        CommandPathParams requestCommandPaths[] = {
            MakeTestCommandPath(kTestCommandIdWithData),
            MakeTestCommandPath(kTestCommandIdCommandSpecificResponse),
        };

        for (uint16_t i = 0; i < numberOfCommandsToSend; i++)
        {
            app::CommandSender::PrepareCommandParameters prepareCommandParams;
            prepareCommandParams.SetStartDataStruct(true);
            prepareCommandParams.SetCommandRef(i);
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.PrepareCommand(requestCommandPaths[i], prepareCommandParams));
            NL_TEST_ASSERT(apSuite,
                           CHIP_NO_ERROR == commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));
            app::CommandSender::FinishCommandParameters finishCommandParams;
            finishCommandParams.SetEndDataStruct(true);
            finishCommandParams.SetCommandRef(i);
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.FinishCommand(finishCommandParams));
        }

        commandSender.MoveToState(app::CommandSender::State::AddedCommand);
    }

    BasicCommandPathRegistry<4> mBasicCommandPathRegistry;
    CommandHandler commandHandler(kCommandHandlerTestOnlyMarker, &mockCommandHandlerDelegate, &mBasicCommandPathRegistry);
    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mResponseSender.SetExchangeContext(exchange);

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    err = commandSender.Finalize(commandDatabuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    sendResponse = true;
    mockCommandHandlerDelegate.ResetCounter();
    commandDispatchedCount = 0;

    InteractionModel::Status status = commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), false);
    NL_TEST_ASSERT(apSuite, status == InteractionModel::Status::Success);

    NL_TEST_ASSERT(apSuite, commandDispatchedCount == 2);

    //
    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message (as is the case when calling ProcessInvokeRequest
    // above, which doesn't actually send back a response in these cases). Since that isn't the case in this artificial
    // setup here (where we created a responder exchange that's not responding to anything), we need to explicitly close it
    // out. This is not expected in normal application logic.
    //
    exchange->Close();
}

void TestCommandInteraction::TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsStatusResponse(
    nlTestSuite * apSuite, void * apContext)
{
    BasicCommandPathRegistry<4> mBasicCommandPathRegistry;
    CommandHandler commandHandler(kCommandHandlerTestOnlyMarker, &mockCommandHandlerDelegate, &mBasicCommandPathRegistry);
    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = mBasicCommandPathRegistry.Add(requestCommandPath1, MakeOptional<uint16_t>(static_cast<uint16_t>(1)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = mBasicCommandPathRegistry.Add(requestCommandPath2, MakeOptional<uint16_t>(static_cast<uint16_t>(2)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(apSuite, &commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    NL_TEST_ASSERT(apSuite, remainingSize == sizeToLeave);

    AddInvokeResponseData(apSuite, apContext, &commandHandler, /* aNeedStatusCode = */ true, kTestCommandIdCommandSpecificResponse,
                          kTestCommandIdCommandSpecificResponse);

    remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    NL_TEST_ASSERT(apSuite, remainingSize > sizeToLeave);
}

void TestCommandInteraction::TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponsePrimative(
    nlTestSuite * apSuite, void * apContext)
{
    BasicCommandPathRegistry<4> mBasicCommandPathRegistry;
    CommandHandler commandHandler(kCommandHandlerTestOnlyMarker, &mockCommandHandlerDelegate, &mBasicCommandPathRegistry);
    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = mBasicCommandPathRegistry.Add(requestCommandPath1, MakeOptional<uint16_t>(static_cast<uint16_t>(1)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = mBasicCommandPathRegistry.Add(requestCommandPath2, MakeOptional<uint16_t>(static_cast<uint16_t>(2)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(apSuite, &commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    NL_TEST_ASSERT(apSuite, remainingSize == sizeToLeave);

    AddInvokeResponseData(apSuite, apContext, &commandHandler, /* aNeedStatusCode = */ false, kTestCommandIdCommandSpecificResponse,
                          kTestCommandIdCommandSpecificResponse);

    remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    NL_TEST_ASSERT(apSuite, remainingSize > sizeToLeave);
}

void TestCommandInteraction::TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponse(nlTestSuite * apSuite,
                                                                                                             void * apContext)
{
    BasicCommandPathRegistry<4> mBasicCommandPathRegistry;
    CommandHandler commandHandler(kCommandHandlerTestOnlyMarker, &mockCommandHandlerDelegate, &mBasicCommandPathRegistry);
    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = mBasicCommandPathRegistry.Add(requestCommandPath1, MakeOptional<uint16_t>(static_cast<uint16_t>(1)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = mBasicCommandPathRegistry.Add(requestCommandPath2, MakeOptional<uint16_t>(static_cast<uint16_t>(2)));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(apSuite, &commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    NL_TEST_ASSERT(apSuite, remainingSize == sizeToLeave);

    uint32_t sizeToFill = 50;
    err                 = commandHandler.AddResponseData(requestCommandPath2, ForcedSizeBuffer(sizeToFill));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    NL_TEST_ASSERT(apSuite, remainingSize > sizeToLeave);
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
//
// This test needs a special unit-test only API being exposed in ExchangeContext to be able to correctly simulate
// the release of a session on the exchange.
//
void TestCommandInteraction::TestCommandHandlerReleaseWithExchangeClosed(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    asyncCommandHandle = nullptr;
    asyncCommand       = true;
    err                = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    // Verify that async command handle has been allocated
    NL_TEST_ASSERT(apSuite, asyncCommandHandle.Get() != nullptr);

    // Mimic closure of the exchange that would happen on a session release and verify that releasing the handle there-after
    // is handled gracefully.
    asyncCommandHandle.Get()->mResponseSender.GetExchangeContext()->GetSessionHolder().Release();
    asyncCommandHandle.Get()->mResponseSender.GetExchangeContext()->OnSessionReleased();

    asyncCommandHandle = nullptr;
}
#endif

} // namespace app
} // namespace chip

namespace {
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestCommandInvalidMessage1", chip::app::TestCommandInteraction::TestCommandInvalidMessage1),
    NL_TEST_DEF("TestCommandInvalidMessage2", chip::app::TestCommandInteraction::TestCommandInvalidMessage2),
    NL_TEST_DEF("TestCommandInvalidMessage3", chip::app::TestCommandInteraction::TestCommandInvalidMessage3),
    NL_TEST_DEF("TestCommandInvalidMessage4", chip::app::TestCommandInteraction::TestCommandInvalidMessage4),
    NL_TEST_DEF("TestCommandSenderWithWrongState", chip::app::TestCommandInteraction::TestCommandSenderWithWrongState),
    NL_TEST_DEF("TestCommandHandlerWithWrongState", chip::app::TestCommandInteraction::TestCommandHandlerWithWrongState),
    NL_TEST_DEF("TestCommandSenderWithSendCommand", chip::app::TestCommandInteraction::TestCommandSenderWithSendCommand),
    NL_TEST_DEF("TestCommandHandlerWithSendEmptyCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand),
    NL_TEST_DEF("TestCommandSenderWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg),
    NL_TEST_DEF("TestCommandSenderExtendableApiWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandSenderExtendableApiWithProcessReceivedMsg),
    NL_TEST_DEF("TestCommandSenderExtendableApiWithProcessReceivedMsgContainingInvalidCommandRef", chip::app::TestCommandInteraction::TestCommandSenderExtendableApiWithProcessReceivedMsgContainingInvalidCommandRef),
    NL_TEST_DEF("TestCommandHandlerEncodeSimpleCommandData", chip::app::TestCommandInteraction::TestCommandHandlerEncodeSimpleCommandData),
    NL_TEST_DEF("TestCommandHandlerCommandDataEncoding", chip::app::TestCommandInteraction::TestCommandHandlerCommandDataEncoding),
    NL_TEST_DEF("TestCommandHandlerCommandEncodeFailure", chip::app::TestCommandInteraction::TestCommandHandlerCommandEncodeFailure),
    NL_TEST_DEF("TestCommandHandlerCommandEncodeExternalFailure", chip::app::TestCommandInteraction::TestCommandHandlerCommandEncodeExternalFailure),
    NL_TEST_DEF("TestCommandHandlerEncodeSimpleStatusCode", chip::app::TestCommandInteraction::TestCommandHandlerEncodeSimpleStatusCode),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedNotExistCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedEmptyDataMsg", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedEmptyDataMsg),
    NL_TEST_DEF("TestCommandHandlerRejectMultipleIdenticalCommands", chip::app::TestCommandInteraction::TestCommandHandlerRejectMultipleIdenticalCommands),
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    NL_TEST_DEF("TestCommandHandlerRejectsMultipleCommandsWithIdenticalCommandRef", chip::app::TestCommandInteraction::TestCommandHandlerRejectsMultipleCommandsWithIdenticalCommandRef),
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
    NL_TEST_DEF("TestCommandHandlerRejectMultipleCommandsWhenHandlerOnlySupportsOne", chip::app::TestCommandInteraction::TestCommandHandlerRejectMultipleCommandsWhenHandlerOnlySupportsOne),
    NL_TEST_DEF("TestCommandHandlerAcceptMultipleCommands", chip::app::TestCommandInteraction::TestCommandHandlerAcceptMultipleCommands),
    NL_TEST_DEF("TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsStatusResponse", chip::app::TestCommandInteraction::TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsStatusResponse),
    NL_TEST_DEF("TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponsePrimative", chip::app::TestCommandInteraction::TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponsePrimative),
    NL_TEST_DEF("TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponse", chip::app::TestCommandInteraction::TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponse),


#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    NL_TEST_DEF("TestCommandHandlerReleaseWithExchangeClosed", chip::app::TestCommandInteraction::TestCommandHandlerReleaseWithExchangeClosed),
#endif
    NL_TEST_DEF("TestCommandSenderLegacyCallbackUnsupportedCommand", chip::app::TestCommandInteraction::TestCommandSenderLegacyCallbackUnsupportedCommand),
    NL_TEST_DEF("TestCommandSenderExtendableCallbackUnsupportedCommand", chip::app::TestCommandInteraction::TestCommandSenderExtendableCallbackUnsupportedCommand),
    NL_TEST_DEF("TestCommandSenderLegacyCallbackBuildingBatchCommandFails", chip::app::TestCommandInteraction::TestCommandSenderLegacyCallbackBuildingBatchCommandFails),
    NL_TEST_DEF("TestCommandSenderExtendableCallbackBuildingBatchDuplicateCommandRefFails", chip::app::TestCommandInteraction::TestCommandSenderExtendableCallbackBuildingBatchDuplicateCommandRefFails),
    NL_TEST_DEF("TestCommandSenderExtendableCallbackBuildingBatchCommandSuccess", chip::app::TestCommandInteraction::TestCommandSenderExtendableCallbackBuildingBatchCommandSuccess),
    NL_TEST_DEF("TestCommandSenderCommandSuccessResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandSuccessResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandAsyncSuccessResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandAsyncSuccessResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandSpecificResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandSpecificResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandFailureResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandFailureResponseFlow),
    NL_TEST_DEF("TestCommandSenderAbruptDestruction", chip::app::TestCommandInteraction::TestCommandSenderAbruptDestruction),
    NL_TEST_DEF("TestCommandHandlerInvalidMessageSync", chip::app::TestCommandInteraction::TestCommandHandlerInvalidMessageSync),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestCommandInteraction",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

} // namespace

int TestCommandInteraction()
{
    chip::gSuite = &sSuite;
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCommandInteraction)
