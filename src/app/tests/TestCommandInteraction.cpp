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
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/PASESession.h>
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
    static void TestCommandSender(nlTestSuite * apSuite, void * apContext);
    static void TestCommandHandler(nlTestSuite * apSuite, void * apContext);

private:
    static void GenerateCommandData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload);
};

void TestCommandInteraction::GenerateCommandData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload)
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

void TestCommandInteraction::TestCommandSender(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandSender commandSender;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = commandSender.Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = commandSender.SendCommandRequest(kTestDeviceNodeId, gAdminId);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateCommandData(apSuite, apContext, buf);

    err = commandSender.ProcessCommandMessage(std::move(buf), Command::CommandRoleId::SenderId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandSender.Shutdown();
}

void TestCommandInteraction::TestCommandHandler(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandHandler commandHandler;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle commandDatabuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                                       = commandHandler.Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    GenerateCommandData(apSuite, apContext, commandDatabuf);

    err = commandHandler.ProcessCommandMessage(std::move(commandDatabuf), Command::CommandRoleId::HandlerId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
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
    NL_TEST_DEF("CheckCommandSender", chip::app::TestCommandInteraction::TestCommandSender),
    NL_TEST_DEF("CheckCommandHandler", chip::app::TestCommandInteraction::TestCommandHandler),
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
