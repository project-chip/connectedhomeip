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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include <pw_unit_test/framework.h>

#include <crypto/RandUtils.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestUtils.h>
#include <system/SystemLayer.h>
#include <transport/TransportMgr.h>
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <transport/raw/TCP.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <transport/raw/tests/TCPBaseTestAccess.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::Inet;
using namespace chip::Transport;

namespace {

constexpr size_t kMaxTcpActiveConnectionCount = 4;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr size_t kPacketSizeBytes             = sizeof(uint32_t);

using TCPImpl    = Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>;
using TestAccess = Transport::TCPBaseTestAccess<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>;

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;
constexpr uint32_t kMessageCounter  = 18;

const char PAYLOAD[]            = "Hello!";
const char PAYLOAD_RESPONSE[]   = "Howdy!";
const char PAYLOAD_B[]          = "Yo!";
const char PAYLOAD_B_RESPONSE[] = "Hi!";
const char messageSize_TEST[]   = "\x00\x00\x00\x00";
const size_t kMaxIncoming       = 2;

uint16_t GetRandomPort()
{
    return static_cast<uint16_t>(CHIP_PORT + chip::Crypto::GetRandU16() % 100);
}

class MockTransportMgrDelegate : public chip::TransportMgrDelegate
{
public:
    ActiveTCPConnectionHolder refHolder;
    ActiveTCPConnectionHolder incoming[kMaxIncoming];
    ActiveTCPConnectionHolder activeTCPConnState;

    using MessageReceivedCallback =
        std::function<CHIP_ERROR(const uint8_t * message, size_t length, int count, ActiveTCPConnectionHolder & conn, void * data)>;

    MockTransportMgrDelegate(IOContext * inContext) : mIOContext(inContext), mCallback(nullptr), mCallbackData(nullptr) {}
    ~MockTransportMgrDelegate() override {}

    void SetCallback(MessageReceivedCallback callback = nullptr, void * callback_data = nullptr)
    {
        mCallback     = callback;
        mCallbackData = callback_data;
    }

    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf,
                           Transport::MessageTransportContext * transCtxt = nullptr) override
    {
        PacketHeader packetHeader;

        EXPECT_EQ(packetHeader.DecodeAndConsume(msgBuf), CHIP_NO_ERROR);

        ActiveTCPConnectionHolder connection;
        if (transCtxt)
        {
            // Store a reference to the connection so it's not auto-disconnected
            ActiveTCPConnectionHolder * available = nullptr;
            for (size_t i = 0; i < kMaxIncoming; i++)
            {
                if (incoming[i] == transCtxt->conn)
                {
                    available = &incoming[i];
                    break;
                }
                if (incoming[i].IsNull())
                {
                    available = &incoming[i];
                }
            }
            ASSERT_NE(available, nullptr);

            *available = transCtxt->conn;
            connection = *available;
        }

        if (mCallback)
        {
            EXPECT_EQ(mCallback(msgBuf->Start(), msgBuf->DataLength(), mReceiveHandlerCallCount, connection, mCallbackData),
                      CHIP_NO_ERROR);
        }

        ChipLogProgress(Inet, "Message Receive Handler called");

        mReceiveHandlerCallCount++;
    }

    void HandleConnectionAttemptComplete(ActiveTCPConnectionHolder & conn, CHIP_ERROR conErr) override
    {
        AppTCPConnectionCallbackCtxt * appConnCbCtxt = nullptr;
        VerifyOrReturn(!conn.IsNull());

        mHandleConnectionCompleteCalled = true;
        appConnCbCtxt                   = conn->mAppState;
        VerifyOrReturn(appConnCbCtxt != nullptr);

        if (appConnCbCtxt->connCompleteCb != nullptr)
        {
            appConnCbCtxt->connCompleteCb(conn, conErr);
        }
        else
        {
            ChipLogProgress(Inet, "Connection established. App callback missing.");
        }
    }

