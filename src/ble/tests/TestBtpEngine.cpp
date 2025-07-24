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

// Test that the BTP engine correctly encodes a standalone ACK packet when there is unacked data.
TEST_F(TestBtpEngine, EncodeStandAloneAckOnePacket)
{
    // Create a packet for receiving a single message.
    constexpr uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage), // Header flags
        0x01, // Sequence number
        0x01, // Payload length, Least Significant Byte
        0x00, // Payload length, Most Significant Byte
        0xff, // Payload
    };
    // Create a packet buffer with the data for the message and check that it is created successfully.
    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_FALSE(packet0.IsNull());

    // Handle the received packet and check the state of the BTP engine.
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);
    EXPECT_TRUE(mBtpEngine.HasUnackedData());

    // Create a new packet buffer for the standalone ACK and check that it is created successfully.
    auto ackPacket = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    EXPECT_FALSE(ackPacket.IsNull());

    // Encode the standalone ACK and check that it is successful and the packet length is correct.
    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket), CHIP_NO_ERROR);
    EXPECT_EQ(ackPacket->DataLength(), kTransferProtocolStandaloneAckHeaderSize);

    // Check that the ACK packet has the correct header flags and sequence number.
    EXPECT_EQ(ackPacket->Start()[0], to_underlying(BtpEngine::HeaderFlags::kFragmentAck));
    EXPECT_EQ(ackPacket->Start()[1], 0x01); // Ack number for the received packet
    EXPECT_EQ(ackPacket->Start()[2], 0x00); // Sequence number of the Ack packet itself
}

// Test that the BTP engine correctly encodes a standalone ACK packet when there is no unacked data.
TEST_F(TestBtpEngine, EncodeStandAloneAckNoUnackedData)
{
    // Create a new packet buffer for the standalone ACK and check that it is created successfully.
    auto ackPacket = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    EXPECT_FALSE(ackPacket.IsNull());

    // Encode the standalone ACK and check that it is successful and the packet length is correct.
    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket), CHIP_NO_ERROR);
    EXPECT_EQ(ackPacket->DataLength(), kTransferProtocolStandaloneAckHeaderSize);

    // Check that the ACK packet has the correct header flags and sequence number.
    EXPECT_EQ(ackPacket->Start()[0], to_underlying(BtpEngine::HeaderFlags::kFragmentAck));
    EXPECT_EQ(ackPacket->Start()[1], 0x00); // Ack number (no previous packets to acknowledge)
    EXPECT_EQ(ackPacket->Start()[2], 0x00); // Sequence number of the Ack packet itself
}

// Test EncodeStandAloneAck when the packet buffer is too small to hold the header.
TEST_F(TestBtpEngine, EncodeStandAloneAckInsufficientBuffer)
{
    // Create a new packet buffer with size smaller than the header size.
    auto ackPacket0 = System::PacketBufferHandle::New(kTransferProtocolStandaloneAckHeaderSize - 1);
    EXPECT_FALSE(ackPacket0.IsNull());

    // Attempt to encode the standalone ACK and check that it fails.
    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket0), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(ackPacket0->DataLength(), static_cast<size_t>(0));
}

