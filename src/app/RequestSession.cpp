/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/RequestSession.h>

#include <platform/LockTracker.h>
#include <transport/GroupSession.h>

namespace chip {
namespace app {

RequestSession::RequestSession(const OptionalSessionHandleRef & session)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturn(session.has_value());
    const SessionHandle & handle = session->get();
    mKind                        = Kind::kUnavailable;

    if (handle->IsGroupSession())
    {
        VerifyOrReturn(handle->GetSessionType() == Transport::Session::SessionType::kGroupOutgoing);
        mGroupId     = handle->AsOutgoingGroupSession()->GetGroupId();
        mFabricIndex = handle->GetFabricIndex();
        mKind        = Kind::kGroupcast;
    }
    else
    {
        VerifyOrReturn(mUnicastSession.Grab(handle));
        mPeerInteractionModelRevision = handle->GetPeerInteractionModelRevision();
        mKind                         = Kind::kUnicast;
    }
}

CHIP_ERROR RequestSession::Validate() const
{
    VerifyOrReturnError(mKind != Kind::kUnavailable, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mKind != Kind::kUnicast || static_cast<bool>(mUnicastSession), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR RequestSession::CheckSession(const SessionHandle & session) const
{
    ReturnErrorOnFailure(Validate());
    if (mKind == Kind::kUnicast)
    {
        VerifyOrReturnError(mUnicastSession.Contains(session), CHIP_ERROR_INVALID_ARGUMENT);
    }
    else if (mKind == Kind::kGroupcast)
    {
        VerifyOrReturnError(session->GetSessionType() == Transport::Session::SessionType::kGroupOutgoing,
                            CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(session->GetFabricIndex() == mFabricIndex &&
                                session->AsOutgoingGroupSession()->GetGroupId() == mGroupId,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }
    return CHIP_NO_ERROR;
}

void RequestSession::Release()
{
    mUnicastSession.Release();
    OnSessionReleased();
}

void RequestSession::OnSessionReleased()
{
    mKind = Kind::kUnavailable;
    mPeerInteractionModelRevision.reset();
}

} // namespace app
} // namespace chip
