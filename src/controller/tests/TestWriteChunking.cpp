/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app/AppConfig.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

#include <memory>
#include <utility>

using TestContext = chip::Test::AppContext;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

uint32_t gIterationCount = 0;
nlTestSuite * gSuite     = nullptr;

//
// The generated endpoint_config for the controller app has Endpoint 1
// already used in the fixed endpoint set of size 1. Consequently, let's use the next
// number higher than that for our dynamic test endpoint.
//
constexpr EndpointId kTestEndpointId      = 2;
constexpr AttributeId kTestListAttribute  = 6;
constexpr AttributeId kTestListAttribute2 = 7;
constexpr uint32_t kTestListLength        = 5;

// We don't really care about the content, we just need a buffer.
uint8_t sByteSpanData[app::kMaxSecureSduLengthBytes];

class TestWriteChunking
{
public:
    TestWriteChunking() {}
    static void TestListChunking(nlTestSuite * apSuite, void * apContext);
    static void TestBadChunking(nlTestSuite * apSuite, void * apContext);
    static void TestConflictWrite(nlTestSuite * apSuite, void * apContext);
    static void TestNonConflictWrite(nlTestSuite * apSuite, void * apContext);
    static void TestTransactionalList(nlTestSuite * apSuite, void * apContext);

private:
};

//clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint)
DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute, ARRAY, 1, ATTRIBUTE_MASK_WRITABLE),
    DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute2, ARRAY, 1, ATTRIBUTE_MASK_WRITABLE), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(TestCluster::Id, testClusterAttrsOnEndpoint, nullptr, nullptr), DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

DataVersion dataVersionStorage[ArraySize(testEndpointClusters)];

//clang-format on

class TestWriteCallback : public app::WriteClient::Callback
{
public:
    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteDataAttributePath & aPath,
                    app::StatusIB status) override
    {
        if (status.mStatus == Protocols::InteractionModel::Status::Success)
        {
            mSuccessCount++;
        }
        else
        {
            mLastErrorReason = status;
            mErrorCount++;
        }
    }

    void OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aError) override
    {
        mLastErrorReason = app::StatusIB(aError);
        mErrorCount++;
    }

    void OnDone(app::WriteClient * apWriteClient) override { mOnDoneCount++; }

    uint32_t mSuccessCount = 0;
    uint32_t mErrorCount   = 0;
    uint32_t mOnDoneCount  = 0;
    app::StatusIB mLastErrorReason;
};

class TestAttrAccess : public app::AttributeAccessInterface
{
public:
    // Register for the Test Cluster cluster on all endpoints.
    TestAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TestCluster::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder) override;

    void OnListWriteBegin(const app::ConcreteAttributePath & aPath) override
    {
        if (mOnListWriteBegin)
        {
            mOnListWriteBegin(aPath);
        }
    }

    void OnListWriteEnd(const app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override
    {
        if (mOnListWriteEnd)
        {
            mOnListWriteEnd(aPath, aWriteWasSuccessful);
        }
    }

    std::function<void(const app::ConcreteAttributePath & path)> mOnListWriteBegin;
    std::function<void(const app::ConcreteAttributePath & path, bool wasSuccessful)> mOnListWriteEnd;
} testServer;

