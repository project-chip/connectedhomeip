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
#include <optional>

#include <pw_unit_test/framework.h>

#include <app/AppConfig.h>
#include <app/CommandHandlerImpl.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model/Encode.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

using namespace chip::Protocols;

namespace {

void CheckForInvalidAction(chip::Test::MessageCapturer & messageLog)
{
    EXPECT_EQ(messageLog.MessageCount(), 1u);
    EXPECT_TRUE(messageLog.IsMessageType(0, chip::Protocols::InteractionModel::MsgType::StatusResponse));
    CHIP_ERROR status;
    EXPECT_EQ(chip::app::StatusResponse::ProcessStatusResponse(std::move(messageLog.MessagePayload(0)), status), CHIP_NO_ERROR);
    EXPECT_EQ(status, CHIP_IM_GLOBAL_STATUS(InvalidAction));
}

} // anonymous namespace

namespace chip {

namespace {
bool isCommandDispatched      = false;
size_t commandDispatchedCount = 0;

bool sendResponse = true;
bool asyncCommand = false;

constexpr EndpointId kTestEndpointId                      = 1;
constexpr ClusterId kTestClusterId                        = 3;
constexpr CommandId kTestCommandIdWithData                = 4;
constexpr CommandId kTestCommandIdNoData                  = 5;
constexpr CommandId kTestCommandIdCommandSpecificResponse = 6;
constexpr CommandId kTestCommandIdFillResponseMessage     = 7;
constexpr CommandId kTestNonExistCommandId                = 0;

const app::CommandSender::TestOnlyMarker kCommandSenderTestOnlyMarker;

class SimpleTLVPayload : public app::DataModel::EncodableToTLV
{
public:
    CHIP_ERROR EncodeTo(TLV::TLVWriter & aWriter, TLV::Tag aTag) const override
    {
        TLV::TLVType outerType;
        ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(aWriter.PutBoolean(chip::TLV::ContextTag(1), true));
        return aWriter.EndContainer(outerType);
    }
};

const chip::Test::MockNodeConfig & TestMockNodeConfig()
{
    using namespace chip::app;
    using namespace chip::Test;
    using namespace chip::app::Clusters::Globals::Attributes;

    // clang-format off
    static const MockNodeConfig config({
        MockEndpointConfig(chip::kTestEndpointId, {
            MockClusterConfig(Clusters::Identify::Id, {
                ClusterRevision::Id, FeatureMap::Id,
            },
            {},      // events
            {
                kTestCommandIdWithData,
                kTestCommandIdNoData,
                kTestCommandIdCommandSpecificResponse,
                kTestCommandIdFillResponseMessage,
            }, // accepted commands
            {} // generated commands
          ),
        }),
    });
    // clang-format on
    return config;
}

} // namespace

namespace app {

CommandHandler::Handle asyncCommandHandle;

enum class ForcedSizeBufferLengthHint
{
    kSizeBetween0and255,
    kSizeGreaterThan255,
};

class ForcedSizeBuffer : public app::DataModel::EncodableToTLV
{
public:
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
    CHIP_ERROR EncodeTo(TLV::TLVWriter & aWriter, TLV::Tag aTag) const override
    {
        VerifyOrReturnError(mBuffer, CHIP_ERROR_NO_MEMORY);

        TLV::TLVType outerContainerType;
        ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, outerContainerType));
        ReturnErrorOnFailure(app::DataModel::Encode(aWriter, TLV::ContextTag(1), ByteSpan(mBuffer.Get(), mBuffer.AllocatedSize())));
        return aWriter.EndContainer(outerContainerType);
    }

private:
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> mBuffer;
};

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

static Protocols::InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aRequestCommandPath)
{
    // Mock cluster catalog, only support commands on one cluster on one endpoint.
    if (aRequestCommandPath.mEndpointId != kTestEndpointId)
    {
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    if (aRequestCommandPath.mClusterId != kTestClusterId)
    {
        return Protocols::InteractionModel::Status::UnsupportedCluster;
    }

    if (aRequestCommandPath.mCommandId == kTestNonExistCommandId)
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }

    return Protocols::InteractionModel::Status::Success;
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
    EXPECT_EQ(aReader.EnterContainer(outerContainerType), CHIP_NO_ERROR);

    CHIP_ERROR err = aReader.Next();
    if (aRequestCommandPath.mCommandId == kTestCommandIdNoData)
    {
        EXPECT_EQ(err, CHIP_ERROR_END_OF_TLV);
    }
    else
    {
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(aReader.GetTag(), TLV::ContextTag(1));

        bool val;
        EXPECT_EQ(aReader.Get(val), CHIP_NO_ERROR);
        EXPECT_TRUE(val);
    }

    EXPECT_EQ(aReader.ExitContainer(outerContainerType), CHIP_NO_ERROR);

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
            SimpleTLVPayload payloadWriter;
            apCommandObj->AddResponse(aRequestCommandPath, aRequestCommandPath.mCommandId, payloadWriter);
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

class MockCommandResponder : public CommandHandlerExchangeInterface
{
public:
    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }
    void HandlingSlowCommand() override {}
    Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor(); }
    FabricIndex GetAccessingFabricIndex() const override { return kUndefinedFabricIndex; }

    Optional<GroupId> GetGroupId() const override { return NullOptional; }

    void AddInvokeResponseToSend(System::PacketBufferHandle && aPacket) override { mChunks.AddToEnd(std::move(aPacket)); }
    void ResponseDropped() override { mResponseDropped = true; }

    size_t GetCommandResponseMaxBufferSize() override { return kMaxSecureSduLengthBytes; }

    System::PacketBufferHandle mChunks;
    bool mResponseDropped = false;
};

