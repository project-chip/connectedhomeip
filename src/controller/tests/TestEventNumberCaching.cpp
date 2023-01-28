/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app/ClusterStateCache.h"
#include "app/ConcreteAttributePath.h"
#include "protocols/interaction_model/Constants.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppConfig.h>
#include <app/AttributeAccessInterface.h>
#include <app/BufferedReadCallback.h>
#include <app/CommandHandlerInterface.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static uint8_t gDebugEventBuffer[4096];
static uint8_t gInfoEventBuffer[4096];
static uint8_t gCritEventBuffer[4096];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

class TestContext : public chip::Test::AppContext
{
public:
    static int Initialize(void * context)
    {
        if (AppContext::Initialize(context) != SUCCESS)
            return FAILURE;

        auto * ctx = static_cast<TestContext *>(context);

        if (ctx->mEventCounter.Init(0) != CHIP_NO_ERROR)
        {
            return FAILURE;
        }

        chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        chip::app::EventManagement::CreateEventManagement(&ctx->GetExchangeManager(),
                                                          sizeof(logStorageResources) / sizeof(logStorageResources[0]),
                                                          gCircularEventBuffer, logStorageResources, &ctx->mEventCounter);

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        chip::app::EventManagement::DestroyEventManagement();

        if (AppContext::Finalize(context) != SUCCESS)
            return FAILURE;

        return SUCCESS;
    }

private:
    MonotonicallyIncreasingCounter<EventNumber> mEventCounter;
};

nlTestSuite * gSuite = nullptr;

//
// The generated endpoint_config for the controller app has Endpoint 1
// already used in the fixed endpoint set of size 1. Consequently, let's use the next
// number higher than that for our dynamic test endpoint.
//
constexpr EndpointId kTestEndpointId = 2;

class TestReadEvents
{
public:
    TestReadEvents() {}
    static void TestEventNumberCaching(nlTestSuite * apSuite, void * apContext);

private:
};

//clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrs)
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrs, nullptr, nullptr), DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

//clang-format on

class TestReadCallback : public app::ClusterStateCache::Callback
{
public:
    TestReadCallback() : mClusterCacheAdapter(*this, Optional<EventNumber>::Missing(), false /*cacheData*/) {}
    void OnDone(app::ReadClient *) {}

    app::ClusterStateCache mClusterCacheAdapter;
};

namespace {

void GenerateEvents(nlTestSuite * apSuite, chip::EventNumber & firstEventNumber, chip::EventNumber & lastEventNumber)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    static uint8_t generationCount = 0;

    Clusters::UnitTesting::Events::TestEvent::Type content;

    for (int i = 0; i < 5; i++)
    {
        content.arg1 = static_cast<uint8_t>(generationCount++);
        NL_TEST_ASSERT(apSuite, (err = app::LogEvent(content, kTestEndpointId, lastEventNumber)) == CHIP_NO_ERROR);
        if (i == 0)
        {
            firstEventNumber = lastEventNumber;
        }
    }
}

} // namespace

/*
 * This validates event caching by forcing a bunch of events to get generated, then reading them back
 * and upon completion of that operation, check the received version from cache, and note that cache would store
 * correpsonding attribute data since data cache is disabled.
 *
 */
void TestReadEvents::TestEventNumberCaching(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                    = *static_cast<TestContext *>(apContext);
    auto sessionHandle                   = ctx.GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[ArraySize(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    chip::EventNumber firstEventNumber;
    chip::EventNumber lastEventNumber;

    GenerateEvents(apSuite, firstEventNumber, lastEventNumber);

    app::EventPathParams eventPath;
    eventPath.mEndpointId = kTestEndpointId;
    eventPath.mClusterId  = app::Clusters::UnitTesting::Id;
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpEventPathParamsList    = &eventPath;
    readParams.mEventPathParamsListSize = 1;
    readParams.mEventNumber.SetValue(firstEventNumber);

    TestReadCallback readCallback;

    {
        Optional<EventNumber> highestEventNumber;
        readCallback.mClusterCacheAdapter.GetHighestReceivedEventNumber(highestEventNumber);
        NL_TEST_ASSERT(apSuite, !highestEventNumber.HasValue());
        app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mClusterCacheAdapter.GetBufferedCallback(),
                                   app::ReadClient::InteractionType::Read);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        readCallback.mClusterCacheAdapter.ForEachEventData([&apSuite, &readCallback](const app::EventHeader & header) {
            NL_TEST_ASSERT(apSuite, header.mPath.mClusterId == Clusters::UnitTesting::Id);
            NL_TEST_ASSERT(apSuite, header.mPath.mEventId == Clusters::UnitTesting::Events::TestEvent::Id);
            NL_TEST_ASSERT(apSuite, header.mPath.mEndpointId == kTestEndpointId);

            Clusters::UnitTesting::Events::TestEvent::DecodableType eventData;
            NL_TEST_ASSERT(apSuite, readCallback.mClusterCacheAdapter.Get(header.mEventNumber, eventData) != CHIP_NO_ERROR);
            return CHIP_NO_ERROR;
        });

        readCallback.mClusterCacheAdapter.GetHighestReceivedEventNumber(highestEventNumber);
        NL_TEST_ASSERT(apSuite, highestEventNumber.HasValue() && highestEventNumber.Value() == 4);
    }

    //
    // Clear out the event cache and set its highest received event number to a non zero value. Validate that
    // we don't receive events lower than that value.
    //
    {
        app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mClusterCacheAdapter.GetBufferedCallback(),
                                   app::ReadClient::InteractionType::Read);

        readCallback.mClusterCacheAdapter.ClearEventCache(true);
        Optional<EventNumber> highestEventNumber;
        readCallback.mClusterCacheAdapter.GetHighestReceivedEventNumber(highestEventNumber);
        NL_TEST_ASSERT(apSuite, !highestEventNumber.HasValue());
        readCallback.mClusterCacheAdapter.SetHighestReceivedEventNumber(3);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        readCallback.mClusterCacheAdapter.ForEachEventData([&apSuite, &readCallback](const app::EventHeader & header) {
            NL_TEST_ASSERT(apSuite, header.mPath.mClusterId == Clusters::UnitTesting::Id);
            NL_TEST_ASSERT(apSuite, header.mPath.mEventId == Clusters::UnitTesting::Events::TestEvent::Id);
            NL_TEST_ASSERT(apSuite, header.mPath.mEndpointId == kTestEndpointId);

            Clusters::UnitTesting::Events::TestEvent::DecodableType eventData;
            NL_TEST_ASSERT(apSuite, readCallback.mClusterCacheAdapter.Get(header.mEventNumber, eventData) != CHIP_NO_ERROR);
            return CHIP_NO_ERROR;
        });

        readCallback.mClusterCacheAdapter.GetHighestReceivedEventNumber(highestEventNumber);
        NL_TEST_ASSERT(apSuite, highestEventNumber.HasValue() && highestEventNumber.Value() == 4);
    }
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestEventNumberCaching", TestReadEvents::TestEventNumberCaching),
    NL_TEST_SENTINEL()
};

// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestEventNumberCaching",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestEventNumberCaching()
{
    gSuite = &sSuite;
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestEventNumberCaching)
