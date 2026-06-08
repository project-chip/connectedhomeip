/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

#include <wifipaf/WiFiPAFTP.h>

namespace chip {
namespace WiFiPAF {

class TestWiFiPAFTP : public WiFiPAFTP, public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::SystemLayer().Init(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::SystemLayer().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    void SetUp() override { ASSERT_EQ(Init(nullptr, false), CHIP_NO_ERROR); }

    void TearDown() override {}
};

TEST_F(TestWiFiPAFTP, CheckLogState)
{
    LogState();
    TakeTxPacket();
    TakeRxPacket();

    PacketBufferHandle AckToSend = System::PacketBufferHandle::New(kTransferProtocolStandaloneAckHeaderSize);
    EXPECT_EQ(EncodeStandAloneAck(AckToSend), CHIP_NO_ERROR);

    EXPECT_NE(GetAndIncrementNextTxSeqNum(), 0);
    EXPECT_NE(GetAndRecordRxAckSeqNum(), 0);
    HasUnackedData();

    uint16_t FragSize = 300;
    SetTxFragmentSize(FragSize);
    SetRxFragmentSize(FragSize);
    EXPECT_EQ(GetTxFragmentSize(), FragSize);
    EXPECT_EQ(GetRxFragmentSize(), FragSize);

    EXPECT_NE(GetRxNextSeqNum(), 0);
    EXPECT_NE(GetLastReceivedSequenceNumber(), 0);
    EXPECT_NE(GetNewestUnackedSentSequenceNumber(), 0);
    EXPECT_EQ(ExpectingAck(), true);
    ClearRxPacket();
    ClearTxPacket();
}

TEST_F(TestWiFiPAFTP, CheckSendSingle)
{
    constexpr size_t kShortPacketLength = 100;
    auto buf                            = System::PacketBufferHandle::New(kShortPacketLength);
    buf->SetDataLength(kShortPacketLength);
    memset(buf->Start(), 0, buf->DataLength());
    mRxNextSeqNum = 1;
    EXPECT_TRUE(HandleCharacteristicSend(buf.Retain(), true));
    EXPECT_EQ(TxState(), kState_Complete);
    TakeTxPacket();
    EXPECT_EQ(TxState(), kState_Idle);
}

TEST_F(TestWiFiPAFTP, CheckSendMultiple)
{
    constexpr size_t kLongPacketLength = 500;
    auto buf                           = System::PacketBufferHandle::New(kLongPacketLength);
    ASSERT_FALSE(buf.IsNull());
    buf->SetDataLength(kLongPacketLength);
    memset(buf->Start(), 0, buf->DataLength());
    EXPECT_TRUE(HandleCharacteristicSend(buf.Retain(), false));

    EXPECT_EQ(TxState(), kState_InProgress);
    EXPECT_TRUE(HandleCharacteristicSend(nullptr, false));
    EXPECT_EQ(TxState(), kState_Complete);
    TakeTxPacket();
    EXPECT_EQ(TxState(), kState_Idle);
}

TEST_F(TestWiFiPAFTP, CheckRecv)
{
    SequenceNumber_t receivedAck = 0;
    bool didReceiveAck           = false;

    // Receive a packet
    constexpr uint8_t packetData0[] = {
        to_underlying(HeaderFlags::kStartMessage) | to_underlying(HeaderFlags::kEndMessage),
        0x01,
        0x01,
        0x00,
        0xff, // payload
    };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(5));

    receivedAck   = 0;
    didReceiveAck = false;
    EXPECT_EQ(HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(RxState(), kState_Complete);
    TakeRxPacket();
    EXPECT_EQ(RxState(), kState_Idle);
}

TEST_F(TestWiFiPAFTP, CheckAckRecv)
{
    constexpr uint8_t packetData0[] = {
        to_underlying(HeaderFlags::kStartMessage) | to_underlying(HeaderFlags::kEndMessage) |
            to_underlying(HeaderFlags::kFragmentAck),
        0x01,
        0x01,
        0x00,
        0x00, // payload
    };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));

    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(5));

    SequenceNumber_t receivedAck = 0;
    bool didReceiveAck           = true;
    mExpectingAck                = true;
    EXPECT_EQ(HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(RxState(), kState_Idle);
}

TEST_F(TestWiFiPAFTP, CheckErrorRecv)
{
    SequenceNumber_t receivedAck = 0;
    bool didReceiveAck           = false;

    // Null-buffer
    auto nullbuf = System::PacketBufferHandle::New(0);
    EXPECT_NE(HandleCharacteristicReceived(std::move(nullbuf), receivedAck, didReceiveAck), CHIP_NO_ERROR);

    // Reveived Invalid packet
    constexpr uint8_t packetData_invalid_ack[] = {
        to_underlying(HeaderFlags::kFragmentAck) | to_underlying(HeaderFlags::kEndMessage),
        0xf0,
        0x01,
        0x00,
        0xff, // payload
    };
    auto packet_invalid_ack = System::PacketBufferHandle::NewWithData(packetData_invalid_ack, sizeof(packetData_invalid_ack));
    EXPECT_EQ(HandleCharacteristicReceived(std::move(packet_invalid_ack), receivedAck, didReceiveAck), CHIP_NO_ERROR);
}
}; // namespace WiFiPAF
}; // namespace chip