class MockCommandHandlerCallback : public CommandHandlerImpl::Callback
{
public:
    void OnDone(CommandHandlerImpl & apCommandHandler) final { onFinalCalledTimes++; }
    void DispatchCommand(CommandHandlerImpl & apCommandObj, const ConcreteCommandPath & aCommandPath,
                         TLV::TLVReader & apPayload) final
    {
        DispatchSingleClusterCommand(aCommandPath, apPayload, &apCommandObj);
    }

    Protocols::InteractionModel::Status ValidateCommandCanBeDispatched(const DataModel::InvokeRequest & request) override
    {
        using Protocols::InteractionModel::Status;

        Status status = ServerClusterCommandExists(request.path);
        if (status != Status::Success)
        {
            return status;
        }

        // NOTE: IM does more validation here, however for now we do minimal options
        //       to pass the test.

        return Status::Success;
    }

    void ResetCounter() { onFinalCalledTimes = 0; }

    int onFinalCalledTimes = 0;
} mockCommandHandlerDelegate;

class TestCommandInteractionModel : public TestImCustomDataModel
{
public:
    static TestCommandInteractionModel * Instance()
    {
        static TestCommandInteractionModel instance;
        return &instance;
    }

    TestCommandInteractionModel() = default;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override
    {
        DispatchSingleClusterCommand(request.path, input_arguments, handler);
        return std::nullopt; // handler status is set by the dispatch
    }
};

class TestCommandInteraction : public chip::Test::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();
        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(TestCommandInteractionModel::Instance());
        chip::Test::SetMockNodeConfig(TestMockNodeConfig());
    }

    void TearDown() override
    {
        chip::Test::ResetMockNodeConfig();
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        AppContext::TearDown();
    }

    static size_t GetNumActiveCommandResponderObjects()
    {
        return chip::app::InteractionModelEngine::GetInstance()->mCommandResponderObjs.Allocated();
    }

    void TestCommandInvalidMessage1();
    void TestCommandInvalidMessage2();
    void TestCommandInvalidMessage3();
    void TestCommandInvalidMessage4();
    void TestCommandSender_WithSendCommand();
    void TestCommandSender_WithProcessReceivedMsg();
    void TestCommandSender_ExtendableApiWithProcessReceivedMsg();
    void TestCommandSender_ExtendableApiWithProcessReceivedMsgContainingInvalidCommandRef();
    void TestCommandSender_ValidateSecondLargeAddRequestDataRollbacked();
    void TestCommandHandler_WithoutResponderCallingAddStatus();
    void TestCommandHandler_WithoutResponderCallingAddResponse();
    void TestCommandHandler_WithoutResponderCallingDirectPrepareFinishCommandApis();
    void TestCommandHandler_RejectsMultipleCommandsWithIdenticalCommandRef();
    void TestCommandHandler_RejectMultipleCommandsWhenHandlerOnlySupportsOne();
    void TestCommandHandler_AcceptMultipleCommands();
    void TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsStatusResponse();
    void TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponsePrimative();
    void TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponse();
    void TestCommandHandler_ReleaseWithExchangeClosed();

    /**
     * With the introduction of batch invoke commands, CommandHandler keeps track of incoming
     * ConcreteCommandPath and the associated CommandRefs. These are normally populated
     * as part of OnInvokeCommandRequest from the incoming request. For some unit tests where
     * we want to test APIs that cluster code uses, we need to inject entries into the
     * CommandPathRegistry directly.
     */
    class CommandHandlerWithUnrespondedCommand : public app::CommandHandlerImpl
    {
    public:
        CommandHandlerWithUnrespondedCommand(CommandHandlerImpl::Callback * apCallback,
                                             const ConcreteCommandPath & aRequestCommandPath, const Optional<uint16_t> & aRef) :
            CommandHandlerImpl(apCallback)
        {
            GetCommandPathRegistry().Add(aRequestCommandPath, aRef.std_optional());
            SetExchangeInterface(&mMockCommandResponder);
        }
        MockCommandResponder mMockCommandResponder;

        using app::CommandHandler::AddResponse;
    };

    // Generate an invoke request.  If aCommandId is kTestCommandIdWithData, a
    // payload will be included.  Otherwise no payload will be included.
    static void GenerateInvokeRequest(System::PacketBufferHandle & aPayload, bool aIsTimedRequest, CommandId aCommandId,
                                      ClusterId aClusterId = kTestClusterId, EndpointId aEndpointId = kTestEndpointId);
    // Generate an invoke response.  If aCommandId is kTestCommandIdWithData, a
    // payload will be included.  Otherwise no payload will be included.
    static void GenerateInvokeResponse(System::PacketBufferHandle & aPayload, CommandId aCommandId,
                                       ClusterId aClusterId = kTestClusterId, EndpointId aEndpointId = kTestEndpointId,
                                       std::optional<uint16_t> aCommandRef = std::nullopt);
    static void AddInvokeRequestData(CommandSender * apCommandSender, CommandId aCommandId = kTestCommandIdWithData);
    static void AddInvalidInvokeRequestData(CommandSender * apCommandSender, CommandId aCommandId = kTestCommandIdWithData);
    static void AddInvokeResponseData(CommandHandler * apCommandHandler, bool aNeedStatusCode,
                                      CommandId aResponseCommandId = kTestCommandIdWithData,
                                      CommandId aRequestCommandId  = kTestCommandIdWithData);
    static uint32_t GetAddResponseDataOverheadSizeForPath(const ConcreteCommandPath & aRequestCommandPath,
                                                          ForcedSizeBufferLengthHint aBufferSizeHint);
    static void FillCurrentInvokeResponseBuffer(CommandHandlerImpl * apCommandHandler,
                                                const ConcreteCommandPath & aRequestCommandPath, uint32_t aSizeToLeaveInBuffer);
    static void ValidateCommandHandlerEncodeInvokeResponseMessage(bool aNeedStatusCode);

