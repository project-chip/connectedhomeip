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

class PeerCacheEntry;
using PeerHandle = ReferenceCountedHandle<PeerCacheEntry>;

class PeerCacheEntryDeleter
{
public:
    // This is a no-op because life-cycle of PeerCache is rotated by LRU
    static void Release(PeerCacheEntry * entry) {}
};

/**
 * @brief
 *   A Peer table entry stores the binding of NodeId, TransportAddress, and message counters. For both secure and unsecure nodes.
 *
 *   The entries are rotated using LRU, but entry can be hold by using PeerHandle, which increase the reference count by 1. When the
 * reference count is not 0, the entry won't be pruned.
 */
class PeerCacheEntry : public ReferenceCounted<PeerCacheEntry, PeerCacheEntryDeleter>
{
public:
    PeerCacheEntry(const PeerId & peer) : mPeer(peer) {}

    PeerCacheEntry(const PeerCacheEntry &) = delete;
    PeerCacheEntry & operator=(const PeerCacheEntry &) = delete;
    PeerCacheEntry(PeerCacheEntry &&)                  = delete;
    PeerCacheEntry & operator=(PeerCacheEntry &&) = delete;

    const PeerId & GetPeer() const { return mPeer; }

    uint64_t GetLastActivityTimeMs() const { return mLastActivityTimeMs; }
    void SetLastActivityTimeMs(uint64_t value) { mLastActivityTimeMs = value; }

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

private:
    uint64_t mLastActivityTimeMs = 0;

    const PeerId mPeer;

    PeerAddress mPeerAddress = PeerAddress::Uninitialized();
};

template <size_t kMaxConnectionCount, Time::Source kTimeSource = Time::Source::kSystem>
class PeerCache
{
public:
    /**
     * Allocates a new peer state object out of the internal resource pool.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum connection count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR AllocEntry(const PeerId & peer, PeerCacheEntry *& entry)
    {
        entry = mEntries.CreateObject(peer);
        if (entry != nullptr)
            return CHIP_NO_ERROR;

        entry = FindLeastRecentUsedEntry();
        VerifyOrDie(entry != nullptr);

        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();
        if (currentTime - entry->GetLastActivityTimeMs() < kMinimalActivityTimeMs)
        {
            // Protect the entry for a short period to prevent from rotating too fast.
            entry = nullptr;
            return CHIP_ERROR_NO_MEMORY;
        }

        mEntries.ResetObject(entry, peer);
        return CHIP_NO_ERROR;
    }

    /**
     * Get a peer given the peer id.
     *
     * @return the peer found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    PeerCacheEntry * FindEntry(const PeerId & peer)
    {
        PeerCacheEntry * result = nullptr;
        mEntries.ForEachActiveObject([&](PeerCacheEntry * entry) {
            if (entry->GetPeer() == peer)
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
    PeerCacheEntry * FindOrAllocateEntry(const PeerId & peer)
    {
        PeerCacheEntry * result = FindEntry(peer);
        if (result != nullptr)
            return result;

        CHIP_ERROR err = AllocEntry(peer, result);
        if (err == CHIP_NO_ERROR)
        {
            return result;
        }
        else
        {
            ChipLogError(Inet, "Peer cache exhausted: %s", ErrorStr(err));
            return nullptr;
        }
    }

    /// Mark a peer as active
    void MarkPeerActive(PeerCacheEntry & entry) { entry.SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs()); }

    /// Allows access to the underlying time source used for keeping track of connection active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    PeerCacheEntry * FindLeastRecentUsedEntry()
    {
        PeerCacheEntry * result = nullptr;
        uint64_t oldestTimeMs   = std::numeric_limits<uint64_t>::max();

        mEntries.ForEachActiveObject([&](PeerCacheEntry * entry) {
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
    BitMapObjectPool<PeerCacheEntry, kMaxConnectionCount> mEntries;
};

} // namespace Transport
} // namespace chip
