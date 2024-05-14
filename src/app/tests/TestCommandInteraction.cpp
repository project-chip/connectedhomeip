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

#include <gtest/gtest.h>

#include <app/tests/CommandHandlerTestAccess.h>
#include <app/tests/CommandSenderTestAccess.h>
#include <messaging/tests/ReliableMessageContextTestAccess.h>

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
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <optional>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

using TestContext = chip::Test::AppContext;
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
} // namespace

namespace app {

CommandHandler::Handle asyncCommandHandle;

enum class ForcedSizeBufferLengthHint
{
    kSizeBetween0and255,
    kSizeGreaterThan255,
};

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
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = aReader.Next();
    if (aRequestCommandPath.mCommandId == kTestCommandIdNoData)
    {
        EXPECT_EQ(err, CHIP_ERROR_END_OF_TLV);
    }
    else
    {
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(aReader.GetTag(), TLV::ContextTag(1));
        bool val;
        err = aReader.Get(val);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(val);
    }

    err = aReader.ExitContainer(outerContainerType);
    EXPECT_EQ(err, CHIP_NO_ERROR);

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

    System::PacketBufferHandle mChunks;
    bool mResponseDropped = false;
};

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

class TestCommandInteraction : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        pTestContext = new TestContext;
        pTestContext->SetUpTestSuite();
    }
    static void TearDownTestSuite()
    {
        pTestContext->TearDownTestSuite();
        if (pTestContext != nullptr)
        {
            delete pTestContext;
        }
    }

    void SetUp() override
    {
        if (pTestContext != nullptr)
        {
            pTestContext->SetUp();
        }
    }
    void TearDown() override
    {
        if (pTestContext != nullptr)
        {
            pTestContext->TearDown();
        }
    }
    static TestContext * pTestContext;

    static size_t GetNumActiveCommandResponderObjects()
    {
        return chip::app::InteractionModelEngine::GetInstance()->mCommandResponderObjs.Allocated();
    }

    /**
     * With the introduction of batch invoke commands, CommandHandler keeps track of incoming
     * ConcreteCommandPath and the associated CommandRefs. These are normally populated
     * as part of OnInvokeCommandRequest from the incoming request. For some unit tests where
     * we want to test APIs that cluster code uses, we need to inject entries into the
     * CommandPathRegistry directly.
     */
    class CommandHandlerWithUnrespondedCommand : public app::CommandHandler
    {
    public:
        CommandHandlerWithUnrespondedCommand(CommandHandler::Callback * apCallback, const ConcreteCommandPath & aRequestCommandPath,
                                             const Optional<uint16_t> & aRef) : CommandHandler(apCallback)
        {
            GetCommandPathRegistry().Add(aRequestCommandPath, aRef.std_optional());
            SetExchangeInterface(&mMockCommandResponder);
        }
        MockCommandResponder mMockCommandResponder;
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
    static void FillCurrentInvokeResponseBuffer(CommandHandler * apCommandHandler, const ConcreteCommandPath & aRequestCommandPath,
                                                uint32_t aSizeToLeaveInBuffer);
    static void ValidateCommandHandlerEncodeInvokeResponseMessage(bool aNeedStatusCode);
};
TestContext * TestCommandInteraction::pTestContext = nullptr;

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
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    err = invokeRequestMessageBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    invokeRequestMessageBuilder.SuppressResponse(true).TimedRequest(aIsTimedRequest);
    InvokeRequests::Builder & invokeRequests = invokeRequestMessageBuilder.CreateInvokeRequests();
    EXPECT_EQ(invokeRequestMessageBuilder.GetError(), CHIP_NO_ERROR);

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
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kFields)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    commandDataIBBuilder.EndOfCommandDataIB();
    EXPECT_EQ(commandDataIBBuilder.GetError(), CHIP_NO_ERROR);

    invokeRequests.EndOfInvokeRequests();
    EXPECT_EQ(invokeRequests.GetError(), CHIP_NO_ERROR);

    invokeRequestMessageBuilder.EndOfInvokeRequestMessage();
    EXPECT_EQ(invokeRequestMessageBuilder.GetError(), CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void TestCommandInteraction::GenerateInvokeResponse(System::PacketBufferHandle & aPayload, CommandId aCommandId,
                                                    ClusterId aClusterId, EndpointId aEndpointId,
                                                    std::optional<uint16_t> aCommandRef)

{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    err = invokeResponseMessageBuilder.Init(&writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

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
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kFields)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
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

    err = writer.Finalize(&aPayload);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void TestCommandInteraction::AddInvokeRequestData(CommandSender * apCommandSender, CommandId aCommandId)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    auto commandPathParams = MakeTestCommandPath(aCommandId);

    err = apCommandSender->PrepareCommand(commandPathParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = apCommandSender->GetCommandDataIBTLVWriter();

    err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = apCommandSender->FinishCommand();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void TestCommandInteraction::AddInvalidInvokeRequestData(CommandSender * apCommandSender, CommandId aCommandId)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    auto commandPathParams = MakeTestCommandPath(aCommandId);

    err = apCommandSender->PrepareCommand(commandPathParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = apCommandSender->GetCommandDataIBTLVWriter();

    err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    apCommandSender->MoveToState(CommandSender::State::AddedCommand);
}

void TestCommandInteraction::AddInvokeResponseData(CommandHandler * apCommandHandler, bool aNeedStatusCode,
                                                   CommandId aResponseCommandId, CommandId aRequestCommandId)
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
        EXPECT_EQ(err, CHIP_NO_ERROR);

        chip::TLV::TLVWriter * writer = apCommandHandler->GetCommandDataIBTLVWriter();

        err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = apCommandHandler->FinishCommand();
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
}

uint32_t TestCommandInteraction::GetAddResponseDataOverheadSizeForPath(const ConcreteCommandPath & aRequestCommandPath,
                                                                       ForcedSizeBufferLengthHint aBufferSizeHint)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandler::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandler commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);
    chip::Test::CommandHandlerTestAccess(&commandHandler).SetReserveSpaceForMoreChunkMessages(true);
    ConcreteCommandPath requestCommandPath1 = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2 = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1)));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = commandHandler.AllocateBuffer();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t remainingSizeBefore =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();

    // When ForcedSizeBuffer exceeds 255, an extra byte is needed for length, affecting the overhead size required by
    // AddResponseData. In order to have this accounted for in overhead calculation we set the length to be 256.
    uint32_t sizeOfForcedSizeBuffer = aBufferSizeHint == ForcedSizeBufferLengthHint::kSizeGreaterThan255 ? 256 : 0;
    err                             = commandHandler.AddResponseData(aRequestCommandPath, ForcedSizeBuffer(sizeOfForcedSizeBuffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    uint32_t remainingSizeAfter =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();
    uint32_t delta = remainingSizeBefore - remainingSizeAfter - sizeOfForcedSizeBuffer;

    return delta;
}

void TestCommandInteraction::FillCurrentInvokeResponseBuffer(CommandHandler * apCommandHandler,
                                                             const ConcreteCommandPath & aRequestCommandPath,
                                                             uint32_t aSizeToLeaveInBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = apCommandHandler->AllocateBuffer();
    EXPECT_EQ(err, CHIP_NO_ERROR);
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

    err = apCommandHandler->AddResponseData(aRequestCommandPath, ForcedSizeBuffer(sizeToFill));
    EXPECT_EQ(err, CHIP_NO_ERROR);
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

TEST_F(TestCommandInteraction, TestCommandSenderWithWrongState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestCommandInteraction, TestCommandHandlerWithWrongState)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ConcreteCommandPath requestCommandPath  = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    CommandHandlerWithUnrespondedCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        const CommandHandler::InvokeResponseParameters prepareParams(requestCommandPath);
        err = commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
    EXPECT_TRUE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

TEST_F(TestCommandInteraction, TestCommandSenderWithSendCommand)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AddInvokeRequestData(&commandSender);
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    GenerateInvokeResponse(buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    chip::Test::CommandSenderTestAccess privatecommandSender(&commandSender);
    err = privatecommandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(moreChunkedMessages);
}

TEST_F(TestCommandInteraction, TestCommandHandlerWithSendEmptyCommand)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    ConcreteCommandPath requestCommandPath  = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    CommandHandlerWithUnrespondedCommand commandHandler(&mockCommandHandlerDelegate, requestCommandPath,
                                                        /* aRef = */ NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        const CommandHandler::InvokeResponseParameters prepareParams(requestCommandPath);
        err = commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = commandHandler.FinishCommand();
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
    EXPECT_FALSE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

TEST_F(TestCommandInteraction, TestCommandSenderWithProcessReceivedMsg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateInvokeResponse(buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    chip::Test::CommandSenderTestAccess privatecommandSender(&commandSender);
    err = privatecommandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(moreChunkedMessages);
}

TEST_F(TestCommandInteraction, TestCommandSenderExtendableApiWithProcessReceivedMsg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);

    uint16_t mockCommandRef = 1;
    pendingResponseTracker.Add(mockCommandRef);
    chip::Test::CommandSenderTestAccess(&commandSender).SetFinishedCommandCount(1);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateInvokeResponse(buf, kTestCommandIdWithData);
    bool moreChunkedMessages = false;
    chip::Test::CommandSenderTestAccess privatecommandSender(&commandSender);
    err = privatecommandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(moreChunkedMessages);

    chip::Test::CommandSenderTestAccess(&commandSender).FlushNoCommandResponse();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onNoResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 0);
}

TEST_F(TestCommandInteraction, TestCommandSenderExtendableApiWithProcessReceivedMsgContainingInvalidCommandRef)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);

    uint16_t mockCommandRef = 1;
    pendingResponseTracker.Add(mockCommandRef);
    chip::Test::CommandSenderTestAccess(&commandSender).SetFinishedCommandCount(1);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    uint16_t invalidResponseCommandRef = 2;
    GenerateInvokeResponse(buf, kTestCommandIdWithData, kTestClusterId, kTestEndpointId,
                           std::make_optional(invalidResponseCommandRef));
    bool moreChunkedMessages = false;
    chip::Test::CommandSenderTestAccess privatecommandSender(&commandSender);
    err = privatecommandSender.ProcessInvokeResponse(std::move(buf), moreChunkedMessages);
    EXPECT_EQ(err, CHIP_ERROR_KEY_NOT_FOUND);
    EXPECT_FALSE(moreChunkedMessages);

    chip::Test::CommandSenderTestAccess(&commandSender).FlushNoCommandResponse();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onNoResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 0);
}

