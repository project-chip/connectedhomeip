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
#include <system/SystemClock.h>

#include <algorithm>
#include <chrono>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using Protocols::InteractionModel::Status;

namespace {
// Timestamp subtraction is 64-bit; StartTimer takes the 32-bit Timeout.
System::Clock::Timeout RemainingUntil(System::Clock::Timestamp deadline, System::Clock::Timestamp now)
{
    return (deadline > now) ? std::chrono::duration_cast<System::Clock::Timeout>(deadline - now) : System::Clock::Timeout(0);
}
} // namespace

void CommissioningProxyBgScanRegistry::CancelLifetime(FabricState & state)
{
    if (state.lifetimeCtx != nullptr)
    {
        mTimerDelegate.CancelTimer(state.lifetimeCtx);
        delete state.lifetimeCtx;
        state.lifetimeCtx = nullptr;
    }
}

bool CommissioningProxyBgScanRegistry::LatestDeadline(const FabricState & state, System::Clock::Timestamp & out)
{
    System::Clock::Timestamp latest{ 0 };
    for (const auto & [nodeId, req] : state.requests)
    {
        if (!req.hasTimeout)
        {
            return false; // one request never expires, so the fabric never does
        }
        latest = std::max(latest, req.expiresAt);
    }
    out = latest;
    return !state.requests.empty();
}

void CommissioningProxyBgScanRegistry::RecomputeFabricLifetime(FabricIndex fabricIndex, FabricState & state)
{
    CancelLifetime(state);

    System::Clock::Timestamp deadline{ 0 };
    if (!LatestDeadline(state, deadline))
    {
        return;
    }

    auto * ctx = new LifetimeCtx(this, fabricIndex);
    if (mTimerDelegate.StartTimer(ctx, RemainingUntil(deadline, mTimerDelegate.GetCurrentMonotonicTimestamp())) != CHIP_NO_ERROR)
    {
        // The fabric would otherwise scan unbounded. Nothing here can reject the command
        // that got us here (it has already been applied), so drop the fabric instead.
        ChipLogError(AppServer, "BgScan: could not re-arm lifetime for fabricIndex=%u; dropping it", fabricIndex);
        delete ctx;
        state.requests.clear();
        return;
    }
    state.lifetimeCtx = ctx;
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
    mHardware.ClearCachedResults({}); // the transport stopped, so all of its results go
}

BitMask<WiFiBandBitmap> CommissioningProxyBgScanRegistry::BandsInUse() const
{
    uint16_t bits = 0;
    for (const auto & [fabricIndex, state] : mFabrics)
    {
        for (const auto & [nodeId, req] : state.requests)
        {
            bits = static_cast<uint16_t>(bits | req.wiFiBands.Raw());
        }
    }
    return BitMask<WiFiBandBitmap>(bits);
}

void CommissioningProxyBgScanRegistry::ClearBandsNoLongerScanned(BitMask<WiFiBandBitmap> candidates)
{
    // Spec: clear the results for the bands scanning has stopped on — which is only the
    // bands no other request, on this or any other fabric, still asks for.
    const uint16_t stopped = static_cast<uint16_t>(candidates.Raw() & ~BandsInUse().Raw());
    if (stopped != 0)
    {
        mHardware.ClearCachedResults(BitMask<WiFiBandBitmap>(stopped));
    }
}

Status CommissioningProxyBgScanRegistry::Start(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                               BitMask<WiFiBandBitmap> wiFiBands, uint16_t timeoutSecs)
{
    const bool wasEmpty = mFabrics.empty();

    // Reject before any side effect: a fabric may only hold so many concurrent requests,
    // and a node already holding one is refreshing rather than adding.
    auto fabricIt = mFabrics.find(fabricIndex);
    if (fabricIt != mFabrics.end() && fabricIt->second.requests.find(nodeId) == fabricIt->second.requests.end() &&
        fabricIt->second.requests.size() >= CHIP_CONFIG_COMMISSIONING_PROXY_MAX_BGSCAN_REQUESTS_PER_FABRIC)
    {
        ChipLogError(AppServer, "BgScan: fabricIndex=%u already has %u background scans", fabricIndex,
                     static_cast<unsigned>(fabricIt->second.requests.size()));
        return Status::ResourceExhausted;
    }

    // Start (or resume) the hardware scan on the first fabric, or whenever we are
    // paused/deferred. BUSY means the radio is held elsewhere; register the fabric
    // anyway and stay paused so ResumeIfNeeded() restarts it once it frees up.
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
            ChipLogError(AppServer, "BgScan: StartHardwareScan failed: %" CHIP_ERROR_FORMAT, err.Format());
            return Status::Failure;
        }
        else
        {
            mPaused = false;
        }
    }

    // Work out the fabric's deadline as it will be once this request is applied, so the
    // timer can be armed before anything is committed.
    const auto now = mTimerDelegate.GetCurrentMonotonicTimestamp();
    Request incoming;
    incoming.transport  = transport;
    incoming.wiFiBands  = wiFiBands;
    incoming.hasTimeout = (timeoutSecs > 0);
    incoming.expiresAt  = now + System::Clock::Seconds16(timeoutSecs);

    bool needTimer                    = incoming.hasTimeout;
    System::Clock::Timestamp deadline = incoming.expiresAt;
    if (fabricIt != mFabrics.end())
    {
        for (const auto & [otherNode, req] : fabricIt->second.requests)
        {
            if (otherNode == nodeId)
            {
                continue; // this request is about to be replaced
            }
            if (!req.hasTimeout)
            {
                needTimer = false;
                break;
            }
            deadline = std::max(deadline, req.expiresAt);
        }
    }

    LifetimeCtx * ctx = nullptr;
    if (needTimer)
    {
        ctx                 = new LifetimeCtx(this, fabricIndex);
        CHIP_ERROR timerErr = mTimerDelegate.StartTimer(ctx, RemainingUntil(deadline, now));
        if (timerErr != CHIP_NO_ERROR)
        {
            // Without a lifetime timer the hardware scan would run unbounded, so reject.
            // If this call is what started the radio and no fabric is registered to keep
            // it running, undo that too.
            ChipLogError(AppServer, "BgScan: lifetime StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            delete ctx;
            if (mFabrics.empty())
            {
                OnBecameEmpty();
            }
            return Status::Failure;
        }
    }

    // From here nothing can fail: the hardware scan is running and the fabric's new
    // timer is armed, so the table is safe to modify.
    //
    // Each node on a fabric keeps its own request and the fabric scans the union of
    // them, so this adds a request or replaces only this node's previous one. The
    // fabric's single timer is swapped for the one armed above, which covers the
    // latest deadline across all of its requests.
    FabricState & state = mFabrics[fabricIndex];
    CancelLifetime(state);
    state.requests[nodeId] = incoming;
    state.lifetimeCtx      = ctx;

    return Status::Success;
}

