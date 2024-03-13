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

/**
 *    @file
 *      This file implements unit tests for CHIP Interaction Model Read Interaction
 *
 */

#include "lib/support/CHIPMem.h"
#include <access/examples/PermissiveAccessControlDelegate.h>
#include <app/AttributeAccessInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelHelper.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/icd/server/ICDServerConfig.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>
#include <type_traits>

namespace {
uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];
chip::ClusterId kTestClusterId          = 6;
chip::ClusterId kTestEventClusterId     = chip::Test::MockClusterId(1);
chip::ClusterId kInvalidTestClusterId   = 7;
chip::EndpointId kTestEndpointId        = 1;
chip::EndpointId kTestEventEndpointId   = chip::Test::kMockEndpoint1;
chip::EventId kTestEventIdDebug         = chip::Test::MockEventId(1);
chip::EventId kTestEventIdCritical      = chip::Test::MockEventId(2);
uint8_t kTestFieldValue1                = 1;
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

class TestContext : public chip::Test::AppContext
{
public:
    // Performs shared setup for all tests in the test suite
    CHIP_ERROR SetUpTestSuite() override
    {
        ReturnErrorOnFailure(chip::Test::AppContext::SetUpTestSuite());
        gRealClock = &chip::System::SystemClock();
        chip::System::Clock::Internal::SetSystemClockForTesting(&gMockClock);

        if (mSyncScheduler)
        {
            gReportScheduler = chip::app::reporting::GetSynchronizedReportScheduler();
            sUsingSubSync    = true;
        }
        else
        {
            gReportScheduler = chip::app::reporting::GetDefaultReportScheduler();
        }

        return CHIP_NO_ERROR;
    }

    static int nlTestSetUpTestSuite_Sync(void * context)
    {
        static_cast<TestContext *>(context)->mSyncScheduler = true;
        return nlTestSetUpTestSuite(context);
    }

    // Performs shared teardown for all tests in the test suite
    void TearDownTestSuite() override
    {
        chip::System::Clock::Internal::SetSystemClockForTesting(gRealClock);
        chip::Test::AppContext::TearDownTestSuite();
    }

    // Performs setup for each individual test in the test suite
    CHIP_ERROR SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        ReturnErrorOnFailure(chip::Test::AppContext::SetUp());

        CHIP_ERROR err = CHIP_NO_ERROR;
        VerifyOrExit((err = mEventCounter.Init(0)) == CHIP_NO_ERROR,
                     ChipLogError(AppServer, "Init EventCounter failed: %" CHIP_ERROR_FORMAT, err.Format()));
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), ArraySize(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);

    exit:
        return err;
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        chip::app::EventManagement::DestroyEventManagement();
        chip::Test::AppContext::TearDown();
    }

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
    bool mSyncScheduler = false;
};

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

void GenerateEvents(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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
    err = logMgmt.LogEvent(&testEventGenerator, options1, eid1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options2, eid2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

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
            mReceivedAttributePaths.push_back(aPath);
            mNumAttributeResponse++;
            mGotReport = true;

            if (aPath.IsListItemOperation())
            {
                mNumArrayItems++;
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
                        mNumArrayItems += static_cast<int>(count);
                    }
                }
            }
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

CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState)
{
    if (aPath.mClusterId >= Test::kMockEndpointMin)
    {
        return Test::ReadSingleMockClusterData(aSubjectDescriptor.fabricIndex, aPath, aAttributeReports, apEncoderState);
    }

    if (!(aPath.mClusterId == kTestClusterId && aPath.mEndpointId == kTestEndpointId))
    {
        AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
        ReturnErrorOnFailure(aAttributeReports.GetError());
        ChipLogDetail(DataManagement, "TEST Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", aPath.mClusterId, aPath.mAttributeId);

        AttributeStatusIB::Builder & attributeStatus = attributeReport.CreateAttributeStatus();
        ReturnErrorOnFailure(attributeReport.GetError());
        AttributePathIB::Builder & attributePath = attributeStatus.CreatePath();
        ReturnErrorOnFailure(attributeStatus.GetError());

        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());
        StatusIB::Builder & errorStatus = attributeStatus.CreateErrorStatus();
        ReturnErrorOnFailure(attributeStatus.GetError());
        errorStatus.EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::UnsupportedAttribute));
        ReturnErrorOnFailure(errorStatus.GetError());
        ReturnErrorOnFailure(attributeStatus.EndOfAttributeStatusIB());
        return attributeReport.EndOfAttributeReportIB();
    }

    return AttributeValueEncoder(aAttributeReports, 0, aPath, 0).Encode(kTestFieldValue1);
}

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    if (kTestDataVersion1 == aRequiredVersion)
    {
        return true;
    }

    return false;
}

bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint)
{
    return false;
}