CHIP_ERROR TestAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR TestAttrAccess::Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder)
{
    // We only care about the number of attribute data.
    if (!aPath.IsListItemOperation())
    {
        app::DataModel::Nullable<app::DataModel::DecodableList<ByteSpan>> list;
        CHIP_ERROR err = aDecoder.Decode(list);
        ChipLogError(Zcl, "Decode result: %s", err.AsString());
        return err;
    }
    if (aPath.mListOp == app::ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ByteSpan listItem;
        CHIP_ERROR err = aDecoder.Decode(listItem);
        ChipLogError(Zcl, "Decode result: %s", err.AsString());
        return err;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

/*
 * This validates all the various corner cases encountered during chunking by artificially reducing the size of a packet buffer used
 * to encode attribute data to force chunking to happen over multiple packets even with a small number of attributes and then slowly
 * increasing the available size by 1 byte in each test iteration and re-running the write request generation logic. This 1-byte
 * incremental approach sweeps through from a base scenario of N attributes fitting in a write request chunk, to eventually
 * resulting in N+1 attributes fitting in a write request chunk.
 *
 * This will cause all the various corner cases encountered of closing out the various containers within the write request and
 * thoroughly and definitely validate those edge cases.
 */
void TestWriteChunking::TestListChunking(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    registerAttributeAccessOverride(&testServer);

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::TestCluster::Id, kTestListAttribute);
    //
    // We've empirically determined that by reserving 950 bytes in the packet buffer, we can fit 2
    // AttributeDataIBs into the packet. ~30-40 bytes covers a single write chunk, but let's 2-3x that
    // to ensure we'll sweep from fitting 2 chunks to 3-4 chunks.
    //
    for (int i = 100; i > 0; i--)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TestWriteCallback writeCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", i);

        gIterationCount = (uint32_t) i;

        app::WriteClient writeClient(&ctx.GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
                                     static_cast<uint16_t>(850 + i) /* reserved buffer size */);

        ByteSpan list[kTestListLength];

        err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = writeClient.SendWriteRequest(sessionHandle);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        //
        // Service the IO + Engine till we get a ReportEnd callback on the client.
        // Since bugs can happen, we don't want this test to never stop, so create a ceiling for how many
        // times this can run without seeing expected results.
        //
        for (int j = 0; j < 10 && writeCallback.mOnDoneCount == 0; j++)
        {
            ctx.DrainAndServiceIO();
        }

        NL_TEST_ASSERT(apSuite,
                       writeCallback.mSuccessCount == kTestListLength + 1 /* an extra item for the empty list at the beginning */);
        NL_TEST_ASSERT(apSuite, writeCallback.mErrorCount == 0);
        NL_TEST_ASSERT(apSuite, writeCallback.mOnDoneCount == 1);

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

// We encode a pretty large write payload to test the corner cases related to message layer and secure session overheads.
// The test should gurantee that if encode returns no error, the send should also success.
// As the actual overhead may change, we will test over a few possible payload lengths, from 850 to MTU used in write clients.
void TestWriteChunking::TestBadChunking(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    bool atLeastOneRequestSent   = false;
    bool atLeastOneRequestFailed = false;

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    registerAttributeAccessOverride(&testServer);

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::TestCluster::Id, kTestListAttribute);

    for (int i = 850; i < static_cast<int>(chip::app::kMaxSecureSduLengthBytes); i++)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TestWriteCallback writeCallback;

        ChipLogDetail(DataManagement, "Running iteration with OCTET_STRING length = %d\n", i);

        gIterationCount = (uint32_t) i;

        app::WriteClient writeClient(&ctx.GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing());

        ByteSpan list[kTestListLength];
        for (uint8_t j = 0; j < kTestListLength; j++)
        {
            list[j] = ByteSpan(sByteSpanData, static_cast<uint32_t>(i));
        }

        err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
        if (err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            // This kind of error is expected.
            atLeastOneRequestFailed = true;
            continue;
        }

        atLeastOneRequestSent = true;

        // If we successfully encoded the attribute, then we must be able to send the message.
        err = writeClient.SendWriteRequest(sessionHandle);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        //
        // Service the IO + Engine till we get a ReportEnd callback on the client.
        // Since bugs can happen, we don't want this test to never stop, so create a ceiling for how many
        // times this can run without seeing expected results.
        //
        for (int j = 0; j < 10 && writeCallback.mOnDoneCount == 0; j++)
        {
            ctx.DrainAndServiceIO();
        }

        NL_TEST_ASSERT(apSuite,
                       writeCallback.mSuccessCount == kTestListLength + 1 /* an extra item for the empty list at the beginning */);
        NL_TEST_ASSERT(apSuite, writeCallback.mErrorCount == 0);
        NL_TEST_ASSERT(apSuite, writeCallback.mOnDoneCount == 1);

        NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (apSuite->flagError)
        {
            break;
        }
    }
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    NL_TEST_ASSERT(apSuite, atLeastOneRequestSent && atLeastOneRequestFailed);
    emberAfClearDynamicEndpoint(0);
}

/*
 * When chunked write is enabled, it is dangerious to handle multiple write requests at the same time. In this case, we will reject
 * the latter write requests to the same attribute.
 */
