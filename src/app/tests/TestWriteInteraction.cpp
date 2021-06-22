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

namespace {
chip::System::Layer gSystemLayer;
chip::SecureSessionMgr gSessionManager;
chip::Messaging::ExchangeManager gExchangeManager;
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::secure_channel::MessageCounterManager gMessageCounterManager;
chip::Transport::AdminId gAdminId = 0;
} // namespace
namespace chip {
namespace app {
class TestWriteInteraction
{
public:
    static void TestWriteClient(nlTestSuite * apSuite, void * apContext);
    static void TestWriteHandler(nlTestSuite * apSuite, void * apContext);

private:
    static void AddAttributeDataElement(nlTestSuite * apSuite, void * apContext, WriteClient & aWriteClient);
    static void AddAttributeStatus(nlTestSuite * apSuite, void * apContext, WriteHandler & aWriteHandler);
    static void GenerateWriteRequest(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload);
    static void GenerateWriteResponse(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload);
};

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && payload) override
    {}

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

void TestWriteInteraction::AddAttributeDataElement(nlTestSuite * apSuite, void * apContext, WriteClient & aWriteClient)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathParams attributePathParams;
    attributePathParams.mNodeId     = 1;
    attributePathParams.mEndpointId = 2;
    attributePathParams.mClusterId  = 3;
    attributePathParams.mFieldId    = 4;
    attributePathParams.mListIndex  = 5;
    attributePathParams.mFlags.Set(AttributePathParams::Flags::kFieldIdValid);

    err = aWriteClient.PrepareAttribute(attributePathParams);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::TLV::TLVWriter * writer = aWriteClient.GetAttributeDataElementTLVWriter();

    err = writer->PutBoolean(chip::TLV::ContextTag(1), true);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aWriteClient.FinishAttribute();
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

    err = aWriteHandler.AddAttributeStatusCode(attributePathParams, Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                               Protocols::SecureChannel::Id, Protocols::InteractionModel::ProtocolCode::Success);
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
    AttributeStatusElement::Builder attributeStatusElementBuilder = attributeStatusListBuilder.CreateAttributeStatusBuilder();
    NL_TEST_ASSERT(apSuite, attributeStatusElementBuilder.GetError() == CHIP_NO_ERROR);

    AttributePath::Builder attributePathBuilder = attributeStatusElementBuilder.CreateAttributePathBuilder();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder.NodeId(1).EndpointId(2).ClusterId(3).FieldId(4).ListIndex(5).EndOfAttributePath();
    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    StatusElement::Builder statusElementBuilder = attributeStatusElementBuilder.CreateStatusElementBuilder();
    NL_TEST_ASSERT(apSuite, statusElementBuilder.GetError() == CHIP_NO_ERROR);
    statusElementBuilder.EncodeStatusElement(chip::Protocols::SecureChannel::GeneralStatusCode::kFailure, 2, 3)
        .EndOfStatusElement();
    err = statusElementBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeStatusElementBuilder.EndOfAttributeStatusElement();
    NL_TEST_ASSERT(apSuite, attributeStatusElementBuilder.GetError() == CHIP_NO_ERROR);

    attributeStatusListBuilder.EndOfAttributeStatusList();
    NL_TEST_ASSERT(apSuite, attributeStatusListBuilder.GetError() == CHIP_NO_ERROR);
    writeResponseBuilder.EndOfWriteResponse();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::TestWriteClient(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::WriteClient writeClient;

    chip::app::InteractionModelDelegate delegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = writeClient.Init(&gExchangeManager, &delegate);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    AddAttributeDataElement(apSuite, apContext, writeClient);

    err = writeClient.SendWriteRequest(kTestDeviceNodeId, gAdminId, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    GenerateWriteResponse(apSuite, apContext, buf);

    err = writeClient.ProcessWriteResponseMessage(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    writeClient.Shutdown();
}

void TestWriteInteraction::TestWriteHandler(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::WriteHandler writeHandler;

    chip::app::InteractionModelDelegate IMdelegate;
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    err                            = writeHandler.Init(&IMdelegate);

    GenerateWriteRequest(apSuite, apContext, buf);

    err = writeHandler.ProcessWriteRequest(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AddAttributeStatus(apSuite, apContext, writeHandler);

    writeHandler.mpExchangeCtx = gExchangeManager.NewContext({ 0, 0, 0 }, nullptr);
    TestExchangeDelegate delegate;
    writeHandler.mpExchangeCtx->SetDelegate(&delegate);
    err = writeHandler.SendWriteResponse();
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NOT_CONNECTED);

    writeHandler.Shutdown();
}
} // namespace app
} // namespace chip

namespace {

void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(gAdminId, chip::kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    gSystemLayer.Init(nullptr);

    err = gSessionManager.Init(chip::kTestDeviceNodeId, &gSystemLayer, &gTransportManager, &admins, &gMessageCounterManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gExchangeManager.Init(&gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gMessageCounterManager.Init(&gExchangeManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
        NL_TEST_DEF("CheckWriteClient", chip::app::TestWriteInteraction::TestWriteClient),
        NL_TEST_DEF("CheckWriteHandler", chip::app::TestWriteInteraction::TestWriteHandler),
        NL_TEST_SENTINEL()
};
// clang-format on
} // namespace

int TestWriteInteraction()
{
    // clang-format off
    nlTestSuite theSuite =
            {
                    "TestWriteInteraction",
                    &sTests[0],
                    nullptr,
                    nullptr
            };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestWriteInteraction)
