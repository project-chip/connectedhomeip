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

#include "CommissioningProxyBgScanRegistry.h"

#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using Protocols::InteractionModel::Status;

bool CommissioningProxyBgScanRegistry::FabricKey::operator<(const FabricKey & o) const
{
    if (fabricIndex != o.fabricIndex)
    {
        return fabricIndex < o.fabricIndex;
    }
    return nodeId < o.nodeId;
}

void CommissioningProxyBgScanRegistry::CancelLifetime(Record & rec)
{
    if (rec.lifetimeCtx != nullptr)
    {
        DeviceLayer::SystemLayer().CancelTimer(LifetimeExpiryCallback, rec.lifetimeCtx);
        delete rec.lifetimeCtx;
        rec.lifetimeCtx = nullptr;
    }
}

void CommissioningProxyBgScanRegistry::OnBecameEmpty()
{
    // Stop the radio only if this registry currently owns it; if paused, a connect or
    // foreground scan owns it and must not be disturbed. The cached results are dropped
    // in either case — no fabric wants them refreshed any more.
    if (!mPaused)
    {
        mHardware.StopHardwareScan();
    }
    mPaused = false;
    mHardware.ClearCachedResults();
}

Status CommissioningProxyBgScanRegistry::Start(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                               BitMask<WiFiBandBitmap> wiFiBands, uint16_t timeoutSecs)
{
    const FabricKey key{ fabricIndex, nodeId };
    const bool wasEmpty = mFabrics.empty();

    auto it = mFabrics.find(key);
    if (it != mFabrics.end() && it->second.lifetimeCtx != nullptr)
    {
        // Refreshing an existing fabric: drop its old lifetime timer before re-arming.
        CancelLifetime(it->second);
    }

    Record & rec    = mFabrics[key];
    rec.transport   = transport;
    rec.wiFiBands   = wiFiBands;
    rec.lifetimeCtx = nullptr;

    // Start (or resume) the hardware scan on the first fabric, or whenever we are
    // paused/deferred. BUSY means the radio is held elsewhere; keep the fabric
    // registered and stay paused so ResumeIfNeeded() restarts it once it frees up.
    if (wasEmpty || mPaused)
    {
        CHIP_ERROR err = mHardware.StartHardwareScan();
        if (err == CHIP_ERROR_BUSY)
        {
            mPaused = true;
            ChipLogProgress(AppServer, "BgScan: radio busy, deferring hardware scan (will resume when free)");
        }
        else if (err != CHIP_NO_ERROR)
        {
            // Hard failure: nothing was started, so do not stop the radio or clear the
            // cache — just drop the fabric we optimistically added and reject.
            ChipLogError(AppServer, "BgScan: StartHardwareScan failed: %" CHIP_ERROR_FORMAT, err.Format());
            mFabrics.erase(key);
            return Status::Failure;
        }
        else
        {
            mPaused = false;
        }
    }

    if (timeoutSecs > 0)
    {
        auto * ctx = new LifetimeCtx{ this, key };
        CHIP_ERROR timerErr =
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(timeoutSecs), LifetimeExpiryCallback, ctx);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "BgScan: lifetime StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            delete ctx;
            // Without a lifetime timer the hardware scan would run indefinitely for this
            // fabric; drop it rather than leave an unbounded scan behind.
            mFabrics.erase(key);
            if (mFabrics.empty())
            {
                OnBecameEmpty();
            }
            return Status::Failure;
        }
        mFabrics[key].lifetimeCtx = ctx;
    }

    return Status::Success;
}

