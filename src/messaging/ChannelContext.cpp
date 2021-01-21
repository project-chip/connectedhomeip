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

#include <messaging/Channel.h>
#include <messaging/ChannelContext.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Messaging {

void ChannelContextDeletor::Release(ChannelContext * context)
{
    context->mExchangeManager->ReleaseChannelContext(context);
}

void ChannelContext::Start(const ChannelBuilder & builder)
{
    if (mState != ChannelState::kChanneState_None)
        return;
    EnterPreparingState(builder);
}

ExchangeContext * ChannelContext::NewExchange(ExchangeDelegate * delegate)
{
    if (GetState() != ChannelState::kChanneState_Ready)
        return nullptr;
    return mExchangeManager->NewContext(mReady.mSession, delegate);
}

bool ChannelContext::MatchesBuilder(const ChannelBuilder & builder, SecureSessionMgr * ssm)
{
    // Channel is identified by {node id, {pase/case, key id}, network interface, tcp/udp}
    // Channel is reused if builder parameters are matching an existing channel
    switch (mState)
    {
    case ChannelState::kChanneState_Closed:
    case ChannelState::kChanneState_None:
    case ChannelState::kChanneState_Failed:
        return false;
    case ChannelState::kChanneState_Preparing: {
        if (builder.GetPeerNodeId() != mPreparing.mBuilder.GetPeerNodeId())
            return false;

        switch (builder.GetSessionType())
        {
        case ChannelBuilder::SessionType::kSession_PASE:
            // only one PASE sessoin per node is supported
            return true;
        case ChannelBuilder::SessionType::kSession_CASE: {
            // TODO: compare network interface id and tcp/udp type
            return builder.GetPeerKeyID() == mPreparing.mBuilder.GetPeerKeyID();
        }
        }
    }
        return false;
    case ChannelState::kChanneState_Ready:
        // TODO
        return false;
    default:
        assert(false);
        return false;
    }
}

bool ChannelContext::MatchesPaseParingSessoin(NodeId nodeId)
{
    return mState == ChannelState::kChanneState_Preparing && mPreparing.mState == PrepareState::kPrepareState_PaseParing &&
        nodeId == mPreparing.mBuilder.GetPeerNodeId();
}

bool ChannelContext::MatchesSession(SecureSessionHandle session, SecureSessionMgr * ssm)
{
    switch (mState)
    {
    case ChannelState::kChanneState_Closed:
    case ChannelState::kChanneState_None:
    case ChannelState::kChanneState_Failed:
        return false;
    case ChannelState::kChanneState_Preparing: {
        switch (mPreparing.mState)
        {
        case kPrepareState_WaitingForInterface:
        case kPrepareState_AddressResolving:
        case kPrepareState_PaseParing:
            return false;
        case kPrepareState_PaseParingDone: {
            auto state = ssm->GetPeerConnectionState(session);
            if (state->GetPeerNodeId() != mPreparing.mBuilder.GetPeerNodeId())
                return false;
            // only one PASE sessoin per node is supported
            return true;
        }
        case kPrepareState_CaseParing: {
            auto state = ssm->GetPeerConnectionState(session);
            return (state->GetPeerNodeId() != mPreparing.mBuilder.GetPeerNodeId() &&
                    state->GetPeerKeyID() == mPreparing.mBuilder.GetPeerKeyID());
        }
        }
    }
        return false;
    case ChannelState::kChanneState_Ready:
        return mReady.mSession == session;
    default:
        assert(false);
        return false;
    }
}

void ChannelContext::EnterPreparingState(const ChannelBuilder & builder)
{
    mState              = ChannelState::kChanneState_Preparing;
    mPreparing.mBuilder = builder;

    // TODO: Skip waiting for interface, it is not clear how to wait for a network interface for now.
    mPreparing.mInterface = INET_NULL_INTERFACEID;

    EnterAddressResolve();
}

void ChannelContext::ExitPreparingState() {}