    void HandleConnectionClosed(ActiveTCPConnectionState & conn, CHIP_ERROR conErr) override
    {
        AppTCPConnectionCallbackCtxt * appConnCbCtxt = nullptr;

        mHandleConnectionCloseCalled = true;
        appConnCbCtxt                = conn.mAppState;
        VerifyOrReturn(appConnCbCtxt != nullptr);

        if (appConnCbCtxt->connClosedCb != nullptr)
        {
            appConnCbCtxt->connClosedCb(conn, conErr);
        }
        else
        {
            ChipLogProgress(Inet, "Connection Closed. App callback missing.");
        }
    }

    void InitializeMessageTest(TCPImpl & tcp, const IPAddress & addr, uint16_t port)
    {
        CHIP_ERROR err = tcp.Init(
            Transport::TcpListenParameters(mIOContext->GetTCPEndPointManager()).SetAddressType(addr.Type()).SetListenPort(port));

        // retry a few times in case the port is somehow in use; shouldn't happen generally
        // as we pick a random port
        for (int i = 0; (i < 50) && (err != CHIP_NO_ERROR); i++)
        {
            ChipLogProgress(NotSpecified, "RETRYING tcp initialization");
            chip::test_utils::SleepMillis(100);
            err = tcp.Init(Transport::TcpListenParameters(mIOContext->GetTCPEndPointManager())
                               .SetAddressType(addr.Type())
                               .SetListenPort(port));
        }

        EXPECT_EQ(err, CHIP_NO_ERROR);

        mTransportMgrBase.SetSessionManager(this);
        mTransportMgrBase.Init(&tcp);

        mReceiveHandlerCallCount        = 0;
        mHandleConnectionCompleteCalled = false;
        mHandleConnectionCloseCalled    = false;
    }

    template <size_t N>
    CHIP_ERROR BufferWithHeader(uint32_t messageCounter, chip::System::PacketBufferHandle & buffer, const char (&payload)[N])
    {
        buffer = chip::System::PacketBufferHandle::NewWithData(payload, N * sizeof(char));
        VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

        PacketHeader header;
        header.SetSourceNodeId(kSourceNodeId).SetDestinationNodeId(kDestinationNodeId).SetMessageCounter(messageCounter);

        return header.EncodeBeforeData(buffer);
    }

    void SingleMessageTest(TCPImpl & tcp, const IPAddress & addr, uint16_t port)
    {
        SetCallback([](const uint8_t * message, size_t length, int count, ActiveTCPConnectionHolder & conn, void * data) {
            return memcmp(message, PAYLOAD, length) == 0 ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
        });

        CHIP_ERROR err = tcp.TCPConnect(Transport::PeerAddress::TCP(addr, port), nullptr, refHolder);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Should be able to send a message to itself by just calling send.
        chip::System::PacketBufferHandle buffer;
        err = BufferWithHeader(kMessageCounter, buffer, PAYLOAD);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = tcp.SendMessage(Transport::PeerAddress::TCP(addr, port), std::move(buffer));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [this]() { return mReceiveHandlerCallCount != 0; });
        EXPECT_EQ(mReceiveHandlerCallCount, 1);

