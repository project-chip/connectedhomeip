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
 *      This file implements unit tests for the SessionManager implementation.
 */

#define CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API // Up here in case some other header
                                              // includes SessionManager.h indirectly

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#undef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Test;

using TestContext = chip::Test::IOContext;

TestContext sContext;

const char PAYLOAD[]                = "Hello!";
constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;

const char LARGE_PAYLOAD[kMaxAppMessageLen + 1] = "test message";

class TestSessionReleaseCallback : public SessionReleaseDelegate
{
public:
    void OnSessionReleased() override { mOldConnectionDropped = true; }
    bool mOldConnectionDropped = false;
};

class TestSessMgrCallback : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           const Transport::PeerAddress & source, DuplicateMessage isDuplicate,
                           System::PacketBufferHandle && msgBuf) override
    {
        size_t data_len = msgBuf->DataLength();

        if (LargeMessageSent)
        {
            int compare = memcmp(msgBuf->Start(), LARGE_PAYLOAD, data_len);
            NL_TEST_ASSERT(mSuite, compare == 0);
        }
        else
        {
            int compare = memcmp(msgBuf->Start(), PAYLOAD, data_len);
            NL_TEST_ASSERT(mSuite, compare == 0);
        }

        ReceiveHandlerCallCount++;
    }

    nlTestSuite * mSuite        = nullptr;
    int ReceiveHandlerCallCount = 0;
    bool LargeMessageSent       = false;
};

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    CHIP_ERROR err;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckMessageTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));
    SessionHolder localToRemoteSession;
    SessionHolder remoteToLocalSession;

    SecurePairingUsingTestSecret pairing1(1, 2);
    err =
        sessionManager.NewPairing(localToRemoteSession, peer, kSourceNodeId, &pairing1, CryptoContext::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = sessionManager.NewPairing(remoteToLocalSession, peer, kDestinationNodeId, &pairing2,
                                    CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    EncryptedPacketBufferHandle preparedMessage;
    err = sessionManager.PrepareMessage(localToRemoteSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Let's send the max sized message and make sure it is received
    chip::System::PacketBufferHandle large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, kMaxAppMessageLen);
    NL_TEST_ASSERT(inSuite, !large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(localToRemoteSession.Get(), payloadHeader, std::move(large_buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    uint16_t large_payload_len = sizeof(LARGE_PAYLOAD);

    // Let's send bigger message than supported and make sure it fails to send
    chip::System::PacketBufferHandle extra_large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, large_payload_len);
    NL_TEST_ASSERT(inSuite, !extra_large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = sessionManager.PrepareMessage(localToRemoteSession.Get(), payloadHeader, std::move(extra_large_buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_MESSAGE_TOO_LONG);

    sessionManager.Shutdown();
}

void SendEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));
    SessionHolder localToRemoteSession;
    SessionHolder remoteToLocalSession;

    SecurePairingUsingTestSecret pairing1(1, 2);
    err =
        sessionManager.NewPairing(localToRemoteSession, peer, kSourceNodeId, &pairing1, CryptoContext::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = sessionManager.NewPairing(remoteToLocalSession, peer, kDestinationNodeId, &pairing2,
                                    CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(localToRemoteSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = remoteToLocalSession.Get()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    sessionManager.Shutdown();
}

void SendBadEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    TestSessMgrCallback callback;
    callback.LargeMessageSent = false;

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    sessionManager.SetMessageDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));
    SessionHolder localToRemoteSession;
    SessionHolder remoteToLocalSession;

    SecurePairingUsingTestSecret pairing1(1, 2);
    err =
        sessionManager.NewPairing(localToRemoteSession, peer, kSourceNodeId, &pairing1, CryptoContext::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = sessionManager.NewPairing(remoteToLocalSession, peer, kDestinationNodeId, &pairing2,
                                    CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = sessionManager.PrepareMessage(localToRemoteSession.Get(), payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    // Reset receive side message counter, or duplicated message will be denied.
    Transport::SecureSession * session = remoteToLocalSession.Get()->AsSecureSession();
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    PacketHeader packetHeader;

    // Change Message ID
    EncryptedPacketBufferHandle badMessageCounterMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badMessageCounterMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    uint32_t messageCounter = packetHeader.GetMessageCounter();
    packetHeader.SetMessageCounter(messageCounter + 1);
    NL_TEST_ASSERT(inSuite, badMessageCounterMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), badMessageCounterMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Key ID
    EncryptedPacketBufferHandle badKeyIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    // the secure channel is setup to use key ID 1, and 2. So let's use 3 here.
    packetHeader.SetSessionId(3);
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), badKeyIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    /* -------------------------------------------------------------------------------------------*/
    session->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Send the correct encrypted msg
    err = sessionManager.SendPreparedMessage(localToRemoteSession.Get(), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    sessionManager.Shutdown();
}

void StaleConnectionDropTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    SessionManager sessionManager;
    secure_channel::MessageCounterManager gMessageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = sessionManager.Init(&ctx.GetSystemLayer(), &transportMgr, &gMessageCounterManager, &deviceStorage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));
    TestSessionReleaseCallback callback;
    SessionHolderWithDelegate session1(callback);
    SessionHolderWithDelegate session2(callback);
    SessionHolderWithDelegate session3(callback);
    SessionHolderWithDelegate session4(callback);
    SessionHolderWithDelegate session5(callback);

    // First pairing
    SecurePairingUsingTestSecret pairing1(1, 1);
    callback.mOldConnectionDropped = false;
    err = sessionManager.NewPairing(session1, peer, kSourceNodeId, &pairing1, CryptoContext::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !callback.mOldConnectionDropped);

    // New pairing with different peer node ID and different local key ID (same peer key ID)
    SecurePairingUsingTestSecret pairing2(1, 2);
    callback.mOldConnectionDropped = false;
    err = sessionManager.NewPairing(session2, peer, kSourceNodeId, &pairing2, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !callback.mOldConnectionDropped);

    // New pairing with undefined node ID and different local key ID (same peer key ID)
    SecurePairingUsingTestSecret pairing3(1, 3);
    callback.mOldConnectionDropped = false;
    err = sessionManager.NewPairing(session3, peer, kUndefinedNodeId, &pairing3, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !callback.mOldConnectionDropped);

    // New pairing with same local key ID, and a given node ID
    SecurePairingUsingTestSecret pairing4(1, 2);
    callback.mOldConnectionDropped = false;
    err = sessionManager.NewPairing(session4, peer, kSourceNodeId, &pairing4, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, callback.mOldConnectionDropped);

    // New pairing with same local key ID, and undefined node ID
    SecurePairingUsingTestSecret pairing5(1, 1);
    callback.mOldConnectionDropped = false;
    err = sessionManager.NewPairing(session5, peer, kUndefinedNodeId, &pairing5, CryptoContext::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, callback.mOldConnectionDropped);

    sessionManager.Shutdown();
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Simple Init Test",               CheckSimpleInitTest),
    NL_TEST_DEF("Message Self Test",              CheckMessageTest),
    NL_TEST_DEF("Send Encrypted Packet Test",     SendEncryptedPacketTest),
    NL_TEST_DEF("Send Bad Encrypted Packet Test", SendBadEncryptedPacketTest),
    NL_TEST_DEF("Drop stale connection Test",     StaleConnectionDropTest),

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-SessionManager",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

/**
 *  Initialize the test suite.
 */
int Initialize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

/**
 *  Main
 */
int TestSessionManager()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    int r = (nlTestRunnerStats(&sSuite));
    return r;
}

CHIP_REGISTER_TEST_SUITE(TestSessionManager);
