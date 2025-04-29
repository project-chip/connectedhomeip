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
#include <pw_unit_test/framework.h>

#include <access/examples/PermissiveAccessControlDelegate.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelHelper.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/icd/server/ICDServerConfig.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/interaction_model/Constants.h>

#include <optional>

namespace {
using namespace chip::app::Clusters::Globals::Attributes;

uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];
chip::ClusterId kTestClusterId          = 6; // OnOff, but not used as OnOff directly
chip::ClusterId kTestEventClusterId     = chip::Test::MockClusterId(1);
chip::ClusterId kInvalidTestClusterId   = 7;
chip::EndpointId kTestEndpointId        = 1;
chip::EndpointId kTestEventEndpointId   = chip::Test::kMockEndpoint1;
chip::EventId kTestEventIdDebug         = chip::Test::MockEventId(1);
chip::EventId kTestEventIdCritical      = chip::Test::MockEventId(2);
chip::TLV::Tag kTestEventTag            = chip::TLV::ContextTag(1);
chip::EndpointId kInvalidTestEndpointId = 3;
chip::DataVersion kTestDataVersion1     = 3;
chip::DataVersion kTestDataVersion2     = 5;

// Number of items in the list for MockAttributeId(4).
constexpr int kMockAttribute4ListLength = 6;

static chip::System::Clock::Internal::MockClock gMockClock;
static chip::System::Clock::ClockBase * gRealClock;
static chip::app::reporting::ReportSchedulerImpl * gReportScheduler;
static bool sUsingSubSync = false;

const chip::Test::MockNodeConfig & TestMockNodeConfig()
{
    using namespace chip::app;
    using namespace chip::Test;

    // clang-format off
    static const chip::Test::MockNodeConfig config({
        MockEndpointConfig(kTestEndpointId, {
            MockClusterConfig(kTestClusterId, {
                ClusterRevision::Id, FeatureMap::Id,
                1,
                2, // treated as a list
            }),
            MockClusterConfig(kInvalidTestClusterId, {
                ClusterRevision::Id, FeatureMap::Id,
                1,
            }),
        }),
        MockEndpointConfig(kMockEndpoint1, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id,
            }, {
                MockEventId(1), MockEventId(2),
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
            }),
        }),
        MockEndpointConfig(kMockEndpoint2, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2),
            }),
            MockClusterConfig(MockClusterId(3), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
            }),
        }),
        MockEndpointConfig(chip::Test::kMockEndpoint3, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
            }),
            MockClusterConfig(MockClusterId(3), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
            MockClusterConfig(MockClusterId(4), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
        }),
    });
    // clang-format on
    return config;
}

class TestEventGenerator : public chip::app::EventLoggingDelegate
{
public:
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter)
    {
        chip::TLV::TLVType dataContainerType;
        ReturnErrorOnFailure(aWriter.StartContainer(chip::TLV::ContextTag(chip::to_underlying(chip::app::EventDataIB::Tag::kData)),
                                                    chip::TLV::kTLVType_Structure, dataContainerType));
        ReturnErrorOnFailure(aWriter.Put(kTestEventTag, mStatus));
        return aWriter.EndContainer(dataContainerType);
    }

    void SetStatus(int32_t aStatus) { mStatus = aStatus; }

private:
    int32_t mStatus;
};

void GenerateEvents()
{
    chip::EventNumber eid1, eid2;
    chip::app::EventOptions options1;
    options1.mPath     = { kTestEventEndpointId, kTestEventClusterId, kTestEventIdDebug };
    options1.mPriority = chip::app::PriorityLevel::Info;

    chip::app::EventOptions options2;
    options2.mPath     = { kTestEventEndpointId, kTestEventClusterId, kTestEventIdCritical };
    options2.mPriority = chip::app::PriorityLevel::Critical;
    TestEventGenerator testEventGenerator;
    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();

    ChipLogDetail(DataManagement, "Generating Events");
    testEventGenerator.SetStatus(0);

    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid1), CHIP_NO_ERROR);
    testEventGenerator.SetStatus(1);

    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options2, eid2), CHIP_NO_ERROR);
}

/// Represents an expected attribute capture
class AttributeCaptureAssertion
{
public:
    constexpr AttributeCaptureAssertion(chip::EndpointId ep, chip::ClusterId cl, chip::AttributeId at,
                                        std::optional<unsigned> listSize = std::nullopt) :
        mEndpoint(ep),
        mCluster(cl), mAttribute(at), mListSize(listSize)
    {}

    chip::app::ConcreteAttributePath Path() const { return chip::app::ConcreteAttributePath(mEndpoint, mCluster, mAttribute); }

    chip::EndpointId Endpoint() const { return mEndpoint; }
    chip::ClusterId Cluster() const { return mCluster; }
    chip::AttributeId Attribute() const { return mAttribute; }
    std::optional<unsigned> ListSize() const { return mListSize; }

    bool Matches(const chip::app::ConcreteAttributePath & path, const std::optional<unsigned> & listSize) const
    {
        return (Path() == path) && (mListSize == listSize);
    }

private:
    // this is split out because ConcreteAttributePath is NOT constexpr
    const chip::EndpointId mEndpoint;
    const chip::ClusterId mCluster;
    const chip::AttributeId mAttribute;
    const std::optional<unsigned> mListSize;
};

class MockInteractionModelApp : public chip::app::ReadClient::Callback
{
public:
    void OnEventData(const chip::app::EventHeader & aEventHeader, chip::TLV::TLVReader * apData,
                     const chip::app::StatusIB * apStatus) override
    {
        ++mNumDataElementIndex;
        mGotEventResponse = true;
        if (apStatus != nullptr && !apStatus->IsSuccess())
        {
            mNumReadEventFailureStatusReceived++;
            mLastStatusReceived = *apStatus;
        }
        else
        {
            mLastStatusReceived = chip::app::StatusIB();
        }
    }

    void OnAttributeData(const chip::app::ConcreteDataAttributePath & aPath, chip::TLV::TLVReader * apData,
                         const chip::app::StatusIB & status) override
    {
        if (status.mStatus == chip::Protocols::InteractionModel::Status::Success)
        {
            ChipLogProgress(Test, "Attribute data received 0x%X/" ChipLogFormatMEI "/" ChipLogFormatMEI " Success: LIST: %s",
                            aPath.mEndpointId, ChipLogValueMEI(aPath.mClusterId), ChipLogValueMEI(aPath.mAttributeId),
                            aPath.IsListOperation() ? "true" : "false");
            mReceivedAttributePaths.push_back(aPath);
            mNumAttributeResponse++;
            mGotReport = true;

            std::optional<unsigned> listSize = std::nullopt;

            if (aPath.IsListItemOperation())
            {
                mNumArrayItems++;
                listSize = 1;
            }
            else if (aPath.IsListOperation())
            {
                // This is an entire list of things; count up how many.
                chip::TLV::TLVType containerType;
                if (apData->EnterContainer(containerType) == CHIP_NO_ERROR)
                {
                    size_t count = 0;
                    if (chip::TLV::Utilities::Count(*apData, count, /* aRecurse = */ false) == CHIP_NO_ERROR)
                    {
                        listSize = static_cast<unsigned>(count);
                        mNumArrayItems += static_cast<int>(count);
                        ChipLogProgress(Test, "   List count: %u", static_cast<unsigned>(count));
                    }
                }
            }
            mReceivedListSizes.push_back(listSize);
        }
        else
        {
            ChipLogError(NotSpecified, "ERROR status for 0x%X/" ChipLogFormatMEI "/" ChipLogFormatMEI, aPath.mEndpointId,
                         ChipLogValueMEI(aPath.mClusterId), ChipLogValueMEI(aPath.mAttributeId));
        }
        mLastStatusReceived = status;
    }

    void OnError(CHIP_ERROR aError) override
    {
        mError     = aError;
        mReadError = true;
    }

    void OnDone(chip::app::ReadClient *) override {}

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        if (aReadPrepareParams.mpAttributePathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpAttributePathParamsList;
        }

        if (aReadPrepareParams.mpEventPathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpEventPathParamsList;
        }

        if (aReadPrepareParams.mpDataVersionFilterList != nullptr)
        {
            delete[] aReadPrepareParams.mpDataVersionFilterList;
        }
    }

    // Log the current captures in a code-like format, to more easily update tests
    void LogCaptures(const char * heading)
    {
        ChipLogProgress(Test, "Captured attributes (%s):", heading);

        for (unsigned i = 0; i < mReceivedAttributePaths.size(); i++)
        {
            const auto & path = mReceivedAttributePaths[i];
            chip::StringBuilder<128> argsBuffer;

            if (path.mEndpointId >= 0xff)
            {
                argsBuffer.AddFormat("0x%X, ", path.mEndpointId);
            }
            else
            {
                argsBuffer.AddFormat("%u, ", path.mEndpointId);
            }

            if (path.mClusterId >= 0xff)
            {
                argsBuffer.AddFormat("0x%lX, ", static_cast<unsigned long>(path.mClusterId));
            }
            else
            {
                argsBuffer.AddFormat("%lu, ", static_cast<unsigned long>(path.mClusterId));
            }

            switch (path.mAttributeId)
            {
            case ClusterRevision::Id:
                argsBuffer.Add("ClusterRevision::Id");
                break;
            case FeatureMap::Id:
                argsBuffer.Add("FeatureMap::Id");
                break;
            case GeneratedCommandList::Id:
                argsBuffer.Add("GeneratedCommandList::Id");
                break;
            case AcceptedCommandList::Id:
                argsBuffer.Add("AcceptedCommandList::Id");
                break;
            case AttributeList::Id:
                argsBuffer.Add("AttributeList::Id");
                break;
            default:
                if (path.mAttributeId >= 0xff)
                {
                    argsBuffer.AddFormat("0x%lX", static_cast<unsigned long>(path.mAttributeId));
                }
                else
                {
                    argsBuffer.AddFormat("%lu", static_cast<unsigned long>(path.mAttributeId));
                }
                break;
            }

            auto listSize = mReceivedListSizes[i];
            if (listSize.has_value())
            {
                argsBuffer.AddFormat(", /* listSize = */ %u", listSize.value());
            }

            ChipLogProgress(Test, "  AttributeCaptureAssertion(%s),", argsBuffer.c_str());
        }
    }

    void Reset()
    {
        mNumDataElementIndex               = 0;
        mGotEventResponse                  = false;
        mNumReadEventFailureStatusReceived = 0;
        mNumAttributeResponse              = 0;
        mNumArrayItems                     = 0;
        mGotReport                         = false;
        mReadError                         = false;
        mError                             = CHIP_NO_ERROR;
        mReceivedAttributePaths.clear();
        mReceivedListSizes.clear();
    }

    bool CapturesMatchExactly(chip::Span<const AttributeCaptureAssertion> captures)
    {
        if (captures.size() != mReceivedAttributePaths.size())
        {
            ChipLogError(Test, "Captures do not match: expected %u, got %u instead", static_cast<unsigned>(captures.size()),
                         static_cast<unsigned>(mReceivedAttributePaths.size()));
            return false;
        }

        for (unsigned i = 0; i < mReceivedAttributePaths.size(); i++)
        {
            if (captures[i].Matches(mReceivedAttributePaths[i], mReceivedListSizes[i]))
            {
                continue;
            }

            ChipLogError(Test, "Failure on expected capture index %u:", i);

            chip::StringBuilder<128> buffer;
            buffer.AddFormat("0x%X/" ChipLogFormatMEI "/" ChipLogFormatMEI "", captures[i].Endpoint(),
                             ChipLogValueMEI(captures[i].Cluster()), ChipLogValueMEI(captures[i].Attribute()));
            auto listSize = captures[i].ListSize();
            if (listSize.has_value())
            {
                buffer.AddFormat(" - list of %u items", *listSize);
            }

            ChipLogError(Test, "  Expected: %s", buffer.c_str());

            buffer.Reset();
            buffer.AddFormat("0x%X/" ChipLogFormatMEI "/" ChipLogFormatMEI "", mReceivedAttributePaths[i].mEndpointId,
                             ChipLogValueMEI(mReceivedAttributePaths[i].mClusterId),
                             ChipLogValueMEI(mReceivedAttributePaths[i].mEndpointId));
            listSize = mReceivedListSizes[i];
            if (listSize.has_value())
            {
                buffer.AddFormat(" - list of %u items", *listSize);
            }
            ChipLogError(Test, "  Actual:   %s", buffer.c_str());
            return false;
        }

        return true;
    }

    int mNumDataElementIndex               = 0;
    bool mGotEventResponse                 = false;
    int mNumReadEventFailureStatusReceived = 0;
    int mNumAttributeResponse              = 0;
    int mNumArrayItems                     = 0;
    bool mGotReport                        = false;
    bool mReadError                        = false;
    chip::app::ReadHandler * mpReadHandler = nullptr;
    chip::app::StatusIB mLastStatusReceived;
    CHIP_ERROR mError = CHIP_NO_ERROR;
    std::vector<chip::app::ConcreteAttributePath> mReceivedAttributePaths;

    // For every received attribute path, report the size of the underlying list
    //   - nullopt if NOT a list
    //   - list size (including 0) if a list
    std::vector<std::optional<unsigned>> mReceivedListSizes;
};

