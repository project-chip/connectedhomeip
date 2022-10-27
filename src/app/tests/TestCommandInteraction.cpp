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
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/core/Optional.h>
#include <lib/support/ErrorStr.h>
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
bool isCommandDispatched = false;

bool sendResponse = true;
bool asyncCommand = false;

// Allow us to do test asserts from arbitrary places.
nlTestSuite * gSuite = nullptr;

constexpr EndpointId kTestEndpointId                      = 1;
constexpr ClusterId kTestClusterId                        = 3;
constexpr CommandId kTestCommandIdWithData                = 4;
constexpr CommandId kTestCommandIdNoData                  = 5;
constexpr CommandId kTestCommandIdCommandSpecificResponse = 6;
constexpr CommandId kTestNonExistCommandId                = 0;
} // namespace

namespace app {

CommandHandler::Handle asyncCommandHandle;

InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // Mock cluster catalog, only support commands on one cluster on one endpoint.
    using InteractionModel::Status;

    if (aCommandPath.mEndpointId != kTestEndpointId)
    {
        return Status::UnsupportedEndpoint;
    }

    if (aCommandPath.mClusterId != kTestClusterId)
    {
        return Status::UnsupportedCluster;
    }

    if (aCommandPath.mCommandId == kTestNonExistCommandId)
    {
        return Status::UnsupportedCommand;
    }

