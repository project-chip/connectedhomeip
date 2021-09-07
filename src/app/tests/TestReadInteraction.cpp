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
#include <transport/SecureSessionMgr.h>
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
                      chip::TLV::TLVReader * apData, chip::Protocols::InteractionModel::ProtocolCode status) override
    {
        if (status == chip::Protocols::InteractionModel::ProtocolCode::Success)
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

    bool mGotEventResponse    = false;
    int mNumAttributeResponse = 0;
    bool mGotReport           = false;
    bool mReadError           = false;
};
} // namespace

namespace chip {
namespace app {
CHIP_ERROR ReadSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    uint64_t version = 0;
    ChipLogDetail(DataManagement, "TEST Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", aClusterInfo.mClusterId,
                  aClusterInfo.mFieldId);
    if (apWriter == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!(aClusterInfo.mClusterId == kTestClusterId && aClusterInfo.mEndpointId == kTestEndpointId))
    {
        if (apDataExists != nullptr)
        {
            *apDataExists = false;
        }

        return apWriter->Put(chip::TLV::ContextTag(AttributeDataElement::kCsTag_Status),
                             chip::Protocols::InteractionModel::ProtocolCode::UnsupportedAttribute);
    }

    if (apDataExists != nullptr)
    {
        *apDataExists = true;
    }

    CHIP_ERROR err = apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), kTestFieldValue1);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
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
    static void TestReadRoundtrip(nlTestSuite * apSuite, void * apContext);
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
    err                            = readClient.Init(&ctx.GetExchangeManager(), &delegate, 0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ReadPrepareParams readPrepareParams(ctx.GetSessionLocalToPeer());
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
    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToPeer(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx);

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

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, 0 /* application identifier */);
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
    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, 0 /* application identifier */);
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
    err                            = readClient.Init(&ctx.GetExchangeManager(), &delegate, 0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ReadPrepareParams readPrepareParams(ctx.GetSessionLocalToPeer());
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
    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToPeer(nullptr);
    readHandler.Init(&ctx.GetExchangeManager(), nullptr, exchangeCtx);

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

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, 0 /* application identifier */);
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

    err = readClient.Init(&ctx.GetExchangeManager(), &delegate, 0 /* application identifier */);
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
    attributePathParams[1].mFieldId    = 1;
    attributePathParams[1].mListIndex  = 1;
    attributePathParams[1].mFlags.Set(chip::app::AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(chip::app::AttributePathParams::Flags::kListIndexValid);

    ReadPrepareParams readPrepareParams(ctx.GetSessionLocalToPeer());
    readPrepareParams.mpEventPathParamsList        = eventPathParams;
    readPrepareParams.mEventPathParamsListSize     = 2;
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 2;
    err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
    NL_TEST_ASSERT(apSuite, delegate.mNumAttributeResponse == 1);
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

    ReadPrepareParams readPrepareParams(ctx.GetSessionLocalToPeer());
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
    gTransportManager.SetSecureSessionMgr(&ctx->GetSecureSessionManager());
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