void TestWriteChunking::TestConflictWrite(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    registerAttributeAccessOverride(&testServer);

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::TestCluster::Id, kTestListAttribute);

    TestWriteCallback writeCallback1;
    app::WriteClient writeClient1(
        &ctx.GetExchangeManager(), &writeCallback1, Optional<uint16_t>::Missing(),
        static_cast<uint16_t>(900) /* use a smaller chunk so we only need a few attributes in the write request. */);

    TestWriteCallback writeCallback2;
    app::WriteClient writeClient2(
        &ctx.GetExchangeManager(), &writeCallback2, Optional<uint16_t>::Missing(),
        static_cast<uint16_t>(900) /* use a smaller chunk so we only need a few attributes in the write request. */);

    ByteSpan list[kTestListLength];

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = writeClient1.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = writeClient2.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = writeClient1.SendWriteRequest(sessionHandle);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = writeClient2.SendWriteRequest(sessionHandle);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    {
        const TestWriteCallback * writeCallbackRef1 = &writeCallback1;
        const TestWriteCallback * writeCallbackRef2 = &writeCallback2;

        // Exactly one of WriteClient1 and WriteClient2 should success, not both.

        if (writeCallback1.mSuccessCount == 0)
        {
            writeCallbackRef2 = &writeCallback1;
            writeCallbackRef1 = &writeCallback2;
        }

        NL_TEST_ASSERT(apSuite,
                       writeCallbackRef1->mSuccessCount ==
                           kTestListLength + 1 /* an extra item for the empty list at the beginning */);
        NL_TEST_ASSERT(apSuite, writeCallbackRef1->mErrorCount == 0);
        NL_TEST_ASSERT(apSuite, writeCallbackRef2->mSuccessCount == 0);
        NL_TEST_ASSERT(apSuite, writeCallbackRef2->mErrorCount == kTestListLength + 1);
        NL_TEST_ASSERT(apSuite, writeCallbackRef2->mLastErrorReason.mStatus == Protocols::InteractionModel::Status::Busy);

        NL_TEST_ASSERT(apSuite, writeCallbackRef1->mOnDoneCount == 1);
        NL_TEST_ASSERT(apSuite, writeCallbackRef2->mOnDoneCount == 1);
    }

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(0);
}

/*
 * When chunked write is enabled, it is dangerious to handle multiple write requests at the same time. However, we will allow such
 * change when writing to different attributes in parallel.
 */
void TestWriteChunking::TestNonConflictWrite(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    registerAttributeAccessOverride(&testServer);

    app::AttributePathParams attributePath1(kTestEndpointId, app::Clusters::TestCluster::Id, kTestListAttribute);
    app::AttributePathParams attributePath2(kTestEndpointId, app::Clusters::TestCluster::Id, kTestListAttribute2);

    TestWriteCallback writeCallback1;
    app::WriteClient writeClient1(
        &ctx.GetExchangeManager(), &writeCallback1, Optional<uint16_t>::Missing(),
        static_cast<uint16_t>(900) /* use a smaller chunk so we only need a few attributes in the write request. */);

    TestWriteCallback writeCallback2;
    app::WriteClient writeClient2(
        &ctx.GetExchangeManager(), &writeCallback2, Optional<uint16_t>::Missing(),
        static_cast<uint16_t>(900) /* use a smaller chunk so we only need a few attributes in the write request. */);

    ByteSpan list[kTestListLength];

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = writeClient1.EncodeAttribute(attributePath1, app::DataModel::List<ByteSpan>(list, kTestListLength));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    err = writeClient2.EncodeAttribute(attributePath2, app::DataModel::List<ByteSpan>(list, kTestListLength));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = writeClient1.SendWriteRequest(sessionHandle);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = writeClient2.SendWriteRequest(sessionHandle);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    {
        NL_TEST_ASSERT(apSuite, writeCallback1.mErrorCount == 0);
        NL_TEST_ASSERT(apSuite, writeCallback1.mSuccessCount == kTestListLength + 1);
        NL_TEST_ASSERT(apSuite, writeCallback2.mErrorCount == 0);
        NL_TEST_ASSERT(apSuite, writeCallback2.mSuccessCount == kTestListLength + 1);

        NL_TEST_ASSERT(apSuite, writeCallback1.mOnDoneCount == 1);
        NL_TEST_ASSERT(apSuite, writeCallback2.mOnDoneCount == 1);
    }

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(0);
}

