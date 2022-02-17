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
#include <app/AttributeAccessInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/tests/AppTestContext.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>

#include <nlunit-test.h>

#include <type_traits>

namespace {
uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];
chip::ClusterId kTestClusterId          = 6;
chip::ClusterId kInvalidTestClusterId   = 7;
chip::EndpointId kTestEndpointId        = 1;
chip::EventId kTestEventIdDebug         = 1;
chip::EventId kTestEventIdCritical      = 2;
uint8_t kTestFieldValue1                = 1;
chip::TLV::Tag kTestEventTag            = chip::TLV::ContextTag(1);
chip::EndpointId kInvalidTestEndpointId = 3;
chip::DataVersion kTestDataVersion1     = 3;
chip::DataVersion kTestDataVersion2     = 5;

class TestContext : public chip::Test::AppContext
{
public:
    static int Initialize(void * context)
    {
        if (AppContext::Initialize(context) != SUCCESS)
            return FAILURE;

        auto * ctx = static_cast<TestContext *>(context);

        chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        chip::app::EventManagement::CreateEventManagement(&ctx->GetExchangeManager(), ArraySize(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, nullptr, 0, nullptr);

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        chip::app::EventManagement::DestroyEventManagement();

        if (AppContext::Finalize(context) != SUCCESS)
            return FAILURE;

        return SUCCESS;
    }
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

void GenerateEvents(nlTestSuite * apSuite, void * apContext, bool aIsUrgent = false)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EventNumber eid1, eid2;
    chip::app::EventOptions options1;
    options1.mPath     = { kTestEndpointId, kTestClusterId, kTestEventIdDebug };
    options1.mPriority = chip::app::PriorityLevel::Info;

    chip::app::EventOptions options2;
    options2.mPath     = { kTestEndpointId, kTestClusterId, kTestEventIdCritical };
    options2.mPriority = chip::app::PriorityLevel::Critical;
    TestEventGenerator testEventGenerator;
    if (aIsUrgent)
    {
        options2.mUrgent = chip::app::EventOptions::Type::kUrgent;
    }
    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
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
    }

    void OnAttributeData(const chip::app::ConcreteDataAttributePath & aPath, chip::TLV::TLVReader * apData,
                         const chip::app::StatusIB & status) override
    {
        if (status.mStatus == chip::Protocols::InteractionModel::Status::Success)
        {
            mNumAttributeResponse++;
            mGotReport = true;
        }
    }

    void OnError(CHIP_ERROR aError) override { mReadError = true; }

    void OnDone() override {}

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
    int mNumAttributeResponse              = 0;
    bool mGotReport                        = false;
    bool mReadError                        = false;
    chip::app::ReadHandler * mpReadHandler = nullptr;
};

//
// This dummy callback is used with a bunch of the tests below that don't go through
// the normal call-path of having the IM engine allocate the ReadHandler object. Instead,
// the object is allocated on stack for the purposes of a very narrow, tightly-coupled test.
//
// The typical callback implementor is the engine, but that would proceed to return the object
// back to the handler pool (which we obviously don't want in this case). This just no-ops those calls.
//
class NullReadHandlerCallback : public chip::app::ReadHandler::Callback
{
public:
    void OnDone(chip::app::ReadHandler & apReadHandlerObj) override {}
};

} // namespace

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
        attributeStatus.EndOfAttributeStatusIB();
        ReturnErrorOnFailure(attributeStatus.GetError());
        return attributeReport.EndOfAttributeReportIB().GetError();
    }

    return AttributeValueEncoder(aAttributeReports, 0, aPath, 0).Encode(kTestFieldValue1);
}

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    if (kTestDataVersion1 == aRequiredVersion)
    {
        return true;
    }
    else
    {
        return false;
    }
}