//
// This dummy callback is used with a bunch of the tests below that don't go through
// the normal call-path of having the IM engine allocate the ReadHandler object. Instead,
// the object is allocated on stack for the purposes of a very narrow, tightly-coupled test.
//
// The typical callback implementor is the engine, but that would proceed to return the object
// back to the handler pool (which we obviously don't want in this case). This just no-ops those calls.
//
class NullReadHandlerCallback : public chip::app::ReadHandler::ManagementCallback
{
public:
    void OnDone(chip::app::ReadHandler & apReadHandlerObj) override {}
    chip::app::ReadHandler::ApplicationCallback * GetAppCallback() override { return nullptr; }
    chip::app::InteractionModelEngine * GetInteractionModelEngine() override
    {
        return chip::app::InteractionModelEngine::GetInstance();
    }
};

} // namespace

using ReportScheduler     = chip::app::reporting::ReportScheduler;
using ReportSchedulerImpl = chip::app::reporting::ReportSchedulerImpl;
using ReadHandlerNode     = chip::app::reporting::ReportScheduler::ReadHandlerNode;

namespace chip {
namespace app {

using Seconds16      = System::Clock::Seconds16;
using Milliseconds32 = System::Clock::Milliseconds32;

class TestReadInteraction : public chip::Test::AppContext
{
public:
    static void SetUpTestSuiteCommon(bool syncScheduler = false)
    {
        AppContext::SetUpTestSuite();

        gRealClock = &chip::System::SystemClock();
        chip::System::Clock::Internal::SetSystemClockForTesting(&gMockClock);

        sUsingSubSync    = syncScheduler;
        gReportScheduler = syncScheduler ? chip::app::reporting::GetSynchronizedReportScheduler()
                                         : chip::app::reporting::GetDefaultReportScheduler();
    }

    static void SetUpTestSuite() { SetUpTestSuiteCommon(false); }

    static void TearDownTestSuite()
    {
        chip::System::Clock::Internal::SetSystemClockForTesting(gRealClock);

        AppContext::TearDownTestSuite();
    }

    void SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        AppContext::SetUp();

        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), MATTER_ARRAY_SIZE(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);
        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(&TestImCustomDataModel::Instance());
        chip::Test::SetMockNodeConfig(TestMockNodeConfig());
        chip::Test::SetVersionTo(chip::Test::kTestDataVersion1);
    }

    void TearDown() override
    {
        chip::Test::ResetMockNodeConfig();
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        chip::app::EventManagement::DestroyEventManagement();
        AppContext::TearDown();
    }

    void TestICDProcessSubscribeRequestInfMaxIntervalCeiling();
    void TestICDProcessSubscribeRequestInvalidIdleModeDuration();
    void TestICDProcessSubscribeRequestMaxMinInterval();
    void TestICDProcessSubscribeRequestSupMaxIntervalCeiling();
    void TestICDProcessSubscribeRequestSupMinInterval();
    void TestPostSubscribeRoundtripChunkReport();
    void TestPostSubscribeRoundtripChunkReportTimeout();
    void TestPostSubscribeRoundtripChunkStatusReportTimeout();
    void TestPostSubscribeRoundtripStatusReportTimeout();
    void TestProcessSubscribeRequest();
    void TestReadChunking();
    void TestReadChunkingInvalidSubscriptionId();
    void TestReadChunkingStatusReportTimeout();
    void TestReadClient();
    void TestReadClientGenerateAttributePathList();
    void TestReadClientGenerateInvalidAttributePathList();
    void TestReadClientGenerateOneEventPaths();
    void TestReadClientGenerateTwoEventPaths();
    void TestReadClientInvalidAttributeId();
    void TestReadClientInvalidReport();
    void TestReadClientReceiveInvalidMessage();
    void TestReadHandler();
    void TestReadHandlerInvalidAttributePath();
    void TestReadHandlerInvalidSubscribeRequest();
    void TestReadHandlerMalformedReadRequest1();
    void TestReadHandlerMalformedReadRequest2();
    void TestReadHandlerMalformedSubscribeRequest();
    void TestReadHandlerSetMaxReportingInterval();
    void TestReadInvalidAttributePathRoundtrip();
    void TestReadReportFailure();
    void TestReadRoundtrip();
    void TestReadRoundtripWithDataVersionFilter();
    void TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter();
    void TestReadRoundtripWithNoMatchPathDataVersionFilter();
    void TestReadRoundtripWithSameDifferentPathsDataVersionFilter();
    void TestReadShutdown();
    void TestReadUnexpectedSubscriptionId();
    void TestReadWildcard();
    void TestSetDirtyBetweenChunks();
    void TestShutdownSubscription();
    void TestSubscribeClientReceiveInvalidReportMessage();
    void TestSubscribeClientReceiveInvalidStatusResponse();
    void TestSubscribeClientReceiveInvalidSubscribeResponseMessage();
    void TestSubscribeClientReceiveUnsolicitedInvalidReportMessage();
    void TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId();
    void TestSubscribeClientReceiveWellFormedStatusResponse();
    void TestSubscribeEarlyReport();
    void TestSubscribeEarlyShutdown();
    void TestSubscribeInvalidateFabric();
    void TestSubscribeInvalidAttributePathRoundtrip();
    void TestSubscribeInvalidInterval();
    void TestSubscribePartialOverlap();
    void TestSubscribeRoundtrip();
    void TestSubscribeRoundtripChunkStatusReportTimeout();
    void TestSubscribeRoundtripStatusReportTimeout();
    void TestSubscribeSendInvalidStatusReport();
    void TestSubscribeSendUnknownMessage();
    void TestSubscribeSetDirtyFullyOverlap();
    void TestSubscribeUrgentWildcardEvent();
    void TestSubscribeWildcard();
    void TestSubscriptionReportWithDefunctSession();

    enum class ReportType : uint8_t
    {
        kValid,
        kInvalidNoAttributeId,
        kInvalidOutOfRangeAttributeId,
    };

    static void GenerateReportData(System::PacketBufferHandle & aPayload, ReportType aReportType, bool aSuppressResponse,
                                   bool aHasSubscriptionId);

protected:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

class TestReadInteractionSync : public TestReadInteraction
{
public:
    static void SetUpTestSuite() { TestReadInteraction::SetUpTestSuiteCommon(true); }
    static void TearDownTestSuite() { TestReadInteraction::TearDownTestSuite(); }
    void SetUp() { TestReadInteraction::SetUp(); }
    void TearDown() { TestReadInteraction::TearDown(); }
};

void TestReadInteraction::GenerateReportData(System::PacketBufferHandle & aPayload, ReportType aReportType, bool aSuppressResponse,
                                             bool aHasSubscriptionId = false)
{
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    ReportDataMessage::Builder reportDataMessageBuilder;

    EXPECT_EQ(reportDataMessageBuilder.Init(&writer), CHIP_NO_ERROR);

    if (aHasSubscriptionId)
    {
        reportDataMessageBuilder.SubscriptionId(1);
        EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);
    }

    AttributeReportIBs::Builder & attributeReportIBsBuilder = reportDataMessageBuilder.CreateAttributeReportIBs();
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    AttributeReportIB::Builder & attributeReportIBBuilder = attributeReportIBsBuilder.CreateAttributeReport();
    EXPECT_EQ(attributeReportIBsBuilder.GetError(), CHIP_NO_ERROR);

    AttributeDataIB::Builder & attributeDataIBBuilder = attributeReportIBBuilder.CreateAttributeData();
    EXPECT_EQ(attributeReportIBBuilder.GetError(), CHIP_NO_ERROR);

    attributeDataIBBuilder.DataVersion(2);
    EXPECT_EQ(attributeDataIBBuilder.GetError(), CHIP_NO_ERROR);

    AttributePathIB::Builder & attributePathBuilder = attributeDataIBBuilder.CreatePath();
    EXPECT_EQ(attributeDataIBBuilder.GetError(), CHIP_NO_ERROR);

    if (aReportType == ReportType::kInvalidNoAttributeId)
    {
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).ListIndex(5).EndOfAttributePathIB();
    }
    else if (aReportType == ReportType::kInvalidOutOfRangeAttributeId)
    {
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(0xFFF18000).EndOfAttributePathIB();
    }
    else
    {
        EXPECT_EQ(aReportType, ReportType::kValid);
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).EndOfAttributePathIB();
    }

    EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = attributeDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        EXPECT_EQ(pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(chip::app::AttributeDataIB::Tag::kData)),
                                          chip::TLV::kTLVType_Structure, dummyType),
                  CHIP_NO_ERROR);

        EXPECT_EQ(pWriter->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);
        EXPECT_EQ(pWriter->EndContainer(dummyType), CHIP_NO_ERROR);
    }

    attributeDataIBBuilder.EndOfAttributeDataIB();
    EXPECT_EQ(attributeDataIBBuilder.GetError(), CHIP_NO_ERROR);

    attributeReportIBBuilder.EndOfAttributeReportIB();
    EXPECT_EQ(attributeReportIBBuilder.GetError(), CHIP_NO_ERROR);

    attributeReportIBsBuilder.EndOfAttributeReportIBs();
    EXPECT_EQ(attributeReportIBsBuilder.GetError(), CHIP_NO_ERROR);

    reportDataMessageBuilder.MoreChunkedMessages(false);
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    reportDataMessageBuilder.SuppressResponse(aSuppressResponse);
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    reportDataMessageBuilder.EndOfReportDataMessage();
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_EQ(writer.Finalize(&aPayload), CHIP_NO_ERROR);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClient)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClient)
void TestReadInteraction::TestReadClient()
{
    MockInteractionModelApp delegate;
    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    GetLoopback().mNumMessagesToDrop = 1;
    DrainAndServiceIO();

    GenerateReportData(buf, ReportType::kValid, true /* aSuppressResponse*/);
    EXPECT_EQ(readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction), CHIP_NO_ERROR);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadUnexpectedSubscriptionId)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadUnexpectedSubscriptionId)
void TestReadInteraction::TestReadUnexpectedSubscriptionId()
{
    MockInteractionModelApp delegate;
    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    GetLoopback().mNumMessagesToDrop = 1;
    DrainAndServiceIO();

    // For read, we don't expect there is subscription id in report data.
    GenerateReportData(buf, ReportType::kValid, true /* aSuppressResponse*/, true /*aHasSubscriptionId*/);
    EXPECT_EQ(readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadHandler)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadHandler)
void TestReadInteraction::TestReadHandler()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    NullReadHandlerCallback nullCallback;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    {
        Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);
        ReadHandler readHandler(nullCallback, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        GenerateReportData(reportDatabuf, ReportType::kValid, false /* aSuppressResponse*/);
        EXPECT_EQ(readHandler.SendReportData(std::move(reportDatabuf), false), CHIP_ERROR_INCORRECT_STATE);

        writer.Init(std::move(readRequestbuf));
        EXPECT_EQ(readRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
        readRequestBuilder.IsFabricFiltered(false).EndOfReadRequestMessage();
        EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&readRequestbuf), CHIP_NO_ERROR);

        // Call ProcessReadRequest directly, because OnInitialRequest sends status
        // messages on the wire instead of returning an error.
        EXPECT_EQ(readHandler.ProcessReadRequest(std::move(readRequestbuf)), CHIP_NO_ERROR);
    }

    engine->Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadHandlerSetMaxReportingInterval)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadHandlerSetMaxReportingInterval)
void TestReadInteraction::TestReadHandlerSetMaxReportingInterval()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    uint16_t kIntervalInfMinInterval = 119;
    uint16_t kMinInterval            = 120;
    uint16_t kMaxIntervalCeiling     = 500;

    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    {

        uint16_t minInterval;
        uint16_t maxInterval;

        // Configure ReadHandler
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        EXPECT_EQ(subscribeRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

#if CHIP_CONFIG_ENABLE_ICD_SERVER
        // When an ICD build, the default behavior is to select the IdleModeDuration as MaxInterval
        kMaxIntervalCeiling = readHandler.GetPublisherSelectedIntervalLimit();
#endif
        // Try to change the MaxInterval while ReadHandler is active
        EXPECT_EQ(readHandler.SetMaxReportingInterval(340), CHIP_ERROR_INCORRECT_STATE);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(kMaxIntervalCeiling, maxInterval);
        // Set ReadHandler to Idle to allow MaxInterval changes
        readHandler.MoveToState(ReadHandler::HandlerState::Idle);

        // TC1: MaxInterval < MinIntervalFloor
        EXPECT_EQ(readHandler.SetMaxReportingInterval(kIntervalInfMinInterval), CHIP_ERROR_INVALID_ARGUMENT);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(kMaxIntervalCeiling, maxInterval);

        // TC2: MaxInterval == MinIntervalFloor
        EXPECT_EQ(readHandler.SetMaxReportingInterval(kMinInterval), CHIP_NO_ERROR);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(kMinInterval, maxInterval);

        // TC3: Minterval < MaxInterval < max(GetPublisherSelectedIntervalLimit(), mSubscriberRequestedMaxInterval)
        EXPECT_EQ(readHandler.SetMaxReportingInterval(kMaxIntervalCeiling), CHIP_NO_ERROR);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(kMaxIntervalCeiling, maxInterval);

        // TC4: MaxInterval == Subscriber Requested Max Interval
        EXPECT_EQ(readHandler.SetMaxReportingInterval(readHandler.GetSubscriberRequestedMaxInterval()), CHIP_NO_ERROR);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(readHandler.GetSubscriberRequestedMaxInterval(), maxInterval);

        // TC4: MaxInterval == GetPublisherSelectedIntervalLimit()
        EXPECT_EQ(readHandler.SetMaxReportingInterval(readHandler.GetPublisherSelectedIntervalLimit()), CHIP_NO_ERROR);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(readHandler.GetPublisherSelectedIntervalLimit(), maxInterval);

        // TC5: MaxInterval >  max(GetPublisherSelectedIntervalLimit(), mSubscriberRequestedMaxInterval)
        EXPECT_EQ(readHandler.SetMaxReportingInterval(std::numeric_limits<uint16_t>::max()), CHIP_ERROR_INVALID_ARGUMENT);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(readHandler.GetPublisherSelectedIntervalLimit(), maxInterval);
    }

    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientGenerateAttributePathList)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientGenerateAttributePathList)
void TestReadInteraction::TestReadClientGenerateAttributePathList()
{
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mAttributeId = 0;
    attributePathParams[1].mAttributeId = 0;
    attributePathParams[1].mListIndex   = 0;

    Span<AttributePathParams> attributePaths(attributePathParams, 2 /*aAttributePathParamsListSize*/);

    AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
    EXPECT_EQ(readClient.GenerateAttributePaths(attributePathListBuilder, attributePaths), CHIP_NO_ERROR);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientGenerateInvalidAttributePathList)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientGenerateInvalidAttributePathList)
void TestReadInteraction::TestReadClientGenerateInvalidAttributePathList()
{
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    writer.Init(std::move(msgBuf));

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mAttributeId = 0;
    attributePathParams[1].mListIndex   = 0;

    Span<AttributePathParams> attributePaths(attributePathParams, 2 /*aAttributePathParamsListSize*/);

    AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
    EXPECT_EQ(readClient.GenerateAttributePaths(attributePathListBuilder, attributePaths),
              CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientInvalidReport)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientInvalidReport)
void TestReadInteraction::TestReadClientInvalidReport()
{
    MockInteractionModelApp delegate;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    GetLoopback().mNumMessagesToDrop = 1;
    DrainAndServiceIO();

    GenerateReportData(buf, ReportType::kInvalidNoAttributeId, true /* aSuppressResponse*/);

    EXPECT_EQ(readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction),
              CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientInvalidAttributeId)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientInvalidAttributeId)
void TestReadInteraction::TestReadClientInvalidAttributeId()
{
    MockInteractionModelApp delegate;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    GetLoopback().mNumMessagesToDrop = 1;
    DrainAndServiceIO();

    GenerateReportData(buf, ReportType::kInvalidOutOfRangeAttributeId, true /* aSuppressResponse*/);

    // Overall processing should succeed.
    EXPECT_EQ(readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction), CHIP_NO_ERROR);

    // We should not have gotten any attribute reports or errors.
    EXPECT_FALSE(delegate.mGotEventResponse);
    EXPECT_EQ(delegate.mNumAttributeResponse, 0);
    EXPECT_FALSE(delegate.mGotReport);
    EXPECT_FALSE(delegate.mReadError);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadHandlerInvalidAttributePath)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadHandlerInvalidAttributePath)
void TestReadInteraction::TestReadHandlerInvalidAttributePath()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    NullReadHandlerCallback nullCallback;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    {
        Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);
        ReadHandler readHandler(nullCallback, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        GenerateReportData(reportDatabuf, ReportType::kValid, false /* aSuppressResponse*/);
        EXPECT_EQ(readHandler.SendReportData(std::move(reportDatabuf), false), CHIP_ERROR_INCORRECT_STATE);

        writer.Init(std::move(readRequestbuf));
        EXPECT_EQ(readRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(attributePathListBuilder.EndOfAttributePathIBs(), CHIP_NO_ERROR);
        readRequestBuilder.EndOfReadRequestMessage();
        EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&readRequestbuf), CHIP_NO_ERROR);

        err = readHandler.ProcessReadRequest(std::move(readRequestbuf));
        ChipLogError(DataManagement, "The error is %s", ErrorStr(err));
        EXPECT_EQ(err, CHIP_ERROR_END_OF_TLV);

        //
        // In the call above to ProcessReadRequest, the handler will not actually close out the EC since
        // it expects the ExchangeManager to do so automatically given it's not calling WillSend() on the EC,
        // and is not sending a response back.
        //
        // Consequently, we have to manually close out the EC here in this test since we're not actually calling
        // methods on these objects in a manner similar to how it would happen in normal use.
        //
        exchangeCtx->Close();
    }

    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientGenerateOneEventPaths)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientGenerateOneEventPaths)
void TestReadInteraction::TestReadClientGenerateOneEventPaths()
{
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    chip::app::EventPathParams eventPathParams[1];
    eventPathParams[0].mEndpointId = 2;
    eventPathParams[0].mClusterId  = 3;
    eventPathParams[0].mEventId    = 4;

    EventPathIBs::Builder & eventPathListBuilder = request.CreateEventRequests();
    Span<EventPathParams> eventPaths(eventPathParams, 1 /*aEventPathParamsListSize*/);
    EXPECT_EQ(readClient.GenerateEventPaths(eventPathListBuilder, eventPaths), CHIP_NO_ERROR);

    request.IsFabricFiltered(false).EndOfReadRequestMessage();
    EXPECT_EQ(CHIP_NO_ERROR, request.GetError());

    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    EXPECT_EQ(readRequestParser.Init(reader), CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    readRequestParser.PrettyPrint();
#endif

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientGenerateTwoEventPaths)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientGenerateTwoEventPaths)
void TestReadInteraction::TestReadClientGenerateTwoEventPaths()
{
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    chip::app::EventPathParams eventPathParams[2];
    eventPathParams[0].mEndpointId = 2;
    eventPathParams[0].mClusterId  = 3;
    eventPathParams[0].mEventId    = 4;

    eventPathParams[1].mEndpointId = 2;
    eventPathParams[1].mClusterId  = 3;
    eventPathParams[1].mEventId    = 5;

    EventPathIBs::Builder & eventPathListBuilder = request.CreateEventRequests();
    Span<EventPathParams> eventPaths(eventPathParams, 2 /*aEventPathParamsListSize*/);
    EXPECT_EQ(readClient.GenerateEventPaths(eventPathListBuilder, eventPaths), CHIP_NO_ERROR);

    request.IsFabricFiltered(false).EndOfReadRequestMessage();
    EXPECT_EQ(CHIP_NO_ERROR, request.GetError());

    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    EXPECT_EQ(readRequestParser.Init(reader), CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    readRequestParser.PrettyPrint();
#endif

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadRoundtrip)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadRoundtrip)
void TestReadInteraction::TestReadRoundtrip()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::EventPathParams eventPathParams[1];
    eventPathParams[0].mEndpointId = kTestEventEndpointId;
    eventPathParams[0].mClusterId  = kTestEventClusterId;

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    attributePathParams[1].mEndpointId  = kTestEndpointId;
    attributePathParams[1].mClusterId   = kTestClusterId;
    attributePathParams[1].mAttributeId = 2;
    attributePathParams[1].mListIndex   = 1;

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = eventPathParams;
    readPrepareParams.mEventPathParamsListSize     = 1;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mEventNumber.SetValue(1);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(delegate.mNumDataElementIndex, 1);
        EXPECT_TRUE(delegate.mGotEventResponse);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);
        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_FALSE(delegate.mReadError);

        delegate.mGotEventResponse     = false;
        delegate.mNumAttributeResponse = 0;
        delegate.mGotReport            = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotEventResponse);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);
        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_FALSE(delegate.mReadError);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadRoundtripWithDataVersionFilter)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadRoundtripWithDataVersionFilter)
void TestReadInteraction::TestReadRoundtripWithDataVersionFilter()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    attributePathParams[1].mEndpointId  = kTestEndpointId;
    attributePathParams[1].mClusterId   = kTestClusterId;
    attributePathParams[1].mAttributeId = 2;
    attributePathParams[1].mListIndex   = 1;

    chip::app::DataVersionFilter dataVersionFilters[1];
    dataVersionFilters[0].mEndpointId = kTestEndpointId;
    dataVersionFilters[0].mClusterId  = kTestClusterId;
    dataVersionFilters[0].mDataVersion.SetValue(kTestDataVersion1);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mNumAttributeResponse, 0);

        delegate.mNumAttributeResponse = 0;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadRoundtripWithNoMatchPathDataVersionFilter)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadRoundtripWithNoMatchPathDataVersionFilter)
void TestReadInteraction::TestReadRoundtripWithNoMatchPathDataVersionFilter()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    attributePathParams[1].mEndpointId  = kTestEndpointId;
    attributePathParams[1].mClusterId   = kTestClusterId;
    attributePathParams[1].mAttributeId = 2;
    attributePathParams[1].mListIndex   = 1;

    chip::app::DataVersionFilter dataVersionFilters[2];
    dataVersionFilters[0].mEndpointId = kTestEndpointId;
    dataVersionFilters[0].mClusterId  = kInvalidTestClusterId;
    dataVersionFilters[0].mDataVersion.SetValue(kTestDataVersion1);

    dataVersionFilters[1].mEndpointId = kInvalidTestEndpointId;
    dataVersionFilters[1].mClusterId  = kTestClusterId;
    dataVersionFilters[1].mDataVersion.SetValue(kTestDataVersion2);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);
        EXPECT_FALSE(delegate.mReadError);

        delegate.mNumAttributeResponse = 0;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter)
void TestReadInteraction::TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    attributePathParams[1].mEndpointId  = kTestEndpointId;
    attributePathParams[1].mClusterId   = kTestClusterId;
    attributePathParams[1].mAttributeId = 2;
    attributePathParams[1].mListIndex   = 1;

    chip::app::DataVersionFilter dataVersionFilters[2];
    dataVersionFilters[0].mEndpointId = kTestEndpointId;
    dataVersionFilters[0].mClusterId  = kTestClusterId;
    dataVersionFilters[0].mDataVersion.SetValue(kTestDataVersion1);

    dataVersionFilters[1].mEndpointId = kTestEndpointId;
    dataVersionFilters[1].mClusterId  = kTestClusterId;
    dataVersionFilters[1].mDataVersion.SetValue(kTestDataVersion2);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);
        EXPECT_FALSE(delegate.mReadError);

        delegate.mNumAttributeResponse = 0;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadRoundtripWithSameDifferentPathsDataVersionFilter)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadRoundtripWithSameDifferentPathsDataVersionFilter)