TEST_F(TestCommandInteraction, TestCommandHandlerEncodeSimpleCommandData)
{
    // Send response which has simple command data and command path
    ValidateCommandHandlerEncodeInvokeResponseMessage(false /*aNeedStatusCode=false*/);
}

TEST_F(TestCommandInteraction, TestCommandHandlerCommandDataEncoding)
{
    auto path               = MakeTestCommandPath();
    auto requestCommandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
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
#define PretendWeGotReplyFromServer(aContext, aClientExchange)                                                                     \
    {                                                                                                                              \
        Messaging::ReliableMessageMgr * localRm    = aContext->GetExchangeManager().GetReliableMessageMgr();                       \
        Messaging::ExchangeContext * localExchange = aClientExchange;                                                              \
        EXPECT_EQ(localRm->TestGetCountRetransTable(), 2);                                                                         \
                                                                                                                                   \
        localRm->ClearRetransTable(localExchange);                                                                                 \
        EXPECT_EQ(localRm->TestGetCountRetransTable(), 1);                                                                         \
                                                                                                                                   \
        localRm->EnumerateRetransTable([localExchange](auto * entry) {                                                             \
            chip::Test::ReliableMessageContextTestAccess(localExchange)                                                            \
                .SetPendingPeerAckMessageCounter(entry->retainedBuf.GetMessageCounter());                                          \
            return Loop::Break;                                                                                                    \
        });                                                                                                                        \
    }

// Command Sender sends invoke request, command handler drops invoke response, then test injects status response message with
// busy to client, client sends out a status response with invalid action.
TEST_F(TestCommandInteraction, TestCommandInvalidMessage1)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 1;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);
    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(pTestContext->GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    StatusResponseMessage::Builder response;
    response.Init(&writer);
    response.Status(Protocols::InteractionModel::Status::Busy);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);
    chip::Test::MessageCapturer messageLog(*pTestContext);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(pTestContext, chip::Test::CommandSenderTestAccess(&commandSender).GetExchangeCtx().Get());

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 0;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    pTestContext->GetLoopback().mDroppedMessageCount              = 0;

    err = chip::Test::CommandSenderTestAccess(&commandSender).OnMessageReceived(payloadHeader, std::move(msgBuf));
    EXPECT_EQ(err, CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

    pTestContext->DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    pTestContext->ExpireSessionAliceToBob();
    pTestContext->ExpireSessionBobToAlice();
    pTestContext->CreateSessionAliceToBob();
    pTestContext->CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects unknown message to client,
// client sends out status response with invalid action.
TEST_F(TestCommandInteraction, TestCommandInvalidMessage2)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 1;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);
    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(pTestContext->GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    ReportDataMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);
    chip::Test::MessageCapturer messageLog(*pTestContext);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(pTestContext, chip::Test::CommandSenderTestAccess(&commandSender).GetExchangeCtx().Get());

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 0;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    pTestContext->GetLoopback().mDroppedMessageCount              = 0;

    err = chip::Test::CommandSenderTestAccess(&commandSender).OnMessageReceived(payloadHeader, std::move(msgBuf));
    EXPECT_EQ(err, CHIP_ERROR_INVALID_MESSAGE_TYPE);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_ERROR_INVALID_MESSAGE_TYPE);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

    pTestContext->DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    pTestContext->ExpireSessionAliceToBob();
    pTestContext->ExpireSessionBobToAlice();
    pTestContext->CreateSessionAliceToBob();
    pTestContext->CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects malformed invoke response
