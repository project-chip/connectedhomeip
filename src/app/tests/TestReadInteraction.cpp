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
#include <app/ClusterStateCache.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelHelper.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/ClusterPathIB.h>
#include <app/MessageDef/DataVersionFilterIB.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/icd/server/ICDConfigurationData.h>
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
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/interaction_model/Constants.h>

#include <optional>
#include <string.h>

namespace {
using namespace chip::app::Clusters::Globals::Attributes;

uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];
chip::ClusterId kTestClusterId          = 6; // OnOff, but not used as OnOff directly
chip::ClusterId kTestEventClusterId     = chip::Testing::MockClusterId(1);
chip::ClusterId kInvalidTestClusterId   = 7;
chip::EndpointId kTestEndpointId        = 1;
chip::EndpointId kTestEventEndpointId   = chip::Testing::kMockEndpoint1;
chip::EventId kTestEventIdDebug         = chip::Testing::MockEventId(1);
chip::EventId kTestEventIdCritical      = chip::Testing::MockEventId(2);
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

const chip::Testing::MockNodeConfig & TestMockNodeConfig()
{
    using namespace chip::app;
    using namespace chip::Testing;

    // clang-format off
    static const chip::Testing::MockNodeConfig config({
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
        MockEndpointConfig(chip::Testing::kMockEndpoint3, {
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

class TestReadInteraction : public chip::Testing::AppContext
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
        chip::Testing::SetMockNodeConfig(TestMockNodeConfig());
        chip::Testing::SetVersionTo(chip::Testing::kTestDataVersion1);
        chip::DeviceLayer::SetSystemLayerForTesting(&GetSystemLayer());
    }

    void TearDown() override
    {
        chip::DeviceLayer::SetSystemLayerForTesting(nullptr);
        chip::Testing::ResetMockNodeConfig();
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
    void TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters();
    void TestSubscribeWithEmptyParamsListUsesCallbackOnly();
    void TestSubscribePacketFullDuringSupplementalEncodeRollsBack();
    void TestSubscribeFullMtuPersistedFiltersRollback();
    void TestSubscribeWireMTURollbackUsesHeapAllocatedFilterArrays();
    void TestSubscribeWireMTURollbackScalesTo1024PersistedFilters();
    void TestSubscribeHeapAllocatedFiltersProduceEncodingMatchingStackBaseline();
    void TestSubscribeDuplicatePersistedFiltersAccepted();
    void TestSubscribeEmptyParamsAndEmptyCallbackEncodesNothing();
    void TestSubscribeCallbackEncodeErrorPropagates();
    void TestGetMinEventNumberBlendsCallerAndCallback();
    void TestGetMinEventNumberCallbackErrorSwallowedWhenCallerSet();
    void TestGetMinEventNumberCallbackErrorPropagatesWithoutCallerSet();
    void TestGetMinEventNumberMaxIsIdempotentAtEquality();
    void TestGetMinEventNumberZeroBoundary();
    void TestSubscribePersistedFilterForIrrelevantClusterIsDropped();
    void TestSubscribeInvalidPersistedFilterRejected();
    void TestGetMinEventNumberCallbackMaxValueWrapBehaviour();
    void TestGetMinEventNumberCallbackErrorWithCallerSetFallsBack();
    void TestReadShutdown();
    void TestReadUnexpectedSubscriptionId();
    void TestReadWildcard();
    void TestSetDirtyBetweenChunks();
    void TestReadClientSuppressResponseFlowWithInvalidReport();
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
    void TestSubscribeWithCache();

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
        EXPECT_SUCCESS(attributePathBuilder.Node(1).Endpoint(2).Cluster(3).ListIndex(5).EndOfAttributePathIB());
    }
    else if (aReportType == ReportType::kInvalidOutOfRangeAttributeId)
    {
        EXPECT_SUCCESS(attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(0xFFF18000).EndOfAttributePathIB());
    }
    else
    {
        EXPECT_EQ(aReportType, ReportType::kValid);
        EXPECT_SUCCESS(attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).EndOfAttributePathIB());
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

    EXPECT_SUCCESS(attributeDataIBBuilder.EndOfAttributeDataIB());
    EXPECT_EQ(attributeDataIBBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_SUCCESS(attributeReportIBBuilder.EndOfAttributeReportIB());
    EXPECT_EQ(attributeReportIBBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_SUCCESS(attributeReportIBsBuilder.EndOfAttributeReportIBs());
    EXPECT_EQ(attributeReportIBsBuilder.GetError(), CHIP_NO_ERROR);

    reportDataMessageBuilder.MoreChunkedMessages(false);
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    reportDataMessageBuilder.SuppressResponse(aSuppressResponse);
    EXPECT_EQ(reportDataMessageBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_SUCCESS(reportDataMessageBuilder.EndOfReportDataMessage());
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

        EXPECT_SUCCESS(attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).EndOfAttributePathIB());
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_SUCCESS(attributePathListBuilder.EndOfAttributePathIBs());
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_SUCCESS(readRequestBuilder.IsFabricFiltered(false).EndOfReadRequestMessage());
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

        EXPECT_SUCCESS(attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB());
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_SUCCESS(attributePathListBuilder.EndOfAttributePathIBs());
        EXPECT_SUCCESS(attributePathListBuilder.GetError());

        EXPECT_SUCCESS(subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage());
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

#if CHIP_CONFIG_ENABLE_ICD_SERVER
        // When an ICD build, the default behavior is to select the IdleModeDuration as MaxInterval
        kMaxIntervalCeiling =
            std::chrono::duration_cast<System::Clock::Seconds16>(ICDConfigurationData::GetInstance().GetIdleModeDuration()).count();
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

#if CHIP_CONFIG_ENABLE_ICD_SERVER
        // TC2: MaxInterval == MinIntervalFloor
        EXPECT_EQ(readHandler.SetMaxReportingInterval(kMinInterval), CHIP_ERROR_INVALID_ARGUMENT);
#else
        // TC2: MaxInterval == MinIntervalFloor
        EXPECT_EQ(readHandler.SetMaxReportingInterval(kMinInterval), CHIP_NO_ERROR);
        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(kMinInterval, maxInterval);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if CHIP_CONFIG_ENABLE_ICD_SERVER
        // TC3: Minterval < MaxInterval < max(GetPublisherSelectedIntervalLimit(), mSubscriberRequestedMaxInterval)
        EXPECT_EQ(readHandler.SetMaxReportingInterval(kMinInterval + 1), CHIP_ERROR_INVALID_ARGUMENT);
#else
        // TC3: Minterval < MaxInterval < max(GetPublisherSelectedIntervalLimit(), mSubscriberRequestedMaxInterval)
        EXPECT_EQ(readHandler.SetMaxReportingInterval(kMaxIntervalCeiling), CHIP_NO_ERROR);
        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(kMaxIntervalCeiling, maxInterval);

        // TC4: MaxInterval == Subscriber Requested Max Interval
        EXPECT_EQ(readHandler.SetMaxReportingInterval(readHandler.GetSubscriberRequestedMaxInterval()), CHIP_NO_ERROR);

        readHandler.GetReportingIntervals(minInterval, maxInterval);
        EXPECT_EQ(readHandler.GetSubscriberRequestedMaxInterval(), maxInterval);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

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

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestReadClientSuppressResponseFlowWithInvalidReport)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestReadClientSuppressResponseFlowWithInvalidReport)
void TestReadInteraction::TestReadClientSuppressResponseFlowWithInvalidReport()
{
    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), gReportScheduler), CHIP_NO_ERROR);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

    // Drop the actual read response from the server
    GetLoopback().mNumMessagesToDrop = 1;
    DrainAndServiceIO();

    GenerateReportData(buf, ReportType::kInvalidNoAttributeId, true /* aSuppressResponse*/);
    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);

    // Since we are dropping packets, things are not getting acked. Set up
    // our MRP state to look like what it would have looked like if the
    // packet had not gotten dropped.
    PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;

    EXPECT_EQ(readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(buf)),
              CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
    DrainAndServiceIO();

    // StatusResponse should NOT be sent because of SuppressResponse.
    EXPECT_EQ(GetLoopback().mSentMessageCount, 1u);

    EXPECT_EQ(delegate.mError, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);

    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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

        EXPECT_SUCCESS(attributePathBuilder.Node(1).Endpoint(2).Cluster(3).EndOfAttributePathIB());
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(attributePathListBuilder.EndOfAttributePathIBs(), CHIP_NO_ERROR);
        EXPECT_SUCCESS(readRequestBuilder.EndOfReadRequestMessage());
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

    EXPECT_SUCCESS(request.IsFabricFiltered(false).EndOfReadRequestMessage());
    EXPECT_EQ(CHIP_NO_ERROR, request.GetError());

    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    EXPECT_EQ(readRequestParser.Init(reader), CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    EXPECT_SUCCESS(readRequestParser.PrettyPrint());
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

    EXPECT_SUCCESS(request.IsFabricFiltered(false).EndOfReadRequestMessage());
    EXPECT_SUCCESS(request.GetError());

    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    EXPECT_EQ(readRequestParser.Init(reader), CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    EXPECT_SUCCESS(readRequestParser.PrettyPrint());
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

namespace {

// Callback that encodes a fixed list of DataVersionFilters into the supplied builder
// (mimicking ClusterStateCache::OnUpdateDataVersionFilterList partially populating the
// list from RAM-cached versions).
class DataVersionFilterCapturingCallback : public MockInteractionModelApp
{
public:
    DataVersionFilterCapturingCallback(const Span<chip::app::DataVersionFilter> & aFiltersToEncode) :
        mFiltersToEncode(aFiltersToEncode)
    {}

    CHIP_ERROR OnUpdateDataVersionFilterList(chip::app::DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                             const chip::Span<chip::app::AttributePathParams> & aAttributePaths,
                                             bool & aEncodedDataVersionList) override
    {
        for (auto & filter : mFiltersToEncode)
        {
            ReturnErrorOnFailure(aDataVersionFilterIBsBuilder.EncodeDataVersionFilterIB(filter));
            aEncodedDataVersionList = true;
        }
        return CHIP_NO_ERROR;
    }

private:
    Span<chip::app::DataVersionFilter> mFiltersToEncode;
};

// Count encoded DataVersionFilterIBs by parsing the closed builder TLV.
//
// Uses a raw TLVWriter / ContiguousBufferTLVReader (rather than PacketBuffer*) so the test fixture
// is self-contained and not subject to PacketBuffer pool/MTU variation across platforms — same
// pattern as TestSubscribePacketFullDuringSupplementalEncodeRollsBack.
size_t CountEncodedDataVersionFilters(chip::TLV::TLVWriter & writer, ReadRequestMessage::Builder & request,
                                      const uint8_t * backingBuf)
{
    EXPECT_EQ(request.EndOfReadRequestMessage(), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    chip::TLV::ContiguousBufferTLVReader reader;
    reader.Init(backingBuf, writer.GetLengthWritten());

    ReadRequestMessage::Parser parser;
    EXPECT_EQ(parser.Init(reader), CHIP_NO_ERROR);

    DataVersionFilterIBs::Parser dvfParser;
    CHIP_ERROR err = parser.GetDataVersionFilters(&dvfParser);
    if (err == CHIP_END_OF_TLV)
    {
        return 0;
    }
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::TLV::TLVReader dvfReader;
    dvfParser.GetReader(&dvfReader);

    size_t count = 0;
    while ((err = dvfReader.Next()) == CHIP_NO_ERROR)
    {
        ++count;
    }
    EXPECT_EQ(err, CHIP_END_OF_TLV);
    return count;
}

} // namespace

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters)
void TestReadInteraction::TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters()
{
    // Simulates MTRDevice resubscribe path:
    //  - mReadPrepareParams.mpDataVersionFilterList carries N=5 persisted DVFs (one per cluster).
    //  - The Callback (e.g. ClusterStateCache) only encodes 1 DVF from RAM (partial warm-up).
    //  - After the fix, GenerateDataVersionFilterList must blend the two: the callback's 1 RAM
    //    DVF plus all 5 persisted DVFs (6 total). The first cluster appears twice with two
    //    different versions; the Matter spec (12.8.2.5) permits duplicate DVFs and leaves the
    //    receiver free to pick one, so no deduplication is performed.
    constexpr size_t kClusterCount = 5;
    chip::app::AttributePathParams attributePathParams[kClusterCount];
    chip::app::DataVersionFilter persistedFilters[kClusterCount];
    for (size_t i = 0; i < kClusterCount; ++i)
    {
        attributePathParams[i].mEndpointId = kTestEndpointId;
        attributePathParams[i].mClusterId  = kTestClusterId + static_cast<chip::ClusterId>(i);

        persistedFilters[i].mEndpointId = kTestEndpointId;
        persistedFilters[i].mClusterId  = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mDataVersion.SetValue(static_cast<chip::DataVersion>(100 + i));
    }

    // Callback supplies just one DVF (RAM-cached for the first cluster, with a different version),
    // simulating a partially-warm ClusterStateCache after a resubscribe.
    chip::app::DataVersionFilter ramFilter[1];
    ramFilter[0].mEndpointId = kTestEndpointId;
    ramFilter[0].mClusterId  = kTestClusterId;
    ramFilter[0].mDataVersion.SetValue(999);

    DataVersionFilterCapturingCallback delegate(Span<chip::app::DataVersionFilter>(ramFilter, 1));

    // Use a raw stack-buffer TLVWriter + ContiguousBufferTLVReader (rather than PacketBuffer*)
    // so the test fixture is self-contained and not subject to PacketBuffer pool/MTU variation
    // across platforms — same pattern as TestSubscribePacketFullDuringSupplementalEncodeRollsBack.
    constexpr size_t kBufSize = 2048;
    uint8_t backingBuf[kBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    // IsFabricFiltered must be written before DataVersionFilters because the on-wire
    // schema requires context tags to appear in ascending order
    // (kIsFabricFiltered=3 before kDataVersionFilters=4 — see ReadRequestMessage.h).
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(
                  dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, kClusterCount),
                  Span<chip::app::DataVersionFilter>(persistedFilters, kClusterCount), encodedList),
              CHIP_NO_ERROR);
    EXPECT_TRUE(encodedList);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    // Total encoded = 1 (callback RAM) + 5 (persisted, fix applied) = 6 with one duplicate cluster
    // (Matter spec 12.8.2.5 permits duplicate DVFs; the receiver picks one). Without the fix this
    // would be 1 (only the callback's RAM entry).
    size_t encodedCount = CountEncodedDataVersionFilters(writer, request, backingBuf);
    EXPECT_EQ(encodedCount, kClusterCount + 1u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeWithEmptyParamsListUsesCallbackOnly)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeWithEmptyParamsListUsesCallbackOnly)
void TestReadInteraction::TestSubscribeWithEmptyParamsListUsesCallbackOnly()
{
    // Pins the !aDataVersionFilters.empty() guard: when no persisted params list is supplied
    // (MTRBaseDevice / non-MTRDevice callers), GenerateDataVersionFilterList must NOT call into
    // BuildDataVersionFilterList — only the callback's encodings should be present.
    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kTestClusterId;
    attributePathParams[1].mEndpointId = kTestEndpointId;
    attributePathParams[1].mClusterId  = kTestClusterId + 1;

    chip::app::DataVersionFilter callbackFilters[2];
    callbackFilters[0].mEndpointId = kTestEndpointId;
    callbackFilters[0].mClusterId  = kTestClusterId;
    callbackFilters[0].mDataVersion.SetValue(1);
    callbackFilters[1].mEndpointId = kTestEndpointId;
    callbackFilters[1].mClusterId  = kTestClusterId + 1;
    callbackFilters[1].mDataVersion.SetValue(2);

    DataVersionFilterCapturingCallback delegate(Span<chip::app::DataVersionFilter>(callbackFilters, 2));

    // Raw stack buffer + ContiguousBufferTLVReader — see ordering/fixture comment in
    // TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    constexpr size_t kBufSize = 2048;
    uint8_t backingBuf[kBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    // See ordering comment in TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, 2),
                                                       Span<chip::app::DataVersionFilter>(), encodedList),
              CHIP_NO_ERROR);
    EXPECT_TRUE(encodedList);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    // Exactly the 2 callback DVFs, no supplemental encoding from BuildDataVersionFilterList.
    EXPECT_EQ(CountEncodedDataVersionFilters(writer, request, backingBuf), 2u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribePacketFullDuringSupplementalEncodeRollsBack)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribePacketFullDuringSupplementalEncodeRollsBack)
void TestReadInteraction::TestSubscribePacketFullDuringSupplementalEncodeRollsBack()
{
    // Pins the existing Checkpoint/Rollback path inside BuildDataVersionFilterList: when the
    // packet fills mid-supplemental-encode, GenerateDataVersionFilterList must return CHIP_NO_ERROR
    // (not propagate CHIP_ERROR_NO_MEMORY) and the callback's encodings must be preserved.
    //
    // We use a stack-allocated raw byte buffer + chip::TLV::TLVWriter (rather than a
    // PacketBufferTLVWriter) so that the writer's capacity is exactly kSmallBufSize on every
    // platform. PacketBufferHandle::New() only guarantees a *minimum* size, so on platforms with
    // a larger pool/MTU (e.g. Zephyr native_posix_64) the underlying buffer can be much bigger
    // than the requested size, in which case the supplemental encode would never hit
    // CHIP_ERROR_NO_MEMORY and the rollback path would never be exercised.
    constexpr size_t kPersistedCount = 64;
    chip::app::AttributePathParams attributePathParams[kPersistedCount];
    chip::app::DataVersionFilter persistedFilters[kPersistedCount];
    for (size_t i = 0; i < kPersistedCount; ++i)
    {
        attributePathParams[i].mEndpointId = kTestEndpointId;
        attributePathParams[i].mClusterId  = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mEndpointId    = kTestEndpointId;
        persistedFilters[i].mClusterId     = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mDataVersion.SetValue(static_cast<chip::DataVersion>(i));
    }

    chip::app::DataVersionFilter callbackFilter[1];
    callbackFilter[0].mEndpointId = kTestEndpointId;
    callbackFilter[0].mClusterId  = kTestClusterId;
    callbackFilter[0].mDataVersion.SetValue(7);

    DataVersionFilterCapturingCallback delegate(Span<chip::app::DataVersionFilter>(callbackFilter, 1));

    // Fixed-size stack buffer: small enough that 64 persisted DVFs cannot all fit, but large
    // enough to encode the message header + the callback's single DVF. This deterministically
    // forces the supplemental encode loop to hit CHIP_ERROR_NO_MEMORY and trip the rollback path.
    //
    // Round-2 review fix: document the byte budget so a future TLV-overhead bump produces a
    // diagnosable failure rather than mysterious "buffer too small" errors:
    //   ReadRequest container (~3B) + IsFabricFiltered (~3B) + DataVersionFilterIBs container
    //   (~3B) + one DVF (endpoint=2 + cluster=4 + dataVersion=4 + struct overhead ~5B = ~15B)
    //   ≈ 24B required, well within kSmallBufSize=100; 64 persisted filters (~960B) cannot
    //   fit, so the rollback path is exercised deterministically.
    constexpr size_t kSmallBufSize = 100;
    uint8_t backingBuf[kSmallBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    // See ordering comment in TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(
                  dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, kPersistedCount),
                  Span<chip::app::DataVersionFilter>(persistedFilters, kPersistedCount), encodedList),
              CHIP_NO_ERROR);
    EXPECT_TRUE(encodedList);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    // Per Gemini code-review feedback: finalize the writer, verify the encoded TLV is well-formed,
    // and assert that the callback's filter is preserved (>=1) while at least one persisted filter
    // was rolled back due to the small buffer (<kPersistedCount + 1). This pins the Checkpoint /
    // Rollback path against future regressions that might corrupt the TLV or drop the callback's
    // filter on packet-full.
    EXPECT_EQ(request.EndOfReadRequestMessage(), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    chip::TLV::ContiguousBufferTLVReader reader;
    reader.Init(backingBuf, writer.GetLengthWritten());

    ReadRequestMessage::Parser parser;
    EXPECT_EQ(parser.Init(reader), CHIP_NO_ERROR);

    DataVersionFilterIBs::Parser dvfParser;
    CHIP_ERROR err = parser.GetDataVersionFilters(&dvfParser);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::TLV::TLVReader dvfReader;
    dvfParser.GetReader(&dvfReader);

    size_t encodedCount             = 0;
    bool sawCallbackEndpointCluster = false;
    while ((err = dvfReader.Next()) == CHIP_NO_ERROR)
    {
        // Round-2 review fix: assert the callback's specific (endpoint, cluster) made it onto
        // the wire, not just count >= 1. Without this a regression that drops the callback DVF
        // and lets a persisted DVF take its place would still satisfy the count check but
        // silently lose the in-RAM cache's contribution.
        DataVersionFilterIB::Parser dvfIb;
        if (dvfIb.Init(dvfReader) == CHIP_NO_ERROR)
        {
            ClusterPathIB::Parser path;
            if (dvfIb.GetPath(&path) == CHIP_NO_ERROR)
            {
                chip::EndpointId endpointId = 0;
                chip::ClusterId clusterId   = 0;
                if (path.GetEndpoint(&endpointId) == CHIP_NO_ERROR && path.GetCluster(&clusterId) == CHIP_NO_ERROR &&
                    endpointId == kTestEndpointId && clusterId == kTestClusterId)
                {
                    sawCallbackEndpointCluster = true;
                }
            }
        }
        ++encodedCount;
    }
    EXPECT_EQ(err, CHIP_END_OF_TLV);
    EXPECT_GE(encodedCount, 1u);
    EXPECT_LT(encodedCount, kPersistedCount + 1u);
    EXPECT_TRUE(sawCallbackEndpointCluster);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeFullMtuPersistedFiltersRollback)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeFullMtuPersistedFiltersRollback)
void TestReadInteraction::TestSubscribeFullMtuPersistedFiltersRollback()
{
    // Hardening: at the *real* wire MTU (kMaxSecureSduLengthBytes ≈ 1232 bytes per spec), a
    // sufficiently large persisted DataVersionFilter list must not overflow the SubscribeRequest.
    // The supplemental BuildDataVersionFilterList path must engage Checkpoint/Rollback once the
    // packet fills, return CHIP_NO_ERROR, leave the encoded TLV well-formed, and emit fewer than
    // the requested kPersistedCount filters.
    //
    // This complements TestSubscribePacketFullDuringSupplementalEncodeRollsBack (which uses a
    // synthetic 100-byte buffer) by exercising the rollback path against the production-sized
    // PacketBufferTLVWriter that ReadClient uses on the wire.
    //
    // Each encoded DataVersionFilterIB is ~14 bytes (endpoint=2, cluster=4, dataVersion=4 + TLV
    // overhead). 256 persisted entries (~3.5KB) deterministically exceed the ~1.2KB SDU on every
    // platform we ship, even accounting for variation in PacketBuffer pool sizes.
    //
    // The two large arrays are heap-allocated via Platform::ScopedMemoryBuffer so this test
    // function's stack frame stays well under the 8KB embedded-target limit
    // (-Werror=stack-usage=8192 on ESP32, nRF Connect SDK, etc.). Stack-allocating
    // AttributePathParams[256] (~5KB) + DataVersionFilter[256] (~3KB) + the wire-MTU backing
    // buffer would push the frame past 9KB and break the embedded build. Using Alloc (not
    // Calloc) because both element types are non-trivial and ScopedMemoryBuffer placement-news
    // each element via the default ctor — Calloc's pre-zero would just be overwritten.
    constexpr size_t kPersistedCount = 256;
    chip::Platform::ScopedMemoryBuffer<chip::app::AttributePathParams> attributePathParamsBuf;
    chip::Platform::ScopedMemoryBuffer<chip::app::DataVersionFilter> persistedFiltersBuf;
    attributePathParamsBuf.Alloc(kPersistedCount);
    persistedFiltersBuf.Alloc(kPersistedCount);
    ASSERT_NE(attributePathParamsBuf.Get(), nullptr);
    ASSERT_NE(persistedFiltersBuf.Get(), nullptr);
    chip::app::AttributePathParams * attributePathParams = attributePathParamsBuf.Get();
    chip::app::DataVersionFilter * persistedFilters      = persistedFiltersBuf.Get();
    for (size_t i = 0; i < kPersistedCount; ++i)
    {
        attributePathParams[i].mEndpointId = kTestEndpointId;
        attributePathParams[i].mClusterId  = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mEndpointId    = kTestEndpointId;
        persistedFilters[i].mClusterId     = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mDataVersion.SetValue(static_cast<chip::DataVersion>(i));
    }

    // No callback-encoded filters: this isolates the supplemental encode path so a regression
    // that fails to roll back (e.g. propagating CHIP_ERROR_NO_MEMORY) is unambiguously visible.
    DataVersionFilterCapturingCallback delegate{ Span<chip::app::DataVersionFilter>{} };

    // Use a raw buffer of the production wire-MTU size + ContiguousBufferTLVReader so the
    // rollback path engages deterministically across platforms (some PacketBuffer pools allocate
    // larger buffers than requested). Static (function-local) so this 1232-byte buffer adds
    // zero to the stack frame; safe because TEST_F bodies run sequentially.
    constexpr size_t kMtuBufSize = chip::app::kMaxSecureSduLengthBytes;
    static uint8_t backingBuf[kMtuBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    // See ordering comment in TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(
                  dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, kPersistedCount),
                  Span<chip::app::DataVersionFilter>(persistedFilters, kPersistedCount), encodedList),
              CHIP_NO_ERROR);
    // Round-2 review fix: mirror production's guard. GenerateDataVersionFilterList only calls
    // EndOfDataVersionFilterIBs when encodedList==true; assert it explicitly so a regression
    // that drops all filters but doesn't fail the call is caught here.
    EXPECT_TRUE(encodedList);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    // The encoded TLV must remain well-formed after rollback.
    size_t encodedCount = CountEncodedDataVersionFilters(writer, request, backingBuf);
    EXPECT_GT(encodedCount, 0u);
    EXPECT_LT(encodedCount, kPersistedCount);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeWireMTURollbackUsesHeapAllocatedFilterArrays)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeWireMTURollbackUsesHeapAllocatedFilterArrays)