class TestReadInteraction
{
    using Seconds16      = System::Clock::Seconds16;
    using Milliseconds32 = System::Clock::Milliseconds32;

public:
    static void TestReadClient(nlTestSuite * apSuite, void * apContext);
    static void TestReadUnexpectedSubscriptionId(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateAttributePathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateInvalidAttributePathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateOneEventPaths(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateTwoEventPaths(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientInvalidReport(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientInvalidAttributeId(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerInvalidAttributePath(nlTestSuite * apSuite, void * apContext);
    static void TestProcessSubscribeRequest(nlTestSuite * apSuite, void * apContext);
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    static void TestICDProcessSubscribeRequestSupMaxIntervalCeiling(nlTestSuite * apSuite, void * apContext);
    static void TestICDProcessSubscribeRequestInfMaxIntervalCeiling(nlTestSuite * apSuite, void * apContext);
    static void TestICDProcessSubscribeRequestSupMinInterval(nlTestSuite * apSuite, void * apContext);
    static void TestICDProcessSubscribeRequestMaxMinInterval(nlTestSuite * apSuite, void * apContext);
    static void TestICDProcessSubscribeRequestInvalidIdleModeDuration(nlTestSuite * apSuite, void * apContext);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    static void TestReadRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestPostSubscribeRoundtripChunkReport(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithNoMatchPathDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithSameDifferentPathsDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadWildcard(nlTestSuite * apSuite, void * apContext);
    static void TestReadChunking(nlTestSuite * apSuite, void * apContext);
    static void TestSetDirtyBetweenChunks(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeEarlyReport(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeUrgentWildcardEvent(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeWildcard(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribePartialOverlap(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeSetDirtyFullyOverlap(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeEarlyShutdown(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestReadInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidInterval(nlTestSuite * apSuite, void * apContext);
    static void TestReadShutdown(nlTestSuite * apSuite, void * apContext);
    static void TestResubscribeRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeRoundtripStatusReportTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestPostSubscribeRoundtripStatusReportTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestReadChunkingStatusReportTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestReadReportFailure(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeRoundtripChunkStatusReportTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestPostSubscribeRoundtripChunkStatusReportTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestPostSubscribeRoundtripChunkReportTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientReceiveInvalidMessage(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeClientReceiveInvalidStatusResponse(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeClientReceiveWellFormedStatusResponse(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeClientReceiveInvalidReportMessage(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeClientReceiveUnsolicitedInvalidReportMessage(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeClientReceiveInvalidSubscribeResponseMessage(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId(nlTestSuite * apSuite,
                                                                                            void * apContext);
    static void TestReadChunkingInvalidSubscriptionId(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerMalformedReadRequest1(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerMalformedReadRequest2(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeSendUnknownMessage(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeSendInvalidStatusReport(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerInvalidSubscribeRequest(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidateFabric(nlTestSuite * apSuite, void * apContext);
    static void TestShutdownSubscription(nlTestSuite * apSuite, void * apContext);
    static void TestSubscriptionReportWithDefunctSession(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerMalformedSubscribeRequest(nlTestSuite * apSuite, void * apContext);

private:
    enum class ReportType : uint8_t
    {
        kValid,
        kInvalidNoAttributeId,
        kInvalidOutOfRangeAttributeId,
    };

    static void GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                   ReportType aReportType, bool aSuppressResponse, bool aHasSubscriptionId);
};

void TestReadInteraction::GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                             ReportType aReportType, bool aSuppressResponse, bool aHasSubscriptionId = false)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    ReportDataMessage::Builder reportDataMessageBuilder;

    err = reportDataMessageBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    if (aHasSubscriptionId)
    {
        reportDataMessageBuilder.SubscriptionId(1);
        NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);
    }

    AttributeReportIBs::Builder & attributeReportIBsBuilder = reportDataMessageBuilder.CreateAttributeReportIBs();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    AttributeReportIB::Builder & attributeReportIBBuilder = attributeReportIBsBuilder.CreateAttributeReport();
    NL_TEST_ASSERT(apSuite, attributeReportIBsBuilder.GetError() == CHIP_NO_ERROR);

    AttributeDataIB::Builder & attributeDataIBBuilder = attributeReportIBBuilder.CreateAttributeData();
    NL_TEST_ASSERT(apSuite, attributeReportIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeDataIBBuilder.DataVersion(2);
    err = attributeDataIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathIB::Builder & attributePathBuilder = attributeDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributeDataIBBuilder.GetError() == CHIP_NO_ERROR);

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
        NL_TEST_ASSERT(apSuite, aReportType == ReportType::kValid);
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).EndOfAttributePathIB();
    }

    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = attributeDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(chip::app::AttributeDataIB::Tag::kData)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    attributeDataIBBuilder.EndOfAttributeDataIB();
    NL_TEST_ASSERT(apSuite, attributeDataIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeReportIBBuilder.EndOfAttributeReportIB();
    NL_TEST_ASSERT(apSuite, attributeReportIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeReportIBsBuilder.EndOfAttributeReportIBs();
    NL_TEST_ASSERT(apSuite, attributeReportIBsBuilder.GetError() == CHIP_NO_ERROR);

    reportDataMessageBuilder.MoreChunkedMessages(false);
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    reportDataMessageBuilder.SuppressResponse(aSuppressResponse);
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    reportDataMessageBuilder.EndOfReportDataMessage();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadClient(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;
    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();

    GenerateReportData(apSuite, apContext, buf, ReportType::kValid, true /* aSuppressResponse*/);
    err = readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadUnexpectedSubscriptionId(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;
    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();

    // For read, we don't expect there is subscription id in report data.
    GenerateReportData(apSuite, apContext, buf, ReportType::kValid, true /* aSuppressResponse*/, true /*aHasSubscriptionId*/);
    err = readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
}

void TestReadInteraction::TestReadHandler(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    NullReadHandlerCallback nullCallback;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);
        ReadHandler readHandler(nullCallback, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        GenerateReportData(apSuite, apContext, reportDatabuf, ReportType::kValid, false /* aSuppressResponse*/);
        err = readHandler.SendReportData(std::move(reportDatabuf), false);
        NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

        writer.Init(std::move(readRequestbuf));
        err = readRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        err = attributePathListBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
        readRequestBuilder.IsFabricFiltered(false).EndOfReadRequestMessage();
        NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&readRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        // Call ProcessReadRequest directly, because OnInitialRequest sends status
        // messages on the wire instead of returning an error.
        err = readHandler.ProcessReadRequest(std::move(readRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    engine->Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadClientGenerateAttributePathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mAttributeId = 0;
    attributePathParams[1].mAttributeId = 0;
    attributePathParams[1].mListIndex   = 0;

    Span<AttributePathParams> attributePaths(attributePathParams, 2 /*aAttributePathParamsListSize*/);

    AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
    err = readClient.GenerateAttributePaths(attributePathListBuilder, attributePaths);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadClientGenerateInvalidAttributePathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mAttributeId = 0;
    attributePathParams[1].mListIndex   = 0;

    Span<AttributePathParams> attributePaths(attributePathParams, 2 /*aAttributePathParamsListSize*/);

    AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
    err = readClient.GenerateAttributePaths(attributePathListBuilder, attributePaths);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
}

void TestReadInteraction::TestReadClientInvalidReport(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();

    GenerateReportData(apSuite, apContext, buf, ReportType::kInvalidNoAttributeId, true /* aSuppressResponse*/);

    err = readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
}

void TestReadInteraction::TestReadClientInvalidAttributeId(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();

    GenerateReportData(apSuite, apContext, buf, ReportType::kInvalidOutOfRangeAttributeId, true /* aSuppressResponse*/);

    err = readClient.ProcessReportData(std::move(buf), ReadClient::ReportType::kContinuingTransaction);
    // Overall processing should succeed.
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // We should not have gotten any attribute reports or errors.
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    NL_TEST_ASSERT(apSuite, !delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, !delegate.mReadError);
}

void TestReadInteraction::TestReadHandlerInvalidAttributePath(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    NullReadHandlerCallback nullCallback;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);
        ReadHandler readHandler(nullCallback, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        GenerateReportData(apSuite, apContext, reportDatabuf, ReportType::kValid, false /* aSuppressResponse*/);
        err = readHandler.SendReportData(std::move(reportDatabuf), false);
        NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

        writer.Init(std::move(readRequestbuf));
        err = readRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        readRequestBuilder.EndOfReadRequestMessage();
        NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&readRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.ProcessReadRequest(std::move(readRequestbuf));
        ChipLogError(DataManagement, "The error is %s", ErrorStr(err));
        NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_END_OF_TLV);

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
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadClientGenerateOneEventPaths(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    chip::app::EventPathParams eventPathParams[1];
    eventPathParams[0].mEndpointId = 2;
    eventPathParams[0].mClusterId  = 3;
    eventPathParams[0].mEventId    = 4;

    EventPathIBs::Builder & eventPathListBuilder = request.CreateEventRequests();
    Span<EventPathParams> eventPaths(eventPathParams, 1 /*aEventPathParamsListSize*/);
    err = readClient.GenerateEventPaths(eventPathListBuilder, eventPaths);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    request.IsFabricFiltered(false).EndOfReadRequestMessage();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == request.GetError());

    err = writer.Finalize(&msgBuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    err = readRequestParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    readRequestParser.PrettyPrint();
#endif

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadClientGenerateTwoEventPaths(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
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
    err = readClient.GenerateEventPaths(eventPathListBuilder, eventPaths);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    request.IsFabricFiltered(false).EndOfReadRequestMessage();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == request.GetError());

    err = writer.Finalize(&msgBuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    err = readRequestParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    readRequestParser.PrettyPrint();
#endif

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadRoundtrip(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

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

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = eventPathParams;
    readPrepareParams.mEventPathParamsListSize     = 1;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mEventNumber.SetValue(1);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mNumDataElementIndex == 1);
        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);

        delegate.mGotEventResponse     = false;
        delegate.mNumAttributeResponse = 0;
        delegate.mGotReport            = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadRoundtripWithDataVersionFilter(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

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

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);

        delegate.mNumAttributeResponse = 0;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadRoundtripWithNoMatchPathDataVersionFilter(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);

        delegate.mNumAttributeResponse = 0;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

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

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);

        delegate.mNumAttributeResponse = 0;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadRoundtripWithSameDifferentPathsDataVersionFilter(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

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

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    readPrepareParams.mpDataVersionFilterList      = dataVersionFilters;
    readPrepareParams.mDataVersionFilterListSize   = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);

        delegate.mNumAttributeResponse = 0;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadWildcard(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId = Test::kMockEndpoint2;
    attributePathParams[0].mClusterId  = Test::MockClusterId(3);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 5);
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// TestReadChunking will try to read a few large attributes, the report won't fit into the MTU and result in chunking.
void TestReadInteraction::TestReadChunking(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with kMockAttribute4ListLength large
    // OCTET_STRING elements.
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        // We get one chunk with 4 array elements, and then one chunk per
        // element, and the total size of the array is
        // kMockAttribute4ListLength.
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1 + (kMockAttribute4ListLength - 4));
        NL_TEST_ASSERT(apSuite, delegate.mNumArrayItems == 6);
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestSetDirtyBetweenChunks(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::app::AttributePathParams attributePathParams[2];
    for (auto & attributePathParam : attributePathParams)
    {
        attributePathParam.mEndpointId  = Test::kMockEndpoint3;
        attributePathParam.mClusterId   = Test::MockClusterId(2);
        attributePathParam.mAttributeId = Test::MockAttributeId(4);
    }

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
                    dirtyPath.mEndpointId  = Test::kMockEndpoint3;
                    dirtyPath.mClusterId   = Test::MockClusterId(2);
                    dirtyPath.mAttributeId = Test::MockAttributeId(4);

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
        NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        // Our list has length kMockAttribute4ListLength.  Since the underlying
        // path iterator should be reset to the beginning of the cluster it is
        // currently iterating, we expect to get another value for our
        // attribute.  The way the packet boundaries happen to fall, that value
        // will encode 4 items in the first IB and then one IB per item.
        const int expectedIBs = 1 + (kMockAttribute4ListLength - 4);
        ChipLogError(DataManagement, "OLD: %d\n", currentAttributeResponsesWhenSetDirty);
        ChipLogError(DataManagement, "NEW: %d\n", delegate.mNumAttributeResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == currentAttributeResponsesWhenSetDirty + expectedIBs);
        NL_TEST_ASSERT(apSuite, delegate.mNumArrayItems == currentArrayItemsWhenSetDirty + kMockAttribute4ListLength);
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kInvalidTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    }

    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestProcessSubscribeRequest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        err = subscribeRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(2);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(3);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        err = attributePathListBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&subscribeRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    engine->Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
/**
 * @brief Test validates that an ICD will choose its IdleModeDuration (GetPublisherSelectedIntervalLimit)
 *        as MaxInterval when the MaxIntervalCeiling is superior.
 */
void TestReadInteraction::TestICDProcessSubscribeRequestSupMaxIntervalCeiling(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint16_t kMinInterval        = 0;
    uint16_t kMaxIntervalCeiling = 1;

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        err = subscribeRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        err = attributePathListBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&subscribeRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        uint16_t idleModeDuration = readHandler.GetPublisherSelectedIntervalLimit();

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        NL_TEST_ASSERT(apSuite, minInterval == kMinInterval);
        NL_TEST_ASSERT(apSuite, maxInterval == idleModeDuration);
    }
    engine->Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

/**
 * @brief Test validates that an ICD will choose its IdleModeDuration (GetPublisherSelectedIntervalLimit)
 *        as MaxInterval when the MaxIntervalCeiling is inferior.
 */
void TestReadInteraction::TestICDProcessSubscribeRequestInfMaxIntervalCeiling(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint16_t kMinInterval        = 0;
    uint16_t kMaxIntervalCeiling = 1;

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        err = subscribeRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        err = attributePathListBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&subscribeRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        uint16_t idleModeDuration = readHandler.GetPublisherSelectedIntervalLimit();

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        NL_TEST_ASSERT(apSuite, minInterval == kMinInterval);
        NL_TEST_ASSERT(apSuite, maxInterval == idleModeDuration);
    }
    engine->Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

/**
 * @brief Test validates that an ICD will choose a multiple of its IdleModeDuration (GetPublisherSelectedIntervalLimit)
 *        as MaxInterval when the MinInterval > IdleModeDuration.
 */
void TestReadInteraction::TestICDProcessSubscribeRequestSupMinInterval(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint16_t kMinInterval        = 305; // Default IdleModeDuration is 300
    uint16_t kMaxIntervalCeiling = 605;

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        err = subscribeRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        err = attributePathListBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&subscribeRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        uint16_t idleModeDuration = readHandler.GetPublisherSelectedIntervalLimit();

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        NL_TEST_ASSERT(apSuite, minInterval == kMinInterval);
        NL_TEST_ASSERT(apSuite, maxInterval == (2 * idleModeDuration));
    }
    engine->Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

/**
 * @brief Test validates that an ICD will choose a maximal value for an uint16 if the multiple of the IdleModeDuration
 *        is greater than variable size.
 */
void TestReadInteraction::TestICDProcessSubscribeRequestMaxMinInterval(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint16_t kMinInterval        = System::Clock::Seconds16::max().count();
    uint16_t kMaxIntervalCeiling = System::Clock::Seconds16::max().count();

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        err = subscribeRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        err = attributePathListBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&subscribeRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        NL_TEST_ASSERT(apSuite, minInterval == kMinInterval);
        NL_TEST_ASSERT(apSuite, maxInterval == kMaxIntervalCeiling);
    }
    engine->Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

/**
 * @brief Test validates that an ICD will choose the MaxIntervalCeiling as MaxInterval if the next multiple after the MinInterval
 *        is greater than the IdleModeDuration and MaxIntervalCeiling
 */
void TestReadInteraction::TestICDProcessSubscribeRequestInvalidIdleModeDuration(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint16_t kMinInterval        = 400;
    uint16_t kMaxIntervalCeiling = 400;

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read, gReportScheduler);

        writer.Init(std::move(subscribeRequestbuf));
        err = subscribeRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.KeepSubscriptions(true);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MinIntervalFloorSeconds(kMinInterval);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        subscribeRequestBuilder.MaxIntervalCeilingSeconds(kMaxIntervalCeiling);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        err = attributePathListBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&subscribeRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        uint16_t minInterval;
        uint16_t maxInterval;
        readHandler.GetReportingIntervals(minInterval, maxInterval);

        NL_TEST_ASSERT(apSuite, minInterval == kMinInterval);
        NL_TEST_ASSERT(apSuite, maxInterval == kMaxIntervalCeiling);
    }
    engine->Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

void TestReadInteraction::TestSubscribeRoundtrip(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    }

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        delegate.mGotReport = false;
        err                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        GenerateEvents(apSuite, apContext);
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
        ctx.GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mGotEventResponse     = false;
        delegate.mNumAttributeResponse = 0;

        err = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse == true);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Test report with 2 different path, and 1 same path
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        ctx.GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        err                            = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Test report with 3 different path, and one path is overlapped with another
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        ctx.GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        err                            = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath3);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Test report with 3 different path, all are not overlapped, one path is not interested for current subscription
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        ctx.GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        err                            = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath4);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        uint16_t minInterval;
        uint16_t maxInterval;
        delegate.mpReadHandler->GetReportingIntervals(minInterval, maxInterval);

        // Test empty report
        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(maxInterval));
        ctx.GetIOContext().DriveIO();

        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().IsRunScheduled());
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestSubscribeEarlyReport(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        readPrepareParams.mpEventPathParamsList[0].mIsUrgentEvent = true;
        delegate.mGotEventResponse                                = false;
        err                                                       = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        System::Clock::Timestamp startTime = gMockClock.GetMonotonicTimestamp();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        uint16_t minInterval;
        uint16_t maxInterval;
        delegate.mpReadHandler->GetReportingIntervals(minInterval, maxInterval);

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) ==
                           gMockClock.GetMonotonicTimestamp() + Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler) ==
                           gMockClock.GetMonotonicTimestamp() + Seconds16(maxInterval));

        // Confirm that the node is scheduled to run
        NL_TEST_ASSERT(apSuite, gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        ReportScheduler::ReadHandlerNode * node = gReportScheduler->GetReadHandlerNode(delegate.mpReadHandler);
        NL_TEST_ASSERT(apSuite, node != nullptr);

        GenerateEvents(apSuite, apContext);

        // modify the node's min timestamp to be 50ms later than the timer expiration time
        node->SetIntervalTimeStamps(delegate.mpReadHandler, startTime + Milliseconds32(50));
        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) ==
                           gMockClock.GetMonotonicTimestamp() + Seconds16(readPrepareParams.mMinIntervalFloorSeconds) +
                               Milliseconds32(50));

        NL_TEST_ASSERT(apSuite, gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) > startTime);
        NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->IsDirty());
        delegate.mGotEventResponse = false;

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        NL_TEST_ASSERT(apSuite, !InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
        // Service Timer expired event
        ctx.GetIOContext().DriveIO();

        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) > gMockClock.GetMonotonicTimestamp());

        // The behavior on min interval elapse is different between synced subscription and non-synced subscription
        if (!sUsingSubSync)
        {
            // Verify the ReadHandler is considered as reportable even if its node's min timestamp has not expired
            NL_TEST_ASSERT(apSuite, gReportScheduler->IsReportableNow(delegate.mpReadHandler));
            NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Service Engine Run
            ctx.GetIOContext().DriveIO();
            // Service EventManagement event
            ctx.GetIOContext().DriveIO();
            ctx.GetIOContext().DriveIO();
            NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        }
        else
        {
            // Verify logic on Min for synced subscription
            // Verify the ReadHandler is not considered as reportable yet
            NL_TEST_ASSERT(apSuite, !gReportScheduler->IsReportableNow(delegate.mpReadHandler));

            // confirm that the timer was kicked off for the next min of the node
            NL_TEST_ASSERT(apSuite, gReportScheduler->IsReportScheduled(delegate.mpReadHandler));

            // Expired new timer
            gMockClock.AdvanceMonotonic(Milliseconds32(50));

            // Service Timer expired event
            ctx.GetIOContext().DriveIO();
            NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Service Engine Run
            ctx.GetIOContext().DriveIO();
            // Service EventManagement event
            ctx.GetIOContext().DriveIO();
            ctx.GetIOContext().DriveIO();
            NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        }

        // The behavior is identical on max since the sync subscription will interpret an early max firing as a earlier node got
        // reportable and allow nodes that have passed their min to sync on it.
        NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->IsDirty());
        delegate.mGotEventResponse = false;
        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) ==
                           gMockClock.GetMonotonicTimestamp() + Seconds16(readPrepareParams.mMinIntervalFloorSeconds));
        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler) ==
                           gMockClock.GetMonotonicTimestamp() + Seconds16(maxInterval));

        // Confirm that the node is scheduled to run
        NL_TEST_ASSERT(apSuite, gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        NL_TEST_ASSERT(apSuite, node != nullptr);

        // modify the node's max timestamp to be 50ms later than the timer expiration time
        node->SetIntervalTimeStamps(delegate.mpReadHandler, gMockClock.GetMonotonicTimestamp() + Milliseconds32(50));
        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler) ==
                           gMockClock.GetMonotonicTimestamp() + Seconds16(maxInterval) + Milliseconds32(50));

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(Seconds16(maxInterval));

        NL_TEST_ASSERT(apSuite, !InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
        // Service Timer expired event
        ctx.GetIOContext().DriveIO();

        // Verify the ReadHandler is considered as reportable even if its node's min timestamp has not expired
        NL_TEST_ASSERT(apSuite,
                       gReportScheduler->GetMaxTimestampForHandler(delegate.mpReadHandler) > gMockClock.GetMonotonicTimestamp());
        NL_TEST_ASSERT(apSuite, gReportScheduler->IsReportableNow(delegate.mpReadHandler));
        NL_TEST_ASSERT(apSuite, !gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->IsDirty());
        NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
        // Service Engine Run
        ctx.GetIOContext().DriveIO();
        // Service EventManagement event
        ctx.GetIOContext().DriveIO();
        ctx.GetIOContext().DriveIO();
        NL_TEST_ASSERT(apSuite, gReportScheduler->IsReportScheduled(delegate.mpReadHandler));
        NL_TEST_ASSERT(apSuite, !InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());
    }
    ctx.DrainAndServiceIO();

    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestSubscribeUrgentWildcardEvent(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    MockInteractionModelApp nonUrgentDelegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
    NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
        app::ReadClient nonUrgentReadClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(),
                                            nonUrgentDelegate, chip::app::ReadClient::InteractionType::Subscribe);
        nonUrgentDelegate.mGotReport = false;
        err                          = nonUrgentReadClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        readPrepareParams.mpEventPathParamsList[0].mIsUrgentEvent = true;
        delegate.mGotReport                                       = false;
        err                                                       = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        System::Clock::Timestamp startTime = gMockClock.GetMonotonicTimestamp();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 2);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        nonUrgentDelegate.mpReadHandler = engine->ActiveHandlerAt(0);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(1) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(1);

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 2);

        GenerateEvents(apSuite, apContext);

        NL_TEST_ASSERT(apSuite, gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) > startTime);
        NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->IsDirty());
        delegate.mGotEventResponse = false;
        delegate.mGotReport        = false;

        NL_TEST_ASSERT(apSuite, gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler) > startTime);
        NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->IsDirty());
        nonUrgentDelegate.mGotEventResponse = false;
        nonUrgentDelegate.mGotReport        = false;

        // wait for min interval 1 seconds (in test, we use 0.6 seconds considering the time variation), expect no event is
        // received, then wait for 0.8 seconds, then the urgent event would be sent out
        //  currently DriveIOUntil will call `DriveIO` at least once, which means that if there is any CPU scheduling issues,
        // there's a chance 1.9s will already have elapsed by the time we get there, which will result in DriveIO being called when
        // it shouldn't. Better fix could happen inside DriveIOUntil, not sure the sideeffect there.

        // Advance monotonic looping to allow events to trigger
        gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(600));
        ctx.GetIOContext().DriveIO();

        NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(800));

