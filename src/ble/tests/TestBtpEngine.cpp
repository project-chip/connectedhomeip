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

// Test sending a 15-byte payload in a single packet with acknowledgment required.
TEST_F(TestBtpEngine, HandleCharacteristicSendOnePacketWithAck)
{
    // Create a new packet buffer with space for a 15-byte payload and set its data length to 15.
    size_t dataLength = 15;
    auto packet0      = System::PacketBufferHandle::New(dataLength);
    packet0->SetDataLength(dataLength);

    // Send the packet with acknowledgment required, checking the transmission state and the packet buffer.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), true));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(dataLength + 5)); // protocol header (5 bytes) + payload (15 bytes)
}

// Test sending a 30-byte payload with acknowledgment required, but providing incorrect ack sequence.
TEST_F(TestBtpEngine, HandleCharacteristicSendTwoPacketWithIncorrectAck)
{
    // Create a new packet buffer with space for a 30-byte payload and set its data length to 30.
    size_t dataLength = 30;
    auto packet0      = System::PacketBufferHandle::New(dataLength);
    packet0->SetDataLength(dataLength);

    // Start sending the payload with acknowledgment required, checking the transmission state and the packet buffer.
    auto data0MaxLength = BtpEngine::sDefaultFragmentSize;
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), true));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(data0MaxLength));

    // Try to continue sending the next fragment, also attempting to send an ack
    // when the sequence is incorrect and check the transmission state.
    EXPECT_FALSE(mBtpEngine.HandleCharacteristicSend(nullptr, true));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);

    // Continue sending without acknowledgment, checking the transmission state and the packet buffer.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(packet0->DataLength(),
              static_cast<size_t>(dataLength - (data0MaxLength - (kTransferProtocolMaxHeaderSize - kTransferProtocolAckSize)) +
                                  kTransferProtocolMidFragmentMaxHeaderSize));
}

// Test sending a 30-byte payload with correct acknowledgment sequence.
TEST_F(TestBtpEngine, HandleCharacteristicSendTwoPacketWithCorrectAck)
{
    // Create a new packet buffer with space for a 30-byte payload and set its data length to 30.
    size_t dataLength = 30;
    auto packet0      = System::PacketBufferHandle::New(dataLength);
    packet0->SetDataLength(dataLength);

    // Initialize the payload data with sequential values for testing.
    auto data0 = packet0->Start();
    ASSERT_NE(data0, nullptr);
    std::iota(data0, data0 + dataLength, 0);

    // Start sending the payload without requiring an acknowledgment for the first fragment.
    auto data0MaxLength = BtpEngine::sDefaultFragmentSize;
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(data0MaxLength));

    // Continue sending second fragment with acknowledgment required.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, true));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(packet0->DataLength(),
              static_cast<size_t>(dataLength - (data0MaxLength - (kTransferProtocolMaxHeaderSize - kTransferProtocolAckSize)) +
                                  kTransferProtocolMidFragmentMaxHeaderSize));
}

// Test that the engine prevents sending a new payload until the previous payload has been acknowledged.
TEST_F(TestBtpEngine, HandleCharacteristicSendRejectsNewSendUntilPreviousAcked)
{
    // Create a new packet buffer with space for a 30-byte payload and set its data length to 30.
    size_t dataLength = 30;
    auto packet0      = System::PacketBufferHandle::New(dataLength);
    packet0->SetDataLength(dataLength);

    // Initialize the payload data with sequential values for testing.
    auto data0 = packet0->Start();
    ASSERT_NE(data0, nullptr);
    std::iota(data0, data0 + dataLength, 0);

    // Start sending the payload without requiring an acknowledgment for the first fragment.
    auto data0MaxLength = BtpEngine::sDefaultFragmentSize;
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(data0MaxLength));

    // Attempt to send another payload while the first one is still in progress.
    auto packet1 = System::PacketBufferHandle::New(dataLength);
    packet1->SetDataLength(dataLength);
    EXPECT_FALSE(mBtpEngine.HandleCharacteristicSend(packet1.Retain(), false));

    // Send the second fragment of the first payload with acknowledgment required.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, true));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(packet0->DataLength(),
              static_cast<size_t>(dataLength - (data0MaxLength - (kTransferProtocolMaxHeaderSize - kTransferProtocolAckSize)) +
                                  kTransferProtocolMidFragmentMaxHeaderSize));
}

