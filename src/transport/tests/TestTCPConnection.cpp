/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file implements an E2E test for the SessionManager/Transport/UnauthenticatedSession
 *      TCP connection.
 */

#include <errno.h>
#include <vector>

#include <pw_unit_test/framework.h>

#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <crypto/RandUtils.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestUtils.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PairingSession.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/TCP.h>
#include <transport/raw/tests/NetworkTestHelpers.h>
#include <transport/raw/tests/TCPBaseTestAccess.h>

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Testing;

namespace {

// Use only 2 active connections (1 outgoing, 1 incoming) to easily detect leaks (exhaustion)
constexpr size_t kMaxTcpActiveConnectionCount = 2;
constexpr size_t kMaxTcpPendingPackets        = 4;
constexpr int kMaxPortBindRetries             = 100;

using TCPImpl = Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>;

uint16_t GetRandomPort()
{
    return static_cast<uint16_t>(CHIP_PORT + chip::Crypto::GetRandU16() % 1000);
}

CHIP_ERROR RetryPortSetup(uint16_t & outPort, std::function<CHIP_ERROR(uint16_t)> setupFn)
{
    CHIP_ERROR err;
    for (int retryCount = 0; retryCount < kMaxPortBindRetries; retryCount++)
    {
        uint16_t port = GetRandomPort();

        err = setupFn(port);
        if (err == CHIP_ERROR_POSIX(EADDRINUSE))
        {
            continue;
        }

        outPort = (err == CHIP_NO_ERROR) ? port : 0;
        return err;
    }
    return err;
}

class TestTCPConnection : public ::testing::Test, public SessionConnectionDelegate, public SessionMessageDelegate
{
public:
    static void SetUpTestSuite()
    {
        if (mIOContext == nullptr)
        {
            mIOContext = new IOContext();
            ASSERT_NE(mIOContext, nullptr);
        }
        EXPECT_SUCCESS(mIOContext->Init());
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

    void SetUp() override
    {
        // Ignore Init return value, we check correctness later/implicitly
        (void) mTransportMgrBase.Init(&mTCP);
        mTransportMgrBase.SetSessionManager(&mSessionManager);

        mSessionManager.SetConnectionDelegate(this);
        mSessionManager.SetMessageDelegate(this);
    }

    void TearDown() override
    {
        mExchangeManager.Shutdown();
        mSessionManager.Shutdown();
        mTransportMgrBase.Close();
        mTCP.Close();
        mFabricTable.Shutdown();
        mOpKeyStore.Finish();
        mOpCertStore.Finish();
    }

    // SessionConnectionDelegate
    void OnTCPConnectionClosed(const Transport::ActiveTCPConnectionState & conn, const SessionHandle & session,
                               CHIP_ERROR conErr) override
    {}

    bool OnTCPConnectionAttemptComplete(Transport::ActiveTCPConnectionHandle & conn, CHIP_ERROR conErr) override { return true; }

    // SessionMessageDelegate
    void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           DuplicateMessage isDuplicate, System::PacketBufferHandle && msgBuf) override
    {
        mReceivedMessageCount++;
        mLastReceivedDuplicate = isDuplicate;
    }

protected:
    size_t mReceivedMessageCount            = 0;
    DuplicateMessage mLastReceivedDuplicate = DuplicateMessage::No;

    static IOContext * mIOContext;
    TCPImpl mTCP;
    TransportMgrBase mTransportMgrBase;
    SessionManager mSessionManager;
    Messaging::ExchangeManager mExchangeManager;
    secure_channel::MessageCounterManager mMessageCounterManager;
    chip::TestPersistentStorageDelegate mDeviceStorage;
    chip::Crypto::DefaultSessionKeystore mSessionKeystore;
    chip::PersistentStorageOperationalKeystore mOpKeyStore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
    FabricTable mFabricTable;

    CHIP_ERROR InitTCP(uint16_t & outPort)
    {
        return RetryPortSetup(outPort, [&](uint16_t port) {
            return mTCP.Init(Transport::TcpListenParameters(mIOContext->GetTCPEndPointManager())
                                 .SetAddressType(IPAddressType::kIPv6)
                                 .SetListenPort(port)
                                 .SetServerListenEnabled(true));
        });
    }

