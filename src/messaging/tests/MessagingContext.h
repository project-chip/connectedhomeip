/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#pragma once

#include <credentials/CHIPOperationalCredentials.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <transport/AdminPairingTable.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

namespace chip {
namespace Test {

/**
 * @brief The context of test cases for messaging layer. It wil initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class MessagingContext : public IOContext
{
public:
    MessagingContext() :
        mPeer(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT)), mPairingPeerToLocal(GetLocalKeyId(), GetPeerKeyId()),
        mPairingLocalToPeer(GetPeerKeyId(), GetLocalKeyId())
    {}

    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init(nlTestSuite * suite, TransportMgrBase * transport);

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    static Inet::IPAddress GetAddress()
    {
        Inet::IPAddress addr;
        Inet::IPAddress::FromString("127.0.0.1", addr);
        return addr;
    }
    NodeId GetSourceNodeId() const { return mSourceNodeId; }
    NodeId GetDestinationNodeId() const { return mDestinationNodeId; }

    void SetSourceNodeId(NodeId nodeId) { mSourceNodeId = nodeId; }
    void SetDestinationNodeId(NodeId nodeId) { mDestinationNodeId = nodeId; }

    uint16_t GetLocalKeyId() const { return mLocalKeyId; }
    uint16_t GetPeerKeyId() const { return mPeerKeyId; }

    void SetLocalKeyId(uint16_t id) { mLocalKeyId = id; }
    void SetPeerKeyId(uint16_t id) { mPeerKeyId = id; }

    uint16_t GetAdminId() const { return mSrcAdminId; }
    void SetAdminId(Transport::AdminId id)
    {
        mSrcAdminId  = id;
        mDestAdminId = id;
    }

    SecureSessionMgr & GetSecureSessionManager() { return mSecureSessionMgr; }
    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeManager; }
    secure_channel::MessageCounterManager & GetMessageCounterManager() { return mMessageCounterManager; }

    SecureSessionHandle GetSessionLocalToPeer();
    SecureSessionHandle GetSessionPeerToLocal();

    Messaging::ExchangeContext * NewExchangeToPeer(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewExchangeToLocal(Messaging::ExchangeDelegate * delegate);

    Credentials::OperationalCredentialSet & GetOperationalCredentialSet() { return mOperationalCredentialSet; }

private:
    SecureSessionMgr mSecureSessionMgr;
    Messaging::ExchangeManager mExchangeManager;
    secure_channel::MessageCounterManager mMessageCounterManager;

    NodeId mSourceNodeId      = 123654;
    NodeId mDestinationNodeId = 111222333;
    uint16_t mLocalKeyId      = 1;
    uint16_t mPeerKeyId       = 2;
    Optional<Transport::PeerAddress> mPeer;
    SecurePairingUsingTestSecret mPairingPeerToLocal;
    SecurePairingUsingTestSecret mPairingLocalToPeer;
    Transport::AdminPairingTable mAdmins;
    Transport::AdminId mSrcAdminId  = 0;
    Transport::AdminId mDestAdminId = 1;
    Credentials::OperationalCredentialSet mOperationalCredentialSet;
};

} // namespace Test
} // namespace chip
