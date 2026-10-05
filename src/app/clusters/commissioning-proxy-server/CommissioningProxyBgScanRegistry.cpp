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

size_t CommissioningProxyBgScanRegistry::FabricState::RequestCount() const
{
    size_t count = 0;
    for (const auto & slot : requests)
    {
        if (slot.inUse)
        {
            count++;
        }
    }
    return count;
}

CommissioningProxyBgScanRegistry::RequestSlot * CommissioningProxyBgScanRegistry::FabricState::Find(NodeId nodeId)
{
    for (auto & slot : requests)
    {
        if (slot.inUse && slot.nodeId == nodeId)
        {
            return &slot;
        }
    }
    return nullptr;
}

CommissioningProxyBgScanRegistry::RequestSlot * CommissioningProxyBgScanRegistry::FabricState::FindFree()
{
    for (auto & slot : requests)
    {
        if (!slot.inUse)
        {
            return &slot;
        }
    }
    return nullptr;
}

CommissioningProxyBgScanRegistry::FabricState * CommissioningProxyBgScanRegistry::FindFabric(FabricIndex fabricIndex)
{
    for (auto & fabric : mFabrics)
    {
        if (fabric.inUse && fabric.fabricIndex == fabricIndex)
        {
            return &fabric;
        }
    }
    return nullptr;
}

bool CommissioningProxyBgScanRegistry::AnyFabricInUse() const
{
    for (const auto & fabric : mFabrics)
    {
        if (fabric.inUse)
        {
            return true;
        }
    }
    return false;
}

