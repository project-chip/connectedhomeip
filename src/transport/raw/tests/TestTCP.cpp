/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      This file implements unit tests for the TcpTransport implementation.
 */

#include "NetworkTestHelpers.h"

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestUtils.h>
#include <system/SystemLayer.h>
#include <transport/TransportMgr.h>
#include <transport/raw/TCP.h>

#include <gtest/gtest.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

using namespace chip;
using namespace chip::Inet;

namespace {

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr uint16_t kPacketSizeBytes           = static_cast<uint16_t>(sizeof(uint16_t));

using TCPImpl = Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>;

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;
constexpr uint32_t kMessageCounter  = 18;

using TestContext = chip::Test::IOContext;

const char PAYLOAD[] = "Hello!";

class MockTransportMgrDelegate : public chip::TransportMgrDelegate
{
public:
    typedef int (*MessageReceivedCallback)(const uint8_t * message, size_t length, int count, void * data);

    MockTransportMgrDelegate(TestContext * inContext) : mContext(inContext), mCallback(nullptr), mCallbackData(nullptr) {}
    ~MockTransportMgrDelegate() override {}

    void SetCallback(MessageReceivedCallback callback = nullptr, void * callback_data = nullptr)
    {
        mCallback     = callback;
        mCallbackData = callback_data;
    }
    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf) override
    {
        PacketHeader packetHeader;

        EXPECT_EQ(packetHeader.DecodeAndConsume(msgBuf), CHIP_NO_ERROR);

        if (mCallback)
        {
            EXPECT_EQ(mCallback(msgBuf->Start(), msgBuf->DataLength(), mReceiveHandlerCallCount, mCallbackData), 0);
        }

        mReceiveHandlerCallCount++;
    }

    void InitializeMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        CHIP_ERROR err = tcp.Init(Transport::TcpListenParameters(mContext->GetTCPEndPointManager()).SetAddressType(addr.Type()));

        // retry a few times in case the port is somehow in use.
        // this is a WORKAROUND for flaky testing if we run tests very fast after each other.
        // in that case, a port could be in a WAIT state.
        //
        // What may be happening:
        //   - We call InitializeMessageTest several times in this unit test
        //   - closing sockets takes a while (FIN-wait or similar)
        //   - trying InitializeMessageTest to take the same port right after may fail
        //
        // The tests may be run with a 0 port (to self select an active port) however I have not
        // validated that this works and we need a followup for it
        //
        // TODO: stop using fixed ports.
        for (int i = 0; (i < 50) && (err != CHIP_NO_ERROR); i++)
        {
            ChipLogProgress(NotSpecified, "RETRYING tcp initialization");
            chip::test_utils::SleepMillis(100);
            err = tcp.Init(Transport::TcpListenParameters(mContext->GetTCPEndPointManager()).SetAddressType(addr.Type()));
        }

        EXPECT_EQ(err, CHIP_NO_ERROR);

        mTransportMgrBase.SetSessionManager(this);
        mTransportMgrBase.Init(&tcp);

        mReceiveHandlerCallCount = 0;
    }

    void SingleMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::NewWithData(PAYLOAD, sizeof(PAYLOAD));
        ASSERT_FALSE(buffer.IsNull());

        PacketHeader header;
        header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageCounter(kMessageCounter);

        SetCallback([](const uint8_t * message, size_t length, int count, void * data) { return memcmp(message, data, length); },
                    const_cast<void *>(static_cast<const void *>(PAYLOAD)));

        CHIP_ERROR err = header.EncodeBeforeData(buffer);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Should be able to send a message to itself by just calling send.
        err = tcp.SendMessage(Transport::PeerAddress::TCP(addr), std::move(buffer));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [this]() { return mReceiveHandlerCallCount != 0; });
        EXPECT_EQ(mReceiveHandlerCallCount, 1);

        SetCallback(nullptr);
    }

    void FinalizeMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        // Disconnect and wait for seeing peer close
        tcp.Disconnect(Transport::PeerAddress::TCP(addr));
        mContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [&tcp]() { return !tcp.HasActiveConnections(); });
    }

    int mReceiveHandlerCallCount = 0;

private:
    TestContext * mContext;
    MessageReceivedCallback mCallback;
    void * mCallbackData;
    TransportMgrBase mTransportMgrBase;
};

/////////////////////////// Init test

class TestTCP : public ::testing::Test
{
protected:
    TestTCP() { inContext = new TestContext(); }
    void SetUp() { ASSERT_EQ(inContext->Init(), CHIP_NO_ERROR); }
    void TearDown() { inContext->Shutdown(); }
    TestContext * inContext;
};

