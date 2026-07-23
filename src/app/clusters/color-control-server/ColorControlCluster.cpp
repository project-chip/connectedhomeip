/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ColorControlCluster.h"
#include "ColorControlSceneInvalidator.h" // injected scene-invalidation hook
#include <algorithm>
#include <app/clusters/on-off-server/OnOffCluster.h> // injected On/Off cluster (GetOnOff) for ShouldExecuteIfOff
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ColorControl/Metadata.h>
#include <cstdlib>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel; // bare ActionReturnStatus / AttributeChangeType
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;
using namespace chip::app::Clusters::ColorControl::Attributes; // bare attribute names (CurrentHue::Id, …)
using chip::Protocols::InteractionModel::Status;
using chip::System::SystemClock; // bare SystemClock() used throughout for the wall-clock anchor

namespace {
static constexpr uint16_t MIN_CIE_XY_VALUE = 0;
static constexpr uint16_t MAX_CIE_XY_VALUE = 0xfeff; // this value comes directly from the ZCL specification table 5.3

static constexpr uint8_t MIN_SATURATION_VALUE = 0;
static constexpr uint8_t MAX_SATURATION_VALUE = 254;

static constexpr uint16_t MAX_INT16U_VALUE = 0xFFFF; // RemainingTime sentinel for indefinite moves

// HueTransition::durationMs sentinel for a MoveHue rate move (no endpoint; runs until StopHueAxis).
// Safely above any real point-to-point duration (kMaxTransitionTime deciseconds * 100).
static constexpr uint32_t kIndefiniteHueMoveMs = UINT32_MAX;

static constexpr uint8_t MIN_CURRENT_LEVEL = 0x01;
static constexpr uint8_t MAX_CURRENT_LEVEL = 0xFE;

} // namespace

ColorControlCluster::ColorControlCluster(EndpointId endpoint, const Config & config) :
    DefaultServerCluster({ endpoint, ColorControl::Id }), mDelegate(&config.mDelegate), mFeatures(config.mFeatures),
    mColorValue(config.mColorValue), mColorLoop(config.mColorLoop), mCT(config.ctConfig), mStaticConfig(config.sc),
    mOnOff(config.onOff), mSceneInvalidator(config.sceneInvalidator)
{
    // ColorCapabilities mirrors the FeatureMap 1:1 (the two bitmaps share bit positions 0x1..0x10).
    mState.colorCapabilities.SetRaw(static_cast<uint16_t>(mFeatures.Raw()));

    // NumberOfPrimaries: count the app-provided fixed primaries; stays null when no StaticConfig is given.
    if (mStaticConfig != nullptr)
    {
        uint8_t count = 0;
        for (const auto & primary : mStaticConfig->primaries)
        {
            if (primary.has_value())
            {
                count++;
            }
        }
        mState.numberOfPrimaries.SetNonNull(count);
    }
}

ColorControlCluster::~ColorControlCluster()
{
    // Guaranteed cleanup: cancel the tick timer so a callback registered against DeviceLayer::SystemLayer()
    // with `this` never fires after the object is gone. Shutdown() also cancels it, but is not always called
    // (e.g. stack-allocated clusters in unit tests), and CancelTimer is a no-op when nothing is armed.
    DeviceLayer::SystemLayer().CancelTimer(&TimerCallback, this);
}

CHIP_ERROR ColorControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // Restore the persisted attributes (per the spec persist flags). Each load falls back to the value
    // already in the member — the constructor default — when nothing is stored yet.
    AttributePersistence persistence(context.attributeStorage);
    const EndpointId ep = mPath.mEndpointId;
    auto path           = [ep](AttributeId id) { return ConcreteAttributePath(ep, ColorControl::Id, id); };

    // The active color is persisted per-axis, but only the active mode's axes are meaningful. The
    // persisted enhancedColorMode is the single source of truth for WHICH alternative to rebuild
    // (colorMode is derivable, so we ignore its persisted copy). Rebuild mColorValue from it.
    EnhancedColorModeEnum mode = EnhancedColorModeEnum::kCurrentXAndCurrentY; // ZAP default (0x01)
    persistence.LoadNativeEndianValue(path(Attributes::EnhancedColorMode::Id), mode, mode);
    switch (mode)
    {
    case EnhancedColorModeEnum::kCurrentXAndCurrentY: {
        XYColor c{};
        persistence.LoadNativeEndianValue(path(CurrentX::Id), c.x, c.x);
        persistence.LoadNativeEndianValue(path(CurrentY::Id), c.y, c.y);
        mColorValue = c;
        break;
    }
    case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation: {
        HueSatColor c{};
        persistence.LoadNativeEndianValue(path(CurrentHue::Id), c.hue, c.hue);
        persistence.LoadNativeEndianValue(path(CurrentSaturation::Id), c.saturation, c.saturation);
        mColorValue = c;
        break;
    }
    case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation: {
        EnhancedHueSatColor c{};
        persistence.LoadNativeEndianValue(path(EnhancedCurrentHue::Id), c.enhancedHue, c.enhancedHue);
        persistence.LoadNativeEndianValue(path(CurrentSaturation::Id), c.saturation, c.saturation);
        mColorValue = c;
        break;
    }
    case EnhancedColorModeEnum::kColorTemperatureMireds: {
        CTColor c{};
        persistence.LoadNativeEndianValue(path(ColorTemperatureMireds::Id), c.mireds, c.mireds);
        mColorValue = c;
        break;
    }
    default:
        break;
    }

    if (HasFeature(Feature::kColorLoop))
    {
        persistence.LoadNativeEndianValue(path(ColorLoopActive::Id), mColorLoop.active, mColorLoop.active);
        persistence.LoadNativeEndianValue(path(Attributes::ColorLoopDirection::Id), mColorLoop.direction, mColorLoop.direction);
        persistence.LoadNativeEndianValue(path(ColorLoopTime::Id), mColorLoop.time, mColorLoop.time);
    }

    if (HasFeature(Feature::kColorTemperature))
    {
        persistence.LoadNativeEndianValue(path(StartUpColorTemperatureMireds::Id), mCT.startUpColorTemperatureMireds,
                                          mCT.startUpColorTemperatureMireds);
        // §3.2.11.10: force ColorTemperatureMireds to the configured startup value (→ CT mode), or when
        // null keep the color we just restored above.
        ApplyStartUpColorTemperature();
    }

    // Resume a color loop that was running at shutdown (§3.2.8.1). Loading ColorLoopActive alone leaves the
    // loop dormant — nothing drives EnhancedCurrentHue until the next command. Re-arm it here, but only if it
    // is actually DRIVING: the monotonic clock resets across reboot, so we cannot recover the original phase;
    // instead re-anchor from the just-restored EnhancedCurrentHue and "now" (drift-free from this point on).
    // This runs AFTER the startup-CT block on purpose: if StartUpColorTemperatureMireds forced CT mode, the
    // loop is active-but-dormant (LoopIsDriving() is false), so we correctly do NOT arm the tick.
    if (HasFeature(Feature::kColorLoop) && LoopIsDriving())
    {
        auto & ehs                   = std::get<EnhancedHueSatColor>(mColorValue);
        mColorLoop.storedEnhancedHue = ehs.enhancedHue; // best-effort restore target (RAM-only, lost on reboot)
        mColorLoopStartHue           = ehs.enhancedHue;
        mColorLoopStartTimeMs        = SystemClock().GetMonotonicMilliseconds64().count();
        ArmTick();
    }

    return CHIP_NO_ERROR;
}

// ── Persistence ──────────────────────────────────────────────────────────────────────────────────
// Mirror of the NVM-flagged attributes restored in Startup(): Startup reads them, these write them back.
// Writes are settle-triggered (transition end / discrete change), never per-tick — RemainingTime and the
// in-flight transition are RAM-only, so persisting each intermediate tick would only wear flash.
template <typename T>
void ColorControlCluster::PersistValue(AttributeId id, const T & value)
{
    VerifyOrReturn(mContext != nullptr);
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, ColorControl::Id, id),
                                                            ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
}

// Persist EnhancedColorMode (the single source of truth for WHICH alternative to rebuild — colorMode is
// derived) plus the active mode's stored axes. This is exactly the set Startup() reads to reconstruct
// mColorValue, at matching native-endian widths.
void ColorControlCluster::PersistCurrentColor()
{
    VerifyOrReturn(mContext != nullptr);
    PersistValue(Attributes::EnhancedColorMode::Id, GetEnhancedColorMode());
    if (auto * c = std::get_if<XYColor>(&mColorValue))
    {
        PersistValue(CurrentX::Id, c->x);
        PersistValue(CurrentY::Id, c->y);
    }
    else if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
    {
        PersistValue(CurrentHue::Id, hs->hue);
        PersistValue(CurrentSaturation::Id, hs->saturation);
    }
    else if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        PersistValue(EnhancedCurrentHue::Id, e->enhancedHue);
        PersistValue(CurrentSaturation::Id, e->saturation);
    }
    else if (auto * ct = std::get_if<CTColor>(&mColorValue))
    {
        PersistValue(ColorTemperatureMireds::Id, ct->mireds);
    }
}

void ColorControlCluster::PersistColorLoop()
{
    VerifyOrReturn(mContext != nullptr);
    PersistValue(ColorLoopActive::Id, mColorLoop.active);
    PersistValue(Attributes::ColorLoopDirection::Id, mColorLoop.direction);
    PersistValue(ColorLoopTime::Id, mColorLoop.time);
}

void ColorControlCluster::Shutdown(ClusterShutdownType type)
{
    // NOTE: intentionally do NOT unregister the scene handler here. It is a single global handler shared
    // by every ColorControl endpoint (registered once, dedup-guarded, in Startup). Unregistering on one
    // endpoint's teardown would remove it for all others; a torn-down endpoint instead stops being served
    // automatically because the handler's SupportsCluster() registry lookup then returns null.

    // Stop the 100ms tick so no callback fires after teardown.
    DeviceLayer::SystemLayer().CancelTimer(&TimerCallback, this);
    DefaultServerCluster::Shutdown(type);
}

// Arm the one-shot 100ms tick. Guard against double-arming so a command issued mid-transition
// doesn't stack a second timer; OnTick re-arms itself while any axis (or the loop) is still moving.
void ColorControlCluster::ArmTick()
{
    if (DeviceLayer::SystemLayer().IsTimerActive(&TimerCallback, this))
    {
        return;
    }
    LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kTickMs), &TimerCallback, this));
}
// Remaining time on one axis, in 1/10 s, from its wall-clock anchor. durationMs == kIndefiniteHueMoveMs
// is the MoveHue rate move, which has no end, so RemainingTime is MAX_INT16U until a Stop clears the
// axis (§3.2.7.4). durationMs == 0 is an immediate (transitionTime 0) move → 0 remaining.
inline uint16_t RemainingTenths(uint64_t startTimeMs, uint32_t durationMs, uint64_t now)
{
    if (durationMs == kIndefiniteHueMoveMs)
        return MAX_INT16U_VALUE;
    const uint64_t endMs = startTimeMs + durationMs;
    if (now >= endMs)
        return 0;
    return static_cast<uint16_t>((endMs - now) / 100); // or (… + 99)/100 to round up
}
bool ColorControlCluster::LoopIsDriving() const
{
    return mColorLoop.active == 1 && std::holds_alternative<EnhancedHueSatColor>(mColorValue);
}

// RemainingTime store + edge-triggered report (§3.2.7.4): report only when it reaches 0, rises from 0
// past the 10-tenths threshold, or a fresh command changes it (isNewTransition); intermediate ticks
// update the value quietly. `report` is computed against the OLD value before SetAttributeValue writes.
void ColorControlCluster::SetQuietReportRemainingTime(uint16_t newRemainingTime, bool isNewTransition)
{
    const bool report = (newRemainingTime == 0) || (mState.remainingTime == 0 && newRemainingTime > 10) || isNewTransition;
    SetAttributeValue(mState.remainingTime, newRemainingTime, RemainingTime::Id,
                      report ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet);
}

