/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/MessageDef/CommandDataIB.h>
#include <app/clusters/diagnostic-logs-server/DiagnosticLogsLogic.h>
#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>
#include <lib/support/Span.h>
#include <protocols/bdx/DiagnosticLogs.h>

#include <cstring>

namespace chip {
namespace app {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DiagnosticLogs;
using chip::Protocols::InteractionModel::Status;

static constexpr EndpointId kEndpoint0 = 0;

class MockCommandHandler : public CommandHandler
{
public:
    ~MockCommandHandler() override {}

    struct ResponseRecord
    {
        ConcreteCommandPath path;
        CommandId commandId;
        chip::System::PacketBufferHandle encodedData;
    };

    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                 const char * context = nullptr) override
    {
        return CHIP_NO_ERROR;
    }

    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                   const char * context = nullptr) override
    {
        CHIP_ERROR err = FallibleAddStatus(aRequestCommandPath, aStatus, context);
        VerifyOrDie(err == CHIP_NO_ERROR);
    }

    FabricIndex GetAccessingFabricIndex() const override { return mFabricIndex; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable) override
    {
        chip::System::PacketBufferHandle handle = chip::MessagePacketBuffer::New(1024);
        VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);
        TLV::TLVWriter baseWriter;
        baseWriter.Init(handle->Start(), handle->MaxDataLength());
        DataModel::FabricAwareTLVWriter writer(baseWriter, /*fabricIndex*/ 1);
        TLV::TLVType ct;
        ReturnErrorOnFailure(
            static_cast<TLV::TLVWriter &>(writer).StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ct));
        ReturnErrorOnFailure(aEncodable.EncodeTo(writer, TLV::ContextTag(app::CommandDataIB::Tag::kFields)));
        ReturnErrorOnFailure(static_cast<TLV::TLVWriter &>(writer).EndContainer(ct));
        handle->SetDataLength(static_cast<TLV::TLVWriter &>(writer).GetLengthWritten());
        mResponse.path        = aRequestCommandPath;
        mResponse.commandId   = aResponseCommandId;
        mResponse.encodedData = std::move(handle);
        return CHIP_NO_ERROR;
    }

    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable) override
    {
        (void) AddResponseData(aRequestCommandPath, aResponseCommandId, aEncodable);
    }

    bool IsTimedInvoke() const override { return false; }
    void FlushAcksRightAwayOnSlowCommand() override {}
    Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor{}; }
    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }

    const ResponseRecord & GetResponse() const { return mResponse; }

private:
    ResponseRecord mResponse;
    FabricIndex mFabricIndex = 0;
};

class MockDelegate : public DiagnosticLogs::DiagnosticLogsProviderDelegate
{
public:
    Optional<uint64_t> configuredTimestamp     = Optional<uint64_t>();
    Optional<uint64_t> configuredTimeSinceBoot = Optional<uint64_t>();

    CHIP_ERROR StartLogCollection(DiagnosticLogs::IntentEnum intent, DiagnosticLogs::LogSessionHandle & outHandle,
                                  Optional<uint64_t> & outTimeStamp, Optional<uint64_t> & outTimeSinceBoot) override
    {
        outHandle        = 1;
        outTimeStamp     = configuredTimestamp;
        outTimeSinceBoot = configuredTimeSinceBoot;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CollectLog(DiagnosticLogs::LogSessionHandle sessionHandle, MutableByteSpan & outBuffer,
                          bool & outIsEndOfLog) override
    {
        outIsEndOfLog = true;
        outBuffer     = MutableByteSpan(diagnosticBuffer, bufferSize);
        return CHIP_NO_ERROR;
    }

    size_t GetSizeForIntent(DiagnosticLogs::IntentEnum intent) override { return bufferSize; }

    CHIP_ERROR GetLogForIntent(DiagnosticLogs::IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp,
                               Optional<uint64_t> & outTimeSinceBoot) override
    {
        VerifyOrReturnError(diagnosticBuffer != nullptr, CHIP_ERROR_INTERNAL);
        size_t copySize = std::min(outBuffer.size(), static_cast<size_t>(bufferSize));
        memcpy(outBuffer.data(), diagnosticBuffer, copySize);
        outBuffer.reduce_size(copySize);
        outTimeStamp     = configuredTimestamp;
        outTimeSinceBoot = configuredTimeSinceBoot;
        return CHIP_NO_ERROR;
    }

    void SetDiagnosticBuffer(uint8_t * buffer, uint16_t size)
    {
        diagnosticBuffer = buffer;
        bufferSize       = size;
    }

private:
    uint8_t * diagnosticBuffer = nullptr;
    uint16_t bufferSize        = 0;
};

static Commands::RetrieveLogsResponse::DecodableType DecodeRetrieveLogsResponse(const MockCommandHandler::ResponseRecord & rec)
{
    TLV::TLVReader reader;
    reader.Init(rec.encodedData->Start(), static_cast<uint32_t>(rec.encodedData->DataLength()));

    CHIP_ERROR err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TLV::TLVReader outer;
    err = reader.OpenContainer(outer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = outer.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(IsContextTag(outer.GetTag()));
    EXPECT_EQ(TagNumFromTag(outer.GetTag()), chip::to_underlying(CommandDataIB::Tag::kFields));

    Commands::RetrieveLogsResponse::DecodableType decoded;
    EXPECT_EQ(decoded.Decode(outer), CHIP_NO_ERROR);
    return decoded;
}

struct TestDiagnosticLogsLogic : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestDiagnosticLogsLogic, ResponsePayload_NoDelegate_NoLogs)
{
    DiagnosticLogsProviderLogic logic;