void TestReadInteraction::TestSubscribeWireMTURollbackUsesHeapAllocatedFilterArrays()
{
    // Companion to TestSubscribeFullMtuPersistedFiltersRollback. That test heap-allocates the
    // two large filter arrays so its stack frame stays under the 8KB embedded-target limit
    // (-Werror=stack-usage=8192). This test pins the *invariant* that the rollback semantics
    // produced by GenerateDataVersionFilterList are independent of where the caller's filter
    // arrays live: heap-backed Spans must produce the same outcome (CHIP_NO_ERROR, well-formed
    // TLV, encoded count strictly less than requested) as stack-backed arrays would. A future
    // refactor that, e.g., took the address of an entry expecting stack semantics would fail
    // this test deterministically.
    constexpr size_t kPersistedCount = 256;
    chip::Platform::ScopedMemoryBuffer<chip::app::AttributePathParams> attributePathParamsBuf;
    chip::Platform::ScopedMemoryBuffer<chip::app::DataVersionFilter> persistedFiltersBuf;
    attributePathParamsBuf.Alloc(kPersistedCount);
    persistedFiltersBuf.Alloc(kPersistedCount);
    ASSERT_NE(attributePathParamsBuf.Get(), nullptr);
    ASSERT_NE(persistedFiltersBuf.Get(), nullptr);
    chip::app::AttributePathParams * attributePathParams = attributePathParamsBuf.Get();
    chip::app::DataVersionFilter * persistedFilters      = persistedFiltersBuf.Get();
    for (size_t i = 0; i < kPersistedCount; ++i)
    {
        attributePathParams[i].mEndpointId = kTestEndpointId;
        attributePathParams[i].mClusterId  = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mEndpointId    = kTestEndpointId;
        persistedFilters[i].mClusterId     = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mDataVersion.SetValue(static_cast<chip::DataVersion>(i));
    }

    DataVersionFilterCapturingCallback delegate{ Span<chip::app::DataVersionFilter>{} };

    constexpr size_t kMtuBufSize = chip::app::kMaxSecureSduLengthBytes;
    static uint8_t backingBuf[kMtuBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);
    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(
                  dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, kPersistedCount),
                  Span<chip::app::DataVersionFilter>(persistedFilters, kPersistedCount), encodedList),
              CHIP_NO_ERROR);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    size_t encodedCount = CountEncodedDataVersionFilters(writer, request, backingBuf);
    EXPECT_GT(encodedCount, 0u);
    EXPECT_LT(encodedCount, kPersistedCount);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeWireMTURollbackScalesTo1024PersistedFilters)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeWireMTURollbackScalesTo1024PersistedFilters)
