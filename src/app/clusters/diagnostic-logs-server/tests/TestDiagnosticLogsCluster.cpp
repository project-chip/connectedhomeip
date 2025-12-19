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
#include <app/clusters/diagnostic-logs-server/DiagnosticLogsCluster.h>
#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <lib/support/Span.h>
#include <protocols/bdx/DiagnosticLogs.h>

#include <cstring>

namespace chip {
namespace app {

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DiagnosticLogs;
using chip::Testing::ClusterTester;

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

struct TestDiagnosticLogsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestDiagnosticLogsCluster, ResponsePayload_WithDelegate_Success)
{
    DiagnosticLogsCluster diagnosticLogsCluster;

    MockDelegate delegate;
    uint8_t buffer[100];
    delegate.SetDiagnosticBuffer(buffer, sizeof(buffer));
    diagnosticLogsCluster.SetDelegate(&delegate);

    ClusterTester tester(diagnosticLogsCluster);
    Commands::RetrieveLogsRequest::Type request;
    request.intent            = DiagnosticLogs::IntentEnum::kEndUserSupport;
    request.requestedProtocol = DiagnosticLogs::TransferProtocolEnum::kResponsePayload;

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().status, DiagnosticLogs::StatusEnum::kSuccess);
    EXPECT_EQ(result.response.value().logContent.size(), sizeof(buffer));
}

// If request is BDX but logs can fit in the response payload, the response should be kExhausted
TEST_F(TestDiagnosticLogsCluster, Bdx_WithDelegate_kExhausted)
{
    DiagnosticLogsCluster diagnosticLogsCluster;

    MockDelegate delegate;
    uint8_t buffer[1024];
    delegate.SetDiagnosticBuffer(buffer, sizeof(buffer));
    diagnosticLogsCluster.SetDelegate(&delegate);

    ClusterTester tester(diagnosticLogsCluster);
    Commands::RetrieveLogsRequest::Type request;
    request.intent                 = DiagnosticLogs::IntentEnum::kEndUserSupport;
    request.requestedProtocol      = DiagnosticLogs::TransferProtocolEnum::kBdx;
    request.transferFileDesignator = MakeOptional(CharSpan::fromCharString("enduser.log"));

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().status, DiagnosticLogs::StatusEnum::kExhausted);
    EXPECT_EQ(result.response.value().logContent.size(), sizeof(buffer));
}

TEST_F(TestDiagnosticLogsCluster, Bdx_WithDelegate_kExhausted_with_buffer_greater_than_kMaxLogContentSize)
{
    DiagnosticLogsCluster diagnosticLogsCluster;

    MockDelegate delegate;
    uint8_t buffer[2048];
    delegate.SetDiagnosticBuffer(buffer, sizeof(buffer));
    diagnosticLogsCluster.SetDelegate(&delegate);

    ClusterTester tester(diagnosticLogsCluster);
    Commands::RetrieveLogsRequest::Type request;
    request.intent                 = DiagnosticLogs::IntentEnum::kEndUserSupport;
    request.requestedProtocol      = DiagnosticLogs::TransferProtocolEnum::kBdx;
    request.transferFileDesignator = MakeOptional(CharSpan::fromCharString("enduser.log"));

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().status, DiagnosticLogs::StatusEnum::kExhausted);
    EXPECT_EQ(result.response.value().logContent.size(), static_cast<size_t>(chip::bdx::DiagnosticLogs::kMaxLogContentSize));
}

TEST_F(TestDiagnosticLogsCluster, ResponsePayload_NoDelegate_NoLogs)
{
    DiagnosticLogsCluster diagnosticLogsCluster;

    ClusterTester tester(diagnosticLogsCluster);
    Commands::RetrieveLogsRequest::Type request;
    request.intent            = DiagnosticLogs::IntentEnum::kEndUserSupport;
    request.requestedProtocol = DiagnosticLogs::TransferProtocolEnum::kResponsePayload;

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().status, DiagnosticLogs::StatusEnum::kNoLogs);
}

TEST_F(TestDiagnosticLogsCluster, ResponsePayload_ZeroBufferSize_NoLogs)
{
    DiagnosticLogsCluster diagnosticLogsCluster;

    MockDelegate delegate;
    uint8_t buffer[10];
    delegate.SetDiagnosticBuffer(buffer, 0);
    diagnosticLogsCluster.SetDelegate(&delegate);

    ClusterTester tester(diagnosticLogsCluster);
    Commands::RetrieveLogsRequest::Type request;
    request.intent            = DiagnosticLogs::IntentEnum::kEndUserSupport;
    request.requestedProtocol = DiagnosticLogs::TransferProtocolEnum::kResponsePayload;

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().status, DiagnosticLogs::StatusEnum::kNoLogs);
}

TEST_F(TestDiagnosticLogsCluster, Bdx_NoDelegate_NoLogs)
{
    DiagnosticLogsCluster diagnosticLogsCluster;

    ClusterTester tester(diagnosticLogsCluster);
    Commands::RetrieveLogsRequest::Type request;
    request.intent                 = DiagnosticLogs::IntentEnum::kEndUserSupport;
    request.requestedProtocol      = DiagnosticLogs::TransferProtocolEnum::kBdx;
    request.transferFileDesignator = MakeOptional(CharSpan::fromCharString("enduser.log"));

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().status, DiagnosticLogs::StatusEnum::kNoLogs);
}

} // namespace app
} // namespace chip