namespace TestTransactionalListInstructions {

using PathStatus = std::pair<app::ConcreteAttributePath, bool>;

enum class Operations : uint8_t
{
    kNoop,
    kShutdownWriteClient,
};

enum class ListData : uint8_t
{
    kNull,
    kList,
    kBadValue,
};

struct Instructions
{
    // The paths used in write request
    std::vector<ConcreteAttributePath> paths;
    // The type of content of the list, it should be an empty vector or its size should equals to the list of paths.
    std::vector<ListData> data;
    // operations on OnListWriteBegin and OnListWriteEnd on the server side.
    std::function<Operations(const app::ConcreteAttributePath & path)> onListWriteBeginActions;
    // The expected status when OnListWriteEnd is called. In the same order as paths
    std::vector<bool> expectedStatus;
};

void RunTest(nlTestSuite * apSuite, TestContext & ctx, Instructions instructions)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    auto sessionHandle = ctx.GetSessionBobToAlice();

    TestWriteCallback writeCallback;
    std::unique_ptr<WriteClient> writeClient = std::make_unique<WriteClient>(
        &ctx.GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
        static_cast<uint16_t>(900) /* use a smaller chunk so we only need a few attributes in the write request. */);

    ConcreteAttributePath onGoingPath = ConcreteAttributePath();
    std::vector<PathStatus> status;

    testServer.mOnListWriteBegin = [&](const ConcreteAttributePath & aPath) {
        NL_TEST_ASSERT(apSuite, onGoingPath == ConcreteAttributePath());
        onGoingPath = aPath;
        ChipLogProgress(Zcl, "OnListWriteBegin endpoint=%u Cluster=" ChipLogFormatMEI " attribute=" ChipLogFormatMEI,
                        aPath.mEndpointId, ChipLogValueMEI(aPath.mClusterId), ChipLogValueMEI(aPath.mAttributeId));
        if (instructions.onListWriteBeginActions)
        {
            switch (instructions.onListWriteBeginActions(aPath))
            {
            case Operations::kNoop:
                break;
            case Operations::kShutdownWriteClient:
                // By setting writeClient to nullptr, we actually shutdown the write interaction to simulate a timeout.
                writeClient = nullptr;
            }
        }
    };
    testServer.mOnListWriteEnd = [&](const ConcreteAttributePath & aPath, bool aWasSuccessful) {
        NL_TEST_ASSERT(apSuite, onGoingPath == aPath);
        status.push_back(PathStatus(aPath, aWasSuccessful));
        onGoingPath = ConcreteAttributePath();
        ChipLogProgress(Zcl, "OnListWriteEnd endpoint=%u Cluster=" ChipLogFormatMEI " attribute=" ChipLogFormatMEI,
                        aPath.mEndpointId, ChipLogValueMEI(aPath.mClusterId), ChipLogValueMEI(aPath.mAttributeId));
    };

    ByteSpan list[kTestListLength];
    uint8_t badList[kTestListLength];

    if (instructions.data.size() == 0)
    {
        instructions.data = std::vector<ListData>(instructions.paths.size(), ListData::kList);
    }
    NL_TEST_ASSERT(apSuite, instructions.paths.size() == instructions.data.size());

    for (size_t i = 0; i < instructions.paths.size(); i++)
    {
        const auto & p = instructions.paths[i];
        switch (instructions.data[i])
        {
        case ListData::kNull: {
            DataModel::Nullable<uint8_t> null; // The actual type is not important since we will only put a null value.
            err = writeClient->EncodeAttribute(AttributePathParams(p.mEndpointId, p.mClusterId, p.mAttributeId), null);
            break;
        }
        case ListData::kList: {
            err = writeClient->EncodeAttribute(AttributePathParams(p.mEndpointId, p.mClusterId, p.mAttributeId),
                                               DataModel::List<ByteSpan>(list, kTestListLength));
            break;
        }
        case ListData::kBadValue: {
            err = writeClient->EncodeAttribute(AttributePathParams(p.mEndpointId, p.mClusterId, p.mAttributeId),
                                               DataModel::List<uint8_t>(badList, kTestListLength));
            break;
        }
        }
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    err = writeClient->SendWriteRequest(sessionHandle);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.GetIOContext().DriveIOUntil(sessionHandle->ComputeRoundTripTimeout(app::kExpectedIMProcessingTime) +
                                        System::Clock::Seconds16(1),
                                    [&]() { return ctx.GetExchangeManager().GetNumActiveExchanges() == 0; });

    NL_TEST_ASSERT(apSuite, onGoingPath == app::ConcreteAttributePath());
    NL_TEST_ASSERT(apSuite, status.size() == instructions.expectedStatus.size());

    for (size_t i = 0; i < status.size(); i++)
    {
        NL_TEST_ASSERT(apSuite, status[i] == PathStatus(instructions.paths[i], instructions.expectedStatus[i]));
    }

    testServer.mOnListWriteBegin = nullptr;
    testServer.mOnListWriteEnd   = nullptr;
}

} // namespace TestTransactionalListInstructions

