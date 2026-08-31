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

#pragma once

#include "ColorControlColorState.h"
#include "ColorControlDelegate.h"
#include <app/CommandHandler.h>
#include <app/clusters/scenes-server/SceneHandlerImpl.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <clusters/ColorControl/Attributes.h>
#include <clusters/ColorControl/Commands.h>
#include <clusters/ColorControl/Enums.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>
#include <variant>

namespace chip {
namespace app {
namespace Clusters {

// HueTransition::durationMs sentinel for a MoveHue rate move: no endpoint, runs until a Stop command.
// Above any real duration (kMaxTransitionTime deciseconds * 100), so it can never collide with one.
constexpr uint32_t kIndefiniteHueMoveMs = UINT32_MAX;

// Where one axis sits at a given instant — the result of Interpolate below.
struct Interpolation
{
    int32_t value; // widest common type; each axis narrows it to its own width
    bool done;     // the axis has arrived, so this position is its exact endpoint
};

// Every axis — hue, saturation, mireds, X and Y — travels start -> start + delta over durationMs from its
// own wall-clock anchor, so the position math lives here once rather than in each Tick*. Two durations are
// special:
//   * 0: an immediate move (transitionTime 0) arrives on this tick. Doubles as the divide-by-zero guard.
//   * kIndefiniteHueMoveMs: a MoveHue rate move — delta is units per SECOND and the move never arrives.
// Arrival is decided on the integers and the arriving step lands on exactly start + delta, so accumulated
// float rounding can never leave an axis short of its target.
inline Interpolation Interpolate(int32_t start, int32_t delta, uint64_t startTimeMs, uint32_t durationMs, uint64_t now)
{
    const uint64_t elapsed = now - startTimeMs;

    if (durationMs == kIndefiniteHueMoveMs)
    {
        // INTEGER math, wrapped mod kHueCircle: an endless move would accumulate the float rounding a
        // finite ramp tolerates and would leave int32 range. Only hue moves at a rate, and hue is
        // circular, so wrapping is its natural bound.
        const int64_t moved = static_cast<int64_t>(delta) * static_cast<int64_t>(elapsed) / 1000;
        return { static_cast<int32_t>((static_cast<int64_t>(start) + moved) & 0xFFFF), false };
    }

    if ((durationMs == 0) || (elapsed >= durationMs))
    {
        return { start + delta, true };
    }

    const float t = static_cast<float>(elapsed) / static_cast<float>(durationMs); // in [0, 1)
    return { start + static_cast<int32_t>(static_cast<float>(delta) * t), false };
}

// CIE xy chromaticity coordinates.
struct XYTransition
{
    uint16_t startX;
    uint16_t targetX;
    uint32_t durationXMs;
    uint16_t startY;
    uint16_t targetY;
    uint32_t durationYMs;
    uint64_t startTimeMs;
};

struct HueTransition
{
    uint16_t startHue;
    int32_t signedDelta;
    uint64_t startTimeMs;
    // Milliseconds to travel signedDelta. 0 == immediate (transitionTime 0), kIndefiniteHueMoveMs ==
    // MoveHue rate move (signedDelta is hue-units/second, runs until a Stop command).
    uint32_t durationMs;
};

struct SatTransition
{
    uint8_t startSat;
    uint8_t targetSat;
    uint64_t startTimeMs;
    uint32_t durationMs;
};
struct CTTransition
{
    uint16_t startMireds;
    uint16_t targetMireds;
    uint64_t startTimeMs;
    uint32_t durationMs;
};
struct HueSatTransition
{
    std::optional<HueTransition> hue; // presence == "hue axis in progress"
    std::optional<SatTransition> sat; // independent of hue
};

// The active driver: exactly one axis group at a time (hue+sat is the only legal concurrent pair,
// §3.2.5.2). monostate == stable. Kept in lockstep with ColorValue's mode by ApplyModeSwitch.
using Transition = std::variant<std::monostate, XYTransition, CTTransition, HueSatTransition>;

// ---- CTConfig (runtime-writable CT limits) ----
struct CTConfig
{
    uint16_t colorTempPhysicalMinMireds      = 1;
    uint16_t colorTempPhysicalMaxMireds      = 0xFEFF;
    uint16_t coupleColorTempToLevelMinMireds = 0;
    DataModel::Nullable<uint16_t> startUpColorTemperatureMireds{};
};

// A chromaticity point: X / Y (Fixed, non-null) plus an Intensity. Used for both Primary1..6 and
// ColorPoint R/G/B — Primary_n_Intensity / ColorPoint_c_Intensity carry the nullable (X) quality.
struct ChromaticityPoint
{
    uint16_t x = 0;
    uint16_t y = 0;
    DataModel::Nullable<uint8_t> intensity{};
};

// ---- StaticConfig (app-owned constants the cluster reads but never mutates) ----
// The Fixed descriptors of §3.2.7 (primaries, white point, color points), the read-only drift-compensation
// reports (§3.2.6.4 / §3.2.7.8), and the behavior choices the spec leaves to the manufacturer (which back
// no attribute). Each descriptor is optional: an absent one reads as UnsupportedAttribute and Attributes()
// does not advertise it. Passed as Config::sc; null == no descriptors, every choice at its default.
struct StaticConfig
{
    std::optional<ChromaticityPoint> primaries[6]; // Primary1..6; NumberOfPrimaries() counts the present ones
    std::optional<uint16_t> whitePointX;
    std::optional<uint16_t> whitePointY;
    std::optional<ChromaticityPoint> colorPointR;
    std::optional<ChromaticityPoint> colorPointG;
    std::optional<ChromaticityPoint> colorPointB;