void TestReadInteraction::TestReadRoundtripWithSameDifferentPathsDataVersionFilter()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    attributePathParams[1].mEndpointId  = kTestEndpointId;
    attributePathParams[1].mClusterId   = kTestClusterId;
    attributePathParams[1].mAttributeId = 2;
    attributePathParams[1].mListIndex   = 1;

    chip::app::DataVersionFilter dataVersionFilters[2];
    dataVersionFilters[0].mEndpointId = kTestEndpointId;
    dataVersionFilters[0].mClusterId  = kTestClusterId;
    dataVersionFilters[0].mDataVersion.SetValue(kTestDataVersion1);

    dataVersionFilters[1].mEndpointId = kInvalidTestEndpointId;
    dataVersionFilters[1].mClusterId  = kTestClusterId;
    dataVersionFilters[1].mDataVersion.SetValue(kTestDataVersion2);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
        EXPECT_FALSE(delegate.mReadError);

        delegate.mNumAttributeResponse = 0;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadWildcard)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadWildcard)
void TestReadInteraction::TestReadWildcard()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId = chip::Test::kMockEndpoint2;
    attributePathParams[0].mClusterId  = chip::Test::MockClusterId(3);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        // Expected attributes:
        //    - 0xFFFD Cluster revision
        //    - 0xFFFC Feature map
        //    - 0xFFF10001
        //    - 0xFFF10002
        //    - 0xFFF10003
        //    - 0xFFF8 / GeneratedCommandList
        //    - 0xFFF9 / AcceptedCommandList
        //    - 0xFFFB / AttributeList
        EXPECT_EQ(delegate.mNumAttributeResponse, 8);
        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_FALSE(delegate.mReadError);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// TestReadChunking will try to read a few large attributes, the report won't fit into the MTU and result in chunking.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadChunking)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadChunking)
void TestReadInteraction::TestReadChunking()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with kMockAttribute4ListLength large
    // OCTET_STRING elements.
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        delegate.LogCaptures("TestReadChunking:");

        constexpr AttributeCaptureAssertion kExpectedResponses[] = {
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 4),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
        };
        ASSERT_TRUE(delegate.CapturesMatchExactly(chip::Span<const AttributeCaptureAssertion>(kExpectedResponses)));
        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_FALSE(delegate.mReadError);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSetDirtyBetweenChunks)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSetDirtyBetweenChunks)
void TestReadInteraction::TestSetDirtyBetweenChunks()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    chip::app::AttributePathParams attributePathParams[2];
    for (auto & attributePathParam : attributePathParams)
    {
        attributePathParam.mEndpointId  = chip::Test::kMockEndpoint3;
        attributePathParam.mClusterId   = chip::Test::MockClusterId(2);
        attributePathParam.mAttributeId = chip::Test::MockAttributeId(4);
    }

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;

    {
        int currentAttributeResponsesWhenSetDirty = 0;
        int currentArrayItemsWhenSetDirty         = 0;

        class DirtyingMockDelegate : public MockInteractionModelApp
        {
        public:
            DirtyingMockDelegate(AttributePathParams (&aReadPaths)[2], int & aNumAttributeResponsesWhenSetDirty,
                                 int & aNumArrayItemsWhenSetDirty) :
                mReadPaths(aReadPaths),
                mNumAttributeResponsesWhenSetDirty(aNumAttributeResponsesWhenSetDirty),
                mNumArrayItemsWhenSetDirty(aNumArrayItemsWhenSetDirty)
            {}

        private:
            void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & status) override
            {
                MockInteractionModelApp::OnAttributeData(aPath, apData, status);
                if (!mGotStartOfFirstReport && aPath.mEndpointId == mReadPaths[0].mEndpointId &&
                    aPath.mClusterId == mReadPaths[0].mClusterId && aPath.mAttributeId == mReadPaths[0].mAttributeId &&
                    !aPath.IsListItemOperation())
                {
                    mGotStartOfFirstReport = true;
                    return;
                }

                if (!mGotStartOfSecondReport && aPath.mEndpointId == mReadPaths[1].mEndpointId &&
                    aPath.mClusterId == mReadPaths[1].mClusterId && aPath.mAttributeId == mReadPaths[1].mAttributeId &&
                    !aPath.IsListItemOperation())
                {
                    mGotStartOfSecondReport = true;
                    // We always have data chunks, so go ahead to mark things
                    // dirty as needed.
                }

                if (!mGotStartOfSecondReport)
                {
                    // Don't do any setting dirty yet; we are waiting for a data
                    // chunk from the second path.
                    return;
                }

                if (mDidSetDirty)
                {
                    if (!aPath.IsListItemOperation())
                    {
                        mGotPostSetDirtyReport = true;
                        return;
                    }

                    if (!mGotPostSetDirtyReport)
                    {
                        // We're finishing out the message where we decided to
                        // SetDirty.
                        ++mNumAttributeResponsesWhenSetDirty;
                        ++mNumArrayItemsWhenSetDirty;
                    }
                }

                if (!mDidSetDirty)
                {
                    mDidSetDirty = true;

                    AttributePathParams dirtyPath;
                    dirtyPath.mEndpointId  = chip::Test::kMockEndpoint3;
                    dirtyPath.mClusterId   = chip::Test::MockClusterId(2);
                    dirtyPath.mAttributeId = chip::Test::MockAttributeId(4);

                    if (aPath.mEndpointId == dirtyPath.mEndpointId && aPath.mClusterId == dirtyPath.mClusterId &&
                        aPath.mAttributeId == dirtyPath.mAttributeId)
                    {
                        // At this time, we are in the middle of report for second item.
                        mNumAttributeResponsesWhenSetDirty = mNumAttributeResponse;
                        mNumArrayItemsWhenSetDirty         = mNumArrayItems;
                        InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath);
                    }
                }
            }

            // Whether we got the start of the report for our first path.
            bool mGotStartOfFirstReport = false;
            // Whether we got the start of the report for our second path.
            bool mGotStartOfSecondReport = false;
            // Whether we got a new non-list-item report after we set dirty.
            bool mGotPostSetDirtyReport = false;
            bool mDidSetDirty           = false;
            AttributePathParams (&mReadPaths)[2];
            int & mNumAttributeResponsesWhenSetDirty;
            int & mNumArrayItemsWhenSetDirty;
        };

        DirtyingMockDelegate delegate(attributePathParams, currentAttributeResponsesWhenSetDirty, currentArrayItemsWhenSetDirty);
        EXPECT_FALSE(delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        // Our list has length kMockAttribute4ListLength.  Since the underlying
        // path iterator should be reset to the beginning of the cluster it is
        // currently iterating, we expect to get another value for our
        // attribute.  The way the packet boundaries happen to fall, that value
        // will encode 4 items in the first IB and then one IB per item.
        const int expectedIBs = 1 + (kMockAttribute4ListLength - 4);
        ChipLogError(DataManagement, "OLD: %d\n", currentAttributeResponsesWhenSetDirty);
        ChipLogError(DataManagement, "NEW: %d\n", delegate.mNumAttributeResponse);
        EXPECT_EQ(delegate.mNumAttributeResponse, currentAttributeResponsesWhenSetDirty + expectedIBs);
        EXPECT_EQ(delegate.mNumArrayItems, currentArrayItemsWhenSetDirty + kMockAttribute4ListLength);
        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_FALSE(delegate.mReadError);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadInvalidAttributePathRoundtrip)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadInvalidAttributePathRoundtrip)
void TestReadInteraction::TestReadInvalidAttributePathRoundtrip()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kInvalidTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    }

    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestProcessSubscribeRequest)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestProcessSubscribeRequest)
void TestReadInteraction::TestProcessSubscribeRequest()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        EXPECT_EQ(subscribeRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(2);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(3);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);
    }

    engine->Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
/**
 * @brief Test validates that an ICD will choose its IdleModeDuration (GetPublisherSelectedIntervalLimit)
 *        as MaxInterval when the MaxIntervalCeiling is superior.
 */
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestICDProcessSubscribeRequestSupMaxIntervalCeiling)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestICDProcessSubscribeRequestSupMaxIntervalCeiling)
void TestReadInteraction::TestICDProcessSubscribeRequestSupMaxIntervalCeiling()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    uint16_t kMinInterval        = 0;
    uint16_t kMaxIntervalCeiling = 1;

    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        EXPECT_EQ(subscribeRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t idleModeDuration = readHandler.GetPublisherSelectedIntervalLimit();

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        EXPECT_EQ(minInterval, kMinInterval);
        EXPECT_EQ(maxInterval, idleModeDuration);
        EXPECT_EQ(kMaxIntervalCeiling, readHandler.GetSubscriberRequestedMaxInterval());
    }
    engine->Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

/**
 * @brief Test validates that an ICD will choose its IdleModeDuration (GetPublisherSelectedIntervalLimit)
 *        as MaxInterval when the MaxIntervalCeiling is inferior.
 */
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestICDProcessSubscribeRequestInfMaxIntervalCeiling)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestICDProcessSubscribeRequestInfMaxIntervalCeiling)
void TestReadInteraction::TestICDProcessSubscribeRequestInfMaxIntervalCeiling()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    uint16_t kMinInterval        = 0;
    uint16_t kMaxIntervalCeiling = 1;

    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        EXPECT_EQ(subscribeRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t idleModeDuration = readHandler.GetPublisherSelectedIntervalLimit();

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        EXPECT_EQ(minInterval, kMinInterval);
        EXPECT_EQ(maxInterval, idleModeDuration);
        EXPECT_EQ(kMaxIntervalCeiling, readHandler.GetSubscriberRequestedMaxInterval());
    }
    engine->Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

/**
 * @brief Test validates that an ICD will choose a multiple of its IdleModeDuration (GetPublisherSelectedIntervalLimit)
 *        as MaxInterval when the MinInterval > IdleModeDuration.
 */
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestICDProcessSubscribeRequestSupMinInterval)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestICDProcessSubscribeRequestSupMinInterval)
void TestReadInteraction::TestICDProcessSubscribeRequestSupMinInterval()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    uint16_t kMinInterval        = 305; // Default IdleModeDuration is 300
    uint16_t kMaxIntervalCeiling = 605;

    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        EXPECT_EQ(subscribeRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t idleModeDuration = readHandler.GetPublisherSelectedIntervalLimit();

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        EXPECT_EQ(minInterval, kMinInterval);
        EXPECT_EQ(maxInterval, (2 * idleModeDuration));
        EXPECT_EQ(kMaxIntervalCeiling, readHandler.GetSubscriberRequestedMaxInterval());
    }
    engine->Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

/**
 * @brief Test validates that an ICD will choose a maximal value for an uint16 if the multiple of the IdleModeDuration
 *        is greater than variable size.
 */
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestICDProcessSubscribeRequestMaxMinInterval)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestICDProcessSubscribeRequestMaxMinInterval)
void TestReadInteraction::TestICDProcessSubscribeRequestMaxMinInterval()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    uint16_t kMinInterval        = System::Clock::Seconds16::max().count();
    uint16_t kMaxIntervalCeiling = System::Clock::Seconds16::max().count();

    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        EXPECT_EQ(subscribeRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        EXPECT_EQ(minInterval, kMinInterval);
        EXPECT_EQ(maxInterval, kMaxIntervalCeiling);
        EXPECT_EQ(kMaxIntervalCeiling, readHandler.GetSubscriberRequestedMaxInterval());
    }
    engine->Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

/**
 * @brief Test validates that an ICD will choose the MaxIntervalCeiling as MaxInterval if the next multiple after the MinInterval
 *        is greater than the IdleModeDuration and MaxIntervalCeiling
 */
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestICDProcessSubscribeRequestInvalidIdleModeDuration)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestICDProcessSubscribeRequestInvalidIdleModeDuration)
void TestReadInteraction::TestICDProcessSubscribeRequestInvalidIdleModeDuration()
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    uint16_t kMinInterval        = 400;
    uint16_t kMaxIntervalCeiling = 400;

    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        EXPECT_EQ(subscribeRequestBuilder.Init(&writer), CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        EXPECT_EQ(minInterval, kMinInterval);
        EXPECT_EQ(maxInterval, kMaxIntervalCeiling);
        EXPECT_EQ(kMaxIntervalCeiling, readHandler.GetSubscriberRequestedMaxInterval());
    }
    engine->Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeRoundtrip)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeRoundtrip)
