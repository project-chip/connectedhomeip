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
#include <utility>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <pw_unit_test/framework.h>

namespace {

constexpr chip::DataVersion kAcceptedDataVersion = 5;
constexpr uint16_t kMaxGroupsPerFabric           = 5;
constexpr uint16_t kMaxGroupKeysPerFabric        = 8;

chip::TestPersistentStorageDelegate gTestStorage;
chip::Crypto::DefaultSessionKeystore gSessionKeystore;
chip::Credentials::GroupDataProviderImpl gGroupsProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);

} // namespace

namespace chip {
namespace app {
class TestWriteInteraction : public chip::Test::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();

        gTestStorage.ClearStorage();
        gGroupsProvider.SetStorageDelegate(&gTestStorage);
        gGroupsProvider.SetSessionKeystore(&gSessionKeystore);
        ASSERT_EQ(gGroupsProvider.Init(), CHIP_NO_ERROR);
        chip::Credentials::SetGroupDataProvider(&gGroupsProvider);

        uint8_t buf[sizeof(chip::CompressedFabricId)];
        chip::MutableByteSpan span(buf);
        ASSERT_EQ(GetBobFabric()->GetCompressedFabricIdBytes(span), CHIP_NO_ERROR);
        ASSERT_EQ(chip::GroupTesting::InitData(&gGroupsProvider, GetBobFabricIndex(), span), CHIP_NO_ERROR);

        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(&TestImCustomDataModel::Instance());
    }
    void TearDown() override
    {
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        chip::Credentials::GroupDataProvider * provider = chip::Credentials::GetGroupDataProvider();
        if (provider != nullptr)
        {
            provider->Finish();
        }
        AppContext::TearDown();
    }

    void TestWriteClient();
    void TestWriteClientGroup();
    void TestWriteHandlerReceiveInvalidMessage();
    void TestWriteHandlerReceiveEmptyWriteRequest();
    void TestWriteInvalidMessage1();
    void TestWriteInvalidMessage2();
    void TestWriteInvalidMessage3();
    void TestWriteInvalidMessage4();

    static void AddAttributeDataIB(WriteClient & aWriteClient);
    static void AddAttributeStatus(WriteHandler & aWriteHandler);
    static void GenerateWriteRequest(bool aIsTimedWrite, System::PacketBufferHandle & aPayload);
    static void GenerateWriteResponse(System::PacketBufferHandle & aPayload);

private:
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
public:
    Protocols::InteractionModel::Status mLastStatus = Protocols::InteractionModel::Status::Success;
    bool mLastStatusParsedSuccessfully              = false;

private:
    CHIP_ERROR UpdateLastStatus(System::PacketBufferHandle && payload)
    {
        StatusResponseMessage::Parser response;
        System::PacketBufferTLVReader reader;
        reader.Init(std::move(payload));
        ReturnErrorOnFailure(response.Init(reader));
        StatusIB status;
        ReturnErrorOnFailure(response.GetStatus(mLastStatus));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        if (payloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
        {
            CHIP_ERROR err                = UpdateLastStatus(std::move(payload));
            mLastStatusParsedSuccessfully = (err == CHIP_NO_ERROR);
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

class TestWriteClientCallback : public chip::app::WriteClient::Callback
{
public:
    void ResetCounter() { mOnSuccessCalled = mOnErrorCalled = mOnDoneCalled = 0; }
    void OnResponse(const WriteClient * apWriteClient, const chip::app::ConcreteDataAttributePath & path, StatusIB status) override
    {
        mStatus = status;
        mOnSuccessCalled++;
    }
    void OnError(const WriteClient * apWriteClient, CHIP_ERROR chipError) override
    {
        mOnErrorCalled++;
        mLastErrorReason = app::StatusIB(chipError);
        mError           = chipError;
    }
    void OnDone(WriteClient * apWriteClient) override { mOnDoneCalled++; }

    int mOnSuccessCalled = 0;
    int mOnErrorCalled   = 0;
    int mOnDoneCalled    = 0;
    StatusIB mStatus;
    StatusIB mLastErrorReason;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};

void TestWriteInteraction::AddAttributeDataIB(WriteClient & aWriteClient)
{
    AttributePathParams attributePathParams;
    bool attributeValue              = true;
    attributePathParams.mEndpointId  = 2;
    attributePathParams.mClusterId   = 3;
    attributePathParams.mAttributeId = 4;

    EXPECT_EQ(aWriteClient.EncodeAttribute(attributePathParams, attributeValue), CHIP_NO_ERROR);
}

void TestWriteInteraction::AddAttributeStatus(WriteHandler & aWriteHandler)
{
    ConcreteAttributePath attributePath(2, 3, 4);

    EXPECT_EQ(aWriteHandler.AddStatus(attributePath, Protocols::InteractionModel::Status::Success), CHIP_NO_ERROR);
}

void TestWriteInteraction::GenerateWriteRequest(bool aIsTimedWrite, System::PacketBufferHandle & aPayload)
{
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    WriteRequestMessage::Builder writeRequestBuilder;
    EXPECT_EQ(writeRequestBuilder.Init(&writer), CHIP_NO_ERROR);
    writeRequestBuilder.TimedRequest(aIsTimedWrite);
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);
    AttributeDataIBs::Builder & attributeDataIBsBuilder = writeRequestBuilder.CreateWriteRequests();
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);
    AttributeDataIB::Builder & attributeDataIBBuilder = attributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    EXPECT_EQ(attributeDataIBsBuilder.GetError(), CHIP_NO_ERROR);

    attributeDataIBBuilder.DataVersion(0);
    EXPECT_EQ(attributeDataIBBuilder.GetError(), CHIP_NO_ERROR);
    AttributePathIB::Builder & attributePathBuilder = attributeDataIBBuilder.CreatePath();
    EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);
    EXPECT_EQ(attributePathBuilder.Node(1)
                  .Endpoint(2)
                  .Cluster(3)
                  .Attribute(4)
                  .ListIndex(DataModel::Nullable<ListIndex>())
                  .EndOfAttributePathIB(),
              CHIP_NO_ERROR);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = attributeDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        EXPECT_EQ(
            pWriter->StartContainer(chip::TLV::ContextTag(AttributeDataIB::Tag::kData), chip::TLV::kTLVType_Structure, dummyType),
            CHIP_NO_ERROR);

        EXPECT_EQ(pWriter->PutBoolean(chip::TLV::ContextTag(1), true), CHIP_NO_ERROR);

        EXPECT_EQ(pWriter->EndContainer(dummyType), CHIP_NO_ERROR);
    }