// Test encoding a standalone ACK packet after receiving a complete multi-fragment message.
TEST_F(TestBtpEngine, EncodeStandAloneAckMultiFragmentMessage)
{
    // Create packet buffers for a multi-fragment message.
    constexpr uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage), // Header flags
        0x01,                                                 // Sequence number
        0x03,                                                 // Payload length, Least Significant Byte
        0x00,                                                 // Payload length, Most Significant Byte
        0xfd,                                                 // Payload
    };
    constexpr uint8_t packetData1[] = {
        to_underlying(BtpEngine::HeaderFlags::kContinueMessage), // Header flags
        0x02,                                                    // Sequence number
        0xfe,                                                    // Payload
    };
    constexpr uint8_t packetData2[] = {
        to_underlying(BtpEngine::HeaderFlags::kEndMessage), // Header flags
        0x03,                                               // Sequence number
        0xff,                                               // Payload
    };

    // Create the first packet and check its data length.
    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_EQ(packet0->DataLength(), static_cast<size_t>(sizeof(packetData0)));

    // Receive the first packet and check the state of the BTP engine.
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_InProgress);

    // Create and receive the second packet, check the state of the BTP engine.
    auto packet1 = System::PacketBufferHandle::NewWithData(packetData1, sizeof(packetData1));
    EXPECT_EQ(packet1->DataLength(), static_cast<size_t>(sizeof(packetData1)));
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet1), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_InProgress);

    // Create and receive the third packet, check the state of the BTP engine.
    auto packet2 = System::PacketBufferHandle::NewWithData(packetData2, sizeof(packetData2));
    EXPECT_EQ(packet2->DataLength(), static_cast<size_t>(sizeof(packetData2)));
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet2), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);

    // Create a new packet buffer for the standalone ACK and check that it is created successfully.
    auto ackPacket = System::PacketBufferHandle::New(kTransferProtocolStandaloneAckHeaderSize);
    EXPECT_FALSE(ackPacket.IsNull());

    // Encode the standalone ACK and check that it is successful and the packet length is correct.
    EXPECT_EQ(mBtpEngine.EncodeStandAloneAck(ackPacket), CHIP_NO_ERROR);
    EXPECT_EQ(ackPacket->DataLength(), kTransferProtocolStandaloneAckHeaderSize);

    // Check that the ACK packet has the correct header flags and sequence number.
    EXPECT_EQ(ackPacket->Start()[0], to_underlying(BtpEngine::HeaderFlags::kFragmentAck));
    EXPECT_EQ(ackPacket->Start()[1], 0x03); // Ack number for the last received packet
    EXPECT_EQ(ackPacket->Start()[2], 0x00); // Sequence number of the Ack packet itself
}

// Test handling a characteristic received with an incorrect sequence number.
TEST_F(TestBtpEngine, HandleCharacteristicReceivedIncorrectSequence)
{
    // Create a packet buffer with a single message and check that it is created successfully.
    uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage), // Header flags
        0x01, // Sequence number
        0x01, // Payload length, Least Significant Byte
        0x00, // Payload length, Most Significant Byte
        0xff, // Payload
    };
    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));
    EXPECT_FALSE(packet0.IsNull());

    // Handle the received packet and check the state of the BTP engine.
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);

    // Create a second packet to test handling when the previous message hasn't been acknowledged.
    uint8_t packetData1[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage), // Header flags
        0x02, // Sequence number
        0x01, // Payload length, Least Significant Byte
        0x00, // Payload length, Most Significant Byte
        0xff, // Payload
    };
    auto packet1 = System::PacketBufferHandle::NewWithData(packetData1, sizeof(packetData1));
    EXPECT_FALSE(packet1.IsNull());

    // Handle the second packet - this should fail due to engine state (unacked previous message), not sequence number.
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet1), receivedAck, didReceiveAck),
              BLE_ERROR_REASSEMBLER_INCORRECT_STATE);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Error);
}

// Test handling a standalone ACK packet that is received unexpectedly.
TEST_F(TestBtpEngine, HandleCharacteristicReceivedUnexpectedStandaloneAck)
{
    // Create an ACK packet for a packet that has not been sent yet and check that it is created successfully.
    constexpr uint8_t ackPacketData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kFragmentAck), // Header flags for ACK
        0x00,                                                // Ack number for the received packet
        0x01,                                                // Sequence number of the Ack packet itself
    };
    auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackPacketData0, sizeof(ackPacketData0));
    EXPECT_FALSE(ackPacket0.IsNull());

    // Handle the received ACK packet and check state of the BTP engine.
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck), BLE_ERROR_INVALID_ACK);
    EXPECT_TRUE(didReceiveAck);
    EXPECT_EQ(receivedAck, 0x00);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Error);
}