class TestReadInteraction
{
public:
    static void TestReadClient(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateAttributePathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateInvalidAttributePathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateOneEventPaths(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateTwoEventPaths(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientInvalidReport(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerInvalidAttributePath(nlTestSuite * apSuite, void * apContext);
    static void TestProcessSubscribeRequest(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithNoMatchPathDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtripWithSameDifferentPathsDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadWildcard(nlTestSuite * apSuite, void * apContext);
    static void TestReadChunking(nlTestSuite * apSuite, void * apContext);
    static void TestSetDirtyBetweenChunks(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeWildcard(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeEarlyShutdown(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestReadInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidIterval(nlTestSuite * apSuite, void * apContext);
    static void TestReadShutdown(nlTestSuite * apSuite, void * apContext);
    static void TestResubscribeRoundtrip(nlTestSuite * apSuite, void * apContext);

private:
    static void GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                   bool aNeedInvalidReport, bool aSuppressResponse);
};

void TestReadInteraction::GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                             bool aNeedInvalidReport, bool aSuppressResponse)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    ReportDataMessage::Builder reportDataMessageBuilder;

    err = reportDataMessageBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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

    if (aNeedInvalidReport)
    {
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).ListIndex(5).EndOfAttributePathIB();
    }
    else
    {
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

    GenerateReportData(apSuite, apContext, buf, false /*aNeedInvalidReport*/, true /* aSuppressResponse*/);
    err = readClient.ProcessReportData(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadHandler(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    MockInteractionModelApp delegate;
    NullReadHandlerCallback nullCallback;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
        ReadHandler readHandler(nullCallback, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);

        GenerateReportData(apSuite, apContext, reportDatabuf, false /*aNeedInvalidReport*/, false /* aSuppressResponse*/);
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

        err = readHandler.OnInitialRequest(std::move(readRequestbuf));
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
    attributePathParams[0].mAttributeId                  = 0;
    attributePathParams[1].mAttributeId                  = 0;
    attributePathParams[1].mListIndex                    = 0;
    AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
    err = readClient.GenerateAttributePathList(attributePathListBuilder, attributePathParams, 2 /*aAttributePathParamsListSize*/);
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
    attributePathParams[0].mAttributeId                  = 0;
    attributePathParams[1].mListIndex                    = 0;
    AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
    err = readClient.GenerateAttributePathList(attributePathListBuilder, attributePathParams, 2 /*aAttributePathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
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

    GenerateReportData(apSuite, apContext, buf, true /*aNeedInvalidReport*/, true /* aSuppressResponse*/);

    err = readClient.ProcessReportData(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
}

void TestReadInteraction::TestReadHandlerInvalidAttributePath(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    MockInteractionModelApp delegate;
    NullReadHandlerCallback nullCallback;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
        ReadHandler readHandler(nullCallback, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);

        GenerateReportData(apSuite, apContext, reportDatabuf, false /*aNeedInvalidReport*/, false /* aSuppressResponse*/);
        err = readHandler.SendReportData(std::move(reportDatabuf), false);
        NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

        writer.Init(std::move(readRequestbuf));
        err = readRequestBuilder.Init(&writer);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        AttributePathIBs::Builder & attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        AttributePathIB::Builder & attributePathBuilder = attributePathListBuilder.CreatePath();
        NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).ListIndex(5).EndOfAttributePathIB();
        err = attributePathBuilder.GetError();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        attributePathListBuilder.EndOfAttributePathIBs();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        readRequestBuilder.IsFabricFiltered(false).EndOfReadRequestMessage();
        NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
        err = writer.Finalize(&readRequestbuf);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = readHandler.OnInitialRequest(std::move(readRequestbuf));
        NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

        //
        // In the call above to OnInitialRequest, the handler will not actually close out the EC since
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
    err = readClient.GenerateEventPaths(eventPathListBuilder, eventPathParams, 1 /*aEventPathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    request.IsFabricFiltered(false).EndOfReadRequestMessage();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == request.GetError());

    err = writer.Finalize(&msgBuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    reader.Next();

    err = readRequestParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
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
    err = readClient.GenerateEventPaths(eventPathListBuilder, eventPathParams, 2 /*aEventPathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    request.IsFabricFiltered(false).EndOfReadRequestMessage();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == request.GetError());

    err = writer.Finalize(&msgBuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequestMessage::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    reader.Next();

    err = readRequestParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
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
    err           = engine->Init(&ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::EventPathParams eventPathParams[1];
    eventPathParams[0].mEndpointId = kTestEndpointId;
    eventPathParams[0].mClusterId  = kTestClusterId;

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
    readPrepareParams.mEventNumber                 = 1;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    err           = engine->Init(&ctx.GetExchangeManager());
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    err           = engine->Init(&ctx.GetExchangeManager());
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    err           = engine->Init(&ctx.GetExchangeManager());
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    err           = engine->Init(&ctx.GetExchangeManager());
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    err           = engine->Init(&ctx.GetExchangeManager());
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    err           = engine->Init(&ctx.GetExchangeManager());
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 7); // One empty string, with 6 array evelemtns.
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

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[2];
    for (int i = 0; i < 2; i++)
    {
        attributePathParams[i].mEndpointId  = Test::kMockEndpoint3;
        attributePathParams[i].mClusterId   = Test::MockClusterId(2);
        attributePathParams[i].mAttributeId = Test::MockAttributeId(4);
    }

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ClusterInfo dirtyPath;
        dirtyPath.mEndpointId  = Test::kMockEndpoint3;
        dirtyPath.mClusterId   = Test::MockClusterId(2);
        dirtyPath.mAttributeId = Test::MockAttributeId(4);

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

        int currentAttributeResponse = delegate.mNumAttributeResponse;

        // At this time, we should have sent 3 chunks, and are in the middle of report for second item.
        InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath);

        for (int i = 0; i < 5; i++)
        {
            // Then start from the beginning. 5 (the loop termination condition) needs to be big enough to let the report engine
            // send all report chunks.
            InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        }

        // We should receive another 2 * (6 + 1) = 14 attribute reports since the underlying path iterator should be reset.
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == currentAttributeResponse + 14);
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
    err           = engine->Init(&ctx.GetExchangeManager());
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

        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

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
    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);

    {
        ReadHandler readHandler(*engine, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);

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

        subscribeRequestBuilder.IsProxy(true);
        NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

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

void TestReadInteraction::TestSubscribeRoundtrip(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    chip::app::EventPathParams eventPathParams[2];
    readPrepareParams.mpEventPathParamsList                = eventPathParams;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestClusterId;
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
    printf("\nSend first subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        delegate.mGotReport = false;
        engine->GetReportingEngine().Run();

        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers() == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);

        GenerateEvents(apSuite, apContext, true /*aIsUrgent*/);
        NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->mHoldReport == false);
        NL_TEST_ASSERT(apSuite, delegate.mpReadHandler->mDirty == true);
        chip::app::ClusterInfo dirtyPath1;
        dirtyPath1.mClusterId   = kTestClusterId;
        dirtyPath1.mEndpointId  = kTestEndpointId;
        dirtyPath1.mAttributeId = 1;

        chip::app::ClusterInfo dirtyPath2;
        dirtyPath2.mClusterId   = kTestClusterId;
        dirtyPath2.mEndpointId  = kTestEndpointId;
        dirtyPath2.mAttributeId = 2;

        chip::app::ClusterInfo dirtyPath3;
        dirtyPath3.mClusterId   = kTestClusterId;
        dirtyPath3.mEndpointId  = kTestEndpointId;
        dirtyPath3.mAttributeId = 2;
        dirtyPath3.mListIndex   = 1;

        chip::app::ClusterInfo dirtyPath4;
        dirtyPath4.mClusterId   = kTestClusterId;
        dirtyPath4.mEndpointId  = kTestEndpointId;
        dirtyPath4.mAttributeId = 3;

        chip::app::ClusterInfo dirtyPath5;
        dirtyPath5.mClusterId   = kTestClusterId;
        dirtyPath5.mEndpointId  = kTestEndpointId;
        dirtyPath5.mAttributeId = 4;

        // Test report with 2 different path
        delegate.mpReadHandler->mHoldReport = false;
        delegate.mGotReport                 = false;
        delegate.mNumAttributeResponse      = 0;

        printf("HereHere\n");

        err = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        engine->GetReportingEngine().Run();
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Test report with 2 different path, and 1 same path
        delegate.mpReadHandler->mHoldReport = false;
        delegate.mGotReport                 = false;
        delegate.mNumAttributeResponse      = 0;
        err                                 = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        engine->GetReportingEngine().Run();
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Test report with 3 different path, and one path is overlapped with another
        delegate.mpReadHandler->mHoldReport = false;
        delegate.mGotReport                 = false;
        delegate.mNumAttributeResponse      = 0;
        err                                 = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath3);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        engine->GetReportingEngine().Run();
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Test report with 3 different path, all are not overlapped, one path is not interested for current subscription
        delegate.mpReadHandler->mHoldReport = false;
        delegate.mGotReport                 = false;
        delegate.mNumAttributeResponse      = 0;
        err                                 = engine->GetReportingEngine().SetDirty(dirtyPath1);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath2);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = engine->GetReportingEngine().SetDirty(dirtyPath4);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        engine->GetReportingEngine().Run();
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

        // Test empty report
        delegate.mpReadHandler->mHoldReport = false;
        delegate.mpReadHandler->mHoldSync   = false;
        delegate.mGotReport                 = false;
        delegate.mNumAttributeResponse      = 0;
        engine->GetReportingEngine().Run();
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
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
    err           = engine->Init(&ctx.GetExchangeManager());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    chip::app::AttributePathParams * attributePathParams = new chip::app::AttributePathParams[2];
    // Subscribe to full wildcard paths, repeat twice to ensure chunking.
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        delegate.mGotReport = false;

        err = readClient.SendAutoResubscribeRequest(std::move(readPrepareParams));
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        for (int i = 0; i < 20 && delegate.mNumAttributeResponse < 70; i++)
        {
            // 20 is a magic number, we assume the initial reports will take no more than 10 chunks.
            engine->GetReportingEngine().Run();
        }
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);

        // We have 29 attributes in our mock attribute storage. And we subscribed twice.
        // And attribute 3/2/4 is a list with 6 elements and list chunking is applied to it, thus we should receive ( 29 + 6 ) * 2 =
        // 70 attribute data in total.
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 70);
        NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        // Set a concrete path dirty
        {
            delegate.mpReadHandler->mHoldReport = false;
            delegate.mGotReport                 = false;
            delegate.mNumAttributeResponse      = 0;

            ClusterInfo dirtyPath;
            dirtyPath.mEndpointId  = Test::kMockEndpoint2;
            dirtyPath.mClusterId   = Test::MockClusterId(3);
            dirtyPath.mAttributeId = Test::MockAttributeId(1);

            err = engine->GetReportingEngine().SetDirty(dirtyPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
            engine->GetReportingEngine().Run();
            NL_TEST_ASSERT(apSuite, delegate.mGotReport);
            // We subscribed wildcard path twice, so we will receive two reports here.
            NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
        }

        // Set a endpoint dirty
        {
            delegate.mpReadHandler->mHoldReport = false;
            delegate.mGotReport                 = false;
            delegate.mNumAttributeResponse      = 0;

            ClusterInfo dirtyPath;
            dirtyPath.mEndpointId = Test::kMockEndpoint3;

            err = engine->GetReportingEngine().SetDirty(dirtyPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            for (int i = 0; i < 10 && delegate.mNumAttributeResponse < 38; i++)
            {
                delegate.mpReadHandler->mHoldReport = false;
                // 10 is a magic number, we assume the report will use no more than 10 chunks.
                engine->GetReportingEngine().Run();
            }

            NL_TEST_ASSERT(apSuite, delegate.mGotReport);
            // Mock endpoint3 has 13 attributes in total, and we subscribed twice.
            // And attribute 3/2/4 is a list with 6 elements and list chunking is applied to it, thus we should receive ( 13 + 6 ) *
            // 2 = 28 attribute data in total.
            NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 38);
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
    NL_TEST_ASSERT(apSuite, engine.Init(&ctx.GetExchangeManager()) == CHIP_NO_ERROR);

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

    printf("Send subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readPrepareParams) == CHIP_NO_ERROR);

        engine.GetReportingEngine().Run();
        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
        NL_TEST_ASSERT(apSuite, engine.GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe) == 1);
        NL_TEST_ASSERT(apSuite, engine.ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine.ActiveHandlerAt(0);
        NL_TEST_ASSERT(apSuite, delegate.mpReadHandler != nullptr);

        // Shutdown the subscription
        readClient.Abort();
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
    err           = engine->Init(&ctx.GetExchangeManager());
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
    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        NL_TEST_ASSERT(apSuite, readClient.SendRequest(readPrepareParams) == CHIP_NO_ERROR);

        delegate.mNumAttributeResponse = 0;
        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);

        NL_TEST_ASSERT(apSuite, engine->ActiveHandlerAt(0) != nullptr);
        delegate.mpReadHandler = engine->ActiveHandlerAt(0);

        delegate.mpReadHandler->mHoldReport = false;
        InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    for (int i = 0; i < 4; i++)
    {
        pClients[i] = Platform::New<app::ReadClient>(engine, &ctx.GetExchangeManager(), delegate,
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

void TestReadInteraction::TestSubscribeInvalidIterval(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager());
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

        printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);
    }

    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);

    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

} // namespace app
} // namespace chip

namespace {

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestReadRoundtrip", chip::app::TestReadInteraction::TestReadRoundtrip),
    NL_TEST_DEF("TestReadRoundtripWithDataVersionFilter", chip::app::TestReadInteraction::TestReadRoundtripWithDataVersionFilter),
    NL_TEST_DEF("TestReadRoundtripWithNoMatchPathDataVersionFilter", chip::app::TestReadInteraction::TestReadRoundtripWithNoMatchPathDataVersionFilter),
    NL_TEST_DEF("TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter", chip::app::TestReadInteraction::TestReadRoundtripWithMultiSamePathDifferentDataVersionFilter),
    NL_TEST_DEF("TestReadRoundtripWithSameDifferentPathsDataVersionFilter", chip::app::TestReadInteraction::TestReadRoundtripWithSameDifferentPathsDataVersionFilter),
    NL_TEST_DEF("TestReadWildcard", chip::app::TestReadInteraction::TestReadWildcard),
    NL_TEST_DEF("TestReadChunking", chip::app::TestReadInteraction::TestReadChunking),
    NL_TEST_DEF("TestSetDirtyBetweenChunks", chip::app::TestReadInteraction::TestSetDirtyBetweenChunks),
    NL_TEST_DEF("CheckReadClient", chip::app::TestReadInteraction::TestReadClient),
    NL_TEST_DEF("CheckReadHandler", chip::app::TestReadInteraction::TestReadHandler),
    NL_TEST_DEF("TestReadClientGenerateAttributePathList", chip::app::TestReadInteraction::TestReadClientGenerateAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateInvalidAttributePathList", chip::app::TestReadInteraction::TestReadClientGenerateInvalidAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateOneEventPaths", chip::app::TestReadInteraction::TestReadClientGenerateOneEventPaths),
    NL_TEST_DEF("TestReadClientGenerateTwoEventPaths", chip::app::TestReadInteraction::TestReadClientGenerateTwoEventPaths),
    NL_TEST_DEF("TestReadClientInvalidReport", chip::app::TestReadInteraction::TestReadClientInvalidReport),
    NL_TEST_DEF("TestReadHandlerInvalidAttributePath", chip::app::TestReadInteraction::TestReadHandlerInvalidAttributePath),
    NL_TEST_DEF("TestProcessSubscribeRequest", chip::app::TestReadInteraction::TestProcessSubscribeRequest),
    NL_TEST_DEF("TestSubscribeRoundtrip", chip::app::TestReadInteraction::TestSubscribeRoundtrip),
    NL_TEST_DEF("TestSubscribeWildcard", chip::app::TestReadInteraction::TestSubscribeWildcard),
    NL_TEST_DEF("TestSubscribeEarlyShutdown", chip::app::TestReadInteraction::TestSubscribeEarlyShutdown),
    NL_TEST_DEF("TestSubscribeInvalidAttributePathRoundtrip", chip::app::TestReadInteraction::TestSubscribeInvalidAttributePathRoundtrip),
    NL_TEST_DEF("TestReadInvalidAttributePathRoundtrip", chip::app::TestReadInteraction::TestReadInvalidAttributePathRoundtrip),
    NL_TEST_DEF("TestSubscribeInvalidIterval", chip::app::TestReadInteraction::TestSubscribeInvalidIterval),
    NL_TEST_DEF("TestReadShutdown", chip::app::TestReadInteraction::TestReadShutdown),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestReadInteraction",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestReadInteraction()
{
    TestContext gContext;
    nlTestRunner(&sSuite, &gContext);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReadInteraction)