    // Reports of the drift-compensation mechanism the device uses; leaving them absent omits the report,
    // it never disables the underlying mechanism.
    std::optional<ColorControl::DriftCompensationEnum> driftCompensation;
    std::optional<CharSpan> compensationText; // references app-owned storage (outlives the cluster)

    // §3.2.11: while ColorLoopActive == 1, a manufacturer MAY ignore the hue-changing commands
    // (MoveHue/MoveToHue/StepHue/MoveToHueAndSaturation). false (default) honors them. Stop-semantic
    // commands are never affected either way.
    bool ignoreHueCommandsWhileColorLooping = false;
};

// ---- State (always present) ----
// colorMode / enhancedColorMode are NOT here — they are derived from mColorValue.index().
struct State
{
    BitMask<ColorControl::OptionsBitmap> options{};
    uint16_t remainingTime = 0;
};

// Forward-declared for the optional On/Off coupling: only a pointer is held here, so the full On/Off
// header is not needed.
class OnOffCluster;

// ColorControlCluster is its own scene handler (mirrors LevelControlCluster): the owning application
// registers this cluster with the endpoint's scene table (table->RegisterHandler(&cluster)) and must
// call table->UnregisterHandler(&cluster) before destroying it. Shutdown() does not do this, because
// the table stores the raw handler pointer and only the registering application knows its lifetime.
class ColorControlCluster : public DefaultServerCluster, public scenes::DefaultSceneHandlerImpl, public TimerContext
{
public:
    static constexpr uint32_t kTickMs = 100; // transition tick period

    // Values the application answers once, at construction. ColorControlDelegate holds only what the
    // cluster cannot do itself — color-space conversion and hardware output — which depend on the values
    // involved and so must be asked per call. A choice that is fixed for the product belongs here, not
    // behind a virtual the cluster would re-ask on every command.
    struct Config
    {
        Config(ColorControlDelegate & delegate, TimerDelegate & timerDelegate) : mDelegate(delegate), mTimerDelegate(timerDelegate)
        {}

        // Optional On/Off coupling (ShouldExecuteIfOff, §3.2.8.3): inject the endpoint's On/Off cluster so
        // the "execute while off" decision honors its live state; null == no coupling (always execute).
        OnOffCluster * onOff = nullptr;

        ColorControl::ColorValue mColorValue;    // variant: XYColor | HueSatColor | EnhancedHueSatColor | CTColor
        ColorControl::ColorLoopState mColorLoop; // mode-independent
        const StaticConfig * sc = nullptr;       // see StaticConfig; null == no descriptors, defaults everywhere
        CTConfig ctConfig;
        ColorControlDelegate & mDelegate;
        BitMask<ColorControl::Feature> mFeatures{};
        TimerDelegate & mTimerDelegate;
    };

