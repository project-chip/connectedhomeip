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

#include <variant>

#include <lib/core/ReferenceCounted.h>
#include <lib/mdns/platform/Mdns.h>
#include <messaging/Channel.h>
#include <transport/CASESession.h>
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

/**
 * @brief
 *  The object of the class holds all state of a channel. It is a state machine, with following states:
 *
 *  N: None, the initial state
 *  P: Preparing
 *  R: Ready, the channel is ready to use
 *  C: Closed, the channel is closed
 *  F: Failed, the channel is failed
 *
 *    +---+   +---+   +---+   +---+
 *    | N |-->| P |-->| R |-->| C |
 *    +---+   +---+   +---+   +---+
 *              |       |
 *              |       |     +---+
 *              +-------+---->| F |
 *                            +---+
 *
 *  Note: The state never goes back, when a channel is failed, it can't be reset or fixed. The application must create a
 *  new channel to replace the failed channel
 *
 *  Preparing Substates:
 *    A: AddressResolving, use mDNS to resolve the node address
 *    P: PasePairing, do SPAKE2 key exchange
 *    PD: PasePairingDone, wait for OnNewConnection from SecureSessionManager
 *    C: CasePairing, do SIGMA key exchange
 *    CD: CasePairingDone, wait for OnNewConnection from SecureSessionManager
 *
 *                    +---+   +----+
 *                 +->| P |-->| PD |--+
 *  /---\   +---+  |  +---+   +----+  |   /---\
 *  |   |-->| A |--+                  +-->| O |
 *  \---/   +---+  |  +---+   +----+  |   \---/
 *                 +->| C |-->| CD |--+
 *                    +---+   +----+
 */
class ChannelContext : public ReferenceCounted<ChannelContext, ChannelContextDeletor>, public SessionEstablishmentDelegate
{
public:
    ChannelContext(ExchangeManager * exchangeManager) : mState(ChannelState::kNone), mExchangeManager(exchangeManager) {}

    void Start(const ChannelBuilder & builder);

    /*
     * @brief
     *  Create a new exchange on the channel.
     *
     * @pre GetState() == ChannelState::kReady
     */
    ExchangeContext * NewExchange(ExchangeDelegate * delegate);

    ChannelState GetState() const { return mState; }

    bool MatchNodeId(NodeId nodeId);
    bool MatchTransport(Transport::Type transport);
    bool MatchTransportPreference(ChannelBuilder::TransportPreference transport);
    bool MatchSessionType(ChannelBuilder::SessionType type);
    bool MatchCaseParameters();

    bool IsPasePairing();

    bool MatchesBuilder(const ChannelBuilder & builder);
    bool MatchesSession(SecureSessionHandle session, SecureSessionMgr * ssm);

    // events of ResolveDelegate, propagated from ExchangeManager
    void HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, const Mdns::MdnsService & address);

    // events of SecureSessionManager, propagated from ExchangeManager
    void OnNewConnection(SecureSessionHandle session);
    void OnConnectionExpired(SecureSessionHandle session);

    // Pairing callbacks
    CHIP_ERROR HandlePairingMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                    System::PacketBufferHandle && msg);
    CHIP_ERROR SendSessionEstablishmentMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                               System::PacketBufferHandle msgIn) override;
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished() override;

private:
    friend class ChannelContextDeletor;
    friend class ChannelHandle;

    ChannelState mState;
    ExchangeManager * mExchangeManager;

    enum class PrepareState
    {
        kAddressResolving,
        kPasePairing,
        kPasePairingDone,
        kCasePairing,
    };

    union StateVars
    {
        StateVars() {}

        // mPreparing is pretty big, consider move it outside
        struct PrepareVars
        {
            PrepareState mState;
            Inet::IPAddressType mAddressType;
            Inet::IPAddress mAddress;
            union Session
            {
                PASESession * mPasePairingSession;
                CASESession * mCasePairingSession;
            } mSession;
            ChannelBuilder mBuilder;
        } mPreparing;

        struct ReadyVars
        {
            SecureSessionHandle mSession;
        } mReady;
    } mStateVars;

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

    void EnterPasePairingState();
    void ExitPasePairingState();

    void EnterCasePairingState();
    void ExitCasePairingState();
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
