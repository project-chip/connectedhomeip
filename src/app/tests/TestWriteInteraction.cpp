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
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>

#include <memory>
#include <nlunit-test.h>
#include <utility>

namespace {

uint8_t attributeDataTLV[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
size_t attributeDataTLVLen                       = 0;
constexpr chip::DataVersion kRejectedDataVersion = 1;
constexpr chip::DataVersion kAcceptedDataVersion = 5;
constexpr uint16_t kMaxGroupsPerFabric           = 5;
constexpr uint16_t kMaxGroupKeysPerFabric        = 8;

chip::TestPersistentStorageDelegate gTestStorage;
chip::Crypto::DefaultSessionKeystore gSessionKeystore;
chip::Credentials::GroupDataProviderImpl gGroupsProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);

class TestContext : public chip::Test::AppContext
{
public:
    // Performs setup for each individual test in the test suite
    CHIP_ERROR SetUp() override
    {
        ReturnErrorOnFailure(chip::Test::AppContext::SetUp());

        gTestStorage.ClearStorage();
        gGroupsProvider.SetStorageDelegate(&gTestStorage);
        gGroupsProvider.SetSessionKeystore(&gSessionKeystore);
        ReturnErrorOnFailure(gGroupsProvider.Init());
        chip::Credentials::SetGroupDataProvider(&gGroupsProvider);

        uint8_t buf[sizeof(chip::CompressedFabricId)];
        chip::MutableByteSpan span(buf);
        ReturnErrorOnFailure(GetBobFabric()->GetCompressedFabricIdBytes(span));
        ReturnErrorOnFailure(chip::GroupTesting::InitData(&gGroupsProvider, GetBobFabricIndex(), span));

        return CHIP_NO_ERROR;
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        chip::Credentials::GroupDataProvider * provider = chip::Credentials::GetGroupDataProvider();
        if (provider != nullptr)
            provider->Finish();
        chip::Test::AppContext::TearDown();
    }
};

} // namespace

namespace chip {
namespace app {
class TestWriteInteraction
{
public:
    static void TestWriteClient(nlTestSuite * apSuite, void * apContext);
    static void TestWriteClientGroup(nlTestSuite * apSuite, void * apContext);
    static void TestWriteHandler(nlTestSuite * apSuite, void * apContext);
    static void TestWriteRoundtrip(nlTestSuite * apSuite, void * apContext);
    static void TestWriteInvalidMessage1(nlTestSuite * apSuite, void * apContext);
    static void TestWriteInvalidMessage2(nlTestSuite * apSuite, void * apContext);
    static void TestWriteInvalidMessage3(nlTestSuite * apSuite, void * apContext);
    static void TestWriteInvalidMessage4(nlTestSuite * apSuite, void * apContext);
    static void TestWriteRoundtripWithClusterObjects(nlTestSuite * apSuite, void * apContext);
    static void TestWriteRoundtripWithClusterObjectsVersionMatch(nlTestSuite * apSuite, void * apContext);
    static void TestWriteRoundtripWithClusterObjectsVersionMismatch(nlTestSuite * apSuite, void * apContext);
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    static void TestWriteHandlerReceiveInvalidMessage(nlTestSuite * apSuite, void * apContext);
    static void TestWriteHandlerInvalidateFabric(nlTestSuite * apSuite, void * apContext);
#endif
private:
    static void AddAttributeDataIB(nlTestSuite * apSuite, void * apContext, WriteClient & aWriteClient);
    static void AddAttributeStatus(nlTestSuite * apSuite, void * apContext, WriteHandler & aWriteHandler);
    static void GenerateWriteRequest(nlTestSuite * apSuite, void * apContext, bool aIsTimedWrite,
                                     System::PacketBufferHandle & aPayload);
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

void TestWriteInteraction::AddAttributeDataIB(nlTestSuite * apSuite, void * apContext, WriteClient & aWriteClient)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathParams attributePathParams;
    bool attributeValue              = true;
    attributePathParams.mEndpointId  = 2;
    attributePathParams.mClusterId   = 3;
    attributePathParams.mAttributeId = 4;

