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
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/TCP.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

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
    {}

protected:
    static IOContext * mIOContext;
    TCPImpl mTCP;
    TransportMgrBase mTransportMgrBase;
    SessionManager mSessionManager;
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

        return mSessionManager.Init(&mIOContext->GetSystemLayer(), &mTransportMgrBase, &mMessageCounterManager, &mDeviceStorage,
                                    &mFabricTable, mSessionKeystore);
    }
};

IOContext * TestTCPConnection::mIOContext = nullptr;

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

} // namespace