    CHIP_ERROR InitSessionManager()
    {
        ReturnErrorOnFailure(mOpKeyStore.Init(&mDeviceStorage));
        ReturnErrorOnFailure(mOpCertStore.Init(&mDeviceStorage));

        chip::FabricTable::InitParams initParams;
        initParams.storage             = &mDeviceStorage;
        initParams.operationalKeystore = &mOpKeyStore;
        initParams.opCertStore         = &mOpCertStore;

        ReturnErrorOnFailure(mFabricTable.Init(initParams));

        ReturnErrorOnFailure(mSessionManager.Init(&mIOContext->GetSystemLayer(), &mTransportMgrBase, &mMessageCounterManager,
                                                  &mDeviceStorage, &mFabricTable, mSessionKeystore));

        return mExchangeManager.Init(&mSessionManager);
    }

    CHIP_ERROR ConnectToSelf(uint16_t port, Transport::PeerAddress & outPeerAddr, ActiveTCPConnectionHandle & outConnHandle)
    {
        IPAddress addr;
        IPAddress::FromString("::1", addr);
        outPeerAddr = Transport::PeerAddress::TCP(addr, port);

        ReturnErrorOnFailure(mTCP.TCPConnect(outPeerAddr, nullptr, outConnHandle));
        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5),
                                 [&]() { return !outConnHandle.IsNull() && outConnHandle->IsConnected(); });
        VerifyOrReturnError(!outConnHandle.IsNull() && outConnHandle->IsConnected(), CHIP_ERROR_NOT_CONNECTED);
        return CHIP_NO_ERROR;
    }

    void ReceiveOverTCP(const Transport::PeerAddress & peerAddr, const ActiveTCPConnectionHandle & connHandle,
                        System::PacketBufferHandle && msg)
    {
        Transport::MessageTransportContext ctxt;
        ctxt.conn = connHandle;
        mSessionManager.OnMessageReceived(peerAddr, std::move(msg), &ctxt);
    }

    void CloseConnection(ActiveTCPConnectionHandle & connHandle)
    {
        connHandle->ForceDisconnect();
        connHandle.Release();
        mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(1), [&]() { return false; });
    }
};

// Injects the two halves of a CASE session pair established over UDP, so that a message
// prepared on `initiator` decrypts on `responder`.
CHIP_ERROR InjectUdpSessionPair(SessionManager & sessionManager, SessionHolder & initiator, SessionHolder & responder)
{
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    const Transport::PeerAddress udpPeerAddr = Transport::PeerAddress::UDP(addr, CHIP_PORT);

    ReturnErrorOnFailure(sessionManager.InjectCaseSessionWithTestKey(initiator, 1, 2, 1234, 5678, 1, udpPeerAddr,
                                                                     CryptoContext::SessionRole::kInitiator));
    return sessionManager.InjectCaseSessionWithTestKey(responder, 2, 1, 5678, 1234, 1, udpPeerAddr,
                                                       CryptoContext::SessionRole::kResponder);
}

CHIP_ERROR PrepareTestMessage(SessionManager & sessionManager, const SessionHandle & session,
                              EncryptedPacketBufferHandle & prepared)
{
    PayloadHeader payloadHeader;
    // Reliable, so a duplicate is not dropped at the no-ack fast path.
    payloadHeader.SetMessageType(Protocols::Id(VendorId::Common, 1221), 112)
        .SetExchangeID(2233)
        .SetInitiator(true)
        .SetNeedsAck(true);

    const uint8_t kPayload[]           = { 0x11, 0x22, 0x33, 0x44 };
    System::PacketBufferHandle payload = MessagePacketBuffer::NewWithData(kPayload, sizeof(kPayload));
    VerifyOrReturnError(!payload.IsNull(), CHIP_ERROR_NO_MEMORY);
    return sessionManager.PrepareMessage(session, payloadHeader, std::move(payload), prepared);
}

IOContext * TestTCPConnection::mIOContext = nullptr;

