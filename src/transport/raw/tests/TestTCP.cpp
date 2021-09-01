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
#include <lib/support/UnitTestRegistration.h>
#include <system/SystemLayer.h>
#include <system/SystemObject.h>
#include <transport/TransportMgr.h>
#include <transport/raw/TCP.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

using namespace chip;
using namespace chip::Inet;

static int Initialize(void * aContext);
static int Finalize(void * aContext);

namespace chip {
namespace Transport {
class TCPTest
{
public:
    static void CheckProcessReceivedBuffer(nlTestSuite * inSuite, void * inContext);
};
} // namespace Transport
} // namespace chip

namespace {

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr uint16_t kPacketSizeBytes           = static_cast<uint16_t>(sizeof(uint16_t));

using TCPImpl = Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>;

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;
constexpr uint32_t kMessageId       = 18;

using TestContext = chip::Test::IOContext;
TestContext sContext;

const char PAYLOAD[] = "Hello!";

class MockTransportMgrDelegate : public chip::TransportMgrDelegate
{
public:
    typedef int (*MessageReceivedCallback)(const uint8_t * message, size_t length, int count, void * data);

    MockTransportMgrDelegate(nlTestSuite * inSuite, TestContext & inContext) :
        mSuite(inSuite), mContext(inContext), mCallback(nullptr), mCallbackData(nullptr)
    {}
    ~MockTransportMgrDelegate() override {}

    void SetCallback(MessageReceivedCallback callback = nullptr, void * callback_data = nullptr)
    {
        mCallback     = callback;
        mCallbackData = callback_data;
    }
    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf) override
    {
        PacketHeader packetHeader;

        CHIP_ERROR error = packetHeader.DecodeAndConsume(msgBuf);
        NL_TEST_ASSERT(mSuite, error == CHIP_NO_ERROR);

        if (mCallback)
        {
            int err = mCallback(msgBuf->Start(), msgBuf->DataLength(), mReceiveHandlerCallCount, mCallbackData);
            NL_TEST_ASSERT(mSuite, err == 0);
        }

        mReceiveHandlerCallCount++;
    }

    void InitializeMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        CHIP_ERROR err = tcp.Init(Transport::TcpListenParameters(&mContext.GetInetLayer()).SetAddressType(addr.Type()));
        NL_TEST_ASSERT(mSuite, err == CHIP_NO_ERROR);

        mTransportMgrBase.SetSecureSessionMgr(this);
        mTransportMgrBase.Init(&tcp);

        mReceiveHandlerCallCount = 0;
    }

    void SingleMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::NewWithData(PAYLOAD, sizeof(PAYLOAD));
        NL_TEST_ASSERT(mSuite, !buffer.IsNull());

        PacketHeader header;
        header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageId(kMessageId);

        SetCallback([](const uint8_t * message, size_t length, int count, void * data) { return memcmp(message, data, length); },
                    const_cast<void *>(static_cast<const void *>(PAYLOAD)));

        CHIP_ERROR err = header.EncodeBeforeData(buffer);
        NL_TEST_ASSERT(mSuite, err == CHIP_NO_ERROR);

        // Should be able to send a message to itself by just calling send.
        err = tcp.SendMessage(Transport::PeerAddress::TCP(addr), std::move(buffer));
        if (err == System::MapErrorPOSIX(EADDRNOTAVAIL))
        {
            // TODO(#2698): the underlying system does not support IPV6. This early return
            // should be removed and error should be made fatal.
            printf("%s:%u: System does NOT support IPV6.\n", __FILE__, __LINE__);
            return;
        }

        NL_TEST_ASSERT(mSuite, err == CHIP_NO_ERROR);

        mContext.DriveIOUntil(5000 /* ms */, [this]() { return mReceiveHandlerCallCount != 0; });
        NL_TEST_ASSERT(mSuite, mReceiveHandlerCallCount == 1);

        SetCallback(nullptr);
    }

    void FinalizeMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        // Disconnect and wait for seeing peer close
        tcp.Disconnect(Transport::PeerAddress::TCP(addr));
        mContext.DriveIOUntil(5000 /* ms */, [&tcp]() { return !tcp.HasActiveConnections(); });
    }

    int mReceiveHandlerCallCount = 0;

private:
    nlTestSuite * mSuite;
    TestContext & mContext;
    MessageReceivedCallback mCallback;
    void * mCallbackData;
    TransportMgrBase mTransportMgrBase;
};

/////////////////////////// Init test

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext, Inet::IPAddressType type)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TCPImpl tcp;

    CHIP_ERROR err = tcp.Init(Transport::TcpListenParameters(&ctx.GetInetLayer()).SetAddressType(type));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

#if INET_CONFIG_ENABLE_IPV4
void CheckSimpleInitTest4(nlTestSuite * inSuite, void * inContext)
{
    CheckSimpleInitTest(inSuite, inContext, kIPAddressType_IPv4);
}
#endif