void TestReadInteraction::TestSubscribeRoundtrip()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();

    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 1;
    readPrepareParams.mMaxIntervalCeilingSeconds = 2;
    printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
    }

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        delegate.mGotReport = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);

        GenerateEvents();
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = kTestClusterId;
        dirtyPath1.mEndpointId  = kTestEndpointId;
        dirtyPath1.mAttributeId = 1;

        chip::app::AttributePathParams dirtyPath2;
        dirtyPath2.mClusterId   = kTestClusterId;
        dirtyPath2.mEndpointId  = kTestEndpointId;
        dirtyPath2.mAttributeId = 2;

        chip::app::AttributePathParams dirtyPath3;
        dirtyPath3.mClusterId   = kTestClusterId;
        dirtyPath3.mEndpointId  = kTestEndpointId;
        dirtyPath3.mAttributeId = 2;
        dirtyPath3.mListIndex   = 1;

        chip::app::AttributePathParams dirtyPath4;
        dirtyPath4.mClusterId   = kTestClusterId;
        dirtyPath4.mEndpointId  = kTestEndpointId;
        dirtyPath4.mAttributeId = 3;

        chip::app::AttributePathParams dirtyPath5;
        dirtyPath5.mClusterId   = kTestClusterId;
        dirtyPath5.mEndpointId  = kTestEndpointId;
        dirtyPath5.mAttributeId = 4;

        // Test report with 2 different path

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mGotEventResponse     = false;
        delegate.mNumAttributeResponse = 0;

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath2), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_TRUE(delegate.mGotEventResponse);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);

        // Test report with 2 different path, and 1 same path
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath2), CHIP_NO_ERROR);
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath2), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);

        // Test report with 3 different path, and one path is overlapped with another
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath2), CHIP_NO_ERROR);
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath3), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);

        // Test report with 3 different path, all are not overlapped, one path is not interested for current subscription
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath2), CHIP_NO_ERROR);
        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath4), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);

        uint16_t minInterval;
        uint16_t maxInterval;
        delegate.mpReadHandler->GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(readPrepareParams.mMaxIntervalCeilingSeconds, delegate.mpReadHandler->GetSubscriberRequestedMaxInterval());

        // Test empty report
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(maxInterval));
        GetIOContext().DriveIO();

        EXPECT_TRUE(engine->GetReportingEngine().IsRunScheduled());
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;

        DrainAndServiceIO();

        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeEarlyReport)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeEarlyReport)
void TestReadInteraction::TestSubscribeEarlyReport()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[1];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;

    readPrepareParams.mEventPathParamsListSize = 1;

    readPrepareParams.mpAttributePathParamsList    = nullptr;
    readPrepareParams.mAttributePathParamsListSize = 0;

    readPrepareParams.mMinIntervalFloorSeconds   = 1;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    readPrepareParams.mKeepSubscriptions = true;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        readPrepareParams.mpEventPathParamsList[0].mIsUrgentEvent = true;
        delegate.mGotEventResponse                                = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        System::Clock::Timestamp startTime = gMockClock.GetMonotonicTimestamp();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        uint16_t minInterval;
        uint16_t maxInterval;
        delegate.mpReadHandler->GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(readPrepareParams.mMaxIntervalCeilingSeconds, delegate.mpReadHandler->GetSubscriberRequestedMaxInterval());

        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);

        EXPECT_EQ(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler),
                  gMockClock.GetMonotonicTimestamp() + Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        EXPECT_EQ(gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler),
                  gMockClock.GetMonotonicTimestamp() + Seconds16(maxInterval));

        // Confirm that the node is scheduled to run
        EXPECT_TRUE(gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        ReportScheduler::ReadHandlerNode * node = gReportScheduler->GetReadHandlerNode(delegate.mpReadHandler);
        ASSERT_NE(node, nullptr);

        GenerateEvents();

        // modify the node's min timestamp to be 50ms later than the timer expiration time
        node->SetIntervalTimeStamps(delegate.mpReadHandler, startTime + Milliseconds32(50));
        EXPECT_EQ(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler),
                  gMockClock.GetMonotonicTimestamp() + Seconds16(readPrepareParams.mMinIntervalFloorSeconds) + Milliseconds32(50));

        EXPECT_GT(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler), startTime);
        EXPECT_TRUE(delegate.mpReadHandler->IsDirty());
        delegate.mGotEventResponse = false;

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        EXPECT_FALSE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
        // Service Timer expired event
        GetIOContext().DriveIO();

        EXPECT_GT(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler), gMockClock.GetMonotonicTimestamp());

        // The behavior on min interval elapse is different between synced subscription and non-synced subscription
        if (!sUsingSubSync)
        {
            // Verify the ReadHandler is considered as reportable even if its node's min timestamp has not expired
            EXPECT_TRUE(gReportScheduler->IsReportableNow(delegate.mpReadHandler));
            EXPECT_TRUE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Service Engine Run
            GetIOContext().DriveIO();
            // Service EventManagement event
            GetIOContext().DriveIO();
            GetIOContext().DriveIO();
            EXPECT_TRUE(delegate.mGotEventResponse);
        }
        else
        {
            // Verify logic on Min for synced subscription
            // Verify the ReadHandler is not considered as reportable yet
            EXPECT_FALSE(gReportScheduler->IsReportableNow(delegate.mpReadHandler));

            // confirm that the timer was kicked off for the next min of the node
            EXPECT_TRUE(gReportScheduler->IsReportScheduled(delegate.mpReadHandler));

            // Expired new timer
            gMockClock.AdvanceMonotonic(Milliseconds32(50));

            // Service Timer expired event
            GetIOContext().DriveIO();
            EXPECT_TRUE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Service Engine Run
            GetIOContext().DriveIO();
            // Service EventManagement event
            GetIOContext().DriveIO();
            GetIOContext().DriveIO();
            EXPECT_TRUE(delegate.mGotEventResponse);
        }

        // The behavior is identical on max since the sync subscription will interpret an early max firing as a earlier node got
        // reportable and allow nodes that have passed their min to sync on it.
        EXPECT_FALSE(delegate.mpReadHandler->IsDirty());
        delegate.mGotEventResponse = false;
        EXPECT_EQ(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler),
                  gMockClock.GetMonotonicTimestamp() + Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        EXPECT_EQ(gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler),
                  gMockClock.GetMonotonicTimestamp() + Seconds16(maxInterval));

        // Confirm that the node is scheduled to run
        EXPECT_TRUE(gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        ASSERT_NE(node, nullptr);

        // modify the node's max timestamp to be 50ms later than the timer expiration time
        node->SetIntervalTimeStamps(delegate.mpReadHandler, gMockClock.GetMonotonicTimestamp() + Milliseconds32(50));
        EXPECT_EQ(gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler),
                  gMockClock.GetMonotonicTimestamp() + Seconds16(maxInterval) + Milliseconds32(50));

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(Seconds16(maxInterval));

        EXPECT_FALSE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
        // Service Timer expired event
        GetIOContext().DriveIO();

        // Verify the ReadHandler is considered as reportable even if its node's min timestamp has not expired
        EXPECT_GT(gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler), gMockClock.GetMonotonicTimestamp());
        EXPECT_TRUE(gReportScheduler->IsReportableNow(delegate.mpReadHandler));
        EXPECT_FALSE(gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        EXPECT_FALSE(delegate.mpReadHandler->IsDirty());
        EXPECT_TRUE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
        // Service Engine Run
        GetIOContext().DriveIO();
        // Service EventManagement event
        GetIOContext().DriveIO();
        GetIOContext().DriveIO();
        EXPECT_TRUE(gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        EXPECT_FALSE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
    }
    DrainAndServiceIO();

    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeUrgentWildcardEvent)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeUrgentWildcardEvent)
