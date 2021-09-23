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

#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventDataElement.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>
#include <transport/raw/UDP.h>
#include <transport/raw/tests/NetworkTestHelpers.h>
#include <type_traits>

namespace {
chip::TransportMgrBase gTransportManager;
chip::Test::LoopbackTransport gLoopback;
chip::Test::IOContext gIOContext;
chip::secure_channel::MessageCounterManager gMessageCounterManager;
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
uint64_t kTestEventTag                = 1;
using TestContext                     = chip::Test::MessagingContext;
TestContext sContext;

void InitializeEventLogging(chip::Messaging::ExchangeManager & aExchangeManager)
{
    chip::app::LogStorageResources logStorageResources[] = {
        { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Debug },
        { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Info },
        { &gCritEventBuffer[0], sizeof(gCritEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Critical },
    };

    chip::app::EventManagement::CreateEventManagement(&aExchangeManager, ArraySize(logStorageResources), gCircularEventBuffer,
                                                      logStorageResources);
}

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

    chip::app::SubscribeResponse::Builder subscribeResponseBuilder;

    err = subscribeResponseBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    subscribeResponseBuilder.SubscriptionId(0);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MinIntervalFloorSeconds(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MaxIntervalCeilingSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.EndOfSubscribeResponse();
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

class MockInteractionModelApp : public chip::app::InteractionModelDelegate
{
public:
    CHIP_ERROR EventStreamReceived(const chip::Messaging::ExchangeContext * apExchangeContext,
                                   chip::TLV::TLVReader * apEventListReader) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::TLV::TLVReader reader;
        int numDataElementIndex = 0;
        reader.Init(*apEventListReader);
        while (CHIP_NO_ERROR == (err = reader.Next()))
        {
            uint8_t priorityLevel = 0;
            chip::app::EventDataElement::Parser event;
            ReturnErrorOnFailure(event.Init(reader));
            ReturnErrorOnFailure(event.GetPriorityLevel(&priorityLevel));
            if (numDataElementIndex == 0)
            {
                VerifyOrReturnError(priorityLevel == static_cast<uint8_t>(chip::app::PriorityLevel::Critical),
                                    CHIP_ERROR_INCORRECT_STATE);
            }
            else if (numDataElementIndex == 1)
            {
                VerifyOrReturnError(priorityLevel == static_cast<uint8_t>(chip::app::PriorityLevel::Info),
                                    CHIP_ERROR_INCORRECT_STATE);
            }
            ++numDataElementIndex;
        }
        if (CHIP_END_OF_TLV == err)
        {
            mGotEventResponse = true;
            err               = CHIP_NO_ERROR;
        }
        return err;
    }

    void OnReportData(const chip::app::ReadClient * apReadClient, const chip::app::ClusterInfo & aPath,
                      chip::TLV::TLVReader * apData, chip::Protocols::InteractionModel::Status status) override
    {
        if (status == chip::Protocols::InteractionModel::Status::Success)
        {
            mNumAttributeResponse++;
        }
    }

    CHIP_ERROR ReportProcessed(const chip::app::ReadClient * apReadClient) override
    {
        mGotReport = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadError(const chip::app::ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        mReadError = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadDone(const chip::app::ReadClient * apReadClient) override { return CHIP_NO_ERROR; }

    CHIP_ERROR SubscriptionEstablished(const chip::app::ReadHandler * apReadHandler) override
    {
        mpReadHandler = const_cast<chip::app::ReadHandler *>(apReadHandler);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    bool mGotEventResponse                 = false;
    int mNumAttributeResponse              = 0;
    bool mGotReport                        = false;
    bool mReadError                        = false;
    chip::app::ReadHandler * mpReadHandler = nullptr;
};
} // namespace

namespace chip {
namespace app {
CHIP_ERROR ReadSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    uint64_t version = 0;
    ChipLogDetail(DataManagement, "TEST Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", aClusterInfo.mClusterId,
                  aClusterInfo.mFieldId);

    if (apDataExists != nullptr)
    {
        *apDataExists = (aClusterInfo.mClusterId == kTestClusterId && aClusterInfo.mEndpointId == kTestEndpointId);
    }

    if (apWriter == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    if (!(aClusterInfo.mClusterId == kTestClusterId && aClusterInfo.mEndpointId == kTestEndpointId))
    {
        return apWriter->Put(chip::TLV::ContextTag(AttributeDataElement::kCsTag_Status),
                             chip::Protocols::InteractionModel::Status::UnsupportedAttribute);
    }

    ReturnErrorOnFailure(apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), kTestFieldValue1));
    return apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_DataVersion), version);
}

class TestReadInteraction
{
public:
    static void TestReadClient(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateAttributePathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateInvalidAttributePathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateOneEventPathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientGenerateTwoEventPathList(nlTestSuite * apSuite, void * apContext);
    static void TestReadClientInvalidReport(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerInvalidAttributePath(nlTestSuite * apSuite, void * apContext);
    static void TestProcessSubscribeResponse(nlTestSuite * apSuite, void * apContext);
    static void TestProcessSubscribeRequest(nlTestSuite * apSuite, void * apContext);
    static void TestReadRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestSubscribeInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestReadInvalidAttributePathRoundtrip(nlTestSuite * apSuite, void * apContext);

private:
    static void GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                   bool aNeedInvalidReport = false);
};

void TestReadInteraction::GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload,
                                             bool aNeedInvalidReport)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    ReportData::Builder reportDataBuilder;

    err = reportDataBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributeDataList::Builder attributeDataListBuilder = reportDataBuilder.CreateAttributeDataListBuilder();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    AttributeDataElement::Builder attributeDataElementBuilder = attributeDataListBuilder.CreateAttributeDataElementBuilder();
    NL_TEST_ASSERT(apSuite, attributeDataListBuilder.GetError() == CHIP_NO_ERROR);

    AttributePath::Builder attributePathBuilder = attributeDataElementBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributeDataElementBuilder.GetError() == CHIP_NO_ERROR);