class PairingSessionUnderTest : public PairingSession
{
public:
    Transport::SecureSession::Type GetSecureSessionType() const override { return Transport::SecureSession::Type::kCASE; }
    ScopedNodeId GetPeer() const override { return ScopedNodeId(); }
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(); }
    CATValues GetPeerCATs() const override { return CATValues(); }
    CHIP_ERROR DeriveSecureSession(CryptoContext &) override { return CHIP_NO_ERROR; }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext *, const PayloadHeader &, System::PacketBufferHandle &&) override
    {
        return CHIP_NO_ERROR;
    }
    void OnResponseTimeout(Messaging::ExchangeContext *) override {}

    using PairingSession::AdoptExchange;
    using PairingSession::AllocateSecureSession;
};

TEST_F(TestTCPConnection, AdoptExchangeGivesTheSecureSessionTheTCPConnection)
{
    EXPECT_SUCCESS(InitSessionManager());

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    Transport::PeerAddress peerAddr = Transport::PeerAddress::TCP(addr, 5540);

    Inet::TCPEndPointHandle endPoint;
    EXPECT_SUCCESS(mIOContext->GetTCPEndPointManager()->NewEndPoint(endPoint));
    ASSERT_FALSE(endPoint.IsNull());

    auto connection = Transport::TCPBaseTestAccess<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>::AllocateConnection(
        mTCP, endPoint, peerAddr, Transport::TCPState::kConnecting);
    ASSERT_TRUE(connection);

    Optional<SessionHandle> unauthenticated = mSessionManager.CreateUnauthenticatedSession(peerAddr, GetDefaultMRPConfig());
    ASSERT_TRUE(unauthenticated.HasValue());
    unauthenticated.Value()->AsUnauthenticatedSession()->SetTCPConnection(connection.Handle());

    PairingSessionUnderTest pairingSession;
    EXPECT_SUCCESS(pairingSession.AllocateSecureSession(mSessionManager));

    Optional<SessionHandle> secure = pairingSession.CopySecureSession();
    ASSERT_TRUE(secure.HasValue());
    ASSERT_TRUE(secure.Value()->AsSecureSession()->GetTCPConnection().IsNull());

    Messaging::ExchangeContext * exchange = mExchangeManager.NewContext(unauthenticated.Value(), &pairingSession);
    ASSERT_NE(exchange, nullptr);

    pairingSession.AdoptExchange(*exchange);

    EXPECT_EQ(static_cast<const void *>(secure.Value()->AsSecureSession()->GetTCPConnection()),
              static_cast<const void *>(connection.Handle()));
}

TEST_F(TestTCPConnection, TestUnauthenticatedSessionReleaseOnConnectionClose)
{
    uint16_t port;
    EXPECT_SUCCESS(InitTCP(port));
    EXPECT_SUCCESS(InitSessionManager());

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    // Connect to self (loopback)
    Transport::PeerAddress peerAddr = Transport::PeerAddress::TCP(addr, port);

    // 1. Establish initial connection (Leaked Candidate)
    ActiveTCPConnectionHandle connHandle;
    EXPECT_SUCCESS(mTCP.TCPConnect(peerAddr, nullptr, connHandle));

    mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5),
                             [&]() { return !connHandle.IsNull() && connHandle->IsConnected(); });
    ASSERT_FALSE(connHandle.IsNull());
    ASSERT_TRUE(connHandle->IsConnected());

    // 2. Create UnauthenticatedSession by simulating an incoming message
    PayloadHeader header;
    uint8_t payloadBuffer[64];
    uint16_t encodeLen;
    header.SetMessageType(Protocols::Id(VendorId::Common, 1221), 112).SetExchangeID(2233).SetInitiator(true);
    EXPECT_SUCCESS(header.Encode(payloadBuffer, &encodeLen));

    auto msgBuf = System::PacketBufferHandle::NewWithData(payloadBuffer, encodeLen);
    PacketHeader packetHeader;
    packetHeader.SetSourceNodeId(1234).SetMessageCounter(1);
    packetHeader.SetSessionType(Header::SessionType::kUnicastSession); // Unauthenticated

    EXPECT_SUCCESS(packetHeader.EncodeBeforeData(msgBuf));

    {
        Transport::MessageTransportContext ctxt;
        ctxt.conn = connHandle;

        mSessionManager.OnMessageReceived(peerAddr, std::move(msgBuf), &ctxt);
    }

    // Close TCP connection & release local handle
    connHandle->ForceDisconnect();
    connHandle.Release();

    // Drive IO to process TCP closure
    mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(1), [&]() { return false; });

    // 5. Try to connect again to make sure there's no leak
    EXPECT_SUCCESS(mTCP.TCPConnect(peerAddr, nullptr, connHandle));
}

