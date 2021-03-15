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

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemLayer.h>
#include <system/SystemObject.h>
#include <transport/TransportMgr.h>
#include <transport/raw/TCP.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>
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
    typedef int (*MessageReceivedCallback)(const uint8_t * message, size_t length, void * data);

    MockTransportMgrDelegate(nlTestSuite * inSuite, TestContext & inContext, MessageReceivedCallback callback = nullptr,
                             void * callback_data = nullptr) :
        mSuite(inSuite),
        mContext(inContext), mCallback(callback), mCallbackData(callback_data)
    {}
    ~MockTransportMgrDelegate() override {}

    void OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                           System::PacketBufferHandle msgBuf) override
    {
        NL_TEST_ASSERT(mSuite, header.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
        NL_TEST_ASSERT(mSuite, header.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
        NL_TEST_ASSERT(mSuite, header.GetMessageId() == kMessageId);

        if (mCallback)
        {
            int err = mCallback(msgBuf->Start(), msgBuf->DataLength(), mCallbackData);
            NL_TEST_ASSERT(mSuite, err == 0);
        }

        mReceiveHandlerCallCount++;
    }

    void InitializeMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        CHIP_ERROR err = tcp.Init(Transport::TcpListenParameters(&mContext.GetInetLayer()).SetAddressType(addr.Type()));
        NL_TEST_ASSERT(mSuite, err == CHIP_NO_ERROR);

        mTransportMgrBase.SetSecureSessionMgr(this);
        mTransportMgrBase.SetRendezvousSession(this);
        mTransportMgrBase.Init(&tcp);

        mReceiveHandlerCallCount = 0;
    }

    void SingleMessageTest(TCPImpl & tcp, const IPAddress & addr)
    {
        chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::NewWithData(PAYLOAD, sizeof(PAYLOAD));
        NL_TEST_ASSERT(mSuite, !buffer.IsNull());

        PacketHeader header;
        header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageId(kMessageId);

        // Should be able to send a message to itself by just calling send.
        CHIP_ERROR err = tcp.SendMessage(header, Transport::PeerAddress::TCP(addr), std::move(buffer));
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

    MockTransportMgrDelegate gMockTransportMgrDelegate(
        inSuite, ctx, [](const uint8_t * message, size_t length, void * data) { return memcmp(message, data, length); },
        const_cast<void *>(static_cast<const void *>(PAYLOAD)));

    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr);
    gMockTransportMgrDelegate.SingleMessageTest(tcp, addr);
    gMockTransportMgrDelegate.FinalizeMessageTest(tcp, addr);
}

void CheckMessageTest4(nlTestSuite * inSuite, void * inContext)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CheckMessageTest(inSuite, inContext, addr);
}

void CheckMessageTest6(nlTestSuite * inSuite, void * inContext)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CheckMessageTest(inSuite, inContext, addr);
}

} // namespace

// Generates a packet buffer or a chain of packet buffers for a single message.
// `sizes[]` is a zero-terminated sequence of packet buffer sizes.
// The first (or only) buffer will *additionally* include a total length field and PacketHeader;
// except for these, the buffer contents is initialized to zero.
chip::System::PacketBufferHandle PreparePacketBuffers(nlTestSuite * inSuite, const uint16_t sizes[])
{
    PacketHeader header;
    header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageId(kMessageId);
    uint16_t header_length = header.EncodeSizeBytes();

    uint16_t message_length = header_length;
    for (int i = 0; sizes[i] != 0; ++i)
    {
        message_length = static_cast<uint16_t>(message_length + sizes[i]);
    }
    constexpr uint16_t kPacketSizeBytes = static_cast<uint16_t>(sizeof(uint16_t));

    const uint16_t headLength             = static_cast<uint16_t>(kPacketSizeBytes + header_length + sizes[0]);
    chip::System::PacketBufferHandle head = chip::System::PacketBufferHandle::New(headLength);
    NL_TEST_ASSERT(inSuite, !head.IsNull());
    chip::Encoding::LittleEndian::Put16(head->Start(), message_length);
    uint16_t header_size;
    CHIP_ERROR err = header.Encode(head->Start() + kPacketSizeBytes,
                                   static_cast<uint16_t>(head->AvailableDataLength() - kPacketSizeBytes), &header_size);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    memset(head->Start() + kPacketSizeBytes + header_size, 0, head->AvailableDataLength() - kPacketSizeBytes - header_size);
    head->SetDataLength(headLength);

    for (int i = 1; sizes[i] != 0; ++i)
    {
        chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::New(sizes[i]);
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());
        memset(head->Start(), 0, head->AvailableDataLength());
        buffer->SetDataLength(sizes[i]);
        head->AddToEnd(std::move(buffer));
    }

    return head;
}