void CheckSimpleInitTest6(nlTestSuite * inSuite, void * inContext)
{
    CheckSimpleInitTest(inSuite, inContext, kIPAddressType_IPv6);
}

/////////////////////////// Messaging test

void CheckMessageTest(nlTestSuite * inSuite, void * inContext, const IPAddress & addr)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TCPImpl tcp;

    MockTransportMgrDelegate gMockTransportMgrDelegate(inSuite, ctx);
    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr);
    gMockTransportMgrDelegate.SingleMessageTest(tcp, addr);
    gMockTransportMgrDelegate.FinalizeMessageTest(tcp, addr);
}

#if INET_CONFIG_ENABLE_IPV4
void CheckMessageTest4(nlTestSuite * inSuite, void * inContext)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CheckMessageTest(inSuite, inContext, addr);
}
#endif // INET_CONFIG_ENABLE_IPV4

void CheckMessageTest6(nlTestSuite * inSuite, void * inContext)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CheckMessageTest(inSuite, inContext, addr);
}

// Generates a packet buffer or a chain of packet buffers for a single message.
struct TestData
{
    // `sizes[]` is a zero-terminated sequence of packet buffer sizes.
    // If total length supplied is not large enough for at least the PacketHeader and length field,
    // the last buffer will be made larger.
    TestData() : mHandle(), mPayload(nullptr), mTotalLength(0), mMessageLength(0), mMessageOffset(0) {}
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
    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageId(kMessageId);
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

void chip::Transport::TCPTest::CheckProcessReceivedBuffer(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TCPImpl tcp;

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    MockTransportMgrDelegate gMockTransportMgrDelegate(inSuite, ctx);
    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr);

    // Send a packet to get TCP going, so that we can find a TCPEndPoint to pass to ProcessReceivedBuffer.
    // (The current TCPEndPoint implementation is not effectively mockable.)
    gMockTransportMgrDelegate.SingleMessageTest(tcp, addr);

    Transport::PeerAddress lPeerAddress    = Transport::PeerAddress::TCP(addr);
    TCPBase::ActiveConnectionState * state = tcp.FindActiveConnection(lPeerAddress);
    NL_TEST_ASSERT(inSuite, state != nullptr);
    Inet::TCPEndPoint * lEndPoint = state->mEndPoint;
    NL_TEST_ASSERT(inSuite, lEndPoint != nullptr);

    CHIP_ERROR err = CHIP_NO_ERROR;
    TestData testData[2];
    gMockTransportMgrDelegate.SetCallback(TestDataCallbackCheck, testData);

    // Test a single packet buffer.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    NL_TEST_ASSERT(inSuite, testData[0].Init((const uint16_t[]){ 111, 0 }));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 1);

    // Test a message in a chain of three packet buffers. The message length is split accross buffers.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    NL_TEST_ASSERT(inSuite, testData[0].Init((const uint16_t[]){ 1, 122, 123, 0 }));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 1);

    // Test two messages in a chain.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    NL_TEST_ASSERT(inSuite, testData[0].Init((const uint16_t[]){ 131, 0 }));
    NL_TEST_ASSERT(inSuite, testData[1].Init((const uint16_t[]){ 132, 0 }));
    testData[0].mHandle->AddToEnd(std::move(testData[1].mHandle));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 2);

    // Test a chain of two messages, each a chain.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    NL_TEST_ASSERT(inSuite, testData[0].Init((const uint16_t[]){ 141, 142, 0 }));
    NL_TEST_ASSERT(inSuite, testData[1].Init((const uint16_t[]){ 143, 144, 0 }));
    testData[0].mHandle->AddToEnd(std::move(testData[1].mHandle));
    err = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 2);

    // Test a message that is too large to coalesce into a single packet buffer.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    gMockTransportMgrDelegate.SetCallback(TestDataCallbackCheck, &testData[1]);
    NL_TEST_ASSERT(inSuite, testData[0].Init((const uint16_t[]){ 51, System::PacketBuffer::kMaxSizeWithoutReserve, 0 }));
    // Sending only the first buffer of the long chain. This should be enough to trigger the error.
    System::PacketBufferHandle head = testData[0].mHandle.PopHead();
    err                             = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(head));
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_MESSAGE_TOO_LONG);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 0);

    gMockTransportMgrDelegate.FinalizeMessageTest(tcp, addr);
}

// Test Suite
/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("Simple Init Test IPV4",        CheckSimpleInitTest4),
    NL_TEST_DEF("Message Self Test IPV4",       CheckMessageTest4),
#endif

    NL_TEST_DEF("Simple Init Test IPV6",        CheckSimpleInitTest6),
    NL_TEST_DEF("Message Self Test IPV6",       CheckMessageTest6),
    NL_TEST_DEF("ProcessReceivedBuffer Test",   chip::Transport::TCPTest::CheckProcessReceivedBuffer),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-Tcp",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

/**
 *  Initialize the test suite.
 */
static int Initialize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init(&sSuite);
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
static int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

int TestTCP()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestTCP);