// message to client, client sends out status response with invalid action.
TEST_F(TestCommandInteraction, TestCommandInvalidMessage3)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 1;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);
    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(pTestContext->GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    InvokeResponseMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandResponse);
    chip::Test::MessageCapturer messageLog(*pTestContext);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(pTestContext, chip::Test::CommandSenderTestAccess(&commandSender).GetExchangeCtx().Get());

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 0;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    pTestContext->GetLoopback().mDroppedMessageCount              = 0;

    err = chip::Test::CommandSenderTestAccess(&commandSender).OnMessageReceived(payloadHeader, std::move(msgBuf));
    EXPECT_EQ(err, CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

    pTestContext->DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    pTestContext->ExpireSessionAliceToBob();
    pTestContext->ExpireSessionBobToAlice();
    pTestContext->CreateSessionAliceToBob();
    pTestContext->CreateSessionBobToAlice();
}

// Command Sender sends invoke request, command handler drops invoke response, then test injects malformed status response to
// client, client responds to the status response with invalid action.
TEST_F(TestCommandInteraction, TestCommandInvalidMessage4)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = false;

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 1;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);
    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(pTestContext->GetLoopback().mDroppedMessageCount, 1u);

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    StatusResponseMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);
    chip::Test::MessageCapturer messageLog(*pTestContext);
    messageLog.mCaptureStandaloneAcks = false;

    // Since we are dropping packets, things are not getting acked.  Set up our
    // MRP state to look like what it would have looked like if the packet had
    // not gotten dropped.
    PretendWeGotReplyFromServer(pTestContext, chip::Test::CommandSenderTestAccess(&commandSender).GetExchangeCtx().Get());

    pTestContext->GetLoopback().mSentMessageCount                 = 0;
    pTestContext->GetLoopback().mNumMessagesToDrop                = 0;
    pTestContext->GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    pTestContext->GetLoopback().mDroppedMessageCount              = 0;

    err = chip::Test::CommandSenderTestAccess(&commandSender).OnMessageReceived(payloadHeader, std::move(msgBuf));
    EXPECT_EQ(err, CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_ERROR_END_OF_TLV);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);

    pTestContext->DrainAndServiceIO();

    // Client sent status report with invalid action, server's exchange has been closed, so all it sent is an MRP Ack
    EXPECT_EQ(pTestContext->GetLoopback().mSentMessageCount, 2u);
    CheckForInvalidAction(messageLog);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    pTestContext->ExpireSessionAliceToBob();
    pTestContext->ExpireSessionBobToAlice();
    pTestContext->CreateSessionAliceToBob();
    pTestContext->CreateSessionBobToAlice();
}

