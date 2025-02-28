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

#include <memory>
#include <utility>

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/attribute-storage.h>
#include <controller/InvokeInteraction.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

uint32_t gIterationCount = 0;

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

class TestWriteChunking : public Test::AppContext
{
private:
    using PathStatus = std::pair<app::ConcreteAttributePath, bool>;

protected:
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

    void RunTest(Instructions instructions);
};

//clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint)
DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute, ARRAY, 1, MATTER_ATTRIBUTE_FLAG_WRITABLE),
    DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute2, ARRAY, 1, MATTER_ATTRIBUTE_FLAG_WRITABLE), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrsOnEndpoint, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpointClusters)];

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
    TestAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::UnitTesting::Id) {}

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
TEST_F(TestWriteChunking, TestListChunking)
{
    auto sessionHandle = GetSessionBobToAlice();

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServer);

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::UnitTesting::Id, kTestListAttribute);
    //
    // We've empirically determined that by reserving all but 75 bytes in the packet buffer, we can fit 2
    // AttributeDataIBs into the packet. ~30-40 bytes covers a single write chunk, but let's 2-3x that
    // to ensure we'll sweep from fitting 2 chunks to 3-4 chunks.
    //
    constexpr size_t minReservationSize = kMaxSecureSduLengthBytes - 75 - 100;
    for (uint32_t i = 100; i > 0; i--)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TestWriteCallback writeCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", static_cast<int>(i));

        gIterationCount = i;

        app::WriteClient writeClient(&GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
                                     static_cast<uint16_t>(minReservationSize + i) /* reserved buffer size */);

        ByteSpan list[kTestListLength];

        err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writeClient.SendWriteRequest(sessionHandle);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        //
        // Service the IO + Engine till we get a ReportEnd callback on the client.
        // Since bugs can happen, we don't want this test to never stop, so create a ceiling for how many
        // times this can run without seeing expected results.
        //
        for (int j = 0; j < 10 && writeCallback.mOnDoneCount == 0; j++)
        {
            DrainAndServiceIO();
        }

        EXPECT_EQ(writeCallback.mSuccessCount, kTestListLength + 1 /* an extra item for the empty list at the beginning */);
        EXPECT_EQ(writeCallback.mErrorCount, 0u);
        EXPECT_EQ(writeCallback.mOnDoneCount, 1u);

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

// We encode a pretty large write payload to test the corner cases related to message layer and secure session overheads.
// The test should gurantee that if encode returns no error, the send should also success.
// As the actual overhead may change, we will test over a few possible payload lengths, from 850 to MTU used in write clients.
TEST_F(TestWriteChunking, TestBadChunking)
{
    auto sessionHandle = GetSessionBobToAlice();

    bool atLeastOneRequestSent   = false;
    bool atLeastOneRequestFailed = false;

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServer);

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::UnitTesting::Id, kTestListAttribute);

    for (int i = 850; i < static_cast<int>(chip::app::kMaxSecureSduLengthBytes); i++)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TestWriteCallback writeCallback;

        ChipLogDetail(DataManagement, "Running iteration with OCTET_STRING length = %d\n", i);

        gIterationCount = (uint32_t) i;

        app::WriteClient writeClient(&GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing());

        ByteSpan list[kTestListLength];
        for (auto & item : list)
        {
            item = ByteSpan(sByteSpanData, static_cast<uint32_t>(i));
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
        EXPECT_EQ(err, CHIP_NO_ERROR);

        //
        // Service the IO + Engine till we get a ReportEnd callback on the client.
        // Since bugs can happen, we don't want this test to never stop, so create a ceiling for how many
        // times this can run without seeing expected results.
        //
        for (int j = 0; j < 10 && writeCallback.mOnDoneCount == 0; j++)
        {
            DrainAndServiceIO();
        }

        EXPECT_EQ(writeCallback.mSuccessCount, kTestListLength + 1 /* an extra item for the empty list at the beginning */);
        EXPECT_EQ(writeCallback.mErrorCount, 0u);
        EXPECT_EQ(writeCallback.mOnDoneCount, 1u);

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (HasFailure())
        {
            break;
        }
    }
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    EXPECT_TRUE(atLeastOneRequestSent && atLeastOneRequestFailed);
    emberAfClearDynamicEndpoint(0);
}

