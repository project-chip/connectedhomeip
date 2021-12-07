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
#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <lib/dnssd/Resolver.h>
#include <system/TimeSource.h>

// set MDNS_LOGGING to enable logging -- sometimes used in debug/test programs -- traces the behavior
#ifdef MDNS_LOGGING
#define MdnsLogProgress ChipLogProgress
#else
#define MdnsLogProgress(...)
#endif

namespace chip {
namespace Dnssd {

template <size_t CACHE_SIZE>
class DnssdCache
{
public:
    DnssdCache() : elementsUsed(CACHE_SIZE)
    {
        for (ResolvedNodeData & e : mLookupTable)
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
    CHIP_ERROR Insert(const ResolvedNodeData & nodeData)
    {
        const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

        ResolvedNodeData * entry;

        entry = FindPeerId(nodeData.mPeerId, currentTime);
        if (entry)
        {
            *entry = nodeData;
            return CHIP_NO_ERROR;
        }

        VerifyOrReturnError(entry = findSlot(currentTime), CHIP_ERROR_TOO_MANY_KEYS);

        // have a free slot for this entry
        *entry = nodeData;
        elementsUsed++;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Delete(PeerId peerId)
    {
        ResolvedNodeData * pentry;
        const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

        VerifyOrReturnError(pentry = FindPeerId(peerId, currentTime), CHIP_ERROR_KEY_NOT_FOUND);

        MarkEntryUnused(*pentry);
        return CHIP_NO_ERROR;
    }

    // given a peerId, find the parameters if its in the cache, or return error
    CHIP_ERROR Lookup(PeerId peerId, ResolvedNodeData & nodeData)
    {
        ResolvedNodeData * pentry;
        const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

        VerifyOrReturnError(pentry = FindPeerId(peerId, currentTime), CHIP_ERROR_KEY_NOT_FOUND);

        nodeData = *pentry;

        return CHIP_NO_ERROR;
    }

    // only useful if MDNS_LOGGING is set.   If not used, should be optimized out
    void DumpCache()
    {
        int i = 0;

        MdnsLogProgress(Discovery, "cache size = %d", elementsUsed);
        for (ResolvedNodeData & e : mLookupTable)
        {
            if (e.mPeerId == nullPeerId)
            {
                MdnsLogProgress(Discovery, "Entry %d unused", i);
            }
            else
            {
                MdnsLogProgress(Discovery, "Entry %d: node %lx fabric %lx, port = %d", i, e.mPeerId.GetNodeId(),
                                e.peerId.GetFabricId(), e.port);
                for (size_t j = 0; j < e.mNumIPs; ++j)
                {
                    char address[Inet::IPAddress::kMaxStringLength];
                    e.mAddress[i].ToString(address);
                    MdnsLogProgress(Discovery, "    address %d: %s", j, address);
                }
            }
            i++;
        }
    }

private:
    PeerId nullPeerId; // indicates a cache entry is unused
    int elementsUsed;  // running count of how many entries are used -- for a sanity check

    ResolvedNodeData mLookupTable[CACHE_SIZE];

    ResolvedNodeData * findSlot(System::Clock::Timestamp currentTime)
    {
        for (ResolvedNodeData & entry : mLookupTable)
        {
            if (entry.mPeerId == nullPeerId)
                return &entry;

            if (entry.mExpiryTime <= currentTime)
            {
                MarkEntryUnused(entry);
                return &entry;
            }
        }
        return nullptr;
    }

    ResolvedNodeData * FindPeerId(PeerId peerId, System::Clock::Timestamp current_time)
    {
        for (ResolvedNodeData & entry : mLookupTable)
        {
            if (entry.mPeerId == peerId)
            {
                if (entry.mExpiryTime < current_time)
                {
                    MarkEntryUnused(entry);
                    break; // return nullptr
                }
                else
                    return &entry;
            }
            if (entry.mPeerId != nullPeerId && entry.mExpiryTime < current_time)
            {
                MarkEntryUnused(entry);
            }
        }

        return nullptr;
    }

    // have a method to mark ununused --  so its easy to change
    void MarkEntryUnused(ResolvedNodeData & pentry)
    {
        pentry.mPeerId = nullPeerId;
        elementsUsed--;
    }
};

#ifndef MDNS_LOGGING
#undef MdnsLogProgress
#endif

} // namespace Dnssd
} // namespace chip
