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
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemLayerImpl.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlunit-test.h>

using TestContext = chip::Test::MessagingContext;

namespace chip {

constexpr ClusterId kTestClusterId        = 6;
constexpr EndpointId kTestEndpointId      = 1;
constexpr chip::AttributeId kTestFieldId1 = 1;
constexpr chip::AttributeId kTestFieldId2 = 2;
constexpr uint8_t kTestFieldValue1        = 1;
constexpr uint8_t kTestFieldValue2        = 2;

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
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

void TestReportingEngine::TestBuildAndSendSingleReportData(nlTestSuite * apSuite, void * aContext)
{
    auto * ctx     = static_cast<TestContext *>(aContext);
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::ReadHandler readHandler;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequest::Builder readRequestBuilder;
    AttributePathList::Builder attributePathListBuilder;
    AttributePath::Builder attributePathBuilder;

    err = InteractionModelEngine::GetInstance()->Init(&ctx->GetExchangeManager(), nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    Messaging::ExchangeContext * exchangeCtx = ctx->GetExchangeManager().NewContext(SessionHandle(0, 0, 0, 0), nullptr);
    TestExchangeDelegate delegate;
    exchangeCtx->SetDelegate(&delegate);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    attributePathListBuilder = readRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder = attributePathBuilder.NodeId(1)
                               .EndpointId(kTestEndpointId)
                               .ClusterId(kTestClusterId)
                               .FieldId(kTestFieldId1)
                               .EndOfAttributePath();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);

    attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder = attributePathBuilder.NodeId(1)
                               .EndpointId(kTestEndpointId)
                               .ClusterId(kTestClusterId)
                               .FieldId(kTestFieldId2)
                               .EndOfAttributePath();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    attributePathListBuilder.EndOfAttributePathList();

    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    readRequestBuilder.EndOfReadRequest();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    err = writer.Finalize(&readRequestbuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readHandler.Init(&ctx->GetExchangeManager(), nullptr, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);
    readHandler.OnReadInitialRequest(std::move(readRequestbuf));
    err = InteractionModelEngine::GetInstance()->GetReportingEngine().BuildAndSendSingleReportData(&readHandler);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);
}

} // namespace reporting
} // namespace app
} // namespace chip

namespace {

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("CheckBuildAndSendSingleReportData", chip::app::reporting::TestReportingEngine::TestBuildAndSendSingleReportData),
                NL_TEST_SENTINEL()
        };
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "TestReportingEngine",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

chip::TransportMgrBase gTransportManager;
chip::Test::LoopbackTransport gLoopback;
chip::Test::IOContext gIOContext;

int Initialize(void * aContext)
{
    // Initialize System memory and resources
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gIOContext.Init(&sSuite) == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gTransportManager.Init(&gLoopback) == CHIP_NO_ERROR, FAILURE);

    auto * ctx = static_cast<TestContext *>(aContext);
    VerifyOrReturnError(ctx->Init(&sSuite, &gTransportManager, &gIOContext) == CHIP_NO_ERROR, FAILURE);

    gTransportManager.SetSessionManager(&ctx->GetSecureSessionManager());
    return SUCCESS;
}

int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

int TestReportingEngine()
{
    TestContext sContext;
    nlTestRunner(&sSuite, &sContext);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReportingEngine)