void ColorControlCluster::OnTick()
{
    const uint64_t now = SystemClock().GetMonotonicMilliseconds64().count();
    bool driverActive  = false;
    uint16_t remaining = 0; // 1/10 s, slowest still-active axis; 0 once everything has settled

    // Remember whether a driver was running: if it settles this tick we persist the final color once.
    const bool hadTransition = !std::holds_alternative<std::monostate>(mTransition);

    if (auto * xytx = std::get_if<XYTransition>(&mTransition))
    {
        driverActive = TickXY(*xytx, now);
        if (driverActive) // X and Y share one start; RemainingTime is the slower axis
        {
            remaining = std::max(RemainingTenths(xytx->startTimeMs, xytx->durationXMs, now),
                                 RemainingTenths(xytx->startTimeMs, xytx->durationYMs, now));
        }
    }
    else if (auto * cttx = std::get_if<CTTransition>(&mTransition))
    {
        driverActive = TickCT(*cttx, now);
        if (driverActive)
        {
            remaining = RemainingTenths(cttx->startTimeMs, cttx->durationMs, now);
        }
    }
    else if (auto * hsx = std::get_if<HueSatTransition>(&mTransition))
    {
        // Hue and saturation are independent axes (§3.2.5.2): clearing one leaves
        // the other running. Each Tick* returns false when its axis has finished.
        // Loop-vs-hue exclusivity is enforced at the commands (they never start a hue
        // axis while the loop drives), so this branch stays loop-agnostic.
        if (hsx->hue && !TickHue(*hsx->hue, now))
        {
            hsx->hue.reset();
        }
        if (hsx->sat && !TickSat(*hsx->sat, now))
        {
            hsx->sat.reset();
        }
        // RemainingTime follows the slower axis: a finished axis contributed nothing.
        if (hsx->hue)
        {
            remaining = std::max(remaining, RemainingTenths(hsx->hue->startTimeMs, hsx->hue->durationMs, now));
        }
        if (hsx->sat)
        {
            remaining = std::max(remaining, RemainingTenths(hsx->sat->startTimeMs, hsx->sat->durationMs, now));
        }
        driverActive = hsx->hue.has_value() || hsx->sat.has_value();
    }
    // else: monostate — nothing is driving this tick

    // Clear a finished driver AFTER the dispatch above (the get_if pointers alias mTransition, so we
    // must not reassign the variant while they are still in use).
    if (!driverActive)
    {
        mTransition = std::monostate{};
        // A transition that just settled wrote the exact target into mColorValue; persist that final
        // value so a reboot restores where the light actually ended up. Guarded on hadTransition so idle
        // ticks (mTransition already monostate) don't write, and it never fires mid-transition.
        if (hadTransition)
        {
            PersistCurrentColor();
        }
    }

    // The color loop is a mode-independent hue driver kept out of mTransition; it may keep ticking
    // even when mTransition is monostate. It never ends, so it pins RemainingTime at MAX until Deactivate.
    const bool loopActive = TickColorLoop(now);
    if (loopActive)
    {
        remaining = MAX_INT16U_VALUE;
    }

    // ONE quiet-report per tick for RemainingTime (§3.2.7.4): the QuieterReporting predicate reports
    // only when the value hits 0 (transition just finished, since driverActive/loopActive went false
    // → remaining stayed 0), crosses 0→(>10), or on a command-caused delta (isNewTransition, set by the
    // command handlers — false here). Mid-transition it is updated quietly, never spamming reports.
    SetQuietReportRemainingTime(remaining, /*isNewTransition=*/false);

    // Re-arm only while something is still moving → zero CPU at steady state.
    if (driverActive || loopActive)
    {
        ArmTick();
    }
}

// ── each tick · store the value + report it. Pure math, one clock read in OnTick. ─
// TickHue: position from wall-clock elapsed, exact endpoint on the last tick, then store + fan-out.
bool ColorControlCluster::TickHue(HueTransition & tx, uint64_t now)
{
    const uint64_t elapsed = now - tx.startTimeMs;
    uint16_t eh            = 0;
    bool done              = false;

    if (tx.durationMs == kIndefiniteHueMoveMs)
    {
        // Indefinite rate move (MoveHue): signedDelta is hue-units per second, wrapping forever
        // until StopHueAxis. Position = start + rate * elapsed, taken mod 65536 (circular).
        const int64_t moved = static_cast<int64_t>(tx.signedDelta) * static_cast<int64_t>(elapsed) / 1000;
        eh                  = static_cast<uint16_t>((static_cast<int64_t>(tx.startHue) + moved) & 0xFFFF);
        done                = false; // only StopHueAxis / ColorLoop-Deactivate ends it
    }
    else
    {
        // Point-to-point: signedDelta is the signed arc to travel over durationMs. durationMs == 0
        // means transitionTime == 0 → jump to the endpoint on the first tick (avoids div-by-zero).
        done              = (tx.durationMs == 0) || (elapsed >= tx.durationMs);
        const float t     = done ? 1.f : std::clamp(static_cast<float>(elapsed) / static_cast<float>(tx.durationMs), 0.f, 1.f);
        const int32_t arc = done ? tx.signedDelta                                                // exact endpoint, no drift
                                 : static_cast<int32_t>(static_cast<float>(tx.signedDelta) * t); // 16-bit circular interpolation
        eh                = static_cast<uint16_t>((static_cast<int32_t>(tx.startHue) + arc) & 0xFFFF);
    }

    // NETWORK REPORTING is throttled (§3.2.7.2): silent mid-transition, report at the end.
    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // The transition runs in 16-bit; store into whichever HS alternative is active, at its native
    // precision. Exactly one of CurrentHue / EnhancedCurrentHue is a stored field; the other is a
    // projection signalled notify-only (mode-dependent).
    if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        SetAttributeValue(ehs->enhancedHue, eh, EnhancedCurrentHue::Id, change); // store 16-bit + report
        NotifyAttributeChanged(CurrentHue::Id, change);                          // projection: hue() = eh >> 8
        mDelegate->OnColorHSChanged(mPath.mEndpointId, ehs->hue(), ehs->saturation);
    }
    else
    {
        auto & hs = std::get<HueSatColor>(mColorValue);
        SetAttributeValue(hs.hue, static_cast<uint8_t>(eh >> 8), CurrentHue::Id, change); // store 8-bit + report
        NotifyAttributeChanged(EnhancedCurrentHue::Id, change);                           // projection: enhancedHue() = hue << 8
        mDelegate->OnColorHSChanged(mPath.mEndpointId, hs.hue, hs.saturation);
    }

    // HARDWARE OUTPUT is continuous, NOT throttled (§3.2.8): the delegate owns no timer, so
    // OnColorHSChanged fires EVERY tick (exact target on the last one) regardless of the
    // kQuiet/kReportable flag, which governs only network reporting.
    return !done; // "still active"
}

// TickSat: linear position from wall-clock elapsed, exact endpoint on the last tick, then store + fan-out.
bool ColorControlCluster::TickSat(SatTransition & tx, uint64_t now)
{
    const uint64_t elapsed = now - tx.startTimeMs;
    const float t          = std::clamp(static_cast<float>(elapsed) / static_cast<float>(tx.durationMs), 0.f, 1.f);
    const bool done        = (t >= 1.f);

    // Saturation is linear (not circular) and 8-bit. Exact target on the last tick avoids drift.
    const uint8_t sat = done
        ? tx.targetSat
        : static_cast<uint8_t>(tx.startSat + static_cast<int32_t>(static_cast<float>(int32_t(tx.targetSat) - tx.startSat) * t));

    // NETWORK REPORTING is throttled (§3.2.7.2): silent mid-transition, report at the end.
    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // CurrentSaturation is a single stored value shared by legacy and enhanced HS — no projection,
    // so SetAttributeValue (store + notify) is all it needs. Write into whichever HS alt is active.
    if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        SetAttributeValue(ehs->saturation, sat, CurrentSaturation::Id, change);
        mDelegate->OnColorHSChanged(mPath.mEndpointId, ehs->hue(), ehs->saturation);
    }
    else
    {
        auto & hs = std::get<HueSatColor>(mColorValue);
        SetAttributeValue(hs.saturation, sat, CurrentSaturation::Id, change);
        mDelegate->OnColorHSChanged(mPath.mEndpointId, hs.hue, hs.saturation);
    }

    // HARDWARE OUTPUT is continuous, NOT throttled (§3.2.8): the delegate owns no timer, so
    // OnColorHSChanged fires EVERY tick (exact target on the last one) regardless of the
    // kQuiet/kReportable flag, which governs only network reporting.
    return !done; // "still active"
}

// TickCT: linear position from wall-clock elapsed, exact endpoint on the last tick, then store + fan-out.
bool ColorControlCluster::TickCT(CTTransition & tx, uint64_t now)
{
    const uint64_t elapsed = now - tx.startTimeMs;
    const float t          = std::clamp(static_cast<float>(elapsed) / static_cast<float>(tx.durationMs), 0.f, 1.f);
    const bool done        = (t >= 1.f);

    // Color temperature is linear, 16-bit mireds. Exact target on the last tick avoids drift.
    const uint16_t mireds = done
        ? tx.targetMireds
        : static_cast<uint16_t>(tx.startMireds +
                                static_cast<int32_t>(static_cast<float>(int32_t(tx.targetMireds) - tx.startMireds) * t));

    // NETWORK REPORTING is throttled (§3.2.7.2): silent mid-transition, report at the end.
    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // CurrentSaturation is a single stored value shared by legacy and enhanced HS — no projection,
    // so SetAttributeValue (store + notify) is all it needs. Write into whichever HS alt is active.
    // In CT mode mColorValue is always CTColor (variant/mode kept in lockstep), so get<> is safe.
    auto & ct = std::get<CTColor>(mColorValue);
    SetAttributeValue(ct.mireds, mireds, ColorTemperatureMireds::Id, change);
    mDelegate->OnColorCTChanged(mPath.mEndpointId, ct.mireds);

    // HARDWARE OUTPUT is continuous, NOT throttled (§3.2.8): the delegate owns no timer, so
    // OnColorHSChanged fires EVERY tick (exact target on the last one) regardless of the
    // kQuiet/kReportable flag, which governs only network reporting.
    return !done; // "still active"
}

// TickColorLoop: the autonomous enhanced-hue driver. Unlike the axes held in mTransition, the loop
// is mode-independent and NEVER finishes — it cycles EnhancedCurrentHue around the full [0, 65536)
// circle at a constant rate until ColorLoopSet(Deactivate) (§3.2.8.1). Nothing else stops it.
//
// Position is derived from the wall-clock anchor (mColorLoopStartHue / mColorLoopStartTimeMs) stamped
// when the loop started, so it is drift-free. Crucially the anchor is NOT re-stamped when the loop
// goes dormant: its phase keeps advancing with real time even while XY/CT owns the output, so when the
// mode returns to enhanced HS the hue is already where wall-clock says it should be — no jump.
bool ColorControlCluster::TickColorLoop(uint64_t now)
{
    // Dormancy guard (§3.2.8.1): the loop can be active while XY/CT owns the output. LoopIsDriving()
    // is true only when ColorLoopActive == 1 AND the active value is EnhancedHueSatColor. When it is
    // dormant we write nothing and return false — a dormant loop has no reason to keep the tick alive
    // (whatever command switches the mode back to enhanced HS will re-arm the timer itself).
    if (!LoopIsDriving())
    {
        return false;
    }

    // Rate is fixed by ColorLoopTime = seconds for one full 65536-unit revolution. Guard time == 0.
    const uint32_t loopTimeSec = (mColorLoop.time == 0) ? 1u : mColorLoop.time;

    // Position from the wall-clock anchor. INTEGER math on purpose: the loop never ends, so the
    // float error every other Tick* tolerates would accumulate without bound here. 65536 * elapsed
    // fits comfortably in int64 for any realistic uptime.
    const uint64_t elapsedMs = now - mColorLoopStartTimeMs;
    const int64_t traveled =
        static_cast<int64_t>(0x10000) * static_cast<int64_t>(elapsedMs) / (static_cast<int64_t>(loopTimeSec) * 1000);
    // ColorLoopDirection: 1 == increment (up), 0 == decrement (down).
    const int64_t signedTraveled = mColorLoop.direction ? traveled : -traveled;

    const uint16_t eh =
        static_cast<uint16_t>((static_cast<int64_t>(mColorLoopStartHue) + signedTraveled) & 0xFFFF); // wrap mod 65536

    // The loop only ever runs in enhanced HS (guaranteed by LoopIsDriving), so get<> is safe and the
    // legacy-mode branch of TickHue is unreachable here. EnhancedCurrentHue is the stored 16-bit
    // field; CurrentHue is its projection (hue() = eh >> 8), signalled notify-only.
    //
    // The loop has no "done" tick, so every tick is kQuiet (§3.2.7.2) — reporting is throttled to at
    // most once per second by the quieter-reporting layer, exactly as TickHue does mid-transition.
    // HARDWARE OUTPUT is continuous and fires on EVERY tick regardless of the report flag (§3.2.8).
    auto & ehs = std::get<EnhancedHueSatColor>(mColorValue);
    SetAttributeValue(ehs.enhancedHue, eh, EnhancedCurrentHue::Id, AttributeChangeType::kQuiet); // store 16-bit
    NotifyAttributeChanged(CurrentHue::Id, AttributeChangeType::kQuiet);                         // projection
    mDelegate->OnColorHSChanged(mPath.mEndpointId, ehs.hue(), ehs.saturation);

    return true; // keep the tick alive until ColorLoopSet(Deactivate)
}