TEST_F(TestTCPConnection, TestDefunctSecureSessionReleaseOnConnectionClose)
{
    uint16_t port;
    EXPECT_SUCCESS(InitTCP(port));
    EXPECT_SUCCESS(InitSessionManager());

    IPAddress addr;
    IPAddress::FromString("::1", addr);

    // Connect to self (loopback)
    Transport::PeerAddress peerAddr = Transport::PeerAddress::TCP(addr, port);

    // 1. Establish initial connection
    ActiveTCPConnectionHandle connHandle;
    EXPECT_SUCCESS(mTCP.TCPConnect(peerAddr, nullptr, connHandle));

    mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5),
                             [&]() { return !connHandle.IsNull() && connHandle->IsConnected(); });
    ASSERT_FALSE(connHandle.IsNull());
    ASSERT_TRUE(connHandle->IsConnected());

    // 2. Inject a Secure CASE Session associated with this connection
    SessionHolder sessionHolder;
    EXPECT_SUCCESS(mSessionManager.InjectCaseSessionWithTestKey(sessionHolder, 1, 2, 1234, 5678, 1, peerAddr,
                                                                CryptoContext::SessionRole::kInitiator));

    // Manually link the TCP connection handle to the session
    sessionHolder->AsSecureSession()->SetTCPConnection(connHandle);

    // 3. Mark the session as DEFUNCT
    sessionHolder->AsSecureSession()->MarkAsDefunct();
    ASSERT_FALSE(sessionHolder->AsSecureSession()->IsActiveSession());

    // 4. Simulate TCP connection closure
    // Close TCP connection & release local handle
    connHandle->ForceDisconnect();
    connHandle.Release();

    // Drive IO to process TCP closure and session eviction logic
    mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(1), [&]() { return false; });

    // Verify the closed connection's slot was released.
    // With loopback and kMaxTcpActiveConnectionCount = 2,
    // a new connection needs both slots.
    // If the defunct secure session still retains the closed
    // connection handle, this reconnect will fail due to pool
    // exhaustion.
    ActiveTCPConnectionHandle secondConnHandle;
    EXPECT_SUCCESS(mTCP.TCPConnect(peerAddr, nullptr, secondConnHandle));

    mIOContext->DriveIOUntil(chip::System::Clock::Seconds16(5),
                             [&]() { return !secondConnHandle.IsNull() && secondConnHandle->IsConnected(); });
    ASSERT_FALSE(secondConnHandle.IsNull());
    ASSERT_TRUE(secondConnHandle->IsConnected());
}

// A message that fails to authenticate must not associate the session named in its header
// with the connection it arrived on, because closing that connection evicts the session.
TEST_F(TestTCPConnection, UndecryptableMessageDoesNotAssociateSessionWithConnection)
{
    uint16_t port;
    ASSERT_SUCCESS(InitTCP(port));
    ASSERT_SUCCESS(InitSessionManager());
    mSessionManager.SetMessageDelegate(this);

    Transport::PeerAddress peerAddr;
    ActiveTCPConnectionHandle connHandle;
    ASSERT_SUCCESS(ConnectToSelf(port, peerAddr, connHandle));

    SessionHolder initiator;
    SessionHolder responder;
    ASSERT_SUCCESS(InjectUdpSessionPair(mSessionManager, initiator, responder));

    EncryptedPacketBufferHandle prepared;
    ASSERT_SUCCESS(PrepareTestMessage(mSessionManager, initiator.Get().Value(), prepared));
    System::PacketBufferHandle msg = prepared.CastToWritable();
    ASSERT_FALSE(msg.IsNull());
    msg->Start()[msg->DataLength() - 1] = static_cast<uint8_t>(msg->Start()[msg->DataLength() - 1] ^ 0xff);

    ReceiveOverTCP(peerAddr, connHandle, std::move(msg));
    EXPECT_EQ(mReceivedMessageCount, 0u);
    EXPECT_TRUE(responder->AsSecureSession()->GetTCPConnection().IsNull());

    CloseConnection(connHandle);
    ASSERT_TRUE(bool(responder));
    EXPECT_TRUE(responder->AsSecureSession()->IsActiveSession());
}

