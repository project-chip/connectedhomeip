/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AppConfig.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/BufferedReadCallback.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/GlobalAttributes.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static uint8_t gDebugEventBuffer[4096];
static uint8_t gInfoEventBuffer[4096];
static uint8_t gCritEventBuffer[4096];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

uint32_t gIterationCount = 0;

//
// The generated endpoint_config for the controller app has Endpoint 1
// already used in the fixed endpoint set of size 1. Consequently, let's use the next
// number higher than that for our dynamic test endpoint.
//
constexpr EndpointId kTestEndpointId          = 2;
constexpr AttributeId kTestListLargeAttribute = 8; // This attribute will be larger than the event size we used in this test.

// The size of the attribute which is a bit larger than the size of event used in the test.
constexpr size_t kSizeOfLargeAttribute = 60;

class TestEventChunking : public chip::Test::AppContext
{
protected:
    // Performs setup for each test in the suite
    void SetUp()
    {
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
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), MATTER_ARRAY_SIZE(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);
    }

    // Performs teardown for each test in the suite
    void TearDown()
    {
        chip::app::EventManagement::DestroyEventManagement();
        AppContext::TearDown();
    }

private:
    MonotonicallyIncreasingCounter<EventNumber> mEventCounter;
};

//clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(0x00000001, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000002, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000003, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000004, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000005, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint4)
DECLARE_DYNAMIC_ATTRIBUTE(kTestListLargeAttribute, ARRAY, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpoint4Clusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrsOnEndpoint4, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint4, testEndpoint4Clusters);

//clang-format on

uint8_t sAnStringThatCanNeverFitIntoTheMTU[4096] = { 0 };

class TestReadCallback : public app::ReadClient::Callback
{
public:
    TestReadCallback() : mBufferedCallback(*this) {}
    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override;

    void OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * apStatus) override;

    void OnDone(app::ReadClient * apReadClient) override;

    void OnReportEnd() override { mOnReportEnd = true; }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override { mOnSubscriptionEstablished = true; }

    uint32_t mAttributeCount        = 0;
    uint32_t mEventCount            = 0;
    bool mOnReportEnd               = false;
    bool mOnSubscriptionEstablished = false;
    app::BufferedReadCallback mBufferedCallback;
};

void TestReadCallback::OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                       const app::StatusIB & aStatus)
{
    if (aPath.mAttributeId == Globals::Attributes::GeneratedCommandList::Id)
    {
        app::DataModel::DecodableList<CommandId> v;
        EXPECT_EQ(app::DataModel::Decode(*apData, v), CHIP_NO_ERROR);
        auto it          = v.begin();
        size_t arraySize = 0;
        while (it.Next())
        {
            FAIL();
        }
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(v.ComputeSize(&arraySize), CHIP_NO_ERROR);
        EXPECT_EQ(arraySize, 0u);
    }
    else if (aPath.mAttributeId == Globals::Attributes::AcceptedCommandList::Id)
    {
        app::DataModel::DecodableList<CommandId> v;
        EXPECT_EQ(app::DataModel::Decode(*apData, v), CHIP_NO_ERROR);
        auto it          = v.begin();
        size_t arraySize = 0;
        while (it.Next())
        {
            FAIL();
        }
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(v.ComputeSize(&arraySize), CHIP_NO_ERROR);
        EXPECT_EQ(arraySize, 0u);
    }
    else if (aPath.mAttributeId == Globals::Attributes::AttributeList::Id)
    {
        // Nothing to check for this one; depends on the endpoint.
    }
    else if (aPath.mAttributeId == kTestListLargeAttribute)
    {
        app::DataModel::DecodableList<ByteSpan> v;
        EXPECT_EQ(app::DataModel::Decode(*apData, v), CHIP_NO_ERROR);
        size_t arraySize = 0;
        EXPECT_EQ(v.ComputeSize(&arraySize), CHIP_NO_ERROR);
        EXPECT_EQ(arraySize, 4u);
    }
    else
    {
        uint8_t v;
        EXPECT_EQ(app::DataModel::Decode(*apData, v), CHIP_NO_ERROR);
        EXPECT_EQ(v, (uint8_t) gIterationCount);
    }
    mAttributeCount++;
}