Status CommissioningProxyBgScanRegistry::Stop(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                              BitMask<WiFiBandBitmap> wiFiBands)
{
    const FabricKey key{ fabricIndex, nodeId };
    auto it = mFabrics.find(key);
    if (it == mFabrics.end())
    {
        return Status::NotFound;
    }

    const uint8_t reqTransportBits = transport.Raw();
    const uint16_t reqBandBits     = wiFiBands.Raw();
    const uint8_t fabTransportBits = it->second.transport.Raw();
    const uint16_t fabBandBits     = it->second.wiFiBands.Raw();

    // A transport bitmap of zero means "stop only the given bands" (spec); otherwise
    // stop the intersection of the requested and registered transports/bands.
    const uint8_t stopTransportBits = (reqTransportBits == 0) ? 0 : static_cast<uint8_t>(reqTransportBits & fabTransportBits);
    const uint16_t stopBandBits     = static_cast<uint16_t>(reqBandBits & fabBandBits);

    if (stopTransportBits == 0 && stopBandBits == 0)
    {
        // Nothing the caller asked to stop was actually being scanned for this fabric.
        return Status::Success;
    }

    const uint8_t remainTransport = static_cast<uint8_t>(fabTransportBits & ~stopTransportBits);
    const uint16_t remainBands    = static_cast<uint16_t>(fabBandBits & ~stopBandBits);
    const bool fabricNowEmpty     = (remainTransport == 0 || remainBands == 0);

    if (fabricNowEmpty)
    {
        CancelLifetime(it->second);
        mFabrics.erase(it);
    }
    else
    {
        it->second.transport = BitMask<CapabilitiesBitmap>(remainTransport);
        it->second.wiFiBands = BitMask<WiFiBandBitmap>(remainBands);
    }

    // If no remaining fabric still covers the stopped transport/bands and the registry
    // is now empty, tear the hardware scan down.
    bool otherFabricCovers = false;
    for (const auto & [otherKey, rec] : mFabrics)
    {
        if ((rec.transport.Raw() & stopTransportBits) != 0 || (rec.wiFiBands.Raw() & stopBandBits) != 0)
        {
            otherFabricCovers = true;
            break;
        }
    }
    if (!otherFabricCovers && mFabrics.empty())
    {
        OnBecameEmpty();
    }

    return Status::Success;
}

void CommissioningProxyBgScanRegistry::Pause()
{
    if (!mFabrics.empty() && !mPaused)
    {
        mHardware.StopHardwareScan();
        mPaused = true;
        ChipLogProgress(AppServer, "BgScan: paused (radio needed for connect/foreground scan)");
    }
}

void CommissioningProxyBgScanRegistry::ResumeIfNeeded()
{
    if (!mPaused || mFabrics.empty())
    {
        return;
    }
    CHIP_ERROR err = mHardware.StartHardwareScan();
    if (err == CHIP_NO_ERROR)
    {
        mPaused = false;
        ChipLogProgress(AppServer, "BgScan: resumed");
    }
    else if (err == CHIP_ERROR_BUSY)
    {
        // Radio still held; stay paused and retry on the next ResumeIfNeeded().
    }
    else
    {
        ChipLogError(AppServer, "BgScan: resume failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void CommissioningProxyBgScanRegistry::Shutdown()
{
    for (auto & [key, rec] : mFabrics)
    {
        CancelLifetime(rec);
    }
    const bool hadFabrics = !mFabrics.empty();
    mFabrics.clear();
    if (hadFabrics && !mPaused)
    {
        mHardware.StopHardwareScan();
    }
    mPaused = false;
}

void CommissioningProxyBgScanRegistry::LifetimeExpiryCallback(System::Layer * /*layer*/, void * appState)
{
    auto * ctx                              = static_cast<LifetimeCtx *>(appState);
    CommissioningProxyBgScanRegistry * self = ctx->registry;
    const FabricKey key                     = ctx->key;
    self->OnLifetimeExpiry(key);
}

void CommissioningProxyBgScanRegistry::OnLifetimeExpiry(const FabricKey & key)
{
    auto it = mFabrics.find(key);
    if (it == mFabrics.end())
    {
        return;
    }
    // The timer that fired owns the ctx; the timer has already been consumed, so just
    // free the ctx here (do not CancelTimer for the one that just fired).
    if (it->second.lifetimeCtx != nullptr)
    {
        delete it->second.lifetimeCtx;
        it->second.lifetimeCtx = nullptr;
    }
    mFabrics.erase(it);

    ChipLogProgress(AppServer, "BgScan: lifetime expired for fabricIndex=%u nodeId=0x" ChipLogFormatX64, key.fabricIndex,
                    ChipLogValueX64(key.nodeId));

    if (mFabrics.empty())
    {
        OnBecameEmpty();
    }
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