// Evicting a session that is still being established aborts its pairing, so a message naming
// such a session must not associate it with a connection either.
TEST_F(TestTCPConnection, MessageForEstablishingSessionDoesNotAssociateSessionWithConnection)
{
    uint16_t port;
    ASSERT_SUCCESS(InitTCP(port));
    ASSERT_SUCCESS(InitSessionManager());
    mSessionManager.SetMessageDelegate(this);

    Transport::PeerAddress peerAddr;
    ActiveTCPConnectionHandle connHandle;
    ASSERT_SUCCESS(ConnectToSelf(port, peerAddr, connHandle));

    PairingSessionUnderTest pairingSession;
    ASSERT_SUCCESS(pairingSession.AllocateSecureSession(mSessionManager));
    Optional<SessionHandle> establishing = pairingSession.CopySecureSession();
    ASSERT_TRUE(establishing.HasValue());
    ASSERT_TRUE(establishing.Value()->AsSecureSession()->IsEstablishing());

    // Header naming the session, followed by bytes standing in for an encrypted payload and its MIC.
    const uint8_t kPayload[32]     = {};
    System::PacketBufferHandle msg = System::PacketBufferHandle::NewWithData(kPayload, sizeof(kPayload));
    ASSERT_FALSE(msg.IsNull());
    PacketHeader packetHeader;
    packetHeader.SetSessionId(establishing.Value()->AsSecureSession()->GetLocalSessionId()).SetMessageCounter(1);
    packetHeader.SetSessionType(Header::SessionType::kUnicastSession);
    ASSERT_SUCCESS(packetHeader.EncodeBeforeData(msg));

    ReceiveOverTCP(peerAddr, connHandle, std::move(msg));
    EXPECT_TRUE(establishing.Value()->AsSecureSession()->GetTCPConnection().IsNull());

    CloseConnection(connHandle);
    EXPECT_TRUE(establishing.Value()->AsSecureSession()->IsEstablishing());
    EXPECT_TRUE(pairingSession.CopySecureSession().HasValue());
}

// A resent copy of an accepted message authenticates, but anyone who captured it can resend it
// over a connection of their own, so it must not associate the session either.
TEST_F(TestTCPConnection, ReplayedMessageDoesNotAssociateSessionWithConnection)
{
    uint16_t port;
    ASSERT_SUCCESS(InitTCP(port));
    ASSERT_SUCCESS(InitSessionManager());
    mSessionManager.SetMessageDelegate(this);

    Transport::PeerAddress peerAddr;
    ActiveTCPConnectionHandle connHandle;
    ASSERT_SUCCESS(ConnectToSelf(port, peerAddr, connHandle));

    SessionHolder initiator;
    SessionHolder responder;
    ASSERT_SUCCESS(InjectUdpSessionPair(mSessionManager, initiator, responder));

    EncryptedPacketBufferHandle prepared;
    ASSERT_SUCCESS(PrepareTestMessage(mSessionManager, initiator.Get().Value(), prepared));
    ASSERT_FALSE(prepared.IsNull());

    EncryptedPacketBufferHandle firstDelivery = prepared.CloneData();
    ASSERT_FALSE(firstDelivery.IsNull());
    mSessionManager.OnMessageReceived(responder->AsSecureSession()->GetPeerAddress(), firstDelivery.CastToWritable());
    ASSERT_EQ(mReceivedMessageCount, 1u);

    EncryptedPacketBufferHandle replay = prepared.CloneData();
    ASSERT_FALSE(replay.IsNull());
    ReceiveOverTCP(peerAddr, connHandle, replay.CastToWritable());
    ASSERT_EQ(mReceivedMessageCount, 2u);
    EXPECT_EQ(mLastReceivedDuplicate, DuplicateMessage::Yes);
    EXPECT_TRUE(responder->AsSecureSession()->GetTCPConnection().IsNull());

    CloseConnection(connHandle);
    ASSERT_TRUE(bool(responder));
    EXPECT_TRUE(responder->AsSecureSession()->IsActiveSession());
}