protected:
    chip::app::DataModel::Provider * mOldProvider = nullptr;
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

void TestCommandInteraction::GenerateInvokeRequest(System::PacketBufferHandle & aPayload, bool aIsTimedRequest,
                                                   CommandId aCommandId, ClusterId aClusterId, EndpointId aEndpointId)

{
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    EXPECT_EQ(invokeRequestMessageBuilder.Init(&writer), CHIP_NO_ERROR);

    invokeRequestMessageBuilder.SuppressResponse(true).TimedRequest(aIsTimedRequest);
    InvokeRequests::Builder & invokeRequests = invokeRequestMessageBuilder.CreateInvokeRequests();
    ASSERT_EQ(invokeRequestMessageBuilder.GetError(), CHIP_NO_ERROR);

    CommandDataIB::Builder & commandDataIBBuilder = invokeRequests.CreateCommandData();
    EXPECT_EQ(invokeRequests.GetError(), CHIP_NO_ERROR);

    CommandPathIB::Builder & commandPathBuilder = commandDataIBBuilder.CreatePath();
    EXPECT_EQ(commandDataIBBuilder.GetError(), CHIP_NO_ERROR);

    commandPathBuilder.EndpointId(aEndpointId).ClusterId(aClusterId).CommandId(aCommandId).EndOfCommandPathIB();
    EXPECT_EQ(commandPathBuilder.GetError(), CHIP_NO_ERROR);

    if (aCommandId == kTestCommandIdWithData)
    {
        chip::TLV::TLVWriter * pWriter = commandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        EXPECT_EQ(pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kFields)),
                                          chip::TLV::kTLVType_Structure, dummyType),
                  CHIP_NO_ERROR);

        EXPECT_EQ(pWriter->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);

        EXPECT_EQ(pWriter->EndContainer(dummyType), CHIP_NO_ERROR);
    }

    commandDataIBBuilder.EndOfCommandDataIB();
    EXPECT_EQ(commandDataIBBuilder.GetError(), CHIP_NO_ERROR);

    invokeRequests.EndOfInvokeRequests();
    EXPECT_EQ(invokeRequests.GetError(), CHIP_NO_ERROR);

    invokeRequestMessageBuilder.EndOfInvokeRequestMessage();
    ASSERT_EQ(invokeRequestMessageBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_EQ(writer.Finalize(&aPayload), CHIP_NO_ERROR);
}

void TestCommandInteraction::GenerateInvokeResponse(System::PacketBufferHandle & aPayload, CommandId aCommandId,
                                                    ClusterId aClusterId, EndpointId aEndpointId,
                                                    std::optional<uint16_t> aCommandRef)

{
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    EXPECT_EQ(invokeResponseMessageBuilder.Init(&writer), CHIP_NO_ERROR);

    invokeResponseMessageBuilder.SuppressResponse(true);
    InvokeResponseIBs::Builder & invokeResponses = invokeResponseMessageBuilder.CreateInvokeResponses();
    EXPECT_EQ(invokeResponseMessageBuilder.GetError(), CHIP_NO_ERROR);

    InvokeResponseIB::Builder & invokeResponseIBBuilder = invokeResponses.CreateInvokeResponse();
    EXPECT_EQ(invokeResponses.GetError(), CHIP_NO_ERROR);

    CommandDataIB::Builder & commandDataIBBuilder = invokeResponseIBBuilder.CreateCommand();
    EXPECT_EQ(commandDataIBBuilder.GetError(), CHIP_NO_ERROR);

    CommandPathIB::Builder & commandPathBuilder = commandDataIBBuilder.CreatePath();
    EXPECT_EQ(commandDataIBBuilder.GetError(), CHIP_NO_ERROR);

    commandPathBuilder.EndpointId(aEndpointId).ClusterId(aClusterId).CommandId(aCommandId).EndOfCommandPathIB();
    EXPECT_EQ(commandPathBuilder.GetError(), CHIP_NO_ERROR);

    if (aCommandId == kTestCommandIdWithData)
    {
        chip::TLV::TLVWriter * pWriter = commandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;

        EXPECT_EQ(pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kFields)),
                                          chip::TLV::kTLVType_Structure, dummyType),
                  CHIP_NO_ERROR);

        EXPECT_EQ(pWriter->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);

        EXPECT_EQ(pWriter->EndContainer(dummyType), CHIP_NO_ERROR);
    }

    if (aCommandRef.has_value())
    {
        EXPECT_EQ(commandDataIBBuilder.Ref(*aCommandRef), CHIP_NO_ERROR);
    }

    commandDataIBBuilder.EndOfCommandDataIB();
    EXPECT_EQ(commandDataIBBuilder.GetError(), CHIP_NO_ERROR);

    invokeResponseIBBuilder.EndOfInvokeResponseIB();
    EXPECT_EQ(invokeResponseIBBuilder.GetError(), CHIP_NO_ERROR);

    invokeResponses.EndOfInvokeResponses();
    EXPECT_EQ(invokeResponses.GetError(), CHIP_NO_ERROR);

    invokeResponseMessageBuilder.EndOfInvokeResponseMessage();
    EXPECT_EQ(invokeResponseMessageBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_EQ(writer.Finalize(&aPayload), CHIP_NO_ERROR);
}