        // Service Timer expired event
        ctx.GetIOContext().DriveIO();

        // Service Engine Run
        ctx.GetIOContext().DriveIO();

        // Service EventManagement event
        ctx.GetIOContext().DriveIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);

        // The logic differs here depending on what Scheduler is implemented
        if (!sUsingSubSync)
        {
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);

            // Since we just sent a report for our urgent subscription, the min interval of the urgent subcription should have been
            // updated
            NL_TEST_ASSERT(
                apSuite, gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) > gMockClock.GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->IsDirty());
            delegate.mGotEventResponse = false;

            // For our non-urgent subscription, we did not send anything, so the min interval should of the non urgent subcription
            // should be in the past
            NL_TEST_ASSERT(apSuite,
                           gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler) <
                               gMockClock.GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->IsDirty());

            // Advance monotonic timestamp for min interval to elapse
            gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(2100));
            ctx.GetIOContext().DriveIO();

            // No reporting should have happened.
            NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);

            // The min-interval should have elapsed for the urgent subscription, and our handler should still
            // not be dirty or reportable.
            NL_TEST_ASSERT(apSuite,
                           gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) <
                               System::SystemClock().GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->ShouldStartReporting());

            // And the non-urgent one should not have changed state either, since
            // it's waiting for the max-interval.
            NL_TEST_ASSERT(apSuite,
                           gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler) <
                               System::SystemClock().GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite,
                           gReportScheduler->GetMaxTimestampForHandler(nonUrgentDelegate.mpReadHandler) >
                               System::SystemClock().GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // There should be no reporting run scheduled.  This is very important;
            // otherwise we can get a false-positive pass below because the run was
            // already scheduled by here.
            NL_TEST_ASSERT(apSuite, !InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Generate some events, which should get reported.
            GenerateEvents(apSuite, apContext);

            // Urgent read handler should now be dirty, and reportable.
            NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->ShouldStartReporting());
            NL_TEST_ASSERT(apSuite, gReportScheduler->IsReadHandlerReportable(delegate.mpReadHandler));

            // Non-urgent read handler should not be reportable.
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // Still no reporting should have happened.
            NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);

            ctx.DrainAndServiceIO();

            // Should get those urgent events reported.
            NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);

            // Should get nothing reported on the non-urgent handler.
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);
        }
        else
        {
            // If we're using the sub-sync scheduler, we should have gotten the non-urgent event as well.
            NL_TEST_ASSERT(apSuite, nonUrgentDelegate.mGotEventResponse);

            // Since we just sent a report for both our subscriptions, the min interval of the urgent subcription should have been
            NL_TEST_ASSERT(
                apSuite, gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) > gMockClock.GetMonotonicTimestamp());

            NL_TEST_ASSERT(apSuite,
                           gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler) >
                               gMockClock.GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->IsDirty());
            delegate.mGotEventResponse          = false;
            nonUrgentDelegate.mGotEventResponse = false;

            // Advance monotonic timestamp for min interval to elapse
            gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(2100));
            ctx.GetIOContext().DriveIO();

            // No reporting should have happened.
            NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);

            // The min-interval should have elapsed for both subscriptions, and our handlers should still
            // not be dirty or reportable.
            NL_TEST_ASSERT(apSuite,
                           gReportScheduler->GetMinTimestampForHandler(delegate.mpReadHandler) <
                               System::SystemClock().GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite,
                           gReportScheduler->GetMinTimestampForHandler(nonUrgentDelegate.mpReadHandler) <
                               System::SystemClock().GetMonotonicTimestamp());
            NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, !delegate.mpReadHandler->ShouldStartReporting());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // There should be no reporting run scheduled.  This is very important;
            // otherwise we can get a false-positive pass below because the run was
            // already scheduled by here.
            NL_TEST_ASSERT(apSuite, !InteractionModelEngine::GetInstance()->GetReportingEngine().IsRunScheduled());

            // Generate some events, which should get reported.
            GenerateEvents(apSuite, apContext);

            // Urgent read handler should now be dirty, and reportable.
            NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->ShouldStartReporting());
            NL_TEST_ASSERT(apSuite, gReportScheduler->IsReadHandlerReportable(delegate.mpReadHandler));

            // Non-urgent read handler should not be reportable.
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->IsDirty());
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mpReadHandler->ShouldStartReporting());

            // Still no reporting should have happened.
            NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
            NL_TEST_ASSERT(apSuite, !nonUrgentDelegate.mGotEventResponse);

            ctx.DrainAndServiceIO();

            // Should get those urgent events reported and the non urgent reported for synchronisation
            NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
            NL_TEST_ASSERT(apSuite, nonUrgentDelegate.mGotEventResponse);
        }
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestSubscribeWildcard(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    std::unique_ptr<chip::app::AttributePathParams[]> attributePathParams(new chip::app::AttributePathParams[2]);
    // Subscribe to full wildcard paths, repeat twice to ensure chunking.
    readPrepareParams.mpAttributePathParamsList    = attributePathParams.get();
    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        err = readClient.SendAutoResubscribeRequest(std::move(readPrepareParams));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);

