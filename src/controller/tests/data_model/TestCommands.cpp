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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app::Clusters;

namespace {
chip::TransportMgrBase gTransportManager;
chip::Test::LoopbackTransport gLoopback;
chip::Test::IOContext gIOContext;
chip::Messaging::ExchangeManager * gExchangeManager;
secure_channel::MessageCounterManager gMessageCounterManager;

using TestContext = chip::Test::MessagingContext;
TestContext sContext;

constexpr EndpointId kTestEndpointId = 1;

enum ResponseDirective
{
    kSendDataResponse,
    kSendSuccessStatusCode,
    kSendError
};

ResponseDirective responseDirective;

} // namespace

namespace chip {
namespace app {

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    ChipLogDetail(Controller,
                  "Received Cluster Command: Endpoint=%" PRIx16 " Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  aCommandPath.mEndpointId, ChipLogValueMEI(aCommandPath.mClusterId), ChipLogValueMEI(aCommandPath.mCommandId));

    if (aCommandPath.mClusterId == TestCluster::Id &&
        aCommandPath.mCommandId == TestCluster::Commands::TestSimpleArgumentRequest::Type::GetCommandId())
    {
        TestCluster::Commands::TestSimpleArgumentRequest::DecodableType dataRequest;

        if (DataModel::Decode(aReader, dataRequest) != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to decode the request");
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Failure);
            return;
        }

        if (responseDirective == kSendDataResponse)
        {
            TestCluster::Commands::TestStructArrayArgumentResponse::Type dataResponse;
            TestCluster::Structs::NestedStructList::Type nestedStructList[4];

            uint8_t i = 0;
            for (auto & item : nestedStructList)
            {
                item.a   = i;
                item.b   = false;
                item.c.a = i;
                item.c.b = true;
                i++;
            }

            dataResponse.arg1 = nestedStructList;
            dataResponse.arg6 = true;

            apCommandObj->AddResponseData(aCommandPath, dataResponse);
        }
        else if (responseDirective == kSendSuccessStatusCode)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Success);
        }
        else if (responseDirective == kSendError)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Failure);
        }
    }
}

bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // Mock cluster catalog, only support one command on one cluster on one endpoint.
    return (aCommandPath.mEndpointId == kTestEndpointId && aCommandPath.mClusterId == TestCluster::Id);
}

CHIP_ERROR ReadSingleClusterData(const ConcreteAttributePath & aPath, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace app
} // namespace chip

namespace {

class TestCommandInteraction
{
public:
    TestCommandInteraction() {}
    static void TestDataResponse(nlTestSuite * apSuite, void * apContext);
    static void TestSuccessNoDataResponse(nlTestSuite * apSuite, void * apContext);
    static void TestFailure(nlTestSuite * apSuite, void * apContext);

private:
};

void TestCommandInteraction::TestDataResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    TestCluster::Commands::TestSimpleArgumentRequest::Type request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;

    request.arg1 = true;

    auto onSuccessCb =
        [apSuite, &onSuccessWasCalled](const app::ConcreteCommandPath & commandPath,
                                       const TestCluster::Commands::TestStructArrayArgumentResponse::DecodableType & dataResponse) {
            uint8_t i;

            i         = 0;
            auto iter = dataResponse.arg1.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();

                NL_TEST_ASSERT(apSuite, item.a == i);
                NL_TEST_ASSERT(apSuite, item.b == false);
                NL_TEST_ASSERT(apSuite, item.c.a == i);
                NL_TEST_ASSERT(apSuite, item.c.b == true);
                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetStatus() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(apSuite, dataResponse.arg6 == true);

            onSuccessWasCalled = true;
        };

    auto onFailureCb = [&onFailureWasCalled](Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError) {
        onFailureWasCalled = true;
    };

    responseDirective = kSendDataResponse;

    chip::Controller::InvokeCommandRequest<TestCluster::Commands::TestStructArrayArgumentResponse::DecodableType>(
        gExchangeManager, sessionHandle, kTestEndpointId, request, onSuccessCb, onFailureCb);

    NL_TEST_ASSERT(apSuite, onSuccessWasCalled && !onFailureWasCalled);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestSuccessNoDataResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    TestCluster::Commands::TestSimpleArgumentRequest::Type request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;

    request.arg1 = true;

    auto onSuccessCb = [&onSuccessWasCalled](const app::ConcreteCommandPath & commandPath,
                                             const chip::app::DataModel::NullObjectType & dataResponse) {
        onSuccessWasCalled = true;
    };

    auto onFailureCb = [&onFailureWasCalled](Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError) {
        onFailureWasCalled = true;
    };

    responseDirective = kSendSuccessStatusCode;

    chip::Controller::InvokeCommandRequest(gExchangeManager, sessionHandle, kTestEndpointId, request, onSuccessCb, onFailureCb);

    NL_TEST_ASSERT(apSuite, onSuccessWasCalled && !onFailureWasCalled);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestFailure(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    TestCluster::Commands::TestSimpleArgumentRequest::Type request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;

    request.arg1 = true;

    auto onSuccessCb = [&onSuccessWasCalled](const app::ConcreteCommandPath & commandPath,
                                             const chip::app::DataModel::NullObjectType & dataResponse) {
        onSuccessWasCalled = true;
    };

    auto onFailureCb = [&onFailureWasCalled](Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError) {
        onFailureWasCalled = true;
    };

    responseDirective = kSendError;

    chip::Controller::InvokeCommandRequest(gExchangeManager, sessionHandle, kTestEndpointId, request, onSuccessCb, onFailureCb);

    NL_TEST_ASSERT(apSuite, !onSuccessWasCalled && onFailureWasCalled);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestDataResponse", TestCommandInteraction::TestDataResponse),
    NL_TEST_DEF("TestSuccessNoDataResponse", TestCommandInteraction::TestSuccessNoDataResponse),
    NL_TEST_DEF("TestFailure", TestCommandInteraction::TestFailure),
    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
        "TestCommands",
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

int TestCommandInteractionTest()
{
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommandInteractionTest)
