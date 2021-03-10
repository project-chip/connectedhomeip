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
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
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

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, const PeerAddress & address, System::PacketBufferHandle msgBuf) override
    {
        HandleMessageReceived(header, address, std::move(msgBuf));
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

class OutgoingTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, const PeerAddress & address, System::PacketBufferHandle msgBuf) override
    {
        System::PacketBufferHandle recvdMsg = msgBuf.CloneData();

        ReturnErrorOnFailure(header.EncodeBeforeData(msgBuf));

        HandleMessageReceived(header, address, std::move(recvdMsg));
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

class TestSessMgrCallback : public SecureSessionMgrDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           System::PacketBufferHandle msgBuf, SecureSessionMgr * mgr) override
    {
        NL_TEST_ASSERT(mSuite, header.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
        NL_TEST_ASSERT(mSuite, header.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
        NL_TEST_ASSERT(mSuite, session == mRemoteToLocalSession); // Packet received by remote peer

        size_t data_len = msgBuf->DataLength();

        int compare = memcmp(msgBuf->Start(), PAYLOAD, data_len);
        NL_TEST_ASSERT(mSuite, compare == 0);

        ReceiveHandlerCallCount++;
    }

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override
    {
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
};

TestSessMgrCallback callback;

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckMessageTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    secureSessionMgr.SetDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    Transport::AdminPairingInfo * admin = admins.AssignAdminId(0, kSourceNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    admin = admins.AssignAdminId(1, kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    SecurePairingUsingTestSecret pairing1(1, 2);
    err = secureSessionMgr.NewPairing(peer, kSourceNodeId, &pairing1, SecureSessionMgr::PairingDirection::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = secureSessionMgr.NewPairing(peer, kDestinationNodeId, &pairing2, SecureSessionMgr::PairingDirection::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle localToRemoteSession = callback.mLocalToRemoteSession;

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    err = secureSessionMgr.SendMessage(localToRemoteSession, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 0; });

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);
}

void SendEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<OutgoingTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    secureSessionMgr.SetDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    Transport::AdminPairingInfo * admin = admins.AssignAdminId(0, kSourceNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    admin = admins.AssignAdminId(1, kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    SecurePairingUsingTestSecret pairing1(1, 2);
    err = secureSessionMgr.NewPairing(peer, kSourceNodeId, &pairing1, SecureSessionMgr::PairingDirection::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = secureSessionMgr.NewPairing(peer, kDestinationNodeId, &pairing2, SecureSessionMgr::PairingDirection::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle localToRemoteSession = callback.mLocalToRemoteSession;

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle msgBuf;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = secureSessionMgr.SendMessage(localToRemoteSession, payloadHeader, std::move(buffer), &msgBuf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 0; });
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    err = secureSessionMgr.SendEncryptedMessage(localToRemoteSession, std::move(msgBuf), nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 1; });
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 2);
}

void SendBadEncryptedPacketTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<OutgoingTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Transport::AdminPairingTable admins;
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr, &admins);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    callback.mSuite = inSuite;

    secureSessionMgr.SetDelegate(&callback);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    Transport::AdminPairingInfo * admin = admins.AssignAdminId(0, kSourceNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    admin = admins.AssignAdminId(1, kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, admin != nullptr);

    SecurePairingUsingTestSecret pairing1(1, 2);
    err = secureSessionMgr.NewPairing(peer, kSourceNodeId, &pairing1, SecureSessionMgr::PairingDirection::kInitiator, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(2, 1);
    err = secureSessionMgr.NewPairing(peer, kDestinationNodeId, &pairing2, SecureSessionMgr::PairingDirection::kResponder, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle localToRemoteSession = callback.mLocalToRemoteSession;

    // Should be able to send a message to itself by just calling send.
    callback.ReceiveHandlerCallCount = 0;

    PayloadHeader payloadHeader;
    EncryptedPacketBufferHandle msgBuf;

    // Set the exchange ID for this header.
    payloadHeader.SetExchangeID(0);

    // Set the protocol ID and message type for this header.
    payloadHeader.SetMessageType(chip::Protocols::Echo::MsgType::EchoRequest);

    payloadHeader.SetInitiator(true);

    err = secureSessionMgr.SendMessage(localToRemoteSession, payloadHeader, std::move(buffer), &msgBuf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 0; });
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    PacketHeader packetHeader;

    // Change Destination Node ID
    EncryptedPacketBufferHandle badDestNodeIdMsg = msgBuf.CloneData();
    NL_TEST_ASSERT(inSuite, badDestNodeIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, packetHeader.GetDestinationNodeId().Value() == kDestinationNodeId);
    packetHeader.SetDestinationNodeId(kSourceNodeId);
    NL_TEST_ASSERT(inSuite, badDestNodeIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendEncryptedMessage(localToRemoteSession, std::move(badDestNodeIdMsg), nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 1; });

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Change Source Node ID
    EncryptedPacketBufferHandle badSrcNodeIdMsg = msgBuf.CloneData();
    NL_TEST_ASSERT(inSuite, badSrcNodeIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    packetHeader.SetSourceNodeId(kDestinationNodeId);
    NL_TEST_ASSERT(inSuite, badSrcNodeIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendEncryptedMessage(localToRemoteSession, std::move(badSrcNodeIdMsg), nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 1; });

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Change Message ID
    EncryptedPacketBufferHandle badMessageIdMsg = msgBuf.CloneData();
    NL_TEST_ASSERT(inSuite, badMessageIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    uint32_t msgID = packetHeader.GetMessageId();
    packetHeader.SetMessageId(msgID + 1);
    NL_TEST_ASSERT(inSuite, badMessageIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendEncryptedMessage(localToRemoteSession, std::move(badMessageIdMsg), nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 1; });

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Change Key ID
    EncryptedPacketBufferHandle badKeyIdMsg = msgBuf.CloneData();
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.ExtractPacketHeader(packetHeader) == CHIP_NO_ERROR);

    // the secure channel is setup to use key ID 1, and 2. So let's use 3 here.
    packetHeader.SetEncryptionKeyID(3);
    NL_TEST_ASSERT(inSuite, badKeyIdMsg.InsertPacketHeader(packetHeader) == CHIP_NO_ERROR);

    err = secureSessionMgr.SendEncryptedMessage(localToRemoteSession, std::move(badKeyIdMsg), nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 1; });

    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);

    // Send the correct encrypted msg
    err = secureSessionMgr.SendEncryptedMessage(localToRemoteSession, std::move(msgBuf), nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.DriveIOUntil(1000 /* ms */, []() { return callback.ReceiveHandlerCallCount != 1; });
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