// TickXY: linear per-axis position from wall-clock elapsed; X and Y have independent durations.
bool ColorControlCluster::TickXY(XYTransition & tx, uint64_t now)
{
    const uint64_t elapsed = now - tx.startTimeMs; // one start time; axes differ only by duration

    const float tX   = std::clamp(static_cast<float>(elapsed) / static_cast<float>(tx.durationXMs), 0.f, 1.f);
    const float tY   = std::clamp(static_cast<float>(elapsed) / static_cast<float>(tx.durationYMs), 0.f, 1.f);
    const bool doneX = (tX >= 1.f);
    const bool doneY = (tY >= 1.f);
    const bool done  = doneX && doneY; // XY is done only when BOTH axes arrive

    // CIE X/Y are 16-bit and linear. Exact target on each axis's last tick avoids drift.
    const uint16_t x = done
        ? tx.targetX
        : static_cast<uint16_t>(tx.startX + static_cast<int32_t>(static_cast<float>(int32_t(tx.targetX) - tx.startX) * tX));
    const uint16_t y = done
        ? tx.targetY
        : static_cast<uint16_t>(tx.startY + static_cast<int32_t>(static_cast<float>(int32_t(tx.targetY) - tx.startY) * tY));

    // NETWORK REPORTING is throttled (3.2.7.2): silent mid-transition, report only when BOTH done.
    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // In XY mode mColorValue is always XYColor (variant/mode kept in lockstep), so get<> is safe.
    auto & xy = std::get<XYColor>(mColorValue);
    SetAttributeValue(xy.x, x, CurrentX::Id, change);
    SetAttributeValue(xy.y, y, CurrentY::Id, change);
    mDelegate->OnColorXYChanged(mPath.mEndpointId, xy.x, xy.y);

    // HARDWARE OUTPUT is continuous, NOT throttled (3.2.8): the delegate owns no timer, so
    // OnColorXYChanged fires EVERY tick (exact target on the last one) regardless of the
    // kQuiet/kReportable flag, which governs only network reporting.
    return !done; // "still active"
}

std::optional<ActionReturnStatus> ColorControlCluster::HandleStopMoveStep(const Commands::StopMoveStep::DecodableType & req,
                                                                          CommandHandler * handler)
{
    return stopMoveStep(req.optionsMask, req.optionsOverride);
}

ColorControlCluster::Status ColorControlCluster::stopMoveStep(OptMask optionsMask, OptMask optionsOverride)
{
    // StopMoveStep has no effect while a color loop is active — leave it running.
    if (HasFeature(Feature::kColorLoop) && mColorLoop.active)
    {
        return Status::Success;
    }

    // OnOff / Options gating.
    if (!ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    // Stop the mTransition-driven axes. Deliberately does NOT cancel the tick: an active color loop
    // is independent and must keep running, so OnTick self-terminates the timer only if nothing else
    // is driving.
    mTransition = std::monostate{};
    SetQuietReportRemainingTime(0, /*isNewTransition=*/false);
    PersistCurrentColor(); // freeze: persist the value the transition stopped at (it won't reach OnTick's settle)
    return Status::Success;
}

bool ColorControlCluster::ShouldExecuteIfOff(BitMask<OptionsBitmap> mask, BitMask<OptionsBitmap> optionsOverride)
{
    // §3.2.8.3 — effective = (options where mask bit = 0) | (override where mask bit = 1)
    BitMask<OptionsBitmap> effective{ static_cast<uint8_t>((mState.options.Raw() & ~mask.Raw()) |
                                                           (optionsOverride.Raw() & mask.Raw())) };

    // Ember-free On/Off coupling via the injected cluster: no injected cluster == no coupling (→ always execute).
    std::optional<bool> isOn = (mOnOff != nullptr) ? std::optional<bool>(mOnOff->GetOnOff()) : std::nullopt;
    if (!isOn.has_value())
        return true; // endpoint has no OnOff cluster → always execute
    if (*isOn)
        return true; // device is on → execute
    return effective.Has(OptionsBitmap::kExecuteIfOff);
}

/**
 * @brief The specification says that if we are transitioning from one color mode
 * into another, we need to compute the new mode's attribute values from the
 * old mode.  However, it also says that if the old mode doesn't translate into
 * the new mode, this must be avoided.
 * I am putting in this function to compute the new attributes based on the old
 * color mode.
 *
 * @param endpoint
 * @param newColorMode
 */
void ColorControlCluster::ApplyModeSwitch(EnhancedColorModeEnum target)
{
    switch (target)
    {
    case EnhancedColorModeEnum::kCurrentXAndCurrentY: {
        if (std::holds_alternative<XYColor>(mColorValue))
            return; // no-op
        XYColor next{};
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToXY(mPath.mEndpointId, hs->hue, hs->saturation, next.x, next.y);
        }
        else if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToXY(mPath.mEndpointId, ehs->hue(), ehs->saturation, next.x, next.y);
        }
        else if (auto * ct = std::get_if<CTColor>(&mColorValue))
        {
            mDelegate->ConvertMiredsToXY(mPath.mEndpointId, ct->mireds, next.x, next.y);
        }
        mColorValue = next; // the variant assignment IS the store
        NotifyAttributeChanged(CurrentX::Id, AttributeChangeType::kQuiet);
        NotifyAttributeChanged(CurrentY::Id, AttributeChangeType::kQuiet);
        break;
    }
    case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
    case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation: {
        const bool toEnhanced = (target == EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation);
        // no-op only if already in the EXACT target alternative
        if (toEnhanced && std::holds_alternative<EnhancedHueSatColor>(mColorValue))
            return;
        if (!toEnhanced && std::holds_alternative<HueSatColor>(mColorValue))
            return;

        uint16_t eh = 0;
        uint8_t sat = 0;
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            eh  = hs->enhancedHue();
            sat = hs->saturation;
        }
        else if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            eh  = ehs->enhancedHue;
            sat = ehs->saturation;
        }
        else if (auto * xy = std::get_if<XYColor>(&mColorValue))
        {
            uint8_t h = 0;
            mDelegate->ConvertXYToHueSat(mPath.mEndpointId, xy->x, xy->y, h, sat);
            eh = static_cast<uint16_t>(uint16_t(h) << 8);
        }
        else if (auto * ct = std::get_if<CTColor>(&mColorValue))
        {
            uint8_t h = 0;
            mDelegate->ConvertMiredsToHueSat(mPath.mEndpointId, ct->mireds, h, sat);
            eh = static_cast<uint16_t>(uint16_t(h) << 8);
        }

        if (toEnhanced)
        {
            mColorValue = EnhancedHueSatColor{ .enhancedHue = eh, .saturation = sat };
        }
        else
        {
            mColorValue = HueSatColor{ .hue = static_cast<uint8_t>(eh >> 8), .saturation = sat };
        }
        NotifyAttributeChanged(EnhancedCurrentHue::Id, AttributeChangeType::kQuiet);
        NotifyAttributeChanged(CurrentHue::Id, AttributeChangeType::kQuiet);
        NotifyAttributeChanged(CurrentSaturation::Id, AttributeChangeType::kQuiet);
        break;
    }
    case EnhancedColorModeEnum::kColorTemperatureMireds: {
        if (std::holds_alternative<CTColor>(mColorValue))
            return;
        CTColor next{};
        if (auto * xy = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate->ConvertXYToMireds(mPath.mEndpointId, xy->x, xy->y, next.mireds);
        }
        else if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToMireds(mPath.mEndpointId, hs->hue, hs->saturation, next.mireds);
        }
        else if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToMireds(mPath.mEndpointId, ehs->hue(), ehs->saturation, next.mireds);
        }
        mColorValue = next; // the variant assignment IS the store
        NotifyAttributeChanged(ColorTemperatureMireds::Id, AttributeChangeType::kQuiet);
        break;
    }
    case EnhancedColorModeEnum::kUnknownEnumValue:
        break; // not a real mode; nothing to switch to
    }
    NotifyModeAttributes(); // colorMode + enhancedColorMode are derived → mark both kReportable
}

void ColorControlCluster::NotifyModeAttributes()
{
    NotifyAttributeChanged(Attributes::ColorMode::Id, AttributeChangeType::kReportable);
    NotifyAttributeChanged(Attributes::EnhancedColorMode::Id, AttributeChangeType::kReportable);
}

// Current-color accessors used by command start-values and apply-scene. Callers ApplyModeSwitch to an
// HS mode first, so mColorValue is a HueSat alternative here; the fallbacks keep them total.
uint8_t ColorControlCluster::GetSaturation() const
{
    if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        return e->saturation;
    if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        return hs->saturation;
    return 0;
}

uint16_t ColorControlCluster::GetEnhancedHue() const
{
    if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        return e->enhancedHue;
    if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        return hs->enhancedHue(); // 8-bit → high byte
    return 0;
}

// ---- Live-state accessors for the scene handler (no cross-mode conversion; see header). ----
uint16_t ColorControlCluster::CurrentX() const
{
    auto * c = std::get_if<XYColor>(&mColorValue);
    return c != nullptr ? c->x : 0;
}

uint16_t ColorControlCluster::CurrentY() const
{
    auto * c = std::get_if<XYColor>(&mColorValue);
    return c != nullptr ? c->y : 0;
}

uint16_t ColorControlCluster::ColorTempMireds() const
{
    auto * ct = std::get_if<CTColor>(&mColorValue);
    return ct != nullptr ? ct->mireds : 0;
}

// Derived from which alternative is active (colorMode / enhancedColorMode are never stored).
EnhancedColorModeEnum ColorControlCluster::GetEnhancedColorMode() const
{
    if (std::holds_alternative<XYColor>(mColorValue))
        return EnhancedColorModeEnum::kCurrentXAndCurrentY;
    if (std::holds_alternative<CTColor>(mColorValue))
        return EnhancedColorModeEnum::kColorTemperatureMireds;
    if (std::holds_alternative<EnhancedHueSatColor>(mColorValue))
        return EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation;
    return EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation; // HueSatColor
}

bool ColorControlCluster::SupportsMode(EnhancedColorModeEnum mode) const
{
    switch (mode)
    {
    case EnhancedColorModeEnum::kCurrentXAndCurrentY:
        return HasFeature(Feature::kXy);
    case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
        return HasFeature(Feature::kHueAndSaturation);
    case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation:
        return HasFeature(Feature::kEnhancedHue);
    case EnhancedColorModeEnum::kColorTemperatureMireds:
        return HasFeature(Feature::kColorTemperature);
    default:
        return false;
    }
}

// Mark stored scenes stale for every fabric because the current color changed (a scene no longer
// matches the live state). Single call site for all commands. Forwards to the injected scene
// invalidator when present (null == no Scene Management coupling); the core never touches Scenes/Ember.
void ColorControlCluster::InvalidateScenes()
{
    if (mSceneInvalidator != nullptr)
    {
        mSceneInvalidator->InvalidateScenes(mPath.mEndpointId);
    }
}

