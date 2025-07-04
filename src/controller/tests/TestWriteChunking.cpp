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
constexpr EndpointId kTestEndpointId      = 0;
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

    enum class EncodingMethod
    {
        Standard,      // Encoding using WriteClient::EncodeAttribute()
        PreencodedTLV, // Encoding using WriteClient::PutPreencodedAttribute()
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

    void RunTest(Instructions instructions, EncodingMethod encodingMethod);
    void RunTest_NonEmptyReplaceAll(Instructions instructions, EncodingMethod encodingMethod);

    template <class T>
    void EncodeAttributeListIntoTLV(const DataModel::List<T> & aListAttribute, TLV::ScopedBufferTLVReader & outTlvReader);
};

// Encodes an attribute of List Data Type into a TLV Reader object for testing WriteClient::PutPreencodedAttribute
// Warning: This method only encodes uint8_t or ByteSpans whose length fits in one octet
template <class T>
void TestWriteChunking::EncodeAttributeListIntoTLV(const DataModel::List<T> & aListAttribute,
                                                   TLV::ScopedBufferTLVReader & outEncodedListTlvReader)
{
    static_assert(std::is_same<T, chip::ByteSpan>::value || std::is_same<T, uint8_t>::value,
                  "This method only encodes uint8_t or ByteSpans whose length fits in one octet");

    size_t estimatedSize = 0;
    for (size_t i = 0; i < aListAttribute.size(); i++)
    {
        if constexpr (std::is_same<T, uint8_t>::value)
        {
            // Control Octet (1) + size of uint8_t
            estimatedSize += 1 + sizeof(uint8_t);
        }
        else if constexpr (std::is_same<T, chip::ByteSpan>::value)
        {
            ASSERT_LE(aListAttribute[i].size(), static_cast<size_t>(UINT8_MAX));

            // Control Octet (1) + Length Octet (1) + size of a single ByteSpan
            estimatedSize += 2 + aListAttribute[i].size();
        }
    }

    // Encode AttributeData into a TLV Array
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
    buffer.Alloc(TLV::EstimateStructOverhead(estimatedSize));

    TLV::TLVWriter writer;
    writer.Init(buffer.Get(), buffer.AllocatedSize());
    TLV::TLVType outerContainer;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerContainer));
    for (auto & item : aListAttribute)
    {
        EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), item));
    }
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(outerContainer));

    // Move Encoded TLV Array into TLVReader Object
    outEncodedListTlvReader.Init(std::move(buffer), writer.GetLengthWritten());
    outEncodedListTlvReader.Next();
}

//clang-format off

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint)
DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute, ARRAY, 1, MATTER_ATTRIBUTE_FLAG_WRITABLE),
    DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute2, ARRAY, 1, MATTER_ATTRIBUTE_FLAG_WRITABLE), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrsOnEndpoint, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

// Second Endpoint to Test Chunking when we send non-empty initial ReplaceAll List, which is used for ACL, see
// WriteClient::EncodeAttribute()

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint2)
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::AccessControl::Attributes::Acl::Id, ARRAY, 8, MATTER_ATTRIBUTE_FLAG_WRITABLE),
    DECLARE_DYNAMIC_ATTRIBUTE(Clusters::AccessControl::Attributes::Extension::Id, ARRAY, 1, MATTER_ATTRIBUTE_FLAG_WRITABLE),
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters2)
DECLARE_DYNAMIC_CLUSTER(Clusters::AccessControl::Id, testClusterAttrsOnEndpoint2, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpointAcl, testEndpointClusters2);

DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpointClusters)];
DataVersion dataVersionStorageAcl[MATTER_ARRAY_SIZE(testEndpointClusters2)];

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
        ChipLogError(Zcl, "NotList/ReplaceAll: Decode result: %s", err.AsString());
        return err;
    }
    if (aPath.mListOp == app::ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ByteSpan listItem;
        CHIP_ERROR err = aDecoder.Decode(listItem);
        ChipLogError(Zcl, "AppendItem: Decode result: %s", err.AsString());
        return err;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

class TestAttrAccessAcl : public app::AttributeAccessInterface
{
public:
    // Register for the Test Cluster cluster on all endpoints.
    TestAttrAccessAcl() : AttributeAccessInterface(Optional<EndpointId>::Missing(), AccessControl::Id) {}

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
} testServerAcl;

CHIP_ERROR TestAttrAccessAcl::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR TestAttrAccessAcl::Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder)
{
    // We only care about the number of attribute data.
    if (!aPath.IsListItemOperation())
    {
        app::DataModel::Nullable<app::DataModel::DecodableList<ByteSpan>> list;
        CHIP_ERROR err = aDecoder.Decode(list);
        ChipLogError(Zcl, "NotList/ReplaceAll: Decode result: %s", err.AsString());
        return err;
    }
    if (aPath.mListOp == app::ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ByteSpan listItem;
        CHIP_ERROR err = aDecoder.Decode(listItem);
        ChipLogError(Zcl, "AppendItem: Decode result: %s", err.AsString());
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

    for (EncodingMethod encodingMethod : { EncodingMethod::Standard, EncodingMethod::PreencodedTLV })
    {

        for (uint32_t i = 100; i > 0; i--)
        {
            CHIP_ERROR err = CHIP_NO_ERROR;
            TestWriteCallback writeCallback;

            ChipLogDetail(DataManagement, "Running iteration %d\n", static_cast<int>(i));

            gIterationCount = i;

            app::WriteClient writeClient(&GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
                                         static_cast<uint16_t>(minReservationSize + i) /* reserved buffer size */);

            ByteSpan list[kTestListLength];

            if (encodingMethod == EncodingMethod::Standard)
            {
                err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength));
                EXPECT_EQ(err, CHIP_NO_ERROR);
            }
            else if (encodingMethod == EncodingMethod::PreencodedTLV)
            {
                TLV::ScopedBufferTLVReader encodedListTLV;
                EncodeAttributeListIntoTLV(DataModel::List<ByteSpan>(list, kTestListLength), encodedListTLV);

                ConcreteDataAttributePath path =
                    ConcreteDataAttributePath(attributePath.mEndpointId, attributePath.mClusterId, attributePath.mAttributeId);
                EXPECT_EQ(writeClient.PutPreencodedAttribute(path, encodedListTLV), CHIP_NO_ERROR);
            }

            //

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
    }
    emberAfClearDynamicEndpoint(0);
}

/*
 * A Variant of TestListChunking above, that tests the Code Path where we encode a Non-Replace All List in WriteRequests, this
 * happens with the ACL Cluster (this would be generalised to all relevant Attributes after issue #38270 is resolved)
 */
TEST_F(TestWriteChunking, TestListChunking_NonEmptyReplaceAllList)
{
    auto sessionHandle = GetSessionBobToAlice();

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpointAcl, Span<DataVersion>(dataVersionStorageAcl));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServerAcl);

    app::AttributePathParams attributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id);

    // We've empirically determined that by reserving all but 65 bytes in the packet buffer, we can fit a Single AttributeDataIB
    // into the packet with a List of 8/9 ByteSpans with empty items. So if we have a ByteSpan List of 20 items, our initial
    // ReplaceAll list will contain 8/9 items, and the remaining items are chunked as appropriate.
    constexpr size_t maxReservationSize = kMaxSecureSduLengthBytes - 65;

    constexpr uint8_t kTestListLength2 = 20;

    // Start with a high reservation (maxReservationSize) to force chunking, then decrease the reservation in 1-byte steps.
    // This increases the buffer space available for encoding, gradually reducing the need for chunking, until chunking would not
    // occur anymore. This helps validate various edge cases.

    for (EncodingMethod encodingMethod : { EncodingMethod::Standard, EncodingMethod::PreencodedTLV })
    {
        for (uint32_t reservationReduction = 0; reservationReduction < 40; reservationReduction++)
        {
            CHIP_ERROR err = CHIP_NO_ERROR;
            TestWriteCallback writeCallback;

            ChipLogDetail(DataManagement, "Running iteration %d\n", static_cast<int>(reservationReduction));

            app::WriteClient writeClient(
                &GetExchangeManager(), &writeCallback, NullOptional,
                static_cast<uint16_t>(maxReservationSize - reservationReduction) /* reserved buffer size */);

            ByteSpan list[kTestListLength2];

            if (encodingMethod == EncodingMethod::Standard)
            {
                err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength2));
                EXPECT_EQ(err, CHIP_NO_ERROR);
            }
            else if (encodingMethod == EncodingMethod::PreencodedTLV)
            {
                TLV::ScopedBufferTLVReader encodedListTLV;
                EncodeAttributeListIntoTLV(DataModel::List<ByteSpan>(list, kTestListLength2), encodedListTLV);

                ConcreteDataAttributePath path =
                    ConcreteDataAttributePath(attributePath.mEndpointId, attributePath.mClusterId, attributePath.mAttributeId);
                EXPECT_EQ(writeClient.PutPreencodedAttribute(path, encodedListTLV), CHIP_NO_ERROR);
            }

            // Ensure that chunking actually occurred in the first iteration. We will iteratively chunk less and less, until
            // chunking would not occur anymore. Thus, this check is only needed at start.
            if (reservationReduction == 0)
            {
                ASSERT_TRUE(writeClient.IsWriteRequestChunked());
            }

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

            // Due to Write Chunking being done dynamically (fitting as many items as possible into an initial ReplaceAll List,
            // before starting to chunk), it is fragile to try to predict mSuccessCount. It all depends on how much was packed into
            // the initial ReplaceAll List. However, we know for sure that writeCallback should NEVER fail.
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
 * A Variant of TestBadChunking above, that tests the Code Path where we encode a Non-Replace All List in WriteRequests, this
 * happens with the ACL Cluster.
 */