    if (aNeedInvalidReport)
    {
        attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).ListIndex(5).EndOfAttributePath();
    }
    else
    {
        attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    }

    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeDataElementBuilder.DataVersion(2);
    err = attributeDataElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = attributeDataElementBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(AttributeDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure,
                                      dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    attributeDataElementBuilder.MoreClusterData(false);
    err = attributeDataElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeDataElementBuilder.EndOfAttributeDataElement();
    NL_TEST_ASSERT(apSuite, attributeDataElementBuilder.GetError() == CHIP_NO_ERROR);

    attributeDataListBuilder.EndOfAttributeDataList();
    NL_TEST_ASSERT(apSuite, attributeDataListBuilder.GetError() == CHIP_NO_ERROR);

    reportDataBuilder.SuppressResponse(true);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    reportDataBuilder.MoreChunkedMessages(false);
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    reportDataBuilder.EndOfReportData();
    NL_TEST_ASSERT(apSuite, reportDataBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadClient(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read,
                          0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendReadRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    GenerateReportData(apSuite, apContext, buf);

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
    ReadRequest::Builder readRequestBuilder;
    chip::app::InteractionModelDelegate delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);

    GenerateReportData(apSuite, apContext, reportDatabuf);
    err = readHandler.SendReportData(std::move(reportDatabuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathList::Builder attributePathListBuilder = readRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    AttributePath::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributePathListBuilder.EndOfAttributePathList();
    err = attributePathListBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    readRequestBuilder.EndOfReadRequest();
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
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequest::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read,
                          0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mFlags.Set(AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(AttributePathParams::Flags::kListIndexValid);
    AttributePathList::Builder & attributePathListBuilder = request.CreateAttributePathListBuilder();
    err = readClient.GenerateAttributePathList(attributePathListBuilder, attributePathParams, 2 /*aAttributePathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadClientGenerateInvalidAttributePathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequest::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read,
                          0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mFlags.Set(AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(AttributePathParams::Flags::kListIndexValid);
    AttributePathList::Builder & attributePathListBuilder = request.CreateAttributePathListBuilder();
    err = readClient.GenerateAttributePathList(attributePathListBuilder, attributePathParams, 2 /*aAttributePathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
}

void TestReadInteraction::TestReadClientInvalidReport(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read,
                          0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    err = readClient.SendReadRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    GenerateReportData(apSuite, apContext, buf, true /*aNeedInvalidReport*/);

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
    ReadRequest::Builder readRequestBuilder;
    chip::app::InteractionModelDelegate delegate;

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);

    GenerateReportData(apSuite, apContext, reportDatabuf);
    err = readHandler.SendReportData(std::move(reportDatabuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INCORRECT_STATE);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathList::Builder attributePathListBuilder = readRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    AttributePath::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).ListIndex(5).EndOfAttributePath();
    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    readRequestBuilder.EndOfReadRequest();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    err = writer.Finalize(&readRequestbuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readHandler.OnReadInitialRequest(std::move(readRequestbuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

    exchangeCtx->Close();
    engine->Shutdown();
}

void TestReadInteraction::TestReadClientGenerateOneEventPathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequest::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read,
                          0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::app::EventPathParams eventPathParams[2];
    eventPathParams[0].mNodeId     = 1;
    eventPathParams[0].mEndpointId = 2;
    eventPathParams[0].mClusterId  = 3;
    eventPathParams[0].mEventId    = 4;

    EventPathList::Builder & eventPathListBuilder = request.CreateEventPathListBuilder();
    err = readClient.GenerateEventPathList(eventPathListBuilder, eventPathParams, 1 /*aEventPathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    request.EndOfReadRequest();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == request.GetError());

    err = writer.Finalize(&msgBuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequest::Parser readRequestParser;

    reader.Init(msgBuf.Retain());
    reader.Next();

    err = readRequestParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void TestReadInteraction::TestReadClientGenerateTwoEventPathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequest::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Read,
                          0 /* application identifier */);
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

    EventPathList::Builder & eventPathListBuilder = request.CreateEventPathListBuilder();
    err = readClient.GenerateEventPathList(eventPathListBuilder, eventPathParams, 2 /*aEventPathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    request.EndOfReadRequest();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == request.GetError());

    err = writer.Finalize(&msgBuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferTLVReader reader;
    ReadRequest::Parser readRequestParser;

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
    attributePathParams[0].mNodeId     = chip::kTestDeviceNodeId;
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kTestClusterId;
    attributePathParams[0].mFieldId    = 1;
    attributePathParams[0].mListIndex  = 0;
    attributePathParams[0].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);

    attributePathParams[1].mNodeId     = chip::kTestDeviceNodeId;
    attributePathParams[1].mEndpointId = kTestEndpointId;
    attributePathParams[1].mClusterId  = kTestClusterId;
    attributePathParams[1].mFieldId    = 2;
    attributePathParams[1].mListIndex  = 1;
    attributePathParams[1].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(chip::app::AttributePathParams::Flags::kListIndexValid);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpEventPathParamsList        = eventPathParams;
    readPrepareParams.mEventPathParamsListSize     = 2;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams);
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
    attributePathParams[0].mNodeId     = chip::kTestDeviceNodeId;
    attributePathParams[0].mEndpointId = kTestEndpointId;
    attributePathParams[0].mClusterId  = kInvalidTestClusterId;
    attributePathParams[0].mFieldId    = 1;
    attributePathParams[0].mListIndex  = 0;
    attributePathParams[0].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);

    ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams);
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
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, chip::app::ReadClient::InteractionType::Subscribe,
                          0 /* application identifier */);
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
    SubscribeRequest::Builder subscribeRequestBuilder;
    chip::app::InteractionModelDelegate delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx, chip::app::ReadHandler::InteractionType::Subscribe);

    writer.Init(std::move(subscribeRequestbuf));
    err = subscribeRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathList::Builder attributePathListBuilder = subscribeRequestBuilder.CreateAttributePathListBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    AttributePath::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);

    attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributePathListBuilder.EndOfAttributePathList();
    err = attributePathListBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    subscribeRequestBuilder.MinIntervalSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.MaxIntervalSeconds(3);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.KeepExistingSubscriptions(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.IsProxy(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.EndOfSubscribeRequest();
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
    readPrepareParams.mpEventPathParamsList[0].mNodeId     = kTestNodeId;
    readPrepareParams.mpEventPathParamsList[0].mEndpointId = kTestEndpointId;
    readPrepareParams.mpEventPathParamsList[0].mClusterId  = kTestClusterId;
    readPrepareParams.mpEventPathParamsList[0].mEventId    = kTestEventIdDebug;

    readPrepareParams.mpEventPathParamsList[1].mNodeId     = kTestNodeId;
    readPrepareParams.mpEventPathParamsList[1].mEndpointId = kTestEndpointId;
    readPrepareParams.mpEventPathParamsList[1].mClusterId  = kTestClusterId;
    readPrepareParams.mpEventPathParamsList[1].mEventId    = kTestEventIdCritical;

    readPrepareParams.mEventPathParamsListSize = 2;

    chip::app::AttributePathParams attributePathParams[2];
    readPrepareParams.mpAttributePathParamsList                = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mNodeId     = chip::kTestDeviceNodeId;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId  = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mFieldId    = 1;
    readPrepareParams.mpAttributePathParamsList[0].mListIndex  = 0;
    readPrepareParams.mpAttributePathParamsList[0].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);

    readPrepareParams.mpAttributePathParamsList[1].mNodeId     = chip::kTestDeviceNodeId;
    readPrepareParams.mpAttributePathParamsList[1].mEndpointId = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[1].mClusterId  = kTestClusterId;
    readPrepareParams.mpAttributePathParamsList[1].mFieldId    = 2;
    readPrepareParams.mpAttributePathParamsList[1].mListIndex  = 0;
    readPrepareParams.mpAttributePathParamsList[1].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);

    readPrepareParams.mAttributePathParamsListSize = 2;

    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = engine->SendSubscribeRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    delegate.mGotReport = false;
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 2);

    chip::app::ClusterInfo dirtyPath1;
    dirtyPath1.mClusterId  = kTestClusterId;
    dirtyPath1.mEndpointId = kTestEndpointId;
    dirtyPath1.mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    dirtyPath1.mFieldId = 1;

    chip::app::ClusterInfo dirtyPath2;
    dirtyPath2.mClusterId  = kTestClusterId;
    dirtyPath2.mEndpointId = kTestEndpointId;
    dirtyPath2.mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    dirtyPath2.mFieldId = 2;

    chip::app::ClusterInfo dirtyPath3;
    dirtyPath2.mClusterId  = kTestClusterId;
    dirtyPath2.mEndpointId = kTestEndpointId;
    dirtyPath2.mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    dirtyPath2.mFlags.Set(chip::app::ClusterInfo::Flags::kListIndexValid);
    dirtyPath2.mFieldId   = 2;
    dirtyPath2.mListIndex = 1;

    chip::app::ClusterInfo dirtyPath4;
    dirtyPath4.mClusterId  = kTestClusterId;
    dirtyPath4.mEndpointId = kTestEndpointId;
    dirtyPath4.mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    dirtyPath4.mFieldId = 3;

    chip::app::ClusterInfo dirtyPath5;
    dirtyPath5.mClusterId  = kTestClusterId;
    dirtyPath5.mEndpointId = kTestEndpointId;
    dirtyPath5.mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    dirtyPath5.mFieldId = 4;

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
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);

    // Test multiple subscriptipn
    delegate.mNumAttributeResponse = 0;
    delegate.mGotReport            = false;
    ReadPrepareParams readPrepareParams1(ctx.GetSessionBobToAlice());
    chip::app::AttributePathParams attributePathParams1[1];
    readPrepareParams1.mpAttributePathParamsList                = attributePathParams1;
    readPrepareParams1.mpAttributePathParamsList[0].mNodeId     = chip::kTestDeviceNodeId;
    readPrepareParams1.mpAttributePathParamsList[0].mEndpointId = kTestEndpointId;
    readPrepareParams1.mpAttributePathParamsList[0].mClusterId  = kTestClusterId;
    readPrepareParams1.mpAttributePathParamsList[0].mFieldId    = 1;
    readPrepareParams1.mpAttributePathParamsList[0].mListIndex  = 0;
    readPrepareParams1.mpAttributePathParamsList[0].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);
    readPrepareParams1.mAttributePathParamsListSize = 1;
    readPrepareParams1.mMinIntervalFloorSeconds     = 2;
    readPrepareParams1.mMaxIntervalCeilingSeconds   = 5;
    printf("\nSend another subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = engine->SendSubscribeRequest(readPrepareParams1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    engine->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotReport);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);

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
    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
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
    readPrepareParams.mpAttributePathParamsList                = attributePathParams;
    readPrepareParams.mpAttributePathParamsList[0].mNodeId     = chip::kTestDeviceNodeId;
    readPrepareParams.mpAttributePathParamsList[0].mEndpointId = kTestEndpointId;
    readPrepareParams.mpAttributePathParamsList[0].mClusterId  = kInvalidTestClusterId;
    readPrepareParams.mpAttributePathParamsList[0].mFieldId    = 1;
    readPrepareParams.mpAttributePathParamsList[0].mListIndex  = 0;
    readPrepareParams.mpAttributePathParamsList[0].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);

    readPrepareParams.mAttributePathParamsListSize = 1;

    readPrepareParams.mSessionHandle             = ctx.GetSessionBobToAlice();
    readPrepareParams.mMinIntervalFloorSeconds   = 2;
    readPrepareParams.mMaxIntervalCeilingSeconds = 5;
    printf("\nSend subscribe request message to Node: %" PRIu64 "\n", chip::kTestDeviceNodeId);

    err = chip::app::InteractionModelEngine::GetInstance()->SendSubscribeRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    delegate.mNumAttributeResponse = 0;
    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
    delegate.mpReadHandler->mHoldReport = false;
    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 0);
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
    NL_TEST_DEF("CheckReadClient", chip::app::TestReadInteraction::TestReadClient),
    NL_TEST_DEF("CheckReadHandler", chip::app::TestReadInteraction::TestReadHandler),
    NL_TEST_DEF("TestReadClientGenerateAttributePathList", chip::app::TestReadInteraction::TestReadClientGenerateAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateInvalidAttributePathList", chip::app::TestReadInteraction::TestReadClientGenerateInvalidAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateOneEventPathList", chip::app::TestReadInteraction::TestReadClientGenerateOneEventPathList),
    NL_TEST_DEF("TestReadClientGenerateTwoEventPathList", chip::app::TestReadInteraction::TestReadClientGenerateTwoEventPathList),
    NL_TEST_DEF("TestReadClientInvalidReport", chip::app::TestReadInteraction::TestReadClientInvalidReport),
    NL_TEST_DEF("TestReadHandlerInvalidAttributePath", chip::app::TestReadInteraction::TestReadHandlerInvalidAttributePath),
    NL_TEST_DEF("TestProcessSubscribeResponse", chip::app::TestReadInteraction::TestProcessSubscribeResponse),
    NL_TEST_DEF("TestProcessSubscribeRequest", chip::app::TestReadInteraction::TestProcessSubscribeRequest),
    NL_TEST_DEF("TestSubscribeRoundtrip", chip::app::TestReadInteraction::TestSubscribeRoundtrip),
    NL_TEST_DEF("TestSubscribeInvalidAttributePathRoundtrip", chip::app::TestReadInteraction::TestSubscribeInvalidAttributePathRoundtrip),
    NL_TEST_DEF("TestReadInvalidAttributePathRoundtrip", chip::app::TestReadInteraction::TestReadInvalidAttributePathRoundtrip),
    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
        "TestReadInteraction",
        &sTests[0],
        Initialize,
        Finalize
};
// clang-format on

int Initialize(void * aContext)
{
    // Initialize System memory and resources
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gIOContext.Init(&sSuite) == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gTransportManager.Init(&gLoopback) == CHIP_NO_ERROR, FAILURE);

    auto * ctx = static_cast<TestContext *>(aContext);
    VerifyOrReturnError(ctx->Init(&sSuite, &gTransportManager, &gIOContext) == CHIP_NO_ERROR, FAILURE);

    InitializeEventLogging(ctx->GetExchangeManager());
    gTransportManager.SetSessionManager(&ctx->GetSecureSessionManager());
    return SUCCESS;
}

int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
    chip::app::EventManagement::DestroyEventManagement();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

int TestReadInteraction()
{
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReadInteraction)