Status CommissioningProxyBgScanRegistry::Stop(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                              BitMask<WiFiBandBitmap> wiFiBands)
{
    auto fabricIt = mFabrics.find(fabricIndex);
    if (fabricIt == mFabrics.end())
    {
        return Status::NotFound;
    }

    // Spec: if the client's NodeID and FabricID do not match those recorded when the
    // scan was started, take no action and reject with NOT_FOUND.
    FabricState & state = fabricIt->second;
    auto reqIt          = state.requests.find(nodeId);
    if (reqIt == state.requests.end())
    {
        return Status::NotFound;
    }

    const uint8_t reqTransportBits = transport.Raw();
    const uint16_t reqBandBits     = wiFiBands.Raw();
    const uint8_t ownTransportBits = reqIt->second.transport.Raw();
    const uint16_t ownBandBits     = reqIt->second.wiFiBands.Raw();

    // A transport bitmap of zero means "stop only the given bands" (spec); otherwise
    // stop the intersection of the requested and this node's own transports/bands.
    const uint8_t stopTransportBits = (reqTransportBits == 0) ? 0 : static_cast<uint8_t>(reqTransportBits & ownTransportBits);
    const uint16_t stopBandBits     = static_cast<uint16_t>(reqBandBits & ownBandBits);

    if (stopTransportBits == 0 && stopBandBits == 0)
    {
        // Nothing the caller asked to stop was actually being scanned for this node.
        return Status::Success;
    }

    const uint8_t remainTransport = static_cast<uint8_t>(ownTransportBits & ~stopTransportBits);
    const uint16_t remainBands    = static_cast<uint16_t>(ownBandBits & ~stopBandBits);
    // A request holds one transport bit (see Start), so the first term decides whenever
    // the stop names this transport; remainBands only fires on a band-only stop that
    // clears the last band.
    if (remainTransport == 0 || remainBands == 0)
    {
        state.requests.erase(reqIt);
    }
    else
    {
        reqIt->second.transport = BitMask<CapabilitiesBitmap>(remainTransport);
        reqIt->second.wiFiBands = BitMask<WiFiBandBitmap>(remainBands);
    }

    // Dropping a request can shorten the fabric back to a surviving request's deadline,
    // so the timer is recomputed rather than left where the removed request put it.
    if (state.requests.empty())
    {
        CancelLifetime(state);
        mFabrics.erase(fabricIt);
    }
    else
    {
        RecomputeFabricLifetime(fabricIndex, state);
    }

    // Other requests — on this fabric or another — may still want the radio, so it is
    // only torn down once nothing is left. Short of that, results for bands nobody
    // scans any more are still dropped.
    if (mFabrics.empty())
    {
        OnBecameEmpty();
    }
    else
    {
        ClearBandsNoLongerScanned(BitMask<WiFiBandBitmap>(stopBandBits));
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

void CommissioningProxyBgScanRegistry::OnLifetimeExpiry(FabricIndex fabricIndex)
{
    auto it = mFabrics.find(fabricIndex);
    if (it == mFabrics.end())
    {
        return;
    }
    // The fabric's timer sits at the latest deadline of all its requests, so when it
    // fires every one of them has expired and the whole fabric goes. The timer that
    // fired owns the ctx and has already been consumed, so just free it here (do not
    // CancelTimer for the one that just fired).
    if (it->second.lifetimeCtx != nullptr)
    {
        delete it->second.lifetimeCtx;
        it->second.lifetimeCtx = nullptr;
    }
    // Spec: when the fabric's Timeout elapses its cached results are cleared — but only
    // for bands no surviving fabric still scans.
    uint16_t expiringBands = 0;
    for (const auto & [nodeId, req] : it->second.requests)
    {
        expiringBands = static_cast<uint16_t>(expiringBands | req.wiFiBands.Raw());
    }
    mFabrics.erase(it);

    ChipLogProgress(AppServer, "BgScan: lifetime expired for fabricIndex=%u", fabricIndex);

    if (mFabrics.empty())
    {
        OnBecameEmpty();
    }
    else
    {
        ClearBandsNoLongerScanned(BitMask<WiFiBandBitmap>(expiringBands));
    }
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