void TestCommandInteraction::AddInvokeRequestData(CommandSender * apCommandSender, CommandId aCommandId)
{
    auto commandPathParams = MakeTestCommandPath(aCommandId);

    EXPECT_EQ(apCommandSender->PrepareCommand(commandPathParams), CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = apCommandSender->GetCommandDataIBTLVWriter();

    EXPECT_EQ(writer->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
    EXPECT_EQ(apCommandSender->FinishCommand(), CHIP_NO_ERROR);
}

void TestCommandInteraction::AddInvalidInvokeRequestData(CommandSender * apCommandSender, CommandId aCommandId)
{
    auto commandPathParams = MakeTestCommandPath(aCommandId);

    EXPECT_EQ(apCommandSender->PrepareCommand(commandPathParams), CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = apCommandSender->GetCommandDataIBTLVWriter();

    EXPECT_EQ(writer->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
    apCommandSender->MoveToState(CommandSender::State::AddedCommand);
}

void TestCommandInteraction::AddInvokeResponseData(CommandHandler * apCommandHandler, bool aNeedStatusCode,
                                                   CommandId aResponseCommandId, CommandId aRequestCommandId)
{
    constexpr EndpointId kTestEndpointId   = 1;
    constexpr ClusterId kTestClusterId     = 3;
    ConcreteCommandPath requestCommandPath = { kTestEndpointId, kTestClusterId, aRequestCommandId };
    if (aNeedStatusCode)
    {
        apCommandHandler->AddStatus(requestCommandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        SimpleTLVPayload payloadWriter;
        EXPECT_EQ(apCommandHandler->AddResponseData(requestCommandPath, aResponseCommandId, payloadWriter), CHIP_NO_ERROR);
    }
}

uint32_t TestCommandInteraction::GetAddResponseDataOverheadSizeForPath(const ConcreteCommandPath & aRequestCommandPath,
                                                                       ForcedSizeBufferLengthHint aBufferSizeHint)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandlerImpl::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandlerImpl commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);
    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);

    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2))),
              CHIP_NO_ERROR);

    EXPECT_EQ(commandHandler.AllocateBuffer(), CHIP_NO_ERROR);

    uint32_t remainingSizeBefore = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();

    // When ForcedSizeBuffer exceeds 255, an extra byte is needed for length, affecting the overhead size required by
    // AddResponseData. In order to have this accounted for in overhead calculation we set the length to be 256.
    uint32_t sizeOfForcedSizeBuffer = aBufferSizeHint == ForcedSizeBufferLengthHint::kSizeGreaterThan255 ? 256 : 0;
    ForcedSizeBuffer responseData(sizeOfForcedSizeBuffer);
    EXPECT_EQ(commandHandler.AddResponseData(aRequestCommandPath, responseData.GetCommandId(), responseData), CHIP_NO_ERROR);
    uint32_t remainingSizeAfter = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    uint32_t delta              = remainingSizeBefore - remainingSizeAfter - sizeOfForcedSizeBuffer;

    return delta;
}

void TestCommandInteraction::FillCurrentInvokeResponseBuffer(CommandHandlerImpl * apCommandHandler,
                                                             const ConcreteCommandPath & aRequestCommandPath,
                                                             uint32_t aSizeToLeaveInBuffer)
{
    EXPECT_EQ(apCommandHandler->AllocateBuffer(), CHIP_NO_ERROR);
    uint32_t remainingSize = apCommandHandler->mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();

    // AddResponseData's overhead calculation depends on the size of ForcedSizeBuffer. If the buffer exceeds 255 bytes, an extra
    // length byte is required. Since tests using FillCurrentInvokeResponseBuffer currently end up with sizeToFill > 255, we
    // inform the calculation of this expectation. Nonetheless, we also validate this assumption for correctness.
    ForcedSizeBufferLengthHint bufferSizeHint    = ForcedSizeBufferLengthHint::kSizeGreaterThan255;
    uint32_t overheadSizeNeededForAddingResponse = GetAddResponseDataOverheadSizeForPath(aRequestCommandPath, bufferSizeHint);
    EXPECT_GT(remainingSize, (aSizeToLeaveInBuffer + overheadSizeNeededForAddingResponse));
    uint32_t sizeToFill = remainingSize - aSizeToLeaveInBuffer - overheadSizeNeededForAddingResponse;

    // Validating assumption. If this fails, it means overheadSizeNeededForAddingResponse is likely too large.
    EXPECT_GE(sizeToFill, 256u);

    ForcedSizeBuffer responseData(sizeToFill);
    EXPECT_EQ(apCommandHandler->AddResponseData(aRequestCommandPath, responseData.GetCommandId(), responseData), CHIP_NO_ERROR);
}

void TestCommandInteraction::ValidateCommandHandlerEncodeInvokeResponseMessage(bool aNeedStatusCode)
{
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandlerWithUnrespondedCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        AddInvokeResponseData(&commandHandler, aNeedStatusCode);
    }
    EXPECT_FALSE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects status response message with