void TestReadInteraction::TestSubscribeWireMTURollbackScalesTo1024PersistedFilters()
{
    // With the filter arrays moved to the heap, the rollback path must scale to filter counts
    // that would have been impossible on the stack. 1024 entries (~16KB+8KB combined) far exceeds
    // any embedded stack budget; encoding still terminates cleanly because the Checkpoint /
    // Rollback path inside BuildDataVersionFilterList stops once the SDU fills, regardless of
    // how many candidates remain unconsumed. This pins that the loop's exit condition is
    // 'packet full', not 'caller-supplied count'.
    constexpr size_t kPersistedCount = 1024;
    chip::Platform::ScopedMemoryBuffer<chip::app::AttributePathParams> attributePathParamsBuf;
    chip::Platform::ScopedMemoryBuffer<chip::app::DataVersionFilter> persistedFiltersBuf;
    attributePathParamsBuf.Alloc(kPersistedCount);
    persistedFiltersBuf.Alloc(kPersistedCount);
    // OOM guard: tight Zephyr / nRF heap configs may not have ~24KB free. Skip cleanly rather
    // than null-deref. (In production CI runners and host builds this allocation succeeds.)
    if (attributePathParamsBuf.Get() == nullptr || persistedFiltersBuf.Get() == nullptr)
    {
        GTEST_SKIP() << "Heap too small for 1024-entry stress test on this platform.";
    }
    chip::app::AttributePathParams * attributePathParams = attributePathParamsBuf.Get();
    chip::app::DataVersionFilter * persistedFilters      = persistedFiltersBuf.Get();
    for (size_t i = 0; i < kPersistedCount; ++i)
    {
        attributePathParams[i].mEndpointId = kTestEndpointId;
        attributePathParams[i].mClusterId  = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mEndpointId    = kTestEndpointId;
        persistedFilters[i].mClusterId     = kTestClusterId + static_cast<chip::ClusterId>(i);
        persistedFilters[i].mDataVersion.SetValue(static_cast<chip::DataVersion>(i));
    }

    DataVersionFilterCapturingCallback delegate{ Span<chip::app::DataVersionFilter>{} };

    constexpr size_t kMtuBufSize = chip::app::kMaxSecureSduLengthBytes;
    static uint8_t backingBuf[kMtuBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);
    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(
                  dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, kPersistedCount),
                  Span<chip::app::DataVersionFilter>(persistedFilters, kPersistedCount), encodedList),
              CHIP_NO_ERROR);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    size_t encodedCount = CountEncodedDataVersionFilters(writer, request, backingBuf);
    EXPECT_GT(encodedCount, 0u);
    // Far fewer than 1024 fit in a 1232-byte SDU; cap by the realistic per-DVFilter encoding.
    EXPECT_LT(encodedCount, kPersistedCount);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeHeapAllocatedFiltersProduceEncodingMatchingStackBaseline)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeHeapAllocatedFiltersProduceEncodingMatchingStackBaseline)
void TestReadInteraction::TestSubscribeHeapAllocatedFiltersProduceEncodingMatchingStackBaseline()
{
    // Equivalence guard for the heap-allocation refactor: at a small (stack-safe) count, encode
    // the same logical filter set twice — once with stack-allocated arrays, once with
    // heap-allocated arrays via Platform::ScopedMemoryBuffer — and assert the encoded TLV is
    // byte-for-byte identical. Pins that swapping storage for the caller-supplied Spans does
    // not perturb the on-wire output. Uses a small count so the rollback path is NOT engaged
    // (encodedCount == kSmallCount), isolating "encoding equivalence" from "rollback semantics".
    constexpr size_t kSmallCount = 8;

    auto encodeWith = [&](chip::app::AttributePathParams * paths, chip::app::DataVersionFilter * filters, uint8_t * outBuf,
                          size_t outBufSize, size_t & outWritten) -> size_t {
        for (size_t i = 0; i < kSmallCount; ++i)
        {
            paths[i].mEndpointId   = kTestEndpointId;
            paths[i].mClusterId    = kTestClusterId + static_cast<chip::ClusterId>(i);
            filters[i].mEndpointId = kTestEndpointId;
            filters[i].mClusterId  = kTestClusterId + static_cast<chip::ClusterId>(i);
            filters[i].mDataVersion.SetValue(static_cast<chip::DataVersion>(i));
        }
        DataVersionFilterCapturingCallback delegate{ Span<chip::app::DataVersionFilter>{} };
        chip::TLV::TLVWriter writer;
        writer.Init(outBuf, outBufSize);
        ReadRequestMessage::Builder request;
        EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
        EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);
        DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
        EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        bool encodedList = false;
        EXPECT_EQ(readClient.GenerateDataVersionFilterList(dvfBuilder, Span<chip::app::AttributePathParams>(paths, kSmallCount),
                                                           Span<chip::app::DataVersionFilter>(filters, kSmallCount), encodedList),
                  CHIP_NO_ERROR);
        EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);
        size_t count = CountEncodedDataVersionFilters(writer, request, outBuf);
        outWritten   = writer.GetLengthWritten();
        return count;
    };

    constexpr size_t kBufSize = chip::app::kMaxSecureSduLengthBytes;
    static uint8_t stackEncodedBuf[kBufSize];
    static uint8_t heapEncodedBuf[kBufSize];

    chip::app::AttributePathParams stackPaths[kSmallCount];
    chip::app::DataVersionFilter stackFilters[kSmallCount];
    size_t stackWritten            = 0;
    size_t stackEncodedFilterCount = encodeWith(stackPaths, stackFilters, stackEncodedBuf, sizeof(stackEncodedBuf), stackWritten);

    chip::Platform::ScopedMemoryBuffer<chip::app::AttributePathParams> heapPathsBuf;
    chip::Platform::ScopedMemoryBuffer<chip::app::DataVersionFilter> heapFiltersBuf;
    heapPathsBuf.Alloc(kSmallCount);
    heapFiltersBuf.Alloc(kSmallCount);
    ASSERT_NE(heapPathsBuf.Get(), nullptr);
    ASSERT_NE(heapFiltersBuf.Get(), nullptr);
    size_t heapWritten = 0;
    size_t heapEncodedFilterCount =
        encodeWith(heapPathsBuf.Get(), heapFiltersBuf.Get(), heapEncodedBuf, sizeof(heapEncodedBuf), heapWritten);

    // No rollback should have engaged at this small count.
    EXPECT_EQ(stackEncodedFilterCount, kSmallCount);
    EXPECT_EQ(heapEncodedFilterCount, kSmallCount);
    // Byte-for-byte equivalence: storage location of the input arrays must not affect the wire output.
    EXPECT_EQ(stackWritten, heapWritten);
    EXPECT_EQ(memcmp(stackEncodedBuf, heapEncodedBuf, stackWritten), 0);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeDuplicatePersistedFiltersAccepted)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeDuplicatePersistedFiltersAccepted)
void TestReadInteraction::TestSubscribeDuplicatePersistedFiltersAccepted()
{
    // Hardening: per Matter spec §12.8.2.5, the receiver tolerates duplicate DataVersionFilterIBs
    // for the same (endpoint, cluster) and picks one. The ReadClient must not reject or dedupe
    // such duplicates client-side — they go on the wire as supplied. This pins the existing
    // behaviour against a future "helpful" dedupe that would silently drop persisted entries
    // when the callback also encoded a RAM-cached filter for the same path (the resubscribe
    // blend case in production).
    constexpr size_t kCallbackCount  = 2;
    constexpr size_t kPersistedCount = 3;

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kTestClusterId;
    attributePathParams[1].mEndpointId = kTestEndpointId;
    attributePathParams[1].mClusterId  = kTestClusterId + 1;

    // Callback encodes 2 DVFs, both for kTestClusterId — duplicates within the callback's own
    // contribution.
    chip::app::DataVersionFilter callbackFilters[kCallbackCount];
    callbackFilters[0].mEndpointId = kTestEndpointId;
    callbackFilters[0].mClusterId  = kTestClusterId;
    callbackFilters[0].mDataVersion.SetValue(10);
    callbackFilters[1].mEndpointId = kTestEndpointId;
    callbackFilters[1].mClusterId  = kTestClusterId;
    callbackFilters[1].mDataVersion.SetValue(11);

    // Persisted list also contains duplicates of kTestClusterId, plus one for the second path.
    chip::app::DataVersionFilter persistedFilters[kPersistedCount];
    persistedFilters[0].mEndpointId = kTestEndpointId;
    persistedFilters[0].mClusterId  = kTestClusterId;
    persistedFilters[0].mDataVersion.SetValue(20);
    persistedFilters[1].mEndpointId = kTestEndpointId;
    persistedFilters[1].mClusterId  = kTestClusterId;
    persistedFilters[1].mDataVersion.SetValue(21);
    persistedFilters[2].mEndpointId = kTestEndpointId;
    persistedFilters[2].mClusterId  = kTestClusterId + 1;
    persistedFilters[2].mDataVersion.SetValue(22);

    DataVersionFilterCapturingCallback delegate(Span<chip::app::DataVersionFilter>(callbackFilters, kCallbackCount));

    // Raw stack buffer + ContiguousBufferTLVReader — see ordering/fixture comment in
    // TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    constexpr size_t kBufSize = 2048;
    uint8_t backingBuf[kBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    // See ordering comment in TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, 2),
                                                       Span<chip::app::DataVersionFilter>(persistedFilters, kPersistedCount),
                                                       encodedList),
              CHIP_NO_ERROR);
    EXPECT_TRUE(encodedList);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    // Spec §12.8.2.5: duplicates must not be rejected. Total = 2 (callback) + 3 (persisted) = 5.
    // If a future change adds client-side dedup, this count would drop and the test would fail.
    EXPECT_EQ(CountEncodedDataVersionFilters(writer, request, backingBuf), kCallbackCount + kPersistedCount);
}

namespace {

// Callback whose GetHighestReceivedEventNumber returns a fixed value, mimicking ClusterStateCache
// after it has observed an event in RAM (and/or been seeded with a persisted highest-observed
// number at construction time).
class FixedHighestEventNumberCallback : public MockInteractionModelApp
{
public:
    explicit FixedHighestEventNumberCallback(Optional<chip::EventNumber> aHighest) : mHighest(aHighest) {}

