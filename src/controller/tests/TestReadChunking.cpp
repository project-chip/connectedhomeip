/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <functional>
#include <map>
#include <utility>

#include <pw_unit_test/framework.h>

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app/ConcreteAttributePath.h"
#include "data-model-providers/codegen/Instance.h"
#include "protocols/interaction_model/Constants.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppConfig.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/BufferedReadCallback.h>
#include <app/CommandHandlerInterface.h>
#include <app/GlobalAttributes.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/UnitTestUtils.h>
#include <lib/support/logging/CHIPLogging.h>

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
constexpr EndpointId kTestEndpointId = 2;
// Another endpoint, with a list attribute only.
constexpr EndpointId kTestEndpointId3 = 3;
// Another endpoint, for adding / enabling during running.
constexpr EndpointId kTestEndpointId4    = 4;
constexpr EndpointId kTestEndpointId5    = 5;
constexpr AttributeId kTestListAttribute = 6;
constexpr AttributeId kTestBadAttribute =
    7; // Reading this attribute will return CHIP_ERROR_NO_MEMORY but nothing is actually encoded.

constexpr int kListAttributeItems = 5;

//clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(0x00000001, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000002, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000003, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000004, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000005, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpointClusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint, testEndpointClusters);

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint3)
DECLARE_DYNAMIC_ATTRIBUTE(kTestListAttribute, ARRAY, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(kTestBadAttribute, ARRAY, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpoint3Clusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrsOnEndpoint3, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint3, testEndpoint3Clusters);

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint4)
DECLARE_DYNAMIC_ATTRIBUTE(0x00000001, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpoint4Clusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrsOnEndpoint4, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint4, testEndpoint4Clusters);