// Command Sender sends malformed invoke request, handler fails to process it and sends status report with invalid action
TEST_F(TestCommandInteraction, TestCommandHandlerInvalidMessageSync)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    chip::isCommandDispatched = false;
    AddInvalidInvokeRequestData(&commandSender);
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    EXPECT_FALSE(chip::isCommandDispatched);
    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandHandlerCommandEncodeExternalFailure)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    auto path               = MakeTestCommandPath();
    auto requestCommandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    CommandHandlerWithUnrespondedCommand commandHandler(nullptr, requestCommandPath, NullOptional);

    {
        // This simulates how cluster would call CommandHandler APIs synchronously. There would
        // be handle already acquired on the callers behalf.
        CommandHandler::Handle handle(&commandHandler);

        err = commandHandler.AddResponseData(requestCommandPath, BadFields());
        EXPECT_NE(err, CHIP_NO_ERROR);
        commandHandler.AddStatus(requestCommandPath, Protocols::InteractionModel::Status::Failure);
    }
    EXPECT_FALSE(commandHandler.mMockCommandResponder.mChunks.IsNull());
}

TEST_F(TestCommandInteraction, TestCommandHandlerEncodeSimpleStatusCode)
{
    // Send response which has simple status code and command path
    ValidateCommandHandlerEncodeInvokeResponseMessage(true /*aNeedStatusCode=true*/);
}

