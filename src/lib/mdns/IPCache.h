#pragma once

#include <cstdint>

#include <core/PeerId.h>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <core/CHIPError.h>
#include <system/SystemTimer.h>
#include <system/TimeSource.h>

namespace chip {
namespace Mdns {

template<size_t IPCACHE_SIZE, uint64_t IPCACHE_TTL_MS>
class IPCache
{
public:
    IPCache() : mOccupancy(0)
    {
        for (IPCacheEntry & e: mLookupTable)
        {
            e.nodeId = kUndefinedNodeId;
        }
    }

    CHIP_ERROR Insert(NodeId nodeId, FabricId fabricId, const Inet::IPAddress & addr, uint16_t port, Inet::InterfaceId iface)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        IPCacheEntry * entry = LookupEntry(nodeId, currentTime);
        if (!entry) entry = LookupAvailableEntry(currentTime);
        if (!entry) return CHIP_ERROR_TOO_MANY_KEYS;

        if (entry->nodeId == kUndefinedNodeId)
	{
            mOccupancy++;
        }

	entry->nodeId = nodeId;
        entry->fabricId = fabricId;
        entry->ipAddr = addr;
        entry->port = port;
        entry->ifaceId = iface;

        entry->expiry = currentTime + IPCACHE_TTL_MS;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Delete(NodeId nodeId)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        IPCacheEntry * entry = LookupEntry(nodeId, currentTime);
        if (!entry) return CHIP_ERROR_KEY_NOT_FOUND;

        entry->nodeId = kUndefinedNodeId;
        mOccupancy--;

        return CHIP_NO_ERROR;
    }

    size_t GetOccupancy() { return mOccupancy; }

    CHIP_ERROR Lookup(NodeId nodeId, FabricId & fabricId, Inet::IPAddress & addr, uint16_t & port, Inet::InterfaceId & iface)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        IPCacheEntry * entry = LookupEntry(nodeId, currentTime);
        if (!entry) return CHIP_ERROR_KEY_NOT_FOUND;

        fabricId = entry->fabricId;
        addr = entry->ipAddr;
        port = entry->port;
        iface = entry->ifaceId;

        return CHIP_NO_ERROR;
    }

private:
    struct IPCacheEntry
    {
        NodeId nodeId;
        FabricId fabricId;
        Inet::IPAddress ipAddr;
        uint16_t port;
        Inet::InterfaceId ifaceId;
        uint64_t expiry;
    };

    IPCacheEntry * LookupEntry(NodeId nodeId, uint64_t currentTime)
    {
        NodeId idx = nodeId % IPCACHE_SIZE;
        for (auto & _: mLookupTable)
        {
           (void) _;

            IPCacheEntry *entry = &mLookupTable[idx];

            if (entry->nodeId == nodeId && entry->expiry >= currentTime)
                return entry;

            idx++;
            idx %= IPCACHE_SIZE;
        }
        return nullptr;
    }

    IPCacheEntry * LookupAvailableEntry(uint64_t currentTime)
    {
        for (IPCacheEntry & entry: mLookupTable)
        {
            if (entry.nodeId == kUndefinedNodeId || entry.expiry < currentTime)
                return &entry;
        }
        return nullptr;
    }

    IPCacheEntry mLookupTable[IPCACHE_SIZE];
    size_t mOccupancy;
    Time::TimeSource<Time::Source::kSystem> mTimeSource;
};

} // namespace mdns
} // namespace chip