        SetCallback(nullptr);
    }

    void MultipleConnectionTest(TCPImpl & tcp, const IPAddress & addr, uint16_t port)
    {
        ActiveTCPConnectionHolder firstConnection;
        ActiveTCPConnectionHolder secondConnection;
        bool firstMessageReceived   = false;
        bool firstResponseReceived  = false;
        bool secondMessageReceived  = false;
        bool secondResponseReceived = false;
        SetCallback(
            [&](const uint8_t * message, size_t length, int count, ActiveTCPConnectionHolder & conn, void * data) -> CHIP_ERROR {
                if (memcmp(message, PAYLOAD, length) == 0)
                {
                    firstMessageReceived = true;

                    chip::System::PacketBufferHandle firstResponse;
                    ReturnErrorOnFailure(BufferWithHeader(kMessageCounter + 2, firstResponse, PAYLOAD_RESPONSE));

                    // First message, respond with first response
                    ReturnErrorOnFailure(tcp.SendMessage(conn, std::move(firstResponse)));
                }
                else if (memcmp(message, PAYLOAD_RESPONSE, length) == 0)
                {
                    firstResponseReceived = true;
                    VerifyOrReturnError(conn == firstConnection, CHIP_ERROR_INCORRECT_STATE);
                }
                else if (memcmp(message, PAYLOAD_B, length) == 0)
                {
                    secondMessageReceived = true;

                    chip::System::PacketBufferHandle secondResponse;
                    ReturnErrorOnFailure(BufferWithHeader(kMessageCounter + 3, secondResponse, PAYLOAD_B_RESPONSE));

                    // Second message, respond with second response
                    ReturnErrorOnFailure(tcp.SendMessage(conn, std::move(secondResponse)));
                }
                else if (memcmp(message, PAYLOAD_B_RESPONSE, length) == 0)
                {
                    secondResponseReceived = true;
                    VerifyOrReturnValue(conn == secondConnection, CHIP_ERROR_INCORRECT_STATE);
                }
                else
                {
                    return CHIP_ERROR_INCORRECT_STATE;
                }
                return CHIP_NO_ERROR;
            });

        CHIP_ERROR err = tcp.TCPConnect(Transport::PeerAddress::TCP(addr, port), nullptr, firstConnection);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Send the first message with expectation it is against first connection
        {
            chip::System::PacketBufferHandle firstMessage;
            err = BufferWithHeader(kMessageCounter, firstMessage, PAYLOAD);
            EXPECT_EQ(err, CHIP_NO_ERROR);
            err = tcp.SendMessage(Transport::PeerAddress::TCP(addr, port), std::move(firstMessage));
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        err = tcp.TCPConnect(Transport::PeerAddress::TCP(addr, port), nullptr, secondConnection);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Send the second message with the expectation it is against the second connection
        // (the only way this works is if these 2 are the same connection, keyed off of PeerAddress
        // as otherwise it would be unreasonable to expect correct mapping from SendMessage call)
        {
            chip::System::PacketBufferHandle secondMessage;
            err = BufferWithHeader(kMessageCounter + 1, secondMessage, PAYLOAD_B);
            EXPECT_EQ(err, CHIP_NO_ERROR);
            err = tcp.SendMessage(Transport::PeerAddress::TCP(addr, port), std::move(secondMessage));
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(10), [this]() { return mReceiveHandlerCallCount >= 4; });

        EXPECT_TRUE(firstMessageReceived);
        EXPECT_TRUE(firstResponseReceived);
        EXPECT_TRUE(secondMessageReceived);
        EXPECT_TRUE(secondResponseReceived);

        SetCallback(nullptr);
    }

    void ConnectTest(TCPImpl & tcp, const IPAddress & addr, uint16_t port)
    {
        // Connect and wait for seeing active connection
        CHIP_ERROR err = tcp.TCPConnect(Transport::PeerAddress::TCP(addr, port), nullptr, activeTCPConnState);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [&tcp]() { return tcp.HasActiveConnections(); });
        EXPECT_EQ(tcp.HasActiveConnections(), true);
    }

    void HandleConnectCompleteCbCalledTest(TCPImpl & tcp, const IPAddress & addr, uint16_t port)
    {
        // Connect and wait for seeing active connection and connection complete
        // handler being called.
        CHIP_ERROR err = tcp.TCPConnect(Transport::PeerAddress::TCP(addr, port), nullptr, activeTCPConnState);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [this]() { return mHandleConnectionCompleteCalled; });
        EXPECT_EQ(mHandleConnectionCompleteCalled, true);
    }

    void HandleConnectCloseCbCalledTest(TCPImpl & tcp, const IPAddress & addr, uint16_t port)
    {
        // Connect and wait for seeing active connection and connection complete
        // handler being called.
        CHIP_ERROR err = tcp.TCPConnect(Transport::PeerAddress::TCP(addr, port), nullptr, activeTCPConnState);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [this]() { return mHandleConnectionCompleteCalled; });
        EXPECT_EQ(mHandleConnectionCompleteCalled, true);

        activeTCPConnState.Release();
        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [&tcp]() { return !tcp.HasActiveConnections(); });
        EXPECT_EQ(mHandleConnectionCloseCalled, true);
    }

    void DisconnectTest(TCPImpl & tcp)
    {
        // Disconnect and wait for seeing peer close
        activeTCPConnState.Release();
        refHolder.Release();
        for (size_t i = 0; i < kMaxIncoming; i++)
        {
            incoming[i].Release();
        }
        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5), [&tcp]() { return !tcp.HasActiveConnections(); });
        EXPECT_EQ(tcp.HasActiveConnections(), false);
    }

    CHIP_ERROR TCPConnect(const Transport::PeerAddress & peerAddress, Transport::AppTCPConnectionCallbackCtxt * appState,
                          Transport::ActiveTCPConnectionHolder & peerConnState)
    {
        return mTransportMgrBase.TCPConnect(peerAddress, appState, peerConnState);
    }

    using OnTCPConnectionReceivedCallback = void (*)(void * context, ActiveTCPConnectionState * conn);

    using OnTCPConnectionCompleteCallback = void (*)(void * context, ActiveTCPConnectionState * conn, CHIP_ERROR conErr);

    using OnTCPConnectionClosedCallback = void (*)(void * context, ActiveTCPConnectionState * conn, CHIP_ERROR conErr);

    void SetConnectionCallbacks(OnTCPConnectionCompleteCallback connCompleteCb, OnTCPConnectionClosedCallback connClosedCb,
                                OnTCPConnectionReceivedCallback connReceivedCb)
    {
        mConnCompleteCb = connCompleteCb;
        mConnClosedCb   = connClosedCb;
        mConnReceivedCb = connReceivedCb;
    }

    int mReceiveHandlerCallCount = 0;

    bool mHandleConnectionCompleteCalled = false;

    bool mHandleConnectionCloseCalled = false;

