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
    ~DnssdCache() { mLookupTable.ReleaseAll(); }

    // insert this entry into the cache.
    // return error if cache is full
    // TODO:   have an eviction policy so if the cache is full, an entry may be deleted.
    //         One policy may be Least-time-to-live
    CHIP_ERROR Insert(const ResolvedNodeData & nodeData)
    {
        const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

        ResolvedNodeData * entry;

        entry = FindPeer(nodeData.mPeerInfo, currentTime);
        if (entry)
        {
            *entry = nodeData;
            return CHIP_NO_ERROR;
        }

        if (mLookupTable.Allocated() >= CACHE_SIZE)
        {
            return CHIP_ERROR_TOO_MANY_KEYS;
        }

        entry = mLookupTable.CreateObject(nodeData);
        VerifyOrReturnError(entry != nullptr, CHIP_ERROR_TOO_MANY_KEYS);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Delete(PeerInfo peerInfo)
    {
        ResolvedNodeData * pentry;
        const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

        VerifyOrReturnError(pentry = FindPeer(peerInfo, currentTime), CHIP_ERROR_KEY_NOT_FOUND);

        mLookupTable.ReleaseObject(pentry);
        return CHIP_NO_ERROR;
    }

    // given a node/fabric, find the parameters if its in the cache, or return error
    CHIP_ERROR Lookup(PeerInfo peerInfo, ResolvedNodeData & nodeData)
    {
        ResolvedNodeData * pentry;
        const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

        VerifyOrReturnError(pentry = FindPeer(peerInfo, currentTime), CHIP_ERROR_KEY_NOT_FOUND);

        nodeData = *pentry;

        return CHIP_NO_ERROR;
    }

    // only useful if MDNS_LOGGING is set.   If not used, should be optimized out
    void DumpCache()
    {
        MdnsLogProgress(Discovery, "cache size = %d", mLookupTable.Allocated());
        mLookupTable.ForEachActiveObject([] (ResolvedNodeData * e) {
            MdnsLogProgress(Discovery, "Entry: node %lx fabric %lx, port = %d", e->mNodeId, e->mCompressedFabricId, e->port);
            for (size_t j = 0; j < e->mNumIPs; ++j)
            {
                char address[Inet::IPAddress::kMaxStringLength];
                e->mAddress[j].ToString(address);
                MdnsLogProgress(Discovery, "    address %d: %s", j, address);
            }
            return Loop::Continue;
        });
    }

    CHIP_ERROR Delete(PeerId peerId, FabricTable * fabricTable) {
        PeerInfo peerInfo;
        ReturnErrorOnFailure(PeerInfo::FromPeerId(peerInfo, peerId, fabricTable));
        return Delete(peerInfo);
    }

    CHIP_ERROR Lookup(PeerId peerId, ResolvedNodeData & nodeData, FabricTable * fabricTable)
    {
        PeerInfo peerInfo;
        ReturnErrorOnFailure(PeerInfo::FromPeerId(peerInfo, peerId, fabricTable));
        return Lookup(peerInfo, nodeData);
    }

private:
    ObjectPool<ResolvedNodeData, CACHE_SIZE> mLookupTable;

    ResolvedNodeData * FindPeer(PeerInfo peerInfo, System::Clock::Timestamp current_time)
    {
        ResolvedNodeData * result = nullptr;

        mLookupTable.ForEachActiveObject([&] (ResolvedNodeData * entry) {
            if (entry->mPeerInfo == peerInfo)
            {
                if (entry->mExpiryTime < current_time)
                {
                    mLookupTable.ReleaseObject(entry);
                }
                else
                {
                    result = entry;
                }
                return Loop::Break;
            }
            else
            {
                if (entry->mExpiryTime < current_time)
                {
                    mLookupTable.ReleaseObject(entry);
                }

                return Loop::Continue;
            }
        });

        return result;
    }
};

#ifndef MDNS_LOGGING
#undef MdnsLogProgress
#endif

} // namespace Dnssd
} // namespace chip
