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

#include <gtest/gtest.h>

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "protocols/interaction_model/Constants.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <controller/ReadInteraction.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
constexpr EndpointId kTestEndpointId = 1;

enum ResponseDirective
{
    kSendDataResponse,
    kSendSuccessStatusCode,
    kSendError,
    kSendSuccessStatusCodeWithClusterStatus,
    kSendErrorWithClusterStatus,
};

ResponseDirective responseDirective;

class TestClusterCommandHandler : public chip::app::CommandHandlerInterface
{
public:
    TestClusterCommandHandler() : chip::app::CommandHandlerInterface(Optional<EndpointId>::Missing(), Clusters::UnitTesting::Id)
    {
        chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this);
    }

    ~TestClusterCommandHandler() { chip::app::InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this); }

    void OverrideAcceptedCommands() { mOverrideAcceptedCommands = true; }
    void ClaimNoCommands() { mClaimNoCommands = true; }

private:
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & handlerContext) final;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) final;

    bool mOverrideAcceptedCommands = false;
    bool mClaimNoCommands          = false;
};

void TestClusterCommandHandler::InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & handlerContext)
{
    HandleCommand<Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::DecodableType>(
        handlerContext, [](chip::app::CommandHandlerInterface::HandlerContext & ctx, const auto & requestPayload) {
            if (responseDirective == kSendDataResponse)
            {
                Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::Type dataResponse;
                Clusters::UnitTesting::Structs::NestedStructList::Type nestedStructList[4];

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

                ctx.mCommandHandler.AddResponse(ctx.mRequestPath, dataResponse);
            }

            return CHIP_NO_ERROR;
        });
}

CHIP_ERROR TestClusterCommandHandler::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                                                CommandHandlerInterface::CommandIdCallback callback, void * context)
{
    if (!mOverrideAcceptedCommands)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (mClaimNoCommands)
    {
        return CHIP_NO_ERROR;
    }

    // We just have one command id.
    callback(Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Id, context);
    return CHIP_NO_ERROR;
}

} // namespace

namespace {

class TestServerCommandDispatch : public ::testing::Test
{
public:
    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        if (mpContext == nullptr)
        {
            mpContext = new TestContext();
            ASSERT_NE(mpContext, nullptr);
        }
        mpContext->SetUpTestSuite();
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        mpContext->TearDownTestSuite();
        if (mpContext != nullptr)
        {
            delete mpContext;
            mpContext = nullptr;
        }
    }

protected:
    // Performs setup for each test in the suite
    void SetUp() { mpContext->SetUp(); }

    // Performs teardown for each test in the suite
    void TearDown() { mpContext->TearDown(); }

    static TestContext * mpContext;

    // Helpers

    static void TestDataResponseHelper(const EmberAfEndpointType * aEndpoint, bool aExpectSuccess);
};
TestContext * TestServerCommandDispatch::mpContext = nullptr;

// We want to send a TestSimpleArgumentRequest::Type, but get a
// TestStructArrayArgumentResponse in return, so need to shadow the actual
// ResponseType that TestSimpleArgumentRequest has.
struct FakeRequest : public Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type
{
    using ResponseType = Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType;
};

TEST_F(TestServerCommandDispatch, TestNoHandler)
{
    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

    request.arg1 = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus, const auto & dataResponse) {
        //
        // We shouldn't be arriving here, since we don't have a command handler installed.
        //
        FAIL();
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [](CHIP_ERROR aError) {
        EXPECT_TRUE(aError.IsIMStatus() &&
                    app::StatusIB(aError).mStatus == Protocols::InteractionModel::Status::UnsupportedEndpoint);
    };

    responseDirective = kSendDataResponse;

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);
}

static const int kDescriptorAttributeArraySize = 254;

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(chip::app::Clusters::Descriptor::Attributes::DeviceTypeList::Id, ARRAY, kDescriptorAttributeArraySize,
                          0), /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(chip::app::Clusters::Descriptor::Attributes::ServerList::Id, ARRAY, kDescriptorAttributeArraySize,
                              0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(chip::app::Clusters::Descriptor::Attributes::ClientList::Id, ARRAY, kDescriptorAttributeArraySize,
                              0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(chip::app::Clusters::Descriptor::Attributes::PartsList::Id, ARRAY, kDescriptorAttributeArraySize,
                              0), /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrs)
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

constexpr CommandId testClusterCommands1[] = {
    Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Id,
    kInvalidCommandId,
};
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters1)
DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::UnitTesting::Id, testClusterAttrs, ZAP_CLUSTER_MASK(SERVER), testClusterCommands1,
                        nullptr),
    DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint1, testEndpointClusters1);

