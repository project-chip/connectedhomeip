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

void SecureSession::MarkForRemoval()
{
    ChipLogDetail(Inet, "SecureSession[%p]: MarkForRemoval Type:%d LSID:%d", this, to_underlying(mSecureSessionType),
                  mLocalSessionId);
    ReferenceCountedHandle<Transport::Session> ref(*this);
    switch (mState)
    {
    case State::kPairing:
        mState = State::kPendingRemoval;
        // Interrupt the pairing
        NotifySessionReleased();
        return;
    case State::kActive:
        Release(); // Decrease the ref which is retained at Activate
        mState = State::kPendingRemoval;
        NotifySessionReleased();
        return;
    case State::kPendingRemoval:
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

} // namespace Transport
} // namespace chip
