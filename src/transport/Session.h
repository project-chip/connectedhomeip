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

#include <access/SubjectDescriptor.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/Optional.h>
#include <lib/core/PeerId.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/ReferenceCountedHandle.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <platform/LockTracker.h>
#include <transport/SessionDelegate.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {
class Session;
} // namespace Transport

/** @brief
 *    Non-copyable session reference. All SessionHandles are created within SessionManager. It is not allowed to store SessionHandle
 *    anywhere except for function arguments and return values.
 *
 *    SessionHandle is reference counted such that it is never dangling, but there can be a gray period when the session is marked
 *    as pending removal but not actually removed yet. During this period, the handle is functional, but the underlying session
 *    won't be able to be grabbed by any SessionHolder. SessionHandle->IsActiveSession can be used to check if the session is
 *    active.
 */
class SessionHandle
{
public:
    SessionHandle(Transport::Session & session) : mSession(session) {}
    ~SessionHandle() {}

    SessionHandle(const SessionHandle &) = delete;
    SessionHandle operator=(const SessionHandle &) = delete;
    SessionHandle(SessionHandle &&)                = default;
    SessionHandle & operator=(SessionHandle &&) = delete;

    bool operator==(const SessionHandle & that) const { return &mSession.Get() == &that.mSession.Get(); }

    Transport::Session * operator->() const { return mSession.operator->(); }

private:
    friend class SessionHolder;
    ReferenceCountedHandle<Transport::Session> mSession;
};

/** @brief
 *    Managed session reference. The object is used to store a session, the stored session will be automatically
 *    released when the underlying session is released. One must verify it is available before use. The object can be
 *    created using SessionHandle.Grab()
 */
class SessionHolder : public IntrusiveListNodeBase<>
{
public:
    SessionHolder() {}
    SessionHolder(const SessionHandle & handle) { Grab(handle); }
    virtual ~SessionHolder();

    SessionHolder(const SessionHolder &);
    SessionHolder(SessionHolder && that);
    SessionHolder & operator=(const SessionHolder &);
    SessionHolder & operator=(SessionHolder && that);

    virtual void SessionReleased() { Release(); }
    virtual void ShiftToSession(const SessionHandle & session)
    {
        Release();
        Grab(session);
    }

    bool Contains(const SessionHandle & session) const
    {
        return mSession.HasValue() && &mSession.Value().Get() == &session.mSession.Get();
    }

    bool GrabPairingSession(const SessionHandle & session); // Should be only used inside CASE/PASE pairing.
    bool Grab(const SessionHandle & session);
    void Release();

    explicit operator bool() const { return mSession.HasValue(); }
    Optional<SessionHandle> Get() const
    {
        //
        // We cannot return mSession directly even if Optional<SessionHandle> is internally composed of the same bits,
        // since they are not actually equivalent type-wise, and SessionHandle does not permit copy-construction.
        //
        // So, construct a new Optional<SessionHandle> from the underlying Transport::Session reference.
        //
        return mSession.HasValue() ? chip::MakeOptional<SessionHandle>(mSession.Value().Get())
                                   : chip::Optional<SessionHandle>::Missing();
    }

    Transport::Session * operator->() const { return &mSession.Value().Get(); }

    // There is not delegate, nothing to do here
    virtual void DispatchSessionEvent(SessionDelegate::Event event) {}

protected:
    // Helper for use by the Grab methods.
    void GrabUnchecked(const SessionHandle & session);

    Optional<ReferenceCountedHandle<Transport::Session>> mSession;
};

/// @brief Extends SessionHolder to allow propagate SessionDelegate::* events to a given destination
class SessionHolderWithDelegate : public SessionHolder
{
public:
    SessionHolderWithDelegate(SessionDelegate & delegate) : mDelegate(delegate) {}
    SessionHolderWithDelegate(const SessionHandle & handle, SessionDelegate & delegate) : SessionHolder(handle), mDelegate(delegate)
    {}
    operator bool() const { return SessionHolder::operator bool(); }

    void SessionReleased() override
    {
        Release();

        // Note, the session is already cleared during mDelegate.OnSessionReleased
        mDelegate.OnSessionReleased();
    }

    void ShiftToSession(const SessionHandle & session) override
    {
        if (mDelegate.GetNewSessionHandlingPolicy() == SessionDelegate::NewSessionHandlingPolicy::kShiftToNewSession)
            SessionHolder::ShiftToSession(session);
    }

    void DispatchSessionEvent(SessionDelegate::Event event) override { (mDelegate.*event)(); }

private:
    SessionDelegate & mDelegate;
};

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

    void AddHolder(SessionHolder & holder)
    {
        assertChipStackLockedByCurrentThread();
        VerifyOrDie(!holder.IsInList());
        mHolders.PushBack(&holder);
    }

    void RemoveHolder(SessionHolder & holder)
    {
        assertChipStackLockedByCurrentThread();
        VerifyOrDie(mHolders.Contains(&holder));
        mHolders.Remove(&holder);
    }

    virtual void Retain()  = 0;
    virtual void Release() = 0;

    virtual bool IsActiveSession() const = 0;

    virtual ScopedNodeId GetPeer() const                                     = 0;
    virtual ScopedNodeId GetLocalScopedNodeId() const                        = 0;
    virtual Access::SubjectDescriptor GetSubjectDescriptor() const           = 0;
    virtual bool RequireMRP() const                                          = 0;
    virtual const ReliableMessageProtocolConfig & GetRemoteMRPConfig() const = 0;
    virtual System::Clock::Timestamp GetMRPBaseTimeout() const               = 0;
    virtual System::Clock::Milliseconds32 GetAckTimeout() const              = 0;

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

    bool IsUnauthenticatedSession() const { return GetSessionType() == SessionType::kUnauthenticated; }

    void DispatchSessionEvent(SessionDelegate::Event event)
    {
        // Holders might remove themselves when notified.
        auto holder = mHolders.begin();
        while (holder != mHolders.end())
        {
            auto cur = holder;
            ++holder;
            cur->DispatchSessionEvent(event);
        }
    }

    // Return a session id that is usable for logging. This is the local session
    // id for secure unicast sessions, 0 for non-secure unicast sessions, and
    // the group id for group sessions.
    uint16_t SessionIdForLogging() const;

protected:
    // This should be called by sub-classes at the very beginning of the destructor, before any data field is disposed, such that
    // the session is still functional during the callback.
    void NotifySessionReleased()
    {
        SessionHandle session(*this);
        while (!mHolders.Empty())
        {
            mHolders.begin()->SessionReleased(); // SessionReleased must remove the item from the linked list
        }
    }

    void SetFabricIndex(FabricIndex index) { mFabricIndex = index; }

    const SecureSession * AsConstSecureSession() const;
    const IncomingGroupSession * AsConstIncomingGroupSession() const;
    const OutgoingGroupSession * AsConstOutgoingGroupSession() const;

    IntrusiveList<SessionHolder> mHolders;

private:
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
};

//
// Return a string representation of the underlying session.
//
// Always returns a non-null pointer.
//
const char * GetSessionTypeString(const SessionHandle & session);

} // namespace Transport
} // namespace chip
