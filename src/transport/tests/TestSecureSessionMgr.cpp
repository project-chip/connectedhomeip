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
 *      This file implements unit tests for the SecureSessionMgr implementation.
 */

#define CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API // Up here in case some other header
                                              // includes SecureSessionMgr.h indirectly

#include <core/CHIPCore.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>
#include <transport/SecureSessionMgr.h>
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

using TestContext = chip::Test::IOContext;

TestContext sContext;

const char PAYLOAD[]                = "Hello!";
constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;

const char LARGE_PAYLOAD[kMaxAppMessageLen + 1] = "test message";

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf) override
    {
        HandleMessageReceived(address, std::move(msgBuf));
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

class OutgoingTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf) override
    {
        System::PacketBufferHandle recvdMsg = msgBuf.CloneData();

        HandleMessageReceived(address, std::move(recvdMsg));
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

class TestSessMgrCallback : public SecureSessionMgrDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf,
                           SecureSessionMgr * mgr) override
    {
        NL_TEST_ASSERT(mSuite, header.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
        NL_TEST_ASSERT(mSuite, header.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
        NL_TEST_ASSERT(mSuite, session == mRemoteToLocalSession); // Packet received by remote peer

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

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override
    {
        // Preset the MessageCounter
        if (NewConnectionHandlerCallCount == 0)
            mRemoteToLocalSession = session;
        if (NewConnectionHandlerCallCount == 1)
            mLocalToRemoteSession = session;
        NewConnectionHandlerCallCount++;
    }
    void OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr) override {}

    nlTestSuite * mSuite = nullptr;
    SecureSessionHandle mRemoteToLocalSession;
    SecureSessionHandle mLocalToRemoteSession;
    int ReceiveHandlerCallCount       = 0;
    int NewConnectionHandlerCallCount = 0;

    bool LargeMessageSent = false;
};

TestSessMgrCallback callback;

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    secure_channel::MessageCounterManager gMessageCounterManager;

    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins, &gMessageCounterManager);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckMessageTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    callback.LargeMessageSent = false;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    secure_channel::MessageCounterManager gMessageCounterManager;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins, &gMessageCounterManager);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    secureSessionMgr.SetDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    Transport::AdminPairingInfo * admin = admins.AssignAdminId(0, kSourceNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    admin = admins.AssignAdminId(1, kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    SecurePairingUsingTestSecret pairing1(1, 2);
    err = secureSessionMgr.NewPairing(peer, kSourceNodeId, &pairing1, SecureSession::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = secureSessionMgr.NewPairing(peer, kDestinationNodeId, &pairing2, SecureSession::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle localToRemoteSession = callback.mLocalToRemoteSession;

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    EncryptedPacketBufferHandle preparedMessage;
    err = secureSessionMgr.BuildEncryptedMessagePayload(localToRemoteSession, payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Let's send the max sized message and make sure it is received
    chip::System::PacketBufferHandle large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, kMaxAppMessageLen);
    NL_TEST_ASSERT(inSuite, !large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = secureSessionMgr.BuildEncryptedMessagePayload(localToRemoteSession, payloadHeader, std::move(large_buffer),
                                                        preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);

    uint16_t large_payload_len = sizeof(LARGE_PAYLOAD);

    // Let's send bigger message than supported and make sure it fails to send
    chip::System::PacketBufferHandle extra_large_buffer = chip::MessagePacketBuffer::NewWithData(LARGE_PAYLOAD, large_payload_len);
    NL_TEST_ASSERT(inSuite, !extra_large_buffer.IsNull());

    callback.LargeMessageSent = true;

    err = secureSessionMgr.BuildEncryptedMessagePayload(localToRemoteSession, payloadHeader, std::move(extra_large_buffer),
                                                        preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_MESSAGE_TOO_LONG);
}

void SendEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    callback.LargeMessageSent = false;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<OutgoingTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    secure_channel::MessageCounterManager gMessageCounterManager;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins, &gMessageCounterManager);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    secureSessionMgr.SetDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    Transport::AdminPairingInfo * admin = admins.AssignAdminId(0, kSourceNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    admin = admins.AssignAdminId(1, kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    SecurePairingUsingTestSecret pairing1(1, 2);
    err = secureSessionMgr.NewPairing(peer, kSourceNodeId, &pairing1, SecureSession::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = secureSessionMgr.NewPairing(peer, kDestinationNodeId, &pairing2, SecureSession::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle localToRemoteSession = callback.mLocalToRemoteSession;

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = secureSessionMgr.BuildEncryptedMessagePayload(localToRemoteSession, payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Reset receive side message counter, or duplicated message will be denied.
    Transport::PeerConnectionState * state = secureSessionMgr.GetPeerConnectionState(callback.mRemoteToLocalSession);
    state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);
}

void SendBadEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    callback.LargeMessageSent = false;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<OutgoingTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    secure_channel::MessageCounterManager gMessageCounterManager;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins, &gMessageCounterManager);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    secureSessionMgr.SetDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    Transport::AdminPairingInfo * admin = admins.AssignAdminId(0, kSourceNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    admin = admins.AssignAdminId(1, kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    SecurePairingUsingTestSecret pairing1(1, 2);
    err = secureSessionMgr.NewPairing(peer, kSourceNodeId, &pairing1, SecureSession::SessionRole::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = secureSessionMgr.NewPairing(peer, kDestinationNodeId, &pairing2, SecureSession::SessionRole::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle localToRemoteSession = callback.mLocalToRemoteSession;

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle preparedMessage;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = secureSessionMgr.BuildEncryptedMessagePayload(localToRemoteSession, payloadHeader, std::move(buffer), preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    // Reset receive side message counter, or duplicated message will be denied.
    Transport::PeerConnectionState * state = secureSessionMgr.GetPeerConnectionState(callback.mRemoteToLocalSession);
    state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    PacketHeader packetHeader;

    // Change Destination Node ID
    EncryptedPacketBufferHandle badDestNodeIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badDestNodeIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, packetHeader.GetDestinationNodeId().Value() == kDestinationNodeId);
    packetHeader.SetDestinationNodeId(kSourceNodeId);
    NL_TEST_ASSERT(inSuite, badDestNodeIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, badDestNodeIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Source Node ID
    EncryptedPacketBufferHandle badSrcNodeIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badSrcNodeIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    packetHeader.SetSourceNodeId(kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, badSrcNodeIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, badSrcNodeIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Source Node ID
    EncryptedPacketBufferHandle noDstNodeIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, noDstNodeIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    packetHeader.ClearDestinationNodeId();
    NL_TEST_ASSERT(inSuite, noDstNodeIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, noDstNodeIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Message ID
    EncryptedPacketBufferHandle badMessageIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badMessageIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    uint32_t msgID = packetHeader.GetMessageId();
    packetHeader.SetMessageId(msgID + 1);
    NL_TEST_ASSERT(inSuite, badMessageIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, badMessageIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    /* -------------------------------------------------------------------------------------------*/
    state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    // Change Key ID
    EncryptedPacketBufferHandle badKeyIdMsg = preparedMessage.CloneData();
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    // the secure channel is setup to use key ID 1, and 2. So let's use 3 here.
    packetHeader.SetEncryptionKeyID(3);
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, badKeyIdMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    /* -------------------------------------------------------------------------------------------*/
    state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(1);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Send the correct encrypted msg
    err = secureSessionMgr.SendPreparedMessage(localToRemoteSession, preparedMessage);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);
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

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-Connection",
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
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init(&sSuite);
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
int TestSecureSessionMgr()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    int r = (nlTestRunnerStats(&sSuite));
    return r;
}

CHIP_REGISTER_TEST_SUITE(TestSecureSessionMgr);