private:
    IOContext * mIOContext;
    MessageReceivedCallback mCallback;
    void * mCallbackData;
    TransportMgrBase mTransportMgrBase;
    OnTCPConnectionCompleteCallback mConnCompleteCb = nullptr;
    OnTCPConnectionClosedCallback mConnClosedCb     = nullptr;
    OnTCPConnectionReceivedCallback mConnReceivedCb = nullptr;
};

// Generates a packet buffer or a chain of packet buffers for a single message.
struct TestData
{
    // `sizes[]` is a zero-terminated sequence of packet buffer sizes.
    // If total length supplied is not large enough for at least the PacketHeader and length field,
    // the last buffer will be made larger.
    TestData() : mPayload(nullptr), mTotalLength(0), mMessageLength(0), mMessageOffset(0) {}
    ~TestData() { Free(); }
    bool Init(const uint32_t sizes[]);
    void Free();
    bool IsValid() { return !mHandle.IsNull() && (mPayload != nullptr); }

    chip::System::PacketBufferHandle mHandle;
    uint8_t * mPayload;
    size_t mTotalLength;
    size_t mMessageLength;
    size_t mMessageOffset;
};

bool TestData::Init(const uint32_t sizes[])
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
    uint32_t additionalLength = 0;
    if (headerLength + kPacketSizeBytes > mTotalLength)
    {
        additionalLength = static_cast<uint32_t>((headerLength + kPacketSizeBytes) - mTotalLength);
        mTotalLength += additionalLength;
    }
    if (mTotalLength > UINT32_MAX)
    {
        return false;
    }
    uint32_t messageLength = static_cast<uint32_t>(mTotalLength - kPacketSizeBytes);

    // Build the test payload.
    uint8_t * payload = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(1, mTotalLength));
    if (payload == nullptr)
    {
        return false;
    }
    chip::Encoding::LittleEndian::Put32(payload, messageLength);
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
        size_t size = sizes[i];
        if (i == bufferCount)
        {
            size = size + additionalLength;
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
            iterator->SetDataLength(iterator->DataLength() + lToWriteToCurrentBuf, head);
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

class TestTCP : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        if (mIOContext == nullptr)
        {
            mIOContext = new IOContext();
            ASSERT_NE(mIOContext, nullptr);
        }
        ASSERT_EQ(mIOContext->Init(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        if (mIOContext != nullptr)
        {
            mIOContext->Shutdown();
            delete mIOContext;
            mIOContext = nullptr;
        }
    }

protected:
    static IOContext * mIOContext;

    /////////////////////////// Init test
    void CheckSimpleInitTest(IPAddressType type)
    {
        TCPImpl tcp;

        uint16_t port = GetRandomPort();
        CHIP_ERROR err =
            tcp.Init(Transport::TcpListenParameters(mIOContext->GetTCPEndPointManager()).SetAddressType(type).SetListenPort(port));

        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    /////////////////////////// Messaging test
    void CheckMessageTest(const IPAddress & addr)
    {
        TCPImpl tcp;

        uint16_t port = GetRandomPort();
        MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
        gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.SingleMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.DisconnectTest(tcp);
    }

    void CheckMultipleConnectionTest(const IPAddress & addr)
    {
        TCPImpl tcp;

        uint16_t port = GetRandomPort();
        MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
        gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.MultipleConnectionTest(tcp, addr, port);
        gMockTransportMgrDelegate.DisconnectTest(tcp);
    }

    void ConnectToSelfTest(const IPAddress & addr)
    {
        TCPImpl tcp;

        uint16_t port = GetRandomPort();
        MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
        gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.ConnectTest(tcp, addr, port);
        gMockTransportMgrDelegate.DisconnectTest(tcp);
    }

    void ConnectSendMessageThenCloseTest(const IPAddress & addr)
    {
        TCPImpl tcp;

        uint16_t port = GetRandomPort();
        MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
        gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.ConnectTest(tcp, addr, port);
        gMockTransportMgrDelegate.SingleMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.DisconnectTest(tcp);
    }

    void HandleConnCompleteTest(const IPAddress & addr)
    {
        TCPImpl tcp;

        uint16_t port = GetRandomPort();
        MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
        gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.HandleConnectCompleteCbCalledTest(tcp, addr, port);
        gMockTransportMgrDelegate.DisconnectTest(tcp);
    }

    void HandleConnCloseTest(const IPAddress & addr)
    {
        TCPImpl tcp;

        uint16_t port = GetRandomPort();
        MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
        gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);
        gMockTransportMgrDelegate.HandleConnectCloseCbCalledTest(tcp, addr, port);
        gMockTransportMgrDelegate.DisconnectTest(tcp);
    }

    // Callback used by CheckProcessReceivedBuffer.
    static CHIP_ERROR TestDataCallbackCheck(const uint8_t * message, size_t length, int count,
                                            ActiveTCPConnectionHolder & connection, void * data)
    {
        if (data == nullptr)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        TestData * currentData = static_cast<TestData *>(data) + count;
        if (currentData->mPayload == nullptr)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        if (currentData->mMessageLength != length)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        if (memcmp(currentData->mPayload + currentData->mMessageOffset, message, length) != 0)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        return CHIP_NO_ERROR;
    }
};

IOContext * TestTCP::mIOContext = nullptr;

#if INET_CONFIG_ENABLE_IPV4
TEST_F(TestTCP, CheckSimpleInitTest4)
{
    CheckSimpleInitTest(IPAddressType::kIPv4);
}

TEST_F(TestTCP, CheckMessageTest4)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CheckMessageTest(addr);
}