void CheckSimpleInitTest(TestContext * ctx, Inet::IPAddressType type)
{
    TCPImpl tcp;

    CHIP_ERROR err = tcp.Init(Transport::TcpListenParameters(ctx->GetTCPEndPointManager()).SetAddressType(type));

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void CheckMessageTest(TestContext * ctx, const IPAddress & addr)
{
    TCPImpl tcp;

    MockTransportMgrDelegate gMockTransportMgrDelegate(ctx);
    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr);
    gMockTransportMgrDelegate.SingleMessageTest(tcp, addr);
    gMockTransportMgrDelegate.FinalizeMessageTest(tcp, addr);
}

#if INET_CONFIG_ENABLE_IPV4
TEST_F(TestTCP, CheckSimpleInitTest4)
{
    CheckSimpleInitTest(inContext, IPAddressType::kIPv4);
}

TEST_F(TestTCP, CheckMessageTest4)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CheckMessageTest(inContext, addr);
}
#endif

TEST_F(TestTCP, CheckSimpleInitTest6)
{
    CheckSimpleInitTest(inContext, IPAddressType::kIPv6);
}

TEST_F(TestTCP, CheckMessageTest6)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CheckMessageTest(inContext, addr);
}

// Generates a packet buffer or a chain of packet buffers for a single message.
struct TestData
{
    // `sizes[]` is a zero-terminated sequence of packet buffer sizes.
    // If total length supplied is not large enough for at least the PacketHeader and length field,
    // the last buffer will be made larger.
    TestData() : mPayload(nullptr), mTotalLength(0), mMessageLength(0), mMessageOffset(0) {}
    ~TestData() { Free(); }
    bool Init(const uint16_t sizes[]);
    void Free();
    bool IsValid() { return !mHandle.IsNull() && (mPayload != nullptr); }

    chip::System::PacketBufferHandle mHandle;
    uint8_t * mPayload;
    size_t mTotalLength;
    size_t mMessageLength;
    size_t mMessageOffset;
};

bool TestData::Init(const uint16_t sizes[])
{
    Free();

    PacketHeader header;
    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageCounter(kMessageCounter);
    const size_t headerLength = header.EncodeSizeBytes();

    // Determine the total length.
    mTotalLength    = 0;
    int bufferCount = 0;
    for (; sizes[bufferCount] != 0; ++bufferCount)
    {
        mTotalLength += sizes[bufferCount];
    }
    --bufferCount;
    uint16_t additionalLength = 0;
    if (headerLength + kPacketSizeBytes > mTotalLength)
    {
        additionalLength = static_cast<uint16_t>((headerLength + kPacketSizeBytes) - mTotalLength);
        mTotalLength += additionalLength;
    }
    if (mTotalLength > UINT16_MAX)
    {
        return false;
    }
    uint16_t messageLength = static_cast<uint16_t>(mTotalLength - kPacketSizeBytes);

    // Build the test payload.
    uint8_t * payload = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(1, mTotalLength));
    if (payload == nullptr)
    {
        return false;
    }
    chip::Encoding::LittleEndian::Put16(payload, messageLength);
    uint16_t headerSize;
    CHIP_ERROR err = header.Encode(payload + kPacketSizeBytes, messageLength, &headerSize);
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }
    mMessageLength = messageLength - headerSize;
    mMessageOffset = kPacketSizeBytes + headerSize;
    // Fill the rest of the payload with a recognizable pattern.
    for (size_t i = mMessageOffset; i < mTotalLength; ++i)
    {
        payload[i] = static_cast<uint8_t>(i);
    }
    // When we get the message back, the header will have been removed.

    // Allocate the buffer chain.
    System::PacketBufferHandle head = chip::System::PacketBufferHandle::New(sizes[0], 0 /* reserve */);
    for (int i = 1; i <= bufferCount; ++i)
    {
        uint16_t size = sizes[i];
        if (i == bufferCount)
        {
            size = static_cast<uint16_t>(size + additionalLength);
        }
        chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::New(size, 0 /* reserve */);
        if (buffer.IsNull())
        {
            return false;
        }
        head.AddToEnd(std::move(buffer));
    }

    // Write the test payload to the buffer chain.
    System::PacketBufferHandle iterator = head.Retain();
    uint8_t * writePayload              = payload;
    size_t writeLength                  = mTotalLength;
    while (writeLength > 0)
    {
        if (iterator.IsNull())
        {
            return false;
        }
        size_t lAvailableLengthInCurrentBuf = iterator->AvailableDataLength();
        size_t lToWriteToCurrentBuf         = lAvailableLengthInCurrentBuf;
        if (writeLength < lToWriteToCurrentBuf)
        {
            lToWriteToCurrentBuf = writeLength;
        }
        if (lToWriteToCurrentBuf != 0)
        {
            memcpy(iterator->Start(), writePayload, lToWriteToCurrentBuf);
            iterator->SetDataLength(static_cast<uint16_t>(iterator->DataLength() + lToWriteToCurrentBuf), head);
            writePayload += lToWriteToCurrentBuf;
            writeLength -= lToWriteToCurrentBuf;
        }
        iterator.Advance();
    }

    mHandle  = std::move(head);
    mPayload = payload;
    return true;
}