// Unlike endpoint 1, we can modify the values for values in endpoint 5
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(testClusterAttrsOnEndpoint5)
DECLARE_DYNAMIC_ATTRIBUTE(0x00000001, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE(0x00000002, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(0x00000003, INT8U, 1, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(testEndpoint5Clusters)
DECLARE_DYNAMIC_CLUSTER(Clusters::UnitTesting::Id, testClusterAttrsOnEndpoint5, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(testEndpoint5, testEndpoint5Clusters);

//clang-format on

uint8_t sAnStringThatCanNeverFitIntoTheMTU[4096] = { 0 };

// Buffered callback class that lets us count the number of attribute data IBs
// we receive.  BufferedReadCallback has all its ReadClient::Callback bits
// private, so we can't just inherit from it and call our super-class functions.
class TestBufferedReadCallback : public ReadClient::Callback
{
public:
    TestBufferedReadCallback(ReadClient::Callback & aNextCallback) : mBufferedCallback(aNextCallback) {}

    // Workaround for all the methods on BufferedReadCallback being private.
    ReadClient::Callback & NextCallback() { return *static_cast<ReadClient::Callback *>(&mBufferedCallback); }

    void OnReportBegin() override { NextCallback().OnReportBegin(); }
    void OnReportEnd() override { NextCallback().OnReportEnd(); }

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override
    {
        if (apData)
        {
            ++mAttributeDataIBCount;

            TLV::TLVReader reader(*apData);
            do
            {
                if (reader.GetType() != TLV::TLVType::kTLVType_Array)
                {
                    // Not a list.
                    break;
                }

                TLV::TLVType containerType;
                CHIP_ERROR err = reader.EnterContainer(containerType);
                if (err != CHIP_NO_ERROR)
                {
                    mDecodingFailed = true;
                    break;
                }

                err = reader.Next();
                if (err == CHIP_END_OF_TLV)
                {
                    mSawEmptyList = true;
                }
                else if (err != CHIP_NO_ERROR)
                {
                    mDecodingFailed = true;
                    break;
                }
            } while (false);
        }
        else
        {
            ++mAttributeStatusIBCount;
        }

        NextCallback().OnAttributeData(aPath, apData, aStatus);
    }

    void OnError(CHIP_ERROR aError) override { NextCallback().OnError(aError); }

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override
    {
        NextCallback().OnEventData(aEventHeader, apData, apStatus);
    }

    void OnDone(ReadClient * apReadClient) override { NextCallback().OnDone(apReadClient); }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override
    {
        NextCallback().OnSubscriptionEstablished(aSubscriptionId);
    }

    CHIP_ERROR OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override
    {
        return NextCallback().OnResubscriptionNeeded(apReadClient, aTerminationCause);
    }

    void OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams) override
    {
        NextCallback().OnDeallocatePaths(std::move(aReadPrepareParams));
    }

    CHIP_ERROR OnUpdateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                             const Span<AttributePathParams> & aAttributePaths,
                                             bool & aEncodedDataVersionList) override
    {
        return NextCallback().OnUpdateDataVersionFilterList(aDataVersionFilterIBsBuilder, aAttributePaths, aEncodedDataVersionList);
    }

    CHIP_ERROR GetHighestReceivedEventNumber(Optional<EventNumber> & aEventNumber) override
    {
        return NextCallback().GetHighestReceivedEventNumber(aEventNumber);
    }

    void OnUnsolicitedMessageFromPublisher(ReadClient * apReadClient) override
    {
        NextCallback().OnUnsolicitedMessageFromPublisher(apReadClient);
    }

    void OnCASESessionEstablished(const SessionHandle & aSession, ReadPrepareParams & aSubscriptionParams) override
    {
        NextCallback().OnCASESessionEstablished(aSession, aSubscriptionParams);
    }

    BufferedReadCallback mBufferedCallback;
    bool mSawEmptyList               = false;
    bool mDecodingFailed             = false;
    uint32_t mAttributeDataIBCount   = 0;
    uint32_t mAttributeStatusIBCount = 0;
};

class TestReadCallback : public app::ReadClient::Callback
{
public:
    TestReadCallback() : mBufferedCallback(*this) {}
    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override;

    void OnDone(app::ReadClient * apReadClient) override;

    void OnReportEnd() override { mOnReportEnd = true; }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override { mOnSubscriptionEstablished = true; }

    void OnError(CHIP_ERROR aError) override { mReadError = aError; }

    uint32_t mAttributeCount        = 0;
    bool mOnReportEnd               = false;
    bool mOnSubscriptionEstablished = false;
    CHIP_ERROR mReadError           = CHIP_NO_ERROR;
    TestBufferedReadCallback mBufferedCallback;
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
    else if (aPath.mAttributeId != kTestListAttribute)
    {
        uint8_t v;
        EXPECT_EQ(app::DataModel::Decode(*apData, v), CHIP_NO_ERROR);
        EXPECT_EQ(v, (uint8_t) gIterationCount);
    }
    else
    {
        app::DataModel::DecodableList<uint8_t> v;
        EXPECT_EQ(app::DataModel::Decode(*apData, v), CHIP_NO_ERROR);
        auto it          = v.begin();
        size_t arraySize = 0;
        while (it.Next())
        {
            EXPECT_EQ(it.GetValue(), static_cast<uint8_t>(gIterationCount));
        }
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(v.ComputeSize(&arraySize), CHIP_NO_ERROR);
        EXPECT_EQ(arraySize, 5u);
    }
    mAttributeCount++;
}

void TestReadCallback::OnDone(app::ReadClient *) {}

class TestMutableAttrAccess
{
public:
    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder);

    void SetDirty(AttributeId attr)
    {
        app::AttributePathParams path;
        path.mEndpointId  = kTestEndpointId5;
        path.mClusterId   = Clusters::UnitTesting::Id;
        path.mAttributeId = attr;
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    }

    // These setters
    void SetVal(uint8_t attribute, uint8_t newVal)
    {
        uint8_t index = static_cast<uint8_t>(attribute - 1);
        if (index < MATTER_ARRAY_SIZE(val) && val[index] != newVal)
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
    VerifyOrReturnError(aPath.mEndpointId == kTestEndpointId5 && index < MATTER_ARRAY_SIZE(val), CHIP_ERROR_NOT_FOUND);
    return aEncoder.Encode(val[index]);
}

TestMutableAttrAccess gMutableAttrAccess;

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
    CHIP_ERROR err = gMutableAttrAccess.Read(aPath, aEncoder);
    if (err != CHIP_ERROR_NOT_FOUND)
    {
        return err;
    }

    switch (aPath.mAttributeId)
    {
    case kTestListAttribute:
        return aEncoder.EncodeList([](const auto & encoder) {
            for (int i = 0; i < kListAttributeItems; i++)
            {
                ReturnErrorOnFailure(encoder.Encode((uint8_t) gIterationCount));
            }
            return CHIP_NO_ERROR;
        });
    case kTestBadAttribute:
        // The "BadAttribute" is implemented by encoding a very large octet string, then the encode will always return
        // CHIP_ERROR_NO_MEMORY.
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

    void OnDone(app::ReadClient *) override {}

    void OnReportBegin() override { mAttributeCount = 0; }

    void OnReportEnd() override { mOnReportEnd = true; }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override { mOnSubscriptionEstablished = true; }

    uint32_t mAttributeCount = 0;
    // We record every dataversion field from every attribute IB.
    std::map<std::pair<EndpointId, AttributeId>, DataVersion> mDataVersions;
    std::map<std::pair<EndpointId, AttributeId>, uint8_t> mValues;
    std::map<std::pair<EndpointId, AttributeId>, std::function<void()>> mActionOn;
    bool mOnReportEnd               = false;
    bool mOnSubscriptionEstablished = false;
    app::BufferedReadCallback mBufferedCallback;
};

void TestMutableReadCallback::OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                              const app::StatusIB & aStatus)
{
    VerifyOrReturn(apData != nullptr);
    EXPECT_EQ(aPath.mClusterId, Clusters::UnitTesting::Id);

    mAttributeCount++;
    if (aPath.mAttributeId <= 5)
    {
        uint8_t v;
        EXPECT_EQ(app::DataModel::Decode(*apData, v), CHIP_NO_ERROR);
        mValues[std::make_pair(aPath.mEndpointId, aPath.mAttributeId)] = v;

        auto action = mActionOn.find(std::make_pair(aPath.mEndpointId, aPath.mAttributeId));
        if (action != mActionOn.end() && action->second)
        {
            action->second();
        }
    }

    if (aPath.mDataVersion.HasValue())
    {
        mDataVersions[std::make_pair(aPath.mEndpointId, aPath.mAttributeId)] = aPath.mDataVersion.Value();
    }
    // Ignore all other attributes, we don't care above the global attributes.
}

class TestReadChunking : public chip::Test::AppContext
{
protected:
    struct Instruction;
    void DoTest(TestMutableReadCallback * callback, Instruction instruction);
    void DriveIOUntilSubscriptionEstablished(TestMutableReadCallback * callback);
    void DriveIOUntilEndOfReport(TestMutableReadCallback * callback);
};

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
TEST_F(TestReadChunking, TestChunking)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpointClusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

    app::AttributePathParams attributePath(kTestEndpointId, app::Clusters::UnitTesting::Id);
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

        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_TRUE(readCallback.mOnReportEnd);

        //
        // Always returns the same number of attributes read (5 + revision + GlobalAttributesNotInMetadata).
        //
        EXPECT_EQ(readCallback.mAttributeCount, 6 + MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata));
        readCallback.mAttributeCount = 0;

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

