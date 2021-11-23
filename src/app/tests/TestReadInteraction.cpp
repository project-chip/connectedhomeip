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

#include <app/AttributeAccessInterface.h>
#include <app/InteractionModelEngine.h>
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
chip::NodeId kTestNodeId              = 1;
chip::ClusterId kTestClusterId        = 6;
chip::ClusterId kInvalidTestClusterId = 7;
chip::EndpointId kTestEndpointId      = 1;
chip::EventId kTestEventIdDebug       = 1;
chip::EventId kTestEventIdCritical    = 2;
uint8_t kTestFieldValue1              = 1;
chip::TLV::Tag kTestEventTag          = chip::TLV::ContextTag(1);

class TestContext : public chip::Test::AppContext
{
public:
    static int Initialize(void * context)
    {
        if (AppContext::Initialize(context) != SUCCESS)
            return FAILURE;

        auto * ctx = static_cast<TestContext *>(context);

        chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Critical },
        };

        chip::app::EventManagement::CreateEventManagement(&ctx->GetExchangeManager(), ArraySize(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources);

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
        CHIP_ERROR err = CHIP_NO_ERROR;
        err            = aWriter.Put(kTestEventTag, mStatus);
        return err;
    }

    void SetStatus(int32_t aStatus) { mStatus = aStatus; }

private:
    int32_t mStatus;
};