TEST_F(TestCommandInteraction, TestCommandHandlerWithoutResponderCallingAddStatus)
{
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandler commandHandler(&mockCommandHandlerDelegate);

    commandHandler.AddStatus(requestCommandPath, Protocols::InteractionModel::Status::Failure);

    // Since calling AddStatus is supposed to be a no-operation when there is no responder, it is
    // hard to validate. Best way is to check that we are still in an Idle state afterwards
    EXPECT_TRUE(chip::Test::CommandHandlerTestAccess(&commandHandler).TestOnlyIsInIdleState());
}

TEST_F(TestCommandInteraction, TestCommandHandlerWithoutResponderCallingAddResponse)
{
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandler commandHandler(&mockCommandHandlerDelegate);

    uint32_t sizeToFill = 50; // This is an arbitrary number, we need to select a non-zero value.
    CHIP_ERROR err      = commandHandler.AddResponseData(requestCommandPath, ForcedSizeBuffer(sizeToFill));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Since calling AddResponseData is supposed to be a no-operation when there is no responder, it is
    // hard to validate. Best way is to check that we are still in an Idle state afterwards
    EXPECT_TRUE(chip::Test::CommandHandlerTestAccess(&commandHandler).TestOnlyIsInIdleState());
}

TEST_F(TestCommandInteraction, TestCommandHandlerWithoutResponderCallingDirectPrepareFinishCommandApis)
{
    chip::app::ConcreteCommandPath requestCommandPath(kTestEndpointId, kTestClusterId, kTestCommandIdWithData);
    CommandHandler commandHandler(&mockCommandHandlerDelegate);

    // We intentionally prevent successful calls to PrepareInvokeResponseCommand and FinishCommand when no
    // responder is present. This aligns with the design decision to promote AddStatus and AddResponseData
    // usage in such scenarios. See GitHub issue #32486 for discussions on phasing out external use of
    // these primitives.
    const CommandHandler::InvokeResponseParameters prepareParams(requestCommandPath);
    ConcreteCommandPath responseCommandPath = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };
    CHIP_ERROR err                          = commandHandler.PrepareInvokeResponseCommand(responseCommandPath, prepareParams);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    EXPECT_EQ(commandHandler.GetCommandDataIBTLVWriter(), nullptr);

    err = commandHandler.FinishCommand();
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    EXPECT_TRUE(chip::Test::CommandHandlerTestAccess(&commandHandler).TestOnlyIsInIdleState());
}

