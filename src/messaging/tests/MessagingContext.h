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

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
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
        mPeer(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT)),
        mPairingPeerToLocal(Optional<NodeId>::Value(GetSourceNodeId()), GetLocalKeyId(), GetPeerKeyId()),
        mPairingLocalToPeer(Optional<NodeId>::Value(GetDestinationNodeId()), GetPeerKeyId(), GetLocalKeyId())
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
    static constexpr NodeId GetSourceNodeId() { return 123654; }
    static constexpr NodeId GetDestinationNodeId() { return 111222333; }

    static constexpr uint16_t GetLocalKeyId() { return 1; }
    static constexpr uint16_t GetPeerKeyId() { return 2; }

    SecureSessionMgr & GetSecureSessionManager() { return mSecureSessionMgr; }
    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeManager; }

    Messaging::ExchangeContext * NewExchangeToPeer(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewExchangeToLocal(Messaging::ExchangeDelegate * delegate);

private:
    SecureSessionMgr mSecureSessionMgr;
    Messaging::ExchangeManager mExchangeManager;

    Optional<Transport::PeerAddress> mPeer;
    SecurePairingUsingTestSecret mPairingPeerToLocal;
    SecurePairingUsingTestSecret mPairingLocalToPeer;
};

} // namespace Test
} // namespace chip