// Test handling acknowledgment received with an incorrect sequence number.
TEST_F(TestBtpEngine, HandleAckReceivedIncorrectSequence)
{
    // Create a packet buffer with a 30-byte payload and set its data length to 30.
    size_t dataLength = 30;
    auto packet0      = System::PacketBufferHandle::New(dataLength);
    packet0->SetDataLength(dataLength);

    // Start sending the packet, checking the transmission state.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_InProgress);

    // Continue sending the next fragment, checking the transmission state.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);

    // Create an ACK packet with an incorrect ACK sequence number and check that it is created successfully.
    constexpr uint8_t ackPacketData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kFragmentAck), // Header flags for ACK
        0x00,                                                // Ack number (correct)
        0x02, // Sequence number of the ACK packet itself (incorrect - should be 0x01)
    };
    auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackPacketData0, sizeof(ackPacketData0));
    EXPECT_FALSE(ackPacket0.IsNull());

    // Handle the received ACK packet with an incorrect ACK sequence number and check the error code and state of the BTP engine.
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck),
              BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER);
    EXPECT_TRUE(didReceiveAck);
    EXPECT_EQ(receivedAck, 0x00);
    EXPECT_EQ(mBtpEngine.ExpectingAck(), 1);
}

// Test handling a packet that contains more data than the declared payload length (simulating BLE packet padding).
TEST_F(TestBtpEngine, HandleCharacteristicReceivedWithPadding)
{
    // Create a packet buffer with a single message and padding bytes.
    // The payload length in the header is 1 byte, but the actual packet has padding.
    constexpr uint8_t packetData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kStartMessage) | to_underlying(BtpEngine::HeaderFlags::kEndMessage), // Header flags
        0x01, // Sequence number
        0x01, // Payload length, Least Significant Byte (only 1 byte of actual payload)
        0x00, // Payload length, Most Significant Byte
        0xff, // Payload
        0x00, // Padding bytes (should be ignored by BTP engine)
        0x00,
        0x00,
        0x00,
        0x00,
    };

    // Create a packet with the full data including padding.
    auto packet0 = System::PacketBufferHandle::NewWithData(packetData0, sizeof(packetData0));

    // Handle the received packet and check the state of the BTP engine.
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(packet0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Complete);

    // Check that the received packet has been processed correctly, ignoring the padding.
    EXPECT_EQ(mBtpEngine.TakeRxPacket()->DataLength(), static_cast<size_t>(1));
}

// Test that the BTP engine correctly handles an ACK packet with a sequence wraparound.
TEST_F(TestBtpEngine, IsValidAckOnSequenceWraparound)
{
    const uint8_t invalidAckValue = 95;

    // Create a packet buffer with a large payload that will result in 257 fragments (to test sequence number wraparound).
    size_t packetLength = mBtpEngine.sDefaultFragmentSize - kTransferProtocolMaxHeaderSize + kTransferProtocolAckSize +
        256 * (mBtpEngine.sDefaultFragmentSize - kTransferProtocolMidFragmentMaxHeaderSize + kTransferProtocolAckSize);
    auto packet0 = System::PacketBufferHandle::New(packetLength);
    packet0->SetDataLength(packetLength);

    // Send the first packet to start transmission.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));

    int count = 0;
    while (count < 256)
    {
        // Continue sending fragments until we reach the expected number.
        EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(nullptr, false));
        count++;

        // Every 10 packets, simulate receiving an ACK to test ACK validation during transmission.
        if (count % 10 == 0)
        {
            // Create an ACK packet with proper sequence number that BTP engine expects.
            uint8_t ackData0[] = {
                to_underlying(BtpEngine::HeaderFlags::kFragmentAck), // Header flags for ACK
                static_cast<uint8_t>(count % 256),                   // Acknowledgment number for the received packet
                static_cast<uint8_t>(count / 10),                    // Sequence number of the ACK packet itself
            };
            auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackData0, sizeof(ackData0));

            // Handle the ACK packet - this should succeed in normal flow.
            SequenceNumber_t receivedAck;
            bool didReceiveAck;
            EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck), CHIP_NO_ERROR);
            EXPECT_TRUE(didReceiveAck);
            EXPECT_EQ(receivedAck, count % 256);
        }
    }
    // After sending all fragments, transmission should be complete.
    EXPECT_EQ(mBtpEngine.TxState(), BtpEngine::kState_Complete);

    // Create an ACK packet with an invalid ACK number and check that it is created successfully.
    uint8_t ackPacketData0[] = {
        to_underlying(BtpEngine::HeaderFlags::kFragmentAck),
        invalidAckValue,                      // Invalid ACK number
        static_cast<uint8_t>(count / 10 + 1), // Sequence number of the ACK packet itself
    };
    auto ackPacket0 = System::PacketBufferHandle::NewWithData(ackPacketData0, sizeof(ackPacketData0));
    EXPECT_FALSE(ackPacket0.IsNull());

    // Handle the received ACK packet with an invalid ACK number and check the error code and state of the BTP engine.
    SequenceNumber_t receivedAck;
    bool didReceiveAck;
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket0), receivedAck, didReceiveAck), BLE_ERROR_INVALID_ACK);
    EXPECT_EQ(mBtpEngine.RxState(), BtpEngine::kState_Error);
    EXPECT_TRUE(didReceiveAck);
    EXPECT_EQ(receivedAck, invalidAckValue);
}