void TestReadCallback::OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * aStatus)
{
    VerifyOrReturn(apData != nullptr);
    mEventCount++;
}

void TestReadCallback::OnDone(app::ReadClient *) {}

class TestAttrAccess : public app::AttributeAccessInterface
{
public:
    // Register for the Test Cluster cluster on all endpoints.
    TestAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::UnitTesting::Id)
    {
        AttributeAccessInterfaceRegistry::Instance().Register(this);
    }

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder) override;
};

TestAttrAccess gAttrAccess;

CHIP_ERROR TestAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case kTestListLargeAttribute:
        return aEncoder.EncodeList([](const auto & encoder) {
            for (int i = 0; i < 4; i++)
            {
                // When putting even numbers of list entries, there is a point (a range of iterations) that we can put an event
                // between two list items in the same chunk.
                ReturnErrorOnFailure(encoder.Encode(ByteSpan(sAnStringThatCanNeverFitIntoTheMTU, kSizeOfLargeAttribute)));
            }
            return CHIP_NO_ERROR;
        });
    default:
        return aEncoder.Encode((uint8_t) gIterationCount);
    }
}

CHIP_ERROR TestAttrAccess::Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

void GenerateEvents(chip::EventNumber & firstEventNumber, chip::EventNumber & lastEventNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Clusters::UnitTesting::Events::TestEvent::Type content;
    content.arg1 = static_cast<uint8_t>(gIterationCount);

    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ((err = app::LogEvent(content, kTestEndpointId, lastEventNumber)), CHIP_NO_ERROR);
        if (i == 0)
        {
            firstEventNumber = lastEventNumber;
        }
    }
}

/*
 * This validates all the various corner cases encountered during chunking by
 * artificially reducing the size of a packet buffer used to encode attribute & event data
 * to force chunking to happen over multiple packets even with a small number of attributes or events
 * and then slowly increasing the available size by 1 byte in each test iteration and re-running
 * the report generation logic. This 1-byte incremental approach sweeps through from a base scenario of
 * N attributes fitting in a report, to eventually resulting in N+1 attributes or events fitting in a report.

 * This will cause all the various corner cases encountered of closing out the various containers within
 * the report and thoroughly and definitely validate those edge cases.
 *
 * Importantly, this test tries to re-use *as much as possible* the actual IM constructs used by real
 * server-side applications. Consequently, this is why it registers a dynamic endpoint + fake attribute access + fake event
 * generation interface to simulate faithfully a real application. This ensures validation of as much production logic pathways
 * as we can possibly cover.
 *
 */
TEST_F(TestEventChunking, TestEventChunking)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    CodegenDataModelProviderInstance(nullptr /* delegate */)->Shutdown();
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    chip::EventNumber firstEventNumber;
    chip::EventNumber lastEventNumber;

    GenerateEvents(firstEventNumber, lastEventNumber);

    app::EventPathParams eventPath;
    eventPath.mEndpointId = kTestEndpointId;
    eventPath.mClusterId  = app::Clusters::UnitTesting::Id;
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpEventPathParamsList    = &eventPath;
    readParams.mEventPathParamsListSize = 1;
    readParams.mEventNumber.SetValue(firstEventNumber);

    // Since we will always read from the first event, we only generate event once.

    //
    // We've empirically determined that by reserving 950 bytes in the packet buffer, we can fit 2
    // AttributeDataIBs into the packet. ~30-40 bytes covers a single EventDataIB, but let's 2-3x that
    // to ensure we'll sweep from fitting 2 IBs to 3-4 IBs.
    //
    for (int i = 100; i > 0; i--)
    {
        TestReadCallback readCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", i);

        gIterationCount = (uint32_t) i;

        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(static_cast<uint32_t>(800 + i));

        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(readCallback.mEventCount, static_cast<uint32_t>((lastEventNumber - firstEventNumber) + 1));
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (HasFailure())
        {
            break;
        }
    }

    emberAfClearDynamicEndpoint(0);
}