void TestData::Free()
{
    chip::Platform::MemoryFree(mPayload);
    mPayload       = nullptr;
    mHandle        = nullptr;
    mTotalLength   = 0;
    mMessageLength = 0;
    mMessageOffset = 0;
}

int TestDataCallbackCheck(const uint8_t * message, size_t length, int count, void * data)
{
    if (data == nullptr)
    {
        return -1;
    }
    TestData * currentData = static_cast<TestData *>(data) + count;
    if (currentData->mPayload == nullptr)
    {
        return -2;
    }
    if (currentData->mMessageLength != length)
    {
        return -3;
    }
    if (memcmp(currentData->mPayload + currentData->mMessageOffset, message, length) != 0)
    {
        return -4;
    }
    return 0;
}

} // namespace

namespace chip {
namespace Transport {
class TCPTest
{
public:
    static void CheckProcessReceivedBuffer(TestContext * ctx);
};
} // namespace Transport
} // namespace chip
void chip::Transport::TCPTest::CheckProcessReceivedBuffer(TestContext * ctx)
{
    TCPImpl tcp;

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    MockTransportMgrDelegate gMockTransportMgrDelegate(ctx);
    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr);

    // Send a packet to get TCP going, so that we can find a TCPEndPoint to pass to ProcessReceivedBuffer.
    // (The current TCPEndPoint implementation is not effectively mockable.)
    gMockTransportMgrDelegate.SingleMessageTest(tcp, addr);

    Transport::PeerAddress lPeerAddress    = Transport::PeerAddress::TCP(addr);
    TCPBase::ActiveConnectionState * state = tcp.FindActiveConnection(lPeerAddress);
    ASSERT_NE(state, nullptr);
    Inet::TCPEndPoint * lEndPoint = state->mEndPoint;
    ASSERT_NE(lEndPoint, nullptr);

    CHIP_ERROR err = CHIP_NO_ERROR;
    TestData testData[2];
    gMockTransportMgrDelegate.SetCallback(TestDataCallbackCheck, testData);

    // Test a single packet buffer.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint16_t[]){ 111, 0 }));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 1);

    // Test a message in a chain of three packet buffers. The message length is split across buffers.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint16_t[]){ 1, 122, 123, 0 }));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 1);

    // Test two messages in a chain.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint16_t[]){ 131, 0 }));
    EXPECT_TRUE(testData[1].Init((const uint16_t[]){ 132, 0 }));
    testData[0].mHandle->AddToEnd(std::move(testData[1].mHandle));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 2);

    // Test a chain of two messages, each a chain.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint16_t[]){ 141, 142, 0 }));
    EXPECT_TRUE(testData[1].Init((const uint16_t[]){ 143, 144, 0 }));
    testData[0].mHandle->AddToEnd(std::move(testData[1].mHandle));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 2);

    // Test a message that is too large to coalesce into a single packet buffer.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    gMockTransportMgrDelegate.SetCallback(TestDataCallbackCheck, &testData[1]);
    EXPECT_TRUE(testData[0].Init((const uint16_t[]){ 51, System::PacketBuffer::kMaxSizeWithoutReserve, 0 }));
    // Sending only the first buffer of the long chain. This should be enough to trigger the error.
    System::PacketBufferHandle head = testData[0].mHandle.PopHead();
    err                             = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(head));
    EXPECT_EQ(err, CHIP_ERROR_MESSAGE_TOO_LONG);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 0);

    gMockTransportMgrDelegate.FinalizeMessageTest(tcp, addr);
}

TEST_F(TestTCP, CheckProcessReceivedBuffer)
{
    chip::Transport::TCPTest::CheckProcessReceivedBuffer(inContext);
}