// Test that the initial sequence numbers are set correctly when the BtpEngine is initialized.
TEST_F(TestBtpEngine, InitialSequenceNumbers)
{
    // inline function completely omitted from the LCOV report, no function symbols are generated.
    EXPECT_EQ(mBtpEngine.GetLastReceivedSequenceNumber(), 0);
    // inline function appears in the LCOV report as uncovered (red), even though is actually covered/executed in the test cases,
    // function symbols are generated.
    EXPECT_EQ(mBtpEngine.GetNewestUnackedSentSequenceNumber(), 0);
}

// Test that sending a packet updates the unacknowledged sequence number correctly,
// and that receiving an acknowledgment updates the state as expected.
TEST_F(TestBtpEngine, NewestUnackedSentSequenceNumberSend)
{
    // Create a 1-byte packet and fill it with data.
    auto packet0 = System::PacketBufferHandle::New(10);
    ASSERT_FALSE(packet0.IsNull());
    packet0->SetDataLength(1);
    auto * data0 = packet0->Start();
    ASSERT_NE(data0, nullptr);
    data0[0] = 0;

    // Send the packet and check that the newest unacked sent sequence number is 0.
    EXPECT_TRUE(mBtpEngine.HandleCharacteristicSend(packet0.Retain(), false));
    EXPECT_EQ(mBtpEngine.GetNewestUnackedSentSequenceNumber(), 0);

    // Confirm that there is unacknowledged data.
    EXPECT_TRUE(mBtpEngine.ExpectingAck());

    // Prepare an acknowledgment packet for sequence number 0.
    uint8_t ackData[] = {
        to_underlying(BtpEngine::HeaderFlags::kFragmentAck),
        0x00,
        0x01,
    };

    auto ackPacket = System::PacketBufferHandle::NewWithData(ackData, sizeof(ackData));

    SequenceNumber_t seqNum;
    bool didRecieveAck;

    // Handle the acknowledgment and verify the result.
    EXPECT_EQ(mBtpEngine.HandleCharacteristicReceived(std::move(ackPacket), seqNum, didRecieveAck), CHIP_NO_ERROR);
    EXPECT_TRUE(didRecieveAck);
    EXPECT_EQ(seqNum, 0);

    // After acknowledgment, we are no longer expecting an ack
    EXPECT_FALSE(mBtpEngine.ExpectingAck());
    // After acknowledgment, the newest unacked sent sequence number should still be 0 (no new sends).
    EXPECT_EQ(mBtpEngine.GetNewestUnackedSentSequenceNumber(), 0);
}

} // namespace
