/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the internal classes used by CHIP Channel.
 */

#pragma once

#include <lib/core/ReferenceCounted.h>
#include <lib/mdns/platform/Mdns.h>
#include <messaging/Channel.h>
#include <transport/PeerConnectionState.h>
#include <transport/SecureSessionMgr.h>

namespace chip {
namespace Messaging {

class ExchangeManager;
class ChannelContext;

class ChannelContextDeletor
{
public:
    static void Release(ChannelContext * context);
};

class ChannelContext : public ReferenceCounted<ChannelContext, ChannelContextDeletor>, public SecurePairingSessionDelegate
{
public:
    ChannelContext(ExchangeManager * exchangeManager) : mState(ChannelState::kChanneState_None), mExchangeManager(exchangeManager)
    {}

    void Start(const ChannelBuilder & builder);
    ExchangeContext * NewExchange(ExchangeDelegate * delegate);

    ChannelState GetState() const { return mState; }

    bool MatchesPaseParingSessoin(NodeId nodeId);
    bool MatchesSession(SecureSessionHandle session, SecureSessionMgr * ssm);
    bool MatchesBuilder(const ChannelBuilder & builder, SecureSessionMgr * ssm);

    // events of ResolveDelegate, propagated from ExchangeManager
    void HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, const Mdns::MdnsService & address);

    // events of SecureSessionManager, propagated from ExchangeManager
    void OnNewConnection(SecureSessionHandle session);
    void OnConnectionExpired(SecureSessionHandle session);

    CHIP_ERROR HandlePairingMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                    System::PacketBufferHandle && msg);
    CHIP_ERROR SendPairingMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                  System::PacketBufferHandle msgIn) override;
    void OnPairingError(CHIP_ERROR error) override;
    void OnPairingComplete() override;

private:
    friend class ChannelContextDeletor;
    friend class ChannelHandle;

    ChannelState mState;
    ExchangeManager * mExchangeManager;

    enum PrepareState
    {
        kPrepareState_WaitingForInterface,
        kPrepareState_AddressResolving,
        kPrepareState_PaseParing,
        kPrepareState_PaseParingDone,
        kPrepareState_CaseParing,
    };

    union
    {
        // mPreparing is pretty big, consider move it outside
        struct
        {
            PrepareState mState;
            Inet::InterfaceId mInterface;
            Inet::IPAddressType mAddressType;
            Inet::IPAddress mAddress;
            SecurePairingSession * mPairingSession;
            ChannelBuilder mBuilder;
        } mPreparing;
        struct
        {
            Inet::InterfaceId mInterface;
            SecureSessionHandle mSession;
        } mReady;
    };

    // State machine functions
    void EnterPreparingState(const ChannelBuilder & builder);
    void ExitPreparingState();

    void EnterReadyState(SecureSessionHandle session);
    void ExitReadyState();

    void EnterFailedState(CHIP_ERROR error);
    void EnterClosedState();

    // Preparing sub-states
    void EnterAddressResolve();
    static void AddressResolveTimeout(System::Layer * aLayer, void * aAppState, System::Error aError);
    void AddressResolveTimeout();
    void ExitAddressResolve() {}

    void EnterPaseParingState();
    void ExitPaseParingState() {}

    void EnterCaseParingState()
    { /* not implemented */
    }
    void ExitCaseParingState()
    { /* not implemented */
    }
};

class ChannelContextHandleAssociation
{
public:
    ChannelContextHandleAssociation(ChannelContext * channelContext, ChannelDelegate * channelDelegate) :
        mChannelContext(channelContext), mChannelDelegate(channelDelegate)
    {
        mChannelContext->Retain();
    }
    ~ChannelContextHandleAssociation() { mChannelContext->Release(); }

private:
    friend class ExchangeManager;
    friend class ChannelHandle;
    ChannelContext * mChannelContext;
    ChannelDelegate * mChannelDelegate;
};

} // namespace Messaging
} // namespace chip