void TestReadInteraction::TestSubscribeUrgentWildcardEvent()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    MockInteractionModelApp nonUrgentDelegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);
    EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    readPrepareParams.mpAttributePathParamsList    = nullptr;
    readPrepareParams.mAttributePathParamsListSize = 0;

    readPrepareParams.mMinIntervalFloorSeconds   = 1;
    readPrepareParams.mMaxIntervalCeilingSeconds = 3600;
    printf("\nSend first subscribe request message with wildcard urgent event to Node: 0x" ChipLogFormatX64 "\n",
           ChipLogValueX64(chip::kTestDeviceNodeId));

    readPrepareParams.mKeepSubscriptions = true;

    {
        app::ReadClient nonUrgentReadClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(),
                                            nonUrgentDelegate, chip::app::ReadClient::InteractionType::Subscribe);
        nonUrgentDelegate.mGotReport = false;
        EXPECT_EQ(nonUrgentReadClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        readPrepareParams.mpEventPathParamsList[0].mIsUrgentEvent = true;
        delegate.mGotReport                                       = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        System::Clock::Timestamp startTime = gMockClock.GetMonotonicTimestamp();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 2u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        nonUrgentDelegate.mpReadHandler = engine->ActiveHandlerAt(0);
        ASSERT_NE(engine->ActiveHandlerAt(1), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(1);

        // Adding These to be able to access private members/methods of ReadHandler

        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 2u);

        GenerateEvents();

        EXPECT_GT(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler), startTime);
        EXPECT_TRUE(delegate.mpReadHandler->IsDirty());
        delegate.mGotEventResponse = false;
        delegate.mGotReport        = false;

        EXPECT_GT(gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler), startTime);
        EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->IsDirty());
        nonUrgentDelegate.mGotEventResponse = false;
        nonUrgentDelegate.mGotReport        = false;

        // wait for min interval 1 seconds (in test, we use 0.6 seconds considering the time variation), expect no event is
        // received, then wait for 0.8 seconds, then the urgent event would be sent out
        //  currently DriveIOUntil will call `DriveIO` at least once, which means that if there is any CPU scheduling issues,
        // there's a chance 1.9s will already have elapsed by the time we get there, which will result in DriveIO being called when
        // it shouldn't. Better fix could happen inside DriveIOUntil, not sure the sideeffect there.

        // Advance monotonic looping to allow events to trigger
        gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(600));
        GetIOContext().DriveIO();

        EXPECT_FALSE(delegate.mGotEventResponse);
        EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(800));

        // Service Timer expired event
        GetIOContext().DriveIO();

        // Service Engine Run
        GetIOContext().DriveIO();

        // Service EventManagement event
        GetIOContext().DriveIO();

        EXPECT_TRUE(delegate.mGotEventResponse);

        // The logic differs here depending on what Scheduler is implemented
        if (!sUsingSubSync)
        {
            EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);

            // Since we just sent a report for our urgent subscription, the min interval of the urgent subcription should have been
            // updated
            EXPECT_GT(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler), gMockClock.GetMonotonicTimestamp());

            EXPECT_FALSE(delegate.mpReadHandler->IsDirty());
            delegate.mGotEventResponse = false;

            // For our non-urgent subscription, we did not send anything, so the min interval should of the non urgent subcription
            // should be in the past
            EXPECT_LT(gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler),
                      gMockClock.GetMonotonicTimestamp());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->IsDirty());

            // Advance monotonic timestamp for min interval to elapse
            gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(2100));
            GetIOContext().DriveIO();

            // No reporting should have happened.
            EXPECT_FALSE(delegate.mGotEventResponse);
            EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);

            // The min-interval should have elapsed for the urgent subscription, and our handler should still
            // not be dirty or reportable.
            EXPECT_LT(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler),
                      System::SystemClock().GetMonotonicTimestamp());
            EXPECT_FALSE(delegate.mpReadHandler->IsDirty());
            EXPECT_FALSE(delegate.mpReadHandler->ShouldStartReporting());

            // And the non-urgent one should not have changed state either, since
            // it's waiting for the max-interval.
            EXPECT_LT(gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler),
                      System::SystemClock().GetMonotonicTimestamp());
            EXPECT_GT(gReportScheduler->GetMaxTimestampForHandler(nonUrgentDelegate.mpReadHandler),
                      System::SystemClock().GetMonotonicTimestamp());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->IsDirty());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // There should be no reporting run scheduled.  This is very important;
            // otherwise we can get a false-positive pass below because the run was
            // already scheduled by here.
            EXPECT_FALSE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Generate some events, which should get reported.
            GenerateEvents();

            // Urgent read handler should now be dirty, and reportable.
            EXPECT_TRUE(delegate.mpReadHandler->IsDirty());
            EXPECT_TRUE(delegate.mpReadHandler->ShouldStartReporting());
            EXPECT_TRUE(gReportScheduler->IsReadHandlerReportable(delegate.mpReadHandler));

            // Non-urgent read handler should not be reportable.
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->IsDirty());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // Still no reporting should have happened.
            EXPECT_FALSE(delegate.mGotEventResponse);
            EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);

            DrainAndServiceIO();

            // Should get those urgent events reported.
            EXPECT_TRUE(delegate.mGotEventResponse);

            // Should get nothing reported on the non-urgent handler.
            EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);
        }
        else
        {
            // If we're using the sub-sync scheduler, we should have gotten the non-urgent event as well.
            EXPECT_TRUE(nonUrgentDelegate.mGotEventResponse);

            // Since we just sent a report for both our subscriptions, the min interval of the urgent subcription should have been
            EXPECT_GT(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler), gMockClock.GetMonotonicTimestamp());

            EXPECT_GT(gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler),
                      gMockClock.GetMonotonicTimestamp());

            EXPECT_FALSE(delegate.mpReadHandler->IsDirty());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->IsDirty());
            delegate.mGotEventResponse          = false;
            nonUrgentDelegate.mGotEventResponse = false;

            // Advance monotonic timestamp for min interval to elapse
            gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(2100));
            GetIOContext().DriveIO();

            // No reporting should have happened.
            EXPECT_FALSE(delegate.mGotEventResponse);
            EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);

            // The min-interval should have elapsed for both subscriptions, and our handlers should still
            // not be dirty or reportable.
            EXPECT_LT(gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler),
                      System::SystemClock().GetMonotonicTimestamp());
            EXPECT_LT(gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler),
                      System::SystemClock().GetMonotonicTimestamp());
            EXPECT_FALSE(delegate.mpReadHandler->IsDirty());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->IsDirty());
            EXPECT_FALSE(delegate.mpReadHandler->ShouldStartReporting());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // There should be no reporting run scheduled.  This is very important;
            // otherwise we can get a false-positive pass below because the run was
            // already scheduled by here.
            EXPECT_FALSE(InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Generate some events, which should get reported.
            GenerateEvents();

            // Urgent read handler should now be dirty, and reportable.
            EXPECT_TRUE(delegate.mpReadHandler->IsDirty());
            EXPECT_TRUE(delegate.mpReadHandler->ShouldStartReporting());
            EXPECT_TRUE(gReportScheduler->IsReadHandlerReportable(delegate.mpReadHandler));

            // Non-urgent read handler should not be reportable.
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->IsDirty());
            EXPECT_FALSE(nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // Still no reporting should have happened.
            EXPECT_FALSE(delegate.mGotEventResponse);
            EXPECT_FALSE(nonUrgentDelegate.mGotEventResponse);

            DrainAndServiceIO();

            // Should get those urgent events reported and the non urgent reported for synchronisation
            EXPECT_TRUE(delegate.mGotEventResponse);
            EXPECT_TRUE(nonUrgentDelegate.mGotEventResponse);
        }
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeWildcard)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeWildcard)
void TestReadInteraction::TestSubscribeWildcard()
{
    // This test in particular is completely tied to the DefaultMockConfig in the mock
    // attribute storage, so reset to that (figuring out chunking location is extra hard to
    // maintain)
    chip::Test::ResetMockNodeConfig();

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    readPrepareParams.mAttributePathParamsListSize = 2;
    auto attributePathParams = std::make_unique<chip::app::AttributePathParams[]>(readPrepareParams.mAttributePathParamsListSize);
    // Subscribe to full wildcard paths, repeat twice to ensure chunking.
    readPrepareParams.mpAttributePathParamsList = attributePathParams.get();

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        EXPECT_EQ(readClient.SendAutoResubscribeRequest(std::move(readPrepareParams)), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);

        // Mock attribute storage that is reset and resides in src/app/util/mock/attribute-storage.cpp
        // has the following items:
        // - Endpoint 0xFFFE
        //    - cluster 0xFFF1'FC01 (2 attributes + 3 GlobalNotInMetadata)
        //    - cluster 0xFFF1'FC02 (3 attributes + 3 GlobalNotInMetadata)
        // - Endpoint 0xFFFD
        //    - cluster 0xFFF1'FC01 (2 attributes + 3 GlobalNotInMetadata)
        //    - cluster 0xFFF1'FC02 (4 attributes + 3 GlobalNotInMetadata)
        //    - cluster 0xFFF1'FC03 (5 attributes + 3 GlobalNotInMetadata)
        // - Endpoint 0xFFFC
        //    - cluster 0xFFF1'FC01 (3 attributes + 3 GlobalNotInMetadata)
        //    - cluster 0xFFF1'FC02 (6 attributes + 3 GlobalNotInMetadata)
        //    - cluster 0xFFF1'FC03 (2 attributes + 3 GlobalNotInMetadata)
        //    - cluster 0xFFF1'FC04 (2 attributes + 3 GlobalNotInMetadata)
        //
        //
        // Actual chunk placement is execution defined, however generally
        // Attribute 0xFFFC::0xFFF1'FC02::0xFFF1'0004 (kMockEndpoint3::MockClusterId(2)::MockAttributeId(4))
        // is a list of kMockAttribute4ListLength of size 256 bytes each, which cannot fit
        // in a single packet, so chunking is applied (we get a list and then individual elements as
        // single items)
        //
        // The assertions below expect a specific order verified as ok (updates should verify
        // that the updates make sense)

        delegate.LogCaptures("TestSubscribeWildcard: initial subscription");

        constexpr AttributeCaptureAssertion kExpectedResponses[] = {
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 5),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 6),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 5),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, 0xFFF10002),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 7),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, 0xFFF10002),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, 0xFFF10003),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, AttributeList::Id, /* listSize = */ 8),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 6),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10002),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10003),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 3),
            // Chunking here
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 9),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AttributeList::Id, /* listSize = */ 5),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AttributeList::Id, /* listSize = */ 5),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 5),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFE, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 6),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 5),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, 0xFFF10002),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 7),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, 0xFFF10002),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, 0xFFF10003),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFD, 0xFFF1FC03, AttributeList::Id, /* listSize = */ 8),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 6),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10001),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10002),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10003),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 3),
            // Chunking here
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 9),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AttributeList::Id, /* listSize = */ 5),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, ClusterRevision::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, FeatureMap::Id),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, GeneratedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AcceptedCommandList::Id, /* listSize = */ 0),
            AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AttributeList::Id, /* listSize = */ 5),
        };

        ASSERT_TRUE(delegate.CapturesMatchExactly(chip::Span<const AttributeCaptureAssertion>(kExpectedResponses)));
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        // Set a concrete path dirty
        {
            delegate.mGotReport            = false;
            delegate.mNumAttributeResponse = 0;

            AttributePathParams dirtyPath;
            dirtyPath.mEndpointId  = chip::Test::kMockEndpoint2;
            dirtyPath.mClusterId   = chip::Test::MockClusterId(3);
            dirtyPath.mAttributeId = chip::Test::MockAttributeId(1);

            EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath), CHIP_NO_ERROR);

            DrainAndServiceIO();

            EXPECT_TRUE(delegate.mGotReport);
            // We subscribed wildcard path twice, so we will receive two reports here.
            EXPECT_EQ(delegate.mNumAttributeResponse, 2);
        }

        // Set a endpoint dirty
        ChipLogProgress(NotSpecified, "Testing updates after dirty path setting");
        {
            delegate.Reset();

            AttributePathParams dirtyPath;
            dirtyPath.mEndpointId = chip::Test::kMockEndpoint3;

            EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath), CHIP_NO_ERROR);

            //
            // We need to DrainAndServiceIO() until attribute callback will be called.
            // This is not correct behavior and is tracked in Issue #17528.
            //
            int last;
            do
            {
                last = delegate.mNumAttributeResponse;
                DrainAndServiceIO();
            } while (last != delegate.mNumAttributeResponse);

            // Mock endpoint3 has 13 attributes in total, and we subscribed twice.
            delegate.LogCaptures("TestSubscribeWildcard: after dirty");
            constexpr AttributeCaptureAssertion kExpectedResponsesAfterDirty[] = {
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, 0xFFF10001),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 6),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10001),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10002),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10003),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 3),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 9),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AttributeList::Id, /* listSize = */ 5),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AttributeList::Id, /* listSize = */ 5),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, 0xFFF10001),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC01, AttributeList::Id, /* listSize = */ 6),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10001),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10002),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10003),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 2),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, 0xFFF10004, /* listSize = */ 1),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC02, AttributeList::Id, /* listSize = */ 9),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC03, AttributeList::Id, /* listSize = */ 5),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, ClusterRevision::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, FeatureMap::Id),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, GeneratedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AcceptedCommandList::Id, /* listSize = */ 0),
                AttributeCaptureAssertion(0xFFFC, 0xFFF1FC04, AttributeList::Id, /* listSize = */ 5),

            };

            ASSERT_TRUE(delegate.CapturesMatchExactly(chip::Span<const AttributeCaptureAssertion>(kExpectedResponsesAfterDirty)));
        }
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Subscribe (wildcard, C3, A1), then setDirty (E2, C3, wildcard), receive one attribute after setDirty
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribePartialOverlap)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribePartialOverlap)
void TestReadInteraction::TestSubscribePartialOverlap()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    readPrepareParams.mAttributePathParamsListSize = 1;
    auto attributePathParams = std::make_unique<chip::app::AttributePathParams[]>(readPrepareParams.mAttributePathParamsListSize);
    attributePathParams[0].mClusterId           = chip::Test::MockClusterId(3);
    attributePathParams[0].mAttributeId         = chip::Test::MockAttributeId(1);
    readPrepareParams.mpAttributePathParamsList = attributePathParams.get();

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        EXPECT_EQ(readClient.SendAutoResubscribeRequest(std::move(readPrepareParams)), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);

        EXPECT_EQ(delegate.mNumAttributeResponse, 1);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        // Set a partial overlapped path dirty
        {
            delegate.mGotReport            = false;
            delegate.mNumAttributeResponse = 0;

            AttributePathParams dirtyPath;
            dirtyPath.mEndpointId = chip::Test::kMockEndpoint2;
            dirtyPath.mClusterId  = chip::Test::MockClusterId(3);

            EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath), CHIP_NO_ERROR);

            DrainAndServiceIO();

            EXPECT_TRUE(delegate.mGotReport);
            EXPECT_EQ(delegate.mNumAttributeResponse, 1);
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mEndpointId, chip::Test::kMockEndpoint2);
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mClusterId, chip::Test::MockClusterId(3));
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mAttributeId, chip::Test::MockAttributeId(1));
        }
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Subscribe (E2, C3, A1), then setDirty (wildcard, wildcard, wildcard), receive one attribute after setDirty
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeSetDirtyFullyOverlap)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeSetDirtyFullyOverlap)
void TestReadInteraction::TestSubscribeSetDirtyFullyOverlap()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    readPrepareParams.mAttributePathParamsListSize = 1;
    auto attributePathParams = std::make_unique<chip::app::AttributePathParams[]>(readPrepareParams.mAttributePathParamsListSize);
    attributePathParams[0].mEndpointId          = chip::Test::kMockEndpoint2;
    attributePathParams[0].mClusterId           = chip::Test::MockClusterId(3);
    attributePathParams[0].mAttributeId         = chip::Test::MockAttributeId(1);
    readPrepareParams.mpAttributePathParamsList = attributePathParams.get();

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        EXPECT_EQ(readClient.SendAutoResubscribeRequest(std::move(readPrepareParams)), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);

        EXPECT_EQ(delegate.mNumAttributeResponse, 1);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        // Set a full overlapped path dirty and expect to receive one E2C3A1
        {
            delegate.mGotReport            = false;
            delegate.mNumAttributeResponse = 0;

            AttributePathParams dirtyPath;
            EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath), CHIP_NO_ERROR);

            DrainAndServiceIO();

            EXPECT_TRUE(delegate.mGotReport);
            EXPECT_EQ(delegate.mNumAttributeResponse, 1);
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mEndpointId, chip::Test::kMockEndpoint2);
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mClusterId, chip::Test::MockClusterId(3));
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mAttributeId, chip::Test::MockAttributeId(1));
        }
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Verify that subscription can be shut down just after receiving SUBSCRIBE RESPONSE,
// before receiving any subsequent REPORT DATA.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeEarlyShutdown)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeEarlyShutdown)
void TestReadInteraction::TestSubscribeEarlyShutdown()
{
    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    InteractionModelEngine & engine    = *InteractionModelEngine::GetInstance();
    MockInteractionModelApp delegate;

    // Initialize Interaction Model Engine
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    EXPECT_EQ(engine.Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    // Subscribe to the attribute
    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = kTestEndpointId;
    attributePathParams.mClusterId   = kTestClusterId;
    attributePathParams.mAttributeId = 1;

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = &attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;
    readPrepareParams.mMinIntervalFloorSeconds     = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds   = 5;
    readPrepareParams.mKeepSubscriptions           = false;

    printf("Send subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 1);
        EXPECT_EQ(engine.GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        ASSERT_NE(engine.ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine.ActiveHandlerAt(0);
        ASSERT_NE(delegate.mpReadHandler, nullptr);
    }

    // Cleanup
    EXPECT_EQ(engine.GetNumActiveReadClients(), 0u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    engine.Shutdown();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeInvalidAttributePathRoundtrip)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeInvalidAttributePathRoundtrip)
void TestReadInteraction::TestSubscribeInvalidAttributePathRoundtrip()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();

    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kInvalidTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mSessionHolder.Grab(GetSessionBobToAlice());
    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        delegate.mNumAttributeResponse = 0;

        DrainAndServiceIO();

        EXPECT_EQ(delegate.mNumAttributeResponse, 0);

        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        uint16_t minInterval;
        uint16_t maxInterval;
        delegate.mpReadHandler->GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(readPrepareParams.mMaxIntervalCeilingSeconds, delegate.mpReadHandler->GetSubscriberRequestedMaxInterval());

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(maxInterval));
        GetIOContext().DriveIO();

        EXPECT_TRUE(engine->GetReportingEngine().IsRunScheduled());
        EXPECT_TRUE(engine->GetReportingEngine().IsRunScheduled());

        DrainAndServiceIO();

        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadShutdown)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadShutdown)
