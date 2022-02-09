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

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "protocols/interaction_model/Constants.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppBuildConfig.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
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
    TestClusterCommandHandler() : chip::app::CommandHandlerInterface(Optional<EndpointId>::Missing(), TestCluster::Id)
    {
        chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this);
    }

    ~TestClusterCommandHandler() { chip::app::InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this); }

private:
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & handlerContext) final;
};

void TestClusterCommandHandler::InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & handlerContext)
{
    HandleCommand<TestCluster::Commands::TestSimpleArgumentRequest::DecodableType>(
        handlerContext, [](chip::app::CommandHandlerInterface::HandlerContext & ctx, const auto & requestPayload) {
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

                ctx.mCommandHandler.AddResponseData(ctx.mRequestPath, dataResponse);
            }

            return CHIP_NO_ERROR;
        });
}

} // namespace

namespace {

class TestCommandInteraction
{
public:
    TestCommandInteraction() {}
    static void TestNoHandler(nlTestSuite * apSuite, void * apContext);
    static void TestDataResponse(nlTestSuite * apSuite, void * apContext);

private:
};

// We want to send a TestSimpleArgumentRequest::Type, but get a
// TestStructArrayArgumentResponse in return, so need to shadow the actual
// ResponseType that TestSimpleArgumentRequest has.
struct FakeRequest : public TestCluster::Commands::TestSimpleArgumentRequest::Type
{
    using ResponseType = TestCluster::Commands::TestStructArrayArgumentResponse::DecodableType;
};

void TestCommandInteraction::TestNoHandler(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    FakeRequest request;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    request.arg1 = true;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                 const auto & dataResponse) {
        //
        // We shouldn't be arriving here, since we don't have a command handler installed.
        //
        NL_TEST_ASSERT(apSuite, false);
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [apSuite](CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite,
                       aError.IsIMStatus() &&
                           app::StatusIB(aError).mStatus == Protocols::InteractionModel::Status::UnsupportedEndpoint);
    };

    responseDirective = kSendDataResponse;

    ctx.EnableAsyncDispatch();

    chip::Controller::InvokeCommandRequest(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, request, onSuccessCb,
                                           onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

static const int kDescriptorAttributeArraySize = 254;

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(chip::app::Clusters::Descriptor::Attributes::DeviceList::Id, ARRAY, kDescriptorAttributeArraySize,
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

constexpr CommandId testClusterCommands[] = {
    TestCluster::Commands::TestSimpleArgumentRequest::Id,
    kInvalidCommandId,
};
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::TestCluster::Id, testClusterAttrs, testClusterCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(chip::app::Clusters::Descriptor::Id, descriptorAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

void TestCommandInteraction::TestDataResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    FakeRequest request;
    auto sessionHandle = ctx.GetSessionBobToAlice();
    TestClusterCommandHandler commandHandler;

    bool onSuccessWasCalled = false;
    bool onFailureWasCalled = false;

    request.arg1 = true;

    //
    // Register descriptors for this endpoint since they are needed
    // at command validation time to ensure the command actually exists on that endpoint.
    //
    DataVersion dataVersionStorage[ArraySize(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, 0, 0, Span<DataVersion>(dataVersionStorage));

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

    emberAfClearDynamicEndpoint(0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestNoHandler", TestCommandInteraction::TestNoHandler),
    NL_TEST_DEF("TestDataResponse", TestCommandInteraction::TestDataResponse),
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