// busy to client, client sends out a status response with invalid action.
TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandInvalidMessage1)
{
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    ASSERT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    StatusResponseMessage::Builder response;
    response.Init(&writer);
    response.Status(Protocols::InteractionModel::Status::Busy);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);
    chip::Test::MessageCapturer messageLog(*this);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.

    PretendWeGotReplyFromServer(*this, commandSender.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;

    EXPECT_EQ(commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

    DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects unknown message to client,
// client sends out status response with invalid action.
TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandInvalidMessage2)
{
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    ASSERT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    ReportDataMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);
    chip::Test::MessageCapturer messageLog(*this);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.

    PretendWeGotReplyFromServer(*this, commandSender.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;

    EXPECT_EQ(commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_ERROR_INVALID_MESSAGE_TYPE);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_ERROR_INVALID_MESSAGE_TYPE);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

    DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects malformed invoke response
// message to client, client sends out status response with invalid action.
TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandInvalidMessage3)
{
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    ASSERT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    InvokeResponseMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandResponse);
    chip::Test::MessageCapturer messageLog(*this);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(*this, commandSender.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;

    EXPECT_EQ(commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

    DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects malformed status response to
// client, client responds to the status response with invalid action.
TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandInvalidMessage4)
{
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    ASSERT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    StatusResponseMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);
    chip::Test::MessageCapturer messageLog(*this);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(*this, commandSender.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;

    EXPECT_EQ(commandSender.OnMessageReceived(commandSender.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

    DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

TEST_F(TestCommandInteraction, TestCommandSender_WithWrongState)
{

    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestCommandInteraction, TestCommandHandler_WithWrongState)
{
    ConcreteCommandPath requestCommandPath  = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    CommandHandlerWithUnrespondedCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        const CommandHandlerImpl::InvokeResponseParameters prepareParams(requestCommandPath);
        EXPECT_EQ(commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams), CHIP_NO_ERROR);
    }
    EXPECT_TRUE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandSender_WithSendCommand)
{

    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AddInvokeRequestData(&commandSender);
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    GenerateInvokeResponse(buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    EXPECT_EQ(commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages), CHIP_NO_ERROR);
    EXPECT_FALSE(moreChunkedMessages);
}

TEST_F(TestCommandInteraction, TestCommandHandler_WithSendEmptyCommand)
{
    ConcreteCommandPath requestCommandPath  = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    CommandHandlerWithUnrespondedCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        const CommandHandlerImpl::InvokeResponseParameters prepareParams(requestCommandPath);
        EXPECT_EQ(commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams), CHIP_NO_ERROR);
        EXPECT_EQ(commandHandler.FinishCommand(), CHIP_NO_ERROR);
    }
    EXPECT_FALSE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandSender_WithProcessReceivedMsg)
{

    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateInvokeResponse(buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;

    EXPECT_EQ(commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages), CHIP_NO_ERROR);
    EXPECT_FALSE(moreChunkedMessages);
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandSender_ExtendableApiWithProcessReceivedMsg)
{

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);

    uint16_t mockCommandRef = 1;
    pendingResponseTracker.Add(mockCommandRef);
    commandSender.mFinishedCommandCount = 1;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateInvokeResponse(buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    EXPECT_EQ(commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages), CHIP_NO_ERROR);
    EXPECT_FALSE(moreChunkedMessages);

    commandSender.FlushNoCommandResponse();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onNoResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 0);
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandSender_ExtendableApiWithProcessReceivedMsgContainingInvalidCommandRef)
{

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);

    uint16_t mockCommandRef = 1;
    pendingResponseTracker.Add(mockCommandRef);

    commandSender.mFinishedCommandCount = 1;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    uint16_t invalidResponseCommandRef = 2;
    GenerateInvokeResponse(buf, kTestCommandIdWithData, kTestClusterId, kTestEndpointId,
                           std::make_optional(invalidResponseCommandRef));
    bool moreChunkedMessages = false;
    EXPECT_EQ(commandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages), CHIP_ERROR_KEY_NOT_FOUND);
    EXPECT_FALSE(moreChunkedMessages);

    commandSender.FlushNoCommandResponse();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onNoResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 0);
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandSender_ValidateSecondLargeAddRequestDataRollbacked)
{
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::AddRequestDataParameters addRequestDataParams;

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(commandSender.SetCommandSenderConfig(config), CHIP_NO_ERROR);

    // The specific values chosen here are arbitrary.
    uint16_t firstCommandRef  = 1;
    uint16_t secondCommandRef = 2;
    auto commandPathParams    = MakeTestCommandPath();
    SimpleTLVPayload simplePayloadWriter;
    addRequestDataParams.SetCommandRef(firstCommandRef);

    EXPECT_EQ(commandSender.AddRequestData(commandPathParams, simplePayloadWriter, addRequestDataParams), CHIP_NO_ERROR);

    uint32_t remainingSize = commandSender.mInvokeRequestBuilder.GetWriter()->GetRemainingFreeLength();
    // Because request is made of both request data and request path (commandPathParams), using
    // `remainingSize` is large enough fail.
    ForcedSizeBuffer requestData(remainingSize);

    addRequestDataParams.SetCommandRef(secondCommandRef);
    EXPECT_EQ(commandSender.AddRequestData(commandPathParams, requestData, addRequestDataParams), CHIP_ERROR_NO_MEMORY);

    // Confirm that we can still send out a request with the first command.
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 0);
}

TEST_F(TestCommandInteraction, TestCommandHandlerEncodeSimpleCommandData)
{
    // Send response which has simple command data and command path
    ValidateCommandHandlerEncodeInvokeResponseMessage(false /*aNeedStatusCode=false*/);
}

