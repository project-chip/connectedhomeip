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
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <clusters/ColorControl/Attributes.h>
#include <clusters/ColorControl/Commands.h>
#include <clusters/ColorControl/Enums.h>
#include <lib/support/BitFlags.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>
#include <variant>

namespace chip {
namespace app {
namespace Clusters {

// CIE xy chromaticity coordinates.
struct XYTransition
{
    uint16_t startX, targetX;
    uint32_t durationXMs;
    uint16_t startY, targetY;
    uint32_t durationYMs;
    uint64_t startTimeMs;
};

struct HueTransition
{
    uint16_t startHue;
    int32_t signedDelta;
    uint64_t startTimeMs;
    uint32_t durationMs;
};

struct SatTransition
{
    uint8_t startSat, targetSat;
    uint64_t startTimeMs;
    uint32_t durationMs;
};
struct CTTransition
{
    uint16_t startMireds, targetMireds;
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
    uint16_t colorTempPhysicalMinMireds      = 0;
    uint16_t colorTempPhysicalMaxMireds      = 0xFEFF;
    uint16_t coupleColorTempToLevelMinMireds = 0;
    DataModel::Nullable<uint16_t> startUpColorTemperatureMireds{};
};

// A chromaticity point: X / Y (Fixed, non-null) plus an optional-on-the-wire Intensity. Used for both
// Primary1..6 and ColorPoint R/G/B — Primary_n_Intensity / ColorPoint_c_Intensity carry the nullable (X)
// quality, hence Nullable.
struct ChromaticityPoint
{
    uint16_t x = 0;
    uint16_t y = 0;
    DataModel::Nullable<uint8_t> intensity{};
};

// ---- StaticConfig (§3.2.7 Fixed descriptors: primaries / white point / color points) ----
// Every field here is a Fixed (F) + read-only attribute, and each is OPTIONAL (conformance O; primaries
// additionally gated on NumberOfPrimaries). Presence is modelled with std::optional so the application
// advertises exactly the descriptors it has — an absent optional → UnsupportedAttribute on read. These
// are device constants owned by the app and handed in by const pointer (Config::sc); never in mState,
// never persisted, never reported.
struct StaticConfig
{
    std::optional<ChromaticityPoint> primaries[6]; // Primary1..6; present ones up to NumberOfPrimaries
    std::optional<uint16_t> whitePointX;
    std::optional<uint16_t> whitePointY;
    std::optional<ChromaticityPoint> colorPointR;
    std::optional<ChromaticityPoint> colorPointG;
    std::optional<ChromaticityPoint> colorPointB;
};

// ---- State (always present) ----
// colorMode / enhancedColorMode are NOT here — they are derived from mColorValue.index().
struct State
{
    BitMask<ColorControl::OptionsBitmap> options{};
    BitMask<ColorControl::ColorCapabilitiesBitmap> colorCapabilities{};
    DataModel::Nullable<uint8_t> numberOfPrimaries{};
    uint16_t remainingTime = 0;
};

// Forward-declared for the optional On/Off coupling: the cluster only holds a pointer and calls the
// (inline) GetOnOff() from the .cpp, so the full On/Off header is not needed here.
class OnOffCluster;

// Forward-declared for the optional Scene Management coupling (injected, ember-free). See
// ColorControlSceneInvalidator.h.
class ColorControlSceneInvalidator;

class ColorControlCluster : public DefaultServerCluster
{
public:
    static constexpr uint32_t kTickMs = 100; // the tick fires every 100 ms

    struct Config
    {
        explicit Config(ColorControlDelegate & delegate) : mDelegate(delegate) {}

        // Optional On/Off coupling (ShouldExecuteIfOff, §3.2.8.3): inject the endpoint's On/Off cluster so
        // the "execute while off" decision honors its live state; null == no coupling (always execute).
        // Ember-free: the cluster is read directly, no registry / Instance().
        OnOffCluster * onOff = nullptr;

