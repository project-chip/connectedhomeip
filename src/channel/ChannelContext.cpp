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

#include <channel/Channel.h>
#include <channel/ChannelContext.h>
#include <channel/Manager.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Messaging {

void ChannelContextDeletor::Release(ChannelContext * context)
{
    context->mChannelManager->ReleaseChannelContext(context);
}

void ChannelContext::Start(const ChannelBuilder & builder)
{
    if (mState != ChannelState::kNone)
        return;
    EnterPreparingState(builder);
}

ExchangeContext * ChannelContext::NewExchange(ExchangeDelegate * delegate)
{
    assert(GetState() == ChannelState::kReady);
    return mExchangeManager->NewContext(mStateVars.mReady.mSession, delegate);
}

bool ChannelContext::MatchNodeId(NodeId nodeId)
{
    switch (mState)
    {
    case ChannelState::kPreparing:
        return nodeId == mStateVars.mPreparing.mBuilder.GetPeerNodeId();
    case ChannelState::kReady: {
        auto state = mExchangeManager->GetSessionMgr()->GetPeerConnectionState(mStateVars.mReady.mSession);
        if (state == nullptr)
            return false;
        return nodeId == state->GetPeerNodeId();
    }
    default:
        return false;
    }
}

bool ChannelContext::MatchTransport(Transport::Type transport)
{
    switch (mState)
    {
    case ChannelState::kPreparing:
        switch (mStateVars.mPreparing.mBuilder.GetTransportPreference())
        {
        case ChannelBuilder::TransportPreference::kPreferConnectionOriented:
        case ChannelBuilder::TransportPreference::kConnectionOriented:
            return transport == Transport::Type::kTcp;
        case ChannelBuilder::TransportPreference::kConnectionless:
            return transport == Transport::Type::kUdp;
        }
        return false;
    case ChannelState::kReady: {
        auto state = mExchangeManager->GetSessionMgr()->GetPeerConnectionState(mStateVars.mReady.mSession);
        if (state == nullptr)
            return false;
        return transport == state->GetPeerAddress().GetTransportType();
    }
    default:
        return false;
    }
}

bool ChannelContext::MatchTransportPreference(ChannelBuilder::TransportPreference transport)
{
    switch (transport)
    {
    case ChannelBuilder::TransportPreference::kPreferConnectionOriented:
    case ChannelBuilder::TransportPreference::kConnectionOriented:
        return MatchTransport(Transport::Type::kTcp);
    case ChannelBuilder::TransportPreference::kConnectionless:
        return MatchTransport(Transport::Type::kUdp);
    }
    return false;
}

bool ChannelContext::MatchCaseParameters()
{
    // TODO: not supported yet, should compare CASE parameters here, and return false if doesn't match
    return true;
}

bool ChannelContext::MatchesBuilder(const ChannelBuilder & builder)
{
    // Channel is identified by {node id, {case key id}, network interface, tcp/udp}
    // PASE is not supported yet
    // Network interface is not supported yet
    // Channel is reused if builder parameters are matching an existing channel
    if (!MatchNodeId(builder.GetPeerNodeId()))
        return false;
    if (!MatchCaseParameters())
        return false;

    return MatchTransportPreference(builder.GetTransportPreference());
}

bool ChannelContext::IsCasePairing()
{
    return mState == ChannelState::kPreparing && mStateVars.mPreparing.mState == PrepareState::kCasePairing;
}

bool ChannelContext::MatchesSession(SecureSessionHandle session, SecureSessionMgr * ssm)
{
    switch (mState)
    {
    case ChannelState::kPreparing: {
        switch (mStateVars.mPreparing.mState)
        {
        case PrepareState::kCasePairing: {
            auto state = ssm->GetPeerConnectionState(session);
            return (state->GetPeerNodeId() == mStateVars.mPreparing.mBuilder.GetPeerNodeId() &&
                    state->GetPeerKeyID() == mStateVars.mPreparing.mBuilder.GetPeerKeyID());
        }
        default:
            return false;
        }
    }
    case ChannelState::kReady:
        return mStateVars.mReady.mSession == session;
    default:
        return false;
    }
}

void ChannelContext::EnterPreparingState(const ChannelBuilder & builder)
{
    mState                         = ChannelState::kPreparing;
    mStateVars.mPreparing.mBuilder = builder;

    EnterAddressResolve();
}

void ChannelContext::ExitPreparingState() {}

// Address resolve
void ChannelContext::EnterAddressResolve()
{
    mStateVars.mPreparing.mState = PrepareState::kAddressResolving;

    // Skip address resolve if the address is provided
    {
        auto addr = mStateVars.mPreparing.mBuilder.GetForcePeerAddress();
        if (addr.HasValue())
        {
            mStateVars.mPreparing.mAddress = addr.Value();
            ExitAddressResolve();
            // Only CASE session is supported
            EnterCasePairingState();
            return;
        }
    }

    // TODO: call mDNS Scanner::SubscribeNode after PR #4459 is ready
    // Scanner::RegisterScannerDelegate(this)
    // Scanner::SubscribeNode(mStateVars.mPreparing.mBuilder.GetPeerNodeId())

    // The HandleNodeIdResolve may already have been called, recheck the state here before set up the timer
    if (mState == ChannelState::kPreparing && mStateVars.mPreparing.mState == PrepareState::kAddressResolving)
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
    if (mState != ChannelState::kPreparing)
        return;
    if (mStateVars.mPreparing.mState != PrepareState::kAddressResolving)
        return;

    ExitAddressResolve();
    ExitPreparingState();
    EnterFailedState(CHIP_ERROR_PEER_NODE_NOT_FOUND);
}

