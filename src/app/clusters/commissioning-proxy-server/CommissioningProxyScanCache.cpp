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
#include <app/clusters/commissioning-proxy-server/CommissioningProxyScanCache.h>

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

namespace {
// One periodic sweep (1 s granularity) reaps expired entries while the cache is
// non-empty — far cheaper than a per-device TTL timer re-armed on every advert.
constexpr System::Clock::Timeout kSweepInterval = System::Clock::Seconds16(1);
} // namespace

bool CommissioningProxyScanCache::Key::operator<(const Key & o) const
{
    if (transport != o.transport)
        return transport < o.transport;
    if (discriminator != o.discriminator)
        return discriminator < o.discriminator;
    if (vid != o.vid)
        return vid < o.vid;
    return pid < o.pid;
}

void CommissioningProxyScanCache::SweepTimerCallback(System::Layer * /*layer*/, void * appState)
{
    static_cast<CommissioningProxyScanCache *>(appState)->OnSweep();
}

void CommissioningProxyScanCache::ArmSweepIfNeeded()
{
    if (mSweepArmed || mEntries.empty())
        return;
    if (DeviceLayer::SystemLayer().StartTimer(kSweepInterval, SweepTimerCallback, this) == CHIP_NO_ERROR)
        mSweepArmed = true;
}

void CommissioningProxyScanCache::OnSweep()
{
    mSweepArmed = false;

    auto now     = System::SystemClock().GetMonotonicTimestamp();
    bool removed = false;
    for (auto it = mEntries.begin(); it != mEntries.end();)
    {
        if (it->second.expiresAt <= now)
        {
            ChipLogProgress(Zcl, "CommissioningProxyScanCache: TTL expired for discriminator %u (transport 0x%x)",
                            it->first.discriminator, it->first.transport);
            it      = mEntries.erase(it);
            removed = true;
        }
        else
        {
            ++it;
        }
    }

    if (removed)
        mCluster.MarkCachedResultsDirty();

    ArmSweepIfNeeded();
}

void CommissioningProxyScanCache::Report(const ScanResultEntry & result)
{
    Key key{ static_cast<uint8_t>(result.transport.Raw()), result.discriminator, static_cast<uint16_t>(result.vendorID),
             result.productID };

    auto expiresAt = System::SystemClock().GetMonotonicTimestamp() + System::Clock::Seconds16(mCluster.GetCacheTimeout());

    auto it = mEntries.find(key);
    if (it != mEntries.end())
    {
        // Re-discovery: refresh TTL only (the visible result is unchanged, so no dirty).
        it->second.expiresAt = expiresAt;
        ArmSweepIfNeeded();
        return;
    }

    if (mEntries.size() >= static_cast<size_t>(mCluster.GetMaxCachedResults()))
    {
        ChipLogDetail(Zcl, "CommissioningProxyScanCache: full (%u entries), dropping discriminator %u",
                      mCluster.GetMaxCachedResults(), key.discriminator);
        return;
    }

    Entry e;
    if (!result.address.IsNull())
    {
        auto span = result.address.Value();
        e.address.assign(span.data(), span.data() + span.size());
        e.hasAddress = true;
    }
    e.transport     = result.transport;
    e.discriminator = result.discriminator;
    e.vendorID      = result.vendorID;
    e.productID     = result.productID;
    if (!result.extendedData.IsNull())
    {
        auto span = result.extendedData.Value();
        e.extendedData.assign(span.data(), span.data() + span.size());
        e.hasExtendedData = true;
    }
    e.wiFiBand    = result.wiFiBand;
    e.expiresAt   = expiresAt;
    mEntries[key] = std::move(e);

    ChipLogProgress(Zcl, "CommissioningProxyScanCache: cached discriminator %u (transport 0x%x, total=%zu)", key.discriminator,
                    key.transport, mEntries.size());

    mCluster.MarkCachedResultsDirty();
    ArmSweepIfNeeded();
}

void CommissioningProxyScanCache::ClearTransport(BitMask<CapabilitiesBitmap> transport)
{
    bool removed = false;
    for (auto it = mEntries.begin(); it != mEntries.end();)
    {
        if ((it->first.transport & transport.Raw()) != 0)
        {
            it      = mEntries.erase(it);
            removed = true;
        }
        else
        {
            ++it;
        }
    }

    if (removed)
        mCluster.MarkCachedResultsDirty();
}

uint8_t CommissioningProxyScanCache::Count() const
{
    return static_cast<uint8_t>(mEntries.size());
}

CHIP_ERROR CommissioningProxyScanCache::Encode(AttributeValueEncoder & encoder) const
{
    if (mEntries.empty())
    {
        DataModel::Nullable<DataModel::List<const ScanResultEntry>> nullValue;
        return encoder.Encode(nullValue);
    }

    return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
        for (const auto & [key, e] : mEntries)
        {
            ScanResultEntry r{};
            if (e.hasAddress)
                r.address.SetNonNull(ByteSpan(e.address.data(), e.address.size()));
            else
                r.address.SetNull();
            r.transport     = e.transport;
            r.discriminator = e.discriminator;
            r.vendorID      = e.vendorID;
            r.productID     = e.productID;
            if (e.hasExtendedData)
                r.extendedData.SetNonNull(ByteSpan(e.extendedData.data(), e.extendedData.size()));
            else
                r.extendedData.SetNull();
            r.wiFiBand = e.wiFiBand;
            ReturnErrorOnFailure(listEncoder.Encode(r));
        }
        return CHIP_NO_ERROR;
    });
}

void CommissioningProxyScanCache::Shutdown()
{
    if (mSweepArmed)
    {
        DeviceLayer::SystemLayer().CancelTimer(SweepTimerCallback, this);
        mSweepArmed = false;
    }
    mEntries.clear();
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