void TestReadInteraction::TestReadShutdown()
{
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    app::ReadClient * pClients[4];
    MockInteractionModelApp delegate;

    //
    // Allocate a number of clients
    //
    for (auto & client : pClients)
    {
        client = Platform::New<app::ReadClient>(engine, &GetExchangeManager(), delegate,
                                                chip::app::ReadClient::InteractionType::Subscribe);
    }

    //
    // Delete every other client to ensure we test out
    // deleting clients from the list of clients tracked by the IM
    //
    Platform::Delete(pClients[1]);
    Platform::Delete(pClients[3]);

    //
    // Shutdown the engine first so that we can
    // de-activate the internal list.
    //
    engine->Shutdown();

    //
    // Shutdown the read clients. These should
    // safely destruct without causing any egregious
    // harm
    //
    Platform::Delete(pClients[0]);
    Platform::Delete(pClients[2]);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeInvalidInterval)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeInvalidInterval)
void TestReadInteraction::TestSubscribeInvalidInterval()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mSessionHolder.Grab(GetSessionBobToAlice());
    readPrepareParams.mMinIntervalFloorSeconds   = 6;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_ERROR_INVALID_ARGUMENT);

        printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

        DrainAndServiceIO();
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);

    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestPostSubscribeRoundtripStatusReportTimeout)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestPostSubscribeRoundtripStatusReportTimeout)
void TestReadInteraction::TestPostSubscribeRoundtripStatusReportTimeout()
{
    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();

    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);

        GenerateEvents();
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = kTestClusterId;
        dirtyPath1.mEndpointId  = kTestEndpointId;
        dirtyPath1.mAttributeId = 1;

        chip::app::AttributePathParams dirtyPath2;
        dirtyPath2.mClusterId   = kTestClusterId;
        dirtyPath2.mEndpointId  = kTestEndpointId;
        dirtyPath2.mAttributeId = 2;

        // Test report with 2 different path
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath2), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 2);

        // Wait for max interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMaxIntervalCeilingSeconds));
        GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        ExpireSessionBobToAlice();

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath2), CHIP_NO_ERROR);

        EXPECT_TRUE(engine->GetReportingEngine().IsRunScheduled());

        DrainAndServiceIO();

        ExpireSessionAliceToBob();
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);
        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeRoundtripStatusReportTimeout)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeRoundtripStatusReportTimeout)
void TestReadInteraction::TestSubscribeRoundtripStatusReportTimeout()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        ExpireSessionAliceToBob();

        DrainAndServiceIO();

        ExpireSessionBobToAlice();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);
        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadChunkingStatusReportTimeout)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadChunkingStatusReportTimeout)
void TestReadInteraction::TestReadChunkingStatusReportTimeout()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        ExpireSessionAliceToBob();
        DrainAndServiceIO();
        ExpireSessionBobToAlice();

        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// ReadClient sends the read request, but handler fails to send the one report (SendMessage returns an error).
// Since this is an un-chunked read, we are not in the AwaitingReportResponse state, so the "reports in flight"
// counter should not increase.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadReportFailure)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadReportFailure)
void TestReadInteraction::TestReadReportFailure()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint2;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(3);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(1);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        GetLoopback().mNumMessagesToAllowBeforeError = 1;
        GetLoopback().mMessageSendError              = CHIP_ERROR_INCORRECT_STATE;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);

        GetLoopback().mNumMessagesToAllowBeforeError = 0;
        GetLoopback().mMessageSendError              = CHIP_NO_ERROR;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeRoundtripChunkStatusReportTimeout)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeRoundtripChunkStatusReportTimeout)
void TestReadInteraction::TestSubscribeRoundtripChunkStatusReportTimeout()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        ExpireSessionAliceToBob();
        DrainAndServiceIO();
        ExpireSessionBobToAlice();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);
        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestPostSubscribeRoundtripChunkStatusReportTimeout)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestPostSubscribeRoundtripChunkStatusReportTimeout)
void TestReadInteraction::TestPostSubscribeRoundtripChunkStatusReportTimeout()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);

        GenerateEvents();
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = chip::Test::MockClusterId(2);
        dirtyPath1.mEndpointId  = chip::Test::kMockEndpoint3;
        dirtyPath1.mAttributeId = chip::Test::MockAttributeId(4);

        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMaxIntervalCeilingSeconds));
        GetIOContext().DriveIO();

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        GetLoopback().mDroppedMessageCount              = 0;

        DrainAndServiceIO();
        // Drop status report for the first chunked report, then expire session, handler would be timeout
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 1u);
        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);

        ExpireSessionAliceToBob();
        ExpireSessionBobToAlice();
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestPostSubscribeRoundtripChunkReportTimeout)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestPostSubscribeRoundtripChunkReportTimeout)
void TestReadInteraction::TestPostSubscribeRoundtripChunkReportTimeout()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);

        GenerateEvents();
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = chip::Test::MockClusterId(2);
        dirtyPath1.mEndpointId  = chip::Test::kMockEndpoint3;
        dirtyPath1.mAttributeId = chip::Test::MockAttributeId(4);

        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMaxIntervalCeilingSeconds));
        GetIOContext().DriveIO();

        EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath1), CHIP_NO_ERROR);

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;

        // Drop second chunked report then expire session, client would be timeout
        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 2;
        GetLoopback().mDroppedMessageCount              = 0;

        DrainAndServiceIO();
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 1u);
        EXPECT_EQ(GetLoopback().mSentMessageCount, 3u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

        ExpireSessionAliceToBob();
        ExpireSessionBobToAlice();
        EXPECT_EQ(delegate.mError, CHIP_ERROR_TIMEOUT);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestPostSubscribeRoundtripChunkReport)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestPostSubscribeRoundtripChunkReport)
void TestReadInteraction::TestPostSubscribeRoundtripChunkReport()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEventEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestEventClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 1;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);

        GenerateEvents();
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = chip::Test::MockClusterId(2);
        dirtyPath1.mEndpointId  = chip::Test::kMockEndpoint3;
        dirtyPath1.mAttributeId = chip::Test::MockAttributeId(4);

        engine->GetReportingEngine().SetDirty(dirtyPath1);
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        delegate.mNumArrayItems        = 0;

        // wait for min interval 1 seconds(in test, we use 0.9second considering the time variation), expect no event is
        // received, then wait for 0.5 seconds, then all chunked dirty reports are sent out, which would not honor minInterval
        gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(900));
        GetIOContext().DriveIO();

        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
        System::Clock::Timestamp startTime = gMockClock.GetMonotonicTimestamp();

        // Increment in time is done by steps here to allow for multiple IO processing at the right time and allow the timer to
        // be rescheduled accordingly
        while (true)
        {
            GetIOContext().DriveIO();
            if ((gMockClock.GetMonotonicTimestamp() - startTime) >= System::Clock::Milliseconds32(500))
            {
                break;
            }
            gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(10));
        }
    }
    // We get one chunk with 4 array elements, and then one chunk per
    // element, and the total size of the array is
    // kMockAttribute4ListLength.
    EXPECT_EQ(delegate.mNumAttributeResponse, 1 + (kMockAttribute4ListLength - 4));
    EXPECT_EQ(delegate.mNumArrayItems, 6);

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
}

namespace {

void CheckForInvalidAction(Test::MessageCapturer & messageLog)
{
    EXPECT_EQ(messageLog.MessageCount(), 1u);
    EXPECT_TRUE(messageLog.IsMessageType(0, Protocols::InteractionModel::MsgType::StatusResponse));
    CHIP_ERROR status;
    EXPECT_EQ(StatusResponse::ProcessStatusResponse(std::move(messageLog.MessagePayload(0)), status), CHIP_NO_ERROR);
    EXPECT_EQ(status, CHIP_IM_GLOBAL_STATUS(InvalidAction));
}

} // anonymous namespace

// Read Client sends the read request, Read Handler drops the response, then test injects unknown status reponse message for
// Read Client.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientReceiveInvalidMessage)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientReceiveInvalidMessage)
void TestReadInteraction::TestReadClientReceiveInvalidMessage()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    {
        app::ReadClient readClient(engine, &GetExchangeManager(), delegate, chip::app::ReadClient::InteractionType::Read);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        StatusResponseMessage::Builder response;
        response.Init(&writer);
        response.Status(Protocols::InteractionModel::Status::Busy);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

        chip::Test::MessageCapturer messageLog(*this);
        messageLog.mCaptureStandaloneAcks = false;

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;
        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));

        DrainAndServiceIO();

        // The ReadHandler closed its exchange when it sent the Report Data (which we dropped).
        // Since we synthesized the StatusResponse to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(Busy));

        CheckForInvalidAction(messageLog);
    }

    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client sends the subscribe request, Read Handler drops the response, then test injects unknown status response message
