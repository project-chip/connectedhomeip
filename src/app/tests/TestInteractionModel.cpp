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
 *      This file implements a test for  CHIP Interaction Model Message Def
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
chip::SecureSessionMgr gSessionManager;
chip::Messaging::ExchangeManager gExchangeManager;
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
const chip::Transport::AdminId gAdminId = 0;

namespace chip{
namespace app{
class TestInteractionModel
{
public:
    static void TestReadClient(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler(nlTestSuite * apSuite, void * apContext);
private:
    static void GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle &&aPayload);
};

void TestInteractionModel::GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle &&aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    ReportData::Builder reportDataBuilder;

    err = reportDataBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    reportDataBuilder.SuppressResponse(true);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    reportDataBuilder.MoreChunkedMessages(false);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    reportDataBuilder.EndOfReportData();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestInteractionModel::TestReadClient(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::app::ReadClient readClient;

    chip::System::PacketBufferHandle buf = chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize);
    err = readClient.Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.SendReadRequest(chip::kTestDeviceNodeId, gAdminId);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateReportData(apSuite, apContext, buf.Retain());

    err = readClient.ProcessReportData(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    readClient.Reset();
    readClient.Shutdown();
}

void TestInteractionModel::TestReadHandler(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::ReadHandler readHandler;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferHandle reportDatabuf = chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize);
    chip::System::PacketBufferHandle readRequestbuf = chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize);
    ReadRequest::Builder readRequestBuilder;
    readHandler.Init(&gExchangeManager, nullptr);

    GenerateReportData(apSuite, apContext, reportDatabuf.Retain());
    err = readHandler.SendReportData(std::move(reportDatabuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readRequestBuilder.EventNumber(1);
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    readRequestBuilder.EndOfReadRequest();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    err = writer.Finalize(&readRequestbuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readHandler.ProcessReadRequest(std::move(readRequestbuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    readHandler.Reset();
    readHandler.Shutdown();
}

}
}

namespace {
void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(gAdminId, chip::kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Initialize the CHIP stack.
    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gTransportManager.Init(
                chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gSessionManager.Init(chip::kTestDeviceNodeId, &chip::DeviceLayer::SystemLayer, &gTransportManager, &admins);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gExchangeManager.Init(chip::kTestDeviceNodeId, &gTransportManager, &gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("CheckReadClient", chip::app::TestInteractionModel::TestReadClient),
                NL_TEST_DEF("CheckReadHandler", chip::app::TestInteractionModel::TestReadHandler),
                NL_TEST_SENTINEL()
        };
// clang-format on
} // namespace

int TestEventLogging()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "InteractionMessage",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestEventLogging)