TEST_F(TestCommandInteraction, TestCommandHandlerCommandDataEncoding)
{
    auto path = MakeTestCommandPath();
    ConcreteCommandPath requestCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    CommandHandlerWithUnrespondedCommand commandHandler(nullptr, requestCommandPath, /* aRef = */ NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        commandHandler.AddResponse(requestCommandPath, Fields());
    }
    EXPECT_FALSE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

TEST_F(TestCommandInteraction, TestCommandHandlerCommandEncodeFailure)
{
    auto path               = MakeTestCommandPath();
    auto requestCommandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    CommandHandlerWithUnrespondedCommand commandHandler(nullptr, requestCommandPath, NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        commandHandler.AddResponse(requestCommandPath, BadFields());
    }
    EXPECT_FALSE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

// Command Sender sends malformed invoke request, handler fails to process it and sends status report with invalid action
TEST_F(TestCommandInteraction, TestCommandHandlerInvalidMessageSync)
{

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    chip::isCommandDispatched = false;
    AddInvalidInvokeRequestData(&commandSender);
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_FALSE(chip::isCommandDispatched);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandHandlerCommandEncodeExternalFailure)
{
    auto path               = MakeTestCommandPath();
    auto requestCommandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    CommandHandlerWithUnrespondedCommand commandHandler(nullptr, requestCommandPath, NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        EXPECT_NE(commandHandler.AddResponseData(requestCommandPath, BadFields()), CHIP_NO_ERROR);
        commandHandler.AddStatus(requestCommandPath, Protocols::InteractionModel::Status::Failure);
    }
    EXPECT_FALSE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

TEST_F(TestCommandInteraction, TestCommandHandlerEncodeSimpleStatusCode)
{
    // Send response which has simple status code and command path
    ValidateCommandHandlerEncodeInvokeResponseMessage(true /*aNeedStatusCode=true*/);
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_WithoutResponderCallingAddStatus)
{
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandlerImpl commandHandler(&mockCommandHandlerDelegate);

    commandHandler.AddStatus(requestCommandPath, Protocols::InteractionModel::Status::Failure);

    // Since calling AddStatus is supposed to be a no-operation when there is no responder, it is
    // hard to validate. Best way is to check that we are still in an Idle state afterwards
    EXPECT_TRUE(commandHandler.TestOnlyIsInIdleState());
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_WithoutResponderCallingAddResponse)
{
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandlerImpl commandHandler(&mockCommandHandlerDelegate);

    uint32_t sizeToFill = 50; // This is an arbitrary number, we need to select a non-zero value.
    ForcedSizeBuffer responseData(sizeToFill);
    EXPECT_EQ(commandHandler.AddResponseData(requestCommandPath, responseData.GetCommandId(), responseData), CHIP_NO_ERROR);

    // Since calling AddResponseData is supposed to be a no-operation when there is no responder, it is
    // hard to validate. Best way is to check that we are still in an Idle state afterwards
    EXPECT_TRUE(commandHandler.TestOnlyIsInIdleState());
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_WithoutResponderCallingDirectPrepareFinishCommandApis)
{
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandlerImpl commandHandler(&mockCommandHandlerDelegate);

    // We intentionally prevent successful calls to PrepareInvokeResponseCommand and FinishCommand when no
    // responder is present. This aligns with the design decision to promote AddStatus and AddResponseData
    // usage in such scenarios. See GitHub issue #32486 for discussions on phasing out external use of
    // these primitives.
    const CommandHandlerImpl::InvokeResponseParameters prepareParams(requestCommandPath);
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };
    EXPECT_EQ(commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams), CHIP_ERROR_INCORRECT_STATE);

    EXPECT_EQ(commandHandler.GetCommandDataIBTLVWriter(), nullptr);

    EXPECT_EQ(commandHandler.FinishCommand(), CHIP_ERROR_INCORRECT_STATE);

    EXPECT_TRUE(commandHandler.TestOnlyIsInIdleState());
}

TEST_F(TestCommandInteraction, TestCommandHandler_WithOnInvokeReceivedNotExistCommand)
{
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    // Use some invalid endpoint / cluster / command.
    GenerateInvokeRequest(commandDatabuf, /* aIsTimedRequest = */ false, 0xEF /* command */, 0xADBE /* cluster */,
                          0xDE /* endpoint */);
    CommandHandlerImpl commandHandler(&mockCommandHandlerDelegate);
    chip::isCommandDispatched = false;

    mockCommandHandlerDelegate.ResetCounter();
    MockCommandResponder mockCommandResponder;
    Protocols::InteractionModel::Status status =
        commandHandler.OnInvokeCommandRequest(mockCommandResponder, std::move(commandDatabuf), false);

    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidAction);
    EXPECT_TRUE(mockCommandResponder.mChunks.IsNull());
    // TODO we can further validate the response is what we expected.
    EXPECT_FALSE(chip::isCommandDispatched);
}

TEST_F(TestCommandInteraction, TestCommandHandler_WithOnInvokeReceivedEmptyDataMsg)
{
    bool allBooleans[] = { true, false };
    for (auto messageIsTimed : allBooleans)
    {
        for (auto transactionIsTimed : allBooleans)
        {
            mockCommandHandlerDelegate.ResetCounter();
            CommandHandlerImpl commandHandler(&mockCommandHandlerDelegate);
            System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

            chip::isCommandDispatched = false;
            GenerateInvokeRequest(commandDatabuf, messageIsTimed, kTestCommandIdNoData);
            MockCommandResponder mockCommandResponder;
            Protocols::InteractionModel::Status status =
                commandHandler.OnInvokeCommandRequest(mockCommandResponder, std::move(commandDatabuf), transactionIsTimed);

            if (messageIsTimed != transactionIsTimed)
            {
                EXPECT_EQ(status, Protocols::InteractionModel::Status::TimedRequestMismatch);
                EXPECT_TRUE(mockCommandResponder.mChunks.IsNull());
            }
            else
            {
                EXPECT_EQ(status, Protocols::InteractionModel::Status::Success);
                EXPECT_FALSE(mockCommandResponder.mChunks.IsNull());
            }
            EXPECT_TRUE(chip::isCommandDispatched == (messageIsTimed == transactionIsTimed));
        }
    }
}

