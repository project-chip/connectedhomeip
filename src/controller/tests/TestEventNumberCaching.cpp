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

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/BufferedReadCallback.h>
#include <app/ClusterStateCache.h>
#include <app/CommandHandlerInterface.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static uint8_t gDebugEventBuffer[4096];
static uint8_t gInfoEventBuffer[4096];
static uint8_t gCritEventBuffer[4096];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

//
// The generated endpoint_config for the controller app has Endpoint 1
// already used in the fixed endpoint set of size 1. Consequently, let's use the next
// number higher than that for our dynamic test endpoint.
//
constexpr EndpointId kTestEndpointId = 2;

class TestEventNumberCaching : public chip::Test::AppContext
{
protected:
    // Performs setup for each test in the suite
    void SetUp()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        AppContext::SetUp();

        CHIP_ERROR err = CHIP_NO_ERROR;
        // TODO: use ASSERT_EQ, once transition to pw_unit_test is complete
        VerifyOrDieWithMsg((err = mEventCounter.Init(0)) == CHIP_NO_ERROR, AppServer,
                           "Init EventCounter failed: %" CHIP_ERROR_FORMAT, err.Format());
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), ArraySize(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);
    }

    // Performs teardown for each test in the suite
    void TearDown()
    {
        chip::app::EventManagement::DestroyEventManagement();
        AppContext::TearDown();
        chip::Platform::MemoryShutdown();
    }

private:
    MonotonicallyIncreasingCounter<EventNumber> mEventCounter;
};

//clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrs)
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

//clang-format on

class TestReadCallback : public app::ClusterStateCache::Callback
{
public:
    TestReadCallback() : mClusterCacheAdapter(*this, Optional<EventNumber>::Missing(), false /*cacheData*/) {}
    void OnDone(app::ReadClient *) override {}

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override
    {
        ++mEventsSeen;
    }

    app::ClusterStateCache mClusterCacheAdapter;

    size_t mEventsSeen = 0;
};

void GenerateEvents(chip::EventNumber & firstEventNumber, chip::EventNumber & lastEventNumber)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    static uint8_t generationCount = 0;

    Clusters::UnitTesting::Events::TestEvent::Type content;

    for (int i = 0; i < 5; i++)
    {
        content.arg1 = static_cast<uint8_t>(generationCount++);
        EXPECT_EQ((err = app::LogEvent(content, kTestEndpointId, lastEventNumber)), CHIP_NO_ERROR);
        if (i == 0)
        {
            firstEventNumber = lastEventNumber;
        }
    }
}

/*
 * This validates event caching by forcing a bunch of events to get generated, then reading them back
 * and upon completion of that operation, check the received version from cache, and note that cache would store
 * correpsonding attribute data since data cache is disabled.
 *
 */
TEST_F(TestEventNumberCaching, TestEventNumberCaching)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[ArraySize(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    chip::EventNumber firstEventNumber;
    chip::EventNumber lastEventNumber;

    GenerateEvents(firstEventNumber, lastEventNumber);
    EXPECT_GT(lastEventNumber, firstEventNumber);

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
        EXPECT_FALSE(highestEventNumber.HasValue());
        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mClusterCacheAdapter.GetBufferedCallback(),
                                   app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(readCallback.mEventsSeen, lastEventNumber - firstEventNumber + 1);

        readCallback.mClusterCacheAdapter.ForEachEventData([](const app::EventHeader & header) {
            // We are not caching data.
            ADD_FAILURE(); // Can't use FAIL() because lambda has non-void return type.
            return CHIP_NO_ERROR;
        });

        readCallback.mClusterCacheAdapter.GetHighestReceivedEventNumber(highestEventNumber);
        EXPECT_TRUE(highestEventNumber.HasValue() && highestEventNumber.Value() == lastEventNumber);
    }

    //
    // Clear out the event cache and set its highest received event number to a non zero value. Validate that
    // we don't receive events except ones larger than that value.
    //
    {
        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mClusterCacheAdapter.GetBufferedCallback(),
                                   app::ReadClient::InteractionType::Read);

        readCallback.mClusterCacheAdapter.ClearEventCache(true);
        Optional<EventNumber> highestEventNumber;
        readCallback.mClusterCacheAdapter.GetHighestReceivedEventNumber(highestEventNumber);
        EXPECT_FALSE(highestEventNumber.HasValue());

        const EventNumber kHighestEventNumberSeen = lastEventNumber - 1;
        EXPECT_LT(kHighestEventNumberSeen, lastEventNumber);

        readCallback.mClusterCacheAdapter.SetHighestReceivedEventNumber(kHighestEventNumberSeen);

        readCallback.mEventsSeen = 0;

        readParams.mEventNumber.ClearValue();
        EXPECT_FALSE(readParams.mEventNumber.HasValue());
        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        // We should only get events with event numbers larger than kHighestEventNumberSeen.
        EXPECT_EQ(readCallback.mEventsSeen, lastEventNumber - kHighestEventNumberSeen);

        readCallback.mClusterCacheAdapter.ForEachEventData([](const app::EventHeader & header) {
            // We are not caching data.
            ADD_FAILURE(); // Can't use FAIL() because lambda has non-void return type.
            return CHIP_NO_ERROR;
        });

        readCallback.mClusterCacheAdapter.GetHighestReceivedEventNumber(highestEventNumber);
        EXPECT_TRUE(highestEventNumber.HasValue() && highestEventNumber.Value() == lastEventNumber);
    }
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

} // namespace