    attributeDataIBBuilder.EndOfAttributeDataIB();
    EXPECT_EQ(attributeDataIBBuilder.GetError(), CHIP_NO_ERROR);

    attributeDataIBsBuilder.EndOfAttributeDataIBs();
    EXPECT_EQ(attributeDataIBsBuilder.GetError(), CHIP_NO_ERROR);
    writeRequestBuilder.EndOfWriteRequestMessage();
    EXPECT_EQ(writeRequestBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_EQ(writer.Finalize(&aPayload), CHIP_NO_ERROR);
}

void TestWriteInteraction::GenerateWriteResponse(System::PacketBufferHandle & aPayload)
{
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    WriteResponseMessage::Builder writeResponseBuilder;
    EXPECT_EQ(writeResponseBuilder.Init(&writer), CHIP_NO_ERROR);
    AttributeStatusIBs::Builder & attributeStatusesBuilder = writeResponseBuilder.CreateWriteResponses();
    EXPECT_EQ(attributeStatusesBuilder.GetError(), CHIP_NO_ERROR);
    AttributeStatusIB::Builder & attributeStatusIBBuilder = attributeStatusesBuilder.CreateAttributeStatus();
    EXPECT_EQ(attributeStatusIBBuilder.GetError(), CHIP_NO_ERROR);

    AttributePathIB::Builder & attributePathBuilder = attributeStatusIBBuilder.CreatePath();
    EXPECT_EQ(attributePathBuilder.GetError(), CHIP_NO_ERROR);
    EXPECT_EQ(attributePathBuilder.Node(1)
                  .Endpoint(2)
                  .Cluster(3)
                  .Attribute(4)
                  .ListIndex(DataModel::Nullable<ListIndex>())
                  .EndOfAttributePathIB(),
              CHIP_NO_ERROR);

    StatusIB::Builder & statusIBBuilder = attributeStatusIBBuilder.CreateErrorStatus();
    StatusIB statusIB;
    statusIB.mStatus = chip::Protocols::InteractionModel::Status::InvalidSubscription;
    EXPECT_EQ(statusIBBuilder.GetError(), CHIP_NO_ERROR);
    statusIBBuilder.EncodeStatusIB(statusIB);
    EXPECT_EQ(statusIBBuilder.GetError(), CHIP_NO_ERROR);

    attributeStatusIBBuilder.EndOfAttributeStatusIB();
    EXPECT_EQ(attributeStatusIBBuilder.GetError(), CHIP_NO_ERROR);

    attributeStatusesBuilder.EndOfAttributeStatuses();
    EXPECT_EQ(attributeStatusesBuilder.GetError(), CHIP_NO_ERROR);
    writeResponseBuilder.EndOfWriteResponseMessage();
    EXPECT_EQ(writeResponseBuilder.GetError(), CHIP_NO_ERROR);

    EXPECT_EQ(writer.Finalize(&aPayload), CHIP_NO_ERROR);
}

TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteClient)
{

    TestWriteClientCallback callback;
    app::WriteClient writeClient(&GetExchangeManager(), &callback, /* aTimedWriteTimeoutMs = */ NullOptional);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(writeClient);

    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    GenerateWriteResponse(buf);

    EXPECT_EQ(writeClient.ProcessWriteResponseMessage(std::move(buf)), CHIP_NO_ERROR);

    writeClient.Close();

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteClientGroup)
{

    TestWriteClientCallback callback;
    app::WriteClient writeClient(&GetExchangeManager(), &callback, /* aTimedWriteTimeoutMs = */ NullOptional);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(writeClient);

    SessionHandle groupSession = GetSessionBobToFriends();
    EXPECT_TRUE(groupSession->IsGroupSession());

    EXPECT_EQ(writeClient.SendWriteRequest(groupSession), CHIP_NO_ERROR);

    DrainAndServiceIO();

    // The WriteClient should be shutdown once we SendWriteRequest for group.
    EXPECT_EQ(writeClient.mState, WriteClient::State::AwaitingDestruction);
}

