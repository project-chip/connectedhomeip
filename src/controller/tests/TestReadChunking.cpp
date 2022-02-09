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
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

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
constexpr EndpointId kTestEndpointId3    = 3;
constexpr AttributeId kTestListAttribute = 6;
constexpr AttributeId kTestBadAttribute  = 7; // Reading this attribute will return CHIP_NO_MEMORY but nothing is actually encoded.

class TestCommandInteraction
{
public:
    TestCommandInteraction() {}
    static void TestChunking(nlTestSuite * apSuite, void * apContext);
    static void TestListChunking(nlTestSuite * apSuite, void * apContext);
    static void TestBadChunking(nlTestSuite * apSuite, void * apContext);

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

//clang-format on

uint8_t sAnStringThatCanNeverFitIntoTheMTU[4096] = { 0 };

class TestReadCallback : public app::ReadClient::Callback
{
public:
    TestReadCallback() : mBufferedCallback(*this) {}
    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, DataVersion aVersion, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override;

    void OnDone() override;

    void OnReportEnd() override { mOnReportEnd = true; }

    uint32_t mAttributeCount = 0;
    bool mOnReportEnd        = false;
    app::BufferedReadCallback mBufferedCallback;
};

void TestReadCallback::OnAttributeData(const app::ConcreteDataAttributePath & aPath, DataVersion aVersion, TLV::TLVReader * apData,
                                       const app::StatusIB & aStatus)
{
    if (aPath.mAttributeId != kTestListAttribute)
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
        NL_TEST_ASSERT(gSuite, arraySize = 5);
    }
    mAttributeCount++;
}

void TestReadCallback::OnDone() {}

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

        //
        // Service the IO + Engine till we get a ReportEnd callback on the client.
        // Since bugs can happen, we don't want this test to never stop, so create a ceiling for how many
        // times this can run without seeing expected results.
        //
        for (int j = 0; j < 10 && !readCallback.mOnReportEnd; j++)
        {
            ctx.DrainAndServiceIO();
            chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
            ctx.DrainAndServiceIO();
        }

        //
        // Always returns the same number of attributes read (5 + revision = 6).
        //
        NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount == 6);
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

        //
        // Service the IO + Engine till we get a ReportEnd callback on the client.
        // Since bugs can happen, we don't want this test to never stop, so create a ceiling for how many
        // times this can run without seeing expected results.
        //
        for (int j = 0; j < 10 && !readCallback.mOnReportEnd; j++)
        {
            ctx.DrainAndServiceIO();
            chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
            ctx.DrainAndServiceIO();
        }

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

        //
        // Service the IO + Engine till we get a ReportEnd callback on the client.
        // The server should return an empty list as attribute data for the first report (for list chunking), and encodes nothing
        // (then shuts down the read handler) for the second report.
        //
        for (int j = 0; j < 2; j++)
        {
            ctx.DrainAndServiceIO();
            chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
            ctx.DrainAndServiceIO();
        }

        // Nothing is actually encoded. buffered callback does not handle the message to us.
        NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount == 0);

        // The server should shutted down, while the client is still alive (pending for the attribute data.)
        NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);
    }

    // Sanity check
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestChunking", TestCommandInteraction::TestChunking),
    NL_TEST_DEF("TestListChunking", TestCommandInteraction::TestListChunking),
    NL_TEST_DEF("TestBadChunking", TestCommandInteraction::TestBadChunking),
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
