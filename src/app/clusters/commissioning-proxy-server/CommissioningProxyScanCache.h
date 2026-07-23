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

#pragma once

#include <app/AttributeValueEncoder.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/BitMask.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

#include <cstdint>
#include <map>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief Callbacks from CommissioningProxyScanCache to its owning cluster.
 *
 * Defined here (not in CommissioningProxyCluster.h) so CommissioningProxyScanCache.cpp
 * can call back without including the full cluster header, which lives in a
 * separate GN target compiled in the consumer's context.
 */
class ScanCacheObserver
{
public:
    virtual ~ScanCacheObserver()                = default;
    virtual void MarkCachedResultsDirty()       = 0;
    virtual uint16_t GetCacheTimeout() const    = 0;
    virtual uint8_t GetMaxCachedResults() const = 0;
};

/**
 * @brief Transport-agnostic background-scan result cache.
 *
 * Backs the CachedResults / NumCachedResults attributes. Per spec, CachedResults is
 * a single cluster-wide list keyed on discriminator/VendorID/ProductID/Transport, so
 * every transport shares ONE cache here — one combined MaxCachedResults cap, one
 * NumCachedResults/CachedResults view, and one TTL sweep timer.
 *
 * CacheTimeout and MaxCachedResults are read back from the owning cluster; whenever
 * the set of entries changes the cache calls the cluster's MarkCachedResultsDirty()
 * so change-reporting stays the cluster's responsibility (delegates no longer touch
 * NumCachedResults/CachedResults directly).
 *
 * All entry points must run on the Matter thread with the stack lock held.
 */
class CommissioningProxyScanCache
{
public:
    using ScanResultEntry = Structs::ScanResultStruct::Type;

    explicit CommissioningProxyScanCache(ScanCacheObserver & cluster) : mCluster(cluster) {}
    ~CommissioningProxyScanCache() = default;

    /**
     * @brief Insert or refresh a discovered device. @p result.transport carries the
     *        single discovering transport bit. Refreshes the entry's TTL if already
     *        cached; otherwise inserts subject to the MaxCachedResults cap. Marks
     *        CachedResults dirty on any change and (re)arms the sweep timer.
     */
    void Report(const ScanResultEntry & result);

    /// Remove all entries whose transport bit is in @p transport (a transport
    /// stopping its background scan). Marks dirty if anything was removed.
    void ClearTransport(BitMask<CapabilitiesBitmap> transport);

    /// NumCachedResults: current combined entry count.
    uint8_t Count() const;

    /// Encode the CachedResults list attribute (NullNullable when empty).
    CHIP_ERROR Encode(app::AttributeValueEncoder & encoder) const;

    /// Cancel the sweep timer and drop all entries (cluster teardown).
    void Shutdown();

private:
    // A device is unique per discriminator/VendorID/ProductID/Transport (spec).
    struct Key
    {
        uint8_t transport;
        uint16_t discriminator;
        uint16_t vid;
        uint16_t pid;
        bool operator<(const Key & o) const;
    };

    // Self-owned copy of a ScanResultStruct so its ByteSpans survive in the cache.
    struct Entry
    {
        bool hasAddress = false;
        std::vector<uint8_t> address;
        BitMask<CapabilitiesBitmap> transport{};
        uint16_t discriminator = 0;
        VendorId vendorID      = static_cast<VendorId>(0);
        uint16_t productID     = 0;
        bool hasExtendedData   = false;
        std::vector<uint8_t> extendedData;
        Optional<BitMask<WiFiBandBitmap>> wiFiBand;
        System::Clock::Timestamp expiresAt{};
    };

    static void SweepTimerCallback(System::Layer * layer, void * appState);
    void OnSweep();
    void ArmSweepIfNeeded();

    ScanCacheObserver & mCluster;
    std::map<Key, Entry> mEntries;
    bool mSweepArmed = false;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