    CHIP_ERROR GetHighestReceivedEventNumber(Optional<chip::EventNumber> & aEventNumber) override
    {
        aEventNumber = mHighest;
        return CHIP_NO_ERROR;
    }

private:
    Optional<chip::EventNumber> mHighest;
};

} // namespace

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestGetMinEventNumberBlendsCallerAndCallback)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestGetMinEventNumberBlendsCallerAndCallback)
void TestReadInteraction::TestGetMinEventNumberBlendsCallerAndCallback()
{
    // GetMinEventNumber has the same callback-vs-caller asymmetry shape as
    // GenerateDataVersionFilterList. Pin the contract: when both the caller (e.g. persisted
    // ReadPrepareParams::mEventNumber) AND the Callback (e.g. ClusterStateCache's in-RAM
    // mHighestReceivedEventNumber + 1) can speak, take the maximum so the resubscribe filter
    // never asks the server for events the client has already received. A *lower* min on the
    // EventFilter would cause redundant event redelivery on the resubscribe.
    //
    // Today's only consumers either set mEventNumber (Java/Python/MTRCluster) without a
    // tracking callback, or use a ClusterStateCache and leave mEventNumber unset (MTRDevice).
    // A future caller that does both should not regress to the lower of the two.
    using chip::app::ReadClient;
    using chip::app::ReadPrepareParams;

    // Case 1: only caller-supplied. Callback returns no value. Result = caller's value (as-is,
    // because mEventNumber is already "first event _after_ the last received").
    {
        FixedHighestEventNumberCallback delegate(Optional<chip::EventNumber>{});
        ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                              ReadClient::InteractionType::Subscribe);
        ReadPrepareParams params;
        params.mEventNumber.SetValue(100);

        Optional<chip::EventNumber> result;
        EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
        ASSERT_TRUE(result.HasValue());
        EXPECT_EQ(result.Value(), 100u);
    }

    // Case 2: only callback. Caller leaves mEventNumber unset. Result = callback value + 1.
    {
        FixedHighestEventNumberCallback delegate(MakeOptional<chip::EventNumber>(static_cast<chip::EventNumber>(50)));
        ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                              ReadClient::InteractionType::Subscribe);
        ReadPrepareParams params;

        Optional<chip::EventNumber> result;
        EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
        ASSERT_TRUE(result.HasValue());
        EXPECT_EQ(result.Value(), 51u);
    }

    // Case 3: both present, callback higher (callback+1 > caller). Result = callback+1.
    {
        FixedHighestEventNumberCallback delegate(MakeOptional<chip::EventNumber>(static_cast<chip::EventNumber>(200)));
        ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                              ReadClient::InteractionType::Subscribe);
        ReadPrepareParams params;
        params.mEventNumber.SetValue(100);

        Optional<chip::EventNumber> result;
        EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
        ASSERT_TRUE(result.HasValue());
        EXPECT_EQ(result.Value(), 201u);
    }

    // Case 4: both present, caller higher. Result = caller (callback's value is stale relative
    // to what was persisted; never regress to it).
    {
        FixedHighestEventNumberCallback delegate(MakeOptional<chip::EventNumber>(static_cast<chip::EventNumber>(50)));
        ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                              ReadClient::InteractionType::Subscribe);
        ReadPrepareParams params;
        params.mEventNumber.SetValue(500);

        Optional<chip::EventNumber> result;
        EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
        ASSERT_TRUE(result.HasValue());
        EXPECT_EQ(result.Value(), 500u);
    }

    // Case 5: neither. Result = no value (server gets no EventFilter).
    {
        FixedHighestEventNumberCallback delegate(Optional<chip::EventNumber>{});
        ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                              ReadClient::InteractionType::Subscribe);
        ReadPrepareParams params;

        Optional<chip::EventNumber> result;
        EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
        EXPECT_FALSE(result.HasValue());
    }
}

namespace {

// Callback that fails OnUpdateDataVersionFilterList with a specific error, used to pin error
// propagation behaviour of GenerateDataVersionFilterList.
class FailingUpdateDvfCallback : public MockInteractionModelApp
{
public:
    explicit FailingUpdateDvfCallback(CHIP_ERROR aError) : mError(aError) {}

    CHIP_ERROR OnUpdateDataVersionFilterList(chip::app::DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                             const chip::Span<chip::app::AttributePathParams> & aAttributePaths,
                                             bool & aEncodedDataVersionList) override
    {
        return mError;
    }

private:
    CHIP_ERROR mError;
};

// Callback whose GetHighestReceivedEventNumber fails with a specific error, used to pin error
// propagation in GetMinEventNumber when the production code now unconditionally consults the
// callback (previously skipped when caller mEventNumber was set).
class FailingHighestEventNumberCallback : public MockInteractionModelApp
{
public:
    explicit FailingHighestEventNumberCallback(CHIP_ERROR aError) : mError(aError) {}

    CHIP_ERROR GetHighestReceivedEventNumber(Optional<chip::EventNumber> & aEventNumber) override { return mError; }

private:
    CHIP_ERROR mError;
};

} // namespace

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeEmptyParamsAndEmptyCallbackEncodesNothing)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeEmptyParamsAndEmptyCallbackEncodesNothing)
void TestReadInteraction::TestSubscribeEmptyParamsAndEmptyCallbackEncodesNothing()
{
    // Hardening of the new `if (!aDataVersionFilters.empty())` guard introduced by this PR.
    //
    // Before the fix, the gate was `if (!aEncodedDataVersionList)` — i.e. when a Callback encoded
    // nothing AND the caller supplied no persisted list, BuildDataVersionFilterList still ran
    // (against an empty filter list, harmlessly). The new gate keys off the *caller-supplied*
    // list instead. This test pins the resulting contract: with both inputs empty,
    // GenerateDataVersionFilterList must succeed, leave aEncodedDataVersionList=false, and
    // produce a SubscribeRequest carrying zero DataVersionFilterIBs on the wire. A regression
    // that flips the guard back (or accidentally encodes empty filters) would surface here.
    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kTestClusterId;

    // Empty callback contribution (zero DVFs to encode).
    DataVersionFilterCapturingCallback delegate{ Span<chip::app::DataVersionFilter>{} };

    constexpr size_t kBufSize = 2048;
    uint8_t backingBuf[kBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);

    // See ordering comment in TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);

    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    // Match the production caller's initialization: ReadClient::GenerateMsg and
    // ReadClient::SendSubscribeRequestImpl both seed `bool encodedDataVersionList = false`.
    // Neither GenerateDataVersionFilterList nor the callback contract ever drives the flag
    // back down to false — callees only ever set it to true. The meaningful assertion for the
    // all-empty case is "stays false AND no IBs land on the wire", not "the function resets
    // a pre-existing true".
    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, 1),
                                                       Span<chip::app::DataVersionFilter>(), encodedList),
              CHIP_NO_ERROR);
    EXPECT_FALSE(encodedList);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    EXPECT_EQ(CountEncodedDataVersionFilters(writer, request, backingBuf), 0u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeCallbackEncodeErrorPropagates)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeCallbackEncodeErrorPropagates)
void TestReadInteraction::TestSubscribeCallbackEncodeErrorPropagates()
{
    // Hardening: when the Callback returns a non-success CHIP_ERROR from
    // OnUpdateDataVersionFilterList, GenerateDataVersionFilterList must surface that error to
    // its caller (via the ReturnErrorOnFailure on the callback line) and MUST NOT fall through
    // to BuildDataVersionFilterList against a half-encoded DVF list (which could otherwise
    // overwrite/clobber state or silently mask the callback failure).
    //
    // The new `!aDataVersionFilters.empty()` gate would still match here (we deliberately
    // supply a non-empty persisted list to prove the propagation short-circuits the supplemental
    // encode); a regression that swallows the callback error would expose persisted DVFs the
    // caller never expected to be encoded after the callback failed.
    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kTestClusterId;

    chip::app::DataVersionFilter persistedFilters[1];
    persistedFilters[0].mEndpointId = kTestEndpointId;
    persistedFilters[0].mClusterId  = kTestClusterId;
    persistedFilters[0].mDataVersion.SetValue(42);

    FailingUpdateDvfCallback delegate(CHIP_ERROR_INTERNAL);

    constexpr size_t kBufSize = 2048;
    uint8_t backingBuf[kBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);
    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, 1),
                                                       Span<chip::app::DataVersionFilter>(persistedFilters, 1), encodedList),
              CHIP_ERROR_INTERNAL);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestGetMinEventNumberCallbackErrorSwallowedWhenCallerSet)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestGetMinEventNumberCallbackErrorSwallowedWhenCallerSet)
void TestReadInteraction::TestGetMinEventNumberCallbackErrorSwallowedWhenCallerSet()
{
    // Round-2 review fix: GetMinEventNumber now treats callback errors as advisory when the
    // caller has supplied a persisted mEventNumber. Pre-PR the callback was bypassed entirely
    // on this path, so a callback that started returning errors would have been invisible.
    // Failing the entire subscribe in that case would be a silent regression for callers that
    // already have their own persisted progress value. Verify: callback error is swallowed,
    // GetMinEventNumber returns CHIP_NO_ERROR, and aEventMin is the caller-supplied value.
    using chip::app::ReadClient;
    using chip::app::ReadPrepareParams;

    FailingHighestEventNumberCallback delegate(CHIP_ERROR_INCORRECT_STATE);
    ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                          ReadClient::InteractionType::Subscribe);

    ReadPrepareParams params;
    params.mEventNumber.SetValue(123);

    Optional<chip::EventNumber> result;
    EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result.Value(), 123u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestGetMinEventNumberCallbackErrorPropagatesWithoutCallerSet)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestGetMinEventNumberCallbackErrorPropagatesWithoutCallerSet)