/*
 * When chunked write is enabled, it is dangerious to handle multiple write requests at the same time. In this case, we will reject
 * the latter write requests to the same attribute.
 */
TEST_F(TestWriteChunking, TestConflictWrite)
{
    auto sessionHandle = GetSessionBobToAlice();

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServer);

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::UnitTesting::Id, kTestListAttribute);

    /* use a smaller chunk (128 bytes) so we only need a few attributes in the write request. */
    constexpr size_t kReserveSize = kMaxSecureSduLengthBytes - 128;

    TestWriteCallback writeCallback1;
    app::WriteClient writeClient1(&GetExchangeManager(), &writeCallback1, Optional<uint16_t>::Missing(),
                                  static_cast<uint16_t>(kReserveSize));

    TestWriteCallback writeCallback2;
    app::WriteClient writeClient2(&GetExchangeManager(), &writeCallback2, Optional<uint16_t>::Missing(),
                                  static_cast<uint16_t>(kReserveSize));

    ByteSpan list[kTestListLength];

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = writeClient1.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = writeClient2.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writeClient1.SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writeClient2.SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    {
        const TestWriteCallback * writeCallbackRef1 = &writeCallback1;
        const TestWriteCallback * writeCallbackRef2 = &writeCallback2;

        // Exactly one of WriteClient1 and WriteClient2 should success, not both.

        if (writeCallback1.mSuccessCount == 0)
        {
            writeCallbackRef2 = &writeCallback1;
            writeCallbackRef1 = &writeCallback2;
        }

        EXPECT_EQ(writeCallbackRef1->mSuccessCount, kTestListLength + 1 /* an extra item for the empty list at the beginning */);
        EXPECT_EQ(writeCallbackRef1->mErrorCount, 0u);
        EXPECT_EQ(writeCallbackRef2->mSuccessCount, 0u);
        EXPECT_EQ(writeCallbackRef2->mErrorCount, kTestListLength + 1);
        EXPECT_EQ(writeCallbackRef2->mLastErrorReason.mStatus, Protocols::InteractionModel::Status::Busy);

        EXPECT_EQ(writeCallbackRef1->mOnDoneCount, 1u);
        EXPECT_EQ(writeCallbackRef2->mOnDoneCount, 1u);
    }

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

/*
 * When chunked write is enabled, it is dangerious to handle multiple write requests at the same time. However, we will allow such
 * change when writing to different attributes in parallel.
 */
TEST_F(TestWriteChunking, TestNonConflictWrite)
{
    auto sessionHandle = GetSessionBobToAlice();

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServer);

    app::AttributePathParams attributePath1(kTestEndpointId, app::Clusters::UnitTesting::Id, kTestListAttribute);
    app::AttributePathParams attributePath2(kTestEndpointId, app::Clusters::UnitTesting::Id, kTestListAttribute2);

    /* use a smaller chunk (128 bytes) so we only need a few attributes in the write request. */
    constexpr size_t kReserveSize = kMaxSecureSduLengthBytes - 128;

    TestWriteCallback writeCallback1;
    app::WriteClient writeClient1(&GetExchangeManager(), &writeCallback1, Optional<uint16_t>::Missing(),
                                  static_cast<uint16_t>(kReserveSize));

    TestWriteCallback writeCallback2;
    app::WriteClient writeClient2(&GetExchangeManager(), &writeCallback2, Optional<uint16_t>::Missing(),
                                  static_cast<uint16_t>(kReserveSize));

    ByteSpan list[kTestListLength];

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = writeClient1.EncodeAttribute(attributePath1, app::DataModel::List<ByteSpan>(list, kTestListLength));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = writeClient2.EncodeAttribute(attributePath2, app::DataModel::List<ByteSpan>(list, kTestListLength));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writeClient1.SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writeClient2.SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    {
        EXPECT_EQ(writeCallback1.mErrorCount, 0u);
        EXPECT_EQ(writeCallback1.mSuccessCount, kTestListLength + 1);
        EXPECT_EQ(writeCallback2.mErrorCount, 0u);
        EXPECT_EQ(writeCallback2.mSuccessCount, kTestListLength + 1);

        EXPECT_EQ(writeCallback1.mOnDoneCount, 1u);
        EXPECT_EQ(writeCallback2.mOnDoneCount, 1u);
    }

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

