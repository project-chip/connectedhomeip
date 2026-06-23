/*
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

#include <access/AuthMode.h>
#include <transport/SecureSession.h>
#include <transport/SecureSessionTable.h>

#include <cstring>

namespace chip {
namespace Transport {

void SecureSessionDeleter::Release(SecureSession * entry)
{
    entry->mTable.ReleaseSession(entry);
}

void SecureSession::Activate(const ScopedNodeId & localNode, const ScopedNodeId & peerNode, CATValues peerCATs,
                             uint16_t peerSessionId, const SessionParameters & sessionParameters)
{
    VerifyOrDie(mState == State::kEstablishing);
    VerifyOrDie(peerNode.GetFabricIndex() == localNode.GetFabricIndex());

    // PASE sessions must always start unassociated with a Fabric!
    VerifyOrDie(!((mSecureSessionType == Type::kPASE) && (peerNode.GetFabricIndex() != kUndefinedFabricIndex)));
    // CASE sessions must always start "associated" a given Fabric!
    VerifyOrDie(!((mSecureSessionType == Type::kCASE) && (peerNode.GetFabricIndex() == kUndefinedFabricIndex)));
    // CASE sessions can only be activated against operational node IDs!
    VerifyOrDie(!((mSecureSessionType == Type::kCASE) &&
                  (!IsOperationalNodeId(peerNode.GetNodeId()) || !IsOperationalNodeId(localNode.GetNodeId()))));

    mPeerNodeId          = peerNode.GetNodeId();
    mLocalNodeId         = localNode.GetNodeId();
    mPeerCATs            = peerCATs;
    mPeerSessionId       = peerSessionId;
    mRemoteSessionParams = sessionParameters;
    SetFabricIndex(peerNode.GetFabricIndex());
    MarkActiveRx(); // Initialize SessionTimestamp and ActiveTimestamp per spec.

    Retain(); // This ref is released inside MarkForEviction
    MoveToState(State::kActive);

    if (mSecureSessionType == Type::kCASE)
        mTable.NewerSessionAvailable(this);

    ChipLogDetail(Inet, "SecureSession[%p]: Activated - Type:%d LSID:%d", this, to_underlying(mSecureSessionType), mLocalSessionId);
}

const char * SecureSession::StateToString(State state) const
{
    switch (state)
    {
    case State::kEstablishing:
        return "kEstablishing";
        break;

    case State::kActive:
        return "kActive";
        break;

    case State::kDefunct:
        return "kDefunct";
        break;

    case State::kPendingEviction:
        return "kPendingEviction";
        break;

    default:
        return "???";
        break;
    }
}

void SecureSession::MoveToState(State targetState)
{
    if (mState != targetState)
    {
        ChipLogProgress(SecureChannel, "SecureSession[%p, LSID:%d]: State change '%s' --> '%s'", this, mLocalSessionId,
                        StateToString(mState), StateToString(targetState));
        mState = targetState;
    }
}

void SecureSession::MarkAsDefunct()
{
    ChipLogDetail(Inet, "SecureSession[%p]: MarkAsDefunct Type:%d LSID:%d", this, to_underlying(mSecureSessionType),
                  mLocalSessionId);
    ReferenceCountedHandle<Transport::Session> ref(*this);

    switch (mState)
    {
    case State::kEstablishing:
        //
        // A session can only be marked as defunct from the state of Active.
        //
        VerifyOrDie(false);
        return;

    case State::kActive:
        MoveToState(State::kDefunct);
        return;

    case State::kDefunct:
        //
        // Do nothing
        //
        return;

    case State::kPendingEviction:
        //
        // Once a session is headed for eviction, we CANNOT bring it back to either being active or defunct. Let's just
        // do nothing and return.
        //
        return;
    }
}

void SecureSession::MarkForEviction()
{
    ChipLogDetail(Inet, "SecureSession[%p]: MarkForEviction Type:%d LSID:%d", this, to_underlying(mSecureSessionType),
                  mLocalSessionId);
    ReferenceCountedHandle<Transport::Session> ref(*this);

    switch (mState)
    {
    case State::kEstablishing:
        MoveToState(State::kPendingEviction);
        // Interrupt the pairing
        NotifySessionReleased();
        return;

    case State::kDefunct:
        FALLTHROUGH;
    case State::kActive:
        Release(); // Decrease the ref which is retained at Activate
        MoveToState(State::kPendingEviction);
        NotifySessionReleased();
        return;

    case State::kPendingEviction:
        // Do nothing
        return;
    }
}

void SecureSession::SetPeerAddress(const PeerAddress & address)
{
    mPeerAddress = address;
    // An explicit SetPeerAddress (CASE/PASE establishment or SessionManager-initiated peer
    // relocation) is treated as ground truth — re-anchor the routable prefix to this address.
    CapturePeerAddressAnchor();
}

void SecureSession::CapturePeerAddressAnchor()
{
    // Only IPv6 unicast peers participate in /64-prefix gating; for everything else (BLE, TCP,
    // IPv4-mapped, multicast, unspecified) leave the anchor cleared so the dispatch site falls
    // back to the original unconditional restamp.
    const Inet::IPAddress & ip = mPeerAddress.GetIPAddress();
    if (mPeerAddress.GetTransportType() != Transport::Type::kUdp || !ip.IsIPv6() || ip.IsIPv6Multicast())
    {
        mPeerAddressAnchorValid = false;
        mPeerAddressRestampRejectCount = 0;
        return;
    }

    // IPAddress::Addr is four uint32_t in network byte order; the first two words form the /64.
    const uint32_t prefixWord0 = ip.Addr[0];
    const uint32_t prefixWord1 = ip.Addr[1];
    memcpy(&mPeerAddressAnchorPrefix[0], &prefixWord0, sizeof(prefixWord0));
    memcpy(&mPeerAddressAnchorPrefix[4], &prefixWord1, sizeof(prefixWord1));
    mPeerAddressAnchorValid        = true;
    mPeerAddressRestampRejectCount = 0;
    memset(mPeerAddressLastRejectedPrefix, 0, sizeof(mPeerAddressLastRejectedPrefix));
}

void SecureSession::RestampPeerAddressIfRoutable(const PeerAddress & candidate)
{
    if (mPeerAddress == candidate)
    {
        return;
    }

    // No anchor (non-IPv6 peer, or never established) — preserve legacy unconditional restamp.
    if (!mPeerAddressAnchorValid)
    {
        mPeerAddress = candidate;
        return;
    }

    const Inet::IPAddress & candidateIp = candidate.GetIPAddress();
    // Candidate isn't an IPv6 UDP peer we can prefix-compare; fall back to legacy behavior so we
    // don't accidentally trap kBle/kTcp/IPv4 transitions.
    if (candidate.GetTransportType() != Transport::Type::kUdp || !candidateIp.IsIPv6())
    {
        mPeerAddress = candidate;
        mPeerAddressRestampRejectCount = 0;
        return;
    }

    uint8_t candidatePrefix[8];
    const uint32_t candidateWord0 = candidateIp.Addr[0];
    const uint32_t candidateWord1 = candidateIp.Addr[1];
    memcpy(&candidatePrefix[0], &candidateWord0, sizeof(candidateWord0));
    memcpy(&candidatePrefix[4], &candidateWord1, sizeof(candidateWord1));

    if (memcmp(candidatePrefix, mPeerAddressAnchorPrefix, sizeof(candidatePrefix)) == 0)
    {
        // Same /64 as anchor — still considered routable; accept the host-bits change.
        mPeerAddress                   = candidate;
        mPeerAddressRestampRejectCount = 0;
        return;
    }

    // Candidate's /64 differs from the routable anchor. Suspect a relay-mangled source or a peer
    // genuinely moving networks. Discard the restamp until the same new prefix repeats enough
    // times to convince us the move is real.
    char candidateStr[Transport::PeerAddress::kMaxToStringSize];
    candidate.ToString(candidateStr);

    if (memcmp(candidatePrefix, mPeerAddressLastRejectedPrefix, sizeof(candidatePrefix)) != 0)
    {
        // New rejected prefix — restart the override counter so unrelated bursts don't accumulate.
        memcpy(mPeerAddressLastRejectedPrefix, candidatePrefix, sizeof(candidatePrefix));
        mPeerAddressRestampRejectCount = 1;
    }
    else if (mPeerAddressRestampRejectCount < UINT8_MAX)
    {
        mPeerAddressRestampRejectCount++;
    }

    if (mPeerAddressRestampRejectCount >= kPeerAddressRestampOverrideThreshold)
    {
        ChipLogProgress(Inet,
                        "SecureSession[%p, LSID:%d]: peer /64 anchor override after %u rejects, accepting %s",
                        this, mLocalSessionId, static_cast<unsigned>(mPeerAddressRestampRejectCount), candidateStr);
        mPeerAddress = candidate;
        // Adopt the new prefix as the anchor going forward.
        memcpy(mPeerAddressAnchorPrefix, candidatePrefix, sizeof(mPeerAddressAnchorPrefix));
        mPeerAddressRestampRejectCount = 0;
        memset(mPeerAddressLastRejectedPrefix, 0, sizeof(mPeerAddressLastRejectedPrefix));
        return;
    }

    ChipLogProgress(Inet,
                    "SecureSession[%p, LSID:%d]: ignoring restamp to non-routable peer %s (reject %u/%u)",
                    this, mLocalSessionId, candidateStr,
                    static_cast<unsigned>(mPeerAddressRestampRejectCount),
                    static_cast<unsigned>(kPeerAddressRestampOverrideThreshold));
}

Access::SubjectDescriptor SecureSession::GetSubjectDescriptor() const
{
    Access::SubjectDescriptor subjectDescriptor;
    if (IsOperationalNodeId(mPeerNodeId))
    {
        subjectDescriptor.authMode        = Access::AuthMode::kCase;
        subjectDescriptor.subject         = mPeerNodeId;
        subjectDescriptor.cats            = mPeerCATs;
        subjectDescriptor.fabricIndex     = GetFabricIndex();
        subjectDescriptor.isCommissioning = IsCommissioningSession();
    }
    else if (IsPAKEKeyId(mPeerNodeId))
    {
        // Responder (aka commissionee) gets subject descriptor filled in.
        // Initiator (aka commissioner) leaves subject descriptor unfilled.
        if (GetCryptoContext().IsResponder())
        {
            subjectDescriptor.authMode        = Access::AuthMode::kPase;
            subjectDescriptor.subject         = mPeerNodeId;
            subjectDescriptor.fabricIndex     = GetFabricIndex();
            subjectDescriptor.isCommissioning = IsCommissioningSession();
        }
    }
    else
    {
        VerifyOrDie(false);
    }
    return subjectDescriptor;
}

bool SecureSession::IsCommissioningSession() const
{
    // PASE session is always a commissioning session.
    if (IsPASESession())
    {
        return true;
    }

    // CASE session is a commissioning session if it was marked as such.
    // The SessionManager is what keeps track.
    if (IsCASESession() && mIsCaseCommissioningSession)
    {
        return true;
    }

    return false;
}

void SecureSession::Retain()
{
#if CHIP_CONFIG_SECURE_SESSION_REFCOUNT_LOGGING
    ChipLogProgress(SecureChannel, "SecureSession[%p]: ++ %d -> %d", this, GetReferenceCount(), GetReferenceCount() + 1);
#endif

    ReferenceCounted<SecureSession, SecureSessionDeleter, 0, uint16_t>::Retain();
}

void SecureSession::Release()
{
#if CHIP_CONFIG_SECURE_SESSION_REFCOUNT_LOGGING
    ChipLogProgress(SecureChannel, "SecureSession[%p]: -- %d -> %d", this, GetReferenceCount(), GetReferenceCount() - 1);
#endif

    ReferenceCounted<SecureSession, SecureSessionDeleter, 0, uint16_t>::Release();
}

void SecureSession::NewerSessionAvailable(const SessionHandle & session)
{
    // Shift to the new session, checks are performed by the the caller SecureSessionTable::NewerSessionAvailable.
    IntrusiveList<SessionHolder>::Iterator iter = mHolders.begin();
    while (iter != mHolders.end())
    {
        // The iterator can be invalid once the session holder is migrated to another session. So we store its next value before
        // notifying the holder.
        IntrusiveList<SessionHolder>::Iterator next = iter;
        ++next;

        iter->ShiftToSession(session);

        iter = next;
    }
}

} // namespace Transport
} // namespace chip
