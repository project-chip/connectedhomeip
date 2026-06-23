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

#include <app-common/zap-generated/cluster-objects.h>

#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class CommissioningProxyCluster;

// Shared background-scan result cache for the Linux commissioning-proxy example.
//
// CachedResults is a single cluster-wide list keyed on
// discriminator/VendorID/ProductID/Transport (CommissioningProxy.adoc,
// CachedResults attribute), so all transports share ONE cache here rather than
// each keeping its own.  This gives a single combined MaxCachedResults cap and a
// single NumCachedResults/CachedResults view, and one TTL sweep timer instead of
// one timer per cached device.
//
// Thread/locking: every entry point must be called with the CHIP stack lock held
// (BLE discovery arrives via the platform ProxyScanForwarder which locks; PAF
// discovery and all timer/attribute callbacks already run on the Matter thread).
namespace BgScanCache {

using ScanResultEntry = Structs::ScanResultStruct::Type;

// Insert or refresh a discovered device.  `result.transport` carries the single
// transport bit.  Refreshes the TTL if already cached; otherwise inserts subject
// to the cluster-wide MaxCachedResults cap and marks CachedResults dirty.
// CacheTimeout/MaxCachedResults are read from the cluster delegate.
void Report(const ScanResultEntry & result, CommissioningProxyCluster * cluster);

// Remove all cached entries whose transport bit is in `transport` (called when a
// transport stops background scanning).  Marks CachedResults dirty if anything
// was removed.
void ClearTransport(chip::BitMask<CapabilitiesBitmap> transport, CommissioningProxyCluster * cluster);

// NumCachedResults: combined count across all transports.
uint8_t Count();

// CachedResults: append every cached entry.  The returned structs' ByteSpans
// point into the cache's own storage, which stays valid for a synchronous encode.
void Collect(std::vector<ScanResultEntry> & out);

} // namespace BgScanCache
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