TEST_F(TestCommandInteraction, TestCommandSenderLegacyCallbackUnsupportedCommand)
{

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestNonExistCommandId);
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Because UnsupportedCommand is a path specific error we will expect it to come via on response when using Extended Path.
TEST_F(TestCommandInteraction, TestCommandSender_ExtendableCallbackUnsupportedCommand)
{

    mockCommandSenderExtendedDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderExtendedDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestNonExistCommandId);
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderLegacyCallbackBuildingBatchCommandFails)
{
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;
    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(0);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(0);

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(commandSender.SetCommandSenderConfig(config), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    // Even though we got an error saying invalid argument we are going to attempt
    // to add two commands.

    auto commandPathParams = MakeTestCommandPath();
    EXPECT_EQ(commandSender.PrepareCommand(commandPathParams, prepareCommandParams), CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    EXPECT_EQ(writer->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
    EXPECT_EQ(commandSender.FinishCommand(finishCommandParams), CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(1);
    EXPECT_EQ(commandSender.PrepareCommand(commandPathParams, prepareCommandParams), CHIP_ERROR_INCORRECT_STATE);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSender_ExtendableCallbackBuildingBatchDuplicateCommandRefFails)
{
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(commandSender.SetCommandSenderConfig(config), CHIP_NO_ERROR);

    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(0);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(0);
    auto commandPathParams = MakeTestCommandPath();
    EXPECT_EQ(commandSender.PrepareCommand(commandPathParams, prepareCommandParams), CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    EXPECT_EQ(writer->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
    EXPECT_EQ(commandSender.FinishCommand(finishCommandParams), CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(0);
    EXPECT_EQ(commandSender.PrepareCommand(commandPathParams, prepareCommandParams), CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSender_ExtendableCallbackBuildingBatchCommandSuccess)
{
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(commandSender.SetCommandSenderConfig(config), CHIP_NO_ERROR);

    // The specific values chosen here are arbitrary. This test primarily verifies that we can
    // use a larger command reference value followed by a smaller one for subsequent command.
    uint16_t firstCommandRef  = 40;
    uint16_t secondCommandRef = 2;
    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(firstCommandRef);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(firstCommandRef);
    auto commandPathParams = MakeTestCommandPath();
    EXPECT_EQ(commandSender.PrepareCommand(commandPathParams, prepareCommandParams), CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    EXPECT_EQ(writer->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
    EXPECT_EQ(commandSender.FinishCommand(finishCommandParams), CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(secondCommandRef);
    finishCommandParams.SetCommandRef(secondCommandRef);
    EXPECT_EQ(commandSender.PrepareCommand(commandPathParams, prepareCommandParams), CHIP_NO_ERROR);
    writer = commandSender.GetCommandDataIBTLVWriter();
    EXPECT_EQ(writer->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
    EXPECT_EQ(commandSender.FinishCommand(finishCommandParams), CHIP_NO_ERROR);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandSuccessResponseFlow)
{

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandAsyncSuccessResponseFlow)
{

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = true;

    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 1u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 2u);

    // Decrease CommandHandler refcount and send response
    asyncCommandHandle = nullptr;

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, CommandSenderDeletedWhenResponseIsPending)
{

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender * commandSender = Platform::New<app::CommandSender>(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(commandSender);
    asyncCommand = true;

    EXPECT_EQ(commandSender->SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 1u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 2u);

    // This is NOT deleting CommandSender in one of the callbacks, so we are not violating
    // the API contract. CommandSender is deleted when no message is being processed which
    // is a time that deleting CommandSender is considered safe.
    Platform::Delete(commandSender);

    // Decrease CommandHandler refcount and send response
    asyncCommandHandle = nullptr;

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandSpecificResponseFlow)
{

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestCommandIdCommandSpecificResponse);
    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandFailureResponseFlow)
{

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestNonExistCommandId);

    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderAbruptDestruction)
{

    //
    // Don't send back a response, just keep the CommandHandler
    // hanging to give us enough time to do what we want with the CommandSender object.
    //
    sendResponse = false;

    mockCommandSenderDelegate.ResetCounter();

    {
        app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

        AddInvokeRequestData(&commandSender, kTestCommandIdCommandSpecificResponse);

        EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

        DrainAndServiceIO();

        //
        // No callbacks should be invoked yet - let's validate that.
        //
        EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 1u);
    }

    //
    // Upon the sender being destructed by the application, our exchange should get cleaned up too.
    //
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandHandler_RejectMultipleIdenticalCommands)
{

    isCommandDispatched = false;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(CHIP_NO_ERROR, commandSender.SetCommandSenderConfig(configParameters));

    // Command ID is not important here, since the command handler should reject the commands without handling it.
    auto commandPathParams = MakeTestCommandPath(kTestCommandIdCommandSpecificResponse);

    for (uint16_t i = 0; i < 2; i++)
    {
        app::CommandSender::PrepareCommandParameters prepareCommandParams;
        prepareCommandParams.SetStartDataStruct(true);
        prepareCommandParams.SetCommandRef(i);
        EXPECT_EQ(CHIP_NO_ERROR, commandSender.PrepareCommand(commandPathParams, prepareCommandParams));
        EXPECT_EQ(CHIP_NO_ERROR, commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));

        app::CommandSender::FinishCommandParameters finishCommandParams;
        finishCommandParams.SetEndDataStruct(true);
        finishCommandParams.SetCommandRef(i);
        EXPECT_EQ(CHIP_NO_ERROR, commandSender.FinishCommand(finishCommandParams));
    }

    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 1);
    EXPECT_FALSE(chip::isCommandDispatched);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_RejectsMultipleCommandsWithIdenticalCommandRef)
{

    isCommandDispatched = false;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(CHIP_NO_ERROR, commandSender.SetCommandSenderConfig(configParameters));

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
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.PrepareCommand(requestCommandPaths[i], prepareCommandParams));
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));
            // TODO fix this comment
            // We are taking advantage of the fact that the commandRef was set into finishCommandParams during PrepareCommand
            // But setting it to a different value here, we are overriding what it was supposed to be.
            app::CommandSender::FinishCommandParameters finishCommandParams;
            finishCommandParams.SetEndDataStruct(true);
            finishCommandParams.SetCommandRef(hardcodedCommandRef);
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.TestOnlyFinishCommand(finishCommandParams));
        }
    }

    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandlerImpl::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandlerImpl commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    EXPECT_EQ(commandSender.Finalize(commandDatabuf), CHIP_NO_ERROR);

    mockCommandHandlerDelegate.ResetCounter();
    commandDispatchedCount = 0;

    Protocols::InteractionModel::Status status = commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), false);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidAction);

    EXPECT_EQ(commandDispatchedCount, 0u);
}