// Similar to the tests above, but it will read attributes AND events
TEST_F(TestEventChunking, TestMixedEventsAndAttributesChunking)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    CodegenDataModelProviderInstance(nullptr /* delegate */)->Shutdown();
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    chip::EventNumber firstEventNumber;
    chip::EventNumber lastEventNumber;

    // We will always read from the first event, so it is enough to only generate events once.
    GenerateEvents(firstEventNumber, lastEventNumber);

    app::EventPathParams eventPath;
    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::UnitTesting::Id);
    eventPath.mEndpointId = kTestEndpointId;
    eventPath.mClusterId  = app::Clusters::UnitTesting::Id;
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mpEventPathParamsList        = &eventPath;
    readParams.mEventPathParamsListSize     = 1;
    readParams.mEventNumber.SetValue(firstEventNumber);

    //
    // We've empirically determined that by reserving 950 bytes in the packet buffer, we can fit 2
    // AttributeDataIBs into the packet. ~30-40 bytes covers a single EventDataIB, but let's 2-3x that
    // to ensure we'll sweep from fitting 2 IBs to 3-4 IBs.
    //
    for (int i = 100; i > 0; i--)
    {
        TestReadCallback readCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", i);

        gIterationCount = (uint32_t) i;

        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(static_cast<uint32_t>(800 + i));

        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        //
        // Always returns the same number of attributes read (5 + revision + GlobalAttributesNotInMetadata).
        //
        EXPECT_TRUE(readCallback.mOnReportEnd);
        EXPECT_EQ(readCallback.mAttributeCount, 6 + MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata));
        EXPECT_EQ(readCallback.mEventCount, static_cast<uint32_t>(lastEventNumber - firstEventNumber + 1));

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (HasFailure())
        {
            break;
        }
    }

    emberAfClearDynamicEndpoint(0);
}

// Similar to the tests above, however, there is one another case -- the event payload is very large usually, so when it is failed
// to encode an attribute, it is usually impossible to encode a event data, so we cannot verify the case when events and attributes
// can be encoded in to one chunk in the tests above. This test will force it by reading only one attribtue and read many events.
TEST_F(TestEventChunking, TestMixedEventsAndLargeAttributesChunking)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    CodegenDataModelProviderInstance(nullptr /* delegate */)->Shutdown();
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint4, Span<DataVersion>(dataVersionStorage));

    chip::EventNumber firstEventNumber;
    chip::EventNumber lastEventNumber;

    // We will always read from the first event, so it is enough to only generate events once.
    GenerateEvents(firstEventNumber, lastEventNumber);

    app::EventPathParams eventPath;
    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::UnitTesting::Id, kTestListLargeAttribute);
    eventPath.mEndpointId = kTestEndpointId;
    eventPath.mClusterId  = app::Clusters::UnitTesting::Id;
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mpEventPathParamsList        = &eventPath;
    readParams.mEventPathParamsListSize     = 1;
    readParams.mEventNumber.SetValue(firstEventNumber);

    //
    // We've empirically determined that by reserving 950 bytes in the packet buffer, we can fit 2
    // AttributeDataIBs into the packet. ~30-40 bytes covers a single EventDataIB, but let's 2-3x that
    // to ensure we'll sweep from fitting 2 IBs to 3-4 IBs.
    //
    for (int i = 100; i > 0; i--)
    {
        TestReadCallback readCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", i);

        gIterationCount = (uint32_t) i;

        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(static_cast<uint32_t>(800 + i));

        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(readCallback.mOnReportEnd);
        EXPECT_EQ(readCallback.mAttributeCount, 1u);
        EXPECT_EQ(readCallback.mEventCount, static_cast<uint32_t>(lastEventNumber - firstEventNumber + 1));

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (HasFailure())
        {
            break;
        }
    }

    emberAfClearDynamicEndpoint(0);
}

} // namespace
