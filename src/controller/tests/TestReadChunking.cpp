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

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app/ConcreteAttributePath.h"
#include "protocols/interaction_model/Constants.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppBuildConfig.h>
#include <app/AttributeAccessInterface.h>
#include <app/BufferedReadCallback.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <functional>
#include <lib/support/ErrorStr.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <map>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>
#include <utility>

using TestContext = chip::Test::AppContext;
using namespace chip;
using namespace chip::app::Clusters;

namespace {

uint32_t gIterationCount = 0;
nlTestSuite * gSuite     = nullptr;

//
// The generated endpoint_config for the controller app has Endpoint 1
// already used in the fixed endpoint set of size 1. Consequently, let's use the next
// number higher than that for our dynamic test endpoint.
//
constexpr EndpointId kTestEndpointId = 2;
// Another endpoint, with a list attribute only.
constexpr EndpointId kTestEndpointId3 = 3;
// Another endpoint, for adding / enabling during running.
constexpr EndpointId kTestEndpointId4    = 4;
constexpr EndpointId kTestEndpointId5    = 5;
constexpr AttributeId kTestListAttribute = 6;
constexpr AttributeId kTestBadAttribute  = 7; // Reading this attribute will return CHIP_NO_MEMORY but nothing is actually encoded.

class TestCommandInteraction
{
public:
    TestCommandInteraction() {}
    static void TestChunking(nlTestSuite * apSuite, void * apContext);
    static void TestListChunking(nlTestSuite * apSuite, void * apContext);
    static void TestBadChunking(nlTestSuite * apSuite, void * apContext);
    static void TestDynamicEndpoint(nlTestSuite * apSuite, void * apContext);
    static void TestSetDirtyBetweenChunks(nlTestSuite * apSuite, void * apContext);

private:
};

//clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(0x00000001, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000002, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000003, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000004, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000005, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(TestCluster::Id, testClusterAttrs, nullptr, nullptr), DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint3)
DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute, ARRAY, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(kTestBadAttribute, ARRAY, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpoint3Clusters)
DECLARE_DYNAMIC_CLUSTER(TestCluster::Id, testClusterAttrsOnEndpoint3, nullptr, nullptr), DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint3, testEndpoint3Clusters);

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint4)
DECLARE_DYNAMIC_ATTRIBUTE(0x00000001, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpoint4Clusters)
DECLARE_DYNAMIC_CLUSTER(TestCluster::Id, testClusterAttrsOnEndpoint4, nullptr, nullptr), DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint4, testEndpoint4Clusters);

// Unlike endpoint 1, we can modify the values for values in endpoint 5
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint5)
DECLARE_DYNAMIC_ATTRIBUTE(0x00000001, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000002, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000003, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpoint5Clusters)
DECLARE_DYNAMIC_CLUSTER(TestCluster::Id, testClusterAttrsOnEndpoint5, nullptr, nullptr), DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint5, testEndpoint5Clusters);

//clang-format on

uint8_t sAnStringThatCanNeverFitIntoTheMTU[4096] = { 0 };

class TestReadCallback : public app::ReadClient::Callback
{
public:
    TestReadCallback() : mBufferedCallback(*this) {}
    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override;

    void OnDone() override;

    void OnReportEnd() override { mOnReportEnd = true; }

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override { mOnSubscriptionEstablished = true; }

    uint32_t mAttributeCount        = 0;
    bool mOnReportEnd               = false;
    bool mOnSubscriptionEstablished = false;
    app::BufferedReadCallback mBufferedCallback;
};