/**
 * @brief Configures and launches color loop for a specified endpoint
 *
 * @param startFromStartHue True, start from StartEnhancedHue attribute. False, start from currentEnhancedHue
 */
void ColorControlCluster::startColorLoop(bool startFromStartHue)
{
    // Loop runs in enhanced-hue mode; make sure the value variant matches.
    ApplyModeSwitch(EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation);
    auto & ehs = std::get<EnhancedHueSatColor>(mColorValue);

    // The loop owns the hue axis exclusively (§3.2.8.1) — drop any in-flight hue transition so it
    // can't fight the loop tick-for-tick. A running saturation axis is independent and survives.
    if (auto * hsx = std::get_if<HueSatTransition>(&mTransition))
    {
        hsx->hue.reset();
    }

    mColorLoop.active            = 1;
    mColorLoop.storedEnhancedHue = ehs.enhancedHue;                                      // remember where to return on stop
    ehs.enhancedHue = startFromStartHue ? mColorLoop.startEnhancedHue : ehs.enhancedHue; // per ColorLoopSet startHue action

    // Stamp the wall-clock anchor TickColorLoop interpolates from. Stamped once here and never again
    // (dormancy does not re-stamp), so the loop's phase tracks real time. Anchor hue = the hue we are
    // starting from (after the optional jump to startEnhancedHue above).
    mColorLoopStartHue    = ehs.enhancedHue;
    mColorLoopStartTimeMs = SystemClock().GetMonotonicMilliseconds64().count();

    // Scenable loop attributes just changed → kReportable (no quieterReporting quality).
    NotifyAttributeChanged(ColorLoopActive::Id, AttributeChangeType::kReportable);
    NotifyAttributeChanged(Attributes::ColorLoopDirection::Id, AttributeChangeType::kReportable);
    NotifyAttributeChanged(ColorLoopTime::Id, AttributeChangeType::kReportable);

    // Loop start switched to enhanced-HS mode and moved EnhancedCurrentHue → persist the settled color so
    // a reboot resumes the loop from the right hue (the loop attrs are persisted by the caller/ColorLoopSet).
    PersistCurrentColor();

    ArmTick(); // ensure the 100ms timer runs so TickColorLoop advances the hue
}

/**
 * @brief Executes move to saturation command
 *
 * @param saturation Target saturation
 * @param transitionTime Transition time in 10th of seconds
 */
