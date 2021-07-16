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
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

namespace chip {
System::Layer gSystemLayer;
SecureSessionMgr gSessionManager;
Messaging::ExchangeManager gExchangeManager;
TransportMgr<Transport::UDP> gTransportManager;
const Transport::AdminId gAdminId = 0;
secure_channel::MessageCounterManager gMessageCounterManager;

namespace app {
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::ReadClient readClient;
    EventNumber eventNumber = 0;

    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = readClient.Init(&gExchangeManager, &delegate, 0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.SendReadRequest(kTestDeviceNodeId, gAdminId, nullptr /* apSecureSession */, nullptr /*apEventPathParamsList*/,
                                     0 /*aEventPathParamsListSize*/, nullptr /*apAttributePathParamsList*/,
                                     0 /*aAttributePathParamsListSize*/, eventNumber /*aEventNumber*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateReportData(apSuite, apContext, buf);

    err = readClient.ProcessReportData(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    readClient.Shutdown();
}

void TestReadInteraction::TestReadHandler(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::ReadHandler readHandler;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequest::Builder readRequestBuilder;
    chip::app::InteractionModelDelegate delegate;
    err = InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readHandler.Init(nullptr);

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

    err = readHandler.OnReadRequest(nullptr, std::move(readRequestbuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadClientGenerateAttributePathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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

    err = readClient.Init(&gExchangeManager, &delegate, 0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mFlags.Set(AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(AttributePathParams::Flags::kListIndexValid);
    err = readClient.GenerateAttributePathList(request, attributePathParams, 2 /*aAttributePathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReadInteraction::TestReadClientGenerateInvalidAttributePathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequest::Builder request;
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = readClient.Init(&gExchangeManager, &delegate, 0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathParams attributePathParams[2];
    attributePathParams[0].mFlags.Set(AttributePathParams::Flags::kFieldIdValid);
    attributePathParams[1].mFlags.Set(AttributePathParams::Flags::kListIndexValid);
    err = readClient.GenerateAttributePathList(request, attributePathParams, 2 /*aAttributePathParamsListSize*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
}

void TestReadInteraction::TestReadClientInvalidReport(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    EventNumber eventNumber = 0;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = readClient.Init(&gExchangeManager, &delegate, 0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.SendReadRequest(kTestDeviceNodeId, gAdminId, nullptr /* apSecureSession */, nullptr /*apEventPathParamsList*/,
                                     0 /*aEventPathParamsListSize*/, nullptr /*apAttributePathParamsList*/,
                                     0 /*aAttributePathParamsListSize*/, eventNumber /*aEventNumber*/);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateReportData(apSuite, apContext, buf, true /*aNeedInvalidReport*/);

    err = readClient.ProcessReportData(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

    readClient.Shutdown();
}

void TestReadInteraction::TestReadHandlerInvalidAttributePath(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::ReadHandler readHandler;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle reportDatabuf  = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequest::Builder readRequestBuilder;
    chip::app::InteractionModelDelegate delegate;

    err = InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    readHandler.Init(nullptr);

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

    err = readHandler.OnReadRequest(nullptr, std::move(readRequestbuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
}

void TestReadInteraction::TestReadClientGenerateOneEventPathList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequest::Builder request;
    chip::EventNumber eventNumber = 0;
    msgBuf                        = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&gExchangeManager, &delegate, 0 /* application identifier */);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::app::EventPathParams eventPathParams[2];
    eventPathParams[0].mNodeId     = 1;
    eventPathParams[0].mEndpointId = 2;
    eventPathParams[0].mClusterId  = 3;
    eventPathParams[0].mEventId    = 4;

    err = readClient.GenerateEventPathList(request, eventPathParams, 1 /*aEventPathParamsListSize*/, eventNumber);
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::ReadClient readClient;
    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    ReadRequest::Builder request;
    chip::EventNumber eventNumber = 0;
    msgBuf                        = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    writer.Init(std::move(msgBuf));
    err = request.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readClient.Init(&gExchangeManager, &delegate, 0 /* application identifier */);
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
    err = readClient.GenerateEventPathList(request, eventPathParams, 2 /*aEventPathParamsListSize*/, eventNumber);
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

} // namespace app
} // namespace chip

namespace {

void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(chip::gAdminId, chip::kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::gSystemLayer.Init();

    err = chip::gSessionManager.Init(chip::kTestDeviceNodeId, &chip::gSystemLayer, &chip::gTransportManager, &admins,
                                     &chip::gMessageCounterManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gExchangeManager.Init(&chip::gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gMessageCounterManager.Init(&chip::gExchangeManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("CheckReadClient", chip::app::TestReadInteraction::TestReadClient),
    NL_TEST_DEF("CheckReadHandler", chip::app::TestReadInteraction::TestReadHandler),
    NL_TEST_DEF("TestReadClientGenerateAttributePathList", chip::app::TestReadInteraction::TestReadClientGenerateAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateInvalidAttributePathList", chip::app::TestReadInteraction::TestReadClientGenerateInvalidAttributePathList),
    NL_TEST_DEF("TestReadClientGenerateOneEventPathList", chip::app::TestReadInteraction::TestReadClientGenerateOneEventPathList),
    NL_TEST_DEF("TestReadClientGenerateTwoEventPathList", chip::app::TestReadInteraction::TestReadClientGenerateTwoEventPathList),
    NL_TEST_DEF("TestReadClientInvalidReport", chip::app::TestReadInteraction::TestReadClientInvalidReport),
    NL_TEST_DEF("TestReadHandlerInvalidAttributePath", chip::app::TestReadInteraction::TestReadHandlerInvalidAttributePath),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestReadInteraction()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestReadInteraction",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReadInteraction)
