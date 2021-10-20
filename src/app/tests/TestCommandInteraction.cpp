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

#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemLayerImpl.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionHandle.h>
#include <transport/SessionManager.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

using namespace chip;
namespace chip {

namespace {
chip::TransportMgrBase gTransportManager;
chip::Test::LoopbackTransport gLoopback;
chip::Test::IOContext gIOContext;
Messaging::ExchangeManager * gExchangeManager;
secure_channel::MessageCounterManager gMessageCounterManager;
FabricIndex gFabricIndex = 0;
bool isCommandDispatched = false;

using TestContext = chip::Test::MessagingContext;
TestContext sContext;
bool sendResponse = true;

constexpr EndpointId kTestEndpointId                      = 1;
constexpr ClusterId kTestClusterId                        = 3;
constexpr CommandId kTestCommandId                        = 4;
constexpr CommandId kTestCommandIdCommandSpecificResponse = 5;
constexpr CommandId kTestNonExistCommandId                = 0;
} // namespace

namespace app {

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    ChipLogDetail(Controller,
                  "Received Cluster Command: Endpoint=%" PRIx16 " Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  aCommandPath.mEndpointId, ChipLogValueMEI(aCommandPath.mClusterId), ChipLogValueMEI(aCommandPath.mCommandId));

    if (sendResponse)
    {
        if (aCommandPath.mCommandId == kTestCommandId)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Success);
        }
        else
        {
            CommandPathParams responseParams = { aCommandPath.mEndpointId, /* groupId */ 0, aCommandPath.mClusterId,
                                                 aCommandPath.mCommandId, (chip::app::CommandPathFlags::kEndpointIdValid) };

            apCommandObj->PrepareCommand(responseParams);
            chip::TLV::TLVWriter * writer = apCommandObj->GetCommandDataIBTLVWriter();
            writer->PutBoolean(chip::TLV::ContextTag(1), true);
            apCommandObj->FinishCommand();
        }
    }

    chip::isCommandDispatched = true;
}

void DispatchSingleClusterResponseCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                          CommandSender * apCommandObj)
{
    ChipLogDetail(Controller,
                  "Received Cluster Command: Endpoint=%" PRIx16 " Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  aCommandPath.mEndpointId, ChipLogValueMEI(aCommandPath.mClusterId), ChipLogValueMEI(aCommandPath.mCommandId));
    // Nothing todo.
}

class MockCommandSenderCallback : public CommandSender::Callback
{
public:
    void OnResponse(chip::app::CommandSender * apCommandSender, const chip::app::ConcreteCommandPath & aPath,
                    chip::TLV::TLVReader * aData) override
    {
        IgnoreUnusedVariable(apCommandSender);
        IgnoreUnusedVariable(aData);
        ChipLogDetail(Controller, "Received Cluster Command: Cluster=%" PRIx32 " Command=%" PRIx32 " Endpoint=%" PRIx16,
                      aPath.mClusterId, aPath.mCommandId, aPath.mEndpointId);
        onResponseCalledTimes++;
    }
    void OnError(const chip::app::CommandSender * apCommandSender, chip::Protocols::InteractionModel::Status aStatus,
                 CHIP_ERROR aError) override
    {
        ChipLogError(Controller, "OnError happens with %" PRIx16 " %" CHIP_ERROR_FORMAT, to_underlying(aStatus), aError.Format());
        onErrorCalledTimes++;
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
} mockCommandSenderDelegate;

class MockCommandHandlerCallback : public CommandHandler::Callback
{
public:
    void OnDone(chip::app::CommandHandler * apCommandHandler) final { onFinalCalledTimes++; }

    int onFinalCalledTimes = 0;
} mockCommandHandlerDelegate;

bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // Mock cluster catalog, only support one command on one cluster on one endpoint.
    return (aCommandPath.mEndpointId == kTestEndpointId && aCommandPath.mClusterId == kTestClusterId &&
            aCommandPath.mCommandId != kTestNonExistCommandId);
}

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
    static void TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendEmptyResponse(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithProcessReceivedEmptyDataMsg(nlTestSuite * apSuite, void * apContext);

    static void TestCommandSenderCommandSuccessResponseFlow(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderCommandFailureResponseFlow(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderCommandSpecificResponseFlow(nlTestSuite * apSuite, void * apContext);

    static void TestCommandSenderAbruptDestruction(nlTestSuite * apSuite, void * apContext);

    static size_t GetNumActiveHandlerObjects()
    {
        return chip::app::InteractionModelEngine::GetInstance()->mCommandHandlerObjs.Allocated();
    }

private:
    static void GenerateReceivedCommand(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                        bool aNeedCommandData, EndpointId aEndpointId = kTestEndpointId,
                                        ClusterId aClusterId = kTestClusterId, CommandId aCommandId = kTestCommandId);
    static void AddCommandDataIB(nlTestSuite * apSuite, void * apContext, Command * apCommand, bool aNeedStatusCode,
                                      CommandId aCommandId = kTestCommandId);
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

CommandPathParams MakeTestCommandPath(CommandId aCommandId = kTestCommandId)
{
    return CommandPathParams(kTestEndpointId, 0, kTestClusterId, aCommandId, (chip::app::CommandPathFlags::kEndpointIdValid));
}

void TestCommandInteraction::GenerateReceivedCommand(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                                     bool aNeedCommandData, EndpointId aEndpointId, ClusterId aClusterId,
                                                     CommandId aCommandId)

{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeCommand::Builder invokeCommandBuilder;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    err = invokeCommandBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    CommandList::Builder commandList = invokeCommandBuilder.CreateCommandListBuilder();
    NL_TEST_ASSERT(apSuite, invokeCommandBuilder.GetError() == CHIP_NO_ERROR);

    CommandDataIB::Builder commandDataIBBuilder = commandList.CreateCommandDataIBBuilder();
    NL_TEST_ASSERT(apSuite, commandList.GetError() == CHIP_NO_ERROR);
    CommandPath::Builder commandPathBuilder = commandDataIBBuilder.CreateCommandPathBuilder();
    NL_TEST_ASSERT(apSuite, commandDataIBBuilder.GetError() == CHIP_NO_ERROR);
    commandPathBuilder.EndpointId(aEndpointId).ClusterId(aClusterId).CommandId(aCommandId).EndOfCommandPath();
    NL_TEST_ASSERT(apSuite, commandPathBuilder.GetError() == CHIP_NO_ERROR);

    if (aNeedCommandData)
    {
        chip::TLV::TLVWriter * pWriter = commandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(CommandDataIB::kCsTag_Data), chip::TLV::kTLVType_Structure,
                                      dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    commandDataIBBuilder.EndOfCommandDataIB();
    NL_TEST_ASSERT(apSuite, commandDataIBBuilder.GetError() == CHIP_NO_ERROR);

    commandList.EndOfCommandList();
    NL_TEST_ASSERT(apSuite, commandList.GetError() == CHIP_NO_ERROR);

    invokeCommandBuilder.EndOfInvokeCommand();
    NL_TEST_ASSERT(apSuite, invokeCommandBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::AddCommandDataIB(nlTestSuite * apSuite, void * apContext, Command * apCommand,
                                                   bool aNeedStatusCode, CommandId aCommandId)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    auto commandPathParams = MakeTestCommandPath(aCommandId);

    if (aNeedStatusCode)
    {
        chip::app::ConcreteCommandPath commandPath(1, // Endpoint
                                                   3, // ClusterId
                                                   4  // CommandId
        );
        apCommand->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        err = apCommand->PrepareCommand(commandPathParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        chip::TLV::TLVWriter * writer = apCommand->GetCommandDataIBTLVWriter();

        err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = apCommand->FinishCommand();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

void TestCommandInteraction::TestCommandSenderWithWrongState(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, gExchangeManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandSender.SendCommandRequest(kTestDeviceNodeId, gFabricIndex, Optional<SessionHandle>::Missing());
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);
}

void TestCommandInteraction::TestCommandHandlerWithWrongState(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    auto commandPathParams = MakeTestCommandPath();

    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);

    err = commandHandler.PrepareCommand(commandPathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    auto exchangeCtx             = gExchangeManager->NewContext(SessionHandle(0, 0, 0, 0), nullptr);
    commandHandler.mpExchangeCtx = exchangeCtx;
    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);
    err = commandHandler.SendCommandResponse();

    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);
}

void TestCommandInteraction::TestCommandSenderWithSendCommand(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, gExchangeManager);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AddCommandDataIB(apSuite, apContext, &commandSender, false);
    err = commandSender.SendCommandRequest(kTestDeviceNodeId, gFabricIndex, Optional<SessionHandle>::Missing());
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateReceivedCommand(apSuite, apContext, buf, true /*aNeedCommandData*/);
    err = commandSender.ProcessCommandMessage(std::move(buf), Command::CommandRoleId::SenderId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    auto commandPathParams = MakeTestCommandPath();

    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    auto exchangeCtx             = gExchangeManager->NewContext(SessionHandle(0, 0, 0, 0), nullptr);
    commandHandler.mpExchangeCtx = exchangeCtx;

    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);
    err = commandHandler.PrepareCommand(commandPathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.FinishCommand();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.SendCommandResponse();
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);
}

void TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender(&mockCommandSenderDelegate, gExchangeManager);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateReceivedCommand(apSuite, apContext, buf, true /*aNeedCommandData*/);
    err = commandSender.ProcessCommandMessage(std::move(buf), Command::CommandRoleId::SenderId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::ValidateCommandHandlerWithSendCommand(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandPacket;

    auto exchangeCtx = gExchangeManager->NewContext(SessionHandle(0, 0, 0, 0), nullptr);

    commandHandler.mpExchangeCtx = exchangeCtx;
    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);

    AddCommandDataIB(apSuite, apContext, &commandHandler, aNeedStatusCode);
    err = commandHandler.Finalize(commandPacket);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    chip::System::PacketBufferTLVReader reader;
    InvokeCommand::Parser invokeCommandParser;
    reader.Init(std::move(commandPacket));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeCommandParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeCommandParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
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

void TestCommandInteraction::TestCommandHandlerCommandDataEncoding(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(nullptr);
    System::PacketBufferHandle commandPacket;

    commandHandler.mpExchangeCtx = gExchangeManager->NewContext(SessionHandle(0, 0, 0, 0), nullptr);
    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);

    auto path = MakeTestCommandPath();

    err = commandHandler.AddResponseData(ConcreteCommandPath(path.mEndpointId, path.mClusterId, path.mCommandId), Fields());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.Finalize(commandPacket);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    chip::System::PacketBufferTLVReader reader;
    InvokeCommand::Parser invokeCommandParser;
    reader.Init(std::move(commandPacket));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeCommandParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = invokeCommandParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple status code and command path
    ValidateCommandHandlerWithSendCommand(apSuite, apContext, true /*aNeedStatusCode=true*/);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    GenerateReceivedCommand(apSuite, apContext, commandDatabuf, true /*aNeedCommandData*/);
    err = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    // Use some invalid endpoint / cluster / command.
    GenerateReceivedCommand(apSuite, apContext, commandDatabuf, false /*aNeedCommandData*/, 0xDE /* endpoint */,
                            0xADBE /* cluster */, 0xEF /* command */);

    // TODO: Need to find a way to get the response instead of only check if a function on key path is called.
    // We should not reach CommandDispatch if requested command does not exist.
    chip::isCommandDispatched = false;
    err                       = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, !chip::isCommandDispatched);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedEmptyDataMsg(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler(&mockCommandHandlerDelegate);
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    chip::isCommandDispatched = false;

    GenerateReceivedCommand(apSuite, apContext, commandDatabuf, false /*aNeedCommandData*/);

    err = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && chip::isCommandDispatched);
}

void TestCommandInteraction::TestCommandSenderCommandSuccessResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, gExchangeManager);

    AddCommandDataIB(apSuite, apContext, &commandSender, false);
    err = commandSender.SendCommandRequest(0, 0, Optional<SessionHandle>(ctx.GetSessionBobToAlice()));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 1 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderCommandSpecificResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, gExchangeManager);

    AddCommandDataIB(apSuite, apContext, &commandSender, false, kTestCommandIdCommandSpecificResponse);
    err = commandSender.SendCommandRequest(0, 0, Optional<SessionHandle>(ctx.GetSessionBobToAlice()));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 1 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderCommandFailureResponseFlow(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    mockCommandSenderDelegate.ResetCounter();
    app::CommandSender commandSender(&mockCommandSenderDelegate, gExchangeManager);

    AddCommandDataIB(apSuite, apContext, &commandSender, false, kTestNonExistCommandId);
    err = commandSender.SendCommandRequest(0, 0, Optional<SessionHandle>(ctx.GetSessionBobToAlice()));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite,
                   mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 1 &&
                       mockCommandSenderDelegate.onErrorCalledTimes == 1);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestCommandSenderAbruptDestruction(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    //
    // Don't send back a response, just keep the CommandHandler
    // hanging to give us enough time to do what we want with the CommandSender object.
    //
    sendResponse = false;

    mockCommandSenderDelegate.ResetCounter();

    {
        app::CommandSender commandSender(&mockCommandSenderDelegate, gExchangeManager);

        AddCommandDataIB(apSuite, apContext, &commandSender, false, kTestCommandIdCommandSpecificResponse);
        err = commandSender.SendCommandRequest(0, 0, Optional<SessionHandle>(ctx.GetSessionBobToAlice()));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        //
        // No callbacks should be invoked yet - let's validate that.
        //
        NL_TEST_ASSERT(apSuite,
                       mockCommandSenderDelegate.onResponseCalledTimes == 0 && mockCommandSenderDelegate.onFinalCalledTimes == 0 &&
                           mockCommandSenderDelegate.onErrorCalledTimes == 0);

        NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 1);
    }

    //
    // Upon the sender being destructed by the application, our exchange should get cleaned up too.
    //
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);

    NL_TEST_ASSERT(apSuite, GetNumActiveHandlerObjects() == 0);
}

} // namespace app
} // namespace chip