#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        // Mock attribute storage in src/app/util/mock/attribute-storage.cpp
        // has the following items
        // - Endpoint 0xFFFE
        //    - cluster 0xFFF1'FC01 (2 attributes)
        //    - cluster 0xFFF1'FC02 (3 attributes)
        // - Endpoint 0xFFFD
        //    - cluster 0xFFF1'FC01 (2 attributes)
        //    - cluster 0xFFF1'FC02 (4 attributes)
        //    - cluster 0xFFF1'FC03 (5 attributes)
        // - Endpoint 0xFFFC
        //    - cluster 0xFFF1'FC01 (3 attributes)
        //    - cluster 0xFFF1'FC02 (6 attributes)
        //    - cluster 0xFFF1'FC03 (2 attributes)
        //    - cluster 0xFFF1'FC04 (2 attributes)
        //
        // For at total of 29 attributes. There are two wildcard subscription
        // paths, for a total of 58 attributes.
        //
        // Attribute 0xFFFC::0xFFF1'FC02::0xFFF1'0004 (kMockEndpoint3::MockClusterId(2)::MockAttributeId(4))
        // is a list of kMockAttribute4ListLength elements of size 256 bytes each, which cannot fit in a single
        // packet, so gets list chunking applied to it.
        //
        // Because delegate.mNumAttributeResponse counts AttributeDataIB instances, not attributes,
        // the count will depend on exactly how the list for attribute
        // 0xFFFC::0xFFF1'FC02::0xFFF1'0004 is chunked.  For each of the two instances of that attribute
        // in the response, there will be one AttributeDataIB for the start of the list (which will include
        // some number of 256-byte elements), then one AttributeDataIB for each of the remaining elements.
        //
        // When EventList is enabled, for the first report for the list attribute we receive three
        // of its items in the initial list, then the remaining items.  For the second report we
        // receive 2 items in the initial list followed by the remaining items.
        constexpr size_t kExpectedAttributeResponse = 29 * 2 + (kMockAttribute4ListLength - 3) + (kMockAttribute4ListLength - 2);
#else
        // When EventList is not enabled, the packet boundaries shift and for the first
        // report for the list attribute we receive four of its items in the initial list,
        // then additional items.  For the second report we receive 4 items in
        // the initial list followed by additional items.
        constexpr size_t kExpectedAttributeResponse = 29 * 2 + (kMockAttribute4ListLength - 4) + (kMockAttribute4ListLength - 4);