TEST_F(TestWriteChunking, TestBadChunking_NonEmptyReplaceAllList)
{
    auto sessionHandle = GetSessionBobToAlice();

    bool atLeastOneRequestSent   = false;
    bool atLeastOneRequestFailed = false;

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpointAcl, Span<DataVersion>(dataVersionStorageAcl));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServerAcl);

    app::AttributePathParams attributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id);

    constexpr uint8_t kTestListLengthBadChunking = 5;

    for (int bufferSize = 850; bufferSize < static_cast<int>(chip::app::kMaxSecureSduLengthBytes); bufferSize++)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TestWriteCallback writeCallback;

        ChipLogDetail(DataManagement, "Running iteration with OCTET_STRING length = %d\n", bufferSize);

        app::WriteClient writeClient(&GetExchangeManager(), &writeCallback, NullOptional);

        ByteSpan list[kTestListLengthBadChunking];
        for (auto & item : list)
        {
            item = ByteSpan(sByteSpanData, static_cast<uint32_t>(bufferSize));
        }

        err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLengthBadChunking));
        if (err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            // This kind of error is expected.
            atLeastOneRequestFailed = true;
            continue;
        }

        atLeastOneRequestSent = true;

        // Ensure that chunking actually occurred. Since chunking is dynamic, it's easy to unintentionally avoid it.
        // This check guarantees that the test is validating chunked behavior as intended.
        EXPECT_TRUE(writeClient.IsWriteRequestChunked());

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

        // Due to the way Write Chunking is done, it is difficult to predict mSuccessCount. It all depends on how much was
        // packed into the initial ReplaceAll List.
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
 * When chunked write is enabled, it is dangerous to handle multiple write requests at the same time. In this case, we will
 * reject the latter write requests to the same attribute.
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
 * A Variant of TestConflictWrite above, that tests the Code Path where we encode a Non-Replace All List in WriteRequests, this
 * happens with the ACL Cluster.
 */