// for Read Client.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeClientReceiveInvalidStatusResponse)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeClientReceiveInvalidStatusResponse)
void TestReadInteraction::TestSubscribeClientReceiveInvalidStatusResponse()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        StatusResponseMessage::Builder response;
        response.Init(&writer);
        response.Status(Protocols::InteractionModel::Status::Busy);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is closed when we synthesize the subscribe response, since it sent the
        // Subscribe Response as the last message in the transaction.
        // Since we synthesized the subscribe response to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(Busy));
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client sends the subscribe request, Read Handler drops the response, then test injects well-formed status response
// message with Success for Read Client, we expect the error with CHIP_ERROR_INVALID_MESSAGE_TYPE
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeClientReceiveWellFormedStatusResponse)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeClientReceiveWellFormedStatusResponse)
void TestReadInteraction::TestSubscribeClientReceiveWellFormedStatusResponse()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        StatusResponseMessage::Builder response;
        response.Init(&writer);
        response.Status(Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the StatusResponse.
        // Since we synthesized the StatusResponse to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

        EXPECT_EQ(delegate.mError, CHIP_ERROR_INVALID_MESSAGE_TYPE);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client sends the subscribe request, Read Handler drops the response, then test injects invalid report message for Read
// Client.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeClientReceiveInvalidReportMessage)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeClientReceiveInvalidReportMessage)
void TestReadInteraction::TestSubscribeClientReceiveInvalidReportMessage()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the ReportData.
        // Since we synthesized the ReportData to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

        EXPECT_EQ(delegate.mError, CHIP_ERROR_END_OF_TLV);

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client create the subscription, handler sends unsolicited malformed report to client,
// InteractionModelEngine::OnUnsolicitedReportData would process this malformed report and sends out status report
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeClientReceiveUnsolicitedInvalidReportMessage)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeClientReceiveUnsolicitedInvalidReportMessage)
void TestReadInteraction::TestSubscribeClientReceiveUnsolicitedInvalidReportMessage()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        GetLoopback().mSentMessageCount = 0;
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);
        DrainAndServiceIO();
        EXPECT_EQ(GetLoopback().mSentMessageCount, 5u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

        GetLoopback().mSentMessageCount = 0;
        auto exchange                   = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(
            delegate.mpReadHandler->mSessionHandle.Get().Value(), delegate.mpReadHandler);

        delegate.mpReadHandler->mExchangeCtx.Grab(exchange);

        EXPECT_EQ(delegate.mpReadHandler->mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData,
                                                                    std::move(msgBuf),
                                                                    Messaging::SendMessageFlags::kExpectResponse),
                  CHIP_NO_ERROR);
        DrainAndServiceIO();

        // The server sends a data report.
        // The client receives the data report data and sends out status report with invalid action.
        // The server acks the status report.
        EXPECT_EQ(GetLoopback().mSentMessageCount, 3u);
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
}

// Read Client sends the subscribe request, Read Handler drops the subscribe response, then test injects invalid subscribe
// response message
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeClientReceiveInvalidSubscribeResponseMessage)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeClientReceiveInvalidSubscribeResponseMessage)
void TestReadInteraction::TestSubscribeClientReceiveInvalidSubscribeResponseMessage()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 3;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        SubscribeResponseMessage::Builder response;
        response.Init(&writer);
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        response.MaxInterval(1);
        response.EndOfSubscribeResponseMessage();
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::SubscribeResponse);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        EXPECT_EQ(GetLoopback().mSentMessageCount, 4u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the subscribe response.
        // Since we synthesized the subscribe response to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

        EXPECT_EQ(delegate.mError, CHIP_ERROR_INVALID_SUBSCRIPTION);
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client create the subscription, handler sends unsolicited malformed report with invalid subscription id to client,
// InteractionModelEngine::OnUnsolicitedReportData would process this malformed report and sends out status report
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId)
void TestReadInteraction::TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mpAttributePathParamsList[1].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mAttributeId = 2;

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        GetLoopback().mSentMessageCount = 0;
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);
        DrainAndServiceIO();
        EXPECT_EQ(GetLoopback().mSentMessageCount, 5u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        response.EndOfReportDataMessage();

        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

        GetLoopback().mSentMessageCount = 0;
        auto exchange                   = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(
            delegate.mpReadHandler->mSessionHandle.Get().Value(), delegate.mpReadHandler);

        delegate.mpReadHandler->mExchangeCtx.Grab(exchange);

        EXPECT_EQ(delegate.mpReadHandler->mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData,
                                                                    std::move(msgBuf),
                                                                    Messaging::SendMessageFlags::kExpectResponse),
                  CHIP_NO_ERROR);
        DrainAndServiceIO();

        // The server sends a data report.
        // The client receives the data report data and sends out status report with invalid subsciption.
        // The server should respond with a status report of its own, leading to 4 messages (because
        // the client would ack the server's status report), just sends an ack to the status report it got.
        EXPECT_EQ(GetLoopback().mSentMessageCount, 3u);
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
}

// TestReadChunkingInvalidSubscriptionId will try to read a few large attributes, the report won't fit into the MTU and result
// in chunking, second report has different subscription id from the first one, read client sends out the status report with
// invalid subscription
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadChunkingInvalidSubscriptionId)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadChunkingInvalidSubscriptionId)
void TestReadInteraction::TestReadChunkingInvalidSubscriptionId()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 3;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        EXPECT_FALSE(msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        response.EndOfReportDataMessage();
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);

        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        EXPECT_EQ(GetLoopback().mSentMessageCount, 4u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the report data message.
        // Since we synthesized the second report data message to the ReadClient with invalid subscription id, instead of
        // sending it from the ReadHandler, the only messages here are the ReadClient's StatusResponse to the unexpected message
        // and an MRP ack.
        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

        EXPECT_EQ(delegate.mError, CHIP_ERROR_INVALID_SUBSCRIPTION);
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client sends a malformed subscribe request, interaction model engine fails to parse the request and generates a status
// report to client, and client is closed.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadHandlerMalformedSubscribeRequest)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadHandlerMalformedSubscribeRequest)
void TestReadInteraction::TestReadHandlerMalformedSubscribeRequest()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);

        EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        ASSERT_NE(exchange, nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);

        EXPECT_EQ(readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                    Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)),
                  CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Read Client sends a malformed read request, interaction model engine fails to parse the request and generates a status report
// to client, and client is closed.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadHandlerMalformedReadRequest1)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadHandlerMalformedReadRequest1)
void TestReadInteraction::TestReadHandlerMalformedReadRequest1()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);

        EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        ASSERT_NE(exchange, nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);
        EXPECT_EQ(readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                    Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)),
                  CHIP_NO_ERROR);
        DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Read Client sends a malformed read request, read handler fails to parse the request and generates a status report to client,
// and client is closed.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadHandlerMalformedReadRequest2)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadHandlerMalformedReadRequest2)
void TestReadInteraction::TestReadHandlerMalformedReadRequest2()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);
        EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
        EXPECT_EQ(request.EndOfReadRequestMessage(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        ASSERT_NE(exchange, nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);
        EXPECT_EQ(readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                    Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)),
                  CHIP_NO_ERROR);
        DrainAndServiceIO();
        ChipLogError(DataManagement, "The error is %s", ErrorStr(delegate.mError));
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Read Client creates a subscription with the server, server sends chunked reports, after the handler sends out the first
// chunked report, client sends out invalid write request message, handler sends status report with invalid action and closes
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeSendUnknownMessage)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeSendUnknownMessage)
void TestReadInteraction::TestSubscribeSendUnknownMessage()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);

        GetLoopback().mSentMessageCount = 0;

        // Server sends out status report, client should send status report along with Piggybacking ack, but we don't do that
        // Instead, we send out unknown message to server

        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;
        chip::app::InitWriterWithSpaceReserved(writer, 0);
        request.Init(&writer);
        writer.Finalize(&msgBuf);

        readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::WriteRequest, std::move(msgBuf));
        DrainAndServiceIO();
        // client sends invalid write request, server sends out status report with invalid action and closes, client replies
        // with status report server replies with MRP Ack
        EXPECT_EQ(GetLoopback().mSentMessageCount, 4u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client creates a subscription with the server, server sends chunked reports, after the handler sends out invalid status
// report, client sends out invalid status report message, handler sends status report with invalid action and close
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeSendInvalidStatusReport)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeSendInvalidStatusReport)
void TestReadInteraction::TestSubscribeSendInvalidStatusReport()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 1;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        GetLoopback().mDroppedMessageCount              = 0;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);
        DrainAndServiceIO();

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
        EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
        GetLoopback().mSentMessageCount = 0;

        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);

        System::PacketBufferHandle msgBuf;
        StatusResponseMessage::Builder request;
        System::PacketBufferTLVWriter writer;
        chip::app::InitWriterWithSpaceReserved(writer, 0);
        request.Init(&writer);
        writer.Finalize(&msgBuf);

        readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::StatusResponse, std::move(msgBuf));
        DrainAndServiceIO();

        // client sends malformed status response, server sends out status report with invalid action and close, client replies
        // with status report server replies with MRP Ack
        EXPECT_EQ(GetLoopback().mSentMessageCount, 4u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 0u);
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Read Client sends a malformed subscribe request, the server fails to parse the request and generates a status report to the
// client, and client closes itself.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadHandlerInvalidSubscribeRequest)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadHandlerInvalidSubscribeRequest)
void TestReadInteraction::TestReadHandlerInvalidSubscribeRequest()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);
        request.Init(&writer);
        writer.Finalize(&msgBuf);

        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        ASSERT_NE(exchange, nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);
        EXPECT_EQ(readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::SubscribeRequest, std::move(msgBuf),
                                                    Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)),
                  CHIP_NO_ERROR);
        DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

// Create the subscription, then remove the corresponding fabric in client and handler, the corresponding
// client and handler would be released as well.
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeInvalidateFabric)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeInvalidateFabric)
void TestReadInteraction::TestSubscribeInvalidateFabric()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    readPrepareParams.mAttributePathParamsListSize = 1;
    auto attributePathParams = std::make_unique<chip::app::AttributePathParams[]>(readPrepareParams.mAttributePathParamsListSize);
    attributePathParams[0].mEndpointId          = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId           = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId         = chip::Test::MockAttributeId(1);
    readPrepareParams.mpAttributePathParamsList = attributePathParams.get();

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 0;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        EXPECT_EQ(readClient.SendAutoResubscribeRequest(std::move(readPrepareParams)), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        GetFabricTable().Delete(GetAliceFabricIndex());
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 0u);
        GetFabricTable().Delete(GetBobFabricIndex());
        EXPECT_EQ(delegate.mError, CHIP_ERROR_IM_FABRIC_DELETED);
        ExpireSessionAliceToBob();
        ExpireSessionBobToAlice();
        CreateAliceFabric();
        CreateBobFabric();
        CreateSessionAliceToBob();
        CreateSessionBobToAlice();
    }
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestShutdownSubscription)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestShutdownSubscription)
void TestReadInteraction::TestShutdownSubscription()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    GenerateEvents();

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());

    readPrepareParams.mAttributePathParamsListSize = 1;
    auto attributePathParams = std::make_unique<chip::app::AttributePathParams[]>(readPrepareParams.mAttributePathParamsListSize);
    attributePathParams[0].mEndpointId          = chip::Test::kMockEndpoint3;
    attributePathParams[0].mClusterId           = chip::Test::MockClusterId(2);
    attributePathParams[0].mAttributeId         = chip::Test::MockAttributeId(1);
    readPrepareParams.mpAttributePathParamsList = attributePathParams.get();

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 0;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        EXPECT_EQ(readClient.SendAutoResubscribeRequest(std::move(readPrepareParams)), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);

        engine->ShutdownSubscription(chip::ScopedNodeId(readClient.GetPeerNodeId(), readClient.GetFabricIndex()),
                                     readClient.GetSubscriptionId().Value());
        EXPECT_TRUE(readClient.IsIdle());
    }
    engine->Shutdown();
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

/**
 * Tests what happens when a subscription tries to deliver reports but the
 * session it has is defunct.  Makes sure we correctly tear down the ReadHandler
 * and don't increment the "reports in flight" count.
 */
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscriptionReportWithDefunctSession)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscriptionReportWithDefunctSession)
void TestReadInteraction::TestSubscriptionReportWithDefunctSession()
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    AttributePathParams subscribePath(chip::Test::kMockEndpoint3, chip::Test::MockClusterId(2), chip::Test::MockAttributeId(1));

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = &subscribePath;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 0;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        EXPECT_EQ(readClient.SendSubscribeRequest(std::move(readPrepareParams)), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 1);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Read), 0u);
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);

        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        auto * readHandler = engine->ActiveHandlerAt(0);

        // Verify that the session we will reset later is the one we will mess
        // with now.
        EXPECT_EQ(SessionHandle(*readHandler->GetSession()), GetSessionAliceToBob());

        // Test that we send reports as needed.
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        engine->GetReportingEngine().SetDirty(subscribePath);
        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 1);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Read), 0u);
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);

        // Test that if the session is defunct we don't send reports and clean
        // up properly.
        readHandler->GetSession()->MarkAsDefunct();
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        engine->GetReportingEngine().SetDirty(subscribePath);

        DrainAndServiceIO();

        EXPECT_FALSE(delegate.mGotReport);
        EXPECT_EQ(delegate.mNumAttributeResponse, 0);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 0u);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Read), 0u);
        EXPECT_EQ(engine->GetReportingEngine().GetNumReportsInFlight(), 0u);
    }
    engine->Shutdown();
    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    // Get rid of our defunct session.
    ExpireSessionAliceToBob();
    CreateSessionAliceToBob();
}

} // namespace app
} // namespace chip
