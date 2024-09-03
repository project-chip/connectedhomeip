/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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
#include <numeric>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#define _CHIP_BLE_BLE_H
#include <ble/BleLayer.h>
#include <ble/BtpEngine.h>

using namespace chip;
using namespace chip::Ble;

namespace {

class TestBtpEngine : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp()
    {
        ASSERT_EQ(mBtpEngine.Init(nullptr, false), CHIP_NO_ERROR);
        ChipLogDetail(Test, "### Initial BTP Engine State:");
        mBtpEngine.LogState();
    }

    void TearDown()
    {
        ChipLogDetail(Test, "### Final BTP Engine State:");
        mBtpEngine.LogState();
    }

    Ble::BtpEngine mBtpEngine;
};

TEST_F(TestBtpEngine, HandleCharacteristicReceivedOnePacket)
{
    constexpr uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage),
        0x01,
        0x01,
        0x00,
        0xff, // payload
    };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(5));

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);
}

TEST_F(TestBtpEngine, HandleCharacteristicReceivedTwoPacket)
{
    constexpr uint8_t packetData0[] = { to_underlying(BtpEngine::HeaderFlags::kStartMessage), 0x01, 0x02, 0x00, 0xfe };
    constexpr uint8_t packetData1[] = { to_underlying(BtpEngine::HeaderFlags::kEndMessage), 0x02, 0xff };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(5));

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_InProgress);

    auto packet1 = System::PacketBufferHandle::NewWithData(packetData1, sizeof(packetData1));
    EXPECT_EQ(packet1->DataLength(), static_cast<size_t>(3));

    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet1), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);
}

TEST_F(TestBtpEngine, HandleCharacteristicReceivedThreePacket)
{
    constexpr uint8_t packetData0[] = { to_underlying(BtpEngine::HeaderFlags::kStartMessage), 0x01, 0x03, 0x00, 0xfd };
    constexpr uint8_t packetData1[] = { to_underlying(BtpEngine::HeaderFlags::kContinueMessage), 0x02, 0xfe };
    constexpr uint8_t packetData2[] = { to_underlying(BtpEngine::HeaderFlags::kEndMessage), 0x03, 0xff };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(5));

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_InProgress);

    auto packet1 = System::PacketBufferHandle::NewWithData(packetData1, sizeof(packetData1));
    EXPECT_EQ(packet1->DataLength(), static_cast<size_t>(3));

    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet1), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_InProgress);

    auto packet2 = System::PacketBufferHandle::NewWithData(packetData2, sizeof(packetData2));
    EXPECT_EQ(packet2->DataLength(), static_cast<size_t>(3));

    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet2), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);
}

TEST_F(TestBtpEngine, HandleCharacteristicSendOnePacket)
{
    auto packet0 = System::PacketBufferHandle::New(10);
    packet0->SetDataLength(1);

    auto data0 = packet0->Start();
    ASSERT_NE(data0, nullptr);
    std::iota(data0, data0 + 1, 0);

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(5));
}

TEST_F(TestBtpEngine, HandleCharacteristicSendTwoPacket)
{
    auto packet0 = System::PacketBufferHandle::New(30);
    packet0->SetDataLength(30);

    auto data0 = packet0->Start();
    ASSERT_NE(data0, nullptr);
    std::iota(data0, data0 + 30, 0);

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(20));

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(16));
}

// Send 40-byte payload.
// Packet0: 4 byte header + 16 byte payload
// Packet1: 2 byte header + 18 byte payload
// Packet2: 2 byte header + 6 byte payload
TEST_F(TestBtpEngine, HandleCharacteristicSendThreePacket)
{
    auto packet0 = System::PacketBufferHandle::New(40);
    packet0->SetDataLength(40);

    auto data0 = packet0->Start();
    ASSERT_NE(data0, nullptr);
    std::iota(data0, data0 + 40, 0);

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(20));

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(20));

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(8));
}

} // namespace