TEST_F(TestCommandInteraction, TestCommandHandlerWithOnInvokeReceivedNotExistCommand)
{
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    // Use some invalid endpoint / cluster / command.
    GenerateInvokeRequest(commandDatabuf, /* aIsTimedRequest = */ false, 0xEF /* command */, 0xADBE /* cluster */,
                          0xDE /* endpoint */);
    CommandHandler commandHandler(&mockCommandHandlerDelegate);
    chip::isCommandDispatched = false;

    mockCommandHandlerDelegate.ResetCounter();
    MockCommandResponder mockCommandResponder;
    InteractionModel::Status status = commandHandler.OnInvokeCommandRequest(mockCommandResponder, std::move(commandDatabuf), false);

    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidAction);
    EXPECT_TRUE(mockCommandResponder.mChunks.IsNull());
    // TODO we can further validate the response is what we expected.
    EXPECT_FALSE(chip::isCommandDispatched);
}

TEST_F(TestCommandInteraction, TestCommandHandlerWithOnInvokeReceivedEmptyDataMsg)
{
    bool allBooleans[] = { true, false };
    for (auto messageIsTimed : allBooleans)
    {
        for (auto transactionIsTimed : allBooleans)
        {
            mockCommandHandlerDelegate.ResetCounter();
            CommandHandler commandHandler(&mockCommandHandlerDelegate);
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestNonExistCommandId);
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Because UnsupportedCommand is a path specific error we will expect it to come via on response when using Extended Path.
TEST_F(TestCommandInteraction, TestCommandSenderExtendableCallbackUnsupportedCommand)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderExtendedDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderExtendedDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestNonExistCommandId);
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderLegacyCallbackBuildingBatchCommandFails)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;
    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(0);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(0);

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    err = commandSender.SetCommandSenderConfig(config);
    EXPECT_EQ(err, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    // Even though we got an error saying invalid argument we are going to attempt
    // to add two commands.

    auto commandPathParams = MakeTestCommandPath();
    err                    = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    err                           = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(1);
    err = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderExtendableCallbackBuildingBatchDuplicateCommandRefFails)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    err = commandSender.SetCommandSenderConfig(config);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(0);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(0);
    auto commandPathParams = MakeTestCommandPath();
    err                    = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    err                           = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(0);
    err = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderExtendableCallbackBuildingBatchCommandSuccess)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);
    app::CommandSender::PrepareCommandParameters prepareCommandParams;
    app::CommandSender::FinishCommandParameters finishCommandParams;

    CommandSender::ConfigParameters config;
    config.SetRemoteMaxPathsPerInvoke(2);
    err = commandSender.SetCommandSenderConfig(config);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // The specific values chosen here are arbitrary. This test primarily verifies that we can
    // use a larger command reference value followed by a smaller one for subsequent command.
    uint16_t firstCommandRef  = 40;
    uint16_t secondCommandRef = 2;
    prepareCommandParams.SetStartDataStruct(true).SetCommandRef(firstCommandRef);
    finishCommandParams.SetEndDataStruct(true).SetCommandRef(firstCommandRef);
    auto commandPathParams = MakeTestCommandPath();
    err                    = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    chip::TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    err                           = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Preparing second command.
    prepareCommandParams.SetCommandRef(secondCommandRef);
    finishCommandParams.SetCommandRef(secondCommandRef);
    err = commandSender.PrepareCommand(commandPathParams, prepareCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    writer = commandSender.GetCommandDataIBTLVWriter();
    err    = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = commandSender.FinishCommand(finishCommandParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandSuccessResponseFlow)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());

    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 0u);

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(commandSender.GetInvokeResponseMessageCount(), 1u);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandAsyncSuccessResponseFlow)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommand = true;
    err          = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());

    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 1u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 2u);

    // Decrease CommandHandler refcount and send response
    asyncCommandHandle = nullptr;

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandSpecificResponseFlow)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestCommandIdCommandSpecificResponse);
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());

    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderCommandFailureResponseFlow)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender, kTestNonExistCommandId);
    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 1);
    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandSenderAbruptDestruction)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // Don't send back a response, just keep the CommandHandler
    // hanging to give us enough time to do what we want with the CommandSender object.
    //
    sendResponse = false;

    mockCommandSenderDelegate.ResetCounter();

    {
        app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

        AddInvokeRequestData(&commandSender, kTestCommandIdCommandSpecificResponse);
        err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());

        EXPECT_EQ(err, CHIP_NO_ERROR);

        pTestContext->DrainAndServiceIO();

        //
        // No callbacks should be invoked yet - let's validate that.
        //
        EXPECT_EQ(mockCommandSenderDelegate.onResponseCalledTimes, 0);
        EXPECT_EQ(mockCommandSenderDelegate.onFinalCalledTimes, 0);
        EXPECT_EQ(mockCommandSenderDelegate.onErrorCalledTimes, 0);

        EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 1u);
    }

    //
    // Upon the sender being destructed by the application, our exchange should get cleaned up too.
    //
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
}

