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

#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>
#include <cstring>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

namespace {
// One periodic sweep (1 s granularity) reaps expired entries while the cache is
// non-empty — far cheaper than a per-device TTL timer re-armed on every advert.
constexpr System::Clock::Timeout kSweepInterval = System::Clock::Seconds16(1);
} // namespace

bool CommissioningProxyScanCache::Key::operator==(const Key & o) const
{
    return transport == o.transport && discriminator == o.discriminator && vid == o.vid && pid == o.pid;
}

CommissioningProxyScanCache::Entry * CommissioningProxyScanCache::FindEntry(const Key & key)
{
    for (auto & entry : mEntries)
    {
        if (entry.inUse && entry.key == key)
        {
            return &entry;
        }
    }
    return nullptr;
}

void CommissioningProxyScanCache::ArmSweepIfNeeded()
{
    if (mSweepArmed || Count() == 0)
    {
        return;
    }
    if (mTimerDelegate.StartTimer(this, kSweepInterval) == CHIP_NO_ERROR)
    {
        mSweepArmed = true;
    }
}

void CommissioningProxyScanCache::OnSweep()
{
    mSweepArmed = false;

    auto now     = mTimerDelegate.GetCurrentMonotonicTimestamp();
    bool removed = false;
    for (auto & entry : mEntries)
    {
        if (entry.inUse && entry.expiresAt <= now)
        {
            ChipLogProgress(Zcl, "CommissioningProxyScanCache: TTL expired for discriminator %u (transport 0x%x)",
                            entry.key.discriminator, entry.key.transport);
            entry.inUse = false;
            removed     = true;
        }
    }

    if (removed)
    {
        mCluster.MarkCachedResultsDirty();
    }

    ArmSweepIfNeeded();
}

void CommissioningProxyScanCache::Report(const ScanResultEntry & result)
{
    Key key{ static_cast<uint8_t>(result.transport.Raw()), result.discriminator, static_cast<uint16_t>(result.vendorID),
             result.productID };

    auto expiresAt = mTimerDelegate.GetCurrentMonotonicTimestamp() + System::Clock::Seconds16(mCluster.GetCacheTimeout());

    if (Entry * existing = FindEntry(key))
    {
        // Re-discovery: refresh TTL only (the visible result is unchanged, so no dirty).
        existing->expiresAt = expiresAt;
        ArmSweepIfNeeded();
        return;
    }

    Entry * slot = nullptr;
    for (auto & candidate : mEntries)
    {
        if (!candidate.inUse)
        {
            slot = &candidate;
            break;
        }
    }
    if (slot == nullptr)
    {
        ChipLogDetail(Zcl, "CommissioningProxyScanCache: full (%u entries), dropping discriminator %u",
                      mCluster.GetMaxCachedResults(), key.discriminator);
        return;
    }

    *slot           = Entry{};
    slot->inUse     = true;
    slot->key       = key;
    slot->transport = result.transport;
    if (!result.address.IsNull())
    {
        auto span            = result.address.Value();
        const size_t copyLen = std::min(span.size(), kMaxAddressBytes);
        memcpy(slot->address, span.data(), copyLen);
        slot->addressLen = static_cast<uint8_t>(copyLen);
        slot->hasAddress = true;
    }
    slot->discriminator = result.discriminator;
    slot->vendorID      = result.vendorID;
    slot->productID     = result.productID;
    if (!result.extendedData.IsNull())
    {
        auto span            = result.extendedData.Value();
        const size_t copyLen = std::min(span.size(), kMaxExtendedDataBytes);
        memcpy(slot->extendedData, span.data(), copyLen);
        slot->extendedDataLen = static_cast<uint8_t>(copyLen);
        slot->hasExtendedData = true;
    }
    slot->wiFiBand  = result.wiFiBand;
    slot->expiresAt = expiresAt;

    ChipLogProgress(Zcl, "CommissioningProxyScanCache: cached discriminator %u (transport 0x%x, total=%u)", key.discriminator,
                    key.transport, Count());

    mCluster.MarkCachedResultsDirty();
    ArmSweepIfNeeded();
}

void CommissioningProxyScanCache::ClearTransport(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> bands)
{
    // Only PAFTP results carry a band, so a BLE stop always arrives with bands == 0.
    const BitMask<WiFiBandBitmap> kBandFallback{ WiFiBandBitmap::k2g4 };

    bool removed = false;
    for (auto & entry : mEntries)
    {
        if (!entry.inUse)
        {
            continue;
        }
        const bool bandMatches = bands.Raw() == 0 || (entry.wiFiBand.ValueOr(kBandFallback).Raw() & bands.Raw()) != 0;
        if ((entry.key.transport & transport.Raw()) != 0 && bandMatches)
        {
            entry.inUse = false;
            removed     = true;
        }
    }

    if (removed)
    {
        mCluster.MarkCachedResultsDirty();
    }
}

uint8_t CommissioningProxyScanCache::Count() const
{
    uint8_t count = 0;
    for (const auto & entry : mEntries)
    {
        count = static_cast<uint8_t>(count + (entry.inUse ? 1 : 0));
    }
    return count;
}

CHIP_ERROR CommissioningProxyScanCache::Encode(AttributeValueEncoder & encoder) const
{
    if (Count() == 0)
    {
        DataModel::Nullable<DataModel::List<const ScanResultEntry>> nullValue;
        return encoder.Encode(nullValue);
    }

    return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
        for (const auto & e : mEntries)
        {
            if (!e.inUse)
            {
                continue;
            }
            ScanResultEntry r{};
            if (e.hasAddress)
            {
                r.address.SetNonNull(ByteSpan(e.address, e.addressLen));
            }
            else
            {
                r.address.SetNull();
            }
            r.transport     = e.transport;
            r.discriminator = e.discriminator;
            r.vendorID      = e.vendorID;
            r.productID     = e.productID;
            if (e.hasExtendedData)
            {
                r.extendedData.SetNonNull(ByteSpan(e.extendedData, e.extendedDataLen));
            }
            else
            {
                r.extendedData.SetNull();
            }
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
        mTimerDelegate.CancelTimer(this);
        mSweepArmed = false;
    }
    for (auto & entry : mEntries)
    {
        entry.inUse = false;
    }
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