// Similar to the test above, but for the list chunking feature.
TEST_F(TestReadChunking, TestListChunking)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpoint3Clusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId3, &testEndpoint3, Span<DataVersion>(dataVersionStorage));

    app::AttributePathParams attributePath(kTestEndpointId3, app::Clusters::UnitTesting::Id, kTestListAttribute);
    app::ReadPrepareParams readParams(sessionHandle);

    // Read the path twice, so we get two lists.  This make it easier to check
    // for what happens when one of the lists starts near the end of a packet
    // boundary.

    AttributePathParams pathList[] = { attributePath, attributePath };

    readParams.mpAttributePathParamsList    = pathList;
    readParams.mAttributePathParamsListSize = MATTER_ARRAY_SIZE(pathList);

    constexpr size_t maxPacketSize = kMaxSecureSduLengthBytes;
    bool gotSuccessfulEncode       = false;
    bool gotFailureResponse        = false;

    //
    // Make sure we start off the packet size large enough that we can fit a
    // single status response in it.  Verify that we get at least one status
    // response.  Then sweep up over packet sizes until we're big enough to hold
    // something like 7 IBs (at 30-40 bytes each, so call it 200 bytes) and check
    // the behavior for all those cases.
    //
    for (uint32_t packetSize = 30; packetSize < 200; packetSize++)
    {
        TestReadCallback readCallback;

        ChipLogDetail(DataManagement, "Running iteration %d\n", static_cast<int>(packetSize));

        gIterationCount = packetSize;

        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(
            static_cast<uint32_t>(maxPacketSize - packetSize));

        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        // Up until our packets are big enough, we might just keep getting
        // errors due to the inability to encode even a single IB in a packet.
        // But once we manage a successful encode, we should not have any more failures.
        if (!gotSuccessfulEncode && readCallback.mReadError != CHIP_NO_ERROR)
        {
            gotFailureResponse = true;
            // Check for the right error type.
            EXPECT_EQ(StatusIB(readCallback.mReadError).mStatus, Protocols::InteractionModel::Status::ResourceExhausted);
        }
        else
        {
            gotSuccessfulEncode = true;

            EXPECT_TRUE(readCallback.mOnReportEnd);

            //
            // Always returns the same number of attributes read (merged by buffered read callback). The content is checked in
            // TestReadCallback::OnAttributeData.  The attribute count is 1
            // because the buffered callback treats the second read's path as being
            // just a replace of the first read's path and buffers it all up as a
            // single value.
            //
            EXPECT_EQ(readCallback.mAttributeCount, 1u);
            readCallback.mAttributeCount = 0;

            // Check that we never saw an empty-list data IB.
            EXPECT_FALSE(readCallback.mBufferedCallback.mDecodingFailed);
            EXPECT_FALSE(readCallback.mBufferedCallback.mSawEmptyList);
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

        //
        // Stop the test if we detected an error. Otherwise, it'll be difficult to read the logs.
        //
        if (HasFailure())
        {
            break;
        }
    }

    // If this fails, our smallest packet size was not small enough.
    EXPECT_TRUE(gotFailureResponse);

    emberAfClearDynamicEndpoint(0);
}