TEST_F(TestCommandInteraction, TestCommandHandlerRejectMultipleIdenticalCommands)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    isCommandDispatched = false;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);

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

    err = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());

    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    EXPECT_EQ(mockCommandSenderExtendedDelegate.onResponseCalledTimes, 0);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onFinalCalledTimes, 1);
    EXPECT_EQ(mockCommandSenderExtendedDelegate.onErrorCalledTimes, 1);
    EXPECT_FALSE(chip::isCommandDispatched);

    EXPECT_EQ(GetNumActiveCommandResponderObjects(), 0u);
    EXPECT_EQ(pTestContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

TEST_F(TestCommandInteraction, TestCommandHandlerRejectsMultipleCommandsWithIdenticalCommandRef)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    isCommandDispatched = false;
    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);

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
    CommandHandler::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandler commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    err = chip::Test::CommandSenderTestAccess(&commandSender).Finalize(commandDatabuf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mockCommandHandlerDelegate.ResetCounter();
    commandDispatchedCount = 0;

    InteractionModel::Status status =
        chip::Test::CommandHandlerTestAccess(&commandHandler).ProcessInvokeRequest(std::move(commandDatabuf), false);
    EXPECT_EQ(status, InteractionModel::Status::InvalidAction);

    EXPECT_EQ(commandDispatchedCount, 0u);
}

#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

TEST_F(TestCommandInteraction, TestCommandHandlerRejectMultipleCommandsWhenHandlerOnlySupportsOne)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    isCommandDispatched = false;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);

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
    err = chip::Test::CommandSenderTestAccess(&commandSender).Finalize(commandDatabuf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mockCommandHandlerDelegate.ResetCounter();
    sendResponse           = true;
    commandDispatchedCount = 0;

    CommandHandler commandHandler(&mockCommandHandlerDelegate);
    MockCommandResponder mockCommandResponder;
    InteractionModel::Status status = commandHandler.OnInvokeCommandRequest(mockCommandResponder, std::move(commandDatabuf), false);
    EXPECT_EQ(status, InteractionModel::Status::InvalidAction);
    EXPECT_TRUE(mockCommandResponder.mChunks.IsNull());

    EXPECT_EQ(commandDispatchedCount, 0u);
}

TEST_F(TestCommandInteraction, TestCommandHandlerAcceptMultipleCommands)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    isCommandDispatched = false;

    mockCommandSenderExtendedDelegate.ResetCounter();
    PendingResponseTrackerImpl pendingResponseTracker;
    app::CommandSender commandSender(kCommandSenderTestOnlyMarker, &mockCommandSenderExtendedDelegate,
                                     &pTestContext->GetExchangeManager(), &pendingResponseTracker);

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
        // changing the state of commandSender to State::AddedCommand
        chip::Test::CommandSenderTestAccess(&commandSender).MoveToStateAddedCommand();
    }

    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandler::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandler commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    // Hackery to steal the InvokeRequest buffer from commandSender.
    System::PacketBufferHandle commandDatabuf;
    err = chip::Test::CommandSenderTestAccess(&commandSender).Finalize(commandDatabuf);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    sendResponse = true;
    mockCommandHandlerDelegate.ResetCounter();
    commandDispatchedCount = 0;

    InteractionModel::Status status =
        chip::Test::CommandHandlerTestAccess(&commandHandler).ProcessInvokeRequest(std::move(commandDatabuf), false);
    EXPECT_EQ(status, InteractionModel::Status::Success);

    EXPECT_EQ(commandDispatchedCount, 2u);
}