    const ConcreteCommandPath kPath{ kEndpoint0, DiagnosticLogs::Id, DiagnosticLogs::Commands::RetrieveLogsRequest::Id };

    MockCommandHandler handler;
    EXPECT_EQ(logic.HandleLogRequestForResponsePayload(&handler, kPath, DiagnosticLogs::IntentEnum::kEndUserSupport),
              Status::InvalidCommand);
    EXPECT_EQ(handler.GetResponse().commandId, DiagnosticLogs::Commands::RetrieveLogsResponse::Id);
    auto decoded = DecodeRetrieveLogsResponse(handler.GetResponse());
    EXPECT_EQ(decoded.status, DiagnosticLogs::StatusEnum::kNoLogs);
}

TEST_F(TestDiagnosticLogsLogic, ResponsePayload_WithDelegate_Success)
{
    DiagnosticLogsProviderLogic logic;

    MockDelegate delegate;
    uint8_t buffer[100];
    delegate.SetDiagnosticBuffer(buffer, sizeof(buffer));
    logic.SetDelegate(&delegate);

    const ConcreteCommandPath kPath{ kEndpoint0, DiagnosticLogs::Id, DiagnosticLogs::Commands::RetrieveLogsRequest::Id };
    MockCommandHandler handler;
    EXPECT_EQ(logic.HandleLogRequestForResponsePayload(&handler, kPath, DiagnosticLogs::IntentEnum::kEndUserSupport),
              Status::InvalidCommand);
    EXPECT_EQ(handler.GetResponse().commandId, DiagnosticLogs::Commands::RetrieveLogsResponse::Id);
    auto decoded = DecodeRetrieveLogsResponse(handler.GetResponse());
    EXPECT_EQ(decoded.status, DiagnosticLogs::StatusEnum::kSuccess);
    size_t logContentSize = decoded.logContent.size();
    EXPECT_EQ(logContentSize, sizeof(buffer));
}

// If request is BDX but logs can fit in the response payload, the response should be kExhausted
TEST_F(TestDiagnosticLogsLogic, Bdx_WithDelegate_kExhausted)
{
    DiagnosticLogsProviderLogic logic;

    MockDelegate delegate;
    uint8_t buffer[1024];
    delegate.SetDiagnosticBuffer(buffer, sizeof(buffer));
    logic.SetDelegate(&delegate);

    const ConcreteCommandPath kPath{ kEndpoint0, DiagnosticLogs::Id, DiagnosticLogs::Commands::RetrieveLogsRequest::Id };
    MockCommandHandler handler;
    EXPECT_EQ(logic.HandleLogRequestForBdx(&handler, kPath, DiagnosticLogs::IntentEnum::kEndUserSupport,
                                           MakeOptional(CharSpan::fromCharString("enduser.log"))),
              Status::InvalidCommand);
    EXPECT_EQ(handler.GetResponse().commandId, DiagnosticLogs::Commands::RetrieveLogsResponse::Id);
    auto decoded = DecodeRetrieveLogsResponse(handler.GetResponse());
    EXPECT_EQ(decoded.status, DiagnosticLogs::StatusEnum::kExhausted);
    size_t logContentSize = decoded.logContent.size();
    EXPECT_EQ(logContentSize, sizeof(buffer));
}

TEST_F(TestDiagnosticLogsLogic, Bdx_WithDelegate_kExhausted_with_buffer_greater_than_kMaxLogContentSize)
{
    DiagnosticLogsProviderLogic logic;

    MockDelegate delegate;
    uint8_t buffer[2048];
    delegate.SetDiagnosticBuffer(buffer, sizeof(buffer));
    logic.SetDelegate(&delegate);

    const ConcreteCommandPath kPath{ kEndpoint0, DiagnosticLogs::Id, DiagnosticLogs::Commands::RetrieveLogsRequest::Id };
    MockCommandHandler handler;
    EXPECT_EQ(logic.HandleLogRequestForBdx(&handler, kPath, DiagnosticLogs::IntentEnum::kEndUserSupport,
                                           MakeOptional(CharSpan::fromCharString("enduser.log"))),
              Status::InvalidCommand);
    EXPECT_EQ(handler.GetResponse().commandId, DiagnosticLogs::Commands::RetrieveLogsResponse::Id);
    auto decoded = DecodeRetrieveLogsResponse(handler.GetResponse());
    EXPECT_EQ(decoded.status, DiagnosticLogs::StatusEnum::kExhausted);
    size_t logContentSize = decoded.logContent.size();

    // The buffer is greater than kMaxLogContentSize, so the log content is cropped to kMaxLogContentSize
    EXPECT_EQ(logContentSize, (size_t) 1024);
}

} // namespace app
} // namespace chip