// Read an attribute that can never fit into the buffer. Result in an empty report, server should shutdown the transaction.
TEST_F(TestReadChunking, TestBadChunking)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(0);

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpoint3Clusters)];
    emberAfSetDynamicEndpoint(0, kTestEndpointId3, &testEndpoint3, Span<DataVersion>(dataVersionStorage));

    app::AttributePathParams attributePath(kTestEndpointId3, app::Clusters::UnitTesting::Id, kTestBadAttribute);
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;

    TestReadCallback readCallback;

    {
        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        // The server should return an empty list as attribute data for the first report (for list chunking), and encodes nothing
        // (then shuts down the read handler) for the second report.
        //

        // Nothing is actually encoded. buffered callback does not handle the message to us.
        EXPECT_EQ(readCallback.mAttributeCount, 0u);
        EXPECT_FALSE(readCallback.mOnReportEnd);

        // The server should shutted down, while the client is still alive (pending for the attribute data.)
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    // Sanity check
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}

/*
 * This test contains two parts, one is to enable a new endpoint on the fly, another is to disable it and re-enable it.
 */
TEST_F(TestReadChunking, TestDynamicEndpoint)
{
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    InitDataModelHandler();

    // Register our fake dynamic endpoint.
    DataVersion dataVersionStorage[MATTER_ARRAY_SIZE(testEndpoint4Clusters)];

    app::AttributePathParams attributePath;
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mMaxIntervalCeilingSeconds   = 1;

    TestReadCallback readCallback;

    {

        app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                   app::ReadClient::InteractionType::Subscribe);
        // Enable the new endpoint
        emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage));

        EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(readCallback.mOnSubscriptionEstablished);
        readCallback.mAttributeCount = 0;

        emberAfSetDynamicEndpoint(0, kTestEndpointId4, &testEndpoint4, Span<DataVersion>(dataVersionStorage));

        DrainAndServiceIO();

        // Ensure we have received the report, we do not care about the initial report here.
        // GlobalAttributesNotInMetadata attributes are not included in testClusterAttrsOnEndpoint4.
        EXPECT_EQ(readCallback.mAttributeCount,
                  MATTER_ARRAY_SIZE(testClusterAttrsOnEndpoint4) + MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata));

        // We have received all report data.
        EXPECT_TRUE(readCallback.mOnReportEnd);

        readCallback.mAttributeCount = 0;
        readCallback.mOnReportEnd    = false;

        // Disable the new endpoint
        emberAfEndpointEnableDisable(kTestEndpointId4, false);

        DrainAndServiceIO();

        // We may receive some attribute reports for descriptor cluster, but we do not care about it for now.

        // Enable the new endpoint

        readCallback.mAttributeCount = 0;
        readCallback.mOnReportEnd    = false;

        emberAfEndpointEnableDisable(kTestEndpointId4, true);
        DrainAndServiceIO();

        // Ensure we have received the report, we do not care about the initial report here.
        // GlobalAttributesNotInMetadata attributes are not included in testClusterAttrsOnEndpoint4.
        EXPECT_EQ(readCallback.mAttributeCount,
                  MATTER_ARRAY_SIZE(testClusterAttrsOnEndpoint4) + MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata));

        // We have received all report data.
        EXPECT_TRUE(readCallback.mOnReportEnd);
    }

    chip::test_utils::SleepMillis(SecondsToMilliseconds(2));

    // Destroying the read client will terminate the subscription transaction.
    DrainAndServiceIO();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(0);
}
/*
 * The tests below are for testing deatiled bwhavior when the attributes are modified between two chunks. In this test, we only care
 * above whether we will receive correct attribute values in reasonable messages with reduced reporting traffic.
 */

