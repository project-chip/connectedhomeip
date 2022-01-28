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

#include "app/data-model/NullObject.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
chip::ClusterStatus kTestSuccessClusterStatus = 1;
chip::ClusterStatus kTestFailureClusterStatus = 2;

constexpr EndpointId kTestEndpointId = 1;

enum ResponseDirective
{
    kSendDataResponse,
    kSendSuccessStatusCode,
    kSendError,
    kSendSuccessStatusCodeWithClusterStatus,
    kSendErrorWithClusterStatus,
    kAsync,
};

ResponseDirective responseDirective;
CommandHandler::Handle asyncHandle;

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
        else if (responseDirective == kSendSuccessStatusCodeWithClusterStatus)
        {
            apCommandObj->AddClusterSpecificSuccess(aCommandPath, kTestSuccessClusterStatus);
        }
        else if (responseDirective == kSendErrorWithClusterStatus)
        {
            apCommandObj->AddClusterSpecificFailure(aCommandPath, kTestFailureClusterStatus);
        }
        else if (responseDirective == kAsync)
        {
            asyncHandle = apCommandObj;
        }
    }
}

bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // Mock cluster catalog, only support one command on one cluster on one endpoint.
    return (aCommandPath.mEndpointId == kTestEndpointId && aCommandPath.mClusterId == TestCluster::Id);
}

CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, ClusterInfo & aClusterInfo,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
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
    static void TestAsyncResponse(nlTestSuite * apSuite, void * apContext);
    static void TestFailure(nlTestSuite * apSuite, void * apContext);
    static void TestSuccessNoDataResponseWithClusterStatus(nlTestSuite * apSuite, void * apContext);
    static void TestFailureWithClusterStatus(nlTestSuite * apSuite, void * apContext);

private:
};