void TestWriteChunking::TestTransactionalList(nlTestSuite * apSuite, void * apContext)
{
    using namespace TestTransactionalListInstructions;

    TestContext & ctx = *static_cast<TestContext *>(apContext);

    // Initialize the ember side server logic
    InitDataModelHandler(&ctx.GetExchangeManager());

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    registerAttributeAccessOverride(&testServer);

    // Test 1: we should receive transaction notifications
    ChipLogProgress(Zcl, "Test 1: we should receive transaction notifications");
    RunTest(apSuite, ctx,
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute) },
                .expectedStatus = { true },
            });

    ChipLogProgress(Zcl, "Test 2: we should receive transaction notifications for incomplete list operations");
    RunTest(
        apSuite, ctx,
        Instructions{
            .paths                   = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute) },
            .onListWriteBeginActions = [&](const app::ConcreteAttributePath & aPath) { return Operations::kShutdownWriteClient; },
            .expectedStatus          = { false },
        });

    ChipLogProgress(Zcl, "Test 3: we should receive transaction notifications for every list in the transaction");
    RunTest(apSuite, ctx,
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute),
                           ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute2) },
                .expectedStatus = { true, true },
            });

    ChipLogProgress(Zcl, "Test 4: we should receive transaction notifications with the status of each list");
    RunTest(apSuite, ctx,
            Instructions{
                .paths = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute),
                           ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute2) },
                .onListWriteBeginActions =
                    [&](const app::ConcreteAttributePath & aPath) {
                        if (aPath.mAttributeId == kTestListAttribute2)
                        {
                            return Operations::kShutdownWriteClient;
                        }
                        return Operations::kNoop;
                    },
                .expectedStatus = { true, false },
            });

    ChipLogProgress(Zcl,
                    "Test 5: transactional list callbacks will be called for nullable lists, test if it is handled correctly for "
                    "null value before non null values");
    RunTest(apSuite, ctx,
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute),
                           ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute) },
                .data           = { ListData::kNull, ListData::kList },
                .expectedStatus = { true },
            });

    ChipLogProgress(Zcl,
                    "Test 6: transactional list callbacks will be called for nullable lists, test if it is handled correctly for "
                    "null value after non null values");
    RunTest(apSuite, ctx,
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute),
                           ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute) },
                .data           = { ListData::kList, ListData::kNull },
                .expectedStatus = { true },
            });

    ChipLogProgress(Zcl,
                    "Test 7: transactional list callbacks will be called for nullable lists, test if it is handled correctly for "
                    "null value between non null values");
    RunTest(apSuite, ctx,
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute),
                           ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute),
                           ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute) },
                .data           = { ListData::kList, ListData::kNull, ListData::kList },
                .expectedStatus = { true },
            });

    ChipLogProgress(Zcl, "Test 8: transactional list callbacks will be called for nullable lists");
    RunTest(apSuite, ctx,
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute) },
                .data           = { ListData::kNull },
                .expectedStatus = { true },
            });

    ChipLogProgress(Zcl,
                    "Test 9: for nullable lists, we should receive notifications for unsuccessful writes when non-fatal occurred "
                    "during processing the requests");
    RunTest(apSuite, ctx,
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::TestCluster::Id, kTestListAttribute) },
                .data           = { ListData::kBadValue },
                .expectedStatus = { false },
            });

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    emberAfClearDynamicEndpoint(0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestListChunking", TestWriteChunking::TestListChunking),
    NL_TEST_DEF("TestBadChunking", TestWriteChunking::TestBadChunking),
    NL_TEST_DEF("TestConflictWrite", TestWriteChunking::TestConflictWrite),
    NL_TEST_DEF("TestNonConflictWrite", TestWriteChunking::TestNonConflictWrite),
    NL_TEST_DEF("TestTransactionalList", TestWriteChunking::TestTransactionalList),
    NL_TEST_SENTINEL()
};

// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestWriteChunking",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestWriteChunkingTests()
{
    gSuite = &sSuite;
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestWriteChunkingTests)