namespace TestSetDirtyBetweenChunksUtil {

using AttributeIdWithEndpointId = std::pair<EndpointId, AttributeId>;

template <AttributeId id>
constexpr AttributeIdWithEndpointId AttrOnEp1 = AttributeIdWithEndpointId(kTestEndpointId, id);

template <AttributeId id>
constexpr AttributeIdWithEndpointId AttrOnEp5 = AttributeIdWithEndpointId(kTestEndpointId5, id);

auto WriteAttrOp(AttributeIdWithEndpointId attr, uint8_t val)
{
    return [=]() { gMutableAttrAccess.SetVal(static_cast<uint8_t>(attr.second), val); };
}

auto TouchAttrOp(AttributeIdWithEndpointId attr)
{
    return [=]() {
        app::AttributePathParams path;
        path.mEndpointId  = attr.first;
        path.mClusterId   = Clusters::UnitTesting::Id;
        path.mAttributeId = attr.second;
        gIterationCount++;
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    };
}

enum AttrIds
{
    Attr1 = 1,
    Attr2 = 2,
    Attr3 = 3,
};

using AttributeWithValue = std::pair<AttributeIdWithEndpointId, uint8_t>;
using AttributesList     = std::vector<AttributeIdWithEndpointId>;

void CheckValues(TestMutableReadCallback * callback, std::vector<AttributeWithValue> expectedValues = {})
{
    for (const auto & vals : expectedValues)
    {
        EXPECT_EQ(callback->mValues[vals.first], vals.second);
    }
}

void ExpectSameDataVersions(TestMutableReadCallback * callback, AttributesList attrList)
{
    if (attrList.size() == 0)
    {
        return;
    }
    DataVersion expectedVersion = callback->mDataVersions[attrList[0]];
    for (const auto & attr : attrList)
    {
        EXPECT_EQ(callback->mDataVersions[attr], expectedVersion);
    }
}

}; // namespace TestSetDirtyBetweenChunksUtil

struct TestReadChunking::Instruction
{
    // The maximum number of attributes should be iterated in a single report chunk.
    uint32_t chunksize;
    // A list of functions that will be executed before driving the main loop.
    std::vector<std::function<void()>> preworks;
    // A list of pair for attributes and their expected values in the report.
    std::vector<TestSetDirtyBetweenChunksUtil::AttributeWithValue> expectedValues;
    // A list of list of various attributes which should have the same data version in the report.
    std::vector<TestSetDirtyBetweenChunksUtil::AttributesList> attributesWithSameDataVersion;
};

void TestReadChunking::DriveIOUntilSubscriptionEstablished(TestMutableReadCallback * callback)
{
    callback->mOnReportEnd = false;
    GetIOContext().DriveIOUntil(System::Clock::Seconds16(5), [&]() { return callback->mOnSubscriptionEstablished; });
    EXPECT_TRUE(callback->mOnReportEnd);
    EXPECT_TRUE(callback->mOnSubscriptionEstablished);
    callback->mActionOn.clear();
}

void TestReadChunking::DriveIOUntilEndOfReport(TestMutableReadCallback * callback)
{
    callback->mOnReportEnd = false;
    GetIOContext().DriveIOUntil(System::Clock::Seconds16(5), [&]() { return callback->mOnReportEnd; });
    EXPECT_TRUE(callback->mOnReportEnd);
    callback->mActionOn.clear();
}

void TestReadChunking::DoTest(TestMutableReadCallback * callback, Instruction instruction)
{
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetMaxAttributesPerChunk(instruction.chunksize);

    for (const auto & act : instruction.preworks)
    {
        act();
    }

    DriveIOUntilEndOfReport(callback);

    TestSetDirtyBetweenChunksUtil::CheckValues(callback, instruction.expectedValues);

    for (const auto & attrList : instruction.attributesWithSameDataVersion)
    {
        TestSetDirtyBetweenChunksUtil::ExpectSameDataVersions(callback, attrList);
    }
}