TEST_F(TestWriteChunking, TestConflictWrite_NonEmptyReplaceAllList)
{
    auto sessionHandle = GetSessionBobToAlice();

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpointAcl, Span<DataVersion>(dataVersionStorageAcl));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServerAcl);

    app::AttributePathParams attributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id);

    // To ensure that chunking is triggered: We've empirically determined that by reserving all but 60 bytes in the packet
    // buffer, we can fit 1 AttributeDataIB into the packet with a List of 5/6 ByteSpans with empty items. So if we have a
    // ByteSpan List of 10 items, our initial ReplaceAll list will contain 5/6 items, and the remaining items are chunked.
    constexpr size_t kReserveSize      = kMaxSecureSduLengthBytes - 60;
    constexpr uint8_t kTestListLength2 = 10;

    ByteSpan list[kTestListLength2];

    TestWriteCallback writeCallback1;
    app::WriteClient writeClient1(&GetExchangeManager(), &writeCallback1, NullOptional, static_cast<uint16_t>(kReserveSize));

    TestWriteCallback writeCallback2;
    app::WriteClient writeClient2(&GetExchangeManager(), &writeCallback2, NullOptional, static_cast<uint16_t>(kReserveSize));

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = writeClient1.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength2));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = writeClient2.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, kTestListLength2));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Ensure that chunking actually occurred. Since chunking is dynamic, it's easy to unintentionally avoid it.
    // This check guarantees that the test is validating chunked behavior as intended.
    EXPECT_TRUE(writeClient1.IsWriteRequestChunked());
    EXPECT_TRUE(writeClient2.IsWriteRequestChunked());

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

        // Due to Write Chunking being done dynamically (fitting as many items as possible into an initial ReplaceAll List,
        // before starting to chunk), it is fragile to try to predict mSuccessCount. It all depends on how much was packed into
        // the initial ReplaceAll List. However, we know for sure that writeCallbackRef1 should NEVER fail, and that
        // writeCallbackRef2 should NEVER Succeed.
        EXPECT_EQ(writeCallbackRef1->mErrorCount, 0u);
        EXPECT_EQ(writeCallbackRef2->mSuccessCount, 0u);

        EXPECT_EQ(writeCallbackRef2->mLastErrorReason.mStatus, Protocols::InteractionModel::Status::Busy);

        EXPECT_EQ(writeCallbackRef1->mOnDoneCount, 1u);
        EXPECT_EQ(writeCallbackRef2->mOnDoneCount, 1u);
    }

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

/*
 * When chunked write is enabled, it is dangerous to handle multiple write requests at the same time. However, we will allow
 * such change when writing to different attributes in parallel.
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

/*
 * A Variant of TestNonConflictWrite above, that tests the Code Path where we encode a Non-Replace All List in WriteRequests,
 * this happens with the ACL Cluster.
 */
TEST_F(TestWriteChunking, TestNonConflictWrite_NonEmptyReplaceAllList)
{
    auto sessionHandle = GetSessionBobToAlice();

    // Initialize the ember side server logic
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpointAcl, Span<DataVersion>(dataVersionStorageAcl));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServerAcl);

    app::AttributePathParams attributePath1(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id);
    app::AttributePathParams attributePath2(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Extension::Id);

    // To ensure that chunking is triggered: We've empirically determined that by reserving all but 60 bytes in the packet
    // buffer, we can fit 1 AttributeDataIB into the packet with a List of 5/6 ByteSpans with empty items. So if we have a
    // ByteSpan List of 10 items, our initial ReplaceAll list will contain 5/6 items, and the remaining items are chunked.
    constexpr size_t kReserveSize      = kMaxSecureSduLengthBytes - 65;
    constexpr uint8_t kTestListLength2 = 10;

    TestWriteCallback writeCallback1;
    app::WriteClient writeClient1(&GetExchangeManager(), &writeCallback1, NullOptional, static_cast<uint16_t>(kReserveSize));

    TestWriteCallback writeCallback2;
    app::WriteClient writeClient2(&GetExchangeManager(), &writeCallback2, NullOptional, static_cast<uint16_t>(kReserveSize));

    ByteSpan list[kTestListLength2];

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = writeClient1.EncodeAttribute(attributePath1, app::DataModel::List<ByteSpan>(list, kTestListLength2));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writeClient2.EncodeAttribute(attributePath2, app::DataModel::List<ByteSpan>(list, kTestListLength2));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Ensure that chunking actually occurred. Since chunking is dynamic, it's easy to unintentionally avoid it.
    // This check guarantees that the test is validating chunked behavior as intended.
    EXPECT_TRUE(writeClient1.IsWriteRequestChunked());
    EXPECT_TRUE(writeClient2.IsWriteRequestChunked());

    err = writeClient1.SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writeClient2.SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    {

        // Due to Write Chunking being done dynamically, it is fragile to try to predict mSuccessCount. It all depends on how
        // much was packed into the initial ReplaceAll List. However, we know for sure that writeCallback1 and writeCallback2
        // should NEVER fail.
        EXPECT_EQ(writeCallback1.mErrorCount, 0u);
        EXPECT_EQ(writeCallback2.mErrorCount, 0u);

        EXPECT_EQ(writeCallback1.mOnDoneCount, 1u);
        EXPECT_EQ(writeCallback2.mOnDoneCount, 1u);
    }

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}
// for (EncodingMethod encodingMethod : { EncodingMethod::Standard, EncodingMethod::PreencodedTLV })