Status ColorControlCluster::moveToSaturation(uint8_t saturation, uint16_t transitionTimeDs, BitMask<OptionsBitmap> optionsMask,
                                             BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(saturation <= MAX_SATURATION_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);
    const uint32_t durationMs = transitionTimeDs * 100u;  // deciseconds → ms
    auto & hs                 = EnsureHueSatTransition(); // HueSatTransition; preserves .hue if already one, replaces XY/CT
    hs.sat                    = SatTransition{
                           .startSat    = GetSaturation(), // read AFTER the mode switch
                           .targetSat   = std::clamp<uint8_t>(saturation, MIN_SATURATION_VALUE, MAX_SATURATION_VALUE),
                           .startTimeMs = SystemClock().GetMonotonicMilliseconds64().count(),
                           .durationMs  = durationMs,
    };
    SetQuietReportRemainingTime(static_cast<uint16_t>(durationMs / 100), /*isNewTransition=*/true); // finite
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

// Returns a REFERENCE into mTransition so callers' writes to .hue/.sat persist. If a HueSatTransition
// is already active it is preserved (hue+sat are the only legal concurrent pair, §3.2.5.2); otherwise
// an XY/CT driver is replaced by a fresh empty one.
HueSatTransition & ColorControlCluster::EnsureHueSatTransition()
{
    if (!std::holds_alternative<HueSatTransition>(mTransition))
    {
        mTransition = HueSatTransition{};
    }
    return std::get<HueSatTransition>(mTransition);
}
/**
 * @brief Executes move to hue and saturatioan command.
 *
 * @param[in] endpoint EndpointId of the recipient Color control cluster.
 * @param[in] hue Target hue.
 * @param[in] saturation Target saturation.
 * @param[in] transitionTime Transition time in 10th of seconds.
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 */
Status ColorControlCluster::moveToHueAndSaturation(uint16_t hue, uint8_t saturation, uint16_t transitionTimeDs, bool isEnhanced,
                                                   BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // Same constraint checks as moveToSaturation (its twin): reject before any mode switch / transition.
    // Note: `hue` is unconstrained — 8-bit legacy hue and 16-bit enhanced hue both span their full range.
    VerifyOrReturnValue(saturation <= MAX_SATURATION_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);
    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);
    uint64_t now       = SystemClock().GetMonotonicMilliseconds64().count();
    uint16_t targetHue = isEnhanced ? hue : static_cast<uint16_t>(hue << 8); // TARGET = requested hue

    HueSatTransition next{};

    // Saturation is independent of the color loop → it always applies.
    next.sat = SatTransition{
        .startSat    = GetSaturation(),
        .targetSat   = std::clamp<uint8_t>(saturation, MIN_SATURATION_VALUE, MAX_SATURATION_VALUE),
        .startTimeMs = now,
        .durationMs  = transitionTimeDs * 100u,
    };

    // Hue IS gated (answering the note that was here): an active color loop owns the hue axis
    // (§3.2.8.1), so start the hue axis only when no loop is running — saturation moves either way.
    if (!(HasFeature(Feature::kColorLoop) && mColorLoop.active))
    {
        const uint16_t start    = GetEnhancedHue();
        const uint16_t upArc    = static_cast<uint16_t>(targetHue - start);                          // distance going up (wraps)
        const int32_t signedArc = (upArc <= 0x8000) ? upArc : (static_cast<int32_t>(upArc) - 65536); // shortest
        next.hue                = HueTransition{ start, signedArc, now, transitionTimeDs * 100u };
    }

    mTransition = next;
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

// ── Scene-restore transition helpers ────────────────────────────────────────────────────────────
// The shared "populate mTransition + stamp the clock + set RemainingTime + arm the tick" core. They
// do NOT switch mode — the caller (HandleApplyScene, or a command) is responsible for calling
// ApplyModeSwitch first, so on entry mColorValue is already the matching alternative and the start of
// the transition is whatever the current materialized value is (mid-flight applies pick up from there).
//
// `target` is the scene's saved ColorValue (same alternative as the active mode); `timeMs` is the
// transition time in milliseconds → durationMs directly; RemainingTime is reported in deciseconds.

CHIP_ERROR ColorControlCluster::StartXYTransition(const ColorControl::ColorValue & target, uint16_t timeMs)
{
    const auto & tgt = std::get<XYColor>(target);
    auto & xy        = std::get<XYColor>(mColorValue); // caller already switched mode → get<> is safe

    mTransition = XYTransition{
        .startX      = xy.x,
        .targetX     = tgt.x,
        .durationXMs = timeMs,
        .startY      = xy.y,
        .targetY     = tgt.y,
        .durationYMs = timeMs, // one duration → both axes arrive together
        .startTimeMs = SystemClock().GetMonotonicMilliseconds64().count(),
    };
    SetQuietReportRemainingTime(static_cast<uint16_t>(timeMs / 100), /*isNewTransition=*/true);
    ArmTick();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlCluster::StartColorTemperatureTransition(const ColorControl::ColorValue & target, uint16_t timeMs)
{
    const auto & tgt = std::get<CTColor>(target);
    auto & ct        = std::get<CTColor>(mColorValue);

    mTransition = CTTransition{
        .startMireds  = ct.mireds,
        .targetMireds = std::clamp<uint16_t>(tgt.mireds, mCT.colorTempPhysicalMinMireds,
                                             mCT.colorTempPhysicalMaxMireds), // scenes clamp to physical range
        .startTimeMs  = SystemClock().GetMonotonicMilliseconds64().count(),
        .durationMs   = timeMs,
    };
    SetQuietReportRemainingTime(static_cast<uint16_t>(timeMs / 100), /*isNewTransition=*/true);
    ArmTick();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlCluster::StartHueAndSatTransition(const ColorControl::ColorValue & target, uint16_t timeMs)
{
    const auto & tgt   = std::get<HueSatColor>(target);
    auto & hs          = std::get<HueSatColor>(mColorValue);
    const uint64_t now = SystemClock().GetMonotonicMilliseconds64().count();

    // Hue always runs in 16-bit canonical; legacy hue is the high byte, so project both ends up and
    // take the shortest arc round the circle (same math the hue commands compute inline).
    const uint16_t start = hs.enhancedHue(), tgtHue = tgt.enhancedHue();
    const uint16_t upArc = static_cast<uint16_t>(tgtHue - start);
    const int32_t arc    = (upArc <= 0x8000) ? upArc : (static_cast<int32_t>(upArc) - 65536);

    HueSatTransition next{};
    next.hue    = HueTransition{ start, arc, now, timeMs };
    next.sat    = SatTransition{ .startSat = hs.saturation, .targetSat = tgt.saturation, .startTimeMs = now, .durationMs = timeMs };
    mTransition = next;

    SetQuietReportRemainingTime(static_cast<uint16_t>(timeMs / 100), /*isNewTransition=*/true);
    ArmTick();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlCluster::StartEnhancedHueAndSatTransition(const ColorControl::ColorValue & target, uint16_t timeMs)
{
    const auto & tgt   = std::get<EnhancedHueSatColor>(target);
    auto & ehs         = std::get<EnhancedHueSatColor>(mColorValue);
    const uint64_t now = SystemClock().GetMonotonicMilliseconds64().count();

    const uint16_t start = ehs.enhancedHue, tgtHue = tgt.enhancedHue;
    const uint16_t upArc = static_cast<uint16_t>(tgtHue - start);
    const int32_t arc    = (upArc <= 0x8000) ? upArc : (static_cast<int32_t>(upArc) - 65536);

    HueSatTransition next{};
    next.hue = HueTransition{ start, arc, now, timeMs };
    next.sat = SatTransition{ .startSat = ehs.saturation, .targetSat = tgt.saturation, .startTimeMs = now, .durationMs = timeMs };
    mTransition = next;

    SetQuietReportRemainingTime(static_cast<uint16_t>(timeMs / 100), /*isNewTransition=*/true);
    ArmTick();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlCluster::HandleApplyScene(ColorControl::EnhancedColorModeEnum ColorMode,
                                                 const ColorControl::ColorValue & target, const ColorControl::ColorLoopState & loop,
                                                 uint16_t timeMs)
{
    if (loop.active == 1)
    {
        mColorLoop.active    = 1;
        mColorLoop.direction = loop.direction;
        mColorLoop.time      = loop.time;
        startColorLoop(/*startFromStartHue=*/true); // persists the current color
        PersistColorLoop();                         // persist the loop attrs the scene just applied
        return CHIP_NO_ERROR;
    }

    // Switch into the scene's mode ONCE (converts the current color into this representation, which
    // becomes the transition start); the pure Start* helpers below assume the variant already matches.
    ApplyModeSwitch(ColorMode);

    switch (ColorMode)
    {
    case ColorControl::EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
        return StartHueAndSatTransition(target, timeMs);
    case ColorControl::EnhancedColorModeEnum::kCurrentXAndCurrentY:
        return StartXYTransition(target, timeMs);
    case ColorControl::EnhancedColorModeEnum::kColorTemperatureMireds:
        return StartColorTemperatureTransition(target, timeMs);
    case ColorControl::EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation:
        return StartEnhancedHueAndSatTransition(target, timeMs);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
/**
 * @brief Executes move Hue Command.
 *
 * @param[in] endpoint EndpointId of the recipient Color control cluster.
 * @param[in] moveMode
 * @param[in] rate
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when Rate is 0 or an unknown moveMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a hue transition state,
 */
Status ColorControlCluster::moveHue(MoveModeEnum moveMode, uint16_t rate, bool isEnhanced, BitMask<OptionsBitmap> optionsMask,
                                    BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // §3.2.8.1: an active color loop owns the hue axis; hue commands must not disturb it.
    if (HasFeature(Feature::kColorLoop) && mColorLoop.active)
    {
        return Status::Success;
    }

    if (moveMode == MoveModeEnum::kStop)
    {
        // Stop the hue axis; a running saturation axis is independent (§3.2.5.2) and survives.
        if (auto * hsx = std::get_if<HueSatTransition>(&mTransition))
        {
            hsx->hue.reset();
            if (!hsx->sat)
                mTransition = std::monostate{};
        }
        SetQuietReportRemainingTime(0, /*isNewTransition=*/false);
        PersistCurrentColor(); // freeze: persist the hue the axis stopped at
        return Status::Success;
    }

    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    // direction is the SIGN of the rate; legacy 8-bit rate projects to 16-bit canonical
    int32_t signedRatePerSec = (isEnhanced ? rate : rate << 8) * (moveMode == MoveModeEnum::kUp ? +1 : -1);

    auto & hs = EnsureHueSatTransition(); // preserve a running sat axis (§3.2.5.2)
    hs.hue    = HueTransition{
           .startHue    = GetEnhancedHue(), // 16-bit canonical current
           .signedDelta = signedRatePerSec, // hue-units per second; sign = up/down
           .startTimeMs = SystemClock().GetMonotonicMilliseconds64().count(),
           .durationMs  = kIndefiniteHueMoveMs, // rate move: runs until StopHueAxis
    };
    SetQuietReportRemainingTime(MAX_INT16U_VALUE, /*isNewTransition=*/true); // indefinite
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

/**
 * @brief Executes move to hue command.
 *
 * @param[in] endpoint EndpointId of the recipient Color control cluster.
 * @param[in] hue
 * @param[in] moveDirection
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when Rate is 0 or an unknown moveDirection is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a hue transition state,
 *         Status::ConstraintError when the other parameters are outside their defined value range.
 */
Status ColorControlCluster::moveToHue(uint16_t hue, DirectionEnum dir, uint16_t transitionTimeDs, bool isEnhanced,
                                      BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // §3.2.8.1: an active color loop owns the hue axis; hue commands must not disturb it.
    if (HasFeature(Feature::kColorLoop) && mColorLoop.active)
    {
        return Status::Success;
    }

    VerifyOrReturnValue(dir != DirectionEnum::kUnknownEnumValue, Status::InvalidCommand);
    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    const uint16_t start  = GetEnhancedHue();                                   // 16-bit canonical
    const uint16_t target = isEnhanced ? hue : static_cast<uint16_t>(hue << 8); // project legacy up
    const uint16_t upArc  = static_cast<uint16_t>(target - start);              // distance going up (wraps)

    bool up;
    switch (dir)
    {
    case DirectionEnum::kUp:
        up = true;
        break;
    case DirectionEnum::kDown:
        up = false;
        break;
    case DirectionEnum::kShortest:
        up = (upArc <= 0x8000);
        break; // up is the shorter arc
    case DirectionEnum::kLongest:
        up = (upArc > 0x8000);
        break;
    case DirectionEnum::kUnknownEnumValue:
        return Status::InvalidCommand;
    }
    const int32_t signedArc = up ? upArc : (static_cast<int32_t>(upArc) - 65536);

    auto & hs = EnsureHueSatTransition();
    hs.hue    = HueTransition{ start, signedArc, SystemClock().GetMonotonicMilliseconds64().count(), transitionTimeDs * 100u };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

/**
 * @brief Executes step hue command.
 *
 * @param[in] endpoint
 * @param[in] stepMode
 * @param[in] stepSize
 * @param[in] transitionTime
 * @param[in] optionsMask
 * @param[in] optionsOverride
 * @param[in] isEnhanced If True, function was called by EnhancedMoveHue command and rate is a uint16 value. If False function
 * was called by MoveHue command and rate is a uint8 value.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when StepSize is 0 or an unknown StepModeEnum is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a hue transition state.
 *         Status::ConstraintError when the other parameters are outside their defined value range.
 */
Status ColorControlCluster::stepHue(StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs, bool isEnhanced,
                                    BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // §3.2.8.1: an active color loop owns the hue axis; hue commands must not disturb it.
    if (HasFeature(Feature::kColorLoop) && mColorLoop.active)
    {
        return Status::Success;
    }

    VerifyOrReturnValue(stepMode != StepModeEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(stepSize != 0, Status::InvalidCommand);

    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    const int32_t step16      = isEnhanced ? stepSize : static_cast<int32_t>(stepSize) << 8;
    const int32_t signedDelta = (stepMode == StepModeEnum::kUp ? +1 : -1) * step16; // sign from stepMode

    auto & hs = EnsureHueSatTransition();
    hs.hue =
        HueTransition{ GetEnhancedHue(), signedDelta, SystemClock().GetMonotonicMilliseconds64().count(), transitionTimeDs * 100u };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

/**
 * @brief Executes moveSaturation command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a rate of 0 for a non-stop move or an unknown MoveModeEnum is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a saturation transition state.
 */
Status ColorControlCluster::moveSaturation(MoveModeEnum moveMode, uint8_t rate, BitMask<OptionsBitmap> optionsMask,
                                           BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(moveMode != MoveModeEnum::kUnknownEnumValue, Status::InvalidCommand);

    if (moveMode == MoveModeEnum::kStop)
    {
        // Stop the saturation axis; a running hue axis is independent (§3.2.5.2) and survives.
        if (auto * hsx = std::get_if<HueSatTransition>(&mTransition))
        {
            hsx->sat.reset();
            if (!hsx->hue)
                mTransition = std::monostate{};
        }
        SetQuietReportRemainingTime(0, /*isNewTransition=*/false); // stopped → RemainingTime 0, like moveHue Stop
        PersistCurrentColor();                                     // freeze: persist the saturation the axis stopped at
        return Status::Success;
    }
    VerifyOrReturnValue(rate != 0, Status::ConstraintError); // rate 0 only valid for Stop

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation); // basic HS, not enhanced

    const uint8_t start       = GetSaturation();
    const uint8_t target      = (moveMode == MoveModeEnum::kUp) ? MAX_SATURATION_VALUE : MIN_SATURATION_VALUE;
    const uint32_t distance   = (start > target) ? (start - target) : (target - start);
    const uint32_t durationMs = distance * 1000u / rate; // distance / rate → ms

    auto & hs = EnsureHueSatTransition(); // preserve a running HUE axis (§3.2.5.2)
    hs.sat    = SatTransition{
           .startSat    = start,
           .targetSat   = target, // the boundary — bounded, so it stops here
           .startTimeMs = SystemClock().GetMonotonicMilliseconds64().count(),
           .durationMs  = durationMs,
    };
    SetQuietReportRemainingTime(static_cast<uint16_t>(durationMs / 100), /*isNewTransition=*/true); // finite
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

/**
 * @brief Executes step saturation command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a step size of 0 or an unknown StepModeEnum is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a saturation transition state,
 */
Status ColorControlCluster::stepSaturation(StepModeEnum stepMode, uint8_t stepSize, uint16_t transitionTimeDs,
                                           BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(stepMode != StepModeEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(stepSize != 0, Status::InvalidCommand);

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    const int32_t signedDelta = (stepMode == StepModeEnum::kUp ? +1 : -1) * static_cast<int32_t>(stepSize);
    const uint8_t start       = GetSaturation();
    const uint8_t target =
        static_cast<uint8_t>(std::clamp<int32_t>(int32_t(start) + signedDelta, MIN_SATURATION_VALUE, MAX_SATURATION_VALUE));

    auto & hs = EnsureHueSatTransition(); // preserve a running HUE axis
    hs.sat    = SatTransition{ start, target, SystemClock().GetMonotonicMilliseconds64().count(), transitionTimeDs * 100u };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

void ColorControlCluster::stopColorLoop()
{
    if (!mColorLoop.active)
        return;
    mColorLoop.active                                      = 0;
    std::get<EnhancedHueSatColor>(mColorValue).enhancedHue = mColorLoop.storedEnhancedHue; // restore
    NotifyAttributeChanged(ColorLoopActive::Id, AttributeChangeType::kReportable);
    NotifyAttributeChanged(EnhancedCurrentHue::Id, AttributeChangeType::kReportable);
    // Deactivate restored EnhancedCurrentHue → persist the settled color (loop attrs persisted by the caller).
    PersistCurrentColor();
    // tick self-terminates once no axis/loop is active
}

/**
 * @brief Executes ColorLoop command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when an unknown action or direction is provided
 */
Status ColorControlCluster::colorLoopSet(BitMask<UpdateFlagsBitmap> updateFlags, ColorLoopActionEnum action,
                                         ColorLoopDirectionEnum direction, uint16_t time, uint16_t startHue,
                                         BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(action != ColorLoopActionEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(direction != ColorLoopDirectionEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);

    if (updateFlags.Has(UpdateFlagsBitmap::kUpdateDirection))
    {
        mColorLoop.direction = to_underlying(direction);
        NotifyAttributeChanged(Attributes::ColorLoopDirection::Id, AttributeChangeType::kReportable);
    }
    if (updateFlags.Has(UpdateFlagsBitmap::kUpdateTime))
    {
        mColorLoop.time = time;
        NotifyAttributeChanged(ColorLoopTime::Id, AttributeChangeType::kReportable);
    }
    if (updateFlags.Has(UpdateFlagsBitmap::kUpdateStartHue))
    {
        mColorLoop.startEnhancedHue = startHue;
        NotifyAttributeChanged(ColorLoopStartEnhancedHue::Id, AttributeChangeType::kReportable);
    }
    if (updateFlags.Has(UpdateFlagsBitmap::kUpdateAction))
    { // action LAST
        switch (action)
        {
        case ColorLoopActionEnum::kDeactivate:
            stopColorLoop();
            break;
        case ColorLoopActionEnum::kActivateFromColorLoopStartEnhancedHue:
            startColorLoop(true);
            break;
        case ColorLoopActionEnum::kActivateFromEnhancedCurrentHue:
            startColorLoop(false);
            break;
        default:
            return Status::InvalidCommand;
        }
    }
    // Persist the loop attrs (ColorLoopActive/Direction/Time are all NVM). start/stopColorLoop already
    // persisted the current color when an action switched mode/hue.
    PersistColorLoop();
    InvalidateScenes();
    return Status::Success;
}

// Advertise the attributes this cluster actually serves: the always-present mandatory set, the
// feature-gated optionals, and the Fixed descriptors (primaries / white-point / color-points) — the
// latter advertised only when the app supplied that descriptor in StaticConfig, matching what
// ReadAttribute will actually produce (an unsupplied descriptor is absent, not just null).
CHIP_ERROR ColorControlCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const bool hs = HasFeature(Feature::kHueAndSaturation);
    const bool xy = HasFeature(Feature::kXy);
    const bool ct = HasFeature(Feature::kColorTemperature);
    const bool eh = HasFeature(Feature::kEnhancedHue);
    const bool cl = HasFeature(Feature::kColorLoop);

    // Fixed-descriptor presence: supplied table AND the specific optional engaged. Each primary gates its
    // X / Y / Intensity together (Intensity's value stays nullable via ChromaticityPoint::intensity).
    const StaticConfig * sc = mStaticConfig;
    const bool p1           = sc && sc->primaries[0].has_value();
    const bool p2           = sc && sc->primaries[1].has_value();
    const bool p3           = sc && sc->primaries[2].has_value();
    const bool p4           = sc && sc->primaries[3].has_value();
    const bool p5           = sc && sc->primaries[4].has_value();
    const bool p6           = sc && sc->primaries[5].has_value();
    const bool wpx          = sc && sc->whitePointX.has_value();
    const bool wpy          = sc && sc->whitePointY.has_value();
    const bool cpr          = sc && sc->colorPointR.has_value();
    const bool cpg          = sc && sc->colorPointG.has_value();
    const bool cpb          = sc && sc->colorPointB.has_value();

    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { true, ColorControl::Attributes::RemainingTime::kMetadataEntry }, // optional, but we run transitions

        { hs, ColorControl::Attributes::CurrentHue::kMetadataEntry },
        { hs, ColorControl::Attributes::CurrentSaturation::kMetadataEntry },

        { xy, ColorControl::Attributes::CurrentX::kMetadataEntry },
        { xy, ColorControl::Attributes::CurrentY::kMetadataEntry },

        { eh, ColorControl::Attributes::EnhancedCurrentHue::kMetadataEntry },

        { cl, ColorControl::Attributes::ColorLoopActive::kMetadataEntry },
        { cl, ColorControl::Attributes::ColorLoopDirection::kMetadataEntry },
        { cl, ColorControl::Attributes::ColorLoopTime::kMetadataEntry },
        { cl, ColorControl::Attributes::ColorLoopStartEnhancedHue::kMetadataEntry },
        { cl, ColorControl::Attributes::ColorLoopStoredEnhancedHue::kMetadataEntry },

        { ct, ColorControl::Attributes::ColorTemperatureMireds::kMetadataEntry },
        { ct, ColorControl::Attributes::ColorTempPhysicalMinMireds::kMetadataEntry },
        { ct, ColorControl::Attributes::ColorTempPhysicalMaxMireds::kMetadataEntry },
        { ct, ColorControl::Attributes::CoupleColorTempToLevelMinMireds::kMetadataEntry },
        { ct, ColorControl::Attributes::StartUpColorTemperatureMireds::kMetadataEntry },

        { p1, ColorControl::Attributes::Primary1X::kMetadataEntry },
        { p1, ColorControl::Attributes::Primary1Y::kMetadataEntry },
        { p1, ColorControl::Attributes::Primary1Intensity::kMetadataEntry },
        { p2, ColorControl::Attributes::Primary2X::kMetadataEntry },
        { p2, ColorControl::Attributes::Primary2Y::kMetadataEntry },
        { p2, ColorControl::Attributes::Primary2Intensity::kMetadataEntry },
        { p3, ColorControl::Attributes::Primary3X::kMetadataEntry },
        { p3, ColorControl::Attributes::Primary3Y::kMetadataEntry },
        { p3, ColorControl::Attributes::Primary3Intensity::kMetadataEntry },
        { p4, ColorControl::Attributes::Primary4X::kMetadataEntry },
        { p4, ColorControl::Attributes::Primary4Y::kMetadataEntry },
        { p4, ColorControl::Attributes::Primary4Intensity::kMetadataEntry },
        { p5, ColorControl::Attributes::Primary5X::kMetadataEntry },
        { p5, ColorControl::Attributes::Primary5Y::kMetadataEntry },
        { p5, ColorControl::Attributes::Primary5Intensity::kMetadataEntry },
        { p6, ColorControl::Attributes::Primary6X::kMetadataEntry },
        { p6, ColorControl::Attributes::Primary6Y::kMetadataEntry },
        { p6, ColorControl::Attributes::Primary6Intensity::kMetadataEntry },

        { wpx, ColorControl::Attributes::WhitePointX::kMetadataEntry },
        { wpy, ColorControl::Attributes::WhitePointY::kMetadataEntry },

        { cpr, ColorControl::Attributes::ColorPointRX::kMetadataEntry },
        { cpr, ColorControl::Attributes::ColorPointRY::kMetadataEntry },
        { cpr, ColorControl::Attributes::ColorPointRIntensity::kMetadataEntry },
        { cpg, ColorControl::Attributes::ColorPointGX::kMetadataEntry },
        { cpg, ColorControl::Attributes::ColorPointGY::kMetadataEntry },
        { cpg, ColorControl::Attributes::ColorPointGIntensity::kMetadataEntry },
        { cpb, ColorControl::Attributes::ColorPointBX::kMetadataEntry },
        { cpb, ColorControl::Attributes::ColorPointBY::kMetadataEntry },
        { cpb, ColorControl::Attributes::ColorPointBIntensity::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(ColorControl::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

// Feature-gated command list. StopMoveStep is accepted whenever any movement feature is present.
CHIP_ERROR ColorControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    namespace C   = Commands;
    const bool hs = HasFeature(Feature::kHueAndSaturation);
    const bool xy = HasFeature(Feature::kXy);
    const bool ct = HasFeature(Feature::kColorTemperature);
    const bool eh = HasFeature(Feature::kEnhancedHue);
    const bool cl = HasFeature(Feature::kColorLoop);

    struct GatedCommand
    {
        bool enabled;
        DataModel::AcceptedCommandEntry entry;
    };
    const GatedCommand commands[] = {
        { hs, Commands::MoveToHue::kMetadataEntry },
        { hs, Commands::MoveHue::kMetadataEntry },
        { hs, Commands::StepHue::kMetadataEntry },
        { hs, Commands::MoveToSaturation::kMetadataEntry },
        { hs, Commands::MoveSaturation::kMetadataEntry },
        { hs, Commands::StepSaturation::kMetadataEntry },
        { hs, Commands::MoveToHueAndSaturation::kMetadataEntry },

        { xy, Commands::MoveToColor::kMetadataEntry },
        { xy, Commands::MoveColor::kMetadataEntry },
        { xy, Commands::StepColor::kMetadataEntry },

        { ct, Commands::MoveToColorTemperature::kMetadataEntry },
        { ct, Commands::MoveColorTemperature::kMetadataEntry },
        { ct, Commands::StepColorTemperature::kMetadataEntry },

        { eh, Commands::EnhancedMoveToHue::kMetadataEntry },
        { eh, Commands::EnhancedMoveHue::kMetadataEntry },
        { eh, Commands::EnhancedStepHue::kMetadataEntry },
        { eh, Commands::EnhancedMoveToHueAndSaturation::kMetadataEntry },

        { cl, Commands::ColorLoopSet::kMetadataEntry },

        { hs || xy || ct, Commands::StopMoveStep::kMetadataEntry },
    };

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(MATTER_ARRAY_SIZE(commands)));
    for (const auto & c : commands)
    {
        if (c.enabled)
        {
            ReturnErrorOnFailure(builder.Append(c.entry));
        }
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ColorControlCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                  AttributeValueDecoder & decoder)
{
    // Only two writable attributes: Options and StartUpColorTemperatureMireds. Everything else
    // (current color, derived modes, physical limits, primaries/white-point) is read-only → the
    // default below returns UnsupportedWrite.
    switch (request.path.mAttributeId)
    {
    case StartUpColorTemperatureMireds::Id: {
        DataModel::Nullable<uint16_t> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        // null = "keep previous value on startup"; a concrete value must be a legal mired (<= 0xFEFF).
        VerifyOrReturnError(value.IsNull() || value.Value() <= kMaxColorTemperatureMireds, Status::ConstraintError);
        mCT.startUpColorTemperatureMireds = value;
        // NVM attribute: persist in the same native-endian storage format Startup() loads (null → sentinel).
        NumericAttributeTraits<uint16_t>::StorageType storage;
        DataModel::NullableToStorage(mCT.startUpColorTemperatureMireds, storage);
        PersistValue(StartUpColorTemperatureMireds::Id, storage);
        NotifyAttributeChanged(request.path.mAttributeId);
        return Status::Success;
    }
    case Options::Id: {
        BitMask<OptionsBitmap> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        mState.options = value;
        NotifyAttributeChanged(request.path.mAttributeId);
        return Status::Success;
    }
    }

    return Status::UnsupportedWrite;
}

// Decode each command's payload from the TLV, then hand the fields to the matching handler. The
// handlers own validation, mode-switch, options gating and the transition itself — InvokeCommand is
// pure wiring. The Enhanced* commands share a handler with their legacy twin, distinguished by the
// isEnhanced flag (legacy hue is 8-bit, enhanced is 16-bit; the handler projects accordingly).
std::optional<DataModel::ActionReturnStatus> ColorControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                chip::TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::MoveToHue::Id: {
        Commands::MoveToHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveToHue(data.hue, data.direction, data.transitionTime, /*isEnhanced=*/false, data.optionsMask,
                         data.optionsOverride);
    }
    case Commands::MoveHue::Id: {
        Commands::MoveHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveHue(data.moveMode, data.rate, /*isEnhanced=*/false, data.optionsMask, data.optionsOverride);
    }
    case Commands::StepHue::Id: {
        Commands::StepHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return stepHue(data.stepMode, data.stepSize, data.transitionTime, /*isEnhanced=*/false, data.optionsMask,
                       data.optionsOverride);
    }
    case Commands::MoveToSaturation::Id: {
        Commands::MoveToSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveToSaturation(data.saturation, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveSaturation::Id: {
        Commands::MoveSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveSaturation(data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::StepSaturation::Id: {
        Commands::StepSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return stepSaturation(data.stepMode, data.stepSize, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveToHueAndSaturation::Id: {
        Commands::MoveToHueAndSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveToHueAndSaturation(data.hue, data.saturation, data.transitionTime, /*isEnhanced=*/false, data.optionsMask,
                                      data.optionsOverride);
    }
    case Commands::MoveToColor::Id: {
        Commands::MoveToColor::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveToColor(data.colorX, data.colorY, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveColor::Id: {
        Commands::MoveColor::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveColor(data.rateX, data.rateY, data.optionsMask, data.optionsOverride);
    }
    case Commands::StepColor::Id: {
        Commands::StepColor::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return stepColor(data.stepX, data.stepY, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveToColorTemperature::Id: {
        Commands::MoveToColorTemperature::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveToColorTemp(data.colorTemperatureMireds, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveColorTemperature::Id: {
        Commands::MoveColorTemperature::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveColorTemp(data.moveMode, data.rate, data.colorTemperatureMinimumMireds, data.colorTemperatureMaximumMireds,
                             data.optionsMask, data.optionsOverride);
    }
    case Commands::StepColorTemperature::Id: {
        Commands::StepColorTemperature::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return stepColorTemp(data.stepMode, data.stepSize, data.transitionTime, data.colorTemperatureMinimumMireds,
                             data.colorTemperatureMaximumMireds, data.optionsMask, data.optionsOverride);
    }

    // ---- Enhanced* commands: same handlers, isEnhanced = true ----
    case Commands::EnhancedMoveToHue::Id: {
        Commands::EnhancedMoveToHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveToHue(data.enhancedHue, data.direction, data.transitionTime, /*isEnhanced=*/true, data.optionsMask,
                         data.optionsOverride);
    }
    case Commands::EnhancedMoveHue::Id: {
        Commands::EnhancedMoveHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveHue(data.moveMode, data.rate, /*isEnhanced=*/true, data.optionsMask, data.optionsOverride);
    }
    case Commands::EnhancedStepHue::Id: {
        Commands::EnhancedStepHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return stepHue(data.stepMode, data.stepSize, data.transitionTime, /*isEnhanced=*/true, data.optionsMask,
                       data.optionsOverride);
    }
    case Commands::EnhancedMoveToHueAndSaturation::Id: {
        Commands::EnhancedMoveToHueAndSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return moveToHueAndSaturation(data.enhancedHue, data.saturation, data.transitionTime, /*isEnhanced=*/true, data.optionsMask,
                                      data.optionsOverride);
    }

    case Commands::ColorLoopSet::Id: {
        Commands::ColorLoopSet::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return colorLoopSet(data.updateFlags, data.action, data.direction, data.time, data.startHue, data.optionsMask,
                            data.optionsOverride);
    }
    case Commands::StopMoveStep::Id: {
        Commands::StopMoveStep::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleStopMoveStep(data, handler);
    }

    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus ColorControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    const EndpointId ep = mPath.mEndpointId;

    // Fixed descriptor readers: a descriptor exists only if the app supplied the table (mStaticConfig) AND
    // the specific optional is engaged; otherwise the attribute is genuinely absent → UnsupportedAttribute.
    // `field` selects one std::optional<ChromaticityPoint> and `proj` picks x / y / intensity off it.
    auto point = [&](std::optional<ChromaticityPoint> StaticConfig::*field, auto proj) -> DataModel::ActionReturnStatus {
        if (mStaticConfig == nullptr || !(mStaticConfig->*field).has_value())
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(proj((mStaticConfig->*field).value()));
    };
    // Same, for a Primary by index (0..5) into the primaries[] array.
    auto primary = [&](size_t i, auto proj) -> DataModel::ActionReturnStatus {
        if (mStaticConfig == nullptr || !mStaticConfig->primaries[i].has_value())
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(proj(mStaticConfig->primaries[i].value()));
    };
    const auto pointX = [](const ChromaticityPoint & p) { return p.x; };
    const auto pointY = [](const ChromaticityPoint & p) { return p.y; };
    const auto pointI = [](const ChromaticityPoint & p) { return p.intensity; };

    // Color axes are projected inline: the active mode returns its stored value / projection, the other
    // modes go through a single Convert*. A read NEVER switches mode (that is a command's job) and NEVER
    // interpolates (OnTick already materialized the value into mColorValue). Only the requested axis is
    // converted. EnhancedHue is 16-bit canonical; CurrentHue is its high byte.
    switch (request.path.mAttributeId)
    {
    // Global attributes the cluster is responsible for serving (like LevelControl/valve). The framework
    // fills the list globals (AttributeList/AcceptedCommandList/...), but FeatureMap/ClusterRevision are ours.
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(ColorControl::kRevision);

    case CurrentHue::Id: {
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            return encoder.Encode(hs->hue);
        }
        if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            return encoder.Encode(e->hue());
        }
        uint8_t h = HueSatColor{}.hue, s = HueSatColor{}.saturation;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate->ConvertXYToHueSat(ep, c->x, c->y, h, s);
        }
        else
        {
            mDelegate->ConvertMiredsToHueSat(ep, std::get<CTColor>(mColorValue).mireds, h, s);
        }
        return encoder.Encode(h);
    }

    case EnhancedCurrentHue::Id: {
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            return encoder.Encode(hs->enhancedHue());
        }
        if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            return encoder.Encode(e->enhancedHue);
        }
        uint8_t h = HueSatColor{}.hue, s = HueSatColor{}.saturation;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate->ConvertXYToHueSat(ep, c->x, c->y, h, s);
        }
        else
        {
            mDelegate->ConvertMiredsToHueSat(ep, std::get<CTColor>(mColorValue).mireds, h, s);
        }
        return encoder.Encode(static_cast<uint16_t>(static_cast<uint16_t>(h) << 8));
    }

    case CurrentSaturation::Id: {
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            return encoder.Encode(hs->saturation);
        }
        if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            return encoder.Encode(e->saturation);
        }
        uint8_t h = HueSatColor{}.hue, s = HueSatColor{}.saturation;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate->ConvertXYToHueSat(ep, c->x, c->y, h, s);
        }
        else
        {
            mDelegate->ConvertMiredsToHueSat(ep, std::get<CTColor>(mColorValue).mireds, h, s);
        }
        return encoder.Encode(s);
    }

    case CurrentX::Id: {
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            return encoder.Encode(c->x);
        }
        uint16_t x = XYColor{}.x, y = XYColor{}.y;
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToXY(ep, hs->hue, hs->saturation, x, y);
        }
        else if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToXY(ep, e->hue(), e->saturation, x, y);
        }
        else
        {
            mDelegate->ConvertMiredsToXY(ep, std::get<CTColor>(mColorValue).mireds, x, y);
        }
        return encoder.Encode(x);
    }

    case CurrentY::Id: {
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            return encoder.Encode(c->y);
        }
        uint16_t x = XYColor{}.x, y = XYColor{}.y;
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToXY(ep, hs->hue, hs->saturation, x, y);
        }
        else if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToXY(ep, e->hue(), e->saturation, x, y);
        }
        else
        {
            mDelegate->ConvertMiredsToXY(ep, std::get<CTColor>(mColorValue).mireds, x, y);
        }
        return encoder.Encode(y);
    }

    case ColorTemperatureMireds::Id: {
        if (auto * ct = std::get_if<CTColor>(&mColorValue))
        {
            return encoder.Encode(ct->mireds);
        }
        uint16_t m = CTColor{}.mireds;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate->ConvertXYToMireds(ep, c->x, c->y, m);
        }
        else if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate->ConvertHueSatToMireds(ep, hs->hue, hs->saturation, m);
        }
        else
        {
            const auto & e = std::get<EnhancedHueSatColor>(mColorValue);
            mDelegate->ConvertHueSatToMireds(ep, e.hue(), e.saturation, m);
        }
        return encoder.Encode(m);
    }

    // ColorMode / EnhancedColorMode are DERIVED from the active variant — never stored. Enhanced HS
    // collapses to CurrentHueAndCurrentSaturation(0) in the non-enhanced ColorMode (§3.2).
    case Attributes::ColorMode::Id:
        return encoder.Encode(std::holds_alternative<XYColor>(mColorValue)       ? ColorModeEnum::kCurrentXAndCurrentY
                                  : std::holds_alternative<CTColor>(mColorValue) ? ColorModeEnum::kColorTemperatureMireds
                                                                                 : ColorModeEnum::kCurrentHueAndCurrentSaturation);
    case Attributes::EnhancedColorMode::Id:
        // Derived from the active variant; GetEnhancedColorMode() is the single source of that mapping.
        return encoder.Encode(GetEnhancedColorMode());

    // ---- plain stored state ----
    case RemainingTime::Id:
        return encoder.Encode(mState.remainingTime);
    case Options::Id:
        return encoder.Encode(mState.options);
    case Attributes::ColorCapabilities::Id:
        return encoder.Encode(mState.colorCapabilities);
    case NumberOfPrimaries::Id:
        return encoder.Encode(mState.numberOfPrimaries);

    // ---- color loop ----
    case ColorLoopActive::Id:
        return encoder.Encode(mColorLoop.active);
    case Attributes::ColorLoopDirection::Id:
        return encoder.Encode(mColorLoop.direction);
    case ColorLoopTime::Id:
        return encoder.Encode(mColorLoop.time);
    case ColorLoopStartEnhancedHue::Id:
        return encoder.Encode(mColorLoop.startEnhancedHue);
    case ColorLoopStoredEnhancedHue::Id:
        return encoder.Encode(mColorLoop.storedEnhancedHue);

    // ---- color-temperature limits / startup ----
    case ColorTempPhysicalMinMireds::Id:
        return encoder.Encode(mCT.colorTempPhysicalMinMireds);
    case ColorTempPhysicalMaxMireds::Id:
        return encoder.Encode(mCT.colorTempPhysicalMaxMireds);
    case CoupleColorTempToLevelMinMireds::Id:
        return encoder.Encode(mCT.coupleColorTempToLevelMinMireds);
    case StartUpColorTemperatureMireds::Id:
        return encoder.Encode(mCT.startUpColorTemperatureMireds);

    // ---- Fixed descriptors (§3.2.7): served from the app-owned StaticConfig, absent → UnsupportedAttribute.
    case Primary1X::Id:
        return primary(0, pointX);
    case Primary1Y::Id:
        return primary(0, pointY);
    case Primary1Intensity::Id:
        return primary(0, pointI);
    case Primary2X::Id:
        return primary(1, pointX);
    case Primary2Y::Id:
        return primary(1, pointY);
    case Primary2Intensity::Id:
        return primary(1, pointI);
    case Primary3X::Id:
        return primary(2, pointX);
    case Primary3Y::Id:
        return primary(2, pointY);
    case Primary3Intensity::Id:
        return primary(2, pointI);
    case Primary4X::Id:
        return primary(3, pointX);
    case Primary4Y::Id:
        return primary(3, pointY);
    case Primary4Intensity::Id:
        return primary(3, pointI);
    case Primary5X::Id:
        return primary(4, pointX);
    case Primary5Y::Id:
        return primary(4, pointY);
    case Primary5Intensity::Id:
        return primary(4, pointI);
    case Primary6X::Id:
        return primary(5, pointX);
    case Primary6Y::Id:
        return primary(5, pointY);
    case Primary6Intensity::Id:
        return primary(5, pointI);

    case WhitePointX::Id:
        VerifyOrReturnValue(mStaticConfig && mStaticConfig->whitePointX.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(mStaticConfig->whitePointX.value());
    case WhitePointY::Id:
        VerifyOrReturnValue(mStaticConfig && mStaticConfig->whitePointY.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(mStaticConfig->whitePointY.value());

    case ColorPointRX::Id:
        return point(&StaticConfig::colorPointR, pointX);
    case ColorPointRY::Id:
        return point(&StaticConfig::colorPointR, pointY);
    case ColorPointRIntensity::Id:
        return point(&StaticConfig::colorPointR, pointI);
    case ColorPointGX::Id:
        return point(&StaticConfig::colorPointG, pointX);
    case ColorPointGY::Id:
        return point(&StaticConfig::colorPointG, pointY);
    case ColorPointGIntensity::Id:
        return point(&StaticConfig::colorPointG, pointI);
    case ColorPointBX::Id:
        return point(&StaticConfig::colorPointB, pointX);
    case ColorPointBY::Id:
        return point(&StaticConfig::colorPointB, pointY);
    case ColorPointBIntensity::Id:
        return point(&StaticConfig::colorPointB, pointI);

    // FeatureMap / ClusterRevision are served at the top of this switch; the list globals
    // (AttributeList / AcceptedCommandList / GeneratedCommandList) are filled by the framework.
    default:
        return Status::UnsupportedAttribute;
    }
}

/**
 * @brief Executes the MoveColor command: continuously moves CurrentX / CurrentY toward the CIE
 *        boundary at the given signed rates (xy-units per second), running until StopMoveStep. A rate
 *        of 0 leaves that axis stationary; both rates 0 stops any active XY movement.
 *
 * @param rateX signed rate for CurrentX in xy-units per second
 * @param rateY signed rate for CurrentY in xy-units per second
 * @return Status::Success (gated by ShouldExecuteIfOff; the command has no out-of-range fields).
 */
Status ColorControlCluster::moveColor(int16_t rateX, int16_t rateY, BitMask<OptionsBitmap> optionsMask,
                                      BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    if (rateX == 0 && rateY == 0)
    { // both zero → stop XY movement
        mTransition = std::monostate{};
        SetQuietReportRemainingTime(0, /*isNewTransition=*/false); // stopped → RemainingTime 0, like moveHue Stop
        PersistCurrentColor();                                     // freeze: persist the XY the movement stopped at
        return Status::Success;
    }

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentXAndCurrentY);
    auto & xy = std::get<XYColor>(mColorValue);

    const uint16_t targetX = (rateX > 0) ? MAX_CIE_XY_VALUE : (rateX < 0 ? MIN_CIE_XY_VALUE : xy.x);
    const uint16_t targetY = (rateY > 0) ? MAX_CIE_XY_VALUE : (rateY < 0 ? MIN_CIE_XY_VALUE : xy.y);
    const uint32_t durX = rateX ? uint32_t(std::abs(int32_t(targetX) - xy.x)) * 1000u / static_cast<uint32_t>(std::abs(rateX)) : 0;
    const uint32_t durY = rateY ? uint32_t(std::abs(int32_t(targetY) - xy.y)) * 1000u / static_cast<uint32_t>(std::abs(rateY)) : 0;

    mTransition = XYTransition{
        .startX      = xy.x,
        .targetX     = targetX,
        .durationXMs = durX,
        .startY      = xy.y,
        .targetY     = targetY,
        .durationYMs = durY,
        .startTimeMs = SystemClock().GetMonotonicMilliseconds64().count(),
    };
    // Fresh-command RemainingTime edge report (§3.2.7.4): the slower axis's time-to-boundary in 1/10 s
    // (an idle axis contributed 0 duration, so max() is the moving axis).
    SetQuietReportRemainingTime(static_cast<uint16_t>(std::max(durX, durY) / 100), /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}
/**
 * @brief executes step color command
 * @param endpoint endpointId of the recipient Color control cluster
 * @param commandData Struct containing the parameters of the command
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a step X and Y of 0 is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a Color XY transition state,
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlCluster::stepColor(int16_t stepX, int16_t stepY, uint16_t transitionTimeDs, BitMask<OptionsBitmap> optionsMask,
                                      BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(!(stepX == 0 && stepY == 0), Status::InvalidCommand); // §3.2.8.13.4
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentXAndCurrentY);
    auto & xy = std::get<XYColor>(mColorValue);

    const uint16_t targetX = static_cast<uint16_t>(std::clamp<int32_t>(int32_t(xy.x) + stepX, MIN_CIE_XY_VALUE, MAX_CIE_XY_VALUE));
    const uint16_t targetY = static_cast<uint16_t>(std::clamp<int32_t>(int32_t(xy.y) + stepY, MIN_CIE_XY_VALUE, MAX_CIE_XY_VALUE));
    const uint32_t durationMs = transitionTimeDs * 100u;

    mTransition = XYTransition{
        .startX      = xy.x,
        .targetX     = targetX,
        .durationXMs = durationMs, // SAME duration
        .startY      = xy.y,
        .targetY     = targetY,
        .durationYMs = durationMs, // both axes arrive together
        .startTimeMs = SystemClock().GetMonotonicMilliseconds64().count(),
    };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

/**
 * @brief Executes move to color temp logic.
 *
 * @param aEndpoint
 * @param colorTemperature
 * @param transitionTime
 * @return Status::Success if successful, Status::UnsupportedEndpoint if the endpoint doesn't support color temperature.
 */
Status ColorControlCluster::moveToColorTemp(uint16_t colorTemperature, uint16_t transitionTimeDs,
                                            BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(colorTemperature <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(EnhancedColorModeEnum::kColorTemperatureMireds);
    auto & ct   = std::get<CTColor>(mColorValue);
    mTransition = CTTransition{
        .startMireds  = ct.mireds,
        .targetMireds = std::clamp<uint16_t>(colorTemperature, mCT.colorTempPhysicalMinMireds,
                                             mCT.colorTempPhysicalMaxMireds), // physical range
        .startTimeMs  = SystemClock().GetMonotonicMilliseconds64().count(),
        .durationMs   = transitionTimeDs * 100u,
    };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

/**
 * @brief Executes move to color command (absolute CIE xy target over a fixed transition time).
 *
 * @param colorX target CurrentX
 * @param colorY target CurrentY
 * @param transitionTimeDs transition time in 1/10ths of a second
 * @return Status::Success if successful, Status::ConstraintError if a parameter is out of range.
 */
Status ColorControlCluster::moveToColor(uint16_t colorX, uint16_t colorY, uint16_t transitionTimeDs,
                                        BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // Command parameter constraint checks (targets already validated → no clamp needed below):
    VerifyOrReturnValue(colorX <= MAX_CIE_XY_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(colorY <= MAX_CIE_XY_VALUE, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentXAndCurrentY);
    auto & xy = std::get<XYColor>(mColorValue);

    const uint32_t durationMs = transitionTimeDs * 100u;
    mTransition               = XYTransition{
                      .startX      = xy.x,
                      .targetX     = colorX,
                      .durationXMs = durationMs,
                      .startY      = xy.y,
                      .targetY     = colorY,
                      .durationYMs = durationMs, // same duration → both axes arrive together
                      .startTimeMs = SystemClock().GetMonotonicMilliseconds64().count(),
    };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

void ColorControlCluster::ApplyStartUpColorTemperature()
{
    // 07-5123-07 (i.e. ZCL 7) 5.2.2.2.1.22 StartUpColorTemperatureMireds Attribute
    // The StartUpColorTemperatureMireds attribute SHALL define the desired startup color
    // temperature values a lamp SHALL use when it is supplied with power and this value SHALL
    // be reflected in the ColorTemperatureMireds attribute. In addition, the ColorMode and
    // EnhancedColorMode attributes SHALL be set to 0x02 (color temperature). The values of
    // the StartUpColorTemperatureMireds attribute are listed in the table below.
    // Value                Action on power up
    // 0x0001-0xffef        Set the ColorTemperatureMireds attribute to this value.
    // null                 Set the ColorTemperatureMireds attribute to its previous value.

    if (mCT.startUpColorTemperatureMireds.IsNull())
        return; // "maintain previous value" — leave the loaded ColorTemperatureMireds

    const uint16_t physMin = std::max<uint16_t>(1u, mCT.colorTempPhysicalMinMireds); // divide-by-zero guard
    const uint16_t physMax = mCT.colorTempPhysicalMaxMireds;
    const uint16_t startUp = mCT.startUpColorTemperatureMireds.Value();

    // Out of physical range → treat as null (leave previous value untouched).
    if (startUp < physMin || startUp > physMax)
        return;

    // Switching the variant to CTColor IS the mode change — colorMode / enhancedColorMode are derived
    // from the alternative, so there is nothing else to set (§ they both read as ColorTemperature now).
    ApplyModeSwitch(EnhancedColorModeEnum::kColorTemperatureMireds);
    std::get<CTColor>(mColorValue).mireds = startUp;
}

/**
 * @brief Executes move color temp command.
 * @param endpoint EndpointId of the recipient Color control cluster.
 * @param commandData Struct containing the parameters of the command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when a rate of 0 for a non-stop move or an unknown MoveModeEnum is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a color temp transition state.
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlCluster::moveColorTemp(MoveModeEnum moveMode, uint16_t rate, uint16_t minFieldMireds, uint16_t maxFieldMireds,
                                          BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(minFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError); // fields: max 65279
    VerifyOrReturnValue(maxFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(moveMode != MoveModeEnum::kUnknownEnumValue, Status::InvalidCommand);

    if (moveMode == MoveModeEnum::kStop)
    { // Stop: stop movement, rate ignored. CT is single-axis, so clearing the driver is the whole stop.
        if (std::holds_alternative<CTTransition>(mTransition))
        {
            mTransition = std::monostate{};
            PersistCurrentColor(); // freeze: persist the mireds the movement stopped at
        }
        SetQuietReportRemainingTime(0, /*isNewTransition=*/false); // stopped → RemainingTime 0, like moveHue Stop
        return Status::Success;
    }
    VerifyOrReturnValue(rate != 0, Status::InvalidCommand); // rate 0 + non-stop → InvalidCommand

    // §3.2.8.21.3/.4: field==0 → physical limit; else clamp the field into the physical range
    const uint16_t lowerBound =
        (minFieldMireds == 0) ? mCT.colorTempPhysicalMinMireds : std::max(minFieldMireds, mCT.colorTempPhysicalMinMireds);
    const uint16_t upperBound =
        (maxFieldMireds == 0) ? mCT.colorTempPhysicalMaxMireds : std::min(maxFieldMireds, mCT.colorTempPhysicalMaxMireds);

    ApplyModeSwitch(EnhancedColorModeEnum::kColorTemperatureMireds);
    auto & ct = std::get<CTColor>(mColorValue);

    const uint16_t target     = (moveMode == MoveModeEnum::kUp) ? upperBound : lowerBound; // Up increases mireds
    const uint32_t distance   = (target > ct.mireds) ? (target - ct.mireds) : (ct.mireds - target);
    const uint32_t durationMs = distance * 1000u / rate;

    mTransition = CTTransition{
        .startMireds  = ct.mireds,
        .targetMireds = target, // already a legal bound — no extra clamp
        .startTimeMs  = SystemClock().GetMonotonicMilliseconds64().count(),
        .durationMs   = durationMs,
    };
    SetQuietReportRemainingTime(static_cast<uint16_t>(durationMs / 100), /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

/**
 * @brief Executes step color temp command.
 * @return Status::Success when successful,
 *         Status::InvalidCommand when stepSize is 0 or an unknown stepMode is provided
 *         Status::UnsupportedEndpoint when the provided endpoint doesn't correspond with a color temp transition state,
 *         Status::ConstraintError when a command parameter is outside its defined value range.
 */
Status ColorControlCluster::stepColorTemp(StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs,
                                          uint16_t minFieldMireds, uint16_t maxFieldMireds, BitMask<OptionsBitmap> optionsMask,
                                          BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(minFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(maxFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);
    VerifyOrReturnValue(stepMode != StepModeEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(stepSize != 0, Status::InvalidCommand);

    ApplyModeSwitch(EnhancedColorModeEnum::kColorTemperatureMireds);
    auto & ct = std::get<CTColor>(mColorValue);

    uint16_t target;
    if (stepMode == StepModeEnum::kUp)
    { // Up increases mireds → clamp at upper
        uint16_t upper =
            (maxFieldMireds == 0) ? mCT.colorTempPhysicalMaxMireds : std::min(maxFieldMireds, mCT.colorTempPhysicalMaxMireds);
        target = static_cast<uint16_t>(std::min<int32_t>(int32_t(ct.mireds) + stepSize, upper));
    }
    else
    { // Down decreases mireds → clamp at lower
        uint16_t lower =
            (minFieldMireds == 0) ? mCT.colorTempPhysicalMinMireds : std::max(minFieldMireds, mCT.colorTempPhysicalMinMireds);
        target = static_cast<uint16_t>(std::max<int32_t>(int32_t(ct.mireds) - stepSize, lower));
    }

    mTransition = CTTransition{
        .startMireds  = ct.mireds,
        .targetMireds = target,
        .startTimeMs  = SystemClock().GetMonotonicMilliseconds64().count(),
        .durationMs   = transitionTimeDs * 100u,
    };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    ArmTick();
    InvalidateScenes();
    return Status::Success;
}

void ColorControlCluster::CoupleColorTempToLevel(uint8_t currentLevel)
{
    // ZCL 5.2.2.1.1 Coupling color temperature to Level Control
    //
    // If the Level Control for Lighting cluster identifier 0x0008 is supported
    // on the same endpoint as the Color Control cluster and color temperature is
    // supported, it is possible to couple changes in the current level to the
    // color temperature.
    //
    // The CoupleColorTempToLevel bit of the Options attribute of the Level
    // Control cluster indicates whether the color temperature is to be linked
    // with the CurrentLevel attribute in the Level Control cluster.
    //
    // If the CoupleColorTempToLevel bit of the Options attribute of the Level
    // Control cluster is equal to 1 and the ColorMode or EnhancedColorMode
    // attribute is set to 0x02 (color temperature) then a change in the
    // CurrentLevel attribute SHALL affect the ColorTemperatureMireds attribute.
    // This relationship is manufacturer specific, with the qualification that
    // the maximum value of the CurrentLevel attribute SHALL correspond to a
    // ColorTemperatureMired attribute value equal to the
    // CoupleColorTempToLevelMinMireds attribute. This relationship is one-way so
    // a change to the ColorTemperatureMireds attribute SHALL NOT have any effect
    // on the CurrentLevel attribute.
    //
    // In order to simulate the behavior of an incandescent bulb, a low value of
    // the CurrentLevel attribute SHALL be associated with a high value of the
    // ColorTemperatureMireds attribute (i.e., a low value of color temperature
    // in kelvins).
    //
    // If the CoupleColorTempToLevel bit of the Options attribute of the Level
    // Control cluster is equal to 0, there SHALL be no link between color
    // temperature and current level. That option gate is enforced by the Level
    // Control cluster before it notifies us, so here we only honor the coupling
    // when the active mode is color temperature.
    VerifyOrReturn(std::holds_alternative<CTColor>(mColorValue));

    const uint16_t tempCoupleMin = std::max<uint16_t>(1u, mCT.coupleColorTempToLevelMinMireds);
    const uint16_t tempPhysMax   = mCT.colorTempPhysicalMaxMireds;

    // Scale color temp between the coupling min and the physical max. Mireds vary inversely with level
    // (low level -> high mireds): peg the extremes, interpolate the middle (u32 math avoids overflow).
    uint16_t newColorTemp;
    if (currentLevel <= MIN_CURRENT_LEVEL)
    {
        newColorTemp = tempPhysMax;
    }
    else if (currentLevel >= MAX_CURRENT_LEVEL)
    {
        newColorTemp = tempCoupleMin;
    }
    else
    {
        const uint32_t u32TempPhysMax = static_cast<uint32_t>(tempPhysMax);
        const uint32_t tempDelta = ((u32TempPhysMax - tempCoupleMin) * currentLevel) / (MAX_CURRENT_LEVEL - MIN_CURRENT_LEVEL + 1);
        newColorTemp             = static_cast<uint16_t>(tempPhysMax - tempDelta);
    }

    // Instantaneous coupling move (transitionTime 0) reusing the fully validated CT move path
    // (mode switch, physical-range clamp, hardware fan-out via the delegate, scene invalidation).
    moveToColorTemp(newColorTemp, 0);
}