#endif
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == kExpectedAttributeResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumArrayItems == 12);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        // Set a concrete path dirty
        {
            delegate.mGotReport            = false;
            delegate.mNumAttributeResponse = 0;

            AttributePathParams dirtyPath;
            dirtyPath.mEndpointId  = Test::kMockEndpoint2;
            dirtyPath.mClusterId   = Test::MockClusterId(3);
            dirtyPath.mAttributeId = Test::MockAttributeId(1);

            err = engine->GetReportingEngine().SetDirty(dirtyPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();

            NL_TEST_ASSERT(apSuite, delegate.mGotReport);
            // We subscribed wildcard path twice, so we will receive two reports here.
            NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        }

        // Set a endpoint dirty
        {
            delegate.mGotReport            = false;
            delegate.mNumAttributeResponse = 0;
            delegate.mNumArrayItems        = 0;

            AttributePathParams dirtyPath;
            dirtyPath.mEndpointId = Test::kMockEndpoint3;

            err = engine->GetReportingEngine().SetDirty(dirtyPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            //
            // We need to DrainAndServiceIO() until attribute callback will be called.
            // This is not correct behavior and is tracked in Issue #17528.
            //
            int last;
            do
            {
                last = delegate.mNumAttributeResponse;
                ctx.DrainAndServiceIO();
            } while (last != delegate.mNumAttributeResponse);

            // Mock endpoint3 has 13 attributes in total, and we subscribed twice.
            // And attribute 3/2/4 is a list with 6 elements and list chunking
            // is applied to it, but the way the packet boundaries fall we get two of
            // its items as a single list, followed by 4 more items for one
            // of our subscriptions, and 3 items as a single list followed by 3
            // more items for the other.
            //
            // Thus we should receive 13*2 + 4 + 3 = 33 attribute data in total.
            ChipLogError(DataManagement, "RESPO: %d\n", delegate.mNumAttributeResponse);
            NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 33);
            NL_TEST_ASSERT(apSuite, delegate.mNumArrayItems == 12);
        }
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Subscribe (wildcard, C3, A1), then setDirty (E2, C3, wildcard), receive one attribute after setDirty
void TestReadInteraction::TestSubscribePartialOverlap(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    std::unique_ptr<chip::app::AttributePathParams[]> attributePathParams(new chip::app::AttributePathParams[2]);
    attributePathParams[0].mClusterId              = Test::MockClusterId(3);
    attributePathParams[0].mAttributeId            = Test::MockAttributeId(1);
    readPrepareParams.mpAttributePathParamsList    = attributePathParams.get();
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        err = readClient.SendAutoResubscribeRequest(std::move(readPrepareParams));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        // Set a partial overlapped path dirty
        {
            delegate.mGotReport            = false;
            delegate.mNumAttributeResponse = 0;

            AttributePathParams dirtyPath;
            dirtyPath.mEndpointId = Test::kMockEndpoint2;
            dirtyPath.mClusterId  = Test::MockClusterId(3);

            err = engine->GetReportingEngine().SetDirty(dirtyPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();

            NL_TEST_ASSERT(apSuite, delegate.mGotReport);
            NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
            NL_TEST_ASSERT(apSuite, delegate.mReceivedAttributePaths[0].mEndpointId == Test::kMockEndpoint2);
            NL_TEST_ASSERT(apSuite, delegate.mReceivedAttributePaths[0].mClusterId == Test::MockClusterId(3));
            NL_TEST_ASSERT(apSuite, delegate.mReceivedAttributePaths[0].mAttributeId == Test::MockAttributeId(1));
        }
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Subscribe (E2, C3, A1), then setDirty (wildcard, wildcard, wildcard), receive one attribute after setDirty
void TestReadInteraction::TestSubscribeSetDirtyFullyOverlap(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    std::unique_ptr<chip::app::AttributePathParams[]> attributePathParams(new chip::app::AttributePathParams[1]);
    attributePathParams[0].mClusterId              = Test::kMockEndpoint2;
    attributePathParams[0].mClusterId              = Test::MockClusterId(3);
    attributePathParams[0].mAttributeId            = Test::MockAttributeId(1);
    readPrepareParams.mpAttributePathParamsList    = attributePathParams.get();
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        attributePathParams.release();
        err = readClient.SendAutoResubscribeRequest(std::move(readPrepareParams));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        // Set a full overlapped path dirty and expect to receive one E2C3A1
        {
            delegate.mGotReport            = false;
            delegate.mNumAttributeResponse = 0;

            AttributePathParams dirtyPath;
            err = engine->GetReportingEngine().SetDirty(dirtyPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();

            NL_TEST_ASSERT(apSuite, delegate.mGotReport);
            NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
            NL_TEST_ASSERT(apSuite, delegate.mReceivedAttributePaths[0].mEndpointId == Test::kMockEndpoint2);
            NL_TEST_ASSERT(apSuite, delegate.mReceivedAttributePaths[0].mClusterId == Test::MockClusterId(3));
            NL_TEST_ASSERT(apSuite, delegate.mReceivedAttributePaths[0].mAttributeId == Test::MockAttributeId(1));
        }
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Verify that subscription can be shut down just after receiving SUBSCRIBE RESPONSE,
// before receiving any subsequent REPORT DATA.
void TestReadInteraction::TestSubscribeEarlyShutdown(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                  = *static_cast<TestContext *>(apContext);
    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    InteractionModelEngine & engine    = *InteractionModelEngine::GetInstance();
    MockInteractionModelApp delegate;

    // Initialize Interaction Model Engine
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(apSuite, engine.Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler) == CHIP_NO_ERROR);

    // Subscribe to the attribute
    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = kTestEndpointId;
    attributePathParams.mClusterId   = kTestClusterId;
    attributePathParams.mAttributeId = 1;

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = &attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;
    readPrepareParams.mMinIntervalFloorSeconds     = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds   = 5;
    readPrepareParams.mKeepSubscriptions           = false;

    printf("Send subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readPrepareParams) == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
        NL_TEST_ASSERT(apSuite, engine.GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine.ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine.ActiveHandlerAt(0);
        NL_TEST_ASSERT(apSuite, delegate.mpReadHandler != nullptr);
    }

    // Cleanup
    NL_TEST_ASSERT(apSuite, engine.GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    engine.Shutdown();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestSubscribeInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kInvalidTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mSessionHolder.Grab(ctx.GetSessionBobToAlice());
    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;
    printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readPrepareParams) == CHIP_NO_ERROR);

        delegate.mNumAttributeResponse = 0;

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);

        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        uint16_t minInterval;
        uint16_t maxInterval;
        delegate.mpReadHandler->GetReportingIntervals(minInterval, maxInterval);

        // Advance monotonic timestamp for min interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(maxInterval));
        ctx.GetIOContext().DriveIO();

        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().IsRunScheduled());
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().IsRunScheduled());

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadShutdown(nlTestSuite * apSuite, void * apContext)
{
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    app::ReadClient * pClients[4];
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    MockInteractionModelApp delegate;

    //
    // Allocate a number of clients
    //
    for (auto & client : pClients)
    {
        client = Platform::New<app::ReadClient>(engine, &ctx.GetExchangeManager(), delegate,
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

void TestReadInteraction::TestSubscribeInvalidInterval(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mSessionHolder.Grab(ctx.GetSessionBobToAlice());
    readPrepareParams.mMinIntervalFloorSeconds   = 6;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readPrepareParams) == CHIP_ERROR_INVALID_ARGUMENT);

        printf("\nSend subscribe request message to Node: 0x" ChipLogFormatX64 "\n", ChipLogValueX64(chip::kTestDeviceNodeId));

        ctx.DrainAndServiceIO();
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);

    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestPostSubscribeRoundtripStatusReportTimeout(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        err                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        GenerateEvents(apSuite, apContext);
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

        err = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Wait for max interval to elapse
        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMaxIntervalCeilingSeconds));
        ctx.GetIOContext().DriveIO();

        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        ctx.ExpireSessionBobToAlice();

        err = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().IsRunScheduled());

        ctx.DrainAndServiceIO();

        ctx.ExpireSessionAliceToBob();
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

void TestReadInteraction::TestSubscribeRoundtripStatusReportTimeout(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        err                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.ExpireSessionAliceToBob();

        ctx.DrainAndServiceIO();

        ctx.ExpireSessionBobToAlice();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

void TestReadInteraction::TestReadChunkingStatusReportTimeout(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.ExpireSessionAliceToBob();
        ctx.DrainAndServiceIO();
        ctx.ExpireSessionBobToAlice();

        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);
        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// ReadClient sends the read request, but handler fails to send the one report (SendMessage returns an error).
// Since this is an un-chunked read, we are not in the AwaitingReportResponse state, so the "reports in flight"
// counter should not increase.
void TestReadInteraction::TestReadReportFailure(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint2;
    attributePathParams[0].mClusterId   = Test::MockClusterId(3);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(1);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        ctx.GetLoopback().mNumMessagesToAllowBeforeError = 1;
        ctx.GetLoopback().mMessageSendError              = CHIP_ERROR_INCORRECT_STATE;
        err                                              = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);

        ctx.GetLoopback().mNumMessagesToAllowBeforeError = 0;
        ctx.GetLoopback().mMessageSendError              = CHIP_NO_ERROR;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestSubscribeRoundtripChunkStatusReportTimeout(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        err                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.ExpireSessionAliceToBob();
        ctx.DrainAndServiceIO();
        ctx.ExpireSessionBobToAlice();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

void TestReadInteraction::TestPostSubscribeRoundtripChunkStatusReportTimeout(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        err                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        GenerateEvents(apSuite, apContext);
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = Test::MockClusterId(2);
        dirtyPath1.mEndpointId  = Test::kMockEndpoint3;
        dirtyPath1.mAttributeId = Test::MockAttributeId(4);

        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMaxIntervalCeilingSeconds));
        ctx.GetIOContext().DriveIO();

        err = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        ctx.DrainAndServiceIO();
        // Drop status report for the first chunked report, then expire session, handler would be timeout
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 1);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);

        ctx.ExpireSessionAliceToBob();
        ctx.ExpireSessionBobToAlice();
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

void TestReadInteraction::TestPostSubscribeRoundtripChunkReportTimeout(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 1;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        err                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        GenerateEvents(apSuite, apContext);
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = Test::MockClusterId(2);
        dirtyPath1.mEndpointId  = Test::kMockEndpoint3;
        dirtyPath1.mAttributeId = Test::MockAttributeId(4);

        gMockClock.AdvanceMonotonic(System::Clock::Seconds16(readPrepareParams.mMaxIntervalCeilingSeconds));
        ctx.GetIOContext().DriveIO();

        err = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;

        // Drop second chunked report then expire session, client would be timeout
        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 2;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 1);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 3);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

        ctx.ExpireSessionAliceToBob();
        ctx.ExpireSessionBobToAlice();
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_ERROR_TIMEOUT);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

