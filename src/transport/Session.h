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

#pragma once

#include <credentials/FabricTable.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/PeerId.h>
#include <lib/core/ScopedNodeId.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <transport/SessionHolder.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

class SecureSession;
class UnauthenticatedSession;
class IncomingGroupSession;
class OutgoingGroupSession;

constexpr System::Clock::Milliseconds32 kMinActiveTime = System::Clock::Milliseconds32(4000);

class Session
{
public:
    virtual ~Session() {}

    enum class SessionType : uint8_t
    {
        kUndefined       = 0,
        kUnauthenticated = 1,
        kSecure          = 2,
        kGroupIncoming   = 3,
        kGroupOutgoing   = 4,
    };

    virtual SessionType GetSessionType() const = 0;
#if CHIP_PROGRESS_LOGGING
    virtual const char * GetSessionTypeString() const = 0;
#endif

    void AddHolder(SessionHolder & holder)
    {
        VerifyOrDie(!holder.IsInList());
        mHolders.PushBack(&holder);
    }

    void RemoveHolder(SessionHolder & holder)
    {
        VerifyOrDie(mHolders.Contains(&holder));
        mHolders.Remove(&holder);
    }

    virtual void Retain()  = 0;
    virtual void Release() = 0;

    virtual bool IsActiveSession() const = 0;

    virtual ScopedNodeId GetPeer() const                               = 0;
    virtual ScopedNodeId GetLocalScopedNodeId() const                  = 0;
    virtual Access::SubjectDescriptor GetSubjectDescriptor() const     = 0;
    virtual bool RequireMRP() const                                    = 0;
    virtual const ReliableMessageProtocolConfig & GetMRPConfig() const = 0;
    virtual System::Clock::Timestamp GetMRPBaseTimeout()               = 0;
    virtual System::Clock::Milliseconds32 GetAckTimeout() const        = 0;

    // Returns a suggested timeout value based on the round-trip time it takes for the peer at the other end of the session to
    // receive a message, process it and send it back. This is computed based on the session type, the type of transport, sleepy
    // characteristics of the target and a caller-provided value for the time it takes to process a message at the upper layer on
    // the target For group sessions, this function will always return 0.
    System::Clock::Timeout ComputeRoundTripTimeout(System::Clock::Timeout upperlayerProcessingTimeout);

    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    SecureSession * AsSecureSession();
    UnauthenticatedSession * AsUnauthenticatedSession();
    IncomingGroupSession * AsIncomingGroupSession();
    OutgoingGroupSession * AsOutgoingGroupSession();

    bool IsGroupSession() const
    {
        return GetSessionType() == SessionType::kGroupIncoming || GetSessionType() == SessionType::kGroupOutgoing;
    }

    bool IsSecureSession() const { return GetSessionType() == SessionType::kSecure; }

    void DispatchSessionEvent(SessionDelegate::Event event)
    {
        for (auto & holder : mHolders)
        {
            holder.DispatchSessionEvent(event);
        }
    }

protected:
    // This should be called by sub-classes at the very beginning of the destructor, before any data field is disposed, such that
    // the session is still functional during the callback.
    void NotifySessionReleased()
    {
        SessionHandle session(*this);
        while (!mHolders.Empty())
        {
            mHolders.begin()->OnSessionReleased(); // OnSessionReleased must remove the item from the linked list
        }
    }

    void SetFabricIndex(FabricIndex index) { mFabricIndex = index; }

private:
    IntrusiveList<SessionHolder> mHolders;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
};

} // namespace Transport
} // namespace chip