void TestReadCallback::OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                       const app::StatusIB & aStatus)
{
    if (aPath.mAttributeId == Globals::Attributes::ServerGeneratedCommandList::Id)
    {
        app::DataModel::DecodableList<CommandId> v;
        NL_TEST_ASSERT(gSuite, app::DataModel::Decode(*apData, v) == CHIP_NO_ERROR);
        auto it          = v.begin();
        size_t arraySize = 0;
        while (it.Next())
        {
            NL_TEST_ASSERT(gSuite, false);
        }
        NL_TEST_ASSERT(gSuite, it.GetStatus() == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, v.ComputeSize(&arraySize) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, arraySize == 0);
    }
    else if (aPath.mAttributeId == Globals::Attributes::ClientGeneratedCommandList::Id)
    {
        app::DataModel::DecodableList<CommandId> v;
        NL_TEST_ASSERT(gSuite, app::DataModel::Decode(*apData, v) == CHIP_NO_ERROR);
        auto it          = v.begin();
        size_t arraySize = 0;
        while (it.Next())
        {
            NL_TEST_ASSERT(gSuite, false);
        }
        NL_TEST_ASSERT(gSuite, it.GetStatus() == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, v.ComputeSize(&arraySize) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, arraySize == 0);
    }
    else if (aPath.mAttributeId == Globals::Attributes::AttributeList::Id)
    {
        // Nothing to check for this one; depends on the endpoint.
    }
    else if (aPath.mAttributeId != kTestListAttribute)
    {
        uint8_t v;
        NL_TEST_ASSERT(gSuite, app::DataModel::Decode(*apData, v) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, v == (uint8_t) gIterationCount);
    }
    else
    {
        app::DataModel::DecodableList<uint8_t> v;
        NL_TEST_ASSERT(gSuite, app::DataModel::Decode(*apData, v) == CHIP_NO_ERROR);
        auto it          = v.begin();
        size_t arraySize = 0;
        while (it.Next())
        {
            NL_TEST_ASSERT(gSuite, it.GetValue() == static_cast<uint8_t>(gIterationCount));
        }
        NL_TEST_ASSERT(gSuite, it.GetStatus() == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, v.ComputeSize(&arraySize) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, arraySize == 5);
    }
    mAttributeCount++;
}

void TestReadCallback::OnDone() {}

class TestMutableAttrAccess
{
public:
    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder);

    void SetDirty(AttributeId attr)
    {
        app::ClusterInfo path;
        path.mEndpointId  = kTestEndpointId5;
        path.mClusterId   = TestCluster::Id;
        path.mAttributeId = attr;
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    }

    // These setters
    void SetVal(uint8_t attribute, uint8_t newVal)
    {
        uint8_t index = static_cast<uint8_t>(attribute - 1);
        if (index < ArraySize(val) && val[index] != newVal)
        {
            val[index] = newVal;
            SetDirty(attribute);
        }
    }

    void Reset() { val[0] = val[1] = val[2] = 0; }

    uint8_t val[3] = { 0, 0, 0 };
};

CHIP_ERROR TestMutableAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    uint8_t index = static_cast<uint8_t>(aPath.mAttributeId - 1);
    VerifyOrReturnError(aPath.mEndpointId == kTestEndpointId5 && index < ArraySize(val), CHIP_ERROR_NOT_FOUND);
    return aEncoder.Encode(val[index]);
}

TestMutableAttrAccess gMutableAttrAccess;

class TestAttrAccess : public app::AttributeAccessInterface
{
public:
    // Register for the Test Cluster cluster on all endpoints.
    TestAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TestCluster::Id)
    {
        registerAttributeAccessOverride(this);
    }

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder) override;
};

TestAttrAccess gAttrAccess;

CHIP_ERROR TestAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = gMutableAttrAccess.Read(aPath, aEncoder);
    if (err != CHIP_ERROR_NOT_FOUND)
    {
        return err;
    }

    switch (aPath.mAttributeId)
    {
    case kTestListAttribute:
        return aEncoder.EncodeList([](const auto & encoder) {
            for (int i = 0; i < 5; i++)
            {
                ReturnErrorOnFailure(encoder.Encode((uint8_t) gIterationCount));
            }
            return CHIP_NO_ERROR;
        });
    case kTestBadAttribute:
        // The "BadAttribute" is implemented by encoding a very large octet string, then the encode will always return
        // CHIP_NO_MEMORY.
        return aEncoder.EncodeList([](const auto & encoder) {
            return encoder.Encode(ByteSpan(sAnStringThatCanNeverFitIntoTheMTU, sizeof(sAnStringThatCanNeverFitIntoTheMTU)));
        });
    default:
        return aEncoder.Encode((uint8_t) gIterationCount);
    }
}

CHIP_ERROR TestAttrAccess::Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

class TestMutableReadCallback : public app::ReadClient::Callback
{
public:
    TestMutableReadCallback() : mBufferedCallback(*this) {}
    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override;

    void OnDone() override {}

    void OnReportBegin() override { mAttributeCount = 0; }

    void OnReportEnd() override { mOnReportEnd = true; }

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override { mOnSubscriptionEstablished = true; }

    uint32_t mAttributeCount = 0;
    // We record every dataversion field from every attribute IB.
    std::map<std::pair<EndpointId, AttributeId>, DataVersion> mDataVersions;
    std::map<std::pair<EndpointId, AttributeId>, uint8_t> mValues;
    std::map<std::pair<EndpointId, AttributeId>, std::function<void(EndpointId, AttributeId)>> mActions;
    bool mOnReportEnd               = false;
    bool mOnSubscriptionEstablished = false;
    app::BufferedReadCallback mBufferedCallback;
};