        // Optional Scene Management coupling: injected so a color change can mark stored scenes stale
        // without the core depending on the Scenes cluster / ScenesServer. Null == no scene coupling.
        ColorControlSceneInvalidator * sceneInvalidator = nullptr;

        ColorControl::ColorValue mColorValue;    // variant: XYColor | HueSatColor | EnhancedHueSatColor | CTColor
        ColorControl::ColorLoopState mColorLoop; // mode-independent; see below
        const StaticConfig * sc = nullptr;       // app-owned fixed descriptors; null == no primaries/points
        CTConfig ctConfig;
        ColorControlDelegate & mDelegate;
        BitMask<ColorControl::Feature> mFeatures{};
    };

    // Per MIGRATION.md: endpoint is explicit and forwarded to the base ConcreteClusterPath (which is where
    // mPath — and thus mPath.mEndpointId — lives). Members are copied out of config at construction.
    ColorControlCluster(chip::EndpointId endpoint, const Config & config);
    ~ColorControlCluster() override = default;

    // Arm the one-shot 100ms tick timer (no-op if already armed). OnTick re-arms itself while active.
    void ArmTick();
    // ── t=100ms, 200ms, … · SystemLayer fires OnTick every 100ms ────────────────
    void OnTick();
    // TickHue: t from wall clock, exact target on the last tick, then store + fan-out.
    bool TickHue(HueTransition & tx, uint64_t now);

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

    CHIP_ERROR HandleApplyScene(ColorControl::EnhancedColorModeEnum ColorMode, const ColorControl::ColorValue & target,
                                const ColorControl::ColorLoopState & loop, uint16_t timeMs);
    bool HasFeature(ColorControl::Feature feature) const { return mFeatures.Has(feature); }

    // §3.2.8.x Coupling color temperature to Level Control. Called (via the Level Control coupling glue)
    // whenever the Level Control cluster's CurrentLevel changes and its CoupleColorTempToLevel option is
    // set. `currentLevel` is Level Control's live value; the mapping is one-way (level → color temp) and
    // only takes effect while the active mode is color temperature. No cross-cluster coupling lives in the
    // core: the caller supplies the level, keeping this cluster free of any Level Control dependency.
    void CoupleColorTempToLevel(uint8_t currentLevel);

    // ---- Live-state accessors (used by the scene handler to serialize a scene) ----
    // Each returns the value of the active color mode when it carries that field, otherwise a neutral
    // default. NO cross-mode conversion is done here (unlike ReadAttribute, which converts for the wire):
    // the scene handler pairs each getter with the matching feature and saves EnhancedColorMode, so a
    // value read out of its mode is never applied on restore.
    uint16_t CurrentX() const;
    uint16_t CurrentY() const;
    // 8-bit CurrentHue: the high byte of the 16-bit enhanced hue (§3.2.7.12). Read-only; there is no
    // direct hue setter — hue changes go through the Move/Step command handlers (transition-only contract).
    uint8_t CurrentHue() const { return static_cast<uint8_t>(GetEnhancedHue() >> 8); }
    uint16_t EnhancedHue() const { return GetEnhancedHue(); }
    uint8_t Saturation() const { return GetSaturation(); }
    uint16_t ColorTempMireds() const;
    uint8_t ColorLoopActive() const { return static_cast<uint8_t>(mColorLoop.active); }
    uint8_t ColorLoopDirection() const { return static_cast<uint8_t>(mColorLoop.direction); }
    uint16_t ColorLoopTime() const { return mColorLoop.time; }
    ColorControl::EnhancedColorModeEnum GetEnhancedColorMode() const;
    bool SupportsMode(ColorControl::EnhancedColorModeEnum mode) const;

    // ---- Command handlers ----
    // Public (application-facing / unit-testable, like LevelControl's command API). The spec does not
    // require these to be private; exposing them lets callers and tests drive commands directly.
    // (decoupled: no EndpointId/DecodableType; mPath carries the endpoint).
    //      Signatures match the definitions in the .cpp exactly. ----
    using Status = chip::Protocols::InteractionModel::Status;