void TestWriteChunking::RunTest(Instructions instructions, EncodingMethod encodingMethod = EncodingMethod::Standard)
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

            if (encodingMethod == EncodingMethod::Standard)
            {
                err = writeClient->EncodeAttribute(AttributePathParams(p.mEndpointId, p.mClusterId, p.mAttributeId),
                                                   DataModel::List<ByteSpan>(list, kTestListLength));
            }
            else if (encodingMethod == EncodingMethod::PreencodedTLV)
            {
                TLV::ScopedBufferTLVReader encodedListTLV;
                EncodeAttributeListIntoTLV(DataModel::List<ByteSpan>(list, kTestListLength), encodedListTLV);

                ConcreteDataAttributePath path = ConcreteDataAttributePath(p.mEndpointId, p.mClusterId, p.mAttributeId);
                err                            = writeClient->PutPreencodedAttribute(path, encodedListTLV);
            }
            break;
        }
        case ListData::kBadValue: {

            if (encodingMethod == EncodingMethod::Standard)
            {
                err = writeClient->EncodeAttribute(AttributePathParams(p.mEndpointId, p.mClusterId, p.mAttributeId),
                                                   DataModel::List<uint8_t>(badList, kTestListLength));
            }
            else if (encodingMethod == EncodingMethod::PreencodedTLV)
            {
                TLV::ScopedBufferTLVReader encodedListTLV;
                EncodeAttributeListIntoTLV(DataModel::List<uint8_t>(badList, kTestListLength), encodedListTLV);

                ConcreteDataAttributePath path = ConcreteDataAttributePath(p.mEndpointId, p.mClusterId, p.mAttributeId);
                err                            = writeClient->PutPreencodedAttribute(path, encodedListTLV);
            }

            break;
        }
        }
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writeClient->SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    GetIOContext().DriveIOUntil(
        sessionHandle->ComputeRoundTripTimeout(app::kExpectedIMProcessingTime, true /*isFirstMessageOnExchange*/) +
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

    for (EncodingMethod encodingMethod : { EncodingMethod::Standard, EncodingMethod::PreencodedTLV })
    {
        // For builds without ChipLogProgress, encodingMethodName will be ununsed and trigger build failures
        [[maybe_unused]] const char * encodingMethodName =
            (encodingMethod == EncodingMethod::Standard ? "StandardEncoding" : "PreencodedTLV");

        // Test 1: we should receive transaction notifications
        ChipLogProgress(Zcl, "Test 1 [%s]: we should receive transaction notifications", encodingMethodName);
        RunTest(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 2a [%s]: we should receive transaction notifications for incomplete list operations",
                        encodingMethodName);
        RunTest(
            Instructions{
                .paths = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
                .onListWriteBeginActions =
                    [&](const app::ConcreteAttributePath & aPath) { return Operations::kShutdownWriteClient; },
                .expectedStatus = { false },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 3 [%s]: we should receive transaction notifications for every list in the transaction",
                        encodingMethodName);
        RunTest(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                                    ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute2) },
                .expectedStatus = { true, true },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 4 [%s]: we should receive transaction notifications with the status of each list",
                        encodingMethodName);
        RunTest(
            Instructions{
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
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 5 [%s]: transactional list callbacks will be called for nullable lists, test if it is handled "
                        "correctly for null value before non null values",
                        encodingMethodName);
        RunTest(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                                    ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
                .data           = { ListData::kNull, ListData::kList },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 6 [%s]: transactional list callbacks will be called for nullable lists, test if it is handled "
                        "correctly for null value after non null values",
                        encodingMethodName);
        RunTest(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                                    ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
                .data           = { ListData::kList, ListData::kNull },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 7 [%s]: transactional list callbacks will be called for nullable lists, test if it is handled "
                        "correctly for null value between non null values",
                        encodingMethodName);
        RunTest(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                                    ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute),
                                    ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
                .data           = { ListData::kList, ListData::kNull, ListData::kList },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 8 [%s]: transactional list callbacks will be called for nullable lists", encodingMethodName);
        RunTest(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
                .data           = { ListData::kNull },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 9 [%s]: for nullable lists, we should receive notifications for unsuccessful writes when non-fatal "
                        "occurred during processing the requests",
                        encodingMethodName);
        RunTest(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, Clusters::UnitTesting::Id, kTestListAttribute) },
                .data           = { ListData::kBadValue },
                .expectedStatus = { false },
            },
            encodingMethod);
    }
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