    err = aWriteClient.EncodeAttribute(attributePathParams, attributeValue);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::AddAttributeStatus(nlTestSuite * apSuite, void * apContext, WriteHandler & aWriteHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ConcreteAttributePath attributePath(2, 3, 4);

    err = aWriteHandler.AddStatus(attributePath, Protocols::InteractionModel::Status::Success);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::GenerateWriteRequest(nlTestSuite * apSuite, void * apContext, bool aIsTimedWrite,
                                                System::PacketBufferHandle & aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    WriteRequestMessage::Builder writeRequestBuilder;
    err = writeRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    writeRequestBuilder.TimedRequest(aIsTimedWrite);
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
    AttributeDataIBs::Builder & attributeDataIBsBuilder = writeRequestBuilder.CreateWriteRequests();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
    AttributeDataIB::Builder & attributeDataIBBuilder = attributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    NL_TEST_ASSERT(apSuite, attributeDataIBsBuilder.GetError() == CHIP_NO_ERROR);

    attributeDataIBBuilder.DataVersion(0);
    NL_TEST_ASSERT(apSuite, attributeDataIBBuilder.GetError() == CHIP_NO_ERROR);
    AttributePathIB::Builder & attributePathBuilder = attributeDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    err = attributePathBuilder.Node(1)
              .Endpoint(2)
              .Cluster(3)
              .Attribute(4)
              .ListIndex(DataModel::Nullable<ListIndex>())
              .EndOfAttributePathIB();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = attributeDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(AttributeDataIB::Tag::kData), chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    attributeDataIBBuilder.EndOfAttributeDataIB();
    NL_TEST_ASSERT(apSuite, attributeDataIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeDataIBsBuilder.EndOfAttributeDataIBs();
    NL_TEST_ASSERT(apSuite, attributeDataIBsBuilder.GetError() == CHIP_NO_ERROR);
    writeRequestBuilder.EndOfWriteRequestMessage();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::GenerateWriteResponse(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    WriteResponseMessage::Builder writeResponseBuilder;
    err = writeResponseBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    AttributeStatusIBs::Builder & attributeStatusesBuilder = writeResponseBuilder.CreateWriteResponses();
    NL_TEST_ASSERT(apSuite, attributeStatusesBuilder.GetError() == CHIP_NO_ERROR);
    AttributeStatusIB::Builder & attributeStatusIBBuilder = attributeStatusesBuilder.CreateAttributeStatus();
    NL_TEST_ASSERT(apSuite, attributeStatusIBBuilder.GetError() == CHIP_NO_ERROR);

    AttributePathIB::Builder & attributePathBuilder = attributeStatusIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    err = attributePathBuilder.Node(1)
              .Endpoint(2)
              .Cluster(3)
              .Attribute(4)
              .ListIndex(DataModel::Nullable<ListIndex>())
              .EndOfAttributePathIB();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    StatusIB::Builder & statusIBBuilder = attributeStatusIBBuilder.CreateErrorStatus();
    StatusIB statusIB;
    statusIB.mStatus = chip::Protocols::InteractionModel::Status::InvalidSubscription;
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    statusIBBuilder.EncodeStatusIB(statusIB);
    err = statusIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeStatusIBBuilder.EndOfAttributeStatusIB();
    NL_TEST_ASSERT(apSuite, attributeStatusIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeStatusesBuilder.EndOfAttributeStatuses();
    NL_TEST_ASSERT(apSuite, attributeStatusesBuilder.GetError() == CHIP_NO_ERROR);
    writeResponseBuilder.EndOfWriteResponseMessage();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestWriteInteraction::TestWriteClient(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    TestWriteClientCallback callback;
    app::WriteClient writeClient(&ctx.GetExchangeManager(), &callback, /* aTimedWriteTimeoutMs = */ NullOptional);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(apSuite, apContext, writeClient);

    err = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    GenerateWriteResponse(apSuite, apContext, buf);

    err = writeClient.ProcessWriteResponseMessage(std::move(buf));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    writeClient.Close();

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
}

void TestWriteInteraction::TestWriteClientGroup(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    TestWriteClientCallback callback;
    app::WriteClient writeClient(&ctx.GetExchangeManager(), &callback, /* aTimedWriteTimeoutMs = */ NullOptional);

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(apSuite, apContext, writeClient);

    SessionHandle groupSession = ctx.GetSessionBobToFriends();
    NL_TEST_ASSERT(apSuite, groupSession->IsGroupSession());

    err = writeClient.SendWriteRequest(groupSession);

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    // The WriteClient should be shutdown once we SendWriteRequest for group.
    NL_TEST_ASSERT(apSuite, writeClient.mState == WriteClient::State::AwaitingDestruction);
}

void TestWriteInteraction::TestWriteHandler(nlTestSuite * apSuite, void * apContext)
{
    using namespace Protocols::InteractionModel;

    TestContext & ctx = *static_cast<TestContext *>(apContext);

    constexpr bool allBooleans[] = { true, false };
    for (auto messageIsTimed : allBooleans)
    {
        for (auto transactionIsTimed : allBooleans)
        {
            CHIP_ERROR err = CHIP_NO_ERROR;

            app::WriteHandler writeHandler;

            System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
            err                            = writeHandler.Init();

            GenerateWriteRequest(apSuite, apContext, messageIsTimed, buf);

            TestExchangeDelegate delegate;
            Messaging::ExchangeContext * exchange = ctx.NewExchangeToBob(&delegate);

            Status status = writeHandler.OnWriteRequest(exchange, std::move(buf), transactionIsTimed);
            if (messageIsTimed == transactionIsTimed)
            {
                NL_TEST_ASSERT(apSuite, status == Status::Success);
            }
            else
            {
                NL_TEST_ASSERT(apSuite, status == Status::UnsupportedAccess);
            }

            ctx.DrainAndServiceIO();

            Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
            NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);
        }
    }
}

const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath)
{
    // Note: This test does not make use of the real attribute metadata.
    static EmberAfAttributeMetadata stub = { .defaultValue = EmberAfDefaultOrMinMaxAttributeValue(uint32_t(0)) };
    return &stub;
}

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    if (aPath.mDataVersion.HasValue() && aPath.mDataVersion.Value() == kRejectedDataVersion)
    {
        return aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::DataVersionMismatch);
    }

    TLV::TLVWriter writer;
    writer.Init(attributeDataTLV);
    writer.CopyElement(TLV::AnonymousTag(), aReader);
    attributeDataTLVLen = writer.GetLengthWritten();
    return aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
}

void TestWriteInteraction::TestWriteRoundtripWithClusterObjects(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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

    writeClient.EncodeAttribute(attributePathParams, dataTx);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0);

    err = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 1);