// Test sending a payload when there is not enough headroom in the packet buffer.
TEST_F(TestBtpEngine, HandleCharacteristicSendInsufficientHeadroom)
{
    // Create a new packet buffer with maximum size and set its data length to the maximum.
    auto packet0 = PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize, 0);
    packet0->SetDataLength(packet0->MaxDataLength());

    // Attempt to send the packet with acknowledgment required, which should fail due to insufficient headroom.
    EXPECT_FALSE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), true));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Error);
    EXPECT_EQ(packet0->DataLength(), packet0->MaxDataLength());

    // Create a new packet buffer with space for a 15-byte payload and zero headroom, then set its data length to 15.
    size_t dataLength = 15;
    auto packet1      = System::PacketBufferHandle::New(dataLength, 0);
    packet1->SetDataLength(dataLength);

    // Attempt to send another packet with insufficient headroom, which should also fail.
    EXPECT_FALSE(mBtpEngine.HandleCharacteristicSend(packet1.Retain(), true));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Error);
    EXPECT_EQ(packet1->DataLength(), packet1->MaxDataLength());
}

TEST_F(TestBtpEngine, EncodeStandAloneAckOnePacket)
{
    constexpr uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage),
        0x01,
        0x01,
        0x00,
        0xff,
    };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_FALSE(packet0.IsNull());

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);

    EXPECT_TRUE(mBtpEngine.HasUnackedData());

    auto ackPacket = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    EXPECT_FALSE(ackPacket.IsNull());

    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket), CHIP_NO_ERROR);
    EXPECT_EQ(ackPacket->DataLength(), kTransferProtocolStandaloneAckHeaderSize);

    EXPECT_EQ(ackPacket->Start()[0], to_underlying(BtpEngine::HeaderFlags::kFragmentAck));
    EXPECT_EQ(ackPacket->Start()[1], 0x01);
    EXPECT_EQ(ackPacket->Start()[2], 0x00);
}

TEST_F(TestBtpEngine, EncodeStandAloneAckNoUnackedData)
{
    auto ackPacket = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    EXPECT_FALSE(ackPacket.IsNull());

    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket), CHIP_NO_ERROR);
    EXPECT_EQ(ackPacket->DataLength(), kTransferProtocolStandaloneAckHeaderSize);

    EXPECT_EQ(ackPacket->Start()[0], to_underlying(BtpEngine::HeaderFlags::kFragmentAck));
    EXPECT_EQ(ackPacket->Start()[1], 0x00);
    EXPECT_EQ(ackPacket->Start()[2], 0x00);
}

TEST_F(TestBtpEngine, EncodeStandAloneAckInsufficientBuffer)
{
    auto ackPacket0 = System::PacketBufferHandle::New(kTransferProtocolStandaloneAckHeaderSize - 1);
    EXPECT_FALSE(ackPacket0.IsNull());

    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket0), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(ackPacket0->DataLength(), static_cast<size_t>(0));
}

TEST_F(TestBtpEngine, EncodeStandAloneAckMultiFragmentMessage)
{
    constexpr uint8_t packetData0[] = { to_underlying(BtpEngine::HeaderFlags::kStartMessage), 0x01, 0x03, 0x00, 0xfd };
    constexpr uint8_t packetData1[] = { to_underlying(BtpEngine::HeaderFlags::kContinueMessage), 0x02, 0xfe };
    constexpr uint8_t packetData2[] = { to_underlying(BtpEngine::HeaderFlags::kEndMessage), 0x03, 0xff };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(sizeof(packetData0)));

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_InProgress);

    auto packet1 = System::PacketBufferHandle::NewWithData(packetData1, sizeof(packetData1));
    EXPECT_EQ(packet1->DataLength(), static_cast<size_t>(sizeof(packetData1)));

    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet1), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_InProgress);

    auto packet2 = System::PacketBufferHandle::NewWithData(packetData2, sizeof(packetData2));
    EXPECT_EQ(packet2->DataLength(), static_cast<size_t>(sizeof(packetData2)));

    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet2), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);

    auto ackPacket = System::PacketBufferHandle::New(kTransferProtocolStandaloneAckHeaderSize);
    EXPECT_FALSE(ackPacket.IsNull());

    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket), CHIP_NO_ERROR);
    EXPECT_EQ(ackPacket->DataLength(), kTransferProtocolStandaloneAckHeaderSize);

    EXPECT_EQ(ackPacket->Start()[0], to_underlying(BtpEngine::HeaderFlags::kFragmentAck));
    EXPECT_EQ(ackPacket->Start()[1], 0x03); // sequence number
    EXPECT_EQ(ackPacket->Start()[2], 0x00); // fragment count
}