void TestReadInteraction::TestPostSubscribeRoundtripChunkReport(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
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
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 1;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        printf("\nSend first subscribe request message to Node: 0x" ChipLogFormatX64 "\n",
               ChipLogValueX64(chip::kTestDeviceNodeId));
        delegate.mGotReport = false;
        err                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        GenerateEvents(apSuite, apContext);
        chip::app::AttributePathParams dirtyPath1;
        dirtyPath1.mClusterId   = Test::MockClusterId(2);
        dirtyPath1.mEndpointId  = Test::kMockEndpoint3;
        dirtyPath1.mAttributeId = Test::MockAttributeId(4);

        err                            = engine->GetReportingEngine().SetDirty(dirtyPath1);
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        delegate.mNumArrayItems        = 0;

        // wait for min interval 1 seconds(in test, we use 0.9second considering the time variation), expect no event is
        // received, then wait for 0.5 seconds, then all chunked dirty reports are sent out, which would not honor minInterval
        gMockClock.AdvanceMonotonic(System::Clock::Milliseconds32(900));
        ctx.GetIOContext().DriveIO();

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
        System::Clock::Timestamp startTime = gMockClock.GetMonotonicTimestamp();

        // Increment in time is done by steps here to allow for multiple IO processing at the right time and allow the timer to
        // be rescheduled accordingly
        while (true)
        {
            ctx.GetIOContext().DriveIO();
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
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1 + (kMockAttribute4ListLength - 4));
    NL_TEST_ASSERT(apSuite, delegate.mNumArrayItems == 6);

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
}

namespace {

void CheckForInvalidAction(nlTestSuite * apSuite, Test::MessageCapturer & messageLog)
{
    NL_TEST_ASSERT(apSuite, messageLog.MessageCount() == 1);
    NL_TEST_ASSERT(apSuite, messageLog.IsMessageType(0, Protocols::InteractionModel::MsgType::StatusResponse));
    CHIP_ERROR status;
    NL_TEST_ASSERT(apSuite,
                   StatusResponse::ProcessStatusResponse(std::move(messageLog.MessagePayload(0)), status) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, status == CHIP_IM_GLOBAL_STATUS(InvalidAction));
}

} // anonymous namespace

/**
 * Helper macro we can use to pretend we got a reply from the server in cases
 * when the reply was actually dropped due to us not wanting the client's state
 * machine to advance.
 *
 * When this macro is used, the client has sent a message and is waiting for an
 * ack+response, and the server has sent a response that got dropped and is
 * waiting for an ack (and maybe a response).
 *
 * What this macro then needs to do is:
 *
 * 1. Pretend that the client got an ack (and clear out the corresponding ack
 *    state).
 * 2. Pretend that the client got a message from the server, with the id of the
 *    message that was dropped, which requires an ack, so the client will send
 *    that ack in its next message.
 *
 * This is a macro so we get useful line numbers on assertion failures
 */
#define PretendWeGotReplyFromServer(aSuite, aContext, aClientExchange)                                                             \
    {                                                                                                                              \
        Messaging::ReliableMessageMgr * localRm    = (aContext).GetExchangeManager().GetReliableMessageMgr();                      \
        Messaging::ExchangeContext * localExchange = aClientExchange;                                                              \
        NL_TEST_ASSERT(aSuite, localRm->TestGetCountRetransTable() == 2);                                                          \
                                                                                                                                   \
        localRm->ClearRetransTable(localExchange);                                                                                 \
        NL_TEST_ASSERT(aSuite, localRm->TestGetCountRetransTable() == 1);                                                          \
                                                                                                                                   \
        localRm->EnumerateRetransTable([localExchange](auto * entry) {                                                             \
            localExchange->SetPendingPeerAckMessageCounter(entry->retainedBuf.GetMessageCounter());                                \
            return Loop::Break;                                                                                                    \
        });                                                                                                                        \
    }

// Read Client sends the read request, Read Handler drops the response, then test injects unknown status reponse message for
// Read Client.
void TestReadInteraction::TestReadClientReceiveInvalidMessage(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

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
        app::ReadClient readClient(engine, &ctx.GetExchangeManager(), delegate, chip::app::ReadClient::InteractionType::Read);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        err                                                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        StatusResponseMessage::Builder response;
        response.Init(&writer);
        response.Status(Protocols::InteractionModel::Status::Busy);
        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

        Test::MessageCapturer messageLog(ctx);
        messageLog.mCaptureStandaloneAcks = false;

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        ctx.DrainAndServiceIO();

        // The ReadHandler closed its exchange when it sent the Report Data (which we dropped).
        // Since we synthesized the StatusResponse to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(Busy));

        CheckForInvalidAction(apSuite, messageLog);
    }

    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client sends the subscribe request, Read Handler drops the response, then test injects unknown status response message
// for Read Client.
void TestReadInteraction::TestSubscribeClientReceiveInvalidStatusResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        err                                                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        StatusResponseMessage::Builder response;
        response.Init(&writer);
        response.Status(Protocols::InteractionModel::Status::Busy);
        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        ctx.DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is closed when we synthesize the subscribe response, since it sent the
        // Subscribe Response as the last message in the transaction.
        // Since we synthesized the subscribe response to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(Busy));
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client sends the subscribe request, Read Handler drops the response, then test injects well-formed status response
// message with Success for Read Client, we expect the error with CHIP_ERROR_INVALID_MESSAGE_TYPE
void TestReadInteraction::TestSubscribeClientReceiveWellFormedStatusResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        err                                                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        StatusResponseMessage::Builder response;
        response.Init(&writer);
        response.Status(Protocols::InteractionModel::Status::Success);
        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        ctx.DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the StatusResponse.
        // Since we synthesized the StatusResponse to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_ERROR_INVALID_MESSAGE_TYPE);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client sends the subscribe request, Read Handler drops the response, then test injects invalid report message for Read
// Client.
void TestReadInteraction::TestSubscribeClientReceiveInvalidReportMessage(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        err                                                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        ctx.DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the ReportData.
        // Since we synthesized the ReportData to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_ERROR_END_OF_TLV);

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client create the subscription, handler sends unsolicited malformed report to client,
// InteractionModelEngine::OnUnsolicitedReportData would process this malformed report and sends out status report
void TestReadInteraction::TestSubscribeClientReceiveUnsolicitedInvalidReportMessage(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

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
        ctx.GetLoopback().mSentMessageCount = 0;
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 5);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);

        ctx.GetLoopback().mSentMessageCount = 0;
        auto exchange                       = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(
            delegate.mpReadHandler->mSessionHandle.Get().Value(), delegate.mpReadHandler);
        delegate.mpReadHandler->mExchangeCtx.Grab(exchange);
        err = delegate.mpReadHandler->mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(msgBuf),
                                                                Messaging::SendMessageFlags::kExpectResponse);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        // The server sends a data report.
        // The client receives the data report data and sends out status report with invalid action.
        // The server acks the status report.
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 3);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
}

// Read Client sends the subscribe request, Read Handler drops the subscribe response, then test injects invalid subscribe
// response message
void TestReadInteraction::TestSubscribeClientReceiveInvalidSubscribeResponseMessage(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

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
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 3;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        err                                                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        SubscribeResponseMessage::Builder response;
        response.Init(&writer);
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        response.MaxInterval(1);
        response.EndOfSubscribeResponseMessage();
        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::SubscribeResponse);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 4);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        ctx.DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the subscribe response.
        // Since we synthesized the subscribe response to the ReadClient, instead of sending it from the ReadHandler,
        // the only messages here are the ReadClient's StatusResponse to the unexpected message and an MRP ack.
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_ERROR_INVALID_SUBSCRIPTION);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client create the subscription, handler sends unsolicited malformed report with invalid subscription id to client,
// InteractionModelEngine::OnUnsolicitedReportData would process this malformed report and sends out status report
void TestReadInteraction::TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId(nlTestSuite * apSuite,
                                                                                                      void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

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
        ctx.GetLoopback().mSentMessageCount = 0;
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 5);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        response.EndOfReportDataMessage();

        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);

        ctx.GetLoopback().mSentMessageCount = 0;
        auto exchange                       = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(
            delegate.mpReadHandler->mSessionHandle.Get().Value(), delegate.mpReadHandler);
        delegate.mpReadHandler->mExchangeCtx.Grab(exchange);
        err = delegate.mpReadHandler->mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(msgBuf),
                                                                Messaging::SendMessageFlags::kExpectResponse);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        // The server sends a data report.
        // The client receives the data report data and sends out status report with invalid subsciption.
        // The server should respond with a status report of its own, leading to 4 messages (because
        // the client would ack the server's status report), just sends an ack to the status report it got.
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 3);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
}

