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
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemLayerImpl.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

namespace chip {
static System::LayerImpl gSystemLayer;
static SecureSessionMgr gSessionManager;
static Messaging::ExchangeManager gExchangeManager;
static TransportMgr<Transport::UDP> gTransportManager;
static secure_channel::MessageCounterManager gMessageCounterManager;
static FabricIndex gFabricIndex = 0;
static bool isCommandDispatched = false;

namespace {
constexpr EndpointId kTestEndpointId = 1;
constexpr ClusterId kTestClusterId   = 3;
constexpr CommandId kTestCommandId   = 4;
} // namespace

namespace app {

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, CommandHandler * apCommandObj)
{
    chip::app::CommandPathParams commandPathParams = { aEndPointId, // Endpoint
                                                       0,           // GroupId
                                                       aClusterId,  // ClusterId
                                                       aCommandId,  // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };

    ChipLogDetail(Controller,
                  "Received Cluster Command: Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI " Endpoint=%" PRIx16,
                  ChipLogValueMEI(aClusterId), ChipLogValueMEI(aCommandId), aEndPointId);

    apCommandObj->AddStatusCode(commandPathParams, Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                Protocols::SecureChannel::Id, Protocols::InteractionModel::ProtocolCode::Success);

    chip::isCommandDispatched = true;
}

void DispatchSingleClusterResponseCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                          chip::TLV::TLVReader & aReader, CommandSender * apCommandObj)
{
    ChipLogDetail(Controller, "Received Cluster Command: Cluster=%" PRIx32 " Command=%" PRIx32 " Endpoint=%" PRIx16, aClusterId,
                  aCommandId, aEndPointId);
    // Nothing todo.
}

bool ServerClusterCommandExists(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId)
{
    // Mock cluster catalog, only support one command on one cluster on one endpoint.
    return (aEndPointId == kTestEndpointId && aClusterId == kTestClusterId && aCommandId == kTestCommandId);
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
    static void TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendEmptyResponse(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithProcessReceivedEmptyDataMsg(nlTestSuite * apSuite, void * apContext);

private:
    static void GenerateReceivedCommand(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                        bool aNeedCommandData, EndpointId aEndpointId = kTestEndpointId,
                                        ClusterId aClusterId = kTestClusterId, CommandId aCommandId = kTestCommandId);
    static void AddCommandDataElement(nlTestSuite * apSuite, void * apContext, Command * apCommand, bool aNeedStatusCode);
    static void ValidateCommandHandlerWithSendCommand(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode);
};

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

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

    CommandDataElement::Builder commandDataElementBuilder = commandList.CreateCommandDataElementBuilder();
    NL_TEST_ASSERT(apSuite, commandList.GetError() == CHIP_NO_ERROR);
    CommandPath::Builder commandPathBuilder = commandDataElementBuilder.CreateCommandPathBuilder();
    NL_TEST_ASSERT(apSuite, commandDataElementBuilder.GetError() == CHIP_NO_ERROR);
    commandPathBuilder.EndpointId(aEndpointId).ClusterId(aClusterId).CommandId(aCommandId).EndOfCommandPath();
    NL_TEST_ASSERT(apSuite, commandPathBuilder.GetError() == CHIP_NO_ERROR);

    if (aNeedCommandData)
    {
        chip::TLV::TLVWriter * pWriter = commandDataElementBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(CommandDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure,
                                      dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    commandDataElementBuilder.EndOfCommandDataElement();
    NL_TEST_ASSERT(apSuite, commandDataElementBuilder.GetError() == CHIP_NO_ERROR);

    commandList.EndOfCommandList();
    NL_TEST_ASSERT(apSuite, commandList.GetError() == CHIP_NO_ERROR);

    invokeCommandBuilder.EndOfInvokeCommand();
    NL_TEST_ASSERT(apSuite, invokeCommandBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestCommandInteraction::AddCommandDataElement(nlTestSuite * apSuite, void * apContext, Command * apCommand,
                                                   bool aNeedStatusCode)
{
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::app::CommandPathParams commandPathParams = { 1, // Endpoint
                                                       2, // GroupId
                                                       3, // ClusterId
                                                       4, // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };

    if (aNeedStatusCode)
    {
        apCommand->AddStatusCode(commandPathParams, Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                 Protocols::SecureChannel::Id, Protocols::InteractionModel::ProtocolCode::Success);
    }
    else
    {
        err = apCommand->PrepareCommand(commandPathParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        chip::TLV::TLVWriter * writer = apCommand->GetCommandDataElementTLVWriter();

        err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = apCommand->FinishCommand();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

void TestCommandInteraction::TestCommandSenderWithWrongState(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::app::CommandPathParams commandPathParams = { 1, // Endpoint
                                                       2, // GroupId
                                                       3, // ClusterId
                                                       4, // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::CommandSender commandSender;

    err = commandSender.PrepareCommand(commandPathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    commandSender.Shutdown();

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = commandSender.Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandSender.SendCommandRequest(kTestDeviceNodeId, gFabricIndex, Optional<SessionHandle>::Missing());
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);
}

void TestCommandInteraction::TestCommandHandlerWithWrongState(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::app::CommandPathParams commandPathParams = { 1, // Endpoint
                                                       2, // GroupId
                                                       3, // ClusterId
                                                       4, // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::CommandHandler commandHandler;

    err = commandHandler.PrepareCommand(commandPathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);
    commandHandler.Shutdown();

    err = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    commandHandler.mpExchangeCtx = gExchangeManager.NewContext(SessionHandle(0, 0, 0, 0), nullptr);
    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);
    err = commandHandler.SendCommandResponse();
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);
    commandHandler.Shutdown();
}

void TestCommandInteraction::TestCommandSenderWithSendCommand(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = commandSender.Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AddCommandDataElement(apSuite, apContext, &commandSender, false);
    err = commandSender.SendCommandRequest(kTestDeviceNodeId, gFabricIndex, Optional<SessionHandle>::Missing());
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateReceivedCommand(apSuite, apContext, buf, true /*aNeedCommandData*/);
    err = commandSender.ProcessCommandMessage(std::move(buf), Command::CommandRoleId::SenderId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    commandSender.Shutdown();
}

void TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::app::CommandPathParams commandPathParams = { 1, // Endpoint
                                                       2, // GroupId
                                                       3, // ClusterId
                                                       4, // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::CommandHandler commandHandler;
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                                       = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandHandler.mpExchangeCtx = gExchangeManager.NewContext(SessionHandle(0, 0, 0, 0), nullptr);

    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);
    err = commandHandler.PrepareCommand(commandPathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.FinishCommand();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = commandHandler.SendCommandResponse();
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);
    commandHandler.Shutdown();
}

void TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = commandSender.Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    GenerateReceivedCommand(apSuite, apContext, buf, true /*aNeedCommandData*/);
    err = commandSender.ProcessCommandMessage(std::move(buf), Command::CommandRoleId::SenderId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    commandSender.Shutdown();
}

void TestCommandInteraction::ValidateCommandHandlerWithSendCommand(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler;
    System::PacketBufferHandle commandPacket;
    err = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandHandler.mpExchangeCtx = gExchangeManager.NewContext(SessionHandle(0, 0, 0, 0), nullptr);
    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);

    AddCommandDataElement(apSuite, apContext, &commandHandler, aNeedStatusCode);
    err = commandHandler.FinalizeCommandsMessage(commandPacket);
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

void TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple status code and command path
    ValidateCommandHandlerWithSendCommand(apSuite, apContext, true /*aNeedStatusCode=true*/);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler;
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                                       = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    GenerateReceivedCommand(apSuite, apContext, commandDatabuf, true /*aNeedCommandData*/);
    err = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    commandHandler.Shutdown();
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler;
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                                       = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Use some invalid endpoint / cluster / command.
    GenerateReceivedCommand(apSuite, apContext, commandDatabuf, false /*aNeedCommandData*/, 0xDE /* endpoint */,
                            0xADBE /* cluster */, 0xEF /* command */);

    // TODO: Need to find a way to get the response instead of only check if a function on key path is called.
    // We should not reach CommandDispatch if requested command does not exist.
    chip::isCommandDispatched = false;
    err                       = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, !chip::isCommandDispatched);
    commandHandler.Shutdown();
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedEmptyDataMsg(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler;
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                                       = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    chip::isCommandDispatched = false;
    GenerateReceivedCommand(apSuite, apContext, commandDatabuf, false /*aNeedCommandData*/);
    err = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && chip::isCommandDispatched);
    commandHandler.Shutdown();
}

} // namespace app
} // namespace chip

namespace {

void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::FabricTable fabrics;

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::gSystemLayer.Init();

    err = chip::gSessionManager.Init(&chip::gSystemLayer, &chip::gTransportManager, &fabrics, &chip::gMessageCounterManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gExchangeManager.Init(&chip::gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gMessageCounterManager.Init(&chip::gExchangeManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestCommandSenderWithWrongState", chip::app::TestCommandInteraction::TestCommandSenderWithWrongState),
    NL_TEST_DEF("TestCommandHandlerWithWrongState", chip::app::TestCommandInteraction::TestCommandHandlerWithWrongState),
    NL_TEST_DEF("TestCommandSenderWithSendCommand", chip::app::TestCommandInteraction::TestCommandSenderWithSendCommand),
    NL_TEST_DEF("TestCommandHandlerWithSendEmptyCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand),
    NL_TEST_DEF("TestCommandSenderWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg),
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleCommandData", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleCommandData),
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleStatusCode", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedMsg),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedNotExistCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedNotExistCommand),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedEmptyDataMsg", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedEmptyDataMsg),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestCommandInteraction()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "CommandInteraction",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    chip::gSystemLayer.Shutdown();

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommandInteraction)