void TestMutableReadCallback::OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                              const app::StatusIB & aStatus)
{
    VerifyOrReturn(apData != nullptr);
    NL_TEST_ASSERT(gSuite, aPath.mClusterId == TestCluster::Id);

    mAttributeCount++;
    if (aPath.mAttributeId <= 5)
    {
        uint8_t v;
        NL_TEST_ASSERT(gSuite, app::DataModel::Decode(*apData, v) == CHIP_NO_ERROR);
        mValues[std::make_pair(aPath.mEndpointId, aPath.mAttributeId)] = v;

        auto action = mActions.find(std::make_pair(aPath.mEndpointId, aPath.mAttributeId));
        if (action != mActions.end() && action->second)
        {
            action->second(aPath.mEndpointId, aPath.mAttributeId);
        }
    }

    if (aPath.mDataVersion.HasValue())
    {
        mDataVersions[std::make_pair(aPath.mEndpointId, aPath.mAttributeId)] = aPath.mDataVersion.Value();
    }
    // Ignore all other attributes, we don't care above the global attributes.
}

/*
 * This validates all the various corner cases encountered during chunking by
 * artificially reducing the size of a packet buffer used to encode attribute data
 * to force chunking to happen over multiple packets even with a small number of attributes
 * and then slowly increasing the available size by 1 byte in each test iteration and re-running
 * the report generation logic. This 1-byte incremental approach sweeps through from a base scenario of
 * N attributes fitting in a report, to eventually resulting in N+1 attributes fitting in a report.

 * This will cause all the various corner cases encountered of closing out the various containers within
 * the report and thoroughly and definitely validate those edge cases.
 *
 * Importantly, this test tries to re-use *as much as possible* the actual IM constructs used by real
 * server-side applications. Consequently, this is why it registers a dynamic endpoint + fake attribute access
 * interface to simulate faithfully a real application. This ensures validation of as much production logic pathways
 * as we can possibly cover.
 *
 */
void TestCommandInteraction::TestChunking(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                    = *static_cast<TestContext *>(apContext);
    auto sessionHandle                   = ctx.GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[ArraySize(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, 0, 0, Span<DataVersion>(dataVersionStorage));

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::TestCluster::Id);
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;

    //
    // We've empirically determined that by reserving 950 bytes in the packet buffer, we can fit 2
    // AttributeDataIBs into the packet. ~30-40 bytes covers a single AttributeDataIB, but let's 2-3x that
    // to ensure we'll sweep from fitting 2 IBs to 3-4 IBs.
    //
    for (int i = 100; i > 0; i--)
    {
        TestReadCallback readCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", i);

        gIterationCount = (uint32_t) i;

        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(static_cast<uint32_t>(850 + i));

        app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);

        //
        // Always returns the same number of attributes read (5 + revision +
        // AttributeList + ClientGeneratedCommandList +
        // ServerGeneratedCommandList = 9).
        //
        NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount == 9);
        readCallback.mAttributeCount = 0;

        NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (apSuite->flagError)
        {
            break;
        }
    }

    emberAfClearDynamicEndpoint(0);
}

// Similar to the test above, but for the list chunking feature.
void TestCommandInteraction::TestListChunking(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                    = *static_cast<TestContext *>(apContext);
    auto sessionHandle                   = ctx.GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[ArraySize(testEndpoint3Clusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId3, &testEndpoint3, 0, 0, Span<DataVersion>(dataVersionStorage));

    app::AttributePathParams attributePath(kTestEndpointId3, app::Clusters::TestCluster::Id, kTestListAttribute);
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;

    //
    // We've empirically determined that by reserving 950 bytes in the packet buffer, we can fit 2
    // AttributeDataIBs into the packet. ~30-40 bytes covers a single AttributeDataIB, but let's 2-3x that
    // to ensure we'll sweep from fitting 2 IBs to 3-4 IBs.
    //
    for (int i = 100; i > 0; i--)
    {
        TestReadCallback readCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", i);

        gIterationCount = (uint32_t) i;

        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(static_cast<uint32_t>(850 + i));

        app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);

        //
        // Always returns the same number of attributes read (merged by buffered read callback). The content is checked in
        // TestReadCallback::OnAttributeData
        //
        NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount == 1);
        readCallback.mAttributeCount = 0;

        NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (apSuite->flagError)
        {
            break;
        }
    }

    emberAfClearDynamicEndpoint(0);
}