/*
 * A Variant of RunTest above, that tests the Code Path where we encode a Non-Replace All List in WriteRequests, this
 * happens with the ACL Cluster.
 */
void TestWriteChunking::RunTest_NonEmptyReplaceAll(Instructions instructions,
                                                   EncodingMethod encodingMethod = EncodingMethod::Standard)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    auto sessionHandle = GetSessionBobToAlice();

    TestWriteCallback writeCallback;
    std::unique_ptr<WriteClient> writeClient = std::make_unique<WriteClient>(
        &GetExchangeManager(), &writeCallback, NullOptional,
        static_cast<uint16_t>(kMaxSecureSduLengthBytes -
                              66) /* use a smaller chunk so we only need a few attributes in the write request. */);

    ConcreteAttributePath onGoingPath = ConcreteAttributePath();
    std::vector<PathStatus> status;

    testServerAcl.mOnListWriteBegin = [&](const ConcreteAttributePath & aPath) {
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
    testServerAcl.mOnListWriteEnd = [&](const ConcreteAttributePath & aPath, bool aWasSuccessful) {
        EXPECT_EQ(onGoingPath, aPath);
        status.push_back(PathStatus(aPath, aWasSuccessful));
        onGoingPath = ConcreteAttributePath();
        ChipLogProgress(Zcl, "OnListWriteEnd endpoint=%u Cluster=" ChipLogFormatMEI " attribute=" ChipLogFormatMEI,
                        aPath.mEndpointId, ChipLogValueMEI(aPath.mClusterId), ChipLogValueMEI(aPath.mAttributeId));
    };

    constexpr uint8_t kTestListLength2 = 10;

    ByteSpan list[kTestListLength2];
    uint8_t badList[kTestListLength2];

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

            if (encodingMethod == EncodingMethod::Standard)
            {
                err = writeClient->EncodeAttribute(AttributePathParams(p.mEndpointId, p.mClusterId, p.mAttributeId),
                                                   DataModel::List<ByteSpan>(list, kTestListLength2));
            }
            else if (encodingMethod == EncodingMethod::PreencodedTLV)
            {
                TLV::ScopedBufferTLVReader encodedListTLV;
                EncodeAttributeListIntoTLV(DataModel::List<ByteSpan>(list, kTestListLength2), encodedListTLV);

                ConcreteDataAttributePath path = ConcreteDataAttributePath(p.mEndpointId, p.mClusterId, p.mAttributeId);
                err                            = writeClient->PutPreencodedAttribute(path, encodedListTLV);
            }
            break;
        }
        case ListData::kBadValue: {

            if (encodingMethod == EncodingMethod::Standard)
            {
                err = writeClient->EncodeAttribute(AttributePathParams(p.mEndpointId, p.mClusterId, p.mAttributeId),
                                                   DataModel::List<uint8_t>(badList, kTestListLength2));
            }
            else if (encodingMethod == EncodingMethod::PreencodedTLV)
            {
                TLV::ScopedBufferTLVReader encodedListTLV;
                EncodeAttributeListIntoTLV(DataModel::List<uint8_t>(badList, kTestListLength2), encodedListTLV);

                ConcreteDataAttributePath path = ConcreteDataAttributePath(p.mEndpointId, p.mClusterId, p.mAttributeId);
                err                            = writeClient->PutPreencodedAttribute(path, encodedListTLV);
            }

            break;
        }
        }
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writeClient->SendWriteRequest(sessionHandle);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    GetIOContext().DriveIOUntil(
        sessionHandle->ComputeRoundTripTimeout(app::kExpectedIMProcessingTime, true /*isFirstMessageOnExchange*/) +
            System::Clock::Seconds16(1),
        [&]() { return GetExchangeManager().GetNumActiveExchanges() == 0; });

    EXPECT_EQ(onGoingPath, app::ConcreteAttributePath());
    EXPECT_EQ(status.size(), instructions.expectedStatus.size());

    for (size_t i = 0; i < status.size(); i++)
    {
        EXPECT_EQ(status[i], PathStatus(instructions.paths[i], instructions.expectedStatus[i]));
    }

    testServerAcl.mOnListWriteBegin = nullptr;
    testServerAcl.mOnListWriteEnd   = nullptr;
}

