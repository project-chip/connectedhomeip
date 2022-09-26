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

namespace chip {
namespace Transport {

void SecureSessionDeleter::Release(SecureSession * entry)
{
    entry->mTable.ReleaseSession(entry);
}

void SecureSession::Activate(const ScopedNodeId & localNode, const ScopedNodeId & peerNode, CATValues peerCATs,
                             uint16_t peerSessionId, const ReliableMessageProtocolConfig & config)
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

    mPeerNodeId      = peerNode.GetNodeId();
    mLocalNodeId     = localNode.GetNodeId();
    mPeerCATs        = peerCATs;
    mPeerSessionId   = peerSessionId;
    mRemoteMRPConfig = config;
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
        ChipLogProgress(SecureChannel, "SecureSession[%p]: Moving from state '%s' --> '%s'", this, StateToString(mState),
                        StateToString(targetState));
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
        // Once a session is headed for eviction, we CANNOT bring it back to either being active or defunct.
        //
        VerifyOrDie(false);
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

Access::SubjectDescriptor SecureSession::GetSubjectDescriptor() const
{
    Access::SubjectDescriptor subjectDescriptor;
    if (IsOperationalNodeId(mPeerNodeId))
    {
        subjectDescriptor.authMode    = Access::AuthMode::kCase;
        subjectDescriptor.subject     = mPeerNodeId;
        subjectDescriptor.cats        = mPeerCATs;
        subjectDescriptor.fabricIndex = GetFabricIndex();
    }
    else if (IsPAKEKeyId(mPeerNodeId))
    {
        // Responder (aka commissionee) gets subject descriptor filled in.
        // Initiator (aka commissioner) leaves subject descriptor unfilled.
        if (GetCryptoContext().IsResponder())
        {
            subjectDescriptor.authMode    = Access::AuthMode::kPase;
            subjectDescriptor.subject     = mPeerNodeId;
            subjectDescriptor.fabricIndex = GetFabricIndex();
        }
    }
    else
    {
        VerifyOrDie(false);
    }
    return subjectDescriptor;
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