void TestCommandInteraction::TestDataResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    // We want to send a TestSimpleArgumentRequest::Type, but get a
    // TestStructArrayArgumentResponse in return, so need to shadow the actual
    // ResponseType that TestSimpleArgumentRequest has.
    struct FakeRequest : public TestCluster::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = TestCluster::Commands::TestStructArrayArgumentResponse::DecodableType;
    };

    FakeRequest request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;

    request.arg1 = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &onSuccessWasCalled](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                                      const auto & dataResponse) {
        uint8_t i = 0;
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

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureWasCalled](CHIP_ERROR aError) { onFailureWasCalled = true; };

    responseDirective = kSendDataResponse;

    chip::Controller::InvokeCommandRequest(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessWasCalled && !onFailureWasCalled);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestSuccessNoDataResponse(nlTestSuite * apSuite, void * apContext)
{
    struct FakeRequest : public TestCluster::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    TestContext & ctx = *static_cast<TestContext *>(apContext);
    FakeRequest request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;
    bool statusCheck        = false;
    request.arg1            = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessWasCalled, &statusCheck](const app::ConcreteCommandPath & commandPath,
                                                           const app::StatusIB & aStatus, const auto & dataResponse) {
        statusCheck        = (aStatus.mStatus == Protocols::InteractionModel::Status::Success);
        onSuccessWasCalled = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureWasCalled](CHIP_ERROR aError) { onFailureWasCalled = true; };

    responseDirective = kSendSuccessStatusCode;

    chip::Controller::InvokeCommandRequest(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessWasCalled && !onFailureWasCalled && statusCheck);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestAsyncResponse(nlTestSuite * apSuite, void * apContext)
{
    struct FakeRequest : public TestCluster::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    TestContext & ctx = *static_cast<TestContext *>(apContext);
    FakeRequest request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;
    bool statusCheck        = false;
    request.arg1            = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessWasCalled, &statusCheck](const app::ConcreteCommandPath & commandPath,
                                                           const app::StatusIB & aStatus, const auto & dataResponse) {
        statusCheck        = (aStatus.mStatus == Protocols::InteractionModel::Status::Success);
        onSuccessWasCalled = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureWasCalled](CHIP_ERROR aError) { onFailureWasCalled = true; };

    responseDirective = kAsync;

    chip::Controller::InvokeCommandRequest(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessWasCalled && !onFailureWasCalled && !statusCheck);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 2);

    CommandHandler * commandHandle = asyncHandle.Get();
    NL_TEST_ASSERT(apSuite, commandHandle != nullptr);

    if (commandHandle == nullptr)
    {
        return;
    }

    commandHandle->AddStatus(ConcreteCommandPath(kTestEndpointId, request.GetClusterId(), request.GetCommandId()),
                             Protocols::InteractionModel::Status::Success);
    asyncHandle.Release();

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessWasCalled && !onFailureWasCalled && statusCheck);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestFailure(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    TestCluster::Commands::TestSimpleArgumentRequest::Type request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;
    bool statusCheck        = false;
    request.arg1            = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessWasCalled](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                             const auto & dataResponse) { onSuccessWasCalled = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureWasCalled, &statusCheck](CHIP_ERROR aError) {
        statusCheck        = aError.IsIMStatus() && app::StatusIB(aError).mStatus == Protocols::InteractionModel::Status::Failure;
        onFailureWasCalled = true;
    };

    responseDirective = kSendError;

    chip::Controller::InvokeCommandRequest(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessWasCalled && onFailureWasCalled && statusCheck);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestSuccessNoDataResponseWithClusterStatus(nlTestSuite * apSuite, void * apContext)
{
    struct FakeRequest : public TestCluster::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    TestContext & ctx = *static_cast<TestContext *>(apContext);
    FakeRequest request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;
    bool statusCheck        = false;
    request.arg1            = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessWasCalled, &statusCheck](const app::ConcreteCommandPath & commandPath,
                                                           const app::StatusIB & aStatus, const auto & dataResponse) {
        statusCheck        = (aStatus.mStatus == Protocols::InteractionModel::Status::Success &&
                       aStatus.mClusterStatus.Value() == kTestSuccessClusterStatus);
        onSuccessWasCalled = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureWasCalled](CHIP_ERROR aError) { onFailureWasCalled = true; };

    responseDirective = kSendSuccessStatusCodeWithClusterStatus;

    chip::Controller::InvokeCommandRequest(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessWasCalled && !onFailureWasCalled && statusCheck);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestCommandInteraction::TestFailureWithClusterStatus(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    TestCluster::Commands::TestSimpleArgumentRequest::Type request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;
    bool statusCheck        = false;
    request.arg1            = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessWasCalled](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                             const auto & dataResponse) { onSuccessWasCalled = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureWasCalled, &statusCheck](CHIP_ERROR aError) {
        statusCheck = aError.IsIMStatus();
        if (statusCheck)
        {
            app::StatusIB status(aError);
            statusCheck = (status.mStatus == Protocols::InteractionModel::Status::Failure &&
                           status.mClusterStatus.Value() == kTestFailureClusterStatus);
        }
        onFailureWasCalled = true;
    };

    responseDirective = kSendErrorWithClusterStatus;

    chip::Controller::InvokeCommandRequest(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessWasCalled && onFailureWasCalled && statusCheck);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestDataResponse", TestCommandInteraction::TestDataResponse),
    NL_TEST_DEF("TestSuccessNoDataResponse", TestCommandInteraction::TestSuccessNoDataResponse),
    NL_TEST_DEF("TestAsyncResponse", TestCommandInteraction::TestAsyncResponse),
    NL_TEST_DEF("TestFailure", TestCommandInteraction::TestFailure),
    NL_TEST_DEF("TestSuccessNoDataResponseWithClusterStatus", TestCommandInteraction::TestSuccessNoDataResponseWithClusterStatus),
    NL_TEST_DEF("TestFailureWithClusterStatus", TestCommandInteraction::TestFailureWithClusterStatus),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestCommands",
    &sTests[0],
    TestContext::InitializeAsync,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestCommandInteractionTest()
{
    TestContext gContext;
    nlTestRunner(&sSuite, &gContext);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommandInteractionTest)