    // Values are copied out of `config`, so it need not outlive this call — but the objects it points at
    // (delegate, timer delegate, On/Off cluster, StaticConfig) are only referenced and must outlive the
    // cluster.
    ColorControlCluster(chip::EndpointId endpoint, const Config & config);
    // Cancels the tick timer. Shutdown() also cancels it, but the timer is registered with `this` as its
    // TimerContext, so a cluster destroyed while a tick is armed (e.g. Shutdown() was never called) would
    // leave the delegate holding a dangling pointer — cancel here too so teardown is guaranteed by RAII.
    ~ColorControlCluster() override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;
    // DefaultServerCluster overrides
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    // ---- Scene handler (scenes::DefaultSceneHandlerImpl) overrides ----
    bool SupportsCluster(EndpointId endpoint, ClusterId clusterId) override;
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId clusterId, MutableByteSpan & serializedBytes) override;
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId clusterId, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override;
    // §3.2.7.1.1: at AddScene, verify the EFS being defined carries the attributes its declared
    // EnhancedColorMode requires — a presence check across the whole EFS, which the per-pair validator
    // cannot do. Then delegates the serialize itself to the base handler.
    CHIP_ERROR SerializeAdd(EndpointId endpoint,
                            const ScenesManagement::Structs::ExtensionFieldSetStruct::DecodableType & extensionFieldSet,
                            MutableByteSpan & serializedBytes) override;

    // Coupling color temperature to Level Control. The application calls this whenever Level Control's
    // CurrentLevel changes and its CoupleColorTempToLevel option is set; `currentLevel` is that live value.
    // The mapping is one-way (level → color temp) and only takes effect while the active mode is color
    // temperature. Having the caller supply the level keeps this cluster free of any Level Control
    // dependency.
    void CoupleColorTempToLevel(uint8_t currentLevel);

    // ---- Live-state accessors (used by the scene handler to serialize a scene) ----
    // Each returns the value of the active color mode when it carries that field, otherwise a neutral
    // default. NO cross-mode conversion is done here (unlike ReadAttribute, which converts for the wire):
    // the scene handler pairs each getter with the matching feature and saves EnhancedColorMode, so a
    // value read out of its mode is never applied on restore.
    uint16_t CurrentX() const;
    uint16_t CurrentY() const;
    // High byte of the enhanced hue, clamped to the CurrentHue constraint max (§3.2.7.12). Read-only:
    // hue changes go through the Move/Step command handlers, never a direct setter.
    uint8_t CurrentHue() const { return ColorControl::Hue8FromEnhancedHue(GetEnhancedHue()); }
    uint16_t EnhancedHue() const { return GetEnhancedHue(); }
    uint8_t Saturation() const { return GetSaturation(); }
    uint16_t ColorTempMireds() const;
    uint8_t ColorLoopActive() const { return static_cast<uint8_t>(mColorLoop.active); }
    uint8_t ColorLoopDirection() const { return static_cast<uint8_t>(mColorLoop.direction); }
    uint16_t ColorLoopTime() const { return mColorLoop.timeSec; }
    ColorControl::EnhancedColorModeEnum GetEnhancedColorMode() const;
    bool SupportsMode(ColorControl::EnhancedColorModeEnum mode) const;

    // ---- Command handlers ----
    // Public (application-facing / unit-testable, like LevelControl's command API); the endpoint comes from
    // mPath, so none of them take one.
    using Status  = chip::Protocols::InteractionModel::Status;
    using OptMask = chip::BitMask<ColorControl::OptionsBitmap>;

    // Conventions shared by every handler below; only departures are noted per command.
    //   * The On/Off + Options gate (§3.2.8.3 / ShouldExecuteIfOff) runs first: a command issued while the
    //     device is off returns Success without acting unless ExecuteIfOff is effective. The mask/override
    //     default to empty for direct callers; InvokeCommand forwards the decoded command fields.
    //   * `isEnhanced` selects a command's Enhanced* twin: hue is the full 16-bit EnhancedCurrentHue rather
    //     than the 8-bit CurrentHue, and the active mode becomes enhanced hue/saturation.
    //   * `transitionTimeDs` is in 1/10 s and is rejected above kMaxTransitionTime with ConstraintError.
    //   * A hue-changing command is dropped (Success, no effect) while a color loop is active if the
    //     application set StaticConfig::ignoreHueCommandsWhileColorLooping (§3.2.11).