void GenerateEvents(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EventNumber eid1, eid2;
    chip::app::EventSchema schema1 = { kTestNodeId, kTestEndpointId, kTestClusterId, kTestEventIdDebug,
                                       chip::app::PriorityLevel::Info };
    chip::app::EventSchema schema2 = { kTestNodeId, kTestEndpointId, kTestClusterId, kTestEventIdCritical,
                                       chip::app::PriorityLevel::Critical };
    chip::app::EventOptions options1;
    chip::app::EventOptions options2;
    TestEventGenerator testEventGenerator;

    options1.mpEventSchema               = &schema1;
    options2.mpEventSchema               = &schema2;
    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options1, eid1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options2, eid2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void GenerateSubscribeResponse(nlTestSuite * apSuite, void * apContext, chip::System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    chip::app::SubscribeResponseMessage::Builder subscribeResponseBuilder;

    err = subscribeResponseBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    subscribeResponseBuilder.SubscriptionId(0);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MinIntervalFloorSeconds(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MaxIntervalCeilingSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.EndOfSubscribeResponseMessage();
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

class MockInteractionModelApp : public chip::app::ReadClient::Callback, public chip::app::InteractionModelDelegate
{
public:
    void OnEventData(const chip::app::ReadClient * apReadClient, const chip::app::EventHeader & aEventHeader,
                     chip::TLV::TLVReader * apData, const chip::app::StatusIB * apStatus) override
    {
        static int numDataElementIndex = 0;

        if (numDataElementIndex == 0)
        {
            VerifyOrReturn(aEventHeader.mPriorityLevel == chip::app::PriorityLevel::Critical);
        }
        else if (numDataElementIndex == 1)
        {
            VerifyOrReturn(aEventHeader.mPriorityLevel == chip::app::PriorityLevel::Info);
        }

        ++numDataElementIndex;
        mGotEventResponse = true;
    }

    void OnAttributeData(const chip::app::ReadClient * apReadClient, const chip::app::ConcreteDataAttributePath & aPath,
                         chip::TLV::TLVReader * apData, const chip::app::StatusIB & status) override
    {
        if (status.mStatus == chip::Protocols::InteractionModel::Status::Success)
        {
            mNumAttributeResponse++;
            mGotReport = true;
        }
    }

    void OnError(const chip::app::ReadClient * apReadClient, CHIP_ERROR aError) override { mReadError = true; }

    void OnDone(chip::app::ReadClient * apReadClient) override {}

    CHIP_ERROR SubscriptionEstablished(const chip::app::ReadHandler * apReadHandler) override
    {
        mpReadHandler = const_cast<chip::app::ReadHandler *>(apReadHandler);
        mNumSubscriptions++;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SubscriptionTerminated(const chip::app::ReadHandler * apReadHandler) override
    {
        if (apReadHandler->IsActiveSubscription())
        {
            mNumSubscriptions--;
        }
        return CHIP_NO_ERROR;
    }

    bool mGotEventResponse                 = false;
    int mNumAttributeResponse              = 0;
    bool mGotReport                        = false;
    bool mReadError                        = false;
    uint32_t mNumSubscriptions             = 0;
    chip::app::ReadHandler * mpReadHandler = nullptr;
};
} // namespace

namespace chip {
namespace app {
CHIP_ERROR ReadSingleClusterData(FabricIndex aAccessingFabricIndex, const ConcreteAttributePath & aPath,
                                 AttributeReportIB::Builder & aAttributeReport)
{
    AttributeDataIB::Builder attributeData;
    AttributePathIB::Builder attributePath;
    ChipLogDetail(DataManagement, "TEST Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", aPath.mClusterId, aPath.mAttributeId);

    if (aPath.mClusterId >= Test::kMockEndpointMin)
    {
        return Test::ReadSingleMockClusterData(aAccessingFabricIndex, aPath, aAttributeReport);
    }

    if (!(aPath.mClusterId == kTestClusterId && aPath.mEndpointId == kTestEndpointId))
    {
        AttributeStatusIB::Builder attributeStatus;
        attributeStatus = aAttributeReport.CreateAttributeStatus();
        attributePath   = attributeStatus.CreatePath();
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());
        StatusIB::Builder errorStatus = attributeStatus.CreateErrorStatus();
        errorStatus.EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::UnsupportedAttribute));
        attributeStatus.EndOfAttributeStatusIB();
        ReturnErrorOnFailure(attributeStatus.GetError());
        return CHIP_NO_ERROR;
    }

    attributeData = aAttributeReport.CreateAttributeData();
    attributePath = attributeData.CreatePath();
    attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
    ReturnErrorOnFailure(attributePath.GetError());
    ReturnErrorOnFailure(AttributeValueEncoder(attributeData.GetWriter(), 0).Encode(kTestFieldValue1));
    attributeData.DataVersion(0).EndOfAttributeDataIB();
    ReturnErrorOnFailure(attributeData.GetError());
    return CHIP_NO_ERROR;
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
    static void TestProcessSubscribeResponse(nlTestSuite * apSuite, void * apContext);
    static void TestProcessSubscribeRequest(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestReadWildcard(nlTestSuite * apSuite, void * apContext);
    static void TestReadChunking(nlTestSuite * apSuite, void * apContext);
    static void TestSetDirtyBetweenChunks(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeWildcard(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeEarlyShutdown(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestReadInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidIterval(nlTestSuite * apSuite, void * apContext);

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

    reportDataMessageBuilder.SuppressResponse(aSuppressResponse);
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    AttributeReportIBs::Builder attributeReportIBsBuilder = reportDataMessageBuilder.CreateAttributeReportIBs();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    AttributeReportIB::Builder attributeReportIBBuilder = attributeReportIBsBuilder.CreateAttributeReport();
    NL_TEST_ASSERT(apSuite, attributeReportIBsBuilder.GetError() == CHIP_NO_ERROR);

    AttributeDataIB::Builder attributeDataIBBuilder = attributeReportIBBuilder.CreateAttributeData();
    NL_TEST_ASSERT(apSuite, attributeReportIBBuilder.GetError() == CHIP_NO_ERROR);

    AttributePathIB::Builder attributePathBuilder = attributeDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributeDataIBBuilder.GetError() == CHIP_NO_ERROR);

    if (aNeedInvalidReport)
    {
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).ListIndex(5).EndOfAttributePathIB();
    }
    else
    {
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
    }

    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeDataIBBuilder.DataVersion(2);
    err = attributeDataIBBuilder.GetError();
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

    reportDataMessageBuilder.EndOfReportDataMessage();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadClient(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    MockInteractionModelApp delegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendReadRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    GenerateReportData(apSuite, apContext, buf, false /*aNeedInvalidReport*/, true /* aSuppressResponse*/);
    err = readClient.ProcessReportData(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    readClient.Shutdown();
}

void TestReadInteraction::TestReadHandler(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadHandler readHandler;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);

    GenerateReportData(apSuite, apContext, reportDatabuf, false /*aNeedInvalidReport*/, false /* aSuppressResponse*/);
    err = readHandler.SendReportData(std::move(reportDatabuf), false);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathIBs::Builder attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    AttributePathIB::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePath();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
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

    err = readHandler.OnReadInitialRequest(std::move(readRequestbuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    exchangeCtx->Close();
    engine->Shutdown();
}

void TestReadInteraction::TestReadClientGenerateAttributePathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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
    app::ReadClient readClient;
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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
    app::ReadClient readClient;
    MockInteractionModelApp delegate;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendReadRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    GenerateReportData(apSuite, apContext, buf, true /*aNeedInvalidReport*/, true /* aSuppressResponse*/);

    err = readClient.ProcessReportData(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

    readClient.Shutdown();
}

void TestReadInteraction::TestReadHandlerInvalidAttributePath(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadHandler readHandler;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    MockInteractionModelApp delegate;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);

    GenerateReportData(apSuite, apContext, reportDatabuf, false /*aNeedInvalidReport*/, false /* aSuppressResponse*/);
    err = readHandler.SendReportData(std::move(reportDatabuf), false);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathIBs::Builder attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    AttributePathIB::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePath();
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

    err = readHandler.OnReadInitialRequest(std::move(readRequestbuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

    exchangeCtx->Close();
    engine->Shutdown();
}

void TestReadInteraction::TestReadClientGenerateOneEventPaths(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::app::EventPathParams eventPathParams[2];
    eventPathParams[0].mNodeId     = 1;
    eventPathParams[0].mEndpointId = 2;
    eventPathParams[0].mClusterId  = 3;
    eventPathParams[0].mEventId    = 4;

    EventPaths::Builder & eventPathListBuilder = request.CreateEventRequests();
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
}

void TestReadInteraction::TestReadClientGenerateTwoEventPaths(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    MockInteractionModelApp delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequestMessage::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::app::EventPathParams eventPathParams[2];
    eventPathParams[0].mNodeId     = 1;
    eventPathParams[0].mEndpointId = 2;
    eventPathParams[0].mClusterId  = 3;
    eventPathParams[0].mEventId    = 4;

    eventPathParams[1].mNodeId     = 1;
    eventPathParams[1].mEndpointId = 2;
    eventPathParams[1].mClusterId  = 3;
    eventPathParams[1].mEventId    = 5;

    EventPaths::Builder & eventPathListBuilder = request.CreateEventRequests();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::EventPathParams eventPathParams[2];
    eventPathParams[0].mNodeId     = kTestNodeId;
    eventPathParams[0].mEndpointId = kTestEndpointId;
    eventPathParams[0].mClusterId  = kTestClusterId;
    eventPathParams[0].mEventId    = kTestEventIdDebug;

    eventPathParams[1].mNodeId     = kTestNodeId;
    eventPathParams[1].mEndpointId = kTestEndpointId;
    eventPathParams[1].mClusterId  = kTestClusterId;
    eventPathParams[1].mEventId    = kTestEventIdCritical;

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
    readPrepareParams.mEventPathParamsListSize     = 2;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
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
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 5);
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[6];
    for (int i = 0; i < 6; i++)
    {
        attributePathParams[i].mEndpointId  = Test::kMockEndpoint3;
        attributePathParams[i].mClusterId   = Test::MockClusterId(2);
        attributePathParams[i].mAttributeId = Test::MockAttributeId(4);
    }

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 6;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 6);
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[6];
    for (int i = 0; i < 6; i++)
    {
        attributePathParams[i].mEndpointId  = Test::kMockEndpoint3;
        attributePathParams[i].mClusterId   = Test::MockClusterId(2);
        attributePathParams[i].mAttributeId = Test::MockAttributeId(4);
    }

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = nullptr;
    readPrepareParams.mEventPathParamsListSize     = 0;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 6;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ClusterInfo dirtyPath;
    dirtyPath.mEndpointId  = Test::kMockEndpoint3;
    dirtyPath.mClusterId   = Test::MockClusterId(2);
    dirtyPath.mAttributeId = Test::MockAttributeId(4);

    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath);
    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

    // We should receive more than 6 attribute reports since the underlying path iterator should be reset.
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse > 6);
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    chip::app::AttributePathParams attributePathParams[2];
    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kInvalidTestClusterId;
    attributePathParams[0].mAttributeId = 1;

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
}

void TestReadInteraction::TestProcessSubscribeResponse(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    MockInteractionModelApp delegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Subscribe);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    readClient.MoveToState(chip::app::ReadClient::ClientState::AwaitingSubscribeResponse);

    GenerateSubscribeResponse(apSuite, apContext, buf.Retain());

    err = readClient.ProcessSubscribeResponse(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readClient.ShutdownInternal(CHIP_NO_ERROR);
}

void TestReadInteraction::TestProcessSubscribeRequest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadHandler readHandler;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle subscribeRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    SubscribeRequestMessage::Builder subscribeRequestBuilder;
    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx, chip::app::ReadHandler::InteractionType::Subscribe);

    writer.Init(std::move(subscribeRequestbuf));
    err = subscribeRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathIBs::Builder attributePathListBuilder = subscribeRequestBuilder.CreateAttributeRequests();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    AttributePathIB::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePath();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    attributePathBuilder.Node(1).Endpoint(2).Cluster(3).Attribute(4).ListIndex(5).EndOfAttributePathIB();
    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributePathListBuilder.EndOfAttributePathIBs();
    err = attributePathListBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    subscribeRequestBuilder.MinIntervalFloorSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.MaxIntervalCeilingSeconds(3);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.KeepSubscriptions(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.IsProxy(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.IsFabricFiltered(false).EndOfSubscribeRequestMessage();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    err = writer.Finalize(&subscribeRequestbuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readHandler.ProcessSubscribeRequest(std::move(subscribeRequestbuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    exchangeCtx->Close();
    engine->Shutdown();
}

void TestReadInteraction::TestSubscribeRoundtrip(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
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

    err = engine->SendSubscribeRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;
    err                                  = engine->SendSubscribeRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    delegate.mGotReport = false;
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);
    NL_TEST_ASSERT(apSuite, delegate.mNumSubscriptions == 1);

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
    err                                 = engine->GetReportingEngine().SetDirty(dirtyPath1);
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
    delegate.mGotReport                 = false;
    delegate.mNumAttributeResponse      = 0;
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);

    // Test multiple subscriptipn
    delegate.mNumAttributeResponse = 0;
    delegate.mGotReport            = false;
    ReadPrepareParams readPrepareParams1(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams1[1];
    readPrepareParams1.mpAttributePathParamsList                 = attributePathParams1;
    readPrepareParams1.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams1.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams1.mpAttributePathParamsList[0].mAttributeId = 1;
    readPrepareParams1.mAttributePathParamsListSize              = 1;
    readPrepareParams1.mMinIntervalFloorSeconds                  = 2;
    readPrepareParams1.mMaxIntervalCeilingSeconds                = 5;

    printf("\nSend 2nd subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = engine->SendSubscribeRequest(readPrepareParams1, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
    NL_TEST_ASSERT(apSuite, delegate.mNumSubscriptions == 2);
    // Test report with 1 path modification for 2 subscription
    delegate.mpReadHandler->mHoldReport = false;
    delegate.mGotReport                 = false;
    delegate.mNumAttributeResponse      = 0;
    err                                 = engine->GetReportingEngine().SetDirty(dirtyPath1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);

    delegate.mpReadHandler->mHoldReport = false;
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);

    delegate.mNumAttributeResponse = 0;
    delegate.mGotReport            = false;
    ReadPrepareParams readPrepareParams2(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams2[1];
    readPrepareParams2.mpAttributePathParamsList                 = attributePathParams2;
    readPrepareParams2.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams2.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams2.mpAttributePathParamsList[0].mAttributeId = 1;
    readPrepareParams2.mAttributePathParamsListSize              = 1;
    readPrepareParams2.mMinIntervalFloorSeconds                  = 2;
    readPrepareParams2.mMaxIntervalCeilingSeconds                = 5;

    printf("\nSend 3rd subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = engine->SendSubscribeRequest(readPrepareParams2, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
    NL_TEST_ASSERT(apSuite, !delegate.mReadError);

    delegate.mNumAttributeResponse = 0;
    delegate.mGotReport            = false;
    ReadPrepareParams readPrepareParams3(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams3[1];
    readPrepareParams3.mpAttributePathParamsList                 = attributePathParams3;
    readPrepareParams3.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams3.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams3.mpAttributePathParamsList[0].mAttributeId = 1;
    readPrepareParams3.mAttributePathParamsListSize              = 1;
    readPrepareParams3.mMinIntervalFloorSeconds                  = 2;
    readPrepareParams3.mMaxIntervalCeilingSeconds                = 5;

    printf("\nSend 4th subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    ReadClient readClient3;
    readClient3.Init(&ctx.GetExchangeManager(), &delegate, ReadClient::InteractionType::Subscribe);
    err = readClient3.SendSubscribeRequest(readPrepareParams3);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
    NL_TEST_ASSERT(apSuite, !delegate.mReadError);

    delegate.mNumAttributeResponse = 0;
    delegate.mGotReport            = false;
    ReadPrepareParams readPrepareParams4(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams4[1];
    readPrepareParams4.mpAttributePathParamsList                 = attributePathParams4;
    readPrepareParams4.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams4.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams4.mpAttributePathParamsList[0].mAttributeId = 1;
    readPrepareParams4.mAttributePathParamsListSize              = 1;
    readPrepareParams4.mMinIntervalFloorSeconds                  = 2;
    readPrepareParams4.mMaxIntervalCeilingSeconds                = 5;

    printf("\nSend 5th subscribe request message to Node: %" PRIu64 ",  resource exhausted\n", chip::kTestDeviceNodeId);

    ReadClient readClient4;
    readClient4.Init(&ctx.GetExchangeManager(), &delegate, ReadClient::InteractionType::Subscribe);
    err = readClient4.SendSubscribeRequest(readPrepareParams4);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, !delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    NL_TEST_ASSERT(apSuite, delegate.mReadError);

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mEventPathParamsListSize = 0;

    chip::app::AttributePathParams attributePathParams[2];
    // Subscribe to full wildcard paths, repeat twice to ensure chunking.
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = engine->SendSubscribeRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    delegate.mNumAttributeResponse       = 0;
    readPrepareParams.mKeepSubscriptions = false;
    err                                  = engine->SendSubscribeRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    delegate.mGotReport = false;

    for (int i = 0; i < 10 && delegate.mNumSubscriptions == 0; i++)
    {
        // 10 is a magic number, we assume the initial reports will take no more than 10 chunks.
        engine->GetReportingEngine().Run();
    }
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);

    // We have 29 attributes in our mock attribute storage. And we subscribed twice.
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 58);
    NL_TEST_ASSERT(apSuite, delegate.mNumSubscriptions == 1);

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

        for (int i = 0; i < 10 && delegate.mNumAttributeResponse < 26; i++)
        {
            delegate.mpReadHandler->mHoldReport = false;
            // 10 is a magic number, we assume the report will use no more than 10 chunks.
            engine->GetReportingEngine().Run();
        }

        NL_TEST_ASSERT(apSuite, delegate.mGotReport);
        // Mock endpoint3 has 13 attributes in total, and we subscribed twice.
        NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 26);
    }

    engine->Shutdown();
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
    NL_TEST_ASSERT(apSuite, engine.Init(&ctx.GetExchangeManager(), &delegate) == CHIP_NO_ERROR);

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
    NL_TEST_ASSERT(apSuite, engine.SendSubscribeRequest(readPrepareParams, &delegate) == CHIP_NO_ERROR);

    engine.GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
    NL_TEST_ASSERT(apSuite, delegate.mNumSubscriptions == 1);
    NL_TEST_ASSERT(apSuite, delegate.mpReadHandler != nullptr);

    // Shutdown the subscription
    uint64_t subscriptionId = 0;
    delegate.mpReadHandler->GetSubscriptionId(subscriptionId);

    NL_TEST_ASSERT(apSuite, subscriptionId != 0);
    NL_TEST_ASSERT(apSuite, engine.ShutdownSubscription(subscriptionId) == CHIP_NO_ERROR);

    // Cleanup
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
    engine.Shutdown();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kInvalidTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mSessionHandle             = ctx.GetSessionBobToAlice();
    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = chip::app::InteractionModelEngine::GetInstance()->SendSubscribeRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    delegate.mNumAttributeResponse = 0;
    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    delegate.mpReadHandler->mHoldReport = false;
    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    engine->Shutdown();
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
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams[1];
    readPrepareParams.mpAttributePathParamsList                 = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId  = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId   = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mAttributeId = 1;

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mSessionHandle             = ctx.GetSessionBobToAlice();
    readPrepareParams.mMinIntervalFloorSeconds   = 5;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = chip::app::InteractionModelEngine::GetInstance()->SendSubscribeRequest(readPrepareParams, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    engine->Shutdown();
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
    NL_TEST_DEF("TestProcessSubscribeResponse", chip::app::TestReadInteraction::TestProcessSubscribeResponse),
    NL_TEST_DEF("TestProcessSubscribeRequest", chip::app::TestReadInteraction::TestProcessSubscribeRequest),
    NL_TEST_DEF("TestSubscribeRoundtrip", chip::app::TestReadInteraction::TestSubscribeRoundtrip),
    NL_TEST_DEF("TestSubscribeWildcard", chip::app::TestReadInteraction::TestSubscribeWildcard),
    NL_TEST_DEF("TestSubscribeEarlyShutdown", chip::app::TestReadInteraction::TestSubscribeEarlyShutdown),
    NL_TEST_DEF("TestSubscribeInvalidAttributePathRoundtrip", chip::app::TestReadInteraction::TestSubscribeInvalidAttributePathRoundtrip),
    NL_TEST_DEF("TestReadInvalidAttributePathRoundtrip", chip::app::TestReadInteraction::TestReadInvalidAttributePathRoundtrip),
    NL_TEST_DEF("TestSubscribeInvalidIterval", chip::app::TestReadInteraction::TestSubscribeInvalidIterval),
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
