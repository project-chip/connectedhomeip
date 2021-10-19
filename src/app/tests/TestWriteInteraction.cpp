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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
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
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>
#include <transport/raw/UDP.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlunit-test.h>

namespace {
chip::TransportMgrBase gTransportManager;
chip::Test::LoopbackTransport gLoopback;
chip::Test::IOContext gIOContext;

uint8_t attributeDataTLV[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
size_t attributeDataTLVLen = 0;

using TestContext = chip::Test::MessagingContext;
TestContext sContext;

} // namespace
namespace chip {
namespace app {
class TestWriteInteraction
{
public:
    static void TestWriteClient(nlTestSuite * apSuite, void * apContext);
    static void TestWriteHandler(nlTestSuite * apSuite, void * apContext);
    static void TestWriteRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestWriteRoundtripWithClusterObjects(nlTestSuite * apSuite, void * apContext);

private:
    static void AddAttributeDataElement(nlTestSuite * apSuite, void * apContext, WriteClientHandle & aWriteClient);
    static void AddAttributeStatus(nlTestSuite * apSuite, void * apContext, WriteHandler & aWriteHandler);
    static void GenerateWriteRequest(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload);
    static void GenerateWriteResponse(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload);
};

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

void TestWriteInteraction::AddAttributeDataElement(nlTestSuite * apSuite, void * apContext, WriteClientHandle & aWriteClient)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathParams attributePathParams;
    attributePathParams.mNodeId     = 1;
    attributePathParams.mEndpointId = 2;
    attributePathParams.mClusterId  = 3;
    attributePathParams.mFieldId    = 4;
    attributePathParams.mListIndex  = 5;
    attributePathParams.mFlags.Set(AttributePathParams::Flags::kFieldIdValid);