void TestReadInteraction::TestGetMinEventNumberCallbackErrorPropagatesWithoutCallerSet()
{
    // Companion to TestGetMinEventNumberCallbackErrorSwallowedWhenCallerSet: covers the OTHER
    // branch of the new conditional, where the caller did NOT set ReadPrepareParams::mEventNumber
    // and the callback's GetHighestReceivedEventNumber returns an error. The error path is
    // shared (same ReturnErrorOnFailure on the unconditional callback call), but the post-error
    // selection branches differ — pin both so a refactor that re-orders the two-branch select
    // cannot regress one without the other.
    using chip::app::ReadClient;
    using chip::app::ReadPrepareParams;

    FailingHighestEventNumberCallback delegate(CHIP_ERROR_BUSY);
    ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                          ReadClient::InteractionType::Subscribe);

    ReadPrepareParams params; // mEventNumber intentionally unset

    Optional<chip::EventNumber> result;
    EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_ERROR_BUSY);
    EXPECT_FALSE(result.HasValue());
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestGetMinEventNumberMaxIsIdempotentAtEquality)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestGetMinEventNumberMaxIsIdempotentAtEquality)
void TestReadInteraction::TestGetMinEventNumberMaxIsIdempotentAtEquality()
{
    // Boundary: caller's mEventNumber exactly equals (callback highest received + 1). std::max
    // of two equal values must yield that same value (not regress to a lower one, not drift
    // higher by an off-by-one). This pins the increment-then-max ordering: if a future refactor
    // accidentally takes max() before adding 1 to the callback value (e.g. max(100, 100) = 100
    // instead of max(100, 101) = 101), it would silently re-deliver the event with EventNumber=100.
    using chip::app::ReadClient;
    using chip::app::ReadPrepareParams;

    // callback=100 → callback+1=101; caller=101 → expected min=101.
    FixedHighestEventNumberCallback delegate(MakeOptional<chip::EventNumber>(static_cast<chip::EventNumber>(100)));
    ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                          ReadClient::InteractionType::Subscribe);
    ReadPrepareParams params;
    params.mEventNumber.SetValue(101);

    Optional<chip::EventNumber> result;
    EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result.Value(), 101u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestGetMinEventNumberZeroBoundary)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestGetMinEventNumberZeroBoundary)
void TestReadInteraction::TestGetMinEventNumberZeroBoundary()
{
    // Zero-boundary: caller mEventNumber=0 (legitimate persisted "we have not yet seen any
    // events past 0"), callback returns 0 (received event 0 in RAM). Production code increments
    // the callback value to 1, then std::max(0, 1) must yield 1 — i.e. the resubscribe filter
    // asks for events strictly after the one already received. A regression that took the max
    // BEFORE incrementing (max(0, 0) = 0, then +1 = 1) would coincidentally produce the same
    // answer here, but if a future refactor drops the +1 entirely (max(0, 0) = 0), the server
    // would re-send event 0 on every resubscribe. Pin the post-condition.
    using chip::app::ReadClient;
    using chip::app::ReadPrepareParams;

    FixedHighestEventNumberCallback delegate(MakeOptional<chip::EventNumber>(static_cast<chip::EventNumber>(0)));
    ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                          ReadClient::InteractionType::Subscribe);
    ReadPrepareParams params;
    params.mEventNumber.SetValue(0);

    Optional<chip::EventNumber> result;
    EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result.Value(), 1u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribePersistedFilterForIrrelevantClusterIsDropped)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribePersistedFilterForIrrelevantClusterIsDropped)
void TestReadInteraction::TestSubscribePersistedFilterForIrrelevantClusterIsDropped()
{
    // Hardening: BuildDataVersionFilterList silently drops persisted DVFs whose cluster does
    // not intersect any path in aAttributePaths (the "irrelevant filter" branch — see
    // ReadClient.cpp `if (!intersected) continue;`). This must continue to hold after the new
    // blend, otherwise the resubscribe SubscribeRequest could carry stale DVFs for clusters the
    // client is no longer subscribing to — wasting MTU and producing a spec-non-compliant
    // request whose filter clusters are not referenced by any AttributePath.
    //
    // A regression that drops the relevance check (e.g. blanket-encoding everything the caller
    // persisted) would surface here as encodedCount > 1.
    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kTestClusterId;

    chip::app::DataVersionFilter persistedFilters[2];
    // Relevant: matches the only attribute path.
    persistedFilters[0].mEndpointId = kTestEndpointId;
    persistedFilters[0].mClusterId  = kTestClusterId;
    persistedFilters[0].mDataVersion.SetValue(7);
    // Irrelevant: no attribute path subscribes to this cluster — must be silently dropped.
    persistedFilters[1].mEndpointId = kTestEndpointId;
    persistedFilters[1].mClusterId  = kTestClusterId + 99;
    persistedFilters[1].mDataVersion.SetValue(8);

    DataVersionFilterCapturingCallback delegate{ Span<chip::app::DataVersionFilter>{} };

    // Raw stack buffer + ContiguousBufferTLVReader — see ordering/fixture comment in
    // TestSubscribeResubscribeEncodesAllPersistedDataVersionFilters.
    constexpr size_t kBufSize = 2048;
    uint8_t backingBuf[kBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);
    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, 1),
                                                       Span<chip::app::DataVersionFilter>(persistedFilters, 2), encodedList),
              CHIP_NO_ERROR);
    EXPECT_TRUE(encodedList);
    EXPECT_EQ(dvfBuilder.EndOfDataVersionFilterIBs(), CHIP_NO_ERROR);

    // Only the relevant DVF should appear on the wire; the irrelevant one is dropped.
    EXPECT_EQ(CountEncodedDataVersionFilters(writer, request, backingBuf), 1u);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeInvalidPersistedFilterRejected)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeInvalidPersistedFilterRejected)
void TestReadInteraction::TestSubscribeInvalidPersistedFilterRejected()
{
    // Hardening: BuildDataVersionFilterList validates each entry via IsValidDataVersionFilter()
    // and returns CHIP_ERROR_INVALID_ARGUMENT on the first malformed one (missing mDataVersion,
    // kInvalidClusterId, or kInvalidEndpointId). Pre-fix, that validation only ran when the
    // caller-supplied list was non-empty AND the callback encoded nothing; post-fix the
    // caller-supplied list is now consulted whenever it is non-empty, regardless of what the
    // callback did. That broadens the surface that can return CHIP_ERROR_INVALID_ARGUMENT.
    //
    // Pin the propagation: a persisted entry without mDataVersion (e.g. corrupted disk state,
    // partial deserialization from MTRDevice's storage layer) must surface as
    // CHIP_ERROR_INVALID_ARGUMENT to the caller — not silently encode garbage on the wire.
    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kTestClusterId;

    // Persisted filter is missing mDataVersion — fails IsValidDataVersionFilter().
    chip::app::DataVersionFilter persistedFilters[1];
    persistedFilters[0].mEndpointId = kTestEndpointId;
    persistedFilters[0].mClusterId  = kTestClusterId;
    // mDataVersion intentionally left unset.

    // Empty callback contribution — isolates the failure to the supplemental encode path.
    DataVersionFilterCapturingCallback delegate{ Span<chip::app::DataVersionFilter>{} };

    constexpr size_t kBufSize = 2048;
    uint8_t backingBuf[kBufSize];
    chip::TLV::TLVWriter writer;
    writer.Init(backingBuf, sizeof(backingBuf));
    ReadRequestMessage::Builder request;
    EXPECT_EQ(request.Init(&writer), CHIP_NO_ERROR);
    EXPECT_EQ(request.IsFabricFiltered(false).GetError(), CHIP_NO_ERROR);
    DataVersionFilterIBs::Builder & dvfBuilder = request.CreateDataVersionFilters();
    EXPECT_EQ(request.GetError(), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Subscribe);

    bool encodedList = false;
    EXPECT_EQ(readClient.GenerateDataVersionFilterList(dvfBuilder, Span<chip::app::AttributePathParams>(attributePathParams, 1),
                                                       Span<chip::app::DataVersionFilter>(persistedFilters, 1), encodedList),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestGetMinEventNumberCallbackMaxValueWrapBehaviour)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestGetMinEventNumberCallbackMaxValueWrapBehaviour)