namespace {
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestCommandSenderWithWrongState", chip::app::TestCommandInteraction::TestCommandSenderWithWrongState),
    NL_TEST_DEF("TestCommandHandlerWithWrongState", chip::app::TestCommandInteraction::TestCommandHandlerWithWrongState),
    NL_TEST_DEF("TestCommandSenderWithSendCommand", chip::app::TestCommandInteraction::TestCommandSenderWithSendCommand),
    NL_TEST_DEF("TestCommandHandlerWithSendEmptyCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand),
    NL_TEST_DEF("TestCommandSenderWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg),
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleCommandData", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleCommandData),
    NL_TEST_DEF("TestCommandHandlerCommandDataEncoding", chip::app::TestCommandInteraction::TestCommandHandlerCommandDataEncoding),
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleStatusCode", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedMsg),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedNotExistCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedEmptyDataMsg", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedEmptyDataMsg),

    NL_TEST_DEF("TestCommandSenderCommandSuccessResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandSuccessResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandSpecificResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandSpecificResponseFlow),
    NL_TEST_DEF("TestCommandSenderCommandFailureResponseFlow", chip::app::TestCommandInteraction::TestCommandSenderCommandFailureResponseFlow),
    NL_TEST_DEF("TestCommandSenderAbruptDestruction", chip::app::TestCommandInteraction::TestCommandSenderAbruptDestruction),
    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
        "TestReadInteraction",
        &sTests[0],
        Initialize,
        Finalize
};
// clang-format on

int Initialize(void * aContext)
{
    // Initialize System memory and resources
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gIOContext.Init(&sSuite) == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gTransportManager.Init(&gLoopback) == CHIP_NO_ERROR, FAILURE);

    auto * ctx = static_cast<TestContext *>(aContext);
    VerifyOrReturnError(ctx->Init(&sSuite, &gTransportManager, &gIOContext) == CHIP_NO_ERROR, FAILURE);

    gTransportManager.SetSessionManager(&ctx->GetSecureSessionManager());
    gExchangeManager = &ctx->GetExchangeManager();
    VerifyOrReturnError(
        chip::app::InteractionModelEngine::GetInstance()->Init(&ctx->GetExchangeManager(), nullptr) == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

int Finalize(void * aContext)
{
    // Shutdown will ensure no leaked exchange context.
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

int TestCommandInteraction()
{
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommandInteraction)
