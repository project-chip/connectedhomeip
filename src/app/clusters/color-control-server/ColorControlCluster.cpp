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
#include <algorithm>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/scenes-server/AttributeValuePairValidator.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ColorControl/Metadata.h>
#include <cstdlib>
#include <lib/support/TypeTraits.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::ColorControl;
using namespace chip::app::Clusters::ColorControl::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {
static constexpr uint16_t kMinCieXyValue = 0x0000;
static constexpr uint16_t kMaxCieXyValue = 0xFEFF; // Max CurrentX/CurrentY value as defined by the spec.

static constexpr uint8_t kMinSaturationValue = 0x00;
static constexpr uint8_t kMaxSaturationValue = 0xFE;

static constexpr uint16_t kMaxInt16uValue = 0xFFFF; // RemainingTime sentinel for indefinite moves

// Largest reportable RemainingTime (§3.2.7.2 constraint max 0xFFFE); 0xFFFF stays reserved for the
// indefinite-move sentinel above. Mirrors ColorControlCluster::kMaxTransitionTime, which is private.
static constexpr uint16_t kMaxRemainingTenths = 0xFFFE;

// ms → RemainingTime deciseconds, saturating at the attribute's constraint max. Durations are not bounded
// by kMaxTransitionTime: a rate move covers its whole axis at the commanded rate (up to 0xFEFF units at
// 1 unit/s ≈ 18 h) and a scene's transition time reaches 60000000 ms — either overflows uint16 once
// divided by 100.
constexpr uint16_t RemainingTenthsFromMs(uint32_t durationMs)
{
    return static_cast<uint16_t>(std::min<uint32_t>(durationMs / 100, kMaxRemainingTenths));
}

// Remaining time on one axis, in 1/10 s, from its wall-clock anchor. durationMs == kIndefiniteHueMoveMs
// is the MoveHue rate move, which has no end, so RemainingTime is kMaxInt16uValue until a Stop clears the
// axis (§3.2.7.4). durationMs == 0 is an immediate (transitionTime 0) move → 0 remaining.
inline uint16_t RemainingTenths(uint64_t startTimeMs, uint32_t durationMs, uint64_t now)
{
    if (durationMs == kIndefiniteHueMoveMs)
    {
        return kMaxInt16uValue;
    }
    const uint64_t endMs = startTimeMs + durationMs;
    if (now >= endMs)
    {
        return 0;
    }
    return RemainingTenthsFromMs(static_cast<uint32_t>(endMs - now));
}
static constexpr uint8_t kMinCurrentLevel = 0x01;
static constexpr uint8_t kMaxCurrentLevel = 0xFE;

// Validates the attribute/value pairs a scene stores for ColorControl. Handed to the
// DefaultSceneHandlerImpl base from the constructor.
class ColorControlValidator : public scenes::AttributeValuePairValidator
{
public:
    CHIP_ERROR Validate(const app::ConcreteClusterPath & clusterPath,
                        AttributeValuePairValidator::AttributeValuePairType & value) override
    {
        VerifyOrReturnError(clusterPath.mClusterId == ColorControl::Id, CHIP_ERROR_INVALID_ARGUMENT);
        // Per-pair only: correct value type present and within the attribute's range. The cross-pair
        // mode<->attribute presence check lives in ColorControlCluster::SerializeAdd, which sees the
        // whole EFS.
        switch (value.attributeID)
        {
        case CurrentX::Id:
        case CurrentY::Id:
            VerifyOrReturnError(value.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(value.valueUnsigned16.Value() <= kMaxCieXyValue, CHIP_ERROR_INVALID_ARGUMENT);
            return CHIP_NO_ERROR;
        case CurrentSaturation::Id:
            VerifyOrReturnError(value.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(value.valueUnsigned8.Value() <= kMaxSaturationValue, CHIP_ERROR_INVALID_ARGUMENT);
            return CHIP_NO_ERROR;
        case ColorTemperatureMireds::Id:
            VerifyOrReturnError(value.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(value.valueUnsigned16.Value() <= 0xFEFF, CHIP_ERROR_INVALID_ARGUMENT); // spec max mireds
            return CHIP_NO_ERROR;
        case Attributes::EnhancedColorMode::Id:
            VerifyOrReturnError(value.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(value.valueUnsigned8.Value() <=
                                    to_underlying(EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation),
                                CHIP_ERROR_INVALID_ARGUMENT);
            return CHIP_NO_ERROR;
        case EnhancedCurrentHue::Id:
        case ColorLoopTime::Id:
            VerifyOrReturnError(value.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT); // full uint16 range
            return CHIP_NO_ERROR;
        case CurrentHue::Id:
        case ColorLoopActive::Id:
            VerifyOrReturnError(value.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT); // full uint8 range
            return CHIP_NO_ERROR;
        case Attributes::ColorLoopDirection::Id:
            VerifyOrReturnError(value.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(value.valueUnsigned8.Value() <= to_underlying(ColorLoopDirectionEnum::kIncrement),
                                CHIP_ERROR_INVALID_ARGUMENT);
            return CHIP_NO_ERROR;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
};

scenes::AttributeValuePairValidator & GlobalColorControlValidator()
{
    static ColorControlValidator sValidator;
    return sValidator;
}

using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

/// Append one attribute/value pair to the scene array and advance the count. The overload set is limited
/// to the two unsigned widths ColorControl scenes use, so any other type is an ambiguous call rather than
/// a silent conversion.
void AddAttributeValuePair(AttributeValuePair * pairs, AttributeId id, uint8_t value, size_t & attributeCount)
{
    pairs[attributeCount].attributeID = id;
    pairs[attributeCount].valueUnsigned8.SetValue(value);
    attributeCount++;
}

void AddAttributeValuePair(AttributeValuePair * pairs, AttributeId id, uint16_t value, size_t & attributeCount)
{
    pairs[attributeCount].attributeID = id;
    pairs[attributeCount].valueUnsigned16.SetValue(value);
    attributeCount++;
}

} // namespace

ColorControlCluster::ColorControlCluster(EndpointId endpoint, const Config & config) :
    DefaultServerCluster({ endpoint, ColorControl::Id }), scenes::DefaultSceneHandlerImpl(GlobalColorControlValidator()),
    mDelegate(config.mDelegate), mTimerDelegate(config.mTimerDelegate), mFeatures(config.mFeatures),
    mColorValue(config.mColorValue), mColorLoop(config.mColorLoop), mCT(config.ctConfig), mStaticConfig(config.sc),
    mOnOff(config.onOff)
{}

Nullable<uint8_t> ColorControlCluster::NumberOfPrimaries() const
{
    VerifyOrReturnValue(mStaticConfig != nullptr, Nullable<uint8_t>());

    uint8_t count = 0;
    for (const auto & primary : mStaticConfig->primaries)
    {
        if (primary.has_value())
        {
            count++;
        }
    }
    return Nullable<uint8_t>(count);
}

ColorControlCluster::~ColorControlCluster()
{
    // Cancel the tick timer so the delegate never calls TimerFired() on a context that is gone. Shutdown()
    // also cancels it but is not always called (e.g. stack-allocated clusters in unit tests); CancelTimer
    // is a no-op when nothing is armed.
    mTimerDelegate.CancelTimer(this);
}

// ---- Scene handler (scenes::DefaultSceneHandlerImpl) ----

bool ColorControlCluster::SupportsCluster(EndpointId endpoint, ClusterId clusterId)
{
    return clusterId == ColorControl::Id && endpoint == mPath.mEndpointId;
}

CHIP_ERROR ColorControlCluster::SerializeSave(EndpointId endpoint, ClusterId clusterId, MutableByteSpan & serializedBytes)
{
    VerifyOrReturnError(SupportsCluster(endpoint, clusterId), CHIP_ERROR_INVALID_ARGUMENT);

    AttributeValuePair pairs[kColorControlScenableAttributesCount];
    size_t attributeCount = 0;

    // Values are read from this cluster's live state (the source of truth); no clamping and no mutation
    // (SerializeSave must never change the live color).
    if (HasFeature(Feature::kXy))
    {
        AddAttributeValuePair(pairs, Attributes::CurrentX::Id, CurrentX(), attributeCount);
        AddAttributeValuePair(pairs, Attributes::CurrentY::Id, CurrentY(), attributeCount);
    }
    if (HasFeature(Feature::kHueAndSaturation))
    {
        // The hue attribute follows the ACTIVE MODE, not the feature: a scene never carries a hue
        // representation its declared EnhancedColorMode cannot express, so ApplyScene never has to guess
        // which of the two was saved.
        if (GetEnhancedColorMode() == EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation)
        {
            AddAttributeValuePair(pairs, Attributes::EnhancedCurrentHue::Id, EnhancedHue(), attributeCount);
        }
        else
        {
            AddAttributeValuePair(pairs, Attributes::CurrentHue::Id, CurrentHue(), attributeCount);
        }
        AddAttributeValuePair(pairs, Attributes::CurrentSaturation::Id, Saturation(), attributeCount);
    }
    if (HasFeature(Feature::kColorLoop))
    {
        AddAttributeValuePair(pairs, Attributes::ColorLoopActive::Id, ColorLoopActive(), attributeCount);
        AddAttributeValuePair(pairs, Attributes::ColorLoopDirection::Id, ColorLoopDirection(), attributeCount);
        AddAttributeValuePair(pairs, Attributes::ColorLoopTime::Id, ColorLoopTime(), attributeCount);
    }
    if (HasFeature(Feature::kColorTemperature))
    {
        AddAttributeValuePair(pairs, Attributes::ColorTemperatureMireds::Id, ColorTempMireds(), attributeCount);
    }

    // EnhancedColorMode is mandatory and disambiguates which mode the scene captured (§3.2.7.1).
    AddAttributeValuePair(pairs, Attributes::EnhancedColorMode::Id, static_cast<uint8_t>(to_underlying(GetEnhancedColorMode())),
                          attributeCount);

    DataModel::List<AttributeValuePair> attributeValueList(pairs, attributeCount);
    return EncodeAttributeValueList(attributeValueList, serializedBytes);
}

CHIP_ERROR ColorControlCluster::ApplyScene(EndpointId endpoint, ClusterId clusterId, const ByteSpan & serializedBytes,
                                           scenes::TransitionTimeMs timeMs)
{
    VerifyOrReturnError(SupportsCluster(endpoint, clusterId), CHIP_ERROR_INVALID_ARGUMENT);

    DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;
    ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

    // Decode into flat locals and build the ColorValue ONCE, after the loop: the EFS carries values for
    // several modes in arbitrary order and a std::variant holds only one alternative, so assigning
    // per-case would drop fields (X then Y) and could build the wrong alternative before EnhancedColorMode
    // is even seen. No range clamping: AddScene already rejected out-of-range pairs through
    // ColorControlValidator. The one exception is ColorTemperatureMireds, whose bounds are per-device and
    // so are applied in StartColorTemperatureTransition rather than by the add-time validator.
    auto targetColorMode = EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation;
    uint16_t x           = 0;
    uint16_t y           = 0;
    uint16_t enhancedHue = 0;
    uint16_t mireds      = 0;
    uint8_t hue          = 0;
    uint8_t saturation   = 0;
    ColorLoopState loop;

    auto it = attributeValueList.begin();
    while (it.Next())
    {
        auto & p = it.GetValue();
        switch (p.attributeID)
        {
        case Attributes::CurrentX::Id:
            VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            x = p.valueUnsigned16.Value();
            break;
        case Attributes::CurrentY::Id:
            VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            y = p.valueUnsigned16.Value();
            break;
        case Attributes::EnhancedCurrentHue::Id:
            VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            enhancedHue = p.valueUnsigned16.Value();
            break;
        case Attributes::CurrentHue::Id:
            VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            hue = p.valueUnsigned8.Value();
            break;
        case Attributes::CurrentSaturation::Id:
            VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            saturation = p.valueUnsigned8.Value();
            break;
        case Attributes::ColorTemperatureMireds::Id:
            VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            mireds = p.valueUnsigned16.Value();
            break;
        case Attributes::ColorLoopActive::Id:
            VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            loop.active = p.valueUnsigned8.Value();
            break;
        case Attributes::ColorLoopDirection::Id:
            VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            loop.direction = p.valueUnsigned8.Value();
            break;
        case Attributes::ColorLoopTime::Id:
            VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            loop.timeSec = p.valueUnsigned16.Value();
            break;
        case Attributes::EnhancedColorMode::Id:
            VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(p.valueUnsigned8.Value() <=
                                    to_underlying(EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation),
                                CHIP_ERROR_INVALID_ARGUMENT);
            targetColorMode = static_cast<EnhancedColorModeEnum>(p.valueUnsigned8.Value());
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    // Build the single alternative matching the scene's declared mode. The color loop can be active in any
    // mode, so it is initialized separately.
    ColorValue target;
    switch (targetColorMode)
    {
    case EnhancedColorModeEnum::kCurrentXAndCurrentY:
        target = XYColor{ .x = x, .y = y };
        break;
    case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation:
        target = EnhancedHueSatColor{ .enhancedHue = enhancedHue, .saturation = saturation };
        break;
    case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
        // The declared mode names its own attributes: SerializeSave writes CurrentHue in this mode and
        // SerializeAdd requires it, so there is no second representation to fall back to.
        target = HueSatColor{ .hue = hue, .saturation = saturation };
        break;
    case EnhancedColorModeEnum::kColorTemperatureMireds:
        target = CTColor{ .mireds = mireds };
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    VerifyOrReturnError(SupportsMode(targetColorMode), CHIP_ERROR_INVALID_ARGUMENT);

    return HandleApplyScene(targetColorMode, target, loop, timeMs);
}

// §3.2.7.1.1 consistency check for AddScene: the EFS being *defined* must carry the attributes its
// declared EnhancedColorMode requires. A PRESENCE check on the scene data — not the values, not the
// server's live mode. Done here rather than in the per-pair validator, which sees one pair at a time.
CHIP_ERROR
ColorControlCluster::SerializeAdd(EndpointId endpoint,
                                  const ScenesManagement::Structs::ExtensionFieldSetStruct::DecodableType & extensionFieldSet,
                                  MutableByteSpan & serializedBytes)
{
    bool sawX           = false;
    bool sawY           = false;
    bool sawEnhancedHue = false;
    bool sawCurrentHue  = false;
    bool sawSaturation  = false;
    bool sawMireds      = false;
    std::optional<uint8_t> mode;

    auto it = extensionFieldSet.attributeValueList.begin();
    while (it.Next())
    {
        switch (it.GetValue().attributeID)
        {
        case Attributes::CurrentX::Id:
            sawX = true;
            break;
        case Attributes::CurrentY::Id:
            sawY = true;
            break;
        case Attributes::EnhancedCurrentHue::Id:
            sawEnhancedHue = true;
            break;
        case Attributes::CurrentHue::Id:
            sawCurrentHue = true;
            break;
        case Attributes::CurrentSaturation::Id:
            sawSaturation = true;
            break;
        case Attributes::ColorTemperatureMireds::Id:
            sawMireds = true;
            break;
        case Attributes::EnhancedColorMode::Id:
            if (it.GetValue().valueUnsigned8.HasValue())
            {
                mode = it.GetValue().valueUnsigned8.Value();
            }
            break;
        default:
            break;
        }
    }
    ReturnErrorOnFailure(it.GetStatus());

    // The conditions are gated on EnhancedColorMode's value; with no declared mode there is nothing to
    // enforce here (per-pair type/range is still checked by the validator in the base call below).
    if (mode.has_value())
    {
        switch (static_cast<EnhancedColorModeEnum>(*mode))
        {
        case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
            VerifyOrReturnError(sawCurrentHue && sawSaturation, CHIP_ERROR_INVALID_ARGUMENT);
            break;
        case EnhancedColorModeEnum::kCurrentXAndCurrentY:
            VerifyOrReturnError(sawX && sawY, CHIP_ERROR_INVALID_ARGUMENT);
            break;
        case EnhancedColorModeEnum::kColorTemperatureMireds:
            VerifyOrReturnError(sawMireds, CHIP_ERROR_INVALID_ARGUMENT);
            break;
        case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation:
            VerifyOrReturnError(sawEnhancedHue && sawSaturation, CHIP_ERROR_INVALID_ARGUMENT);
            break;
        default:
            break; // an out-of-range mode value is rejected per-pair by the validator
        }
    }

    return DefaultSceneHandlerImpl::SerializeAdd(endpoint, extensionFieldSet, serializedBytes);
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
    // persisted enhancedColorMode decides WHICH alternative to rebuild (colorMode is derivable, so its
    // persisted copy is ignored). With nothing stored yet, fall back to the first mode the feature map
    // actually supports rather than the ZAP default: without the XY feature, CurrentX/CurrentY are not
    // even in the attribute list, so seeding kCurrentXAndCurrentY would report a mode the endpoint does
    // not have. Enhanced hue/sat is not a candidate — it also needs HS, which the first branch picks.
    EnhancedColorModeEnum mode = HasFeature(Feature::kHueAndSaturation) ? EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation
        : HasFeature(Feature::kXy)                                      ? EnhancedColorModeEnum::kCurrentXAndCurrentY
                                                                        : EnhancedColorModeEnum::kColorTemperatureMireds;
    persistence.LoadNativeEndianValue(path(Attributes::EnhancedColorMode::Id), mode, mode);
    switch (mode)
    {
    // Each axis is seeded from mColorValue — still the constructor-supplied color here — so a first boot
    // with nothing stored keeps the color the application configured. That only applies when the
    // configured color is of this mode's alternative; a stored mode that differs starts from the type's
    // own defaults and overwrites them with its stored axes.
    case EnhancedColorModeEnum::kCurrentXAndCurrentY: {
        XYColor c = std::holds_alternative<XYColor>(mColorValue) ? std::get<XYColor>(mColorValue) : XYColor{};
        persistence.LoadNativeEndianValue(path(CurrentX::Id), c.x, c.x);
        persistence.LoadNativeEndianValue(path(CurrentY::Id), c.y, c.y);
        mColorValue = c;
        break;
    }
    case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation: {
        HueSatColor c = std::holds_alternative<HueSatColor>(mColorValue) ? std::get<HueSatColor>(mColorValue) : HueSatColor{};
        persistence.LoadNativeEndianValue(path(CurrentHue::Id), c.hue, c.hue);
        persistence.LoadNativeEndianValue(path(CurrentSaturation::Id), c.saturation, c.saturation);
        mColorValue = c;
        break;
    }
    case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation: {
        EnhancedHueSatColor c = std::holds_alternative<EnhancedHueSatColor>(mColorValue)
            ? std::get<EnhancedHueSatColor>(mColorValue)
            : EnhancedHueSatColor{};
        persistence.LoadNativeEndianValue(path(EnhancedCurrentHue::Id), c.enhancedHue, c.enhancedHue);
        persistence.LoadNativeEndianValue(path(CurrentSaturation::Id), c.saturation, c.saturation);
        mColorValue = c;
        break;
    }
    case EnhancedColorModeEnum::kColorTemperatureMireds: {
        CTColor c = std::holds_alternative<CTColor>(mColorValue) ? std::get<CTColor>(mColorValue) : CTColor{};
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
        persistence.LoadNativeEndianValue(path(ColorLoopTime::Id), mColorLoop.timeSec, mColorLoop.timeSec);
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
    // loop dormant — nothing drives EnhancedCurrentHue until the next command — so re-arm it here, but only
    // if it is actually DRIVING. The monotonic clock resets across reboot, so the original phase is gone:
    // re-anchor from the just-restored EnhancedCurrentHue and "now" (drift-free from here on). Runs AFTER
    // the startup-CT block on purpose: if StartUpColorTemperatureMireds forced CT mode the loop is
    // active-but-dormant (LoopIsDriving() is false), so the tick is correctly not armed.
    if (HasFeature(Feature::kColorLoop) && LoopIsDriving())
    {
        auto & ehs                   = std::get<EnhancedHueSatColor>(mColorValue);
        mColorLoop.storedEnhancedHue = ehs.enhancedHue; // best-effort restore target (RAM-only, lost on reboot)
        mColorLoopStartHue           = ehs.enhancedHue;
        mColorLoopStartTimeMs        = NowMs();
        LogErrorOnFailure(ArmTick());
    }

    return CHIP_NO_ERROR;
}

template <typename T>
void ColorControlCluster::PersistValue(AttributeId id, const T & value)
{
    VerifyOrReturn(mContext != nullptr);
    AttributePersistence persistence(mContext->attributeStorage);
    LogErrorOnFailure(persistence.StoreNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, ColorControl::Id, id), value));
}

// Persist EnhancedColorMode (which decides the alternative to rebuild — colorMode is derived) plus the
// active mode's stored axes: exactly the set Startup() reads back, at matching native-endian widths.
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

// Freezes the value the stopped axes were at, which would otherwise never reach OnTick's settle. The early
// return keeps a repeated idle Stop from rewriting unchanged values into flash.
bool ColorControlCluster::StopTransitionAndFreeze()
{
    VerifyOrReturnValue(!std::holds_alternative<std::monostate>(mTransition), false);
    mTransition = std::monostate{};
    // Hardware running its own fade must abort it here, or the cluster freezes its value while the light
    // keeps moving and the two disagree for good. The frozen value is the last one already fed to the
    // delegate. Only this cut-short path notifies: a movement that reaches its endpoint ends in OnTick,
    // whose final On*Changed already carried transitionActive == false.
    mDelegate.OnTransitionStopped();
    PersistCurrentColor();
    return true;
}

void ColorControlCluster::PersistColorLoop()
{
    VerifyOrReturn(mContext != nullptr);
    PersistValue(ColorLoopActive::Id, mColorLoop.active);
    PersistValue(Attributes::ColorLoopDirection::Id, mColorLoop.direction);
    PersistValue(ColorLoopTime::Id, mColorLoop.timeSec);
}

void ColorControlCluster::Shutdown(ClusterShutdownType type)
{
    // NOTE: intentionally do NOT unregister the scene handler here. Registration is owned by the
    // application that called table->RegisterHandler(&cluster), so the matching UnregisterHandler() is
    // its responsibility as well (see the class comment) — the scene table keeps a raw handler pointer.

    // Stop the tick so no callback fires after teardown.
    mTimerDelegate.CancelTimer(this);
    DefaultServerCluster::Shutdown(type);
}

// Arm the one-shot tick. Guarded against double-arming so a command issued mid-transition doesn't stack
// a second timer; OnTick re-arms itself while any axis (or the loop) is still moving.
CHIP_ERROR ColorControlCluster::ArmTick()
{
    if (mTimerDelegate.IsTimerActive(this))
    {
        return CHIP_NO_ERROR;
    }
    return mTimerDelegate.StartTimer(this, System::Clock::Milliseconds32(kTickMs));
}

bool ColorControlCluster::LoopIsDriving() const
{
    // Drives only when it holds the green light (not parked by a hue command — see OnTick's latch), is
    // active, and enhanced-HS owns the output (XY / CT / legacy-HS park it via the mode check). The green
    // light is the only thing separating a normally-running loop from one parked after a finite move:
    // both otherwise sit at active==1 / monostate / enhanced-HS.
    return mColorLoopEngaged && mColorLoop.active == 1 && std::holds_alternative<EnhancedHueSatColor>(mColorValue);
}

// §3.2.11: while a color loop is active, the manufacturer MAY ignore commands that change hue. Held in
// one place so every hue-changing command applies the choice identically.
bool ColorControlCluster::ShouldIgnoreHueCommandNow() const
{
    // No StaticConfig at all means the application never opted in, so the commands are honored.
    return HasFeature(Feature::kColorLoop) && mColorLoop.active && mStaticConfig != nullptr &&
        mStaticConfig->ignoreHueCommandsWhileColorLooping;
}

// RemainingTime store + edge-triggered report (§3.2.7.4): report only when it reaches 0, rises from 0
// past the 10-tenths threshold, or a fresh command moves it by MORE than 10 tenths; intermediate ticks
// update the value quietly. The delta gate on the isNewTransition arm keeps a short command (e.g.
// MoveToHue with TransitionTime 5) from reporting. `report` is computed against the OLD value.
void ColorControlCluster::SetQuietReportRemainingTime(uint16_t newRemainingTime, bool isNewTransition)
{
    constexpr uint16_t kReportDelta = 10;
    const bool commandDeltaExceeded = std::abs(newRemainingTime - mState.remainingTime) > kReportDelta;

    const bool report = (newRemainingTime == 0) || (mState.remainingTime == 0 && newRemainingTime > kReportDelta) ||
        (isNewTransition && commandDeltaExceeded);
    SetAttributeValue(mState.remainingTime, newRemainingTime, RemainingTime::Id,
                      report ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet);
}

void ColorControlCluster::OnTick()
{
    const uint64_t now = NowMs();
    bool driverActive  = false;
    uint16_t remaining = 0; // 1/10 s, slowest still-active axis; 0 once everything has settled

    // Remember whether a driver was running: if it settles this tick we persist the final color once.
    const bool hadTransition = !std::holds_alternative<std::monostate>(mTransition);

    // Green-light latch: once a command's hue transition owns the hue axis, the loop parks and stays
    // parked (ColorLoopActive stays 1) even after that transition clears to monostate — only ColorLoopSet
    // re-engages it. Checked BEFORE the dispatch below so a single-tick (transitionTime == 0) hue command
    // latches it before its transition completes. A sat-only transition has no hue axis → loop keeps hue.
    if (auto * hsx = std::get_if<HueSatTransition>(&mTransition); hsx != nullptr && hsx->hue.has_value())
    {
        mColorLoopEngaged = false;
    }

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
        // Hue and saturation are independent axes (§3.2.5.2): clearing one leaves the other running.
        // Each Tick* returns false when its axis has finished. Loop-vs-hue exclusivity is enforced at
        // the commands, so this branch stays loop-agnostic.
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
        // A transition that just settled wrote the exact target into mColorValue; persist it so a reboot
        // restores where the light actually ended up. Guarded on hadTransition so idle ticks don't write.
        if (hadTransition)
        {
            PersistCurrentColor();
        }
    }

    // The color loop is a mode-independent hue driver kept out of mTransition; it may keep ticking even at
    // monostate — but only while it holds the green light (latched off above once a hue command took the
    // axis). It never ends, so it pins RemainingTime at MAX until Deactivate.
    const bool loopActive = TickColorLoop(now);
    if (loopActive)
    {
        remaining = kMaxInt16uValue;
    }

    // ONE RemainingTime write per tick (§3.2.7.4). isNewTransition is false here: only a command can
    // cause the delta-based report, so a tick reports only on hitting 0 or crossing 0→(>10).
    SetQuietReportRemainingTime(remaining, /*isNewTransition=*/false);

    // Re-arm only while something is still moving → zero CPU at steady state.
    if (driverActive || loopActive)
    {
        LogErrorOnFailure(ArmTick());
    }
}

// ── Tick*: one step of one axis ─────────────────────────────────────────────────────────────────
// Each interpolates its axis from the wall-clock anchor (landing on the exact endpoint on the last
// tick), stores the result, fans it out, and returns whether the axis is still moving. OnTick reads
// the clock once and passes `now` in, so the tick itself is pure math.
//
// The two fan-out channels run at different cadences:
//   * NETWORK REPORTING is throttled (§3.2.7.2) — the kQuiet/kReportable flag each Tick* computes
//     keeps the wire silent mid-transition and reports on the arriving tick.
//   * HARDWARE OUTPUT is continuous (§3.2.8) — the delegate owns no timer, so its callbacks fire on
//     EVERY tick regardless of that flag.
//
// Each output callback carries transitionActive == !done for its own axis: true marks an intermediate
// frame, false the exact endpoint. Hardware ramping natively (see the transition-start notifications on
// ColorControlDelegate) drops the true frames and applies only the endpoint, which re-syncs it with the
// cluster's own interpolation.

bool ColorControlCluster::TickHue(HueTransition & tx, uint64_t now)
{
    // signedDelta is the signed arc to travel over durationMs — or, for a MoveHue rate move
    // (durationMs == kIndefiniteHueMoveMs), hue-units per second with no endpoint.
    const auto [position, done] = Interpolate(tx.startHue, tx.signedDelta, tx.startTimeMs, tx.durationMs, now);
    const uint16_t eh           = static_cast<uint16_t>(position & 0xFFFF); // hue is circular: mod kHueCircle, never clamped

    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // Exactly one of CurrentHue / EnhancedCurrentHue is a stored field and the other is a projection of
    // it, decided by the active alternative. Store at that alternative's native precision, signal the
    // projection notify-only, and hand the hardware the same precision.
    if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        SetAttributeValue(ehs->enhancedHue, eh, EnhancedCurrentHue::Id, change);
        NotifyAttributeChanged(CurrentHue::Id, change);
        // 16-bit out; truncating here would drop the EHUE resolution
        mDelegate.OnEnhancedHueChanged(ehs->enhancedHue, !done);
    }
    else
    {
        auto & hs = std::get<HueSatColor>(mColorValue);
        SetAttributeValue(hs.hue, static_cast<uint8_t>(eh >> 8), CurrentHue::Id, change);
        NotifyAttributeChanged(EnhancedCurrentHue::Id, change);
        mDelegate.OnColorHSChanged(hs.hue, hs.saturation, !done);
    }

    return !done;
}

bool ColorControlCluster::TickSat(SatTransition & tx, uint64_t now)
{
    // Saturation is linear (not circular) and 8-bit.
    const auto [position, done] =
        Interpolate(tx.startSat, int32_t{ tx.targetSat } - int32_t{ tx.startSat }, tx.startTimeMs, tx.durationMs, now);
    const uint8_t sat = static_cast<uint8_t>(position);

    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // CurrentSaturation is one stored value shared by legacy and enhanced HS — no projection to signal.
    if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        SetAttributeValue(ehs->saturation, sat, CurrentSaturation::Id, change);
        // OnColorHSChanged is the only saturation channel, so it carries the 8-bit hue too. Re-assert
        // the 16-bit hue afterwards: a hue transition running alongside this one feeds the hardware at
        // full precision, and the truncated hue above must not be the last word between its ticks.
        //
        // That re-assert describes the HUE axis, so it carries the hue axis's own liveness, not this
        // axis's `done`: hardware fading the hue must not be snapped to the software position just
        // because saturation arrived first. OnTick ticks hue before saturation and clears a finished
        // axis right after its Tick* returns, so hsx->hue reads post-completion here.
        const auto * hsx     = std::get_if<HueSatTransition>(&mTransition);
        const bool hueMoving = (hsx != nullptr) && hsx->hue.has_value();
        mDelegate.OnColorHSChanged(ehs->hue8(), ehs->saturation, !done);
        mDelegate.OnEnhancedHueChanged(ehs->enhancedHue, hueMoving);
    }
    else
    {
        auto & hs = std::get<HueSatColor>(mColorValue);
        SetAttributeValue(hs.saturation, sat, CurrentSaturation::Id, change);
        mDelegate.OnColorHSChanged(hs.hue, hs.saturation, !done);
    }

    return !done;
}

bool ColorControlCluster::TickCT(CTTransition & tx, uint64_t now)
{
    // Color temperature is linear, 16-bit mireds.
    const auto [position, done] =
        Interpolate(tx.startMireds, int32_t{ tx.targetMireds } - int32_t{ tx.startMireds }, tx.startTimeMs, tx.durationMs, now);
    const uint16_t mireds = static_cast<uint16_t>(position);

    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // In CT mode mColorValue is always CTColor (variant and mode are kept in lockstep), so get<> is safe.
    auto & ct = std::get<CTColor>(mColorValue);
    SetAttributeValue(ct.mireds, mireds, ColorTemperatureMireds::Id, change);
    mDelegate.OnColorCTChanged(ct.mireds, !done);

    return !done;
}

// The autonomous enhanced-hue driver. Unlike the axes held in mTransition, the loop is mode-independent
// and NEVER finishes — it cycles EnhancedCurrentHue around the full [0, kHueCircle) circle at a constant
// rate until ColorLoopSet(Deactivate) (§3.2.8.1). Nothing else stops it.
//
// Position is derived from the wall-clock anchor (mColorLoopStartHue / mColorLoopStartTimeMs) stamped
// when the loop started, so it is drift-free. Crucially the anchor is NOT re-stamped when the loop goes
// dormant: its phase keeps advancing with real time even while XY/CT owns the output, so when the mode
// returns to enhanced HS the hue is already where wall-clock says it should be — no jump.
bool ColorControlCluster::TickColorLoop(uint64_t now)
{
    // Dormancy guard (§3.2.8.1): the loop can be active while XY/CT owns the output. A dormant loop
    // writes nothing and returns false — it has no reason to keep the tick alive, and whatever command
    // switches the mode back to enhanced HS re-arms the timer itself.
    if (!LoopIsDriving())
    {
        return false;
    }

    // Rate is fixed by ColorLoopTime = seconds for one full kHueCircle revolution. Guard timeSec == 0.
    const uint32_t loopTimeSec = (mColorLoop.timeSec == 0) ? 1u : mColorLoop.timeSec;

    // Position from the wall-clock anchor. INTEGER math on purpose: the loop never ends, so the
    // float error every other Tick* tolerates would accumulate without bound here. kHueCircle * elapsed
    // fits comfortably in int64 for any realistic uptime.
    const uint64_t elapsedMs = now - mColorLoopStartTimeMs;
    const int64_t traveled =
        static_cast<int64_t>(kHueCircle) * static_cast<int64_t>(elapsedMs) / (static_cast<int64_t>(loopTimeSec) * 1000);
    // ColorLoopDirection: 1 == increment (up), 0 == decrement (down).
    const int64_t signedTraveled = mColorLoop.direction ? traveled : -traveled;

    const uint16_t eh =
        static_cast<uint16_t>((static_cast<int64_t>(mColorLoopStartHue) + signedTraveled) & 0xFFFF); // wrap mod kHueCircle

    // LoopIsDriving() guarantees enhanced HS, so get<> is safe. The loop has no arriving tick, so every
    // tick is kQuiet and every delegate frame is transitionActive == true: continuous movement that only
    // ColorLoopSet(Deactivate) ends, with no settled value to apply.
    auto & ehs = std::get<EnhancedHueSatColor>(mColorValue);
    SetAttributeValue(ehs.enhancedHue, eh, EnhancedCurrentHue::Id, AttributeChangeType::kQuiet);
    NotifyAttributeChanged(CurrentHue::Id, AttributeChangeType::kQuiet); // projection
    mDelegate.OnEnhancedHueChanged(ehs.enhancedHue, /*transitionActive=*/true);

    return true; // keep the tick alive until ColorLoopSet(Deactivate)
}

bool ColorControlCluster::TickXY(XYTransition & tx, uint64_t now)
{
    // CIE X/Y are 16-bit and linear, and the axes ramp independently: one start time, a duration each.
    const auto [positionX, doneX] =
        Interpolate(tx.startX, int32_t{ tx.targetX } - int32_t{ tx.startX }, tx.startTimeMs, tx.durationXMs, now);
    const auto [positionY, doneY] =
        Interpolate(tx.startY, int32_t{ tx.targetY } - int32_t{ tx.startY }, tx.startTimeMs, tx.durationYMs, now);

    const uint16_t x = static_cast<uint16_t>(positionX);
    const uint16_t y = static_cast<uint16_t>(positionY);
    const bool done  = doneX && doneY; // XY is done only when BOTH axes arrive

    const auto change = done ? AttributeChangeType::kReportable : AttributeChangeType::kQuiet;

    // In XY mode mColorValue is always XYColor (variant and mode are kept in lockstep), so get<> is safe.
    auto & xy = std::get<XYColor>(mColorValue);
    SetAttributeValue(xy.x, x, CurrentX::Id, change);
    SetAttributeValue(xy.y, y, CurrentY::Id, change);
    mDelegate.OnColorXYChanged(xy.x, xy.y, !done);

    return !done;
}

ColorControlCluster::Status ColorControlCluster::StopMoveStep(OptMask optionsMask, OptMask optionsOverride)
{
    // OnOff / Options gating.
    if (!ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    // Stops only the mTransition-driven axes. §3.2.8.1 ("no effect while a color loop is active") needs no
    // guard: a color loop is NOT in mTransition — TickColorLoop drives it from mColorLoop plus its
    // wall-clock anchor — so it is structurally untouched and keeps running. Guarding on mColorLoop.active
    // instead would over-reach: that attribute stays 1 while the loop is dormant, and while the loop drives
    // hue the saturation axis can still carry an independent transition — in both cases a real transition
    // must stop. Deliberately does NOT cancel the tick: OnTick self-terminates once nothing is driving.
    StopTransitionAndFreeze();
    // A driving loop pins RemainingTime at its max (see OnTick), so only zero it when nothing is left.
    if (!LoopIsDriving())
    {
        SetQuietReportRemainingTime(0, /*isNewTransition=*/false);
    }
    return Status::Success;
}

bool ColorControlCluster::ShouldExecuteIfOff(BitMask<OptionsBitmap> mask, BitMask<OptionsBitmap> optionsOverride)
{
    // §3.2.8.3 — effective = (options where mask bit = 0) | (override where mask bit = 1)
    BitMask<OptionsBitmap> effective{ static_cast<uint8_t>((mState.options.Raw() & ~mask.Raw()) |
                                                           (optionsOverride.Raw() & mask.Raw())) };

    // On/Off coupling via the injected cluster: no injected cluster == no coupling.
    if (mOnOff == nullptr)
    {
        return true;
    }
    if (mOnOff->GetOnOff())
    {
        return true;
    }
    return effective.Has(OptionsBitmap::kExecuteIfOff);
}

void ColorControlCluster::ApplyModeSwitch(EnhancedColorModeEnum target)
{
    // Each arm converts the current color through the delegate and assigns the new alternative; that
    // assignment IS the store, since colorMode / enhancedColorMode are derived from which one is active.
    switch (target)
    {
    case EnhancedColorModeEnum::kCurrentXAndCurrentY: {
        if (std::holds_alternative<XYColor>(mColorValue))
        {
            return;
        }
        XYColor next{};
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToXY(hs->hue, hs->saturation, next.x, next.y);
        }
        else if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToXY(ehs->hue8(), ehs->saturation, next.x, next.y);
        }
        else if (auto * ct = std::get_if<CTColor>(&mColorValue))
        {
            mDelegate.ConvertMiredsToXY(ct->mireds, next.x, next.y);
        }
        mColorValue = next;
        NotifyAttributeChanged(CurrentX::Id, AttributeChangeType::kQuiet);
        NotifyAttributeChanged(CurrentY::Id, AttributeChangeType::kQuiet);
        break;
    }
    case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
    case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation: {
        const bool toEnhanced = (target == EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation);
        // No-op only when already in the EXACT target alternative: legacy and enhanced HS are distinct modes.
        if (toEnhanced && std::holds_alternative<EnhancedHueSatColor>(mColorValue))
        {
            return;
        }
        if (!toEnhanced && std::holds_alternative<HueSatColor>(mColorValue))
        {
            return;
        }

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
            mDelegate.ConvertXYToHueSat(xy->x, xy->y, h, sat);
            eh = static_cast<uint16_t>(uint16_t(h) << 8);
        }
        else if (auto * ct = std::get_if<CTColor>(&mColorValue))
        {
            uint8_t h = 0;
            mDelegate.ConvertMiredsToHueSat(ct->mireds, h, sat);
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
        {
            return;
        }
        CTColor next{};
        if (auto * xy = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate.ConvertXYToMireds(xy->x, xy->y, next.mireds);
        }
        else if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToMireds(hs->hue, hs->saturation, next.mireds);
        }
        else if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToMireds(ehs->hue8(), ehs->saturation, next.mireds);
        }
        mColorValue = next;
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

// Current-color accessors used by command start-values and apply-scene. Callers run ApplyModeSwitch to
// an HS mode first, so mColorValue is a HueSat alternative here; the fallbacks keep them total.
uint8_t ColorControlCluster::GetSaturation() const
{
    if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        return e->saturation;
    }
    if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
    {
        return hs->saturation;
    }
    return 0;
}

uint16_t ColorControlCluster::GetEnhancedHue() const
{
    if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        return e->enhancedHue;
    }
    if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
    {
        return hs->enhancedHue(); // 8-bit → high byte
    }
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
    {
        return EnhancedColorModeEnum::kCurrentXAndCurrentY;
    }
    if (std::holds_alternative<CTColor>(mColorValue))
    {
        return EnhancedColorModeEnum::kColorTemperatureMireds;
    }
    if (std::holds_alternative<EnhancedHueSatColor>(mColorValue))
    {
        return EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation;
    }
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

void ColorControlCluster::StartColorLoop(bool startFromStartHue)
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
    mColorLoopEngaged            = true;            // (re)engage the green light — ColorLoopSet is the only way back on
    mColorLoop.storedEnhancedHue = ehs.enhancedHue; // remember where to return on stop
    NotifyAttributeChanged(ColorLoopStoredEnhancedHue::Id, AttributeChangeType::kReportable);

    if (startFromStartHue) // per ColorLoopSet startHue action
    {
        ehs.enhancedHue = mColorLoop.startEnhancedHue;
        NotifyAttributeChanged(EnhancedCurrentHue::Id, AttributeChangeType::kReportable);
        NotifyAttributeChanged(CurrentHue::Id, AttributeChangeType::kReportable); // projection
    }

    // Stamp the wall-clock anchor TickColorLoop interpolates from: once here and never again (dormancy
    // does not re-stamp), so the loop's phase tracks real time. The anchor hue is where we start from,
    // after the optional jump to startEnhancedHue above.
    mColorLoopStartHue    = ehs.enhancedHue;
    mColorLoopStartTimeMs = NowMs();

    // Scenable loop attributes just changed → kReportable (no quieterReporting quality).
    NotifyAttributeChanged(ColorLoopActive::Id, AttributeChangeType::kReportable);
    NotifyAttributeChanged(Attributes::ColorLoopDirection::Id, AttributeChangeType::kReportable);
    NotifyAttributeChanged(ColorLoopTime::Id, AttributeChangeType::kReportable);

    // The start switched to enhanced-HS mode and moved EnhancedCurrentHue → persist the settled color so
    // a reboot resumes from the right hue. The loop attributes are persisted by the caller.
    PersistCurrentColor();

    // Hand the loop to the delegate: hardware able to rotate hue on its own starts here and ignores the
    // per-tick OnEnhancedHueChanged frames. The anchor stamped just above IS the starting phase, so this
    // doubles as the re-anchor when ColorLoopSet(Activate) restarts an already-running loop.
    mDelegate.OnColorLoopStarted(mColorLoopStartHue, mColorLoop.timeSec, mColorLoop.direction != 0);

    LogErrorOnFailure(ArmTick()); // the tick is what advances the hue
}

Status ColorControlCluster::MoveToSaturation(uint8_t saturation, uint16_t transitionTimeDs, BitMask<OptionsBitmap> optionsMask,
                                             BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(saturation <= kMaxSaturationValue, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    // Saturation is flavor-neutral (uint8_t in both hue/sat modes, and MoveToSaturation has no Enhanced*
    // twin), so keep whichever flavor is active: asking for legacy unconditionally would truncate
    // EnhancedCurrentHue to its high byte (§3.2.7.12) and, via LoopIsDriving, silently park a running
    // color loop. Coming from XY / CT the delegate yields an 8-bit hue anyway, so legacy is right there.
    ApplyModeSwitch(std::holds_alternative<EnhancedHueSatColor>(mColorValue)
                        ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                        : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);
    const uint32_t durationMs = transitionTimeDs * 100u;
    auto & hs                 = EnsureHueSatTransition(); // HueSatTransition; preserves .hue if already one, replaces XY/CT
    hs.sat                    = SatTransition{
                           .startSat    = GetSaturation(), // read AFTER the mode switch
                           .targetSat   = std::clamp<uint8_t>(saturation, kMinSaturationValue, kMaxSaturationValue),
                           .startTimeMs = NowMs(),
                           .durationMs  = durationMs,
    };
    SetQuietReportRemainingTime(RemainingTenthsFromMs(durationMs), /*isNewTransition=*/true);
    NotifySatTransition(*hs.sat);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
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

Status ColorControlCluster::MoveToHueAndSaturation(uint16_t hue, uint8_t saturation, uint16_t transitionTimeDs, bool isEnhanced,
                                                   BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // Same constraint checks as MoveToSaturation (its twin): reject before any mode switch / transition.
    // Legacy Hue is constrained to kMaxCurrentHue; EnhancedHue spans the full uint16 range.
    VerifyOrReturnValue(isEnhanced || hue <= kMaxCurrentHue, Status::ConstraintError);
    VerifyOrReturnValue(saturation <= kMaxSaturationValue, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    // Saturation always moves; the hue axis moves unless the manufacturer opted to ignore hue commands
    // while a color loop runs (§3.2.11). The decision is made here and passed to the builder as moveHue.
    const bool moveHue      = !ShouldIgnoreHueCommandNow();
    const uint8_t targetSat = std::clamp<uint8_t>(saturation, kMinSaturationValue, kMaxSaturationValue);
    const uint32_t timeMs   = transitionTimeDs * 100u;
    if (isEnhanced)
    {
        StartEnhancedHueAndSatTransition(EnhancedHueSatColor{ .enhancedHue = hue, .saturation = targetSat }, timeMs, moveHue);
    }
    else
    {
        // HueSatColor holds the legacy 8-bit hue as-is (enhancedHue() projects it up), so the command
        // value goes in unshifted; the constraint check above already bounded it.
        StartHueAndSatTransition(HueSatColor{ .hue = static_cast<uint8_t>(hue), .saturation = targetSat }, timeMs, moveHue);
    }
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

// ── Transition builders (shared by the commands and scene restore) ──────────────────────────────
// Each populates mTransition, stamps the clock and sets RemainingTime. They do NOT switch mode — the
// caller (HandleApplyScene, or a command) runs ApplyModeSwitch first, so mColorValue already holds the
// matching alternative and its current value is the start of the transition (a mid-flight apply picks up
// from there) — and they do not arm the tick, which the caller does so a failed arm can be surfaced.
//
// `target` is a ColorValue of the active mode's alternative; `timeMs` is the transition time in
// milliseconds, used as durationMs directly. RemainingTime is reported in deciseconds.

// ── Transition start notifications (contract documented in the header) ──────────────────────────
// The zero-duration test lives here, once, so no origin can announce a discrete jump (TransitionTime
// == 0) as a movement.

void ColorControlCluster::NotifyXYTransition(const XYTransition & tx)
{
    // MoveColor gives each axis its own rate, so an idle axis is a 0 duration next to a moving one. Only
    // both-idle means nothing is actually moving.
    VerifyOrReturn(tx.durationXMs != 0 || tx.durationYMs != 0);
    mDelegate.OnXYTransitionStarted(tx.targetX, tx.durationXMs, tx.targetY, tx.durationYMs);
}

void ColorControlCluster::NotifyCTTransition(const CTTransition & tx)
{
    VerifyOrReturn(tx.durationMs != 0);
    mDelegate.OnCTTransitionStarted(tx.targetMireds, tx.durationMs);
}

void ColorControlCluster::NotifyHueTransition(const HueTransition & tx)
{
    // kIndefiniteHueMoveMs passes the guard: a rate move only ends when a Stop arrives, so it is the most
    // continuous movement there is and the one hardware most wants to own.
    VerifyOrReturn(tx.durationMs != 0);
    mDelegate.OnEnhancedHueTransitionStarted(tx.signedDelta, tx.durationMs);
}

void ColorControlCluster::NotifySatTransition(const SatTransition & tx)
{
    VerifyOrReturn(tx.durationMs != 0);
    mDelegate.OnSaturationTransitionStarted(tx.targetSat, tx.durationMs);
}

void ColorControlCluster::StartXYTransition(const ColorControl::ColorValue & target, uint32_t timeMs)
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
        .startTimeMs = NowMs(),
    };
    SetQuietReportRemainingTime(RemainingTenthsFromMs(timeMs), /*isNewTransition=*/true);
    NotifyXYTransition(std::get<XYTransition>(mTransition));
}

void ColorControlCluster::StartColorTemperatureTransition(const ColorControl::ColorValue & target, uint32_t timeMs)
{
    const auto & tgt = std::get<CTColor>(target);
    auto & ct        = std::get<CTColor>(mColorValue);

    mTransition = CTTransition{
        .startMireds  = ct.mireds,
        .targetMireds = std::clamp<uint16_t>(tgt.mireds, mCT.colorTempPhysicalMinMireds,
                                             mCT.colorTempPhysicalMaxMireds), // scenes clamp to physical range
        .startTimeMs  = NowMs(),
        .durationMs   = timeMs,
    };
    SetQuietReportRemainingTime(RemainingTenthsFromMs(timeMs), /*isNewTransition=*/true);
    NotifyCTTransition(std::get<CTTransition>(mTransition));
}

void ColorControlCluster::StartHueAndSatTransition(const ColorControl::ColorValue & target, uint32_t timeMs, bool moveHue)
{
    const auto & tgt   = std::get<HueSatColor>(target);
    auto & hs          = std::get<HueSatColor>(mColorValue);
    const uint64_t now = NowMs();

    HueSatTransition next{};
    // Saturation always moves. The hue axis moves only when the caller allows it: a command may defer hue
    // to an active color loop per the manufacturer's choice (§3.2.11); the scene path always passes true.
    if (moveHue)
    {
        // Hue always runs in 16-bit canonical, and legacy hue is the high byte: project both ends up and
        // take the shortest arc round the circle (the same math the hue commands compute inline).
        const uint16_t start  = hs.enhancedHue();
        const uint16_t tgtHue = tgt.enhancedHue();
        const uint16_t upArc  = static_cast<uint16_t>(tgtHue - start);
        const int32_t arc     = (upArc <= kHalfHueCircle) ? upArc : (upArc - kHueCircle);
        next.hue              = HueTransition{ start, arc, now, timeMs };
    }
    next.sat    = SatTransition{ .startSat = hs.saturation, .targetSat = tgt.saturation, .startTimeMs = now, .durationMs = timeMs };
    mTransition = next;

    SetQuietReportRemainingTime(RemainingTenthsFromMs(timeMs), /*isNewTransition=*/true);
    // One notification per axis: §3.2.5.2 makes them independent, and `moveHue` may have left hue idle.
    if (next.hue)
    {
        NotifyHueTransition(*next.hue);
    }
    NotifySatTransition(*next.sat);
}

void ColorControlCluster::StartEnhancedHueAndSatTransition(const ColorControl::ColorValue & target, uint32_t timeMs, bool moveHue)
{
    const auto & tgt   = std::get<EnhancedHueSatColor>(target);
    auto & ehs         = std::get<EnhancedHueSatColor>(mColorValue);
    const uint64_t now = NowMs();

    HueSatTransition next{};
    // Saturation always moves; the hue axis moves only when the caller allows it (see StartHueAndSatTransition).
    if (moveHue)
    {
        const uint16_t start  = ehs.enhancedHue;
        const uint16_t tgtHue = tgt.enhancedHue;
        const uint16_t upArc  = static_cast<uint16_t>(tgtHue - start);
        const int32_t arc     = (upArc <= kHalfHueCircle) ? upArc : (upArc - kHueCircle);
        next.hue              = HueTransition{ start, arc, now, timeMs };
    }
    next.sat = SatTransition{ .startSat = ehs.saturation, .targetSat = tgt.saturation, .startTimeMs = now, .durationMs = timeMs };
    mTransition = next;

    SetQuietReportRemainingTime(RemainingTenthsFromMs(timeMs), /*isNewTransition=*/true);
    // One notification per axis: §3.2.5.2 makes them independent, and `moveHue` may have left hue idle.
    if (next.hue)
    {
        NotifyHueTransition(*next.hue);
    }
    NotifySatTransition(*next.sat);
}

CHIP_ERROR ColorControlCluster::HandleApplyScene(ColorControl::EnhancedColorModeEnum ColorMode,
                                                 const ColorControl::ColorValue & target, const ColorControl::ColorLoopState & loop,
                                                 uint32_t timeMs)
{
    // A saved active loop takes over the output, so the scene's saved color axes are deliberately not
    // restored — the loop drives hue itself. Gated on the feature: nothing upstream rejects a scene
    // carrying ColorLoopActive on an endpoint without ColorLoop support (the per-pair validator only
    // type-checks, SerializeAdd only checks mode/axis presence), and starting a loop there would switch
    // the endpoint into enhanced-hue mode and report loop attributes that do not exist on it.
    if (HasFeature(Feature::kColorLoop) && loop.active == 1)
    {
        mColorLoop.active    = 1;
        mColorLoop.direction = loop.direction;
        mColorLoop.timeSec   = loop.timeSec;
        StartColorLoop(/*startFromStartHue=*/true); // persists the current color; arms its own tick (best-effort)
        PersistColorLoop();                         // persist the loop attrs the scene just applied
        return CHIP_NO_ERROR;
    }

    if (HasFeature(Feature::kColorLoop))
    {
        StopColorLoop();
        PersistColorLoop();
    }

    // Switch into the scene's mode ONCE (converts the current color into this representation, which
    // becomes the transition start); the pure Start* helpers below assume the variant already matches.
    ApplyModeSwitch(ColorMode);

    switch (ColorMode)
    {
    case ColorControl::EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
        StartHueAndSatTransition(target, timeMs, /*moveHue=*/true); // scene restores both axes
        break;
    case ColorControl::EnhancedColorModeEnum::kCurrentXAndCurrentY:
        StartXYTransition(target, timeMs);
        break;
    case ColorControl::EnhancedColorModeEnum::kColorTemperatureMireds:
        StartColorTemperatureTransition(target, timeMs);
        break;
    case ColorControl::EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation:
        StartEnhancedHueAndSatTransition(target, timeMs, /*moveHue=*/true); // scene restores both axes
        break;
    default:
        break;
    }

    // Arming the tick is the one fallible step, done here so a failure propagates out through ApplyScene
    // rather than leaving a scene un-animated.
    return ArmTick();
}

Status ColorControlCluster::MoveHue(MoveModeEnum moveMode, uint16_t rate, bool isEnhanced, BitMask<OptionsBitmap> optionsMask,
                                    BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(moveMode != MoveModeEnum::kUnknownEnumValue, Status::InvalidCommand);

    if (moveMode == MoveModeEnum::kStop)
    {
        // Stop halts any ongoing hue *and* saturation transition (§3.2.8.5.4): the §3.2.5.2 axis
        // independence that lets the two overlap governs only MoveMode != Stop. It is scoped to those
        // two axes though — an XY / CT transition is neither, so it keeps running. It must also not
        // disturb an active color loop (§3.2.8.1), which needs no guard: the loop is not in mTransition.
        if (std::holds_alternative<HueSatTransition>(mTransition))
        {
            StopTransitionAndFreeze(); // freeze: persist the hue/saturation the movement stopped at
            // A driving loop pins RemainingTime at its max (see OnTick), so only zero it when nothing is left.
            if (!LoopIsDriving())
            {
                SetQuietReportRemainingTime(0, /*isNewTransition=*/false);
            }
        }
        return Status::Success;
    }

    // Hue-changing move: honored unless the manufacturer opted to ignore hue commands while a loop runs.
    VerifyOrReturnValue(!ShouldIgnoreHueCommandNow(), Status::Success);
    VerifyOrReturnValue(rate != 0, Status::InvalidCommand);

    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    // direction is the SIGN of the rate; legacy 8-bit rate projects to 16-bit canonical
    int32_t signedRatePerSec = (isEnhanced ? rate : rate << 8) * (moveMode == MoveModeEnum::kUp ? +1 : -1);

    auto & hs = EnsureHueSatTransition(); // preserve a running sat axis (§3.2.5.2)
    hs.hue    = HueTransition{
           .startHue    = GetEnhancedHue(), // 16-bit canonical current
           .signedDelta = signedRatePerSec, // hue-units per second; sign = up/down
           .startTimeMs = NowMs(),
           .durationMs  = kIndefiniteHueMoveMs, // rate move: runs until a Stop command
    };
    SetQuietReportRemainingTime(kMaxInt16uValue, /*isNewTransition=*/true);
    NotifyHueTransition(*hs.hue);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::MoveToHue(uint16_t hue, DirectionEnum dir, uint16_t transitionTimeDs, bool isEnhanced,
                                      BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // Hue-changing command: honored unless the manufacturer opted to ignore hue commands while a loop runs.
    VerifyOrReturnValue(!ShouldIgnoreHueCommandNow(), Status::Success);

    VerifyOrReturnValue(dir != DirectionEnum::kUnknownEnumValue, Status::InvalidCommand);
    // Both MoveToHue and EnhancedMoveToHue carry a uint16 TransitionTime constrained to max 65534.
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);
    // MoveToHue's Hue is constrained to kMaxCurrentHue; EnhancedMoveToHue's EnhancedHue spans the full uint16 range.
    VerifyOrReturnValue(isEnhanced || hue <= kMaxCurrentHue, Status::ConstraintError);

    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    const uint16_t start = GetEnhancedHue(); // 16-bit canonical
    // The arc math below is all 16-bit, so a legacy hue is projected up into that space; the constraint
    // check above bounded it at kMaxCurrentHue (§3.2.7.11). Enhanced hue is already canonical.
    const uint16_t target = isEnhanced ? hue : static_cast<uint16_t>(hue << 8);
    const uint16_t upArc  = static_cast<uint16_t>(target - start); // distance going up (wraps)

    bool up = false;
    switch (dir)
    {
    case DirectionEnum::kUp:
        up = true;
        break;
    case DirectionEnum::kDown:
        up = false;
        break;
    // On an exact half-circle both arcs are the same length; §3.2.8.4.5 resolves that tie to Up, hence
    // the inclusive comparison on BOTH arms.
    case DirectionEnum::kShortest:
        up = (upArc <= kHalfHueCircle);
        break; // up is the shorter arc
    case DirectionEnum::kLongest:
        up = (upArc >= kHalfHueCircle);
        break; // up is the longer arc
    case DirectionEnum::kUnknownEnumValue:
        return Status::InvalidCommand;
    }
    const int32_t signedArc = up ? upArc : (upArc - kHueCircle);

    auto & hs = EnsureHueSatTransition();
    hs.hue    = HueTransition{ start, signedArc, NowMs(), transitionTimeDs * 100u };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    NotifyHueTransition(*hs.hue);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::StepHue(StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs, bool isEnhanced,
                                    BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // Hue-changing command: honored unless the manufacturer opted to ignore hue commands while a loop runs.
    VerifyOrReturnValue(!ShouldIgnoreHueCommandNow(), Status::Success);

    VerifyOrReturnValue(stepMode != StepModeEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(stepSize != 0, Status::InvalidCommand);
    // EnhancedStepHue's TransitionTime is a uint16 constrained to max 0xFFFE (legacy StepHue's is a uint8,
    // so it can never trip this).
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(isEnhanced ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                               : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    const int32_t step16      = isEnhanced ? stepSize : static_cast<int32_t>(stepSize) << 8;
    const int32_t signedDelta = (stepMode == StepModeEnum::kUp ? +1 : -1) * step16; // sign from stepMode

    auto & hs = EnsureHueSatTransition();
    hs.hue    = HueTransition{ GetEnhancedHue(), signedDelta, NowMs(), transitionTimeDs * 100u };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    NotifyHueTransition(*hs.hue);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::MoveSaturation(MoveModeEnum moveMode, uint8_t rate, BitMask<OptionsBitmap> optionsMask,
                                           BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(moveMode != MoveModeEnum::kUnknownEnumValue, Status::InvalidCommand);

    if (moveMode == MoveModeEnum::kStop)
    {
        // Halts the hue axis as well as saturation (§3.2.8.8.4); mirrors MoveHue's Stop exactly.
        if (std::holds_alternative<HueSatTransition>(mTransition))
        {
            StopTransitionAndFreeze(); // freeze: persist the hue/saturation the movement stopped at
            // A driving loop pins RemainingTime at its max (see OnTick), so only zero it when nothing is left.
            if (!LoopIsDriving())
            {
                SetQuietReportRemainingTime(0, /*isNewTransition=*/false);
            }
        }
        return Status::Success;
    }
    VerifyOrReturnValue(rate != 0, Status::InvalidCommand);

    // Flavor-neutral like the other saturation commands: preserve enhanced hue/sat rather than demoting
    // it (see MoveToSaturation).
    ApplyModeSwitch(std::holds_alternative<EnhancedHueSatColor>(mColorValue)
                        ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                        : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    const uint8_t start       = GetSaturation();
    const uint8_t target      = (moveMode == MoveModeEnum::kUp) ? kMaxSaturationValue : kMinSaturationValue;
    const uint32_t distance   = (start > target) ? (start - target) : (target - start);
    const uint32_t durationMs = distance * 1000u / rate; // distance / rate → ms

    auto & hs = EnsureHueSatTransition(); // preserve a running HUE axis (§3.2.5.2)
    hs.sat    = SatTransition{
           .startSat    = start,
           .targetSat   = target, // the boundary — bounded, so it stops here
           .startTimeMs = NowMs(),
           .durationMs  = durationMs,
    };
    SetQuietReportRemainingTime(RemainingTenthsFromMs(durationMs), /*isNewTransition=*/true);
    NotifySatTransition(*hs.sat);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::StepSaturation(StepModeEnum stepMode, uint8_t stepSize, uint16_t transitionTimeDs,
                                           BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(stepMode != StepModeEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(stepSize != 0, Status::InvalidCommand);

    // Flavor-neutral like the other saturation commands: preserve enhanced hue/sat rather than demoting
    // it (see MoveToSaturation).
    ApplyModeSwitch(std::holds_alternative<EnhancedHueSatColor>(mColorValue)
                        ? EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation
                        : EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);

    const int32_t signedDelta = (stepMode == StepModeEnum::kUp ? +1 : -1) * static_cast<int32_t>(stepSize);
    const uint8_t start       = GetSaturation();
    const uint8_t target =
        static_cast<uint8_t>(std::clamp<int32_t>(int32_t(start) + signedDelta, kMinSaturationValue, kMaxSaturationValue));

    auto & hs = EnsureHueSatTransition(); // preserve a running HUE axis
    hs.sat    = SatTransition{ start, target, NowMs(), transitionTimeDs * 100u };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    NotifySatTransition(*hs.sat);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

void ColorControlCluster::StopColorLoop()
{
    if (!mColorLoop.active)
    {
        return;
    }
    mColorLoop.active = 0;
    NotifyAttributeChanged(ColorLoopActive::Id, AttributeChangeType::kReportable);

    // Restore only when enhanced-HS still owns the output; a MoveToColor/MoveToColorTemperature
    // can leave the loop active-but-dormant with XY/CT owning mColorValue (§3.2.8.1).
    if (auto * ehs = std::get_if<EnhancedHueSatColor>(&mColorValue))
    {
        ehs->enhancedHue = mColorLoop.storedEnhancedHue;
        NotifyAttributeChanged(EnhancedCurrentHue::Id, AttributeChangeType::kReportable);
    }
    mDelegate.OnColorLoopStopped(); // hardware rotating hue on its own must stop
    PersistCurrentColor();
}

Status ColorControlCluster::ColorLoopSet(BitMask<UpdateFlagsBitmap> updateFlags, ColorLoopActionEnum action,
                                         ColorLoopDirectionEnum direction, uint16_t timeSec, uint16_t startHue,
                                         BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(action != ColorLoopActionEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(direction != ColorLoopDirectionEnum::kUnknownEnumValue, Status::InvalidCommand);
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);

    // No recognized update bit (reserved bits alone do not count): skip the NVM write and scene
    // invalidation below.
    VerifyOrReturnValue(updateFlags.HasAny(UpdateFlagsBitmap::kUpdateAction, UpdateFlagsBitmap::kUpdateDirection,
                                           UpdateFlagsBitmap::kUpdateTime, UpdateFlagsBitmap::kUpdateStartHue),
                        Status::Success);

    if (updateFlags.Has(UpdateFlagsBitmap::kUpdateDirection))
    {
        mColorLoop.direction = to_underlying(direction);
        NotifyAttributeChanged(Attributes::ColorLoopDirection::Id, AttributeChangeType::kReportable);
    }
    if (updateFlags.Has(UpdateFlagsBitmap::kUpdateTime))
    {
        mColorLoop.timeSec = timeSec;
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
            StopColorLoop();
            break;
        case ColorLoopActionEnum::kActivateFromColorLoopStartEnhancedHue:
            StartColorLoop(true);
            break;
        case ColorLoopActionEnum::kActivateFromEnhancedCurrentHue:
            StartColorLoop(false);
            break;
        default:
            return Status::InvalidCommand;
        }
    }
    // ColorLoopActive/Direction/Time are all NVM. Start/StopColorLoop already persisted the current color
    // if an action switched mode/hue.
    PersistColorLoop();
    return Status::Success;
}

// Advertise the attributes this cluster actually serves: the mandatory set, the feature-gated optionals,
// and the Fixed descriptors (primaries / white-point / color-points). A descriptor is advertised only
// when the app supplied it in StaticConfig, matching what ReadAttribute produces — an unsupplied
// descriptor is absent, not null.
CHIP_ERROR ColorControlCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const bool hs = HasFeature(Feature::kHueAndSaturation);
    const bool xy = HasFeature(Feature::kXy);
    const bool ct = HasFeature(Feature::kColorTemperature);
    const bool eh = HasFeature(Feature::kEnhancedHue);
    const bool cl = HasFeature(Feature::kColorLoop);

    // Fixed-descriptor presence: supplied table AND the specific optional engaged. Each primary gates its
    // X / Y / Intensity together; Intensity's value is still nullable via ChromaticityPoint::intensity.
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
    // Drift-compensation reports (§3.2.6.4 / §3.2.7.8): advertised only when the app opts in by supplying them.
    const bool drift = sc && sc->driftCompensation.has_value();
    const bool comp  = sc && sc->compensationText.has_value();

    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { true, ColorControl::Attributes::RemainingTime::kMetadataEntry }, // optional, but we run transitions

        { drift, ColorControl::Attributes::DriftCompensation::kMetadataEntry },
        { comp, ColorControl::Attributes::CompensationText::kMetadataEntry },

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
        // NVM attribute: the Nullable overload of StoreNativeEndianValue writes the same native-endian
        // storage format (null → sentinel) that Startup()'s Nullable load reads back.
        PersistValue(StartUpColorTemperatureMireds::Id, mCT.startUpColorTemperatureMireds);
        NotifyAttributeChanged(request.path.mAttributeId);
        return Status::Success;
    }
    case Options::Id: {
        BitMask<OptionsBitmap> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        // ExecuteIfOff is the only defined bit; reserved bits must be rejected.
        VerifyOrReturnError(value.HasOnly(OptionsBitmap::kExecuteIfOff), Status::ConstraintError);
        mState.options = value;
        NotifyAttributeChanged(request.path.mAttributeId);
        return Status::Success;
    }
    }

    return Status::UnsupportedWrite;
}

// Pure wiring: decode each command's payload from the TLV and hand the fields to the matching handler,
// which owns validation, mode-switch, options gating and the transition. The Enhanced* commands share a
// handler with their legacy twin, distinguished by isEnhanced (legacy hue is 8-bit, enhanced 16-bit).
std::optional<DataModel::ActionReturnStatus> ColorControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                chip::TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::MoveToHue::Id: {
        Commands::MoveToHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveToHue(data.hue, data.direction, data.transitionTime, /*isEnhanced=*/false, data.optionsMask,
                         data.optionsOverride);
    }
    case Commands::MoveHue::Id: {
        Commands::MoveHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveHue(data.moveMode, data.rate, /*isEnhanced=*/false, data.optionsMask, data.optionsOverride);
    }
    case Commands::StepHue::Id: {
        Commands::StepHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return StepHue(data.stepMode, data.stepSize, data.transitionTime, /*isEnhanced=*/false, data.optionsMask,
                       data.optionsOverride);
    }
    case Commands::MoveToSaturation::Id: {
        Commands::MoveToSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveToSaturation(data.saturation, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveSaturation::Id: {
        Commands::MoveSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveSaturation(data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::StepSaturation::Id: {
        Commands::StepSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return StepSaturation(data.stepMode, data.stepSize, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveToHueAndSaturation::Id: {
        Commands::MoveToHueAndSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveToHueAndSaturation(data.hue, data.saturation, data.transitionTime, /*isEnhanced=*/false, data.optionsMask,
                                      data.optionsOverride);
    }
    case Commands::MoveToColor::Id: {
        Commands::MoveToColor::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveToColor(data.colorX, data.colorY, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveColor::Id: {
        Commands::MoveColor::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveColor(data.rateX, data.rateY, data.optionsMask, data.optionsOverride);
    }
    case Commands::StepColor::Id: {
        Commands::StepColor::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return StepColor(data.stepX, data.stepY, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveToColorTemperature::Id: {
        Commands::MoveToColorTemperature::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveToColorTemp(data.colorTemperatureMireds, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveColorTemperature::Id: {
        Commands::MoveColorTemperature::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveColorTemp(data.moveMode, data.rate, data.colorTemperatureMinimumMireds, data.colorTemperatureMaximumMireds,
                             data.optionsMask, data.optionsOverride);
    }
    case Commands::StepColorTemperature::Id: {
        Commands::StepColorTemperature::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return StepColorTemp(data.stepMode, data.stepSize, data.transitionTime, data.colorTemperatureMinimumMireds,
                             data.colorTemperatureMaximumMireds, data.optionsMask, data.optionsOverride);
    }

    // ---- Enhanced* commands: same handlers, isEnhanced = true ----
    case Commands::EnhancedMoveToHue::Id: {
        Commands::EnhancedMoveToHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveToHue(data.enhancedHue, data.direction, data.transitionTime, /*isEnhanced=*/true, data.optionsMask,
                         data.optionsOverride);
    }
    case Commands::EnhancedMoveHue::Id: {
        Commands::EnhancedMoveHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveHue(data.moveMode, data.rate, /*isEnhanced=*/true, data.optionsMask, data.optionsOverride);
    }
    case Commands::EnhancedStepHue::Id: {
        Commands::EnhancedStepHue::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return StepHue(data.stepMode, data.stepSize, data.transitionTime, /*isEnhanced=*/true, data.optionsMask,
                       data.optionsOverride);
    }
    case Commands::EnhancedMoveToHueAndSaturation::Id: {
        Commands::EnhancedMoveToHueAndSaturation::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return MoveToHueAndSaturation(data.enhancedHue, data.saturation, data.transitionTime, /*isEnhanced=*/true, data.optionsMask,
                                      data.optionsOverride);
    }

    case Commands::ColorLoopSet::Id: {
        Commands::ColorLoopSet::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return ColorLoopSet(data.updateFlags, data.action, data.direction, data.time, data.startHue, data.optionsMask,
                            data.optionsOverride);
    }
    case Commands::StopMoveStep::Id: {
        Commands::StopMoveStep::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return StopMoveStep(data.optionsMask, data.optionsOverride);
    }

    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus ColorControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    // Fixed descriptor readers: a descriptor exists only if the app supplied the table (mStaticConfig) AND
    // the specific optional is engaged; otherwise the attribute is genuinely absent → UnsupportedAttribute.
    // `field` selects one std::optional<ChromaticityPoint>, `proj` picks x / y / intensity off it.
    auto point = [&](std::optional<ChromaticityPoint> StaticConfig::* field, auto proj) -> DataModel::ActionReturnStatus {
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

    // Color axes are projected inline: the active mode returns its stored value / projection, any other
    // mode goes through a single Convert* for the requested axis only. A read NEVER switches mode (that is
    // a command's job) and NEVER interpolates (OnTick already materialized the value into mColorValue).
    switch (request.path.mAttributeId)
    {
    // The framework fills the list globals (AttributeList/AcceptedCommandList/...); FeatureMap and
    // ClusterRevision are ours to serve.
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
            return encoder.Encode(e->hue8());
        }
        HueSatColor hueSat;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate.ConvertXYToHueSat(c->x, c->y, hueSat.hue, hueSat.saturation);
        }
        else
        {
            mDelegate.ConvertMiredsToHueSat(std::get<CTColor>(mColorValue).mireds, hueSat.hue, hueSat.saturation);
        }
        return encoder.Encode(hueSat.hue);
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
        HueSatColor hueSat;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate.ConvertXYToHueSat(c->x, c->y, hueSat.hue, hueSat.saturation);
        }
        else
        {
            mDelegate.ConvertMiredsToHueSat(std::get<CTColor>(mColorValue).mireds, hueSat.hue, hueSat.saturation);
        }
        return encoder.Encode(hueSat.enhancedHue());
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
        HueSatColor hueSat;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate.ConvertXYToHueSat(c->x, c->y, hueSat.hue, hueSat.saturation);
        }
        else
        {
            mDelegate.ConvertMiredsToHueSat(std::get<CTColor>(mColorValue).mireds, hueSat.hue, hueSat.saturation);
        }
        return encoder.Encode(hueSat.saturation);
    }

    case CurrentX::Id: {
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            return encoder.Encode(c->x);
        }
        XYColor xy;
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToXY(hs->hue, hs->saturation, xy.x, xy.y);
        }
        else if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToXY(e->hue8(), e->saturation, xy.x, xy.y);
        }
        else
        {
            mDelegate.ConvertMiredsToXY(std::get<CTColor>(mColorValue).mireds, xy.x, xy.y);
        }
        return encoder.Encode(xy.x);
    }

    case CurrentY::Id: {
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            return encoder.Encode(c->y);
        }
        XYColor xy;
        if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToXY(hs->hue, hs->saturation, xy.x, xy.y);
        }
        else if (auto * e = std::get_if<EnhancedHueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToXY(e->hue8(), e->saturation, xy.x, xy.y);
        }
        else
        {
            mDelegate.ConvertMiredsToXY(std::get<CTColor>(mColorValue).mireds, xy.x, xy.y);
        }
        return encoder.Encode(xy.y);
    }

    case ColorTemperatureMireds::Id: {
        if (auto * ct = std::get_if<CTColor>(&mColorValue))
        {
            return encoder.Encode(ct->mireds);
        }
        CTColor ct;
        if (auto * c = std::get_if<XYColor>(&mColorValue))
        {
            mDelegate.ConvertXYToMireds(c->x, c->y, ct.mireds);
        }
        else if (auto * hs = std::get_if<HueSatColor>(&mColorValue))
        {
            mDelegate.ConvertHueSatToMireds(hs->hue, hs->saturation, ct.mireds);
        }
        else
        {
            const auto & e = std::get<EnhancedHueSatColor>(mColorValue);
            mDelegate.ConvertHueSatToMireds(e.hue8(), e.saturation, ct.mireds);
        }
        return encoder.Encode(ct.mireds);
    }

    // ColorMode / EnhancedColorMode are DERIVED from the active variant — never stored. Enhanced HS
    // collapses to CurrentHueAndCurrentSaturation(0) in the non-enhanced ColorMode (§3.2).
    case Attributes::ColorMode::Id:
        return encoder.Encode(std::holds_alternative<XYColor>(mColorValue)       ? ColorModeEnum::kCurrentXAndCurrentY
                                  : std::holds_alternative<CTColor>(mColorValue) ? ColorModeEnum::kColorTemperatureMireds
                                                                                 : ColorModeEnum::kCurrentHueAndCurrentSaturation);
    case Attributes::EnhancedColorMode::Id:
        return encoder.Encode(GetEnhancedColorMode());

    // ---- Drift-compensation reports (§3.2.6.4 / §3.2.7.8): app-owned, read-only. Absent means not
    // reported (UnsupportedAttribute), which is NOT DriftCompensationEnum::kNone — that is a positive
    // "no compensation".
    case DriftCompensation::Id:
        VerifyOrReturnValue(mStaticConfig && mStaticConfig->driftCompensation.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(mStaticConfig->driftCompensation.value());
    case CompensationText::Id:
        VerifyOrReturnValue(mStaticConfig && mStaticConfig->compensationText.has_value(), Status::UnsupportedAttribute);
        return encoder.Encode(mStaticConfig->compensationText.value());

    // ---- plain stored state ----
    case RemainingTime::Id:
        return encoder.Encode(mState.remainingTime);
    case Options::Id:
        return encoder.Encode(mState.options);
    case Attributes::ColorCapabilities::Id:
        // ColorCapabilities mirrors the FeatureMap 1:1 (the two bitmaps share bit positions 0x1..0x10),
        // so it is derived here rather than stored.
        return encoder.Encode(BitMask<ColorControl::ColorCapabilitiesBitmap>(static_cast<uint16_t>(mFeatures.Raw())));
    case NumberOfPrimaries::Id:
        return encoder.Encode(NumberOfPrimaries());

    // ---- color loop ----
    case ColorLoopActive::Id:
        return encoder.Encode(mColorLoop.active);
    case Attributes::ColorLoopDirection::Id:
        return encoder.Encode(mColorLoop.direction);
    case ColorLoopTime::Id:
        return encoder.Encode(mColorLoop.timeSec);
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

Status ColorControlCluster::MoveColor(int16_t rateX, int16_t rateY, BitMask<OptionsBitmap> optionsMask,
                                      BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    if (rateX == 0 && rateY == 0)
    { // both zero → stop all transitions except color loop
        StopTransitionAndFreeze();
        SetQuietReportRemainingTime(0, /*isNewTransition=*/false); // stopped → RemainingTime 0, like MoveHue Stop
        return Status::Success;
    }

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentXAndCurrentY);
    auto & xy = std::get<XYColor>(mColorValue);

    const uint16_t targetX = (rateX > 0) ? kMaxCieXyValue : (rateX < 0 ? kMinCieXyValue : xy.x);
    const uint16_t targetY = (rateY > 0) ? kMaxCieXyValue : (rateY < 0 ? kMinCieXyValue : xy.y);
    const uint32_t durX = rateX ? uint32_t(std::abs(int32_t(targetX) - xy.x)) * 1000u / static_cast<uint32_t>(std::abs(rateX)) : 0;
    const uint32_t durY = rateY ? uint32_t(std::abs(int32_t(targetY) - xy.y)) * 1000u / static_cast<uint32_t>(std::abs(rateY)) : 0;

    mTransition = XYTransition{
        .startX      = xy.x,
        .targetX     = targetX,
        .durationXMs = durX,
        .startY      = xy.y,
        .targetY     = targetY,
        .durationYMs = durY,
        .startTimeMs = NowMs(),
    };
    // Fresh-command RemainingTime edge report (§3.2.7.4): the slower axis's time-to-boundary in 1/10 s
    // (an idle axis contributed 0 duration, so max() is the moving axis).
    SetQuietReportRemainingTime(RemainingTenthsFromMs(std::max(durX, durY)), /*isNewTransition=*/true);
    NotifyXYTransition(std::get<XYTransition>(mTransition));
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::StepColor(int16_t stepX, int16_t stepY, uint16_t transitionTimeDs, BitMask<OptionsBitmap> optionsMask,
                                      BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(!(stepX == 0 && stepY == 0), Status::InvalidCommand); // §3.2.8.13.4
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentXAndCurrentY);
    auto & xy = std::get<XYColor>(mColorValue);

    const uint16_t targetX    = static_cast<uint16_t>(std::clamp<int32_t>(int32_t(xy.x) + stepX, kMinCieXyValue, kMaxCieXyValue));
    const uint16_t targetY    = static_cast<uint16_t>(std::clamp<int32_t>(int32_t(xy.y) + stepY, kMinCieXyValue, kMaxCieXyValue));
    const uint32_t durationMs = transitionTimeDs * 100u;

    mTransition = XYTransition{
        .startX      = xy.x,
        .targetX     = targetX,
        .durationXMs = durationMs, // SAME duration
        .startY      = xy.y,
        .targetY     = targetY,
        .durationYMs = durationMs, // both axes arrive together
        .startTimeMs = NowMs(),
    };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    NotifyXYTransition(std::get<XYTransition>(mTransition));
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::MoveToColorTemp(uint16_t colorTemperature, uint16_t transitionTimeDs,
                                            BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(colorTemperature <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(EnhancedColorModeEnum::kColorTemperatureMireds);
    // The builder clamps the target into the physical range and arms the transition (immediate when 0).
    StartColorTemperatureTransition(CTColor{ .mireds = colorTemperature }, transitionTimeDs * 100u);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::MoveToColor(uint16_t colorX, uint16_t colorY, uint16_t transitionTimeDs,
                                        BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    // Command parameter constraint checks (targets already validated → no clamp needed below):
    VerifyOrReturnValue(colorX <= kMaxCieXyValue, Status::ConstraintError);
    VerifyOrReturnValue(colorY <= kMaxCieXyValue, Status::ConstraintError);
    VerifyOrReturnValue(transitionTimeDs <= kMaxTransitionTime, Status::ConstraintError);

    ApplyModeSwitch(EnhancedColorModeEnum::kCurrentXAndCurrentY);
    // Targets are already range-checked above; the builder arms the transition (immediate when 0).
    StartXYTransition(XYColor{ .x = colorX, .y = colorY }, transitionTimeDs * 100u);
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

void ColorControlCluster::ApplyStartUpColorTemperature()
{
    // §3.2.11.10: a concrete StartUpColorTemperatureMireds becomes ColorTemperatureMireds on power up and
    // forces color-temperature mode; null keeps the previous value.
    if (mCT.startUpColorTemperatureMireds.IsNull())
    {
        return; // leave the ColorTemperatureMireds just loaded by Startup()
    }

    const uint16_t physMin = std::max<uint16_t>(1u, mCT.colorTempPhysicalMinMireds); // divide-by-zero guard
    const uint16_t physMax = mCT.colorTempPhysicalMaxMireds;
    const uint16_t startUp = mCT.startUpColorTemperatureMireds.Value();

    // Out of physical range → treat as null (leave previous value untouched).
    if (startUp < physMin || startUp > physMax)
    {
        return;
    }

    // Switching the variant to CTColor IS the mode change — colorMode / enhancedColorMode are derived
    // from the alternative, so there is nothing else to set.
    ApplyModeSwitch(EnhancedColorModeEnum::kColorTemperatureMireds);
    std::get<CTColor>(mColorValue).mireds = startUp;

    // Store what the attributes now report, so the persisted color does not lag behind them. Startup()
    // rebuilds mColorValue from the persisted EnhancedColorMode, so leaving the pre-startup mode and axes
    // in storage would resurrect them the first time StartUpColorTemperatureMireds is written null — the
    // null case owes the PREVIOUS value, and this is it.
    PersistCurrentColor();
}

Status ColorControlCluster::MoveColorTemp(MoveModeEnum moveMode, uint16_t rate, uint16_t minFieldMireds, uint16_t maxFieldMireds,
                                          BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(minFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError); // fields: max 0xFEFF
    VerifyOrReturnValue(maxFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    // Only when both are set do they describe a window; a crossed one bounds the movement by nothing.
    VerifyOrReturnValue(minFieldMireds == 0 || maxFieldMireds == 0 || minFieldMireds <= maxFieldMireds, Status::ConstraintError);
    VerifyOrReturnValue(moveMode != MoveModeEnum::kUnknownEnumValue, Status::InvalidCommand);

    if (moveMode == MoveModeEnum::kStop)
    { // rate is ignored. CT is single-axis, so clearing the driver is the whole stop.
        if (std::holds_alternative<CTTransition>(mTransition))
        {
            StopTransitionAndFreeze(); // freeze: persist the mireds the movement stopped at
        }
        SetQuietReportRemainingTime(0, /*isNewTransition=*/false); // stopped → RemainingTime 0, like MoveHue Stop
        return Status::Success;
    }
    VerifyOrReturnValue(rate != 0, Status::InvalidCommand);

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
        .startTimeMs  = NowMs(),
        .durationMs   = durationMs,
    };
    SetQuietReportRemainingTime(RemainingTenthsFromMs(durationMs), /*isNewTransition=*/true);
    NotifyCTTransition(std::get<CTTransition>(mTransition));
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

Status ColorControlCluster::StepColorTemp(StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTimeDs,
                                          uint16_t minFieldMireds, uint16_t maxFieldMireds, BitMask<OptionsBitmap> optionsMask,
                                          BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnValue(minFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(maxFieldMireds <= kMaxColorTemperatureMireds, Status::ConstraintError);
    VerifyOrReturnValue(minFieldMireds == 0 || maxFieldMireds == 0 || minFieldMireds <= maxFieldMireds, Status::ConstraintError);
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
        .startTimeMs  = NowMs(),
        .durationMs   = transitionTimeDs * 100u,
    };
    SetQuietReportRemainingTime(transitionTimeDs, /*isNewTransition=*/true);
    NotifyCTTransition(std::get<CTTransition>(mTransition));
    VerifyOrReturnValue(ArmTick() == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

void ColorControlCluster::CoupleColorTempToLevel(uint8_t currentLevel)
{
    // While Level Control's CoupleColorTempToLevel option is set and the active mode is color temperature,
    // a change in CurrentLevel SHALL affect ColorTemperatureMireds. The mapping is manufacturer specific,
    // qualified by: max CurrentLevel corresponds to CoupleColorTempToLevelMinMireds, and a low level maps
    // to a high mired value (simulating an incandescent bulb). The relationship is one-way — a change to
    // ColorTemperatureMireds SHALL NOT affect CurrentLevel.
    //
    // The option-bit gate is enforced by the Level Control cluster before it calls here, so the only
    // condition left to check is that color temperature owns the output.
    VerifyOrReturn(std::holds_alternative<CTColor>(mColorValue));

    const uint16_t tempCoupleMin = std::max<uint16_t>(1u, mCT.coupleColorTempToLevelMinMireds);
    const uint16_t tempPhysMax   = mCT.colorTempPhysicalMaxMireds;
    VerifyOrReturn(tempCoupleMin <= tempPhysMax);

    // Scale between the coupling min and the physical max. Mireds vary inversely with level, so peg the
    // extremes and interpolate the middle (u32 math avoids overflow).
    uint16_t newColorTemp;
    if (currentLevel <= kMinCurrentLevel)
    {
        newColorTemp = tempPhysMax;
    }
    else if (currentLevel >= kMaxCurrentLevel)
    {
        newColorTemp = tempCoupleMin;
    }
    else
    {
        const uint32_t u32TempPhysMax = static_cast<uint32_t>(tempPhysMax);
        const uint32_t tempDelta = ((u32TempPhysMax - tempCoupleMin) * currentLevel) / (kMaxCurrentLevel - kMinCurrentLevel + 1);
        newColorTemp             = static_cast<uint16_t>(tempPhysMax - tempDelta);
    }

    // Instantaneous coupling move (transitionTime 0) reusing the fully validated CT move path
    // (mode switch, physical-range clamp, hardware fan-out via the delegate, scene invalidation).
    MoveToColorTemp(newColorTemp, 0);
}