void TestReadInteraction::TestGetMinEventNumberCallbackMaxValueWrapBehaviour()
{
    // Boundary regression guard: GetMinEventNumber unconditionally adds 1 to the callback's
    // highest-received EventNumber. If the callback ever returned EventNumber=UINT64_MAX,
    // a naive (callback + 1) would wrap to 0 — and with caller unset the result would be 0,
    // which silently asks the server to re-deliver every event from the start of the log.
    //
    // Round-2 review fix added a saturating-increment guard: when the callback returns
    // UINT64_MAX the production code clears the value instead of wrapping. Pin both branches
    // of that guard so a future regression that restores the naive `+1` is caught.
    using chip::app::ReadClient;
    using chip::app::ReadPrepareParams;

    // Sub-case A: caller unset, callback=UINT64_MAX. With the saturating-increment guard the
    // computed min is empty (no EventFilter on the wire) rather than 0. A regression that
    // wraps to 0 would silently request a full event-log replay on every resubscribe.
    {
        FixedHighestEventNumberCallback delegate(MakeOptional<chip::EventNumber>(UINT64_MAX));
        ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                              ReadClient::InteractionType::Subscribe);
        ReadPrepareParams params;

        Optional<chip::EventNumber> result;
        EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
        EXPECT_FALSE(result.HasValue());
    }

    // Sub-case B: caller=UINT64_MAX, callback=UINT64_MAX. Callback saturates to "no value";
    // caller stays at UINT64_MAX; the blended path picks the caller. Demonstrates that
    // persisted caller state continues to drive the min at the extreme boundary.
    {
        FixedHighestEventNumberCallback delegate(MakeOptional<chip::EventNumber>(UINT64_MAX));
        ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                              ReadClient::InteractionType::Subscribe);
        ReadPrepareParams params;
        params.mEventNumber.SetValue(UINT64_MAX);

        Optional<chip::EventNumber> result;
        EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
        ASSERT_TRUE(result.HasValue());
        EXPECT_EQ(result.Value(), UINT64_MAX);
    }
}

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestGetMinEventNumberCallbackErrorWithCallerSetFallsBack)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestGetMinEventNumberCallbackErrorWithCallerSetFallsBack)
void TestReadInteraction::TestGetMinEventNumberCallbackErrorWithCallerSetFallsBack()
{
    // Round-2 review fix: pin the advisory-error semantics. When the caller has supplied a
    // persisted ReadPrepareParams::mEventNumber, a callback that returns an error MUST be
    // logged-and-swallowed (not propagated) so the caller's persisted value is used. Pre-PR
    // the callback was bypassed entirely on this path; failing the entire subscribe would be
    // a silent regression for callers like MTRDevice that have their own progress value.
    //
    // Companion to TestGetMinEventNumberCallbackErrorPropagatesWithoutCallerSet, which pins
    // the OTHER branch (caller unset → error propagates because we have nothing to fall back
    // to). Together they fence the advisory-error semantics on both sides.
    using chip::app::ReadClient;
    using chip::app::ReadPrepareParams;

    FailingHighestEventNumberCallback delegate(CHIP_ERROR_INCORRECT_STATE);
    ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                          ReadClient::InteractionType::Subscribe);

    ReadPrepareParams params;
    params.mEventNumber.SetValue(100);

    Optional<chip::EventNumber> result;
    EXPECT_EQ(readClient.GetMinEventNumber(params, result), CHIP_NO_ERROR);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result.Value(), 100u);
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
    attributePathParams[0].mEndpointId = chip::Testing::kMockEndpoint2;
    attributePathParams[0].mClusterId  = chip::Testing::MockClusterId(3);

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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
        attributePathParam.mEndpointId  = chip::Testing::kMockEndpoint3;
        attributePathParam.mClusterId   = chip::Testing::MockClusterId(2);
        attributePathParam.mAttributeId = chip::Testing::MockAttributeId(4);
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
                    dirtyPath.mEndpointId  = chip::Testing::kMockEndpoint3;
                    dirtyPath.mClusterId   = chip::Testing::MockClusterId(2);
                    dirtyPath.mAttributeId = chip::Testing::MockAttributeId(4);

                    if (aPath.mEndpointId == dirtyPath.mEndpointId && aPath.mClusterId == dirtyPath.mClusterId &&
                        aPath.mAttributeId == dirtyPath.mAttributeId)
                    {
                        // At this time, we are in the middle of report for second item.
                        mNumAttributeResponsesWhenSetDirty = mNumAttributeResponse;
                        mNumArrayItemsWhenSetDirty         = mNumArrayItems;
                        EXPECT_SUCCESS(InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath));
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

        EXPECT_SUCCESS(attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB());
        EXPECT_SUCCESS(attributePathBuilder.GetError());

        EXPECT_SUCCESS(attributePathListBuilder.EndOfAttributePathIBs());
        EXPECT_SUCCESS(attributePathListBuilder.GetError());

        EXPECT_SUCCESS(subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage());
        EXPECT_SUCCESS(subscribeRequestBuilder.GetError());

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

        TEMPORARY_RETURN_IGNORED attributePathBuilder.Node(1)
            .Endpoint(2)
            .Cluster(3)
            .Attribute(4)
            .ListIndex(5)
            .EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t idleModeDuration =
            std::chrono::duration_cast<System::Clock::Seconds16>(ICDConfigurationData::GetInstance().GetIdleModeDuration()).count();

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

        TEMPORARY_RETURN_IGNORED attributePathBuilder.Node(1)
            .Endpoint(2)
            .Cluster(3)
            .Attribute(4)
            .ListIndex(5)
            .EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t idleModeDuration =
            std::chrono::duration_cast<System::Clock::Seconds16>(ICDConfigurationData::GetInstance().GetIdleModeDuration()).count();

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

        TEMPORARY_RETURN_IGNORED attributePathBuilder.Node(1)
            .Endpoint(2)
            .Cluster(3)
            .Attribute(4)
            .ListIndex(5)
            .EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);

        EXPECT_EQ(subscribeRequestBuilder.GetError(), CHIP_NO_ERROR);
        EXPECT_EQ(writer.Finalize(&subscribeRequestbuf), CHIP_NO_ERROR);

        EXPECT_EQ(readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf)), CHIP_NO_ERROR);

        uint16_t idleModeDuration =
            std::chrono::duration_cast<System::Clock::Seconds16>(ICDConfigurationData::GetInstance().GetIdleModeDuration()).count();

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

        TEMPORARY_RETURN_IGNORED attributePathBuilder.Node(1)
            .Endpoint(2)
            .Cluster(3)
            .Attribute(4)
            .ListIndex(5)
            .EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
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
 *        is greater than the publisher selected max interval limit and the MaxIntervalCeiling
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

    // Since the default IdleModeDuration is 300s for unit test,
    // we need to set both values higher than 3600s which is the publisher selected max interval limit in this situation.
    uint16_t kMinInterval        = 3610;
    uint16_t kMaxIntervalCeiling = 3610;

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

        TEMPORARY_RETURN_IGNORED attributePathBuilder.Node(1)
            .Endpoint(2)
            .Cluster(3)
            .Attribute(4)
            .ListIndex(5)
            .EndOfAttributePathIB();
        EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED attributePathListBuilder.EndOfAttributePathIBs();
        EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);

        TEMPORARY_RETURN_IGNORED subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
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

class IntegrationCacheCallback : public ClusterStateCache::Callback
{
public:
    void Reset()
    {
        mOnDoneCalledCount                        = 0;
        mOnReportBeginCalledCount                 = 0;
        mOnReportEndCalledCount                   = 0;
        mOnAttributeChangedCalledCount            = 0;
        mNotifySubscriptionStillActiveCalledCount = 0;
        mOnSubscriptionEstablishedCalledCount     = 0;
    }

    void OnDone(ReadClient * apReadClient) override { mOnDoneCalledCount++; }
    void OnReportBegin() override { mOnReportBeginCalledCount++; }
    void OnReportEnd() override { mOnReportEndCalledCount++; }
    void OnAttributeChanged(ClusterStateCache * cache, const ConcreteAttributePath & path) override
    {
        mOnAttributeChangedCalledCount++;
    }
    void NotifySubscriptionStillActive(const ReadClient & aReadClient) override
    {
        mNotifySubscriptionStillActiveCalledCount++;
        System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
        if (mLastLivenessTime != System::Clock::kZero)
        {
            mSubscriptionUptime += std::chrono::duration_cast<System::Clock::Timeout>(now - mLastLivenessTime);
        }
        mLastLivenessTime = now;
    }
    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override
    {
        mOnSubscriptionEstablishedCalledCount++;
        mSubscriptionEstablishmentTime = System::SystemClock().GetMonotonicTimestamp();
        mLastLivenessTime              = mSubscriptionEstablishmentTime;
        mSubscriptionUptime            = System::Clock::kZero;
    }

    System::Clock::Timeout GetSubscriptionUptime() const { return mSubscriptionUptime; }

    int mOnDoneCalledCount                        = 0;
    int mOnReportBeginCalledCount                 = 0;
    int mOnReportEndCalledCount                   = 0;
    int mOnAttributeChangedCalledCount            = 0;
    int mNotifySubscriptionStillActiveCalledCount = 0;
    int mOnSubscriptionEstablishedCalledCount     = 0;

private:
    System::Clock::Timestamp mSubscriptionEstablishmentTime = System::Clock::kZero;
    System::Clock::Timestamp mLastLivenessTime              = System::Clock::kZero;
    System::Clock::Timeout mSubscriptionUptime              = System::Clock::kZero;
};

TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteraction, TestSubscribeWithCache)
TEST_F_FROM_FIXTURE_NO_BODY(TestReadInteractionSync, TestSubscribeWithCache)
void TestReadInteraction::TestSubscribeWithCache()
{
    // Test that ClusterStateCache correctly forwards all subscription reporting lifecycle callbacks
    // (such as OnReportBegin, OnReportEnd, OnAttributeChanged, and NotifySubscriptionStillActive)
    // to the registered application callback. This also verifies that we can track subscription
    // liveness and uptime from the application callback.
    IntegrationCacheCallback cacheCallback;
    ClusterStateCache cache(cacheCallback);

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;
    readPrepareParams.mAttributePathParamsListSize              = 1;
    readPrepareParams.mMinIntervalFloorSeconds                  = 1;
    readPrepareParams.mMaxIntervalCeilingSeconds                = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(),
                                   cache.GetBufferedCallback(), chip::app::ReadClient::InteractionType::Subscribe);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        auto * engine = chip::app::InteractionModelEngine::GetInstance();
        EXPECT_EQ(engine->GetNumActiveReadHandlers(), 1u);
        ASSERT_NE(engine->ActiveHandlerAt(0), nullptr);
        auto & readHandler = *engine->ActiveHandlerAt(0);

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        // Priming report should have been received.
        EXPECT_EQ(cacheCallback.mOnReportBeginCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnReportEndCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnAttributeChangedCalledCount, 1);
        EXPECT_EQ(cacheCallback.mNotifySubscriptionStillActiveCalledCount, 0); // Not called for priming report
        EXPECT_EQ(cacheCallback.mOnSubscriptionEstablishedCalledCount, 1);

        // Initial uptime should be 0
        EXPECT_EQ(cacheCallback.GetSubscriptionUptime(), System::Clock::kZero);

        TLV::TLVReader reader;
        EXPECT_EQ(cache.Get(ConcreteAttributePath(kTestEndpointId, kTestClusterId, 1), reader), CHIP_NO_ERROR);

        // Case 1: NON-EMPTY report (data update)
        // Trigger report after subscription established -> should call NotifySubscriptionStillActive
        System::Clock::Timeout delta1 = System::Clock::Seconds16(minInterval);
        gMockClock.AdvanceMonotonic(delta1);

        chip::app::AttributePathParams dirtyPath;
        dirtyPath.mEndpointId  = kTestEndpointId;
        dirtyPath.mClusterId   = kTestClusterId;
        dirtyPath.mAttributeId = 1;
        EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath), CHIP_NO_ERROR);

        cacheCallback.Reset();
        DrainAndServiceIO();

        EXPECT_EQ(cacheCallback.mNotifySubscriptionStillActiveCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnReportBeginCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnReportEndCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnAttributeChangedCalledCount, 1);
        EXPECT_EQ(cacheCallback.GetSubscriptionUptime(), delta1);

        // Case 2: EMPTY report (keep-alive)
        // Advance clock by MaxInterval without making anything dirty -> should trigger keep-alive
        System::Clock::Timeout delta2 = System::Clock::Seconds16(maxInterval);
        gMockClock.AdvanceMonotonic(delta2);
        cacheCallback.Reset();
        GetIOContext().DriveIO(); // Trigger timer and schedule run

        DrainAndServiceIO();

        EXPECT_EQ(cacheCallback.mNotifySubscriptionStillActiveCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnReportBeginCalledCount, 0); // No data, so no ReportBegin
        EXPECT_EQ(cacheCallback.mOnReportEndCalledCount, 0);   // No data, so no ReportEnd
        EXPECT_EQ(cacheCallback.mOnAttributeChangedCalledCount, 0);
        EXPECT_EQ(cacheCallback.GetSubscriptionUptime(), delta1 + delta2);

        // Case 3: Another NON-EMPTY report
        System::Clock::Timeout delta3 = System::Clock::Seconds16(minInterval);
        gMockClock.AdvanceMonotonic(delta3);
        EXPECT_EQ(chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath), CHIP_NO_ERROR);

        cacheCallback.Reset();
        DrainAndServiceIO();

        EXPECT_EQ(cacheCallback.mNotifySubscriptionStillActiveCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnReportBeginCalledCount, 1);
        EXPECT_EQ(cacheCallback.mOnReportEndCalledCount, 1);
        EXPECT_EQ(cacheCallback.GetSubscriptionUptime(), delta1 + delta2 + delta3);

        chip::app::InteractionModelEngine::GetInstance()->ShutdownAllSubscriptions();
    }

    DrainAndServiceIO();
    EXPECT_EQ(cacheCallback.mOnDoneCalledCount, 1);

    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
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
    chip::Testing::ResetMockNodeConfig();

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
            dirtyPath.mEndpointId  = chip::Testing::kMockEndpoint2;
            dirtyPath.mClusterId   = chip::Testing::MockClusterId(3);
            dirtyPath.mAttributeId = chip::Testing::MockAttributeId(1);

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
            dirtyPath.mEndpointId = chip::Testing::kMockEndpoint3;

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
    attributePathParams[0].mClusterId           = chip::Testing::MockClusterId(3);
    attributePathParams[0].mAttributeId         = chip::Testing::MockAttributeId(1);
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
            dirtyPath.mEndpointId = chip::Testing::kMockEndpoint2;
            dirtyPath.mClusterId  = chip::Testing::MockClusterId(3);

            EXPECT_EQ(engine->GetReportingEngine().SetDirty(dirtyPath), CHIP_NO_ERROR);

            DrainAndServiceIO();

            EXPECT_TRUE(delegate.mGotReport);
            EXPECT_EQ(delegate.mNumAttributeResponse, 1);
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mEndpointId, chip::Testing::kMockEndpoint2);
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mClusterId, chip::Testing::MockClusterId(3));
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mAttributeId, chip::Testing::MockAttributeId(1));
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
    attributePathParams[0].mEndpointId          = chip::Testing::kMockEndpoint2;
    attributePathParams[0].mClusterId           = chip::Testing::MockClusterId(3);
    attributePathParams[0].mAttributeId         = chip::Testing::MockAttributeId(1);
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
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mEndpointId, chip::Testing::kMockEndpoint2);
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mClusterId, chip::Testing::MockClusterId(3));
            EXPECT_EQ(delegate.mReceivedAttributePaths[0].mAttributeId, chip::Testing::MockAttributeId(1));
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint2;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(3);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(1);

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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
        dirtyPath1.mClusterId   = chip::Testing::MockClusterId(2);
        dirtyPath1.mEndpointId  = chip::Testing::kMockEndpoint3;
        dirtyPath1.mAttributeId = chip::Testing::MockAttributeId(4);

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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
        dirtyPath1.mClusterId   = chip::Testing::MockClusterId(2);
        dirtyPath1.mEndpointId  = chip::Testing::kMockEndpoint3;
        dirtyPath1.mAttributeId = chip::Testing::MockAttributeId(4);

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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
        dirtyPath1.mClusterId   = chip::Testing::MockClusterId(2);
        dirtyPath1.mEndpointId  = chip::Testing::kMockEndpoint3;
        dirtyPath1.mAttributeId = chip::Testing::MockAttributeId(4);

        EXPECT_SUCCESS(engine->GetReportingEngine().SetDirty(dirtyPath1));
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

void CheckForInvalidAction(Testing::MessageCapturer & messageLog)
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
        EXPECT_SUCCESS(response.Init(&writer));
        response.Status(Protocols::InteractionModel::Status::Busy);
        EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

        chip::Testing::MessageCapturer messageLog(*this);
        messageLog.mCaptureStandaloneAcks = false;

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(*this, readClient.mExchange.Get());

        GetLoopback().mSentMessageCount                 = 0;
        GetLoopback().mNumMessagesToDrop                = 0;
        GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        GetLoopback().mDroppedMessageCount              = 0;
        EXPECT_EQ(readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf)),
                  CHIP_IM_GLOBAL_STATUS(Busy));

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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
        EXPECT_SUCCESS(response.Init(&writer));
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

        EXPECT_EQ(readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf)),
                  CHIP_IM_GLOBAL_STATUS(Busy));
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
        EXPECT_SUCCESS(response.Init(&writer));
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

        EXPECT_EQ(readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf)),
                  CHIP_ERROR_INVALID_MESSAGE_TYPE);
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
        EXPECT_SUCCESS(response.Init(&writer));
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

        EXPECT_EQ(readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf)),
                  CHIP_ERROR_END_OF_TLV);
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
        EXPECT_SUCCESS(response.Init(&writer));
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
        EXPECT_SUCCESS(response.Init(&writer));
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        response.MaxInterval(1);
        EXPECT_SUCCESS(response.EndOfSubscribeResponseMessage());
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

        EXPECT_EQ(readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf)),
                  CHIP_ERROR_INVALID_SUBSCRIPTION);
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
        EXPECT_SUCCESS(response.Init(&writer));
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        EXPECT_SUCCESS(response.EndOfReportDataMessage());

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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
        EXPECT_SUCCESS(response.Init(&writer));
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        EXPECT_SUCCESS(response.EndOfReportDataMessage());
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

        EXPECT_EQ(readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf)),
                  CHIP_ERROR_INVALID_SUBSCRIPTION);
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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

        EXPECT_SUCCESS(chip::app::InitWriterWithSpaceReserved(writer, 0));

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

        EXPECT_SUCCESS(chip::app::InitWriterWithSpaceReserved(writer, 0));

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

        EXPECT_SUCCESS(chip::app::InitWriterWithSpaceReserved(writer, 0));
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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
        EXPECT_SUCCESS(chip::app::InitWriterWithSpaceReserved(writer, 0));
        EXPECT_SUCCESS(request.Init(&writer));
        EXPECT_SUCCESS(writer.Finalize(&msgBuf));

        EXPECT_SUCCESS(readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::WriteRequest, std::move(msgBuf)));
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId   = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId = chip::Testing::MockAttributeId(4);

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
        EXPECT_SUCCESS(chip::app::InitWriterWithSpaceReserved(writer, 0));
        EXPECT_SUCCESS(request.Init(&writer));
        EXPECT_SUCCESS(writer.Finalize(&msgBuf));

        EXPECT_SUCCESS(readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::StatusResponse, std::move(msgBuf)));
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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
    EXPECT_SUCCESS(CreateSessionBobToAlice());
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

        EXPECT_SUCCESS(chip::app::InitWriterWithSpaceReserved(writer, 0));
        EXPECT_SUCCESS(request.Init(&writer));
        EXPECT_SUCCESS(writer.Finalize(&msgBuf));

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
    attributePathParams[0].mEndpointId          = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId           = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId         = chip::Testing::MockAttributeId(1);
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

        EXPECT_SUCCESS(GetFabricTable().Delete(GetAliceFabricIndex()));
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 0u);
        EXPECT_SUCCESS(GetFabricTable().Delete(GetBobFabricIndex()));
        EXPECT_EQ(delegate.mError, CHIP_ERROR_IM_FABRIC_DELETED);
        ExpireSessionAliceToBob();
        ExpireSessionBobToAlice();
        EXPECT_SUCCESS(CreateAliceFabric());
        EXPECT_SUCCESS(CreateBobFabric());
        EXPECT_SUCCESS(CreateSessionAliceToBob());
        EXPECT_SUCCESS(CreateSessionBobToAlice());
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
    attributePathParams[0].mEndpointId          = chip::Testing::kMockEndpoint3;
    attributePathParams[0].mClusterId           = chip::Testing::MockClusterId(2);
    attributePathParams[0].mAttributeId         = chip::Testing::MockAttributeId(1);
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

        EXPECT_SUCCESS(engine->ShutdownSubscription(chip::ScopedNodeId(readClient.GetPeerNodeId(), readClient.GetFabricIndex()),
                                                    readClient.GetSubscriptionId().Value()));
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

    AttributePathParams subscribePath(chip::Testing::kMockEndpoint3, chip::Testing::MockClusterId(2),
                                      chip::Testing::MockAttributeId(1));

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
        EXPECT_SUCCESS(engine->GetReportingEngine().SetDirty(subscribePath));
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
        EXPECT_SUCCESS(engine->GetReportingEngine().SetDirty(subscribePath));

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
    EXPECT_SUCCESS(CreateSessionAliceToBob());
}

} // namespace app
} // namespace chip