TEST_F(TestWriteInteraction, TestWriteHandler)
{
    using namespace Protocols::InteractionModel;

    constexpr bool allBooleans[] = { true, false };
    for (auto messageIsTimed : allBooleans)
    {
        for (auto transactionIsTimed : allBooleans)
        {

            app::WriteHandler writeHandler;

            System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

            writeHandler.Init(chip::app::InteractionModelEngine::GetInstance()->GetDataModelProvider(),
                              chip::app::InteractionModelEngine::GetInstance());

            GenerateWriteRequest(messageIsTimed, buf);

            TestExchangeDelegate delegate;
            Messaging::ExchangeContext * exchange = NewExchangeToBob(&delegate);

            Status status = writeHandler.OnWriteRequest(exchange, std::move(buf), transactionIsTimed);
            if (messageIsTimed == transactionIsTimed)
            {
                EXPECT_EQ(status, Status::Success);
            }
            else
            {
                EXPECT_EQ(status, Status::TimedRequestMismatch);
            }

            DrainAndServiceIO();

            Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
            EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
        }
    }
}

TEST_F(TestWriteInteraction, TestWriteRoundtripWithClusterObjects)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = 2;
    attributePathParams.mClusterId   = 3;
    attributePathParams.mAttributeId = 4;

    const uint8_t byteSpanData[]     = { 0xde, 0xad, 0xbe, 0xef };
    static const char charSpanData[] = "a simple test string";

    app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTx;
    dataTx.a = 12;
    dataTx.b = true;
    dataTx.d = chip::ByteSpan(byteSpanData);
    // Spec A.11.2 strings SHALL NOT include a terminating null character to mark the end of a string.
    dataTx.e = chip::Span<const char>(charSpanData, strlen(charSpanData));

    EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx), CHIP_NO_ERROR);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);

    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(callback.mOnSuccessCalled, 1);

    {
        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataRx;
        TLV::TLVReader reader;
        reader.Init(chip::Test::attributeDataTLV, chip::Test::attributeDataTLVLen);
        reader.Next();
        EXPECT_EQ(CHIP_NO_ERROR, DataModel::Decode(reader, dataRx));
        EXPECT_EQ(dataRx.a, dataTx.a);
        EXPECT_EQ(dataRx.b, dataTx.b);
        EXPECT_TRUE(dataRx.d.data_equal(dataTx.d));
        // Equals to dataRx.e.size() == dataTx.e.size() && memncmp(dataRx.e.data(), dataTx.e.data(), dataTx.e.size()) == 0
        EXPECT_TRUE(dataRx.e.data_equal(dataTx.e));
    }

    EXPECT_EQ(callback.mOnSuccessCalled, 1);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 1);

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    engine->Shutdown();
}