// Address resolve
void ChannelContext::EnterAddressResolve()
{
    mPreparing.mState = PrepareState::kPrepareState_AddressResolving;

#ifndef NDEBUG
    {
        auto addr = mPreparing.mBuilder.GetHintPeerAddress();
        if (addr.HasValue())
        {
            mPreparing.mAddress = addr.Value();
            ExitAddressResolve();
            switch (mPreparing.mBuilder.GetSessionType())
            {
            case ChannelBuilder::SessionType::kSession_PASE:
                EnterPaseParingState();
                break;
            case ChannelBuilder::SessionType::kSession_CASE:
                EnterCaseParingState();
                break;
            }
            return;
        }
    }
#endif // NDEBUG

    // TODO: mDNS is working in progress
    // TODO: mDNS API will support interface id in the future
    // TODO: mDNS API will provide a subscription interface
    // TODO: Register mDNS, query address for the node id
    // Mdns::DiscoveryManager::GetInstance().ResolveNodeId(mPreparing.mBuilder.GetPeerNodeId(), 0);

    // The HandleNodeIdResolve may already have been called, recheck the state here before set up the timer
    if (mState == ChannelState::kChanneState_Preparing && mPreparing.mState == kPrepareState_AddressResolving)
    {
        System::Layer * layer = mExchangeManager->GetSessionMgr()->SystemLayer();
        layer->StartTimer(CHIP_CONFIG_NODE_ADDRESS_RESOLVE_TIMEOUT_MSECS, AddressResolveTimeout, this);
        Retain(); // Keep the pointer in the timer
    }
}

void ChannelContext::AddressResolveTimeout(System::Layer * aLayer, void * aAppState, System::Error aError)
{
    ChannelContext * me = static_cast<ChannelContext *>(aAppState);
    me->AddressResolveTimeout();
    me->Release();
}

void ChannelContext::AddressResolveTimeout()
{
    if (mState != ChannelState::kChanneState_Preparing)
        return;
    if (mPreparing.mState != PrepareState::kPrepareState_AddressResolving)
        return;

    ExitAddressResolve();
    ExitPreparingState();
    EnterFailedState(CHIP_ERROR_PEER_NODE_NOT_FOUND);
}

void ChannelContext::HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, const Mdns::MdnsService & address)
{
    switch (mState)
    {
    case ChannelState::kChanneState_Ready: {
        auto peer = mExchangeManager->GetSessionMgr()->GetPeerConnectionState(mReady.mSession);
        if (peer->GetPeerNodeId() != nodeId || mReady.mInterface != address.mInterface)
            return;

        if (error != CHIP_NO_ERROR)
        {
            // Ignore mDNS fail in ready state
            return;
        }

        // TODO: adjust peer address
    }
        return;
    case ChannelState::kChanneState_Preparing: {
        // Skip responses which are not related with this context
        if (nodeId != mPreparing.mBuilder.GetPeerNodeId())
            return;
        if (mPreparing.mInterface != INET_NULL_INTERFACEID && mPreparing.mInterface != address.mInterface)
            return;

        switch (mPreparing.mState)
        {
        case kPrepareState_WaitingForInterface:
            return;
        case kPrepareState_AddressResolving: {
            if (error != CHIP_NO_ERROR)
            {
                ExitAddressResolve();
                ExitPreparingState();
                EnterFailedState(error);
                return;
            }

            if (!address.mAddress.HasValue())
                return;
            mPreparing.mAddressType = address.mAddressType;
            mPreparing.mAddress     = address.mAddress.Value();
            ExitAddressResolve();
            switch (mPreparing.mBuilder.GetSessionType())
            {
            case ChannelBuilder::SessionType::kSession_PASE:
                EnterPaseParingState();
                break;
            case ChannelBuilder::SessionType::kSession_CASE:
                EnterCaseParingState();
                break;
            }
        }
            return;
        case kPrepareState_PaseParing:
        case kPrepareState_PaseParingDone:
        case kPrepareState_CaseParing:
            return;
        }
    }
        return;
    case ChannelState::kChanneState_None:
    case ChannelState::kChanneState_Closed:
    case ChannelState::kChanneState_Failed:
        return;
    }
}