TEST_F(TestTCPConnection, AuthenticatedMessageAssociatesSessionWithConnection)
{
    uint16_t port;
    ASSERT_SUCCESS(InitTCP(port));
    ASSERT_SUCCESS(InitSessionManager());
    mSessionManager.SetMessageDelegate(this);

    Transport::PeerAddress peerAddr;
    ActiveTCPConnectionHandle connHandle;
    ASSERT_SUCCESS(ConnectToSelf(port, peerAddr, connHandle));

    SessionHolder initiator;
    SessionHolder responder;
    ASSERT_SUCCESS(InjectUdpSessionPair(mSessionManager, initiator, responder));

    EncryptedPacketBufferHandle prepared;
    ASSERT_SUCCESS(PrepareTestMessage(mSessionManager, initiator.Get().Value(), prepared));
    ASSERT_FALSE(prepared.IsNull());

    ReceiveOverTCP(peerAddr, connHandle, prepared.CastToWritable());
    ASSERT_EQ(mReceivedMessageCount, 1u);
    EXPECT_EQ(static_cast<const void *>(responder->AsSecureSession()->GetTCPConnection()), static_cast<const void *>(connHandle));

    CloseConnection(connHandle);
    EXPECT_FALSE(bool(responder));
}

// A session associated with a TCP connection takes messages only from that connection, so a
// message that arrives on another connection is dropped even when it authenticates.
TEST_F(TestTCPConnection, MessageOverAnotherConnectionIsDropped)
{
    ASSERT_SUCCESS(InitSessionManager());
    mSessionManager.SetMessageDelegate(this);

    // The connection pool fits only one loopback connection, so both connections are allocated without sockets.
    using TCPAccess = Transport::TCPBaseTestAccess<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>;
    IPAddress addr;
    IPAddress::FromString("::1", addr);
    const Transport::PeerAddress sessionPeerAddr = Transport::PeerAddress::TCP(addr, CHIP_PORT);
    const Transport::PeerAddress otherPeerAddr   = Transport::PeerAddress::TCP(addr, CHIP_PORT + 1);

    Inet::TCPEndPointHandle sessionEndPoint;
    Inet::TCPEndPointHandle otherEndPoint;
    ASSERT_SUCCESS(mIOContext->GetTCPEndPointManager()->NewEndPoint(sessionEndPoint));
    ASSERT_SUCCESS(mIOContext->GetTCPEndPointManager()->NewEndPoint(otherEndPoint));
    auto sessionConnection =
        TCPAccess::AllocateConnection(mTCP, sessionEndPoint, sessionPeerAddr, Transport::TCPState::kConnecting);
    auto otherConnection = TCPAccess::AllocateConnection(mTCP, otherEndPoint, otherPeerAddr, Transport::TCPState::kConnecting);
    ASSERT_TRUE(sessionConnection);
    ASSERT_TRUE(otherConnection);

    SessionHolder initiator;
    SessionHolder responder;
    ASSERT_SUCCESS(InjectUdpSessionPair(mSessionManager, initiator, responder));

    EncryptedPacketBufferHandle first;
    ASSERT_SUCCESS(PrepareTestMessage(mSessionManager, initiator.Get().Value(), first));
    ReceiveOverTCP(sessionPeerAddr, sessionConnection.Handle(), first.CastToWritable());
    ASSERT_EQ(mReceivedMessageCount, 1u);
    ASSERT_EQ(static_cast<const void *>(responder->AsSecureSession()->GetTCPConnection()),
              static_cast<const void *>(sessionConnection.Handle()));

    EncryptedPacketBufferHandle second;
    ASSERT_SUCCESS(PrepareTestMessage(mSessionManager, initiator.Get().Value(), second));
    ReceiveOverTCP(otherPeerAddr, otherConnection.Handle(), second.CastToWritable());
    EXPECT_EQ(mReceivedMessageCount, 1u);
    EXPECT_EQ(static_cast<const void *>(responder->AsSecureSession()->GetTCPConnection()),
              static_cast<const void *>(sessionConnection.Handle()));
    EXPECT_TRUE(responder->AsSecureSession()->GetPeerAddress() == sessionPeerAddr);
}

} // namespace
