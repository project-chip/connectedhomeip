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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::Protocols::SecureChannel;

struct TestStatusReport : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        CHIP_ERROR error = chip::Platform::MemoryInit();
        ASSERT_EQ(error, CHIP_NO_ERROR);
    }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestStatusReport, NoData)
{
    GeneralStatusCode generalCode = GeneralStatusCode::kSuccess;
    auto protocolId               = SecureChannel::Id;
    uint16_t protocolCode         = kProtocolCodeSuccess;

    StatusReport testReport(generalCode, protocolId, protocolCode);

    size_t msgSize = testReport.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    testReport.WriteToBuffer(bbuf);

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    ASSERT_FALSE(msgBuf.IsNull());

    StatusReport reportToParse;
    EXPECT_EQ(reportToParse.Parse(std::move(msgBuf)), CHIP_NO_ERROR);
    EXPECT_EQ(reportToParse.GetGeneralCode(), generalCode);
    EXPECT_EQ(reportToParse.GetProtocolId(), protocolId);
    EXPECT_EQ(reportToParse.GetProtocolCode(), protocolCode);

    const System::PacketBufferHandle & data = reportToParse.GetProtocolData();
    EXPECT_TRUE(data.IsNull());
}

TEST_F(TestStatusReport, WithData)
{
    GeneralStatusCode generalCode      = GeneralStatusCode::kFailure;
    auto protocolId                    = SecureChannel::Id;
    uint16_t protocolCode              = static_cast<uint16_t>(StatusCode::InvalidFabricConfig);
    uint8_t data[6]                    = { 42, 19, 3, 1, 3, 0 };
    const uint16_t dataLen             = 6;
    System::PacketBufferHandle dataBuf = System::PacketBufferHandle::NewWithData(data, dataLen);

    StatusReport testReport(generalCode, protocolId, protocolCode, std::move(dataBuf));

    size_t msgSize = testReport.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    testReport.WriteToBuffer(bbuf);

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    ASSERT_FALSE(msgBuf.IsNull());

    StatusReport reportToParse;
    EXPECT_EQ(reportToParse.Parse(std::move(msgBuf)), CHIP_NO_ERROR);
    EXPECT_EQ(reportToParse.GetGeneralCode(), generalCode);
    EXPECT_EQ(reportToParse.GetProtocolId(), protocolId);
    EXPECT_EQ(reportToParse.GetProtocolCode(), protocolCode);

    const System::PacketBufferHandle & rcvData = reportToParse.GetProtocolData();
    ASSERT_FALSE(rcvData.IsNull());
    EXPECT_EQ(rcvData->DataLength(), dataLen);
    EXPECT_EQ(memcmp(rcvData->Start(), data, dataLen), 0);
}

TEST_F(TestStatusReport, TestBadStatusReport)
{
    StatusReport report;
    System::PacketBufferHandle badMsg = System::PacketBufferHandle::New(10);
    CHIP_ERROR err                    = report.Parse(std::move(badMsg));
    EXPECT_NE(err, CHIP_NO_ERROR);

    StatusReport report2;
    badMsg = nullptr;
    err    = report2.Parse(std::move(badMsg));
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST_F(TestStatusReport, TestMakeBusyStatusReport)
{
    GeneralStatusCode generalCode                 = GeneralStatusCode::kBusy;
    auto protocolId                               = SecureChannel::Id;
    uint16_t protocolCode                         = kProtocolCodeBusy;
    System::Clock::Milliseconds16 minimumWaitTime = System::Clock::Milliseconds16(5000);

    System::PacketBufferHandle handle = StatusReport::MakeBusyStatusReportMessage(minimumWaitTime);
    ASSERT_FALSE(handle.IsNull());

    StatusReport reportToParse;
    EXPECT_EQ(reportToParse.Parse(std::move(handle)), CHIP_NO_ERROR);
    EXPECT_EQ(reportToParse.GetGeneralCode(), generalCode);
    EXPECT_EQ(reportToParse.GetProtocolId(), protocolId);
    EXPECT_EQ(reportToParse.GetProtocolCode(), protocolCode);

    const System::PacketBufferHandle & rcvData = reportToParse.GetProtocolData();
    ASSERT_FALSE(rcvData.IsNull());
    EXPECT_EQ(rcvData->DataLength(), sizeof(minimumWaitTime));

    uint16_t readMinimumWaitTime = 0;
    Encoding::LittleEndian::Reader reader(rcvData->Start(), rcvData->DataLength());
    EXPECT_EQ(reader.Read16(&readMinimumWaitTime).StatusCode(), CHIP_NO_ERROR);
    EXPECT_EQ(System::Clock::Milliseconds16(readMinimumWaitTime), minimumWaitTime);
}

// Test Suite