void ChannelContext::HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, const Mdns::MdnsService & address)
{
    switch (mState)
    {
    case ChannelState::kReady: {
        if (error != CHIP_NO_ERROR)
        {
            // Ignore mDNS fail in ready state
            return;
        }

        // TODO: adjust peer address, secure session manager is not able to change peer address.
        return;
    }
    case ChannelState::kPreparing: {
        switch (mStateVars.mPreparing.mState)
        {
        case PrepareState::kAddressResolving: {
            if (error != CHIP_NO_ERROR)
            {
                ExitAddressResolve();
                ExitPreparingState();
                EnterFailedState(error);
                return;
            }

            if (!address.mAddress.HasValue())
                return;
            mStateVars.mPreparing.mAddressType = address.mAddressType;
            mStateVars.mPreparing.mAddress     = address.mAddress.Value();
            ExitAddressResolve();
            EnterCasePairingState();
            return;
        }
        case PrepareState::kCasePairing:
        case PrepareState::kCasePairingDone:
            return;
        }
        return;
    }
    case ChannelState::kNone:
    case ChannelState::kClosed:
    case ChannelState::kFailed:
        return;
    }
}

// Session establishment
CHIP_ERROR ChannelContext::SendSessionEstablishmentMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                                           System::PacketBufferHandle msgIn)
{
    return mExchangeManager->GetSessionMgr()->GetTransportManager()->SendMessage(header, peerAddress, std::move(msgIn));
}

CHIP_ERROR ChannelContext::HandlePairingMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                System::PacketBufferHandle && msg)
{
    if (IsCasePairing())
        return mStateVars.mPreparing.mCasePairingSession->HandlePeerMessage(packetHeader, peerAddress, std::move(msg));
    return CHIP_ERROR_INCORRECT_STATE;
}

void ChannelContext::EnterCasePairingState()
{
    mStateVars.mPreparing.mState              = PrepareState::kCasePairing;
    mStateVars.mPreparing.mCasePairingSession = Platform::New<CASESession>();
    // TODO: currently only supports IP/UDP paring
    Transport::PeerAddress addr;
    addr.SetTransportType(Transport::Type::kUdp).SetIPAddress(mStateVars.mPreparing.mAddress);
    CHIP_ERROR err = mStateVars.mPreparing.mCasePairingSession->EstablishSession(
        addr, &mStateVars.mPreparing.mBuilder.GetOperationalCredentialSet(),
        Optional<NodeId>::Value(mExchangeManager->GetSessionMgr()->GetLocalNodeId()),
        mStateVars.mPreparing.mBuilder.GetPeerNodeId(), mExchangeManager->GetNextKeyId(), this);
    if (err != CHIP_NO_ERROR)
    {
        ExitCasePairingState();
        ExitPreparingState();
        EnterFailedState(err);
    }
}

void ChannelContext::ExitCasePairingState()
{
    Platform::Delete(mStateVars.mPreparing.mCasePairingSession);
}

void ChannelContext::OnSessionEstablishmentError(CHIP_ERROR error)
{
    if (mState != ChannelState::kPreparing)
        return;
    switch (mStateVars.mPreparing.mState)
    {
    case PrepareState::kCasePairing:
        ExitCasePairingState();
        ExitPreparingState();
        EnterFailedState(error);
        return;
    default:
        return;
    }
}

void ChannelContext::OnSessionEstablished()
{
    if (mState != ChannelState::kPreparing)
        return;
    switch (mStateVars.mPreparing.mState)
    {
    case PrepareState::kCasePairing:
        ExitCasePairingState();
        mStateVars.mPreparing.mState = PrepareState::kCasePairingDone;
        // TODO: current CASE paring session API doesn't show how to derive a secure session
        return;
    default:
        return;
    }
}

void ChannelContext::OnNewConnection(SecureSessionHandle session)
{
    if (mState != ChannelState::kPreparing)
        return;
    if (mStateVars.mPreparing.mState != PrepareState::kCasePairingDone)
        return;

    ExitPreparingState();
    EnterReadyState(session);
}

void ChannelContext::EnterReadyState(SecureSessionHandle session)
{
    mState = ChannelState::kReady;

    mStateVars.mReady.mSession = session;
    mChannelManager->NotifyChannelEvent(this, [](ChannelDelegate * delegate) { delegate->OnEstablished(); });
}

void ChannelContext::OnConnectionExpired(SecureSessionHandle session)
{
    if (mState != ChannelState::kReady)
        return;

    ExitReadyState();
    EnterClosedState();
}

void ChannelContext::ExitReadyState()
{
    // TODO: close sesure session
    // Currently SecureSessionManager doesn't provide an interface to close a session

    // TODO: call mDNS Scanner::UnubscribeNode after PR #4459 is ready
    // Scanner::UnsubscribeNode(mStateVars.mPreparing.mBuilder.GetPeerNodeId())
}

void ChannelContext::EnterFailedState(CHIP_ERROR error)
{
    mState = ChannelState::kFailed;
    mChannelManager->NotifyChannelEvent(this, [error](ChannelDelegate * delegate) { delegate->OnFail(error); });
}

void ChannelContext::EnterClosedState()
{
    mState = ChannelState::kClosed;
    mChannelManager->NotifyChannelEvent(this, [](ChannelDelegate * delegate) { delegate->OnClosed(); });
}

} // namespace Messaging
} // namespace chip
