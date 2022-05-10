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

// Note: This file offloads some test cases (currently TestReadHandler_MultipleSubscriptionsWithDataVersionFilter only) since the
// gcc-debug will become very slow (and the transactions will timeout) when there are lots of memory allocations.

#include "transport/SecureSession.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/ClusterStateCache.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <controller/ReadInteraction.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>

#include "TestRead.h"

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::test;
using namespace chip::Protocols;

namespace {

constexpr EndpointId kTestEndpointId = 1;
constexpr DataVersion kDataVersion   = 5;

enum ResponseDirective
{
    kSendDataResponse,
    kSendDataError
};

ResponseDirective responseDirective;
} // namespace

namespace {

class TestReadInteraction : public chip::test::TestReadInteractionBase
{
public:
    TestReadInteraction() {}
    virtual ~TestReadInteraction() {}

    static void TestReadHandler_MultipleSubscriptionsWithDataVersionFilter(nlTestSuite * apSuite, void * apContext);
};

TestReadInteraction gTestReadInteraction;

void TestReadInteraction::TestReadHandler_MultipleSubscriptionsWithDataVersionFilter(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                        = *static_cast<TestContext *>(apContext);
    auto sessionHandle                       = ctx.GetSessionBobToAlice();
    uint32_t numSuccessCalls                 = 0;
    uint32_t numSubscriptionEstablishedCalls = 0;

    responseDirective = kSendDataResponse;

    {
        app::AttributePathParams path;
        path.mEndpointId  = kTestEndpointId;
        path.mClusterId   = TestCluster::Id;
        path.mAttributeId = TestCluster::Attributes::Int16u::Id;
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    }

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &numSuccessCalls](const app::ConcreteDataAttributePath & attributePath,
                                                   const auto & dataResponse) {
        NL_TEST_ASSERT(apSuite, attributePath.mDataVersion.HasValue() && attributePath.mDataVersion.Value() == kDataVersion);
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&apSuite](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        //
        // We shouldn't be encountering any failures in this test.
        //
        NL_TEST_ASSERT(apSuite, false);
    };

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls](const app::ReadClient & readClient) {
        numSubscriptionEstablishedCalls++;
    };

    //
    // Try to issue parallel subscriptions that will exceed the value for CHIP_IM_MAX_NUM_READ_HANDLER.
    // If heap allocation is correctly setup, this should result in it successfully servicing more than the number
    // present in that define.
    //
    chip::Optional<chip::DataVersion> dataVersion(1);

    // Note: The CI might take a lots of time to handle too many subscriptions at the same time (due to excessive checks for memory
    // sanity, on gcc_debug variant), so we send and handle the subscriptions in batch.
    for (int i = 0; i < (CHIP_IM_MAX_NUM_READ_HANDLER + 1); i += CHIP_IM_MAX_REPORTS_IN_FLIGHT)
    {
        for (int j = 0; i + j < (CHIP_IM_MAX_NUM_READ_HANDLER + 1) && j < CHIP_IM_MAX_REPORTS_IN_FLIGHT; j++)
        {
            NL_TEST_ASSERT(apSuite,
                           Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                               &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 120,
                               onSubscriptionEstablishedCb, false, true, dataVersion) == CHIP_NO_ERROR);
        }
        ctx.DrainAndServiceIO();
    }

    NL_TEST_ASSERT(apSuite,
                   app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));

    ChipLogError(Zcl, "Success call cnt: %u (expect %u) subscription cnt: %u (expect %u)", numSuccessCalls,
                 uint32_t(CHIP_IM_MAX_NUM_READ_HANDLER + 1), numSubscriptionEstablishedCalls,
                 uint32_t(CHIP_IM_MAX_NUM_READ_HANDLER + 1));

    NL_TEST_ASSERT(apSuite, numSuccessCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestReadHandler_MultipleSubscriptionsWithDataVersionFilter", TestReadInteraction::TestReadHandler_MultipleSubscriptionsWithDataVersionFilter),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestRead-2",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestReadInteractionTest2()
{
    TestContext gContext;
    nlTestRunner(&sSuite, &gContext);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReadInteractionTest2)