// Read an attribute that can never fit into the buffer. Result in an empty report, server should shutdown the transaction.
void TestCommandInteraction::TestBadChunking(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                    = *static_cast<TestContext *>(apContext);
    auto sessionHandle                   = ctx.GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(0);

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[ArraySize(testEndpoint3Clusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId3, &testEndpoint3, 0, 0, Span<DataVersion>(dataVersionStorage));

    app::AttributePathParams attributePath(kTestEndpointId3, app::Clusters::TestCluster::Id, kTestBadAttribute);
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;

    TestReadCallback readCallback;

    {
        app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        // The server should return an empty list as attribute data for the first report (for list chunking), and encodes nothing
        // (then shuts down the read handler) for the second report.
        //

        // Nothing is actually encoded. buffered callback does not handle the message to us.
        NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount == 0);
        NL_TEST_ASSERT(apSuite, !readCallback.mOnReportEnd);

        // The server should shutted down, while the client is still alive (pending for the attribute data.)
        NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    // Sanity check
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(0);
}

/*
 * This test contains two parts, one is to enable a new endpoint on the fly, another is to disable it and re-enable it.
 */
void TestCommandInteraction::TestDynamicEndpoint(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                    = *static_cast<TestContext *>(apContext);
    auto sessionHandle                   = ctx.GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[ArraySize(testEndpoint4Clusters)];

    app::AttributePathParams attributePath;
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mMaxIntervalCeilingSeconds   = 1;

    TestReadCallback readCallback;

    {

        app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Subscribe);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        // We should not receive any reports in initial reports, so check mOnSubscriptionEstablished instead.
        NL_TEST_ASSERT(apSuite, readCallback.mOnSubscriptionEstablished);
        readCallback.mAttributeCount = 0;

        // Enable the new endpoint
        emberAfSetDynamicEndpoint(0, kTestEndpointId4, &testEndpoint4, 0, 0, Span<DataVersion>(dataVersionStorage));

        ctx.DrainAndServiceIO();

        // Ensure we have received the report, we do not care about the initial report here.
        // ClientGeneratedCommandList / ServerGeneratedCommandList / AttributeList attribute are not included in
        // testClusterAttrsOnEndpoint4.
        NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount == ArraySize(testClusterAttrsOnEndpoint4) + 3);

        // We have received all report data.
        NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);

        readCallback.mAttributeCount = 0;
        readCallback.mOnReportEnd    = false;

        // Disable the new endpoint
        emberAfEndpointEnableDisable(kTestEndpointId4, false);

        ctx.DrainAndServiceIO();

        // We may receive some attribute reports for descriptor cluster, but we do not care about it for now.

        // Enable the new endpoint

        readCallback.mAttributeCount = 0;
        readCallback.mOnReportEnd    = false;

        emberAfEndpointEnableDisable(kTestEndpointId4, true);
        ctx.DrainAndServiceIO();

        // Ensure we have received the report, we do not care about the initial report here.
        // ClientGeneratedCommandList / ServerGeneratedCommandList / AttributeList attribute are not include in
        // testClusterAttrsOnEndpoint4.
        NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount == ArraySize(testClusterAttrsOnEndpoint4) + 3);

        // We have received all report data.
        NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);
    }

    chip::test_utils::SleepMillis(secondsToMilliseconds(2));

    // Destroying the read client will terminate the subscription transaction.
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(0);
}

/*
 * The tests below are for testing deatiled bwhavior when the attributes are modified between two chunks. In this test, we only care
 * above whether we will receive correct attribute values in reasonable messages with reduced reporting traffic.
 */
void TestCommandInteraction::TestSetDirtyBetweenChunks(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                    = *static_cast<TestContext *>(apContext);
    auto sessionHandle                   = ctx.GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(0);
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetMaxAttributesPerChunk(2);

    DataVersion dataVersionStorage1[ArraySize(testEndpointClusters)];
    DataVersion dataVersionStorage5[ArraySize(testEndpoint5Clusters)];

    gMutableAttrAccess.Reset();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, 0, 0, Span<DataVersion>(dataVersionStorage1));
    emberAfSetDynamicEndpoint(1, kTestEndpointId5, &testEndpoint5, 0, 0, Span<DataVersion>(dataVersionStorage5));