TEST_F(TestBtpEngine, HandleCharacteristicReceivedIncorrectSequence)
{
    uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage),
        0x01, // sequence number increments
        0x01,
        0x00,
        0xff,
    };
    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_FALSE(packet0.IsNull());

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);

    uint8_t packetData1[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage),
        0x02, // sequence number increments
        0x01,
        0x00,
        0xff,
    };
    auto packet1 = System::PacketBufferHandle::NewWithData(packetData1, sizeof(packetData1));
    EXPECT_FALSE(packet1.IsNull());

    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet1), receivedAck, didReceiveAck), BLE_ERROR_REASSEMBLER_INCORRECT_STATE);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Error);
}

TEST_F(TestBtpEngine, HandleCharacteristicReceivedUnexpectedStandaloneAck)
{
    constexpr uint8_t ackPacketData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kFragmentAck),
        0x00,
        0x01,
    };

    auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackPacketData0, sizeof(ackPacketData0));
    EXPECT_FALSE(ackPacket0.IsNull());

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck), BLE_ERROR_INVALID_ACK);
    EXPECT_TRUE(didReceiveAck);
    EXPECT_EQ(receivedAck, 0x00);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Error);
}

TEST_F(TestBtpEngine, HandleAckReceivedIncorrectSequence)
{
    size_t dataLength = 30;
    auto packet0 = System::PacketBufferHandle::New(dataLength);
    packet0->SetDataLength(dataLength);

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);

    constexpr uint8_t ackPacketData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kFragmentAck),
        0x00,
        0x02,
    };

    auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackPacketData0, sizeof(ackPacketData0));
    EXPECT_FALSE(ackPacket0.IsNull());

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck), BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER);
    EXPECT_TRUE(didReceiveAck);
    EXPECT_EQ(receivedAck, 0x00);
    EXPECT_EQ(mBtpEngine.ExpectingAck(), 1);
}

TEST_F(TestBtpEngine, HandleCharacteristicReceivedWithPadding)
{
    constexpr uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage),
        0x01,
        0x01,
        0x00,
        0xff,
    };

    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, 10);
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);
    EXPECT_EQ(mBtpEngine.TakeRxPacket()->DataLength(), static_cast<size_t>(1));
}

TEST_F(TestBtpEngine, IsValidAckOnSequenceWraparound)
{
    const uint8_t invalidAckValue = 100;

    size_t packetLength = mBtpEngine.sDefaultFragmentSize - kTransferProtocolMaxHeaderSize + kTransferProtocolAckSize + 256 * (mBtpEngine.sDefaultFragmentSize - kTransferProtocolMidFragmentMaxHeaderSize + kTransferProtocolAckSize);
    auto packet0 = System::PacketBufferHandle::New(packetLength);
    packet0->SetDataLength(packetLength);

    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));

    int count = 0;
    while (count < 256)
    {
        EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
        count++;
        if (count % 10 == 0 && mBtpEngine.ExpectingAck())
        {
            uint8_t ackData0[] = {
                to_underlying(BtpEngine::HeaderFlags::kFragmentAck),
                static_cast<uint8_t>(count % 256),
                static_cast<uint8_t>((count + 1) % 256),
            };
            auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackData0, sizeof(ackData0));
            SequenceNumber_t receivedAck;
            bool didReceiveAck;
            EXPECT_NE(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
            EXPECT_TRUE(didReceiveAck);
            EXPECT_EQ(receivedAck, count % 256);
        }
    }
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);

    constexpr uint8_t ackPacketData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kFragmentAck),
        invalidAckValue,
        0xFF,
    };

    auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackPacketData0, sizeof(ackPacketData0));
    EXPECT_FALSE(ackPacket0.IsNull());

    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck), BLE_ERROR_INVALID_ACK);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Error);
    EXPECT_TRUE(didReceiveAck);
    EXPECT_EQ(receivedAck, invalidAckValue);
}


} // namespace