TEST_F(TestCommandInteraction, TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsStatusResponse)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandler::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandler commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    chip::Test::CommandHandlerTestAccess(&commandHandler).SetReserveSpaceForMoreChunkMessages(true);
    ConcreteCommandPath requestCommandPath1 = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2 = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1)));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(&commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();
    EXPECT_EQ(remainingSize, sizeToLeave);

    AddInvokeResponseData(&commandHandler, /* aNeedStatusCode = */ true, kTestCommandIdCommandSpecificResponse,
                          kTestCommandIdCommandSpecificResponse);

    remainingSize =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();
    EXPECT_GT(remainingSize, sizeToLeave);
}
TEST_F(TestCommandInteraction, TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponsePrimative)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandler::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandler commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);

    chip::Test::CommandHandlerTestAccess(&commandHandler).SetReserveSpaceForMoreChunkMessages(true);
    ConcreteCommandPath requestCommandPath1 = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2 = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1)));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(&commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();
    EXPECT_EQ(remainingSize, sizeToLeave);

    AddInvokeResponseData(&commandHandler, /* aNeedStatusCode = */ false, kTestCommandIdCommandSpecificResponse,
                          kTestCommandIdCommandSpecificResponse);

    remainingSize =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();
    EXPECT_GT(remainingSize, sizeToLeave);
}

TEST_F(TestCommandInteraction, TestCommandHandler_FillUpInvokeResponseMessageWhereSecondResponseIsDataResponse)
{
    BasicCommandPathRegistry<4> basicCommandPathRegistry;
    MockCommandResponder mockCommandResponder;
    CommandHandler::TestOnlyOverrides testOnlyOverrides{ &basicCommandPathRegistry, &mockCommandResponder };
    CommandHandler commandHandler(testOnlyOverrides, &mockCommandHandlerDelegate);
    chip::Test::CommandHandlerTestAccess(&commandHandler).SetReserveSpaceForMoreChunkMessages(true);
    ConcreteCommandPath requestCommandPath1 = { kTestEndpointId, kTestClusterId, kTestCommandIdFillResponseMessage };
    ConcreteCommandPath requestCommandPath2 = { kTestEndpointId, kTestClusterId, kTestCommandIdCommandSpecificResponse };

    CHIP_ERROR err = basicCommandPathRegistry.Add(requestCommandPath1, std::make_optional<uint16_t>(static_cast<uint16_t>(1)));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = basicCommandPathRegistry.Add(requestCommandPath2, std::make_optional<uint16_t>(static_cast<uint16_t>(2)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint32_t sizeToLeave = 0;
    FillCurrentInvokeResponseBuffer(&commandHandler, requestCommandPath1, sizeToLeave);
    uint32_t remainingSize =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();
    EXPECT_EQ(remainingSize, sizeToLeave);

    uint32_t sizeToFill = 50;
    err                 = commandHandler.AddResponseData(requestCommandPath2, ForcedSizeBuffer(sizeToFill));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    remainingSize =
        chip::Test::CommandHandlerTestAccess(&commandHandler).GetInvokeResponseBuilder().GetWriter()->GetRemainingFreeLength();
    EXPECT_GT(remainingSize, sizeToLeave);
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
//
// This test needs a special unit-test only API being exposed in ExchangeContext to be able to correctly simulate
// the release of a session on the exchange.
//
TEST_F(TestCommandInteraction, TestCommandHandlerReleaseWithExchangeClosed)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &pTestContext->GetExchangeManager());

    AddInvokeRequestData(&commandSender);
    asyncCommandHandle = nullptr;
    asyncCommand       = true;
    err                = commandSender.SendCommandRequest(pTestContext->GetSessionBobToAlice());

    EXPECT_EQ(err, CHIP_NO_ERROR);

    pTestContext->DrainAndServiceIO();

    // Verify that async command handle has been allocated
    ASSERT_NE(asyncCommandHandle.Get(), nullptr);

    // Mimic closure of the exchange that would happen on a session release and verify that releasing the handle there-after
    // is handled gracefully.
    chip::Test::CommandHandlerTestAccess(asyncCommandHandle.Get())
        .GetmpResponder()
        ->GetExchangeContext()
        ->GetSessionHolder()
        .Release();
    chip::Test::CommandHandlerTestAccess(asyncCommandHandle.Get()).GetmpResponder()->GetExchangeContext()->OnSessionReleased();

    asyncCommandHandle = nullptr;
}
#endif

} // namespace app
} // namespace chip
