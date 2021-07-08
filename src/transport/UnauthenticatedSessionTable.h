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

#include <core/CHIPError.h>
#include <core/ReferenceCounted.h>
#include <support/CodeUtils.h>
#include <support/Pool.h>
#include <support/ReferenceCountedHandle.h>
#include <support/logging/CHIPLogging.h>
#include <system/TimeSource.h>
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
 *
 *   The entries are rotated using LRU, but entry can be hold by using UnauthenticatedSessionHandle, which increase the reference
 * count by 1. If the reference count is not 0, the entry won't be pruned.
 */
class UnauthenticatedSession : public ReferenceCounted<UnauthenticatedSession, UnauthenticatedSessionDeleter>
{
public:
    UnauthenticatedSession(const PeerAddress & address) : mPeerAddress(address) {}

    UnauthenticatedSession(const UnauthenticatedSession &) = delete;
    UnauthenticatedSession & operator=(const UnauthenticatedSession &) = delete;
    UnauthenticatedSession(UnauthenticatedSession &&)                  = delete;
    UnauthenticatedSession & operator=(UnauthenticatedSession &&) = delete;

    uint64_t GetLastActivityTimeMs() const { return mLastActivityTimeMs; }
    void SetLastActivityTimeMs(uint64_t value) { mLastActivityTimeMs = value; }

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }

    MessageCounter & GetLocalMessageCounter() { return mLocalMessageCounter; }
    PeerMessageCounter & GetPeerMessageCounter() { return mPeerMessageCounter; }

private:
    uint64_t mLastActivityTimeMs = 0;

    const PeerAddress mPeerAddress;
    GlobalUnencryptedMessageCounter mLocalMessageCounter;
    PeerMessageCounter mPeerMessageCounter;
};

template <size_t kMaxConnectionCount, Time::Source kTimeSource = Time::Source::kSystem>
class UnauthenticatedSessionTable
{
public:
    /**
     * Allocates a new session out of the internal resource pool.
     *
     * @returns CHIP_NO_ERROR if new session created. May fail if maximum connection count has been reached (with
     * CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR AllocEntry(const PeerAddress & address, UnauthenticatedSession *& entry)
    {
        entry = mEntries.CreateObject(address);
        if (entry != nullptr)
            return CHIP_NO_ERROR;

        entry = FindLeastRecentUsedEntry();
        if (entry == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();
        if (currentTime - entry->GetLastActivityTimeMs() < kMinimalActivityTimeMs)
        {
            // Protect the entry for a short period to prevent from rotating too fast.
            entry = nullptr;
            return CHIP_ERROR_NO_MEMORY;
        }

        mEntries.ResetObject(entry, address);
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
            if (entry->GetPeerAddress() == address)
            {
                result = entry;
                return false;
            }
            return true;
        });
        return result;
    }

    /**
     * Get a peer given the peer id. If the peer doesn't exist in the cache, allocate a new entry for it.
     *
     * @return the peer found or allocated, nullptr if not found and allocate failed.
     */
    CHECK_RETURN_VALUE
    UnauthenticatedSession * FindOrAllocateEntry(const PeerAddress & address)
    {
        UnauthenticatedSession * result = FindEntry(address);
        if (result != nullptr)
            return result;

        CHIP_ERROR err = AllocEntry(address, result);
        if (err == CHIP_NO_ERROR)
        {
            return result;
        }
        else
        {
            return nullptr;
        }
    }

    /// Mark a session as active
    void MarkSessionActive(UnauthenticatedSession & entry) { entry.SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs()); }

    /// Allows access to the underlying time source used for keeping track of connection active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    UnauthenticatedSession * FindLeastRecentUsedEntry()
    {
        UnauthenticatedSession * result = nullptr;
        uint64_t oldestTimeMs           = std::numeric_limits<uint64_t>::max();

        mEntries.ForEachActiveObject([&](UnauthenticatedSession * entry) {
            if (entry->GetReferenceCount() == 0 && entry->GetLastActivityTimeMs() < oldestTimeMs)
            {
                result       = entry;
                oldestTimeMs = entry->GetLastActivityTimeMs();
            }
            return true;
        });

        return result;
    }

    static constexpr uint64_t kMinimalActivityTimeMs = 30000;
    Time::TimeSource<Time::Source::kSystem> mTimeSource;
    BitMapObjectPool<UnauthenticatedSession, kMaxConnectionCount> mEntries;
};

} // namespace Transport
} // namespace chip