TEST_F(TestReadChunking, TestSetDirtyBetweenChunks)
{
    using namespace TestSetDirtyBetweenChunksUtil;
    auto sessionHandle                   = GetSessionBobToAlice();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();

    // Initialize the ember side server logic
    InitDataModelHandler();

    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetWriterReserved(0);
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetMaxAttributesPerChunk(2);

    DataVersion dataVersionStorage1[MATTER_ARRAY_SIZE(testEndpointClusters)];
    DataVersion dataVersionStorage5[MATTER_ARRAY_SIZE(testEndpoint5Clusters)];

    gMutableAttrAccess.Reset();

    // Register our fake dynamic endpoint.
    emberAfSetDynamicEndpoint(0, kTestEndpointId, &testEndpoint, Span<DataVersion>(dataVersionStorage1));
    emberAfSetDynamicEndpoint(1, kTestEndpointId5, &testEndpoint5, Span<DataVersion>(dataVersionStorage5));

    {
        app::AttributePathParams attributePath;
        app::ReadPrepareParams readParams(sessionHandle);

        readParams.mpAttributePathParamsList    = &attributePath;
        readParams.mAttributePathParamsListSize = 1;
        readParams.mMinIntervalFloorSeconds     = 0;
        readParams.mMaxIntervalCeilingSeconds   = 2;

        // TEST 1 -- Read using wildcard paths
        ChipLogProgress(DataManagement, "Test 1: Read using wildcard paths.");
        {
            TestMutableReadCallback readCallback;

            gIterationCount = 1;

            app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                       app::ReadClient::InteractionType::Subscribe);

            EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

            // CASE 1 -- Touch an attribute during priming report, then verify it is included in first report after priming report.
            {
                // When the report engine starts to report attributes in endpoint 5, mark cluster 1 as dirty.
                // The report engine should NOT include it in initial report to reduce traffic.
                // We are expected to miss attributes on kTestEndpointId during initial reports.
                ChipLogProgress(DataManagement, "Case 1-1: Set dirty during priming report.");
                readCallback.mActionOn[AttrOnEp5<Attr1>] = TouchAttrOp(AttrOnEp1<Attr1>);
                DriveIOUntilSubscriptionEstablished(&readCallback);
                CheckValues(&readCallback, { { AttrOnEp1<Attr1>, 1 } });

                ChipLogProgress(DataManagement, "Case 1-2: Check for attributes missed last report.");
                DoTest(&readCallback, Instruction{ .chunksize = 2, .expectedValues = { { AttrOnEp1<Attr1>, 2 } } });
            }

            // CASE 2 -- Set dirty during chunked report, the attribute is already dirty.
            {
                ChipLogProgress(DataManagement, "Case 2: Set dirty during chunked report by wildcard path.");
                readCallback.mActionOn[AttrOnEp5<Attr2>] = WriteAttrOp(AttrOnEp5<Attr3>, 3);
                DoTest(
                    &readCallback,
                    Instruction{ .chunksize      = 2,
                                 .preworks       = { WriteAttrOp(AttrOnEp5<Attr1>, 2), WriteAttrOp(AttrOnEp5<Attr2>, 2),
                                                     WriteAttrOp(AttrOnEp5<Attr3>, 2) },
                                 .expectedValues = { { AttrOnEp5<Attr1>, 2 }, { AttrOnEp5<Attr2>, 2 }, { AttrOnEp5<Attr3>, 3 } },
                                 .attributesWithSameDataVersion = { { AttrOnEp5<Attr1>, AttrOnEp5<Attr2>, AttrOnEp5<Attr3> } } });
            }

            // CASE 3 -- Set dirty during chunked report, the attribute is not dirty, and it may catch / missed the current report.
            {
                ChipLogProgress(DataManagement,
                                "Case 3-1: Set dirty during chunked report by wildcard path -- new dirty attribute.");
                readCallback.mActionOn[AttrOnEp5<Attr2>] = WriteAttrOp(AttrOnEp5<Attr3>, 4);
                DoTest(
                    &readCallback,
                    Instruction{ .chunksize      = 1,
                                 .preworks       = { WriteAttrOp(AttrOnEp5<Attr1>, 4), WriteAttrOp(AttrOnEp5<Attr2>, 4) },
                                 .expectedValues = { { AttrOnEp5<Attr1>, 4 }, { AttrOnEp5<Attr2>, 4 }, { AttrOnEp5<Attr3>, 4 } },
                                 .attributesWithSameDataVersion = { { AttrOnEp5<Attr1>, AttrOnEp5<Attr2>, AttrOnEp5<Attr3> } } });

                ChipLogProgress(DataManagement,
                                "Case 3-2: Set dirty during chunked report by wildcard path -- new dirty attribute.");
                app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetMaxAttributesPerChunk(1);
                readCallback.mActionOn[AttrOnEp5<Attr2>] = WriteAttrOp(AttrOnEp5<Attr1>, 5);
                DoTest(
                    &readCallback,
                    Instruction{ .chunksize      = 1,
                                 .preworks       = { WriteAttrOp(AttrOnEp5<Attr2>, 5), WriteAttrOp(AttrOnEp5<Attr3>, 5) },
                                 .expectedValues = { { AttrOnEp5<Attr1>, 5 }, { AttrOnEp5<Attr2>, 5 }, { AttrOnEp5<Attr3>, 5 } },
                                 .attributesWithSameDataVersion = { { AttrOnEp5<Attr1>, AttrOnEp5<Attr2>, AttrOnEp5<Attr3> } } });
            }
        }
    }
    // The read client is destructed, server will shutdown the corresponding subscription later.

    // TEST 2 -- Read using concrete paths.
    ChipLogProgress(DataManagement, "Test 2: Read using concrete paths.");
    {
        app::AttributePathParams attributePath[3];
        app::ReadPrepareParams readParams(sessionHandle);

        attributePath[0] = app::AttributePathParams(kTestEndpointId5, Clusters::UnitTesting::Id, Attr1);
        attributePath[1] = app::AttributePathParams(kTestEndpointId5, Clusters::UnitTesting::Id, Attr2);
        attributePath[2] = app::AttributePathParams(kTestEndpointId5, Clusters::UnitTesting::Id, Attr3);

        readParams.mpAttributePathParamsList    = attributePath;
        readParams.mAttributePathParamsListSize = 3;
        readParams.mMinIntervalFloorSeconds     = 0;
        readParams.mMaxIntervalCeilingSeconds   = 2;
        gMutableAttrAccess.Reset();

        // CASE 1 -- Touch an attribute during priming report, then verify it is included in first report after priming report.
        {
            TestMutableReadCallback readCallback;

            app::ReadClient readClient(engine, &GetExchangeManager(), readCallback.mBufferedCallback,
                                       app::ReadClient::InteractionType::Subscribe);

            EXPECT_EQ(readClient.SendRequest(readParams), CHIP_NO_ERROR);

            DriveIOUntilSubscriptionEstablished(&readCallback);

            // Note, although the two attributes comes from the same cluster, they are generated by different interested paths.
            // In this case, we won't reset the path iterator.
            ChipLogProgress(DataManagement, "Case 1-1: Test set dirty during reports generated by concrete paths.");
            readCallback.mActionOn[AttrOnEp5<Attr2>] = WriteAttrOp(AttrOnEp5<Attr3>, 4);
            DoTest(&readCallback,
                   Instruction{ .chunksize      = 1,
                                .preworks       = { WriteAttrOp(AttrOnEp5<Attr1>, 3), WriteAttrOp(AttrOnEp5<Attr2>, 3),
                                                    WriteAttrOp(AttrOnEp5<Attr3>, 3) },
                                .expectedValues = { { AttrOnEp5<Attr1>, 3 }, { AttrOnEp5<Attr2>, 3 }, { AttrOnEp5<Attr3>, 3 } } });

            // The attribute failed to catch last report will be picked by this report.
            ChipLogProgress(DataManagement, "Case 1-2: Check for attributes missed last report.");
            DoTest(&readCallback, { .chunksize = 1, .expectedValues = { { AttrOnEp5<Attr3>, 4 } } });
        }
    }

    chip::test_utils::SleepMillis(SecondsToMilliseconds(3));

    // Destroying the read client will terminate the subscription transaction.
    DrainAndServiceIO();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    emberAfClearDynamicEndpoint(1);
    emberAfClearDynamicEndpoint(0);
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetMaxAttributesPerChunk(UINT32_MAX);
}

} // namespace