// TestReadChunkingInvalidSubscriptionId will try to read a few large attributes, the report won't fit into the MTU and result
// in chunking, second report has different subscription id from the first one, read client sends out the status report with
// invalid subscription
void TestReadInteraction::TestReadChunkingInvalidSubscriptionId(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 3;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        err                                                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(msgBuf));
        ReportDataMessage::Builder response;
        response.Init(&writer);
        response.SubscriptionId(readClient.mSubscriptionId + 1);
        response.EndOfReportDataMessage();
        PayloadHeader payloadHeader;
        payloadHeader.SetExchangeID(0);
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::ReportData);

        NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 4);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 0;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        readClient.OnMessageReceived(readClient.mExchange.Get(), payloadHeader, std::move(msgBuf));
        ctx.DrainAndServiceIO();

        // TODO: Need to validate what status is being sent to the ReadHandler
        // The ReadHandler's exchange is still open when we synthesize the report data message.
        // Since we synthesized the second report data message to the ReadClient with invalid subscription id, instead of
        // sending it from the ReadHandler, the only messages here are the ReadClient's StatusResponse to the unexpected message
        // and an MRP ack.
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_ERROR_INVALID_SUBSCRIPTION);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client sends a malformed subscribe request, interaction model engine fails to parse the request and generates a status
// report to client, and client is closed.
void TestReadInteraction::TestReadHandlerMalformedSubscribeRequest(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);
        err = request.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = writer.Finalize(&msgBuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        NL_TEST_ASSERT(apSuite, exchange != nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);
        err = readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Read Client sends a malformed read request, interaction model engine fails to parse the request and generates a status report
// to client, and client is closed.
void TestReadInteraction::TestReadHandlerMalformedReadRequest1(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);
        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);
        err = request.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = writer.Finalize(&msgBuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        NL_TEST_ASSERT(apSuite, exchange != nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);
        err = readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Read Client sends a malformed read request, read handler fails to parse the request and generates a status report to client,
// and client is closed.
void TestReadInteraction::TestReadHandlerMalformedReadRequest2(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);
        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);
        err = request.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, request.EndOfReadRequestMessage() == CHIP_NO_ERROR);
        err = writer.Finalize(&msgBuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        NL_TEST_ASSERT(apSuite, exchange != nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);
        err = readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();
        ChipLogError(DataManagement, "The error is %s", ErrorStr(delegate.mError));
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Read Client creates a subscription with the server, server sends chunked reports, after the handler sends out the first
// chunked report, client sends out invalid write request message, handler sends status report with invalid action and closes
void TestReadInteraction::TestSubscribeSendUnknownMessage(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        ctx.GetLoopback().mDroppedMessageCount              = 0;
        err                                                 = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);

        ctx.GetLoopback().mSentMessageCount = 0;

        // Server sends out status report, client should send status report along with Piggybacking ack, but we don't do that
        // Instead, we send out unknown message to server

        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;
        chip::app::InitWriterWithSpaceReserved(writer, 0);
        request.Init(&writer);
        writer.Finalize(&msgBuf);

        err = readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::WriteRequest, std::move(msgBuf));
        ctx.DrainAndServiceIO();
        // client sends invalid write request, server sends out status report with invalid action and closes, client replies
        // with status report server replies with MRP Ack
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 4);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client creates a subscription with the server, server sends chunked reports, after the handler sends out invalid status
// report, client sends out invalid status report message, handler sends status report with invalid action and close
void TestReadInteraction::TestSubscribeSendInvalidStatusReport(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::app::AttributePathParams attributePathParams[1];
    // Mock Attribute 4 is a big attribute, with 6 large OCTET_STRING
    attributePathParams[0].mEndpointId  = Test::kMockEndpoint3;
    attributePathParams[0].mClusterId   = Test::MockClusterId(2);
    attributePathParams[0].mAttributeId = Test::MockAttributeId(4);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        ctx.GetLoopback().mSentMessageCount                 = 0;
        ctx.GetLoopback().mNumMessagesToDrop                = 1;
        ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
        ctx.GetLoopback().mDroppedMessageCount              = 0;

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        // Since we are dropping packets, things are not getting acked.  Set up
        // our MRP state to look like what it would have looked like if the
        // packet had not gotten dropped.
        PretendWeGotReplyFromServer(apSuite, ctx, readClient.mExchange.Get());

        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);
        ctx.GetLoopback().mSentMessageCount = 0;

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);

        System::PacketBufferHandle msgBuf;
        StatusResponseMessage::Builder request;
        System::PacketBufferTLVWriter writer;
        chip::app::InitWriterWithSpaceReserved(writer, 0);
        request.Init(&writer);
        writer.Finalize(&msgBuf);

        err = readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::StatusResponse, std::move(msgBuf));
        ctx.DrainAndServiceIO();

        // client sends malformed status response, server sends out status report with invalid action and close, client replies
        // with status report server replies with MRP Ack
        NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 4);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 0);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Read Client sends a malformed subscribe request, the server fails to parse the request and generates a status report to the
// client, and client closes itself.
void TestReadInteraction::TestReadHandlerInvalidSubscribeRequest(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);
        System::PacketBufferHandle msgBuf;
        ReadRequestMessage::Builder request;
        System::PacketBufferTLVWriter writer;

        chip::app::InitWriterWithSpaceReserved(writer, 0);
        err = request.Init(&writer);
        err = writer.Finalize(&msgBuf);

        auto exchange = readClient.mpExchangeMgr->NewContext(readPrepareParams.mSessionHolder.Get().Value(), &readClient);
        NL_TEST_ASSERT(apSuite, exchange != nullptr);
        readClient.mExchange.Grab(exchange);
        readClient.MoveToState(app::ReadClient::ClientState::AwaitingInitialReport);
        err = readClient.mExchange->SendMessage(Protocols::InteractionModel::MsgType::SubscribeRequest, std::move(msgBuf),
                                                Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// Create the subscription, then remove the corresponding fabric in client and handler, the corresponding
// client and handler would be released as well.
void TestReadInteraction::TestSubscribeInvalidateFabric(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = new chip::app::AttributePathParams[1];
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = Test::kMockEndpoint3;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = Test::MockClusterId(2);
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = Test::MockAttributeId(1);

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 0;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        err = readClient.SendAutoResubscribeRequest(std::move(readPrepareParams));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        ctx.GetFabricTable().Delete(ctx.GetAliceFabricIndex());
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 0);
        ctx.GetFabricTable().Delete(ctx.GetBobFabricIndex());
        NL_TEST_ASSERT(apSuite, delegate.mError == CHIP_ERROR_IM_FABRIC_DELETED);
        ctx.ExpireSessionAliceToBob();
        ctx.ExpireSessionBobToAlice();
        ctx.CreateAliceFabric();
        ctx.CreateBobFabric();
        ctx.CreateSessionAliceToBob();
        ctx.CreateSessionBobToAlice();
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestShutdownSubscription(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = new chip::app::AttributePathParams[1];
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = Test::kMockEndpoint3;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = Test::MockClusterId(2);
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = Test::MockAttributeId(1);

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 0;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        err = readClient.SendAutoResubscribeRequest(std::move(readPrepareParams));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        engine->ShutdownSubscription(chip::ScopedNodeId(readClient.GetPeerNodeId(), readClient.GetFabricIndex()),
                                     readClient.GetSubscriptionId().Value());
        NL_TEST_ASSERT(apSuite, readClient.IsIdle());
    }
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

/**
 * Tests what happens when a subscription tries to deliver reports but the
 * session it has is defunct.  Makes sure we correctly tear down the ReadHandler
 * and don't increment the "reports in flight" count.
 */
void TestReadInteraction::TestSubscriptionReportWithDefunctSession(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), gReportScheduler);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathParams subscribePath(Test::kMockEndpoint3, Test::MockClusterId(2), Test::MockAttributeId(1));

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = &subscribePath;
    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mMinIntervalFloorSeconds   = 0;
    readPrepareParams.mMaxIntervalCeilingSeconds = 0;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        err = readClient.SendSubscribeRequest(std::move(readPrepareParams));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Read) == 0);
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);

        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        auto * readHandler = engine->ActiveHandlerAt(0);

        // Verify that the session we will reset later is the one we will mess
        // with now.
        NL_TEST_ASSERT(apSuite, SessionHandle(*readHandler->GetSession()) == ctx.GetSessionAliceToBob());

        // Test that we send reports as needed.
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        engine->GetReportingEngine().SetDirty(subscribePath);
        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Read) == 0);
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);

        // Test that if the session is defunct we don't send reports and clean
        // up properly.
        readHandler->GetSession()->MarkAsDefunct();
        delegate.mGotReport            = false;
        delegate.mNumAttributeResponse = 0;
        engine->GetReportingEngine().SetDirty(subscribePath);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, !delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 0);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Read) == 0);
        NL_TEST_ASSERT(apSuite, engine->GetReportingEngine().GetNumReportsInFlight() == 0);
    }
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    // Get rid of our defunct session.
    ctx.ExpireSessionAliceToBob();
    ctx.CreateSessionAliceToBob();
}

} // namespace app
} // namespace chip