    // `rate` is hue units per second and the move has no endpoint — it runs until StopMoveStep or a
    // MoveMode of Stop, which also halts a running saturation axis (§3.2.8.5.4).
    // Returns InvalidCommand when rate is 0 on a non-stop move.
    Status MoveHue(ColorControl::MoveModeEnum moveMode, uint16_t rate, bool isEnhanced, OptMask optionsMask = {},
                   OptMask optionsOverride = {});
    // `dir` picks which way round the circle to travel; an exact half-circle ties to Up (§3.2.8.4.5).
    Status MoveToHue(uint16_t hue, ColorControl::DirectionEnum dir, uint16_t transitionTimeDs, bool isEnhanced,
                     OptMask optionsMask = {}, OptMask optionsOverride = {});
    // Returns InvalidCommand when stepSize is 0.
    Status StepHue(ColorControl::StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs, bool isEnhanced,
                   OptMask optionsMask = {}, OptMask optionsOverride = {});
    // The saturation commands have no Enhanced* twin and preserve whichever hue/sat flavor is active.
    Status MoveToSaturation(uint8_t saturation, uint16_t transitionTimeDs, OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status MoveToHueAndSaturation(uint16_t hue, uint8_t saturation, uint16_t transitionTimeDs, bool isEnhanced,
                                  OptMask optionsMask = {}, OptMask optionsOverride = {});
    // `rate` is saturation units per second; the move ends at the 0 / 0xFE boundary. MoveMode Stop halts the
    // hue axis too (§3.2.8.8.4). Returns InvalidCommand when rate is 0 on a non-stop move.
    Status MoveSaturation(ColorControl::MoveModeEnum moveMode, uint8_t rate, OptMask optionsMask = {},
                          OptMask optionsOverride = {});
    // Returns InvalidCommand when stepSize is 0.
    Status StepSaturation(ColorControl::StepModeEnum stepMode, uint8_t stepSize, uint16_t transitionTimeDs,
                          OptMask optionsMask = {}, OptMask optionsOverride = {});
    // Signed xy-units per second, running toward the CIE boundary until StopMoveStep. A rate of 0 leaves
    // that axis stationary; both rates 0 stops any XY movement.
    Status MoveColor(int16_t rateX, int16_t rateY, OptMask optionsMask = {}, OptMask optionsOverride = {});
    // Returns InvalidCommand when both steps are 0 (§3.2.8.13.4).
    Status StepColor(int16_t stepX, int16_t stepY, uint16_t transitionTimeDs, OptMask optionsMask = {},
                     OptMask optionsOverride = {});
    Status MoveToColor(uint16_t colorX, uint16_t colorY, uint16_t transitionTimeDs, OptMask optionsMask = {},
                       OptMask optionsOverride = {});
    // The target is clamped into the endpoint's physical mired range.
    Status MoveToColorTemp(uint16_t colorTemperature, uint16_t transitionTimeDs, OptMask optionsMask = {},
                           OptMask optionsOverride = {});
    // `rate` is mireds per second, moving to whichever bound the direction implies. A min/max field of 0
    // means "use the physical limit"; otherwise the field is clamped into the physical range (§3.2.8.21.3/.4).
    // Two set fields must not cross (min > max is ConstraintError). Returns InvalidCommand when rate is 0
    // on a non-stop move.
    Status MoveColorTemp(ColorControl::MoveModeEnum moveMode, uint16_t rate, uint16_t minFieldMireds, uint16_t maxFieldMireds,
                         OptMask optionsMask = {}, OptMask optionsOverride = {});
    // Same min/max field handling as MoveColorTemp. Returns InvalidCommand when stepSize is 0.
    Status StepColorTemp(ColorControl::StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs, uint16_t minFieldMireds,
                         uint16_t maxFieldMireds, OptMask optionsMask = {}, OptMask optionsOverride = {});
    // Bits absent from `updateFlags` leave the corresponding loop attribute untouched; the action bit is
    // applied last, after any direction/time/startHue update it accompanies.
    Status ColorLoopSet(chip::BitMask<ColorControl::UpdateFlagsBitmap> updateFlags, ColorControl::ColorLoopActionEnum action,
                        ColorControl::ColorLoopDirectionEnum direction, uint16_t timeSec, uint16_t startHue,
                        chip::BitMask<ColorControl::OptionsBitmap> optionsMask,
                        chip::BitMask<ColorControl::OptionsBitmap> optionsOverride);

    // §3.2.11.20: stops the mTransition-driven axes, leaving an active color loop running (§3.2.8.1).
    // Public so the backward-compat command facade (see CodegenIntegration.h) and unit tests can invoke it.
    Status StopMoveStep(OptMask optionsMask = {}, OptMask optionsOverride = {});

    // Consulted by command handlers to decide whether to run while the device is off (§3.2.8.3). Reads
    // the injected On/Off cluster (null == no coupling → always execute). Public for direct unit testing.
    bool ShouldExecuteIfOff(chip::BitMask<ColorControl::OptionsBitmap> optionMask,
                            chip::BitMask<ColorControl::OptionsBitmap> optionOverride);

private:
    // Arm the one-shot tick timer (no-op if already armed). OnTick re-arms itself while active. Returns
    // the StartTimer error so callers that must not silently drop a transition can propagate it.
    CHIP_ERROR ArmTick();
    // Advance every active axis to `now`, then re-arm while anything is still moving.
    void OnTick();
    // TimerContext: the cluster is its own timer context, so the delegate calls straight back here.
    void TimerFired() override { OnTick(); }
    // Wall clock for every transition: read through the timer delegate, never from the system clock
    // directly, so a test driving the delegate's fake clock moves time for the whole cluster.
    uint64_t NowMs() const { return mTimerDelegate.GetCurrentMonotonicTimestamp().count(); }

    // Clamps the scene's values, switches mode, (re)starts the color loop and starts the transition over the
    // scene's transition time. Reached only through ApplyScene, which decodes the extension field set first.
    CHIP_ERROR HandleApplyScene(ColorControl::EnhancedColorModeEnum ColorMode, const ColorControl::ColorValue & target,
                                const ColorControl::ColorLoopState & loop, uint32_t timeMs);
    // Counts the app-provided fixed primaries; null when no StaticConfig is given. Derived on read like
    // every other StaticConfig-backed attribute, so it is never stored in mState.
    DataModel::Nullable<uint8_t> NumberOfPrimaries() const;
    // Tick*: advance one axis to where `now` puts it, store it, fan it out, and return whether the axis is
    // still moving. `now` comes from OnTick's single clock read.
    bool TickHue(HueTransition & tx, uint64_t now);
    bool TickXY(XYTransition & xyx, uint64_t now);
    bool TickSat(SatTransition & sx, uint64_t now);
    bool TickColorLoop(uint64_t now);

    // Runtime (NON-persistent) wall-clock anchor for the color loop. Stamped once when the loop starts
    // and never re-stamped on dormancy, so the loop's phase advances with real time even while XY/CT owns
    // the output. LoopIsDriving() decides whether TickColorLoop reads these each tick.
    uint16_t mColorLoopStartHue    = 0; // EnhancedCurrentHue at the moment the loop started
    uint64_t mColorLoopStartTimeMs = 0; // NowMs() reading at the moment the loop started
    // Runtime (NON-persistent) "green light" for the loop, distinct from the ColorLoopActive attribute.
    // Latched off in OnTick once a command's hue transition owns the hue axis: the loop then stays dormant
    // (ColorLoopActive stays 1) and does NOT resume when that transition ends — only ColorLoopSet
    // re-engages it. This is the one bit (active, mTransition, mode) cannot carry: a loop running normally
    // and a loop parked after a finite move both sit at active==1 / monostate / enhanced-HS.
    // Defaults true so a loop active at construction / reboot drives; dormancy is transient, not persisted.
    bool mColorLoopEngaged = true;

    // ---- Cluster state (initialized from Config in the constructor) ----
    ColorControlDelegate & mDelegate;
    // Owns the tick AND the wall clock (see NowMs): the cluster never touches the system layer or the
    // system clock itself, so tests inject a fake one.
    TimerDelegate & mTimerDelegate;
    BitMask<ColorControl::Feature> mFeatures{};   // advertised features
    ColorControl::ColorValue mColorValue;         // active color; its alternative encodes the mode
    Transition mTransition;                       // active driver (monostate == stable)
    ColorControl::ColorLoopState mColorLoop;      // mode-independent autonomous hue driver
    State mState;                                 // Options + RemainingTime
    CTConfig mCT;                                 // color-temperature limits + startup
    const StaticConfig * mStaticConfig = nullptr; // app-owned fixed descriptors; null == none supported
    OnOffCluster * mOnOff              = nullptr; // injected On/Off cluster for ShouldExecuteIfOff; null == no coupling

    bool HasFeature(ColorControl::Feature feature) const { return mFeatures.Has(feature); }
    bool LoopIsDriving() const;
    // §3.2.11: true when a hue-changing command should be dropped because a color loop is active and the
    // manufacturer opted to ignore such commands. Saturation-only and stop-semantic commands never consult it.
    bool ShouldIgnoreHueCommandNow() const;

    // Transition builders, shared by the commands and scene restore. They populate mTransition and
    // RemainingTime only — no mode switch (the caller runs ApplyModeSwitch first, so mColorValue already
    // holds the matching alternative and its current value is the start of the transition) and no tick
    // arm, which is the one fallible step and stays with the caller so a failed arm can be surfaced.
    void StartXYTransition(const ColorControl::ColorValue & target, uint32_t timeMs);
    void StartColorTemperatureTransition(const ColorControl::ColorValue & target, uint32_t timeMs);
    // `moveHue` gates only the hue axis (saturation always moves): a command passes the manufacturer's
    // ignore-while-looping choice; the scene-restore path passes true so it always restores both axes.
    void StartHueAndSatTransition(const ColorControl::ColorValue & target, uint32_t timeMs, bool moveHue);
    void StartEnhancedHueAndSatTransition(const ColorControl::ColorValue & target, uint32_t timeMs, bool moveHue);
    // Switch the active mode to `target`, converting the current color into the new representation through
    // the delegate. No-op when that alternative is already active.
    void ApplyModeSwitch(ColorControl::EnhancedColorModeEnum target);

    // ---- Color-loop lifecycle ----
    // `startFromStartHue` starts the loop at ColorLoopStartEnhancedHue instead of the current
    // EnhancedCurrentHue; StopColorLoop restores the hue stored when the loop began.
    void StartColorLoop(bool startFromStartHue);
    void StopColorLoop();

    // ---- Transition start notifications ----
    // Announce a newly installed movement to the delegate, so hardware with its own fade engine can run
    // the ramp instead of following the ticks (see ColorControlDelegate). Called immediately after the axis
    // is written into mTransition, by EVERY origin — commands, scene restore, StartUpColorTemperature and
    // CoupleColorTempToLevel — since the hardware cares that a movement started, not what caused it.
    //
    // A zero-duration axis is deliberately NOT announced: §3.2.8 makes a transition continuous and lasting
    // TransitionTime, so TransitionTime == 0 is a discrete jump, which reaches the delegate as the single
    // settling On*Changed (transitionActive == false). The test lives here so no origin can get it wrong.
    void NotifyXYTransition(const XYTransition & tx);
    void NotifyCTTransition(const CTTransition & tx);
    void NotifyHueTransition(const HueTransition & tx);
    void NotifySatTransition(const SatTransition & tx);

    // ---- Per-tick / mode / reporting helpers ----
    bool TickCT(CTTransition & tx, uint64_t now);
    void NotifyModeAttributes();
    void ApplyStartUpColorTemperature();
    void SetQuietReportRemainingTime(uint16_t newRemainingTime, bool isNewTransition = false);

    // ---- Current-color accessors + scene/level helpers ----
    HueSatTransition & EnsureHueSatTransition();
    uint8_t GetSaturation() const;
    uint16_t GetEnhancedHue() const;

    // ---- Persistence (mirror of the NVM-flagged attributes restored in Startup) ----
    // Writes are settle-triggered (transition end / discrete change), never per-tick — RemainingTime and
    // the in-flight transition are RAM-only, so persisting mid-transition would only wear flash.
    template <typename T>
    void PersistValue(chip::AttributeId id, const T & value);
    void PersistCurrentColor();     // EnhancedColorMode + the active mode's stored axes
    void PersistColorLoop();        // ColorLoopActive / ColorLoopDirection / ColorLoopTime
    bool StopTransitionAndFreeze(); // true when a driver was actually stopped (i.e. the output moved)

    static constexpr uint16_t kMaxTransitionTime         = 0xFFFE; // Max value as defined by the spec.
    static constexpr uint16_t kMaxColorTemperatureMireds = 0xFEFF; // Max value as defined by the spec.
    // 10 distinct attribute IDs are scenable; CurrentHue and EnhancedCurrentHue are mutually exclusive,
    // so at most 9 are saved for any one device.
    static constexpr uint8_t kColorControlScenableAttributesCount = 10;
};
} // namespace Clusters
} // namespace app
} // namespace chip