    {
        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataRx;
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

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 1 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 1);

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
}

void TestWriteInteraction::TestWriteRoundtripWithClusterObjectsVersionMatch(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = 2;
    attributePathParams.mClusterId   = 3;
    attributePathParams.mAttributeId = 4;

    DataModel::Nullable<app::Clusters::UnitTesting::Structs::SimpleStruct::Type> dataTx;

    Optional<DataVersion> version(kAcceptedDataVersion);

    writeClient.EncodeAttribute(attributePathParams, dataTx, version);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0);

    err = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   callback.mOnSuccessCalled == 1 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 1 &&
                       callback.mStatus.mStatus == Protocols::InteractionModel::Status::Success);

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
}

void TestWriteInteraction::TestWriteRoundtripWithClusterObjectsVersionMismatch(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

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
    Optional<DataVersion> version(kRejectedDataVersion);
    writeClient.EncodeAttribute(attributePathParams, dataTx, version);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0);

    err = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   callback.mOnSuccessCalled == 1 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 1 &&
                       callback.mStatus.mStatus == Protocols::InteractionModel::Status::DataVersionMismatch);

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

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(apSuite, apContext, writeClient);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 0);

    err = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 1 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 1);

    // By now we should have closed all exchanges and sent all pending acks, so
    // there should be no queued-up things in the retransmit table.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    engine->Shutdown();
}

