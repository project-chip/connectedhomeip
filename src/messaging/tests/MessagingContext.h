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
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

namespace chip {
namespace Test {

/**
 * @brief The context of test cases for messaging layer. It wil initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class MessagingContext
{
public:
    MessagingContext() :
        mInitialized(false), mAliceAddress(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT + 1)),
        mBobAddress(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT)), mPairingAliceToBob(GetBobKeyId(), GetAliceKeyId()),
        mPairingBobToAlice(GetAliceKeyId(), GetBobKeyId())
    {}
    ~MessagingContext() { VerifyOrDie(mInitialized == false); }

    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init(nlTestSuite * suite, TransportMgrBase * transport, IOContext * io);

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    static Inet::IPAddress GetAddress()
    {
        Inet::IPAddress addr;
        Inet::IPAddress::FromString("::1", addr);
        return addr;
    }
    NodeId GetBobNodeId() const { return mBobNodeId; }
    NodeId GetAliceNodeId() const { return mAliceNodeId; }

    void SetBobNodeId(NodeId nodeId) { mBobNodeId = nodeId; }
    void SetAliceNodeId(NodeId nodeId) { mAliceNodeId = nodeId; }

    uint16_t GetBobKeyId() const { return mBobKeyId; }
    uint16_t GetAliceKeyId() const { return mAliceKeyId; }

    void SetBobKeyId(uint16_t id) { mBobKeyId = id; }
    void SetAliceKeyId(uint16_t id) { mAliceKeyId = id; }

    FabricIndex GetFabricIndex() const { return mSrcFabricIndex; }
    void SetFabricIndex(FabricIndex id)
    {
        mSrcFabricIndex  = id;
        mDestFabricIndex = id;
    }

    SessionManager & GetSecureSessionManager() { return mSessionManager; }
    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeManager; }
    secure_channel::MessageCounterManager & GetMessageCounterManager() { return mMessageCounterManager; }

    SessionHandle GetSessionBobToAlice();
    SessionHandle GetSessionAliceToBob();

    Messaging::ExchangeContext * NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate);

    Messaging::ExchangeContext * NewExchangeToAlice(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewExchangeToBob(Messaging::ExchangeDelegate * delegate);

    System::Layer & GetSystemLayer() { return mIOContext->GetSystemLayer(); }

private:
    bool mInitialized;
    SessionManager mSessionManager;
    Messaging::ExchangeManager mExchangeManager;
    secure_channel::MessageCounterManager mMessageCounterManager;
    IOContext * mIOContext;

    NodeId mBobNodeId    = 123654;
    NodeId mAliceNodeId  = 111222333;
    uint16_t mBobKeyId   = 1;
    uint16_t mAliceKeyId = 2;
    Transport::PeerAddress mAliceAddress;
    Transport::PeerAddress mBobAddress;
    SecurePairingUsingTestSecret mPairingAliceToBob;
    SecurePairingUsingTestSecret mPairingBobToAlice;
    FabricIndex mSrcFabricIndex  = 0;
    FabricIndex mDestFabricIndex = 0;
};

} // namespace Test
} // namespace chip