TEST_F(TestWriteChunking, TestTransactionalList_NonEmptyReplaceAllList)
{
    // Initialize the ember side server logic.
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpointAcl, Span<DataVersion>(dataVersionStorageAcl));

    // Register our fake attribute access interface.
    AttributeAccessInterfaceRegistry::Instance().Register(&testServerAcl);

    for (EncodingMethod encodingMethod : { EncodingMethod::Standard, EncodingMethod::PreencodedTLV })
    {
        // For builds without ChipLogProgress, encodingMethodName will be ununsed and trigger build failures
        [[maybe_unused]] const char * encodingMethodName =
            (encodingMethod == EncodingMethod::Standard ? "StandardEncoding" : "PreencodedTLV");

        // Test 1: we should receive transaction notifications
        ChipLogProgress(Zcl, "Test 1 [%s]: we should receive transaction notifications", encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id) },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 2 [%s]: we should receive transaction notifications for incomplete list operations",
                        encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id) },
                .onListWriteBeginActions =
                    [&](const app::ConcreteAttributePath & aPath) { return Operations::kShutdownWriteClient; },
                .expectedStatus = { false },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 3 [%s]: we should receive transaction notifications for every list in the transaction",
                        encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id),
                                    ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Extension::Id) },
                .expectedStatus = { true, true },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 4 [%s]: we should receive transaction notifications with the status of each list",
                        encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id),
                           ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Extension::Id) },
                .onListWriteBeginActions =
                    [&](const app::ConcreteAttributePath & aPath) {
                        if (aPath.mAttributeId == AccessControl::Attributes::Extension::Id)
                        {
                            return Operations::kShutdownWriteClient;
                        }
                        return Operations::kNoop;
                    },
                .expectedStatus = { true, false },
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 5 [%s]: transactional list callbacks will be called for nullable lists, test if it is handled "
                        "correctly for null value before non null values",
                        encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id),
                                    ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id) },
                .data           = { ListData::kNull, ListData::kList },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 6 [%s]: transactional list callbacks will be called for nullable lists, test if it is handled "
                        "correctly for null value after non null values",
                        encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id),
                                    ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id) },
                .data           = { ListData::kList, ListData::kNull },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 7 [%s]: transactional list callbacks will be called for nullable lists, test if it is handled "
                        "correctly for null value between non null values",
                        encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id),
                                    ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id),
                                    ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id) },
                .data           = { ListData::kList, ListData::kNull, ListData::kList },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl, "Test 8 [%s]: transactional list callbacks will be called for nullable lists", encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id) },
                .data           = { ListData::kNull },
                .expectedStatus = { true },
            },
            encodingMethod);

        ChipLogProgress(Zcl,
                        "Test 9 [%s]: for nullable lists, we should receive notifications for unsuccessful writes when non-fatal "
                        "occurred during processing the requests",
                        encodingMethodName);
        RunTest_NonEmptyReplaceAll(
            Instructions{
                .paths          = { ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id) },
                .data           = { ListData::kBadValue },
                .expectedStatus = { false },
            },
            encodingMethod);

        // This TestCase tests corner cases when we Encode many attributes into the same WriteRequest, up to 10 Attributes will be
        // Encoded.

        for (int nullableListCount = 1; nullableListCount <= 10; nullableListCount++)
        {
            ChipLogProgress(Zcl, "Test 10.%d [%s]: Encoding %d nullable list(s) following a single non-nullable list",
                            nullableListCount, encodingMethodName, nullableListCount);

            Instructions test;

            // Add the single non-nullable list
            test.paths.push_back(ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id));
            test.data.push_back(ListData::kList);

            // Add the nullable lists
            for (int i = 0; i < nullableListCount; i++)
            {
                test.paths.push_back(ConcreteAttributePath(kTestEndpointId, AccessControl::Id, AccessControl::Attributes::Acl::Id));
                test.data.push_back(ListData::kNull);
            }

            test.expectedStatus = { true };
            RunTest_NonEmptyReplaceAll(test, encodingMethod);
        }
    }
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

} // namespace
