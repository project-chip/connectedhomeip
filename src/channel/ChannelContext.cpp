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
    return mExchangeManager->NewContext(GetReadyVars().mSession, delegate);
}

bool ChannelContext::MatchNodeId(NodeId nodeId)
{
    switch (mState)
    {
    case ChannelState::kPreparing:
        return nodeId == GetPrepareVars().mBuilder.GetPeerNodeId();
    case ChannelState::kReady: {
        auto state = mExchangeManager->GetSessionMgr()->GetPeerConnectionState(GetReadyVars().mSession);
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
        switch (GetPrepareVars().mBuilder.GetTransportPreference())
        {
        case ChannelBuilder::TransportPreference::kPreferConnectionOriented:
        case ChannelBuilder::TransportPreference::kConnectionOriented:
            return transport == Transport::Type::kTcp;
        case ChannelBuilder::TransportPreference::kConnectionless:
            return transport == Transport::Type::kUdp;
        }
        return false;
    case ChannelState::kReady: {
        auto state = mExchangeManager->GetSessionMgr()->GetPeerConnectionState(GetReadyVars().mSession);
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
    return mState == ChannelState::kPreparing && GetPrepareVars().mState == PrepareState::kCasePairing;
}

bool ChannelContext::MatchesSession(SecureSessionHandle session, SecureSessionMgr * ssm)
{
    switch (mState)
    {
    case ChannelState::kPreparing: {
        switch (GetPrepareVars().mState)
        {
        case PrepareState::kCasePairing: {
            auto state = ssm->GetPeerConnectionState(session);
            return (state->GetPeerNodeId() == GetPrepareVars().mBuilder.GetPeerNodeId() &&
                    state->GetPeerKeyID() == GetPrepareVars().mBuilder.GetPeerKeyID());
        }
        default:
            return false;
        }
    }
    case ChannelState::kReady:
        return GetReadyVars().mSession == session;
    default:
        return false;
    }
}

void ChannelContext::EnterPreparingState(const ChannelBuilder & builder)
{
    mState = ChannelState::kPreparing;

    mStateVars.Set<PrepareVars>();
    GetPrepareVars().mBuilder = builder;

    EnterAddressResolve();
}

void ChannelContext::ExitPreparingState() {}

// Address resolve
void ChannelContext::EnterAddressResolve()
{
    GetPrepareVars().mState = PrepareState::kAddressResolving;

    // Skip address resolve if the address is provided
    {
        auto addr = GetPrepareVars().mBuilder.GetForcePeerAddress();
        if (addr.HasValue())
        {
            GetPrepareVars().mAddress = addr.Value();
            ExitAddressResolve();
            // Only CASE session is supported
            EnterCasePairingState();
            return;
        }
    }

    // TODO: call mDNS Scanner::SubscribeNode after PR #4459 is ready
    // Scanner::RegisterScannerDelegate(this)
    // Scanner::SubscribeNode(GetPrepareVars().mBuilder.GetPeerNodeId())

    // The HandleNodeIdResolve may already have been called, recheck the state here before set up the timer
    if (mState == ChannelState::kPreparing && GetPrepareVars().mState == PrepareState::kAddressResolving)
    {
        System::Layer * layer = mExchangeManager->GetSessionMgr()->SystemLayer();
        layer->StartTimer(CHIP_CONFIG_NODE_ADDRESS_RESOLVE_TIMEOUT_MSECS, AddressResolveTimeout, this);
        Retain(); // Keep the pointer in the timer
    }
}

void ChannelContext::AddressResolveTimeout(System::Layer * aLayer, void * aAppState, CHIP_ERROR aError)
{
    ChannelContext * me = static_cast<ChannelContext *>(aAppState);
    me->AddressResolveTimeout();
    me->Release();
}

void ChannelContext::AddressResolveTimeout()
{
    if (mState != ChannelState::kPreparing)
        return;
    if (GetPrepareVars().mState != PrepareState::kAddressResolving)
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
        switch (GetPrepareVars().mState)
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
            GetPrepareVars().mAddressType = address.mAddressType;
            GetPrepareVars().mAddress     = address.mAddress.Value();
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

void ChannelContext::EnterCasePairingState()
{
    auto & prepare              = GetPrepareVars();
    prepare.mCasePairingSession = Platform::New<CASESession>();

    ExchangeContext * ctxt = mExchangeManager->NewContext(SecureSessionHandle(), prepare.mCasePairingSession);
    VerifyOrReturn(ctxt != nullptr);

    // TODO: currently only supports IP/UDP paring
    Transport::PeerAddress addr;
    addr.SetTransportType(Transport::Type::kUdp).SetIPAddress(prepare.mAddress);
    CHIP_ERROR err = prepare.mCasePairingSession->EstablishSession(addr, &prepare.mBuilder.GetOperationalCredentialSet(),
                                                                   prepare.mBuilder.GetPeerNodeId(),
                                                                   mExchangeManager->GetNextKeyId(), ctxt, this);
    if (err != CHIP_NO_ERROR)
    {
        ExitCasePairingState();
        ExitPreparingState();
        EnterFailedState(err);
    }
}

void ChannelContext::ExitCasePairingState()
{
    Platform::Delete(GetPrepareVars().mCasePairingSession);
}

void ChannelContext::OnSessionEstablishmentError(CHIP_ERROR error)
{
    if (mState != ChannelState::kPreparing)
        return;
    switch (GetPrepareVars().mState)
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
    switch (GetPrepareVars().mState)
    {
    case PrepareState::kCasePairing:
        ExitCasePairingState();
        GetPrepareVars().mState = PrepareState::kCasePairingDone;
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
    if (GetPrepareVars().mState != PrepareState::kCasePairingDone)
        return;

    ExitPreparingState();
    EnterReadyState(session);
}

void ChannelContext::EnterReadyState(SecureSessionHandle session)
{
    mState = ChannelState::kReady;
    mStateVars.Set<ReadyVars>(session);
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
    // Scanner::UnsubscribeNode(GetPrepareVars().mBuilder.GetPeerNodeId())
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
