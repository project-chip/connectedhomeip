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

#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

namespace chip {
static System::Layer gSystemLayer;
static SecureSessionMgr gSessionManager;
static Messaging::ExchangeManager gExchangeManager;
static TransportMgr<Transport::UDP> gTransportManager;
static Transport::AdminId gAdminId = 0;

namespace app {
class TestCommandInteraction
{
public:
    static void TestCommandSenderWithSendCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendEmptyCommand(nlTestSuite * apSuite, void * apContext);
    static void TestCommandSenderWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendSimpleCommandData(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithSendEmptyResponse(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandlerWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext);

private:
    static void GenerateReceivedCommand(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload);
    static void AddCommandDataElement(nlTestSuite * apSuite, void * apContext, Command * apCommand, bool aNeedStatusCode,
                                      bool aIsEmptyResponse);
    static void ValidateCommandHandlerWithSendCommand(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode,
                                                      bool aIsEmptyResponse);
};

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle payload) override
    {}

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

void TestCommandInteraction::GenerateReceivedCommand(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload)
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
    commandPathBuilder.EndpointId(1).ClusterId(3).CommandId(4).EndOfCommandPath();
    NL_TEST_ASSERT(apSuite, commandPathBuilder.GetError() == CHIP_NO_ERROR);

    chip::TLV::TLVWriter * pWriter = commandDataElementBuilder.GetWriter();
    chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
    err = pWriter->StartContainer(chip::TLV::ContextTag(CommandDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure, dummyType);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = pWriter->EndContainer(dummyType);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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
                                                   bool aNeedStatusCode, bool aIsEmptyResponse)
{
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::app::CommandPathParams commandPathParams = { 1, // Endpoint
                                                       2, // GroupId
                                                       3, // ClusterId
                                                       4, // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };

    if (aNeedStatusCode)
    {
        if (aIsEmptyResponse)
        {
            apCommand->AddStatusCode(nullptr, Protocols::SecureChannel::GeneralStatusCode::kSuccess, Protocols::SecureChannel::Id,
                                     Protocols::SecureChannel::kProtocolCodeSuccess);
        }
        else
        {
            apCommand->AddStatusCode(&commandPathParams, Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                     Protocols::SecureChannel::Id, Protocols::SecureChannel::kProtocolCodeSuccess);
        }
    }
    else
    {
        err = apCommand->PrepareCommand(&commandPathParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        chip::TLV::TLVWriter * writer = apCommand->GetCommandDataElementTLVWriter();

        err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = apCommand->FinishCommand();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

void TestCommandInteraction::TestCommandSenderWithSendCommand(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = commandSender.Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AddCommandDataElement(apSuite, apContext, &commandSender, false, false);
    err = commandSender.SendCommandRequest(kTestDeviceNodeId, gAdminId);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateReceivedCommand(apSuite, apContext, buf);
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

    commandHandler.mpExchangeCtx = gExchangeManager.NewContext({ 0, 0, 0 }, nullptr);

    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);
    err = commandHandler.PrepareCommand(&commandPathParams);
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

    GenerateReceivedCommand(apSuite, apContext, buf);
    err = commandSender.ProcessCommandMessage(std::move(buf), Command::CommandRoleId::SenderId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    commandSender.Shutdown();
}

void TestCommandInteraction::ValidateCommandHandlerWithSendCommand(nlTestSuite * apSuite, void * apContext, bool aNeedStatusCode,
                                                                   bool aIsEmptyResponse)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler;
    err = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandHandler.mpExchangeCtx = gExchangeManager.NewContext({ 0, 0, 0 }, nullptr);
    TestExchangeDelegate delegate;
    commandHandler.mpExchangeCtx->SetDelegate(&delegate);

    AddCommandDataElement(apSuite, apContext, &commandHandler, aNeedStatusCode, aIsEmptyResponse);
    err = commandHandler.SendCommandResponse();
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);
}

void TestCommandInteraction::TestCommandHandlerWithSendSimpleCommandData(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple command data and command path
    ValidateCommandHandlerWithSendCommand(apSuite, apContext, false /*aNeedStatusCode=false*/, false /*aIsEmptyResponse=false*/);
}

void TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode(nlTestSuite * apSuite, void * apContext)
{
    // Send response which has simple status code and command path
    ValidateCommandHandlerWithSendCommand(apSuite, apContext, true /*aNeedStatusCode=true*/, false /*aIsEmptyResponse=false*/);
}

void TestCommandInteraction::TestCommandHandlerWithSendEmptyResponse(nlTestSuite * apSuite, void * apContext)
{
    // Send empty response, which has only simple status code without command path
    ValidateCommandHandlerWithSendCommand(apSuite, apContext, true /*aNeedStatusCode=true*/, true /*aIsEmptyResponse=true*/);
}

void TestCommandInteraction::TestCommandHandlerWithProcessReceivedMsg(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler;
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                                       = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    GenerateReceivedCommand(apSuite, apContext, commandDatabuf);
    err = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    commandHandler.Shutdown();
}

} // namespace app
} // namespace chip

namespace {

void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(chip::gAdminId, chip::kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::gSystemLayer.Init(nullptr);

    err = chip::gSessionManager.Init(chip::kTestDeviceNodeId, &chip::gSystemLayer, &chip::gTransportManager, &admins);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gExchangeManager.Init(&chip::gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestCommandSenderWithSendCommand", chip::app::TestCommandInteraction::TestCommandSenderWithSendCommand),
    NL_TEST_DEF("TestCommandHandlerWithSendEmptyCommand", chip::app::TestCommandInteraction::TestCommandHandlerWithSendEmptyCommand),
    NL_TEST_DEF("TestCommandSenderWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandSenderWithProcessReceivedMsg),
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleCommandData", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleCommandData),
    NL_TEST_DEF("TestCommandHandlerWithSendSimpleStatusCode", chip::app::TestCommandInteraction::TestCommandHandlerWithSendSimpleStatusCode),
    NL_TEST_DEF("TestCommandHandlerWithSendEmptyResponse", chip::app::TestCommandInteraction::TestCommandHandlerWithSendEmptyResponse),
    NL_TEST_DEF("TestCommandHandlerWithProcessReceivedMsg", chip::app::TestCommandInteraction::TestCommandHandlerWithProcessReceivedMsg),
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

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommandInteraction)