constexpr CommandId testClusterCommands2[] = {
    kInvalidCommandId,
};
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters2)
DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::UnitTesting::Id, testClusterAttrs, ZAP_CLUSTER_MASK(SERVER), testClusterCommands2,
                        nullptr),
    DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint2, testEndpointClusters2);

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters3)
DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::UnitTesting::Id, testClusterAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint3, testEndpointClusters3);

void TestServerCommandDispatch::TestDataResponseHelper(const EmberAfEndpointType * aEndpoint, bool aExpectSuccess)
{
    FakeRequest request;
    auto sessionHandle = mpContext->GetSessionBobToAlice();

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;

    request.arg1 = true;

    //
    // Register descriptors for this endpoint since they are needed
    // at command validation time to ensure the command actually exists on that
    // endpoint.
    //
    // All our endpoints have the same number of clusters, so just pick one.
    //
    DataVersion dataVersionStorage[ArraySize(testEndpointClusters1)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, aEndpoint, Span<DataVersion>(dataVersionStorage));

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

    responseDirective = kSendDataResponse;

    chip::Controller::InvokeCommandRequest(&mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessWasCalled == aExpectSuccess && onFailureWasCalled != aExpectSuccess);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);

    onSuccessWasCalled = false;
    onFailureWasCalled = false;

    auto readSuccessCb = [&onSuccessWasCalled, aExpectSuccess](const ConcreteDataAttributePath &,
                                                               const DataModel::DecodableList<CommandId> & commandList) {
        auto count = 0;
        auto iter  = commandList.begin();
        while (iter.Next())
        {
            // We only expect 0 or 1 command ids here.
            EXPECT_EQ(count, 0);
            EXPECT_EQ(iter.GetValue(), Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Id);
            ++count;
        }
        EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
        if (aExpectSuccess)
        {
            EXPECT_EQ(count, 1);
        }
        else
        {
            EXPECT_EQ(count, 0);
        }
        onSuccessWasCalled = true;
    };
    auto readFailureCb = [&onFailureWasCalled](const ConcreteDataAttributePath *, CHIP_ERROR aError) {
        onFailureWasCalled = true;
        ChipLogError(NotSpecified, "TEST FAILURE: %" CHIP_ERROR_FORMAT, aError.Format());
    };

    chip::Controller::ReadAttribute<Clusters::UnitTesting::Attributes::AcceptedCommandList::TypeInfo>(
        &mpContext->GetExchangeManager(), sessionHandle, kTestEndpointId, readSuccessCb, readFailureCb);

    mpContext->DrainAndServiceIO();

    EXPECT_TRUE(onSuccessWasCalled && !onFailureWasCalled);
    EXPECT_EQ(mpContext->GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

TEST_F(TestServerCommandDispatch, TestDataResponse)
{
    TestClusterCommandHandler commandHandler;
    TestDataResponseHelper(&testEndpoint1, true);
}

TEST_F(TestServerCommandDispatch, TestDataResponseNoCommand1)
{
    // Check what happens if we don't claim our command id is supported, by
    // overriding the acceptedCommandList with an empty list.
    TestClusterCommandHandler commandHandler;
    commandHandler.OverrideAcceptedCommands();
    commandHandler.ClaimNoCommands();
    TestDataResponseHelper(&testEndpoint1, false);
}

TEST_F(TestServerCommandDispatch, TestDataResponseNoCommand2)
{
    // Check what happens if we don't claim our command id is supported, by
    // having an acceptedCommandList that ends immediately.
    TestClusterCommandHandler commandHandler;
    TestDataResponseHelper(&testEndpoint2, false);
}

TEST_F(TestServerCommandDispatch, TestDataResponseNoCommand3)
{
    // Check what happens if we don't claim our command id is supported, by
    // having an acceptedCommandList that is null.
    TestClusterCommandHandler commandHandler;
    TestDataResponseHelper(&testEndpoint3, false);
}

TEST_F(TestServerCommandDispatch, TestDataResponseHandlerOverride1)
{
    TestClusterCommandHandler commandHandler;
    commandHandler.OverrideAcceptedCommands();
    TestDataResponseHelper(&testEndpoint2, true);
}

TEST_F(TestServerCommandDispatch, TestDataResponseHandlerOverride2)
{
    TestClusterCommandHandler commandHandler;
    commandHandler.OverrideAcceptedCommands();
    TestDataResponseHelper(&testEndpoint3, true);
}

} // namespace