// This test creates a chunked write request, we drop the second write chunk message, then write handler receives unknown
// report message and sends out a status report with invalid action.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
void TestWriteInteraction::TestWriteHandlerReceiveInvalidMessage(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    app::AttributePathParams attributePath(2, 3, 4);

    CHIP_ERROR err                     = CHIP_NO_ERROR;
    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback writeCallback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Reserve all except the last 128 bytes, so that we make sure to chunk.
    app::WriteClient writeClient(&ctx.GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
                                 static_cast<uint16_t>(kMaxSecureSduLengthBytes - 128) /* reserved buffer size */);

    ByteSpan list[5];

    err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, 5));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 2;
    err                                                 = writeClient.SendWriteRequest(sessionHandle);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 1);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 3);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

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

    auto * writeHandler = InteractionModelEngine::GetInstance()->ActiveWriteHandlerAt(0);
    rm->ClearRetransTable(writeClient.mExchangeCtx.Get());
    rm->ClearRetransTable(writeHandler->mExchangeCtx.Get());
    ctx.GetLoopback().mSentMessageCount  = 0;
    ctx.GetLoopback().mNumMessagesToDrop = 0;
    writeHandler->OnMessageReceived(writeHandler->mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, writeCallback.mLastErrorReason.mStatus == Protocols::InteractionModel::Status::InvalidAction);
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// This test is to create Chunked write requests, we drop the message since the 3rd message, then remove fabrics for client and
// handler, the corresponding client and handler would be released as well.
void TestWriteInteraction::TestWriteHandlerInvalidateFabric(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    app::AttributePathParams attributePath(2, 3, 4);

    CHIP_ERROR err                     = CHIP_NO_ERROR;
    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback writeCallback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Reserve all except the last 128 bytes, so that we make sure to chunk.
    app::WriteClient writeClient(&ctx.GetExchangeManager(), &writeCallback, Optional<uint16_t>::Missing(),
                                 static_cast<uint16_t>(kMaxSecureSduLengthBytes - 128) /* reserved buffer size */);

    ByteSpan list[5];

    err = writeClient.EncodeAttribute(attributePath, app::DataModel::List<ByteSpan>(list, 5));
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ctx.GetLoopback().mDroppedMessageCount              = 0;
    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 2;
    err                                                 = writeClient.SendWriteRequest(sessionHandle);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 1);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 3);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

    ctx.GetFabricTable().Delete(ctx.GetAliceFabricIndex());
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateAliceFabric();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

#endif

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

// Write Client sends a write request, receives an unexpected message type, sends a status response to that.
void TestWriteInteraction::TestWriteInvalidMessage1(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(apSuite, apContext, writeClient);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 0);

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err                                                 = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

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
    PretendWeGotReplyFromServer(apSuite, ctx, writeClient.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err = writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_INVALID_MESSAGE_TYPE);
    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(apSuite, callback.mError == CHIP_ERROR_INVALID_MESSAGE_TYPE);
    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 1 && callback.mOnDoneCalled == 1);

    // TODO: Check that the server gets the right status.
    // Client sents status report with invalid action, server's exchange has been closed, so all it sends is an MRP Ack
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Write Client sends a write request, receives a malformed write response message, sends a Status Report.
void TestWriteInteraction::TestWriteInvalidMessage2(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(apSuite, apContext, writeClient);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 0);

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err                                                 = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mDroppedMessageCount == 1);

    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    NL_TEST_ASSERT(apSuite, !msgBuf.IsNull());
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(msgBuf));
    WriteResponseMessage::Builder response;
    response.Init(&writer);
    NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);
    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::WriteResponse);

    // Since we are dropping packets, things are not getting acked.  Set up
    // our MRP state to look like what it would have looked like if the
    // packet had not gotten dropped.
    PretendWeGotReplyFromServer(apSuite, ctx, writeClient.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err = writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_END_OF_TLV);
    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(apSuite, callback.mError == CHIP_ERROR_END_OF_TLV);
    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 1 && callback.mOnDoneCalled == 1);

    // Client sents status report with invalid action, server's exchange has been closed, so all it sends is an MRP Ack
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Write Client sends a write request, receives a malformed status response message.
void TestWriteInteraction::TestWriteInvalidMessage3(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(apSuite, apContext, writeClient);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 0);

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err                                                 = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
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
    NL_TEST_ASSERT(apSuite, writer.Finalize(&msgBuf) == CHIP_NO_ERROR);
    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::StatusResponse);

    // Since we are dropping packets, things are not getting acked.  Set up
    // our MRP state to look like what it would have looked like if the
    // packet had not gotten dropped.
    PretendWeGotReplyFromServer(apSuite, ctx, writeClient.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err = writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_END_OF_TLV);
    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(apSuite, callback.mError == CHIP_ERROR_END_OF_TLV);
    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 1 && callback.mOnDoneCalled == 1);

    // TODO: Check that the server gets the right status
    // Client sents status report with invalid action, server's exchange has been closed, so all it sends is an MRP ack.
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
}