TEST_F(TestTCP, CheckMultipleConnectionTest4)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CheckMultipleConnectionTest(addr);
}
#endif

TEST_F(TestTCP, CheckSimpleInitTest6)
{
    CheckSimpleInitTest(IPAddressType::kIPv6);
}

TEST_F(TestTCP, InitializeAsTCPClient)
{
    TCPImpl tcp;
    auto tcpListenParams = Transport::TcpListenParameters(mIOContext->GetTCPEndPointManager());
    uint16_t port        = GetRandomPort();
    CHIP_ERROR err =
        tcp.Init(tcpListenParams.SetAddressType(IPAddressType::kIPv6).SetListenPort(port).SetServerListenEnabled(false));

    EXPECT_EQ(err, CHIP_NO_ERROR);

    bool isServerEnabled = tcpListenParams.IsServerListenEnabled();
    EXPECT_EQ(isServerEnabled, false);
}

TEST_F(TestTCP, InitializeAsTCPClientServer)
{
    TCPImpl tcp;
    auto tcpListenParams = Transport::TcpListenParameters(mIOContext->GetTCPEndPointManager());

    uint16_t port  = GetRandomPort();
    CHIP_ERROR err = tcp.Init(tcpListenParams.SetAddressType(IPAddressType::kIPv6).SetListenPort(port));

    EXPECT_EQ(err, CHIP_NO_ERROR);

    bool isServerEnabled = tcpListenParams.IsServerListenEnabled();
    EXPECT_EQ(isServerEnabled, true);
}