TEST_F(TestWriteInteraction, TestWriteRoundtripWithClusterObjectsVersionMatch)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = 2;
    attributePathParams.mClusterId   = 3;
    attributePathParams.mAttributeId = 4;

    DataModel::Nullable<app::Clusters::UnitTesting::Structs::SimpleStruct::Type> dataTx;

    Optional<DataVersion> version(kAcceptedDataVersion);

    EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx, version), CHIP_NO_ERROR);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);

    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(callback.mOnSuccessCalled, 1);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 1);
    EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::Success);

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    engine->Shutdown();
}

TEST_F(TestWriteInteraction, TestWriteRoundtripWithClusterObjectsVersionMismatch)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = 2;
    attributePathParams.mClusterId   = 3;
    attributePathParams.mAttributeId = 4;

    app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTxValue;
    dataTxValue.a = 12;
    dataTxValue.b = true;
    DataModel::Nullable<app::Clusters::UnitTesting::Structs::SimpleStruct::Type> dataTx;
    dataTx.SetNonNull(dataTxValue);
    Optional<DataVersion> version(chip::Test::kRejectedDataVersion);

    EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx, version), CHIP_NO_ERROR);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);

    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(callback.mOnSuccessCalled, 1);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 1);
    EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::DataVersionMismatch);

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    engine->Shutdown();
}

TEST_F(TestWriteInteraction, TestWriteRoundtrip)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(writeClient);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 0);

    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(callback.mOnSuccessCalled, 1);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 1);
    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    engine->Shutdown();
}

// This test creates a chunked write request, we drop the second write chunk message, then write handler receives unknown
// report message and sends out a status report with invalid action.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteHandlerReceiveInvalidMessage)
{
    auto sessionHandle = GetSessionBobToAlice();

    app::AttributePathParams attributePath(2, 3, 4);

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback writeCallback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    // Reserve all except the last 128 bytes, so that we make sure to chunk.
    app::WriteClient writeClient(&GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
                                 static_cast<uint16_t>(kMaxSecureSduLengthBytes - 128) /* reserved buffer size */);

    ByteSpan list[5];

    EXPECT_EQ(writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, 5)), CHIP_NO_ERROR);

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 2;
    EXPECT_EQ(writeClient.SendWriteRequest(sessionHandle), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 1u);
    EXPECT_EQ(GetLoopback().mSentMessageCount, 3u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

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

    auto * writeHandler = InteractionModelEngine::GetInstance()->ActiveWriteHandlerAt(0);

    rm->ClearRetransTable(writeClient.mExchangeCtx.Get());
    rm->ClearRetransTable(writeHandler->mExchangeCtx.Get());
    GetLoopback().mSentMessageCount  = 0;
    GetLoopback().mNumMessagesToDrop = 0;
    writeHandler->OnMessageReceived(writeHandler->mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    DrainAndServiceIO();

    EXPECT_EQ(writeCallback.mLastErrorReason.mStatus, Protocols::InteractionModel::Status::InvalidAction);
    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// This test is to create Chunked write requests, we drop the message since the 3rd message, then remove fabrics for client and
// handler, the corresponding client and handler would be released as well.
TEST_F(TestWriteInteraction, TestWriteHandlerInvalidateFabric)
{
    auto sessionHandle = GetSessionBobToAlice();

    app::AttributePathParams attributePath(2, 3, 4);

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback writeCallback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    // Reserve all except the last 128 bytes, so that we make sure to chunk.
    app::WriteClient writeClient(&GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
                                 static_cast<uint16_t>(kMaxSecureSduLengthBytes - 128) /* reserved buffer size */);

    ByteSpan list[5];

    EXPECT_EQ(writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, 5)), CHIP_NO_ERROR);

    GetLoopback().mDroppedMessageCount              = 0;
    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 2;
    EXPECT_EQ(writeClient.SendWriteRequest(sessionHandle), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 1u);
    EXPECT_EQ(GetLoopback().mSentMessageCount, 3u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

    GetFabricTable().Delete(GetAliceFabricIndex());
    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateAliceFabric();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

#endif

// This test sends an invalid (because there is no application payload at all)
// Write Request message and makes sure that a correct Status Response is
// received.
TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteHandlerReceiveEmptyWriteRequest)
{
    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback writeCallback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    GetLoopback().mDroppedMessageCount = 0;
    GetLoopback().mSentMessageCount    = 0;
    GetLoopback().mNumMessagesToDrop   = 0;

    // Just send an empty message claiming to be a write request.
    System::PacketBufferHandle emptyMessage = System::PacketBufferHandle::New(Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    TestExchangeDelegate delegate;
    delegate.mLastStatusParsedSuccessfully = false;

    auto exchange = NewExchangeToAlice(&delegate);
    EXPECT_NE(exchange, nullptr);

    EXPECT_EQ(exchange->SendMessage(Protocols::InteractionModel::MsgType::WriteRequest, std::move(emptyMessage),
                                    Messaging::SendMessageFlags::kExpectResponse),
              CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers(), 0u);
    EXPECT_EQ(GetLoopback().mSentMessageCount, 3u); // Request, response, ack
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 0u);

    EXPECT_TRUE(delegate.mLastStatusParsedSuccessfully);
    EXPECT_EQ(delegate.mLastStatus, Protocols::InteractionModel::Status::InvalidAction);

    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Write Client sends a write request, receives an unexpected message type, sends a status response to that.
TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteInvalidMessage1)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(writeClient);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 0);

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

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
    PretendWeGotReplyFromServer(*this, writeClient.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_ERROR_INVALID_MESSAGE_TYPE);
    DrainAndServiceIO();
    EXPECT_EQ(callback.mError, CHIP_ERROR_INVALID_MESSAGE_TYPE);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 1);
    EXPECT_EQ(callback.mOnDoneCalled, 1);

    // TODO: Check that the server gets the right status.
    // Client sents status report with invalid action, server's exchange has been closed, so all it sends is an MRP Ack
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Write Client sends a write request, receives a malformed write response message, sends a Status Report.
TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteInvalidMessage2)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(writeClient);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 0);

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    WriteResponseMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::WriteResponse);

    // Since we are dropping packets, things are not getting acked.  Set up
    // our MRP state to look like what it would have looked like if the
    // packet had not gotten dropped.
    PretendWeGotReplyFromServer(*this, writeClient.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_ERROR_END_OF_TLV);
    DrainAndServiceIO();
    EXPECT_EQ(callback.mError, CHIP_ERROR_END_OF_TLV);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 1);
    EXPECT_EQ(callback.mOnDoneCalled, 1);

    // Client sents status report with invalid action, server's exchange has been closed, so all it sends is an MRP Ack
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Write Client sends a write request, receives a malformed status response message.
TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteInvalidMessage3)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(writeClient);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 0);

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    EXPECT_FALSE(msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    StatusResponseMessage::Builder response;
    response.Init(&writer);
    EXPECT_EQ(writer.Finalize(&msgBuf), CHIP_NO_ERROR);
    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

    // Since we are dropping packets, things are not getting acked.  Set up
    // our MRP state to look like what it would have looked like if the
    // packet had not gotten dropped.
    PretendWeGotReplyFromServer(*this, writeClient.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_ERROR_END_OF_TLV);
    DrainAndServiceIO();
    EXPECT_EQ(callback.mError, CHIP_ERROR_END_OF_TLV);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 1);
    EXPECT_EQ(callback.mOnDoneCalled, 1);

    // TODO: Check that the server gets the right status
    // Client sents status report with invalid action, server's exchange has been closed, so all it sends is an MRP ack.
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