// Write Client sends a write request, receives a busy status response message.
void TestWriteInteraction::TestWriteInvalidMessage4(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    TestWriteClientCallback callback;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    AddAttributeDataIB(apSuite, apContext, writeClient);

    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 0 && callback.mOnDoneCalled == 0);

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 1;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 1;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err                                                 = writeClient.SendWriteRequest(ctx.GetSessionBobToAlice());
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

    // Since we are dropping packets, things are not getting acked.  Set up
    // our MRP state to look like what it would have looked like if the
    // packet had not gotten dropped.
    PretendWeGotReplyFromServer(apSuite, ctx, writeClient.mExchangeCtx.Get());

    ctx.GetLoopback().mSentMessageCount                 = 0;
    ctx.GetLoopback().mNumMessagesToDrop                = 0;
    ctx.GetLoopback().mNumMessagesToAllowBeforeDropping = 0;
    ctx.GetLoopback().mDroppedMessageCount              = 0;
    err = writeClient.OnMessageReceived(writeClient.mExchangeCtx.Get(), payloadHeader, std::move(msgBuf));
    NL_TEST_ASSERT(apSuite, err == CHIP_IM_GLOBAL_STATUS(Busy));
    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(apSuite, callback.mError == CHIP_IM_GLOBAL_STATUS(Busy));
    NL_TEST_ASSERT(apSuite, callback.mOnSuccessCalled == 0 && callback.mOnErrorCalled == 1 && callback.mOnDoneCalled == 1);

    // TODO: Check that the server gets the right status..
    // Client sents status report with invalid action, server's exchange has been closed, so it just sends an MRP ack.
    NL_TEST_ASSERT(apSuite, ctx.GetLoopback().mSentMessageCount == 2);

    engine->Shutdown();
    ctx.ExpireSessionAliceToBob();
    ctx.ExpireSessionBobToAlice();
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();
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
        NL_TEST_DEF("CheckWriteClientGroup", chip::app::TestWriteInteraction::TestWriteClientGroup),
        NL_TEST_DEF("CheckWriteHandler", chip::app::TestWriteInteraction::TestWriteHandler),
        NL_TEST_DEF("CheckWriteRoundtrip", chip::app::TestWriteInteraction::TestWriteRoundtrip),
        NL_TEST_DEF("TestWriteRoundtripWithClusterObjects", chip::app::TestWriteInteraction::TestWriteRoundtripWithClusterObjects),
        NL_TEST_DEF("TestWriteRoundtripWithClusterObjectsVersionMatch", chip::app::TestWriteInteraction::TestWriteRoundtripWithClusterObjectsVersionMatch),
        NL_TEST_DEF("TestWriteRoundtripWithClusterObjectsVersionMismatch", chip::app::TestWriteInteraction::TestWriteRoundtripWithClusterObjectsVersionMismatch),
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        NL_TEST_DEF("TestWriteHandlerReceiveInvalidMessage", chip::app::TestWriteInteraction::TestWriteHandlerReceiveInvalidMessage),
        NL_TEST_DEF("TestWriteHandlerInvalidateFabric", chip::app::TestWriteInteraction::TestWriteHandlerInvalidateFabric),
#endif
        NL_TEST_DEF("TestWriteInvalidMessage1", chip::app::TestWriteInteraction::TestWriteInvalidMessage1),
        NL_TEST_DEF("TestWriteInvalidMessage2", chip::app::TestWriteInteraction::TestWriteInvalidMessage2),
        NL_TEST_DEF("TestWriteInvalidMessage3", chip::app::TestWriteInteraction::TestWriteInvalidMessage3),
        NL_TEST_DEF("TestWriteInvalidMessage4", chip::app::TestWriteInteraction::TestWriteInvalidMessage4),
        NL_TEST_SENTINEL()
};
// clang-format on

nlTestSuite sSuite = {
    "TestWriteInteraction",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};

} // namespace

int TestWriteInteraction()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestWriteInteraction)