void CommissioningProxyBgScanRegistry::CancelLifetime(FabricState & state)
{
    if (state.lifetime.armed)
    {
        mTimerDelegate.CancelTimer(&state.lifetime);
        state.lifetime.armed = false;
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
    mHardware.ClearCachedResults({}); // the transport stopped, so all of its results go
}

BitMask<WiFiBandBitmap> CommissioningProxyBgScanRegistry::BandsInUse() const
{
    uint16_t bits = 0;
    for (const auto & fabric : mFabrics)
    {
        if (!fabric.inUse)
        {
            continue;
        }
        for (const auto & slot : fabric.requests)
        {
            if (slot.inUse)
            {
                bits = static_cast<uint16_t>(bits | slot.request.wiFiBands.Raw());
            }
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

bool CommissioningProxyBgScanRegistry::LatestDeadline(const FabricState & state, System::Clock::Timestamp & out)
{
    System::Clock::Timestamp latest{ 0 };
    bool any = false;
    for (const auto & slot : state.requests)
    {
        if (!slot.inUse)
        {
            continue;
        }
        if (!slot.request.hasTimeout)
        {
            return false; // one request never expires, so the fabric never does
        }
        latest = std::max(latest, slot.request.expiresAt);
        any    = true;
    }
    out = latest;
    return any;
}

BitMask<WiFiBandBitmap> CommissioningProxyBgScanRegistry::ReleaseFabric(FabricState & state)
{
    uint16_t releasedBands = 0;
    for (auto & slot : state.requests)
    {
        if (slot.inUse)
        {
            releasedBands = static_cast<uint16_t>(releasedBands | slot.request.wiFiBands.Raw());
            slot.inUse    = false;
        }
    }
    CancelLifetime(state);
    state.inUse = false;
    return BitMask<WiFiBandBitmap>(releasedBands);
}

BitMask<WiFiBandBitmap> CommissioningProxyBgScanRegistry::RecomputeFabricLifetime(FabricIndex fabricIndex, FabricState & state)
{
    CancelLifetime(state);

    System::Clock::Timestamp deadline{ 0 };
    if (!LatestDeadline(state, deadline))
    {
        return {};
    }

    state.lifetime.registry    = this;
    state.lifetime.fabricIndex = fabricIndex;
    if (mTimerDelegate.StartTimer(&state.lifetime, RemainingUntil(deadline, mTimerDelegate.GetCurrentMonotonicTimestamp())) !=
        CHIP_NO_ERROR)
    {
        // The fabric would otherwise scan unbounded. Nothing here can reject the command
        // that got us here (it has already been applied), so drop the fabric instead and
        // let the caller settle the radio and the cached results.
        ChipLogError(AppServer, "BgScan: could not re-arm lifetime for fabricIndex=%u; dropping it", fabricIndex);
        return ReleaseFabric(state);
    }
    state.lifetime.armed = true;
    return {};
}

Status CommissioningProxyBgScanRegistry::Start(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                               BitMask<WiFiBandBitmap> wiFiBands, System::Clock::Seconds16 timeout)
{
    const bool wasEmpty = !AnyFabricInUse();

    // Reject before any side effect: a fabric may only hold so many concurrent requests,
    // and a node already holding one is refreshing rather than adding.
    FabricState * fabric = FindFabric(fabricIndex);
    if (fabric != nullptr && fabric->Find(nodeId) == nullptr && fabric->FindFree() == nullptr)
    {
        ChipLogError(AppServer, "BgScan: fabricIndex=%u already has %u background scans", fabricIndex,
                     static_cast<unsigned>(fabric->RequestCount()));
        return Status::ResourceExhausted;
    }
    if (fabric == nullptr)
    {
        for (auto & candidate : mFabrics)
        {
            if (!candidate.inUse)
            {
                fabric = &candidate;
                break;
            }
        }
        VerifyOrReturnError(fabric != nullptr, Status::ResourceExhausted);
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
    incoming.hasTimeout = (timeout.count() > 0);
    incoming.expiresAt  = now + timeout;

    bool needTimer                    = incoming.hasTimeout;
    System::Clock::Timestamp deadline = incoming.expiresAt;
    if (fabric->inUse)
    {
        for (const auto & slot : fabric->requests)
        {
            if (!slot.inUse || slot.nodeId == nodeId)
            {
                continue; // this node's request is about to be replaced
            }
            if (!slot.request.hasTimeout)
            {
                needTimer = false;
                break;
            }
            deadline = std::max(deadline, slot.request.expiresAt);
        }
    }

    // Arming can fail, so it happens before the table is touched. The old timer is
    // cancelled only once the new one is running.
    const bool hadTimer = fabric->inUse && fabric->lifetime.armed;
    if (needTimer)
    {
        // Cancel first: a context can hold only one timer, and this reuses the slot's.
        if (hadTimer)
        {
            mTimerDelegate.CancelTimer(&fabric->lifetime);
        }
        fabric->lifetime.registry    = this;
        fabric->lifetime.fabricIndex = fabricIndex;
        CHIP_ERROR timerErr          = mTimerDelegate.StartTimer(&fabric->lifetime, RemainingUntil(deadline, now));
        if (timerErr != CHIP_NO_ERROR)
        {
            // Without a lifetime timer the hardware scan would run unbounded, so reject.
            // Any requests the fabric already held go with it: the old timer was cancelled
            // just above and cannot be put back, so keeping them would scan unbounded too.
            // If this call is what started the radio and nothing is left to keep it
            // running, undo that as well.
            ChipLogError(AppServer, "BgScan: lifetime StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            fabric->lifetime.armed                      = false;
            const BitMask<WiFiBandBitmap> releasedBands = ReleaseFabric(*fabric);
            if (!AnyFabricInUse())
            {
                OnBecameEmpty();
            }
            else
            {
                ClearBandsNoLongerScanned(releasedBands);
            }
            return Status::Failure;
        }
        fabric->lifetime.armed = true;
    }
    else if (hadTimer)
    {
        CancelLifetime(*fabric);
    }

    // From here nothing can fail: the hardware scan is running and the fabric's new
    // timer is armed, so the table is safe to modify.
    //
    // Each node on a fabric keeps its own request and the fabric scans the union of
    // them, so this adds a request or replaces only this node's previous one.
    fabric->inUse       = true;
    fabric->fabricIndex = fabricIndex;
    RequestSlot * slot  = fabric->Find(nodeId);
    if (slot == nullptr)
    {
        slot = fabric->FindFree();
        VerifyOrDie(slot != nullptr); // capacity was checked above
    }
    slot->inUse   = true;
    slot->nodeId  = nodeId;
    slot->request = incoming;

    return Status::Success;
}

Status CommissioningProxyBgScanRegistry::Stop(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                              BitMask<WiFiBandBitmap> wiFiBands)
{
    FabricState * fabric = FindFabric(fabricIndex);
    VerifyOrReturnValue(fabric != nullptr, Status::NotFound);

    // Spec: if the client's NodeID and FabricID do not match those recorded when the
    // scan was started, take no action and reject with NOT_FOUND.
    RequestSlot * slot = fabric->Find(nodeId);
    VerifyOrReturnValue(slot != nullptr, Status::NotFound);

    const uint8_t reqTransportBits = transport.Raw();
    const uint16_t reqBandBits     = wiFiBands.Raw();
    const uint8_t ownTransportBits = slot->request.transport.Raw();
    const uint16_t ownBandBits     = slot->request.wiFiBands.Raw();

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
    // clears the last band. A request with no bands at all is not constrained by them.
    if (remainTransport == 0 || (ownBandBits != 0 && remainBands == 0))
    {
        slot->inUse = false;
    }
    else
    {
        slot->request.transport = BitMask<CapabilitiesBitmap>(remainTransport);
        slot->request.wiFiBands = BitMask<WiFiBandBitmap>(remainBands);
    }

    // Dropping a request can shorten the fabric back to a surviving request's deadline,
    // so the timer is recomputed rather than left where the removed request put it. A
    // timer that cannot be re-armed releases the fabric, whose bands then go too.
    uint16_t releasedBands = stopBandBits;
    if (fabric->RequestCount() == 0)
    {
        ReleaseFabric(*fabric);
    }
    else
    {
        releasedBands = static_cast<uint16_t>(releasedBands | RecomputeFabricLifetime(fabricIndex, *fabric).Raw());
    }

    // Other requests — on this fabric or another — may still want the radio, so it is
    // only torn down once nothing is left. Short of that, results for bands nobody
    // scans any more are still dropped.
    if (!AnyFabricInUse())
    {
        OnBecameEmpty();
    }
    else
    {
        ClearBandsNoLongerScanned(BitMask<WiFiBandBitmap>(releasedBands));
    }

    return Status::Success;
}

void CommissioningProxyBgScanRegistry::RemoveFabric(FabricIndex fabricIndex)
{
    FabricState * fabric = FindFabric(fabricIndex);
    VerifyOrReturn(fabric != nullptr);

    const BitMask<WiFiBandBitmap> removedBands = ReleaseFabric(*fabric);

    ChipLogProgress(AppServer, "BgScan: dropped background scans for removed fabricIndex=%u", fabricIndex);

    if (!AnyFabricInUse())
    {
        OnBecameEmpty();
    }
    else
    {
        ClearBandsNoLongerScanned(removedBands);
    }
}

void CommissioningProxyBgScanRegistry::Pause()
{
    if (AnyFabricInUse() && !mPaused)
    {
        mHardware.StopHardwareScan();
        mPaused = true;
        ChipLogProgress(AppServer, "BgScan: paused (radio needed for connect/foreground scan)");
    }
}

void CommissioningProxyBgScanRegistry::ResumeIfNeeded()
{
    if (!mPaused || !AnyFabricInUse())
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
    const bool hadFabrics = AnyFabricInUse();
    for (auto & fabric : mFabrics)
    {
        ReleaseFabric(fabric);
    }
    if (hadFabrics && !mPaused)
    {
        mHardware.StopHardwareScan();
    }
    mPaused = false;
}

void CommissioningProxyBgScanRegistry::OnLifetimeExpiry(FabricIndex fabricIndex)
{
    FabricState * fabric = FindFabric(fabricIndex);
    VerifyOrReturn(fabric != nullptr);

    // The fabric's timer sits at the latest deadline of all its requests, so when it
    // fires every one of them has expired and the whole fabric goes. The timer has
    // already been consumed, so just mark it disarmed (do not cancel the one that fired).
    fabric->lifetime.armed = false;

    // Spec: when the fabric's Timeout elapses its cached results are cleared — but only
    // for bands no surviving fabric still scans.
    const BitMask<WiFiBandBitmap> expiringBands = ReleaseFabric(*fabric);

    ChipLogProgress(AppServer, "BgScan: lifetime expired for fabricIndex=%u", fabricIndex);

    if (!AnyFabricInUse())
    {
        OnBecameEmpty();
    }
    else
    {
        ClearBandsNoLongerScanned(expiringBands);
    }
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