// Write Client sends a write request, receives a busy status response message.
TEST_F_FROM_FIXTURE(TestWriteInteraction, TestWriteInvalidMessage4)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(writeClient);

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 0);
    EXPECT_EQ(callback.mOnDoneCalled, 0);

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 1;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);
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

    // Since we are dropping packets, things are not getting acked.  Set up
    // our MRP state to look like what it would have looked like if the
    // packet had not gotten dropped.
    PretendWeGotReplyFromServer(*this, writeClient.mExchangeCtx.Get());

    GetLoopback().mSentMessageCount                 = 0;
    GetLoopback().mNumMessagesToDrop                = 0;
    GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    GetLoopback().mDroppedMessageCount              = 0;
    EXPECT_EQ(writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf)),
              CHIP_IM_GLOBAL_STATUS(Busy));
    DrainAndServiceIO();
    EXPECT_EQ(callback.mError, CHIP_IM_GLOBAL_STATUS(Busy));

    EXPECT_EQ(callback.mOnSuccessCalled, 0);
    EXPECT_EQ(callback.mOnErrorCalled, 1);
    EXPECT_EQ(callback.mOnDoneCalled, 1);

    // TODO: Check that the server gets the right status..
    // Client sents status report with invalid action, server's exchange has been closed, so it just sends an MRP ack.
    EXPECT_EQ(GetLoopback().mSentMessageCount, 2u);

    engine->Shutdown();
    ExpireSessionAliceToBob();
    ExpireSessionBobToAlice();
    CreateSessionAliceToBob();
    CreateSessionBobToAlice();
}

} // namespace app
} // namespace chip
