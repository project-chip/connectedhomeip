/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
#include "CommissioningProxyBgScanCache.h"

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {
namespace BgScanCache {
namespace {

// Cache key: a device is unique per discriminator/VendorID/ProductID/Transport.
struct Key
{
    uint8_t transport;
    uint16_t discriminator;
    uint16_t vid;
    uint16_t pid;

    bool operator<(const Key & o) const
    {
        if (transport != o.transport)
            return transport < o.transport;
        if (discriminator != o.discriminator)
            return discriminator < o.discriminator;
        if (vid != o.vid)
            return vid < o.vid;
        return pid < o.pid;
    }
};

// Self-owned copy of a ScanResultStruct so its ByteSpans survive in the cache.
struct Entry
{
    bool hasAddress = false;
    std::vector<uint8_t> address;
    chip::BitMask<CapabilitiesBitmap> transport{};
    uint16_t discriminator  = 0;
    chip::VendorId vendorID = static_cast<chip::VendorId>(0);
    uint16_t productID      = 0;
    bool hasExtendedData    = false;
    std::vector<uint8_t> extendedData;
    chip::Optional<chip::BitMask<WiFiBandBitmap>> wiFiBand;
    chip::System::Clock::Timestamp expiresAt{};
};

std::map<Key, Entry> sCache;
CommissioningProxyCluster * sCluster = nullptr;
bool sSweepArmed                     = false;

// Fallbacks used only on the defensive cluster == nullptr path; they mirror the
// spec CacheTimeout default and the delegate's MaxCachedResults.
constexpr uint16_t kDefaultCacheTimeoutSecs = 120;
constexpr uint8_t kDefaultMaxCachedResults  = 10;

// One periodic sweep (1 s granularity) reaps expired entries while the cache is
// non-empty — far cheaper than a per-device TTL timer re-armed on every advert.
constexpr chip::System::Clock::Timeout kSweepInterval = chip::System::Clock::Seconds16(1);

void OnSweep(chip::System::Layer * layer, void * appState);

void ArmSweepIfNeeded()
{
    if (sSweepArmed || sCache.empty())
        return;
    if (chip::DeviceLayer::SystemLayer().StartTimer(kSweepInterval, OnSweep, nullptr) == CHIP_NO_ERROR)
        sSweepArmed = true;
}

void OnSweep(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    sSweepArmed = false;

    auto now     = chip::System::SystemClock().GetMonotonicTimestamp();
    bool removed = false;
    for (auto it = sCache.begin(); it != sCache.end();)
    {
        if (it->second.expiresAt <= now)
        {
            ChipLogProgress(AppServer, "BgScanCache: TTL expired for discriminator %u (transport 0x%x)", it->first.discriminator,
                            it->first.transport);
            it      = sCache.erase(it);
            removed = true;
        }
        else
        {
            ++it;
        }
    }

    if (removed && sCluster != nullptr)
        sCluster->MarkCachedResultsDirty();

    ArmSweepIfNeeded();
}

Entry EntryFromResult(const ScanResultEntry & r, chip::System::Clock::Timestamp expiresAt)
{
    Entry e;
    if (!r.address.IsNull())
    {
        auto span = r.address.Value();
        e.address.assign(span.data(), span.data() + span.size());
        e.hasAddress = true;
    }
    e.transport     = r.transport;
    e.discriminator = r.discriminator;
    e.vendorID      = r.vendorID;
    e.productID     = r.productID;
    if (!r.extendedData.IsNull())
    {
        auto span = r.extendedData.Value();
        e.extendedData.assign(span.data(), span.data() + span.size());
        e.hasExtendedData = true;
    }
    e.wiFiBand  = r.wiFiBand;
    e.expiresAt = expiresAt;
    return e;
}

ScanResultEntry ResultFromEntry(const Entry & e)
{
    ScanResultEntry r{};
    if (e.hasAddress)
        r.address.SetNonNull(chip::ByteSpan(e.address.data(), e.address.size()));
    else
        r.address.SetNull();
    r.transport     = e.transport;
    r.discriminator = e.discriminator;
    r.vendorID      = e.vendorID;
    r.productID     = e.productID;
    if (e.hasExtendedData)
        r.extendedData.SetNonNull(chip::ByteSpan(e.extendedData.data(), e.extendedData.size()));
    else
        r.extendedData.SetNull();
    r.wiFiBand = e.wiFiBand;
    return r;
}

} // namespace

void Report(const ScanResultEntry & result, CommissioningProxyCluster * cluster)
{
    if (sCluster == nullptr)
        sCluster = cluster;

    Key key{ static_cast<uint8_t>(result.transport.Raw()), result.discriminator, static_cast<uint16_t>(result.vendorID),
             result.productID };

    uint16_t cacheTimeout =
        (cluster != nullptr) ? static_cast<uint16_t>(cluster->GetDelegate().GetCacheTimeout()) : kDefaultCacheTimeoutSecs;
    auto expiresAt        = chip::System::SystemClock().GetMonotonicTimestamp() + chip::System::Clock::Seconds16(cacheTimeout);

    auto it = sCache.find(key);
    if (it != sCache.end())
    {
        // Re-discovery: refresh TTL only (no dirty — the visible result is unchanged).
        it->second.expiresAt = expiresAt;
        ArmSweepIfNeeded();
        return;
    }

    uint8_t maxResults = (cluster != nullptr) ? cluster->GetDelegate().GetMaxCachedResults() : kDefaultMaxCachedResults;
    if (sCache.size() >= static_cast<size_t>(maxResults))
    {
        ChipLogDetail(AppServer, "BgScanCache: full (%u entries), dropping discriminator %u", maxResults, key.discriminator);
        return;
    }

    sCache[key] = EntryFromResult(result, expiresAt);
    ChipLogProgress(AppServer, "BgScanCache: cached discriminator %u (transport 0x%x, total=%zu, TTL=%us)", key.discriminator,
                    key.transport, sCache.size(), cacheTimeout);

    if (cluster != nullptr)
        cluster->MarkCachedResultsDirty();
    ArmSweepIfNeeded();
}

void ClearTransport(chip::BitMask<CapabilitiesBitmap> transport, CommissioningProxyCluster * cluster)
{
    bool removed = false;
    for (auto it = sCache.begin(); it != sCache.end();)
    {
        if ((it->first.transport & transport.Raw()) != 0)
        {
            it      = sCache.erase(it);
            removed = true;
        }
        else
        {
            ++it;
        }
    }

    if (removed && cluster != nullptr)
        cluster->MarkCachedResultsDirty();

    if (sCache.empty())
        sCluster = nullptr;
}

uint8_t Count()
{
    return static_cast<uint8_t>(sCache.size());
}

void Collect(std::vector<ScanResultEntry> & out)
{
    out.reserve(out.size() + sCache.size());
    for (const auto & [key, entry] : sCache)
    {
        out.push_back(ResultFromEntry(entry));
    }
}

void Unregister(CommissioningProxyCluster * cluster)
{
    if (sCluster != cluster)
        return;

    if (sSweepArmed)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(OnSweep, nullptr);
        sSweepArmed = false;
    }
    sCache.clear();
    sCluster = nullptr;
}

} // namespace BgScanCache
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
