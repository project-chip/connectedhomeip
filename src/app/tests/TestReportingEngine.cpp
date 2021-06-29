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
 *      This file implements unit tests for CHIP Interaction Model Reporting Engine
 *
 */

#include <app/InteractionModelEngine.h>
#include <app/reporting/Engine.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
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
static secure_channel::MessageCounterManager gMessageCounterManager;
static const Transport::AdminId gAdminId = 0;
constexpr ClusterId kTestClusterId       = 6;
constexpr EndpointId kTestEndpointId     = 1;
constexpr chip::FieldId kTestFieldId1    = 1;
constexpr chip::FieldId kTestFieldId2    = 2;
constexpr uint8_t kTestFieldValue1       = 1;
constexpr uint8_t kTestFieldValue2       = 2;

namespace app {
CHIP_ERROR ReadSingleClusterData(AttributePathParams & aAttributePathParams, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(aAttributePathParams.mClusterId == kTestClusterId && aAttributePathParams.mEndpointId == kTestEndpointId,
                 err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(apWriter != nullptr, /* no op */);

    if (aAttributePathParams.mFieldId == kTestFieldId1)
    {
        err = apWriter->Put(TLV::ContextTag(kTestFieldId1), kTestFieldValue1);
        SuccessOrExit(err);
    }
    if (aAttributePathParams.mFieldId == kTestFieldId2)
    {
        err = apWriter->Put(TLV::ContextTag(kTestFieldId2), kTestFieldValue2);
        SuccessOrExit(err);
    }

exit:
    ChipLogFunctError(err);
    return err;
}

namespace reporting {
class TestReportingEngine
{
public:
    static void TestBuildAndSendSingleReportData(nlTestSuite * apSuite, void * apContext);
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

void TestReportingEngine::TestBuildAndSendSingleReportData(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::ReadHandler readHandler;
    Engine reportingEngine;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequest::Builder readRequestBuilder;
    AttributePathList::Builder attributePathListBuilder;
    AttributePath::Builder attributePathBuilder;

    err = InteractionModelEngine::GetInstance()->Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    Messaging::ExchangeContext * exchangeCtx = gExchangeManager.NewContext({ 0, 0, 0 }, nullptr);
    TestExchangeDelegate delegate;
    exchangeCtx->SetDelegate(&delegate);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    attributePathListBuilder = readRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder =
        attributePathBuilder.NodeId(1).EndpointId(kTestEndpointId).ClusterId(kTestClusterId).FieldId(0).EndOfAttributePath();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    attributePathListBuilder.EndOfAttributePathList();
    readRequestBuilder.EventNumber(1);
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    readRequestBuilder.EndOfReadRequest();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    err = writer.Finalize(&readRequestbuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    readHandler.OnReadRequest(exchangeCtx, std::move(readRequestbuf));
    reportingEngine.Init();
    err = reportingEngine.BuildAndSendSingleReportData(&readHandler);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);
}
} // namespace reporting
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

    err = chip::gSessionManager.Init(chip::kTestDeviceNodeId, &chip::gSystemLayer, &chip::gTransportManager, &admins,
                                     &chip::gMessageCounterManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gExchangeManager.Init(&chip::gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gMessageCounterManager.Init(&chip::gExchangeManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("CheckBuildAndSendSingleReportData", chip::app::reporting::TestReportingEngine::TestBuildAndSendSingleReportData),
                NL_TEST_SENTINEL()
        };
// clang-format on
} // namespace

int TestReportingEngine()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestReportingEngine",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReportingEngine)