    // Every command handler honors the On/Off + Options gate (§3.2.7.4 / ShouldExecuteIfOff) as its
    // first action, so a command issued while the device is off is suppressed unless ExecuteIfOff is
    // effective. optionsMask/optionsOverride default to empty so direct (test) callers that don't care
    // about the gate can omit them; InvokeCommand always forwards the decoded command fields.
    using OptMask = chip::BitMask<ColorControl::OptionsBitmap>;

    Status moveHue(ColorControl::MoveModeEnum moveMode, uint16_t rate, bool isEnhanced, OptMask optionsMask = {},
                   OptMask optionsOverride = {});
    Status moveToHue(uint16_t hue, ColorControl::DirectionEnum dir, uint16_t transitionTimeDs, bool isEnhanced,
                     OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status stepHue(ColorControl::StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs, bool isEnhanced,
                   OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status moveToSaturation(uint8_t saturation, uint16_t transitionTimeDs, OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status moveToHueAndSaturation(uint16_t hue, uint8_t saturation, uint16_t transitionTimeDs, bool isEnhanced,
                                  OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status moveSaturation(ColorControl::MoveModeEnum moveMode, uint8_t rate, OptMask optionsMask = {},
                          OptMask optionsOverride = {});
    Status stepSaturation(ColorControl::StepModeEnum stepMode, uint8_t stepSize, uint16_t transitionTimeDs,
                          OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status moveColor(int16_t rateX, int16_t rateY, OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status stepColor(int16_t stepX, int16_t stepY, uint16_t transitionTimeDs, OptMask optionsMask = {},
                     OptMask optionsOverride = {});
    Status moveToColor(uint16_t colorX, uint16_t colorY, uint16_t transitionTimeDs, OptMask optionsMask = {},
                       OptMask optionsOverride = {});
    Status moveToColorTemp(uint16_t colorTemperature, uint16_t transitionTimeDs, OptMask optionsMask = {},
                           OptMask optionsOverride = {});
    Status moveColorTemp(ColorControl::MoveModeEnum moveMode, uint16_t rate, uint16_t minFieldMireds, uint16_t maxFieldMireds,
                         OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status stepColorTemp(ColorControl::StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs, uint16_t minFieldMireds,
                         uint16_t maxFieldMireds, OptMask optionsMask = {}, OptMask optionsOverride = {});
    Status ColorLoopSet(chip::BitMask<ColorControl::UpdateFlagsBitmap> updateFlags, ColorControl::ColorLoopActionEnum action,
                        ColorControl::ColorLoopDirectionEnum direction, uint16_t time, uint16_t startHue,
                        chip::BitMask<ColorControl::OptionsBitmap> optionsMask,
                        chip::BitMask<ColorControl::OptionsBitmap> optionsOverride);

    // StopMoveStep command (§3.2.11.20): stops the mTransition-driven axes. Public so the backward-compat
    // command facade (see CodegenIntegration.h) and unit tests can invoke it directly. Has no effect while a
    // color loop is active (§3.2.8.1). InvokeCommand routes the wire command through HandleStopMoveStep,
    // which forwards here.
    Status stopMoveStep(OptMask optionsMask = {}, OptMask optionsOverride = {});

    // Consulted by command handlers to decide whether to run while the device is off (§3.2.8.3). Reads
    // the injected On/Off cluster (null == no coupling → always execute). Public for direct unit testing.
    bool ShouldExecuteIfOff(chip::BitMask<ColorControl::OptionsBitmap> optionMask,
                            chip::BitMask<ColorControl::OptionsBitmap> optionOverride);

private:
    static void TimerCallback(System::Layer *, void * ctx) { static_cast<ColorControlCluster *>(ctx)->OnTick(); }

    std::optional<DataModel::ActionReturnStatus> HandleStopMoveStep(const ColorControl::Commands::StopMoveStep::DecodableType & req,
                                                                    CommandHandler * handler);
    bool TickXY(XYTransition & xyx, uint64_t now);
    bool TickSat(SatTransition & sx, uint64_t now);
    bool TickColorLoop(uint64_t now);

    // Runtime (NON-persistent) wall-clock anchor for the color loop. Stamped once when the loop starts
    // driving and never re-stamped on dormancy, so the loop's phase advances with real time even while
    // XY/CT owns the output. LoopIsDriving() decides whether TickColorLoop reads these each tick.
    uint16_t mColorLoopStartHue    = 0; // EnhancedCurrentHue at the moment the loop started
    uint64_t mColorLoopStartTimeMs = 0; // SystemClock() reading at the moment the loop started

    // ---- Cluster state (initialized from Config in the constructor) ----
    ColorControlDelegate & mDelegate;             // reference → must be set in the ctor init list
    BitMask<ColorControl::Feature> mFeatures{};   // advertised features
    ColorControl::ColorValue mColorValue;         // active color; its alternative encodes the mode
    Transition mTransition;                       // active driver (monostate == stable)
    ColorControl::ColorLoopState mColorLoop;      // mode-independent autonomous hue driver
    State mState;                                 // options, capabilities, numberOfPrimaries, remainingTime
    CTConfig mCT;                                 // color-temperature limits + startup
    const StaticConfig * mStaticConfig = nullptr; // app-owned fixed descriptors; null == none supported
    OnOffCluster * mOnOff              = nullptr; // injected On/Off cluster for ShouldExecuteIfOff; null == no coupling
    ColorControlSceneInvalidator * mSceneInvalidator = nullptr; // injected scene-invalidation hook; null == no coupling

    bool LoopIsDriving() const;

    // Scene-restore transition helpers: populate mTransition + arm the tick from a saved ColorValue.
    // They do NOT switch mode — the caller runs ApplyModeSwitch first so mColorValue already matches.
    CHIP_ERROR StartXYTransition(const ColorControl::ColorValue & target, uint16_t timeMs);
    CHIP_ERROR StartColorTemperatureTransition(const ColorControl::ColorValue & target, uint16_t timeMs);
    CHIP_ERROR StartHueAndSatTransition(const ColorControl::ColorValue & target, uint16_t timeMs);
    CHIP_ERROR StartEnhancedHueAndSatTransition(const ColorControl::ColorValue & target, uint16_t timeMs);
    void ApplyModeSwitch(ColorControl::EnhancedColorModeEnum target);

    // ---- Color-loop lifecycle ----
    void startColorLoop(bool fromStartHue);
    void stopColorLoop();

    // ---- Per-tick / mode / reporting helpers ----
    bool TickCT(CTTransition & tx, uint64_t now);
    void NotifyModeAttributes();
    void ApplyStartUpColorTemperature();
    void SetQuietReportRemainingTime(uint16_t newRemainingTime, bool isNewTransition = false);

    // ---- Current-color accessors + scene/level helpers ----
    HueSatTransition & EnsureHueSatTransition();
    uint8_t GetSaturation() const;
    uint16_t GetEnhancedHue() const;
    void InvalidateScenes();

    // ---- Persistence (mirror of the NVM-flagged attributes restored in Startup) ----
    // Startup restores these; something has to write them back or a reboot always sees stale defaults.
    // Writes are settle-triggered (transition end / discrete change), never per-tick — RemainingTime and
    // the in-flight transition are RAM-only, so persisting mid-transition would only wear flash.
    template <typename T>
    void PersistValue(chip::AttributeId id, const T & value);
    void PersistCurrentColor(); // EnhancedColorMode + the active mode's stored axes
    void PersistColorLoop();    // ColorLoopActive / ColorLoopDirection / ColorLoopTime

    static constexpr uint16_t kMaxTransitionTime         = 65534; // Max value as defined by the spec.
    static constexpr uint16_t kMaxColorTemperatureMireds = 65279; // Max value as defined by the spec (0xFEFF).
};
} // namespace Clusters
} // namespace app
} // namespace chip