TEST_F(TestTCP, CheckMessageTest6)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CheckMessageTest(addr);
}

TEST_F(TestTCP, CheckMultipleConnectionTest6)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CheckMultipleConnectionTest(addr);
}

#if INET_CONFIG_ENABLE_IPV4
TEST_F(TestTCP, ConnectToSelfTest4)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    ConnectToSelfTest(addr);
}

TEST_F(TestTCP, ConnectSendMessageThenCloseTest4)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    ConnectSendMessageThenCloseTest(addr);
}

TEST_F(TestTCP, HandleConnCompleteCalledTest4)
{
    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    HandleConnCompleteTest(addr);
}
#endif // INET_CONFIG_ENABLE_IPV4

TEST_F(TestTCP, ConnectSendMessageThenCloseTest6)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    ConnectSendMessageThenCloseTest(addr);
}

TEST_F(TestTCP, HandleConnCompleteCalledTest6)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    HandleConnCompleteTest(addr);
}

TEST_F(TestTCP, HandleConnCloseCalledTest6)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    HandleConnCloseTest(addr);
}

TEST_F(TestTCP, CheckTCPEndpointAfterCloseTest)
{
    TCPImpl tcp;

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    uint16_t port = GetRandomPort();
    MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);
    gMockTransportMgrDelegate.ConnectTest(tcp, addr, port);

    Transport::PeerAddress lPeerAddress = Transport::PeerAddress::TCP(addr, port);
    auto state                          = TestAccess::FindActiveConnection(tcp, lPeerAddress);
    ASSERT_TRUE(state);
    TCPEndPoint * lEndPoint = TestAccess::GetEndpoint(state);
    ASSERT_NE(lEndPoint, nullptr);

    // Call Close and check the TCPEndpoint
    tcp.Close();
    lEndPoint = TestAccess::GetEndpoint(state);
    ASSERT_EQ(lEndPoint, nullptr);
}