// The ReadHandler uses ScheduleWork to kick the report engine without a network traffic, which cannot be handled by DriveIO
// directly. Also, we want the event loop to stop when we fully received one report. But DrainAndServiceIO cannot be stopped by some
// conditions.
#define DRIVE_IO_UNTIL_END_OF_NEXT_REPORT(timeoutSeconds)                                                                          \
    readCallback.mOnReportEnd = false;                                                                                             \
    ctx.GetIOContext().DriveIOUntil(System::Clock::Seconds16(timeoutSeconds), [&]() { return readCallback.mOnReportEnd; });        \
    NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);

    {
        app::AttributePathParams attributePath;
        app::ReadPrepareParams readParams(sessionHandle);

        readParams.mpAttributePathParamsList    = &attributePath;
        readParams.mAttributePathParamsListSize = 1;
        readParams.mMinIntervalFloorSeconds     = 0;
        readParams.mMaxIntervalCeilingSeconds   = 2;

        //// TEST 1 -- Read using wildcard paths
        {
            TestMutableReadCallback readCallback;

            gIterationCount = 1;

            // When the report engine starts to report attributes in endpoint 5, mark cluster 1 as dirty.
            // The report engine should NOT include it in initial report to reduce traffic.
            readCallback.mActions[std::make_pair(kTestEndpointId5, 1)] = [](EndpointId, AttributeId) {
                app::ClusterInfo path;
                path.mEndpointId  = kTestEndpointId;
                path.mClusterId   = TestCluster::Id;
                path.mAttributeId = 1;
                gIterationCount   = 2;
                app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
            };

            app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mBufferedCallback,
                                       app::ReadClient::InteractionType::Subscribe);

            NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

            // Wait for next reporting session.
            ctx.GetIOContext().DriveIOUntil(System::Clock::Seconds16(2), [&]() { return readCallback.mOnSubscriptionEstablished; });

            NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);
            NL_TEST_ASSERT(apSuite, readCallback.mOnSubscriptionEstablished);

            // We are expected to miss attributes on kTestEndpointId during initial reports.
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId, 1)] == 1);

            readCallback.mActions.clear();

            DRIVE_IO_UNTIL_END_OF_NEXT_REPORT(2);

            // The missing attribute should be reported now.
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId, 1)] == 2);

            readCallback.mActions.clear();
            readCallback.mDataVersions.clear();
            readCallback.mActions[std::make_pair(kTestEndpointId5, 2)] = [](EndpointId, AttributeId) {
                gMutableAttrAccess.SetVal(3, 3);
            };
            gMutableAttrAccess.SetVal(1, 2);
            gMutableAttrAccess.SetVal(2, 2);
            gMutableAttrAccess.SetVal(3, 2);

            DRIVE_IO_UNTIL_END_OF_NEXT_REPORT(2);

            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 1)] == 2);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 2)] == 2);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 3)] == 3);
            // It should report attribute 1 and attribute 2 since that are under the same cluster as cluster 3.
            // We should see unique data versions from the same report session if they comes from the same attribute path params.
            NL_TEST_ASSERT(apSuite,
                           readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 1)] ==
                               readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 2)]);
            NL_TEST_ASSERT(apSuite,
                           readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 1)] ==
                               readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 3)]);

            app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetMaxAttributesPerChunk(1);

            readCallback.mOnReportEnd = false;
            readCallback.mActions.clear();
            readCallback.mDataVersions.clear();
            readCallback.mActions[std::make_pair(kTestEndpointId5, 2)] = [](EndpointId, AttributeId) {
                gMutableAttrAccess.SetVal(3, 4);
            };
            gMutableAttrAccess.SetVal(1, 4);
            gMutableAttrAccess.SetVal(2, 4);

            DRIVE_IO_UNTIL_END_OF_NEXT_REPORT(2);

            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 1)] == 4);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 2)] == 4);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 3)] == 4);
            // It should report attribute 1 and attribute 2 since that are under the same cluster as cluster 3.
            // We should see unique data versions from the same report session.
            NL_TEST_ASSERT(apSuite,
                           readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 1)] ==
                               readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 2)]);
            NL_TEST_ASSERT(apSuite,
                           readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 1)] ==
                               readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 3)]);
            // We have received all report data.
            NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);

            readCallback.mOnReportEnd = false;
            readCallback.mActions.clear();
            readCallback.mDataVersions.clear();
            readCallback.mActions[std::make_pair(kTestEndpointId5, 2)] = [](EndpointId, AttributeId) {
                gMutableAttrAccess.SetVal(1, 5);
            };
            gMutableAttrAccess.SetVal(2, 5);
            gMutableAttrAccess.SetVal(3, 5);

            DRIVE_IO_UNTIL_END_OF_NEXT_REPORT(2);

            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 1)] == 5);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 2)] == 5);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 3)] == 5);
            // It should report attribute 1 and attribute 2 since that are under the same cluster as cluster 3.
            // We should see unique data versions from the same report session.
            NL_TEST_ASSERT(apSuite,
                           readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 1)] ==
                               readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 2)]);
            NL_TEST_ASSERT(apSuite,
                           readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 1)] ==
                               readCallback.mDataVersions[std::make_pair(kTestEndpointId5, 3)]);
            // We have received all report data.
            NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);
        }
    }
    // The read client is destructed, server will shutdown the corresponding subscription later.

    {
        app::AttributePathParams attributePath[3];
        app::ReadPrepareParams readParams(sessionHandle);

        attributePath[0] = app::AttributePathParams(kTestEndpointId5, TestCluster::Id, 1);
        attributePath[1] = app::AttributePathParams(kTestEndpointId5, TestCluster::Id, 2);
        attributePath[2] = app::AttributePathParams(kTestEndpointId5, TestCluster::Id, 3);

        readParams.mpAttributePathParamsList    = attributePath;
        readParams.mAttributePathParamsListSize = 3;
        readParams.mMinIntervalFloorSeconds     = 0;
        readParams.mMaxIntervalCeilingSeconds   = 2;
        gMutableAttrAccess.Reset();

        //// TEST 2 -- Read using concrete paths
        {
            TestMutableReadCallback readCallback;

            app::ReadClient readClient(engine, &ctx.GetExchangeManager(), readCallback.mBufferedCallback,
                                       app::ReadClient::InteractionType::Subscribe);

            NL_TEST_ASSERT(apSuite, readClient.SendRequest(readParams) == CHIP_NO_ERROR);

            readCallback.mOnReportEnd = false;
            ctx.GetIOContext().DriveIOUntil(System::Clock::Seconds16(5), [&]() { return readCallback.mOnSubscriptionEstablished; });
            NL_TEST_ASSERT(apSuite, readCallback.mOnReportEnd);
            NL_TEST_ASSERT(apSuite, readCallback.mOnSubscriptionEstablished);

            readCallback.mActions.clear();
            readCallback.mDataVersions.clear();
            readCallback.mActions[std::make_pair(kTestEndpointId5, 2)] = [](EndpointId, AttributeId) {
                gMutableAttrAccess.SetVal(3, 4);
            };
            gMutableAttrAccess.SetVal(1, 3);
            gMutableAttrAccess.SetVal(2, 3);
            gMutableAttrAccess.SetVal(3, 3);

            DRIVE_IO_UNTIL_END_OF_NEXT_REPORT(2);

            // Note, although the two attributes comes from the same cluster, they are generated by different interested paths.
            // In this case, we won't reset the path iterator.
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 1)] == 3);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 2)] == 3);
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 3)] == 3);

            readCallback.mActions.clear();

            DRIVE_IO_UNTIL_END_OF_NEXT_REPORT(2);

            // The attribute failed to catch last report will be picked by this report.
            NL_TEST_ASSERT(apSuite, readCallback.mValues[std::make_pair(kTestEndpointId5, 3)] == 4);
        }
    }

#undef DRIVE_IO_UNTIL_END_OF_NEXT_REPORT

    chip::test_utils::SleepMillis(secondsToMilliseconds(3));

    // Destroying the read client will terminate the subscription transaction.
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(1);
    emberAfClearDynamicEndpoint(0);
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetMaxAttributesPerChunk(UINT32_MAX);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestChunking", TestCommandInteraction::TestChunking),
    NL_TEST_DEF("TestListChunking", TestCommandInteraction::TestListChunking),
    NL_TEST_DEF("TestBadChunking", TestCommandInteraction::TestBadChunking),
    NL_TEST_DEF("TestDynamicEndpoint", TestCommandInteraction::TestDynamicEndpoint),
    NL_TEST_DEF("TestSetDirtyBetweenChunks", TestCommandInteraction::TestSetDirtyBetweenChunks),
    NL_TEST_SENTINEL()
};

// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestReadChunking",
    &sTests[0],
    TestContext::InitializeAsync,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestReadChunkingTests()
{
    TestContext gContext;
    gSuite = &sSuite;
    nlTestRunner(&sSuite, &gContext);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReadChunkingTests)