#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_RejectMultipleCommandsWhenHandlerOnlySupportsOne)
{

    isCommandDispatched = false;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(CHIP_NO_ERROR, commandSender.SetCommandSenderConfig(configParameters));

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
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.PrepareCommand(requestCommandPaths[i], prepareCommandParams));
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));
            app::CommandSender::FinishCommandParameters finishCommandParams;
            finishCommandParams.SetEndDataStruct(true);
            finishCommandParams.SetCommandRef(i);
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.FinishCommand(finishCommandParams));
        }
    }

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    EXPECT_EQ(commandSender.Finalize(commandDatabuf), CHIP_NO_ERROR);

    mockCommandHandlerDelegate.ResetCounter();
    sendResponse           = true;
    commandDispatchedCount = 0;

    CommandHandlerImpl commandHandler(&mockCommandHandlerDelegate);
    MockCommandResponder mockCommandResponder;
    Protocols::InteractionModel::Status status =
        commandHandler.OnInvokeCommandRequest(mockCommandResponder, std::move(commandDatabuf), false);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidAction);
    EXPECT_TRUE(mockCommandResponder.mChunks.IsNull());

    EXPECT_EQ(commandDispatchedCount, 0u);
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_AcceptMultipleCommands)
{

    isCommandDispatched = false;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate, &GetExchangeManager(),
                                     &pendingResponseTracker);

    app::CommandSender::ConfigParameters configParameters;
    configParameters.SetRemoteMaxPathsPerInvoke(2);
    EXPECT_EQ(CHIP_NO_ERROR, commandSender.SetCommandSenderConfig(configParameters));

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
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.PrepareCommand(requestCommandPaths[i], prepareCommandParams));
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.GetCommandDataIBTLVWriter()->PutBoolean(chip::TLV::ContextTag(1), true));
            app::CommandSender::FinishCommandParameters finishCommandParams;
            finishCommandParams.SetEndDataStruct(true);
            finishCommandParams.SetCommandRef(i);
            EXPECT_EQ(CHIP_NO_ERROR, commandSender.FinishCommand(finishCommandParams));
        }
        commandSender.MoveToState(app::CommandSender::State::AddedCommand);
    }

    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandlerImpl::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandlerImpl commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    EXPECT_EQ(commandSender.Finalize(commandDatabuf), CHIP_NO_ERROR);

    sendResponse = true;
    mockCommandHandlerDelegate.ResetCounter();
    commandDispatchedCount = 0;

    Protocols::InteractionModel::Status status = commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), false);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::Success);

    EXPECT_EQ(commandDispatchedCount, 2u);
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsStatusResponse)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandlerImpl::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandlerImpl commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);
    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2))),
              CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(&commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    EXPECT_EQ(remainingSize, sizeToLeave);

    AddInvokeResponseData(&commandHandler, /* aNeedStatusCode = */ true, kTestCommandIdCommandSpecificResponse,
                          kTestCommandIdCommandSpecificResponse);

    remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    EXPECT_GT(remainingSize, sizeToLeave);
}
TEST_F_FROM_FIXTURE(TestCommandInteraction,
                    TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponsePrimative)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandlerImpl::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandlerImpl commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);
    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2))),
              CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(&commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    EXPECT_EQ(remainingSize, sizeToLeave);

    AddInvokeResponseData(&commandHandler, /* aNeedStatusCode = */ false, kTestCommandIdCommandSpecificResponse,
                          kTestCommandIdCommandSpecificResponse);

    remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    EXPECT_GT(remainingSize, sizeToLeave);
}

TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponse)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandlerImpl::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandlerImpl commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);
    commandHandler.mReserveSpaceForMoreChunkMessages = true;
    ConcreteCommandPath requestCommandPath1          = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2          = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);
    EXPECT_EQ(basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2))),
              CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(&commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    EXPECT_EQ(remainingSize, sizeToLeave);

    uint32_t sizeToFill = 50;
    ForcedSizeBuffer responseData(sizeToFill);
    EXPECT_EQ(commandHandler.AddResponseData(requestCommandPath2, responseData.GetCommandId(), responseData), CHIP_NO_ERROR);

    remainingSize = commandHandler.mInvokeResponseBuilder.GetWriter()->GetRemainingFreeLength();
    EXPECT_GT(remainingSize, sizeToLeave);
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
//
// This test needs a special unit-test only API being exposed in ExchangeContext to be able to correctly simulate
// the release of a session on the exchange.
//
TEST_F_FROM_FIXTURE(TestCommandInteraction, TestCommandHandler_ReleaseWithExchangeClosed)
{

    app::CommandSender commandSender(&mockCommandSenderDelegate, &GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommandHandle = nullptr;
    asyncCommand       = true;

    EXPECT_EQ(commandSender.SendCommandRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    // Verify that async command handle has been allocated
    ASSERT_NE(asyncCommandHandle.Get(), nullptr);

    // Mimic closure of the exchange that would happen on a session release and verify that releasing the handle there-after
    // is handled gracefully.
    asyncCommandHandle.Get()->GetExchangeContext()->GetSessionHolder().Release();
    asyncCommandHandle.Get()->GetExchangeContext()->OnSessionReleased();
    asyncCommandHandle = nullptr;
}
#endif

} // namespace app
} // namespace chip