TEST_F(TestTCP, CheckProcessReceivedBuffer)
{
    TCPImpl tcp;

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    uint16_t port = GetRandomPort();
    MockTransportMgrDelegate gMockTransportMgrDelegate(mIOContext);
    gMockTransportMgrDelegate.InitializeMessageTest(tcp, addr, port);

    // Send a packet to get TCP going, so that we can find a TCPEndPoint to pass to ProcessReceivedBuffer.
    // (The current TCPEndPoint implementation is not effectively mockable.)
    gMockTransportMgrDelegate.SingleMessageTest(tcp, addr, port);

    Transport::PeerAddress lPeerAddress = Transport::PeerAddress::TCP(addr, port);
    auto state                          = TestAccess::FindActiveConnection(tcp, lPeerAddress);
    ASSERT_TRUE(state);
    TCPEndPoint * lEndPoint = TestAccess::GetEndpoint(state);
    ASSERT_NE(lEndPoint, nullptr);

    CHIP_ERROR err = CHIP_NO_ERROR;
    TestData testData[2];
    gMockTransportMgrDelegate.SetCallback(TestDataCallbackCheck, testData);

    // Test a single packet buffer with zero message size.
    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(messageSize_TEST, 4);
    ASSERT_NE(&buf, nullptr);
    err = TestAccess::ProcessReceivedBuffer(tcp, lEndPoint, lPeerAddress, std::move(buf));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test a single packet buffer.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint32_t[]){ 111, 0 }));
    err = TestAccess::ProcessReceivedBuffer(tcp, lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 1);

    // Test a message in a chain of three packet buffers. The message length is split across buffers.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint32_t[]){ 1, 122, 123, 0 }));
    err = TestAccess::ProcessReceivedBuffer(tcp, lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 1);

    // Test two messages in a chain.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint32_t[]){ 131, 0 }));
    EXPECT_TRUE(testData[1].Init((const uint32_t[]){ 132, 0 }));
    testData[0].mHandle->AddToEnd(std::move(testData[1].mHandle));
    err = TestAccess::ProcessReceivedBuffer(tcp, lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 2);

    // Test a chain of two messages, each a chain.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint32_t[]){ 141, 142, 0 }));
    EXPECT_TRUE(testData[1].Init((const uint32_t[]){ 143, 144, 0 }));
    testData[0].mHandle->AddToEnd(std::move(testData[1].mHandle));
    err = TestAccess::ProcessReceivedBuffer(tcp, lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 2);

    // Test a single packet buffer that is larger than
    // kMaxSizeWithoutReserve but less than CHIP_CONFIG_MAX_LARGE_PAYLOAD_SIZE_BYTES.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    EXPECT_TRUE(testData[0].Init((const uint32_t[]){ System::PacketBuffer::kMaxSizeWithoutReserve + 1, 0 }));
    err = TestAccess::ProcessReceivedBuffer(tcp, lEndPoint, lPeerAddress, std::move(testData[0].mHandle));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 1);

    // Test a message that is too large to coalesce into a single packet buffer.
    gMockTransportMgrDelegate.mReceiveHandlerCallCount = 0;
    gMockTransportMgrDelegate.SetCallback(TestDataCallbackCheck, &testData[1]);
    EXPECT_TRUE(testData[0].Init((const uint32_t[]){ 51, CHIP_SYSTEM_CONFIG_MAX_LARGE_BUFFER_SIZE_BYTES, 0 }));
    // Sending only the first buffer of the long chain. This should be enough to trigger the error.
    System::PacketBufferHandle head = testData[0].mHandle.PopHead();
    err                             = TestAccess::ProcessReceivedBuffer(tcp, lEndPoint, lPeerAddress, std::move(head));
    EXPECT_EQ(err, CHIP_ERROR_MESSAGE_TOO_LONG);
    EXPECT_EQ(gMockTransportMgrDelegate.mReceiveHandlerCallCount, 0);

    // The receipt of a message exceeding the allowed size should have
    // closed the connection.
    EXPECT_EQ(TestAccess::GetEndpoint(state), nullptr);
}

} // namespace