// Session establishment
CHIP_ERROR ChannelContext::SendPairingMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                              System::PacketBufferHandle msgIn)
{
    return mExchangeManager->GetTransportManager()->SendMessage(header, peerAddress, std::move(msgIn));
}

CHIP_ERROR ChannelContext::HandlePairingMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                System::PacketBufferHandle && msg)
{
    return mPreparing.mPairingSession->HandlePeerMessage(packetHeader, peerAddress, std::move(msg));
}

void ChannelContext::EnterPaseParingState()
{
    mPreparing.mState          = PrepareState::kPrepareState_PaseParing;
    mPreparing.mPairingSession = chip::Platform::New<SecurePairingSession>();
    // TODO: currently only supports IP/UDP paring
    Transport::PeerAddress addr;
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(mPreparing.mAddress);
    mPreparing.mPairingSession->Pair(addr, mPreparing.mBuilder.GetPeerSetUpPINCode(),
                                     Optional<NodeId>(mExchangeManager->GetLocalNodeId()), mPreparing.mBuilder.GetPeerNodeId(),
                                     mExchangeManager->GetNextPaseKeyId(), this);
}

void ChannelContext::OnPairingError(CHIP_ERROR error)
{
    if (mState != ChannelState::kChanneState_Preparing)
        return;
    if (mPreparing.mState != PrepareState::kPrepareState_PaseParing)
        return;
    ExitPaseParingState();
    ExitPreparingState();
    mState = ChannelState::kChanneState_Failed;
}

void ChannelContext::OnPairingComplete()
{
    if (mState != ChannelState::kChanneState_Preparing)
        return;
    if (mPreparing.mState != PrepareState::kPrepareState_PaseParing)
        return;

    ExitPaseParingState();
    mPreparing.mState = PrepareState::kPrepareState_PaseParingDone;

    Transport::PeerAddress addr;
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(mPreparing.mAddress);
    // This will trigger OnNewConnection callback from SecureSessionManager
    mExchangeManager->GetSessionMgr()->NewPairing(Optional<Transport::PeerAddress>(addr), mPreparing.mBuilder.GetPeerNodeId(),
                                                  mPreparing.mPairingSession);
}

void ChannelContext::OnNewConnection(SecureSessionHandle session)
{
    if (mState != ChannelState::kChanneState_Preparing)
        return;
    if (mPreparing.mState != PrepareState::kPrepareState_PaseParingDone)
        return;

    ExitPreparingState();
    EnterReadyState(session);
}

void ChannelContext::EnterReadyState(SecureSessionHandle session)
{
    mState = ChannelState::kChanneState_Ready;

    mReady.mSession = session;
    mExchangeManager->NotifyChannelEvent(this, [](ChannelDelegate * delegate) { delegate->OnEstablished(); });
}

void ChannelContext::OnConnectionExpired(SecureSessionHandle session)
{
    if (mState != ChannelState::kChanneState_Ready)
        return;

    ExitReadyState();
    EnterClosedState();
}

void ChannelContext::ExitReadyState()
{
    // TODO: close sesure session
    // TODO: unsubscribe node from mDNS manager
}

void ChannelContext::EnterFailedState(CHIP_ERROR error)
{
    mState = ChannelState::kChanneState_Failed;
    mExchangeManager->NotifyChannelEvent(this, [error](ChannelDelegate * delegate) { delegate->OnFail(error); });
}

void ChannelContext::EnterClosedState()
{
    mState = ChannelState::kChanneState_Closed;
    mExchangeManager->NotifyChannelEvent(this, [](ChannelDelegate * delegate) { delegate->OnClosed(); });
}

} // namespace Messaging
} // namespace chip