namespace {

const nlTest sTests[] = {
    NL_TEST_DEF("TestReadRoundtrip", chip::app::TestReadInteraction::TestReadRoundtrip),
    NL_TEST_DEF("TestReadRoundtripWithDataVersionFilter", chip::app::TestReadInteraction::TestReadRoundtripWithDataVersionFilter),
    NL_TEST_DEF("TestReadRoundtripWithNoMatchPathDataVersionFilter",
                chip::app::TestReadInteraction::TestReadRoundtripWithNoMatchPathDataVersionFilter),
    NL_TEST_DEF("TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter",
                chip::app::TestReadInteraction::TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter),
    NL_TEST_DEF("TestReadRoundtripWithSameDifferentPathsDataVersionFilter",
                chip::app::TestReadInteraction::TestReadRoundtripWithSameDifferentPathsDataVersionFilter),
    NL_TEST_DEF("TestReadWildcard", chip::app::TestReadInteraction::TestReadWildcard),
    NL_TEST_DEF("TestReadChunking", chip::app::TestReadInteraction::TestReadChunking),
    NL_TEST_DEF("TestSetDirtyBetweenChunks", chip::app::TestReadInteraction::TestSetDirtyBetweenChunks),
    NL_TEST_DEF("CheckReadClient", chip::app::TestReadInteraction::TestReadClient),
    NL_TEST_DEF("TestReadUnexpectedSubscriptionId", chip::app::TestReadInteraction::TestReadUnexpectedSubscriptionId),
    NL_TEST_DEF("CheckReadHandler", chip::app::TestReadInteraction::TestReadHandler),
    NL_TEST_DEF("TestReadClientGenerateAttributePathList", chip::app::TestReadInteraction::TestReadClientGenerateAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateInvalidAttributePathList",
                chip::app::TestReadInteraction::TestReadClientGenerateInvalidAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateOneEventPaths", chip::app::TestReadInteraction::TestReadClientGenerateOneEventPaths),
    NL_TEST_DEF("TestReadClientGenerateTwoEventPaths", chip::app::TestReadInteraction::TestReadClientGenerateTwoEventPaths),
    NL_TEST_DEF("TestReadClientInvalidReport", chip::app::TestReadInteraction::TestReadClientInvalidReport),
    NL_TEST_DEF("TestReadClientInvalidAttributeId", chip::app::TestReadInteraction::TestReadClientInvalidAttributeId),
    NL_TEST_DEF("TestReadHandlerInvalidAttributePath", chip::app::TestReadInteraction::TestReadHandlerInvalidAttributePath),
    NL_TEST_DEF("TestProcessSubscribeRequest", chip::app::TestReadInteraction::TestProcessSubscribeRequest),
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    NL_TEST_DEF("TestICDProcessSubscribeRequestSupMaxIntervalCeiling",
                chip::app::TestReadInteraction::TestICDProcessSubscribeRequestSupMaxIntervalCeiling),
    NL_TEST_DEF("TestICDProcessSubscribeRequestInfMaxIntervalCeiling",
                chip::app::TestReadInteraction::TestICDProcessSubscribeRequestInfMaxIntervalCeiling),
    NL_TEST_DEF("TestICDProcessSubscribeRequestSupMinInterval",
                chip::app::TestReadInteraction::TestICDProcessSubscribeRequestSupMinInterval),
    NL_TEST_DEF("TestICDProcessSubscribeRequestMaxMinInterval",
                chip::app::TestReadInteraction::TestICDProcessSubscribeRequestMaxMinInterval),
    NL_TEST_DEF("TestICDProcessSubscribeRequestInvalidIdleModeDuration",
                chip::app::TestReadInteraction::TestICDProcessSubscribeRequestInvalidIdleModeDuration),
#endif // #if CHIP_CONFIG_ENABLE_ICD_SERVER
    NL_TEST_DEF("TestSubscribeRoundtrip", chip::app::TestReadInteraction::TestSubscribeRoundtrip),
    NL_TEST_DEF("TestSubscribeEarlyReport", chip::app::TestReadInteraction::TestSubscribeEarlyReport),
    NL_TEST_DEF("TestPostSubscribeRoundtripChunkReport", chip::app::TestReadInteraction::TestPostSubscribeRoundtripChunkReport),
    NL_TEST_DEF("TestReadClientReceiveInvalidMessage", chip::app::TestReadInteraction::TestReadClientReceiveInvalidMessage),
    NL_TEST_DEF("TestSubscribeClientReceiveInvalidStatusResponse",
                chip::app::TestReadInteraction::TestSubscribeClientReceiveInvalidStatusResponse),
    NL_TEST_DEF("TestSubscribeClientReceiveWellFormedStatusResponse",
                chip::app::TestReadInteraction::TestSubscribeClientReceiveWellFormedStatusResponse),
    NL_TEST_DEF("TestSubscribeClientReceiveInvalidReportMessage",
                chip::app::TestReadInteraction::TestSubscribeClientReceiveInvalidReportMessage),
    NL_TEST_DEF("TestSubscribeClientReceiveUnsolicitedInvalidReportMessage",
                chip::app::TestReadInteraction::TestSubscribeClientReceiveUnsolicitedInvalidReportMessage),
    NL_TEST_DEF("TestSubscribeClientReceiveInvalidSubscribeResponseMessage",
                chip::app::TestReadInteraction::TestSubscribeClientReceiveInvalidSubscribeResponseMessage),
    NL_TEST_DEF("TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId",
                chip::app::TestReadInteraction::TestSubscribeClientReceiveUnsolicitedReportMessageWithInvalidSubscriptionId),
    NL_TEST_DEF("TestReadChunkingInvalidSubscriptionId", chip::app::TestReadInteraction::TestReadChunkingInvalidSubscriptionId),
    NL_TEST_DEF("TestReadHandlerMalformedReadRequest1", chip::app::TestReadInteraction::TestReadHandlerMalformedReadRequest1),
    NL_TEST_DEF("TestReadHandlerMalformedReadRequest2", chip::app::TestReadInteraction::TestReadHandlerMalformedReadRequest2),
    NL_TEST_DEF("TestReadHandlerMalformedSubscribeRequest",
                chip::app::TestReadInteraction::TestReadHandlerMalformedSubscribeRequest),
    NL_TEST_DEF("TestSubscribeSendUnknownMessage", chip::app::TestReadInteraction::TestSubscribeSendUnknownMessage),
    NL_TEST_DEF("TestSubscribeSendInvalidStatusReport", chip::app::TestReadInteraction::TestSubscribeSendInvalidStatusReport),
    NL_TEST_DEF("TestReadHandlerInvalidSubscribeRequest", chip::app::TestReadInteraction::TestReadHandlerInvalidSubscribeRequest),
    NL_TEST_DEF("TestSubscribeInvalidateFabric", chip::app::TestReadInteraction::TestSubscribeInvalidateFabric),
    NL_TEST_DEF("TestShutdownSubscription", chip::app::TestReadInteraction::TestShutdownSubscription),
    NL_TEST_DEF("TestSubscribeUrgentWildcardEvent", chip::app::TestReadInteraction::TestSubscribeUrgentWildcardEvent),
    NL_TEST_DEF("TestSubscribeWildcard", chip::app::TestReadInteraction::TestSubscribeWildcard),
    NL_TEST_DEF("TestSubscribePartialOverlap", chip::app::TestReadInteraction::TestSubscribePartialOverlap),
    NL_TEST_DEF("TestSubscribeSetDirtyFullyOverlap", chip::app::TestReadInteraction::TestSubscribeSetDirtyFullyOverlap),
    NL_TEST_DEF("TestSubscribeEarlyShutdown", chip::app::TestReadInteraction::TestSubscribeEarlyShutdown),
    NL_TEST_DEF("TestSubscribeInvalidAttributePathRoundtrip",
                chip::app::TestReadInteraction::TestSubscribeInvalidAttributePathRoundtrip),
    NL_TEST_DEF("TestReadInvalidAttributePathRoundtrip", chip::app::TestReadInteraction::TestReadInvalidAttributePathRoundtrip),
    NL_TEST_DEF("TestSubscribeInvalidInterval", chip::app::TestReadInteraction::TestSubscribeInvalidInterval),
    NL_TEST_DEF("TestSubscribeRoundtripStatusReportTimeout",
                chip::app::TestReadInteraction::TestSubscribeRoundtripStatusReportTimeout),
    NL_TEST_DEF("TestPostSubscribeRoundtripStatusReportTimeout",
                chip::app::TestReadInteraction::TestPostSubscribeRoundtripStatusReportTimeout),
    NL_TEST_DEF("TestReadChunkingStatusReportTimeout", chip::app::TestReadInteraction::TestReadChunkingStatusReportTimeout),
    NL_TEST_DEF("TestReadReportFailure", chip::app::TestReadInteraction::TestReadReportFailure),
    NL_TEST_DEF("TestSubscribeRoundtripChunkStatusReportTimeout",
                chip::app::TestReadInteraction::TestSubscribeRoundtripChunkStatusReportTimeout),
    NL_TEST_DEF("TestPostSubscribeRoundtripChunkStatusReportTimeout",
                chip::app::TestReadInteraction::TestPostSubscribeRoundtripChunkStatusReportTimeout),
    NL_TEST_DEF("TestPostSubscribeRoundtripChunkReportTimeout",
                chip::app::TestReadInteraction::TestPostSubscribeRoundtripChunkReportTimeout),
    NL_TEST_DEF("TestReadShutdown", chip::app::TestReadInteraction::TestReadShutdown),
    NL_TEST_DEF("TestSubscriptionReportWithDefunctSession",
                chip::app::TestReadInteraction::TestSubscriptionReportWithDefunctSession),
    NL_TEST_SENTINEL(),
};

nlTestSuite sSuite = {
    "TestReadInteraction",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};

nlTestSuite sSyncSuite = {
    "TestSyncReadInteraction",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite_Sync,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};

} // namespace

int TestReadInteraction()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

int TestSyncReadInteraction()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSyncSuite);
}

CHIP_REGISTER_TEST_SUITE(TestReadInteraction)
CHIP_REGISTER_TEST_SUITE(TestSyncReadInteraction)