void chip::Transport::TCPTest::CheckProcessReceivedBuffer(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TCPImpl tcp;

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    // On receiving a message, we verify that its length matches the expected length.
    // The test uses a unique length for each message, so this confirms receiving the expected message.
    const uint16_t * expected_lengths = nullptr;
    MockTransportMgrDelegate gMockTransportMgrDelegate(
        inSuite, ctx,
        [](const uint8_t * message, size_t length, void * data) -> int {
            const uint16_t ** lp = static_cast<const uint16_t **>(data);
            return (lp == nullptr) || (*lp == nullptr) || (*(*lp)++ != length);
        },
        &expected_lengths);
    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr);

    // Send a packet to get TCP going, so that we can find a TCPEndPoint to pass to ProcessReceivedBuffer.
    // (The current TCPEndPoint implementation is not effectively mockable.)
    expected_lengths = (const uint16_t[]){ sizeof(PAYLOAD) };
    gMockTransportMgrDelegate.SingleMessageTest(tcp, addr);

    Transport::PeerAddress lPeerAddress    = Transport::PeerAddress::TCP(addr);
    TCPBase::ActiveConnectionState * state = tcp.FindActiveConnection(lPeerAddress);
    NL_TEST_ASSERT(inSuite, state != nullptr);
    Inet::TCPEndPoint * lEndPoint = state->mEndPoint;
    NL_TEST_ASSERT(inSuite, lEndPoint != nullptr);

    chip::System::PacketBufferHandle buffer;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Test a single packet buffer.
    expected_lengths                                   = (const uint16_t[]){ 11, 0 };
    buffer                                             = PreparePacketBuffers(inSuite, expected_lengths);
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    err                                                = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 1);

    // Test a message in a chain of three packet buffers.
    const uint16_t k2Lengths[]                         = { 21, 22, 23, 0 };
    expected_lengths                                   = (const uint16_t[]){ 66, 0 };
    buffer                                             = PreparePacketBuffers(inSuite, k2Lengths);
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    err                                                = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 1);

    // Test two messages in a chain.
    const uint16_t k3Lengths1[] = { 31, 0 };
    const uint16_t k3Lengths2[] = { 32, 0 };
    expected_lengths            = (const uint16_t[]){ 31, 32, 0 };
    buffer                      = PreparePacketBuffers(inSuite, k3Lengths1);
    buffer->AddToEnd(PreparePacketBuffers(inSuite, k3Lengths2));
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    err                                                = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 2);

    // Test a chain of two messages, each a chain.
    const uint16_t k4Lengths1[] = { 41, 42, 0 };
    const uint16_t k4Lengths2[] = { 43, 44, 0 };
    expected_lengths            = (const uint16_t[]){ 83, 87, 0 };
    buffer                      = PreparePacketBuffers(inSuite, k4Lengths1);
    buffer->AddToEnd(PreparePacketBuffers(inSuite, k4Lengths2));
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    err                                                = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 2);

    // Test a chain that is too large to coalesce into a single packet buffer, followed by a normal message.
    // We expect to receive only the latter.
    const uint16_t k5Lengths1[] = { 51, System::PacketBuffer::kMaxSize, 0 };
    const uint16_t k5Lengths2[] = { 53, 54, 0 };
    expected_lengths            = (const uint16_t[]){ 107, 0 };
    buffer                      = PreparePacketBuffers(inSuite, k5Lengths1);
    buffer->AddToEnd(PreparePacketBuffers(inSuite, k5Lengths2));

    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    err                                                = tcp.ProcessReceivedBuffer(lEndPoint, lPeerAddress, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gMockTransportMgrDelegate.mReceiveHandlerCallCount == 1);

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