    err = aWriteClient->PrepareAttribute(attributePathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = aWriteClient->GetAttributeDataElementTLVWriter();

    err = writer->PutBoolean(chip::TLV::ContextTag(chip::app::AttributeDataElement::kCsTag_Data), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aWriteClient->FinishAttribute();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::AddAttributeStatus(nlTestSuite * apSuite, void * apContext, WriteHandler & aWriteHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathParams attributePathParams;
    attributePathParams.mNodeId     = 1;
    attributePathParams.mEndpointId = 2;
    attributePathParams.mClusterId  = 3;
    attributePathParams.mFieldId    = 4;
    attributePathParams.mListIndex  = 5;
    attributePathParams.mFlags.Set(AttributePathParams::Flags::kFieldIdValid);

    err = aWriteHandler.AddStatus(attributePathParams, Protocols::InteractionModel::Status::Success);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::GenerateWriteRequest(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    WriteRequest::Builder writeRequestBuilder;
    err = writeRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    AttributeDataList::Builder attributeDataListBuilder = writeRequestBuilder.CreateAttributeDataListBuilder();
    NL_TEST_ASSERT(apSuite, attributeDataListBuilder.GetError() == CHIP_NO_ERROR);
    AttributeDataElement::Builder attributeDataElementBuilder = attributeDataListBuilder.CreateAttributeDataElementBuilder();
    NL_TEST_ASSERT(apSuite, attributeDataElementBuilder.GetError() == CHIP_NO_ERROR);

    AttributePath::Builder attributePathBuilder = attributeDataElementBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    err = attributePathBuilder.GetError();
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

    attributeDataElementBuilder.DataVersion(0);
    attributeDataElementBuilder.EndOfAttributeDataElement();
    NL_TEST_ASSERT(apSuite, attributeDataElementBuilder.GetError() == CHIP_NO_ERROR);

    attributeDataListBuilder.EndOfAttributeDataList();
    NL_TEST_ASSERT(apSuite, attributeDataListBuilder.GetError() == CHIP_NO_ERROR);
    writeRequestBuilder.EndOfWriteRequest();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::GenerateWriteResponse(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    WriteResponse::Builder writeResponseBuilder;
    err = writeResponseBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    AttributeStatusList::Builder attributeStatusListBuilder = writeResponseBuilder.CreateAttributeStatusListBuilder();
    NL_TEST_ASSERT(apSuite, attributeStatusListBuilder.GetError() == CHIP_NO_ERROR);
    AttributeStatusIB::Builder attributeStatusIBBuilder = attributeStatusListBuilder.CreateAttributeStatusBuilder();
    NL_TEST_ASSERT(apSuite, attributeStatusIBBuilder.GetError() == CHIP_NO_ERROR);

    AttributePath::Builder attributePathBuilder = attributeStatusIBBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    StatusIB::Builder statusIBBuilder = attributeStatusIBBuilder.CreateStatusIBBuilder();
    StatusIB statusIB;
    statusIB.mStatus = chip::Protocols::InteractionModel::Status::InvalidSubscription;
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    statusIBBuilder.EncodeStatusIB(statusIB);
    err = statusIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeStatusIBBuilder.EndOfAttributeStatusIB();
    NL_TEST_ASSERT(apSuite, attributeStatusIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeStatusListBuilder.EndOfAttributeStatusList();
    NL_TEST_ASSERT(apSuite, attributeStatusListBuilder.GetError() == CHIP_NO_ERROR);
    writeResponseBuilder.EndOfWriteResponse();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::TestWriteClient(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    app::WriteClient writeClient;
    app::WriteClientHandle writeClientHandle;
    writeClientHandle.SetWriteClient(&writeClient);

    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = writeClient.Init(&ctx.GetExchangeManager(), &delegate, 0);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    AddAttributeDataElement(apSuite, apContext, writeClientHandle);

    SessionHandle session = ctx.GetSessionBobToAlice();
    err = writeClientHandle.SendWriteRequest(ctx.GetAliceNodeId(), ctx.GetFabricIndex(), Optional<SessionHandle>::Value(session));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // The internal WriteClient should be nullptr once we SendWriteRequest.
    NL_TEST_ASSERT(apSuite, nullptr == writeClientHandle.mpWriteClient);

    GenerateWriteResponse(apSuite, apContext, buf);

    err = writeClient.ProcessWriteResponseMessage(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    writeClient.Shutdown();

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
}

void TestWriteInteraction::TestWriteHandler(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    app::WriteHandler writeHandler;

    chip::app::InteractionModelDelegate IMdelegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = writeHandler.Init(&IMdelegate);

    GenerateWriteRequest(apSuite, apContext, buf);

    TestExchangeDelegate delegate;
    Messaging::ExchangeContext * exchange = ctx.NewExchangeToBob(&delegate);
    err                                   = writeHandler.OnWriteRequest(exchange, std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
}

CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    TLV::TLVWriter writer;
    writer.Init(attributeDataTLV);
    writer.CopyElement(TLV::AnonymousTag, aReader);
    attributeDataTLVLen = writer.GetLengthWritten();
    return aWriteHandler->AddStatus(AttributePathParams(aClusterInfo.mNodeId, aClusterInfo.mEndpointId, aClusterInfo.mClusterId,
                                                        aClusterInfo.mFieldId, aClusterInfo.mListIndex,
                                                        AttributePathParams::Flags::kFieldIdValid),
                                    Protocols::InteractionModel::Status::Success);
}

class RoundtripDelegate : public chip::app::InteractionModelDelegate
{
public:
    CHIP_ERROR WriteResponseStatus(const WriteClient * apWriteClient, const app::StatusIB & aStatusIB,
                                   AttributePathParams & aAttributePathParams, uint8_t aAttributeIndex) override
    {
        mGotResponse = true;
        return CHIP_NO_ERROR;
    }

    bool mGotResponse = false;
};

void TestWriteInteraction::TestWriteRoundtripWithClusterObjects(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    RoundtripDelegate delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClientHandle writeClient;
    err = engine->NewWriteClient(writeClient);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AttributePathParams attributePathParams;
    attributePathParams.mNodeId     = 1;
    attributePathParams.mEndpointId = 2;
    attributePathParams.mClusterId  = 3;
    attributePathParams.mFieldId    = 4;
    attributePathParams.mFlags.Set(AttributePathParams::Flags::kFieldIdValid);

    const uint8_t byteSpanData[] = { 0xde, 0xad, 0xbe, 0xef };
    const char charSpanData[]    = "a simple test string";

    app::Clusters::TestCluster::Structs::SimpleStruct::Type dataTx;
    dataTx.a = 12;
    dataTx.b = true;
    dataTx.d = chip::ByteSpan(byteSpanData);
    // Spec A.11.2 strings SHALL NOT include a terminating null character to mark the end of a string.
    dataTx.e = chip::Span<const char>(charSpanData, strlen(charSpanData));

    writeClient.EncodeAttributeWritePayload(attributePathParams, dataTx);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, !delegate.mGotResponse);

    SessionHandle session = ctx.GetSessionBobToAlice();

    err = writeClient.SendWriteRequest(ctx.GetAliceNodeId(), ctx.GetFabricIndex(), Optional<SessionHandle>::Value(session));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, delegate.mGotResponse);

    {
        app::Clusters::TestCluster::Structs::SimpleStruct::Type dataRx;
        TLV::TLVReader reader;
        reader.Init(attributeDataTLV, attributeDataTLVLen);
        reader.Next();
        NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == DataModel::Decode(reader, dataRx));
        NL_TEST_ASSERT(apSuite, dataRx.a == dataTx.a);
        NL_TEST_ASSERT(apSuite, dataRx.b == dataTx.b);
        NL_TEST_ASSERT(apSuite, dataRx.d.data_equal(dataTx.d));
        // Equals to dataRx.e.size() == dataTx.e.size() && memncmp(dataRx.e.data(), dataTx.e.data(), dataTx.e.size()) == 0
        NL_TEST_ASSERT(apSuite, dataRx.e.data_equal(dataTx.e));
    }

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
}

void TestWriteInteraction::TestWriteRoundtrip(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    RoundtripDelegate delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClientHandle writeClient;
    err = engine->NewWriteClient(writeClient);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataElement(apSuite, apContext, writeClient);

    NL_TEST_ASSERT(apSuite, !delegate.mGotResponse);

    SessionHandle session = ctx.GetSessionBobToAlice();

    err = writeClient.SendWriteRequest(ctx.GetAliceNodeId(), ctx.GetFabricIndex(), Optional<SessionHandle>::Value(session));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, delegate.mGotResponse);

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
        NL_TEST_DEF("CheckWriteClient", chip::app::TestWriteInteraction::TestWriteClient),
        NL_TEST_DEF("CheckWriteHandler", chip::app::TestWriteInteraction::TestWriteHandler),
        NL_TEST_DEF("CheckWriteRoundtrip", chip::app::TestWriteInteraction::TestWriteRoundtrip),
        NL_TEST_DEF("TestWriteRoundtripWithClusterObjects", chip::app::TestWriteInteraction::TestWriteRoundtripWithClusterObjects),
        NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "TestWriteInteraction",
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

    gTransportManager.SetSessionManager(&ctx->GetSecureSessionManager());
    return SUCCESS;
}

int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

int TestWriteInteraction()
{
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestWriteInteraction)