void TestWriteChunking::RunTest(Instructions instructions)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    auto sessionHandle = GetSessionBobToAlice();

    TestWriteCallback writeCallback;
    std::unique_ptr<WriteClient> writeClient = std::make_unique<WriteClient>(
        &GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
        static_cast<uint16_t>(kMaxSecureSduLengthBytes -
                              128) /* use a smaller chunk so we only need a few attributes in the write request. */);

    ConcreteAttributePath onGoingPath = ConcreteAttributePath();
    std::vector<PathStatus> status;

    testServer.mOnListWriteBegin = [&](const ConcreteAttributePath & aPath) {
        EXPECT_EQ(onGoingPath, ConcreteAttributePath());
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
        EXPECT_EQ(onGoingPath, aPath);
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
    EXPECT_EQ(instructions.paths.size(), instructions.data.size());

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
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writeClient->SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    GetIOContext().DriveIOUntil(sessionHandle->ComputeRoundTripTimeout(app::kExpectedIMProcessingTime) +
                                    System::Clock::Seconds16(1),
                                [&]() { return GetExchangeManager().GetNumActiveExchanges() == 0; });

    EXPECT_EQ(onGoingPath, app::ConcreteAttributePath());
    EXPECT_EQ(status.size(), instructions.expectedStatus.size());

    for (size_t i = 0; i < status.size(); i++)
    {
        EXPECT_EQ(status[i], PathStatus(instructions.paths[i], instructions.expectedStatus[i]));
    }

    testServer.mOnListWriteBegin = nullptr;
    testServer.mOnListWriteEnd   = nullptr;
}

TEST_F(TestWriteChunking, TestTransactionalList)
{
    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServer);

    // Test 1: we should receive transaction notifications
    ChipLogProgress(Zcl, "Test 1: we should receive transaction notifications");
    RunTest(Instructions{
        .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
        .expectedStatus = { true },
    });

    ChipLogProgress(Zcl, "Test 2: we should receive transaction notifications for incomplete list operations");
    RunTest(Instructions{
        .paths                   = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
        .onListWriteBeginActions = [&](const app::ConcreteAttributePath & aPath) { return Operations::kShutdownWriteClient; },
        .expectedStatus          = { false },
    });

    ChipLogProgress(Zcl, "Test 3: we should receive transaction notifications for every list in the transaction");
    RunTest(Instructions{
        .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                            ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute2) },
        .expectedStatus = { true, true },
    });

    ChipLogProgress(Zcl, "Test 4: we should receive transaction notifications with the status of each list");
    RunTest(Instructions{
        .paths = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                   ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute2) },
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
    RunTest(Instructions{
        .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                            ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
        .data           = { ListData::kNull, ListData::kList },
        .expectedStatus = { true },
    });

    ChipLogProgress(Zcl,
                    "Test 6: transactional list callbacks will be called for nullable lists, test if it is handled correctly for "
                    "null value after non null values");
    RunTest(Instructions{
        .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                            ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
        .data           = { ListData::kList, ListData::kNull },
        .expectedStatus = { true },
    });

    ChipLogProgress(Zcl,
                    "Test 7: transactional list callbacks will be called for nullable lists, test if it is handled correctly for "
                    "null value between non null values");
    RunTest(Instructions{
        .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                            ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                            ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
        .data           = { ListData::kList, ListData::kNull, ListData::kList },
        .expectedStatus = { true },
    });

    ChipLogProgress(Zcl, "Test 8: transactional list callbacks will be called for nullable lists");
    RunTest(Instructions{
        .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
        .data           = { ListData::kNull },
        .expectedStatus = { true },
    });

    ChipLogProgress(Zcl,
                    "Test 9: for nullable lists, we should receive notifications for unsuccessful writes when non-fatal occurred "
                    "during processing the requests");
    RunTest(Instructions{
        .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
        .data           = { ListData::kBadValue },
        .expectedStatus = { false },
    });

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

} // namespace