    return Status::Success;
}

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    ChipLogDetail(Controller, "Received Cluster Command: Endpoint=%x Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  aCommandPath.mEndpointId, ChipLogValueMEI(aCommandPath.mClusterId), ChipLogValueMEI(aCommandPath.mCommandId));

    // Duplicate what our normal command-field-decode code does, in terms of
    // checking for a struct and then entering it before getting the fields.
    if (aReader.GetType() != TLV::kTLVType_Structure)
    {
        apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::InvalidAction);
        return;
    }

    TLV::TLVType outerContainerType;
    CHIP_ERROR err = aReader.EnterContainer(outerContainerType);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = aReader.Next();
    if (aCommandPath.mCommandId == kTestCommandIdNoData)
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
        if (aCommandPath.mCommandId == kTestCommandIdNoData || aCommandPath.mCommandId == kTestCommandIdWithData)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Success);
        }
        else
        {
            apCommandObj->PrepareCommand(aCommandPath);
            chip::TLV::TLVWriter * writer = apCommandObj->GetCommandDataIBTLVWriter();
            writer->PutBoolean(chip::TLV::ContextTag(1), true);
            apCommandObj->FinishCommand();
        }
    }

    chip::isCommandDispatched = true;
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
    static void TestCommandHandlerWithProcessReceivedNotExistCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendSimpleCommandData(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerCommandDataEncoding(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerCommandEncodeFailure(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage1(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage2(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage3(nlTestSuite * apSuite, void * apContext);
    static void TestCommandInvalidMessage4(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerInvalidMessageSync(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerInvalidMessageAsync(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerCommandEncodeExternalFailure(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendEmptyResponse(nlTestSuite * apSuite, void * apContext);

    static void TestCommandHandlerWithProcessReceivedEmptyDataMsg(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerRejectMultipleCommands(nlTestSuite * apSuite, void * apContext);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    static void TestCommandHandlerReleaseWithExchangeClosed(nlTestSuite * apSuite, void * apContext);
#endif

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
    // Generate an invoke request.  If aCommandId is kTestCommandIdWithData, a
    // payload will be included.  Otherwise no payload will be included.
    static void GenerateInvokeRequest(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                      bool aIsTimedRequest, CommandId aCommandId, ClusterId aClusterId = kTestClusterId,
                                      EndpointId aEndpointId = kTestEndpointId);
    // Generate an invoke response.  If aCommandId is kTestCommandIdWithData, a
    // payload will be included.  Otherwise no payload will be included.
    static void GenerateInvokeResponse(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                       CommandId aCommandId, ClusterId aClusterId = kTestClusterId,
                                       EndpointId aEndpointId = kTestEndpointId);
    static void AddInvokeRequestData(nlTestSuite * apSuite, void * apContext, CommandSender * apCommandSender,
                                     CommandId aCommandId = kTestCommandIdWithData);
    static void AddInvalidInvokeRequestData(nlTestSuite * apSuite, void * apContext, CommandSender * apCommandSender,
                                            CommandId aCommandId = kTestCommandIdWithData);
    static void AddInvokeResponseData(nlTestSuite * apSuite, void * apContext, CommandHandler * apCommandHandler,
                                      bool aNeedStatusCode, CommandId aCommandId = kTestCommandIdWithData);
    static void ValidateCommandHandlerWithSendCommand(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode);
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
                                                    CommandId aCommandId, ClusterId aClusterId, EndpointId aEndpointId)

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
                                                   bool aNeedStatusCode, CommandId aCommandId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aNeedStatusCode)
    {
        chip::app::ConcreteCommandPath commandPath(1, // Endpoint
                                                   3, // ClusterId
                                                   4  // CommandId
        );
        apCommandHandler->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ConcreteCommandPath path = { kTestEndpointId, kTestClusterId, aCommandId };
        err                      = apCommandHandler->PrepareCommand(path);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        chip::TLV::TLVWriter * writer = apCommandHandler->GetCommandDataIBTLVWriter();

        err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = apCommandHandler->FinishCommand();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
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
    TestContext & ctx        = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err           = CHIP_NO_ERROR;
    ConcreteCommandPath path = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);

    err = commandHandler.PrepareCommand(path);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    TestExchangeDelegate delegate;

    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mExchangeCtx.Grab(exchange);

    err = commandHandler.SendCommandResponse();

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
    err = commandSender.ProcessInvokeResponse(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx        = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err           = CHIP_NO_ERROR;
    ConcreteCommandPath path = { kTestEndpointId, kTestClusterId, kTestCommandIdNoData };

    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mExchangeCtx.Grab(exchange);

    err = commandHandler.PrepareCommand(path);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.FinishCommand();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.SendCommandResponse();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandHandler.Close();
}

void TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateInvokeResponse(apSuite, apContext, buf, kTestCommandIdWithData);
    err = commandSender.ProcessInvokeResponse(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::ValidateCommandHandlerWithSendCommand(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandPacket;

    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mExchangeCtx.Grab(exchange);

    AddInvokeResponseData(apSuite, apContext, &commandHandler, aNeedStatusCode);
    err = commandHandler.Finalize(commandPacket);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    chip::System::PacketBufferTLVReader reader;
    InvokeResponseMessage::Parser invokeResponseMessageParser;
    reader.Init(std::move(commandPacket));
    err = invokeResponseMessageParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeResponseMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif

    //
    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message. Since that isn't the case in this artificial setup here
    // (where we created a responder exchange that's not responding to anything), we need to explicitly close it out. This is not
    // expected in normal application logic.
    //
    exchange->Close();
}

void TestCommandInteraction::TestCommandHandlerWithSendSimpleCommandData(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple command data and command path
    ValidateCommandHandlerWithSendCommand(apSuite, apContext, false /*aNeedStatusCode=false*/);
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
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(nullptr);
    System::PacketBufferHandle commandPacket;

    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mExchangeCtx.Grab(exchange);

    auto path = MakeTestCommandPath();

    commandHandler.AddResponse(ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId), Fields());
    err = commandHandler.Finalize(commandPacket);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    chip::System::PacketBufferTLVReader reader;
    InvokeResponseMessage::Parser invokeResponseMessageParser;
    reader.Init(std::move(commandPacket));
    err = invokeResponseMessageParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeResponseMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif

    //
    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message. Since that isn't the case in this artificial setup here
    //  (where we created a responder exchange that's not responding to anything), we need to explicitly close it out. This is not
    //  expected in normal application logic.
    //
    exchange->Close();
}

void TestCommandInteraction::TestCommandHandlerCommandEncodeFailure(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(nullptr);
    System::PacketBufferHandle commandPacket;

    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mExchangeCtx.Grab(exchange);

    auto path = MakeTestCommandPath();

    commandHandler.AddResponse(ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId), BadFields());
    err = commandHandler.Finalize(commandPacket);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    chip::System::PacketBufferTLVReader reader;
    InvokeResponseMessage::Parser invokeResponseMessageParser;
    reader.Init(std::move(commandPacket));
    err = invokeResponseMessageParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeResponseMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif

    //
    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message. Since that isn't the case in this artificial setup here
    // (where we created a responder exchange that's not responding to anything), we need to explicitly close it out. This is not
    // expected in normal application logic.
    //
    exchange->Close();
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

    AddInvalidInvokeRequestData(apSuite, apContext, &commandSender);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);
    NL_TEST_ASSERT(apSuite, mockCommandSenderDelegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Command Sender sends malformed invoke request, this command is aysnc command, handler fails to process it and sends status
// report with invalid action
void TestCommandInteraction::TestCommandHandlerInvalidMessageAsync(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());
    asyncCommand = true;
    AddInvalidInvokeRequestData(apSuite, apContext, &commandSender);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Decrease CommandHandler refcount and send response
    asyncCommandHandle = nullptr;

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);
    NL_TEST_ASSERT(apSuite, mockCommandSenderDelegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandHandlerCommandEncodeExternalFailure(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(nullptr);
    System::PacketBufferHandle commandPacket;

    TestExchangeDelegate delegate;
    auto exchange = ctx.NewExchangeToAlice(&delegate, false);
    commandHandler.mExchangeCtx.Grab(exchange);

    auto path = MakeTestCommandPath();

    err = commandHandler.AddResponseData(ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId), BadFields());
    NL_TEST_ASSERT(apSuite, err != CHIP_NO_ERROR);
    err = commandHandler.AddStatus(ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId),
                                   Protocols::InteractionModel::Status::Failure);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.Finalize(commandPacket);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    chip::System::PacketBufferTLVReader reader;
    InvokeResponseMessage::Parser invokeResponseMessageParser;
    reader.Init(std::move(commandPacket));
    err = invokeResponseMessageParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeResponseMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif

    //
    // Ordinarily, the ExchangeContext will close itself on a responder exchange when unwinding back from an
    // OnMessageReceived callback and not having sent a subsequent message. Since that isn't the case in this artificial setup here
    // (where we created a responder exchange that's not responding to anything), we need to explicitly close it out. This is not
    // expected in normal application logic.
    //
    exchange->Close();
}

void TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple status code and command path
    ValidateCommandHandlerWithSendCommand(apSuite, apContext, true /*aNeedStatusCode=true*/);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    TestExchangeDelegate delegate;
    commandHandler.mExchangeCtx.Grab(ctx.NewExchangeToAlice(&delegate));
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
            commandHandler.mExchangeCtx.Grab(exchange);

            chip::isCommandDispatched = false;
            GenerateInvokeRequest(apSuite, apContext, commandDatabuf, messageIsTimed, kTestCommandIdNoData);
            Protocols::InteractionModel::Status status =
                commandHandler.ProcessInvokeRequest(std::move(commandDatabuf), transactionIsTimed);
            if (messageIsTimed != transactionIsTimed)
            {
                NL_TEST_ASSERT(apSuite, status == Protocols::InteractionModel::Status::UnsupportedAccess);
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

void TestCommandInteraction::TestCommandSenderCommandSuccessResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    AddInvokeRequestData(apSuite, apContext, &commandSender);
    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 1 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

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

void TestCommandInteraction::TestCommandHandlerRejectMultipleCommands(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    isCommandDispatched = false;
    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, &ctx.GetExchangeManager());

    {
        // Command ID is not important here, since the command handler should reject the commands without handling it.
        auto commandPathParams = MakeTestCommandPath(kTestCommandIdCommandSpecificResponse);

        commandSender.AllocateBuffer();

        // CommandSender does not support sending multiple commands with public API, so we craft a message manaully.
        for (int i = 0; i < 2; i++)
        {
            InvokeRequests::Builder & invokeRequests = commandSender.mInvokeRequestBuilder.GetInvokeRequests();
            CommandDataIB::Builder & invokeRequest   = invokeRequests.CreateCommandData();
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == invokeRequests.GetError());
            CommandPathIB::Builder & path = invokeRequest.CreatePath();
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == invokeRequest.GetError());
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == path.Encode(commandPathParams));
            NL_TEST_ASSERT(
                apSuite,
                CHIP_NO_ERROR ==
                    invokeRequest.GetWriter()->StartContainer(TLV::ContextTag(to_underlying(CommandDataIB::Tag::kFields)),
                                                              TLV::kTLVType_Structure, commandSender.mDataElementContainerType));
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == invokeRequest.GetWriter()->PutBoolean(chip::TLV::ContextTag(1), true));
            NL_TEST_ASSERT(apSuite,
                           CHIP_NO_ERROR == invokeRequest.GetWriter()->EndContainer(commandSender.mDataElementContainerType));
            NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == invokeRequest.EndOfCommandDataIB().GetError());
        }

        NL_TEST_ASSERT(apSuite,
                       CHIP_NO_ERROR == commandSender.mInvokeRequestBuilder.GetInvokeRequests().EndOfInvokeRequests().GetError());
        NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == commandSender.mInvokeRequestBuilder.EndOfInvokeRequestMessage().GetError());

        commandSender.MoveToState(app::CommandSender::State::AddedCommand);
    }

    err = commandSender.SendCommandRequest(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);
    NL_TEST_ASSERT(apSuite, !chip::isCommandDispatched);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
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

    // Mimick closure of the exchange that would happen on a session release and verify that releasing the handle there-after
    // is handled gracefully.
    asyncCommandHandle.Get()->mExchangeCtx->GetSessionHolder().Release();
    asyncCommandHandle.Get()->mExchangeCtx->OnSessionReleased();

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
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleCommandData", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleCommandData),
    NL_TEST_DEF("TestCommandHandlerCommandDataEncoding", chip::app::TestCommandInteraction::TestCommandHandlerCommandDataEncoding),
    NL_TEST_DEF("TestCommandHandlerCommandEncodeFailure", chip::app::TestCommandInteraction::TestCommandHandlerCommandEncodeFailure),
    NL_TEST_DEF("TestCommandHandlerCommandEncodeExternalFailure", chip::app::TestCommandInteraction::TestCommandHandlerCommandEncodeExternalFailure),
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleStatusCode", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedNotExistCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedEmptyDataMsg", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedEmptyDataMsg),
    NL_TEST_DEF("TestCommandHandlerRejectMultipleCommands", chip::app::TestCommandInteraction::TestCommandHandlerRejectMultipleCommands),

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    NL_TEST_DEF("TestCommandHandlerReleaseWithExchangeClosed", chip::app::TestCommandInteraction::TestCommandHandlerReleaseWithExchangeClosed),
#endif

    NL_TEST_DEF("TestCommandSenderCommandSuccessResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandSuccessResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandAsyncSuccessResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandAsyncSuccessResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandSpecificResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandSpecificResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandFailureResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandFailureResponseFlow),
    NL_TEST_DEF("TestCommandSenderAbruptDestruction", chip::app::TestCommandInteraction::TestCommandSenderAbruptDestruction),
    NL_TEST_DEF("TestCommandHandlerInvalidMessageSync", chip::app::TestCommandInteraction::TestCommandHandlerInvalidMessageSync),
    NL_TEST_DEF("TestCommandHandlerInvalidMessageAsync", chip::app::TestCommandInteraction::TestCommandHandlerInvalidMessageAsync),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestCommandInteraction",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestCommandInteraction()
{
    chip::gSuite = &sSuite;
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCommandInteraction)
