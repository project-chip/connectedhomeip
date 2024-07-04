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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include "DataModelFixtures.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/NullObject.h>
#include <app/tests/AppTestContext.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/interaction_model/StatusCode.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModelTests;
using namespace chip::Protocols;

namespace {

class TestCommands : public ::testing::Test
{
public:
    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        mpContext = new TestContext();
        ASSERT_NE(mpContext, nullptr);
        mpContext->SetUpTestSuite();
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        mpContext->TearDownTestSuite();
        delete mpContext;
    }

protected:
    // Performs setup for each individual test in the test suite
    void SetUp() { mpContext->SetUp(); }

    // Performs teardown for each individual test in the test suite
    void TearDown() { mpContext->TearDown(); }

    static TestContext * mpContext;
};
TestContext * TestCommands::mpContext = nullptr;

TEST_F(TestCommands, TestDataResponse)
{
    // We want to send a TestSimpleArgumentRequest::Type, but get a
    // TestStructArrayArgumentResponse in return, so need to shadow the actual
    // ResponseType that TestSimpleArgumentRequest has.
    struct FakeRequest : public Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType;
    };

    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;

    request.arg1 = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessWasCalled](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                             const auto & dataResponse) {
        uint8_t i = 0;
        auto iter = dataResponse.arg1.begin();
        while (iter.Next())
        {
            auto & item = iter.GetValue();

            EXPECT_EQ(item.a, i);
            EXPECT_FALSE(item.b);
            EXPECT_EQ(item.c.a, i);
            EXPECT_TRUE(item.c.b);
            i++;
        }

        EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
        EXPECT_TRUE(dataResponse.arg6);

        onSuccessWasCalled = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureWasCalled](CHIP_ERROR aError) { onFailureWasCalled = true; };

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kSendDataResponse);

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessWasCalled && !onFailureWasCalled);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommands, TestSuccessNoDataResponse)
{
    struct FakeRequest : public Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

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

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kSendSuccessStatusCode);

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessWasCalled && !onFailureWasCalled && statusCheck);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommands, TestMultipleSuccessNoDataResponses)
{
    struct FakeRequest : public Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

    size_t successCalls = 0;
    size_t failureCalls = 0;
    bool statusCheck    = false;
    request.arg1        = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls, &statusCheck](const ConcreteCommandPath & commandPath, const StatusIB & aStatus,
                                                     const auto & dataResponse) {
        statusCheck = (aStatus.mStatus == InteractionModel::Status::Success);
        ++successCalls;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls](CHIP_ERROR aError) { ++failureCalls; };

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kSendMultipleSuccessStatusCodes);

    Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                     onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(successCalls == 1 && statusCheck);
    EXPECT_EQ(failureCalls, 0u);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommands, TestAsyncResponse)
{
    struct FakeRequest : public Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

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

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kAsync);

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessWasCalled && !onFailureWasCalled && !statusCheck);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 2u);

    CommandHandler * commandHandle = gAsyncCommandHandle.Get();
    ASSERT_NE(commandHandle, nullptr);

    commandHandle->AddStatus(ConcreteCommandPath(kTestEndpointId, request.GetClusterId(), request.GetCommandId()),
                             Protocols::InteractionModel::Status::Success);
    gAsyncCommandHandle.Release();

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessWasCalled && !onFailureWasCalled && statusCheck);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommands, TestFailure)
{
    Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

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

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kSendError);

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessWasCalled && onFailureWasCalled && statusCheck);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommands, TestMultipleFailures)
{
    struct FakeRequest : public Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

    size_t successCalls = 0;
    size_t failureCalls = 0;
    bool statusCheck    = false;
    request.arg1        = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls](const ConcreteCommandPath & commandPath, const StatusIB & aStatus,
                                       const auto & dataResponse) { ++successCalls; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls, &statusCheck](CHIP_ERROR aError) {
        statusCheck = aError.IsIMStatus() && StatusIB(aError).mStatus == InteractionModel::Status::Failure;
        ++failureCalls;
    };

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kSendMultipleErrors);

    Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                     onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_EQ(successCalls, 0u);
    EXPECT_TRUE(failureCalls == 1 && statusCheck);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommands, TestSuccessNoDataResponseWithClusterStatus)
{
    struct FakeRequest : public Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type
    {
        using ResponseType = DataModel::NullObjectType;
    };

    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

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

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kSendSuccessStatusCodeWithClusterStatus);

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessWasCalled && !onFailureWasCalled && statusCheck);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestCommands, TestFailureWithClusterStatus)
{
    Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

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

    ScopedChange directive(gCommandResponseDirective, CommandResponseDirective::kSendErrorWithClusterStatus);

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(!onSuccessWasCalled && onFailureWasCalled && statusCheck);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

} // namespace
