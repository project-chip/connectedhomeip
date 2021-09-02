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

#pragma once

#include <cstdint>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <system/SystemTimer.h>
#include <system/TimeSource.h>

// set MDNS_LOGGING to enable logging -- sometimes used in debug/test programs -- traces the behavior
#ifdef MDNS_LOGGING
#define MdnsLogProgress ChipLogProgress
#else
#define MdnsLogProgress(...)
#endif

namespace chip {
namespace Mdns {

template <size_t CACHE_SIZE>
class MdnsCache
{
public:
    MdnsCache() : elementsUsed(CACHE_SIZE)
    {
        for (MdnsCacheEntry & e : mLookupTable)
        {
            // each unused entry decrements the count
            MarkEntryUnused(e);
        }
        MdnsLogProgress(Discovery, "construct mdns cache of size %ld", CACHE_SIZE);
    }

    // insert this entry into the cache.
    // return error if cache is full
    // TODO:   have an eviction policy so if the cache is full, an entry may be deleted.
    //         One policy may be Least-time-to-live
    CHIP_ERROR Insert(PeerId peerId, const Inet::IPAddress & addr, uint16_t port, Inet::InterfaceId iface, uint32_t TTLms)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        MdnsCacheEntry * entry;

        entry = FindPeerId(peerId, currentTime);
        if (entry)
        {
            // update timeout if found entry
            entry->expiryTime = currentTime + TTLms;
            entry->TTL        = TTLms; // in case it changes */
            return CHIP_NO_ERROR;
        }

        VerifyOrReturnError(entry = findSlot(currentTime), CHIP_ERROR_TOO_MANY_KEYS);

        // have a free slot for this entry
        entry->peerId     = peerId;
        entry->ipAddr     = addr;
        entry->port       = port;
        entry->ifaceId    = iface;
        entry->TTL        = TTLms;
        entry->expiryTime = currentTime + TTLms;
        elementsUsed++;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Delete(PeerId peerId)
    {
        MdnsCacheEntry * pentry;
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        VerifyOrReturnError(pentry = FindPeerId(peerId, currentTime), CHIP_ERROR_KEY_NOT_FOUND);

        MarkEntryUnused(*pentry);
        return CHIP_NO_ERROR;
    }

    // given a peerId, find the parameters if its in the cache, or return error
    CHIP_ERROR Lookup(PeerId peerId, Inet::IPAddress & addr, uint16_t & port, Inet::InterfaceId & iface)
    {
        MdnsCacheEntry * pentry;
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        VerifyOrReturnError(pentry = FindPeerId(peerId, currentTime), CHIP_ERROR_KEY_NOT_FOUND);

        addr  = pentry->ipAddr;
        port  = pentry->port;
        iface = pentry->ifaceId;

        return CHIP_NO_ERROR;
    }

    // only useful if MDNS_LOGGING is set.   If not used, should be optimized out
    void DumpCache()
    {
        int i = 0;

        MdnsLogProgress(Discovery, "cache size = %d", elementsUsed);
        for (MdnsCacheEntry & e : mLookupTable)
        {
            if (e.peerId == nullPeerId)
            {
                MdnsLogProgress(Discovery, "Entry %d unused", i);
            }
            else
            {
                char address[100];

                e.ipAddr.ToString(address, sizeof address);
                MdnsLogProgress(Discovery, "Entry %d: node %lx fabric %lx, port = %d, address = %s", i, e.peerId.GetNodeId(),
                                e.peerId.GetFabricId(), e.port, address);
            }
            i++;
        }
    }

private:
    struct MdnsCacheEntry
    {
        PeerId peerId;
        Inet::IPAddress ipAddr;
        uint16_t port;
        Inet::InterfaceId ifaceId;
        uint64_t TTL;        // from mdns record -- units?
        uint64_t expiryTime; // units?
    };
    PeerId nullPeerId; // indicates a cache entry is unused
    int elementsUsed;  // running count of how many entries are used -- for a sanity check

    MdnsCacheEntry mLookupTable[CACHE_SIZE];
    Time::TimeSource<Time::Source::kSystem> mTimeSource;

    MdnsCacheEntry * findSlot(uint64_t currentTime)
    {
        for (MdnsCacheEntry & entry : mLookupTable)
        {
            if (entry.peerId == nullPeerId)
                return &entry;

            if (entry.expiryTime <= currentTime)
            {
                MarkEntryUnused(entry);
                return &entry;
            }
        }
        return nullptr;
    }

    MdnsCacheEntry * FindPeerId(PeerId peerId, uint64_t current_time)
    {
        for (MdnsCacheEntry & entry : mLookupTable)
        {
            if (entry.peerId == peerId)
            {
                if (entry.expiryTime < current_time)
                {
                    MarkEntryUnused(entry);
                    break; // return nullptr
                }
                else
                    return &entry;
            }
            if (entry.peerId != nullPeerId && entry.expiryTime < current_time)
            {
                MarkEntryUnused(entry);
            }
        }

        return nullptr;
    }

    // have a method to mark ununused --  so its easy to change
    void MarkEntryUnused(MdnsCacheEntry & pentry)
    {
        pentry.peerId = nullPeerId;
        elementsUsed--;
    }
};

#ifndef MDNS_LOGGING
#undef MdnsLogProgress
#endif

} // namespace Mdns
} // namespace chip
