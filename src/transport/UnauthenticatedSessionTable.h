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
#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <lib/support/ReferenceCountedHandle.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <system/TimeSource.h>
#include <transport/MessageCounter.h>
#include <transport/PeerMessageCounter.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

class UnauthenticatedSession;
using UnauthenticatedSessionHandle = ReferenceCountedHandle<UnauthenticatedSession>;

class UnauthenticatedSessionDeleter
{
public:
    // This is a no-op because life-cycle of UnauthenticatedSessionTable is rotated by LRU
    static void Release(UnauthenticatedSession * entry) {}
};

/**
 * @brief
 *   An UnauthenticatedSession stores the binding of TransportAddress, and message counters.
 */
class UnauthenticatedSession : public ReferenceCounted<UnauthenticatedSession, UnauthenticatedSessionDeleter, 0>
{
public:
    UnauthenticatedSession(const PeerAddress & address, const ReliableMessageProtocolConfig & config) :
        mPeerAddress(address), mMRPConfig(config)
    {}

    UnauthenticatedSession(const UnauthenticatedSession &) = delete;
    UnauthenticatedSession & operator=(const UnauthenticatedSession &) = delete;
    UnauthenticatedSession(UnauthenticatedSession &&)                  = delete;
    UnauthenticatedSession & operator=(UnauthenticatedSession &&) = delete;

    System::Clock::Timestamp GetLastActivityTime() const { return mLastActivityTime; }
    void SetLastActivityTime(System::Clock::Timestamp value) { mLastActivityTime = value; }

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }

    void SetMRPConfig(const ReliableMessageProtocolConfig & config) { mMRPConfig = config; }

    const ReliableMessageProtocolConfig & GetMRPConfig() const { return mMRPConfig; }

    PeerMessageCounter & GetPeerMessageCounter() { return mPeerMessageCounter; }

private:
    System::Clock::Timestamp mLastActivityTime = System::Clock::kZero;

    const PeerAddress mPeerAddress;
    ReliableMessageProtocolConfig mMRPConfig;
    PeerMessageCounter mPeerMessageCounter;
};

/*
 * @brief
 *   An table which manages UnauthenticatedSessions
 *
 *   The UnauthenticatedSession entries are rotated using LRU, but entry can be
 *   hold by using UnauthenticatedSessionHandle, which increase the reference
 *   count by 1. If the reference count is not 0, the entry won't be pruned.
 */
template <size_t kMaxSessionCount, Time::Source kTimeSource = Time::Source::kSystem>
class UnauthenticatedSessionTable
{
public:
    /**
     * Get a session given the peer address. If the session doesn't exist in the cache, allocate a new entry for it.
     *
     * @return the session found or allocated, nullptr if not found and allocation failed.
     */
    CHECK_RETURN_VALUE
    Optional<UnauthenticatedSessionHandle> FindOrAllocateEntry(const PeerAddress & address,
                                                               const ReliableMessageProtocolConfig & config)
    {
        UnauthenticatedSession * result = FindEntry(address);
        if (result != nullptr)
            return MakeOptional<UnauthenticatedSessionHandle>(*result);

        CHIP_ERROR err = AllocEntry(address, config, result);
        if (err == CHIP_NO_ERROR)
        {
            return MakeOptional<UnauthenticatedSessionHandle>(*result);
        }
        else
        {
            return Optional<UnauthenticatedSessionHandle>::Missing();
        }
    }

    /// Mark a session as active
    void MarkSessionActive(UnauthenticatedSessionHandle session)
    {
        session->SetLastActivityTime(mTimeSource.GetMonotonicTimestamp());
    }

    /// Allows access to the underlying time source used for keeping track of session active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    /**
     * Allocates a new session out of the internal resource pool.
     *
     * @returns CHIP_NO_ERROR if new session created. May fail if maximum session count has been reached (with
     * CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR AllocEntry(const PeerAddress & address, const ReliableMessageProtocolConfig & config,
                          UnauthenticatedSession *& entry)
    {
        entry = mEntries.CreateObject(address, config);
        if (entry != nullptr)
            return CHIP_NO_ERROR;

        entry = FindLeastRecentUsedEntry();
        if (entry == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        mEntries.ResetObject(entry, address, config);
        return CHIP_NO_ERROR;
    }

    /**
     * Get a session using given address
     *
     * @return the peer found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    UnauthenticatedSession * FindEntry(const PeerAddress & address)
    {
        UnauthenticatedSession * result = nullptr;
        mEntries.ForEachActiveObject([&](UnauthenticatedSession * entry) {
            if (MatchPeerAddress(entry->GetPeerAddress(), address))
            {
                result = entry;
                return Loop::Break;
            }
            return Loop::Continue;
        });
        return result;
    }

    UnauthenticatedSession * FindLeastRecentUsedEntry()
    {
        UnauthenticatedSession * result     = nullptr;
        System::Clock::Timestamp oldestTime = System::Clock::Timestamp(std::numeric_limits<System::Clock::Timestamp::rep>::max());

        mEntries.ForEachActiveObject([&](UnauthenticatedSession * entry) {
            if (entry->GetReferenceCount() == 0 && entry->GetLastActivityTime() < oldestTime)
            {
                result     = entry;
                oldestTime = entry->GetLastActivityTime();
            }
            return Loop::Continue;
        });

        return result;
    }

    // A temporary solution for #11120
    // Enforce interface match if not null
    static bool MatchInterface(Inet::InterfaceId i1, Inet::InterfaceId i2)
    {
        if (i1.IsPresent() && i2.IsPresent())
        {
            return i1 == i2;
        }
        else
        {
            // One of the interfaces is null.
            return true;
        }
    }

    static bool MatchPeerAddress(const PeerAddress & a1, const PeerAddress & a2)
    {
        if (a1.GetTransportType() != a2.GetTransportType())
            return false;

        switch (a1.GetTransportType())
        {
        case Transport::Type::kUndefined:
            return false;
        case Transport::Type::kUdp:
        case Transport::Type::kTcp:
            return a1.GetIPAddress() == a2.GetIPAddress() && a1.GetPort() == a2.GetPort() &&
                // Enforce interface equal-ness if the address is link-local, otherwise ignore interface
                // Use MatchInterface for a temporary solution for #11120
                (a1.GetIPAddress().IsIPv6LinkLocal() ? a1.GetInterface() == a2.GetInterface()
                                                     : MatchInterface(a1.GetInterface(), a2.GetInterface()));
        case Transport::Type::kBle:
            // TODO: complete BLE address comparation
            return true;
        }

        return false;
    }

    Time::TimeSource<Time::Source::kSystem> mTimeSource;
    BitMapObjectPool<UnauthenticatedSession, kMaxSessionCount, OnObjectPoolDestruction::IgnoreUnsafeDoNotUseInNewCode> mEntries;
};

} // namespace Transport
} // namespace chip
