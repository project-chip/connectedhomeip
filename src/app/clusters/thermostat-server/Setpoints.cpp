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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <clusters/Thermostat/Metadata.h>

#include "SetpointAttributes.h"
#include "SetpointLimits.h"
#include "SetpointRange.h"
#include "Setpoints.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

SetpointRange & Setpoints::GetRange(OccupancyBitmap occupancy)
{
    if (occupancy == OccupancyBitmap::kOccupied)
    {
        return occupied;
    }
    return unoccupied;
}

bool Setpoints::Valid()
{
    if (heatSupported)
    {
        if (!absoluteHeatLimits.IsValid()) {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid absolute heat limits: %" PRId16 " - %" PRId16, absoluteHeatLimits.Minimum(), absoluteHeatLimits.Maximum());
            return false;
        }
        if (!userHeatLimits.IsValid()) {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid heat limits: %" PRId16 " - %" PRId16, userHeatLimits.userMinimum.Value(), userHeatLimits.userMaximum.Value());
            return false;
        }
        if (!userHeatLimits.Valid(OccupiedHeatingSetpoint::Id, occupied.heating))
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: occupied heating setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", occupied.heating, userHeatLimits.Minimum(), userHeatLimits.Maximum());
            return false;
        }
        if (occupancySupported && !userHeatLimits.Valid(UnoccupiedHeatingSetpoint::Id, unoccupied.heating)) {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: unoccupied heating setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", unoccupied.heating, userHeatLimits.Minimum(), userHeatLimits.Maximum());
            return false;
        }
    }
    if (coolSupported)
    {
        if (!absoluteCoolLimits.IsValid()) {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid absolute cool limits: %" PRId16 " - %" PRId16, absoluteCoolLimits.Minimum(), absoluteCoolLimits.Maximum());
            return false;
        }
        if (!userCoolLimits.IsValid()) {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid cool limits: %" PRId16 " - %" PRId16, userCoolLimits.userMinimum.Value(), userCoolLimits.userMaximum.Value());
            return false;
        }
        if (!userCoolLimits.Valid(OccupiedCoolingSetpoint::Id, occupied.cooling))
        {
            ChipLogError(Zcl, "Thermostat:Setpoints::Valid: occupied cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", occupied.cooling, userCoolLimits.Minimum(), userCoolLimits.Maximum());
            return false;
        }
        if (occupancySupported && !userCoolLimits.Valid(UnoccupiedCoolingSetpoint::Id, unoccupied.cooling)) {
            ChipLogError(Zcl, "Thermostat:Setpoints::Valid: unoccupied cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", unoccupied.cooling, userCoolLimits.Minimum(), userCoolLimits.Maximum());
            return false;
        }
    }
    if (autoSupported)
    {
        if (userHeatLimits.userMaximum.HasValue() && userCoolLimits.userMaximum.HasValue() && userCoolLimits.userMaximum.Value() - userHeatLimits.userMaximum.Value() < deadBand) {
            ChipLogError(Zcl, "Thermostat:Setpoints::Valid: user maximum setpoints difference (cool= %" PRId16 " - heat= %" PRId16 " = %" PRId16 ") is smaller than min setpoint deadband %d", userCoolLimits.userMaximum.Value(), userHeatLimits.userMaximum.Value(), userCoolLimits.userMaximum.Value() - userHeatLimits.userMaximum.Value(), deadBand);
            return false;
        }
        if (userHeatLimits.userMinimum.HasValue() && userCoolLimits.userMinimum.HasValue() && userCoolLimits.userMinimum.Value() - userHeatLimits.userMinimum.Value() < deadBand) {
            ChipLogError(Zcl, "Thermostat:Setpoints::Valid: user minimum setpoints difference (cool= %" PRId16 " - heat= %" PRId16 " = %" PRId16 ") is smaller than min setpoint deadband %d", userCoolLimits.userMinimum.Value(), userHeatLimits.userMinimum.Value(), userCoolLimits.userMinimum.Value() - userHeatLimits.userMinimum.Value(), deadBand);
            return false;
        }
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Valid: occupied setpoints difference (cool= %" PRIi16 " - heat= %" PRIi16 " = %" PRIi16 "), deadband= %" PRIi16 ", valid: %s", occupied.cooling, occupied.heating, static_cast<int16_t>(occupied.cooling - occupied.heating), deadBand, ((static_cast<int16_t>(occupied.cooling - occupied.heating) >= deadBand) ? "true" : "false"));
        if (static_cast<int16_t>(occupied.cooling - occupied.heating) < deadBand)
        {
            ChipLogError(Zcl, "Thermostat:Setpoints::Valid: occupied setpoints difference (cool= %" PRIi16 " - heat= %" PRIi16 " = %" PRIi16 ") is smaller than min setpoint deadband %d", occupied.cooling, occupied.heating, static_cast<int16_t>(occupied.cooling - occupied.heating), deadBand);
            return false;
        }
        if (occupancySupported && static_cast<int16_t>(unoccupied.cooling - unoccupied.heating) < deadBand)
        {
            ChipLogError(Zcl, "Thermostat:Setpoints::Valid: unoccupied setpoints difference (cool= %" PRIi16 " - heat= %" PRIi16 " = %" PRIi16 ") is smaller than min setpoint deadband %d", unoccupied.cooling, unoccupied.heating, static_cast<int16_t>(unoccupied.cooling - unoccupied.heating), deadBand);
            return false;
        }
    }
    return true;
}

/**
 * @brief Fix user limits if they are outside the absolute limits or create an invalid deadband
 * @param absoluteLimits The absolute limits for this setpoint mode.
 * @param userLimits The user limits to fix.
 * @param minAttribute The attribute ID of the minimum setpoint.
 * @param maxAttribute The attribute ID of the maximum setpoint.
 * @param changedAttributes Bit flags for attributes that were changed prior to this call
 * @param fixedAttributes Bit flags for attributes that were fixed by this call
 * @return Whether the user limits were able to be fixed (if not, Status::ConstraintError will be returned)
 */
void Setpoints::FixUserLimits(SetpointLimits & absoluteLimits, 
    UserSetpointLimits & userLimits, 
    chip::AttributeId minAttribute,
    chip::AttributeId maxAttribute,
    SetpointAttributes & changedAttributes,
    SetpointAttributes & fixedAttributes)
{
    if (!absoluteLimits.IsValid()) {
        // Would only happen if the host fed us bad limits, as these are not user-settable
        return;
    }
    if (!absoluteLimits.Valid(minAttribute, userLimits.Minimum())) {
        ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s minimum: %" PRId16, userLimits.absoluteLimits.mode == SystemModeEnum::kHeat ? "heating" : "cooling", userLimits.userMinimum.Value());
        userLimits.userMinimum.SetValue(absoluteLimits.Minimum());
        fixedAttributes.Set(minAttribute);
    }
    if (!absoluteLimits.Valid(maxAttribute, userLimits.Maximum())) {
        ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s maximum: %" PRId16, userLimits.absoluteLimits.mode == SystemModeEnum::kHeat ? "heating" : "cooling", userLimits.userMaximum.Value());
        userLimits.userMaximum.SetValue(absoluteLimits.Maximum());
        fixedAttributes.Set(maxAttribute);
    }
    if (!userLimits.IsValid()) {
        // We somehow ended up with a user limit maximum that's less than the minimum
        if (changedAttributes.Has(minAttribute)) {
            ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s limits: %" PRId16 " - %" PRId16 ", moving maximum to %" PRId16, userLimits.absoluteLimits.mode == SystemModeEnum::kHeat ? "heating" : "cooling", userLimits.userMinimum.Value(), userLimits.userMaximum.Value(), userLimits.Minimum());
            userLimits.userMaximum.SetValue(userLimits.Minimum());
            fixedAttributes.Set(maxAttribute);
        } else if (changedAttributes.Has(maxAttribute)) {
            ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s limits: %" PRId16 " - %" PRId16 ", moving minimum to %" PRId16, userLimits.absoluteLimits.mode == SystemModeEnum::kHeat ? "heating" : "cooling", userLimits.userMinimum.Value(), userLimits.userMaximum.Value(), userLimits.Maximum());
            userLimits.userMinimum.SetValue(userLimits.Maximum());
            fixedAttributes.Set(minAttribute);
        } else {
            ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s limits on unknown attribute change: 0x%x", userLimits.absoluteLimits.mode == SystemModeEnum::kHeat ? "heating" : "cooling", changedAttributes.Raw());
        }
    }
}

/**
 * @brief Fix the user limits to maintain the deadband
 * @param heatLimit The user heat limit.
 * @param coolLimit The user cool limit.
 * @param absoluteHeatLimit The absolute heat limit.
 * @param absoluteCoolLimit The absolute cool limit.
 * @param heatAttribute The attribute ID of the heat limit.
 * @param coolAttribute The attribute ID of the cool limit.
 * @param changedAttributes Bit flags for attributes that were changed prior to this call.
 * @param fixedAttributes Bit flags for attributes that were fixed by this call.
 */
void Setpoints::FixUserLimitDeadband( 
    Optional<temperature> & heatLimit, 
    Optional<temperature> & coolLimit,
    temperature absoluteHeatLimit,
    temperature absoluteCoolLimit,
    chip::AttributeId heatLimitAttribute,
    chip::AttributeId coolLimitAttribute,
    SetpointAttributes & changedAttributes,
    SetpointAttributes & fixedAttributes)
{
    temperature effectiveHeatLimit = heatLimit.HasValue() ? heatLimit.Value() : absoluteHeatLimit;
    temperature effectiveCoolLimit = coolLimit.HasValue() ? coolLimit.Value() : absoluteCoolLimit;
    if (effectiveCoolLimit - effectiveHeatLimit >= deadBand) {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::FixUserLimitDeadband: effective heat limit (%" PRId16 ") and cool limit (%" PRId16 ") are outside deadband (%" PRId16 ")", effectiveHeatLimit, effectiveCoolLimit, deadBand);
        return;
    }
    // Our new limits violate the deadband
    if (changedAttributes.HasAny(heatLimitAttribute)) {
        // If the user was adjusting the heat limit, then we assume they 
        // want to keep it at that value, and we'll move the cool
        // limit up...
        temperature newCoolLimit = heatLimit.Value() + deadBand;
        if (absoluteCoolLimits.Valid(coolLimitAttribute, newCoolLimit)) {
            ChipLogProgress(Zcl, "Thermostat:Setpoints::FixUserLimitDeadband: increasing cool limit from %" PRId16 " to %" PRId16 " to maintain deadband %" PRId16 "", coolLimit.Value(), newCoolLimit, deadBand);
            coolLimit.SetValue(newCoolLimit);
            fixedAttributes.Set(coolLimitAttribute);
        } else {
            // ...unless that violates the absolute maximum, in which case we set the cool max to the absolute max,
            // and adjust the heat max downwards to maintain the deadband
            ChipLogProgress(Zcl, "Thermostat:Setpoints::FixUserLimitDeadband: setting cool limit to absolute max %" PRId16 ", and heat limit to %" PRId16 " to maintain deadband %" PRId16 "", absoluteCoolLimit, absoluteCoolLimit - deadBand, deadBand);
            coolLimit.SetValue(absoluteCoolLimit);
            heatLimit.SetValue(absoluteCoolLimit - deadBand);
            fixedAttributes.Set(coolLimitAttribute);
            fixedAttributes.Set(heatLimitAttribute);
        }
    } else if (changedAttributes.Has(coolLimitAttribute)) {
        // If the user was adjusting the cool limit, then we assume they 
        // want to keep it at that value, and we'll move the heat
        // limit down...
        temperature newHeatLimit = coolLimit.Value() - deadBand;
        if (absoluteHeatLimits.Valid(heatLimitAttribute, newHeatLimit)) {
            ChipLogProgress(Zcl, "Thermostat:Setpoints::FixUserLimitDeadband: decreasing heat limit from %" PRId16 " to %" PRId16 " to maintain deadband %" PRId16 "", heatLimit.Value(), newHeatLimit, deadBand);
            heatLimit.SetValue(newHeatLimit);
            fixedAttributes.Set(heatLimitAttribute);
        } else {
            // ...unless that violates the absolute maximum, in which case we set the heat max to the absolute max,
            // and adjust the cool max upwards to maintain the deadband
            ChipLogProgress(Zcl, "Thermostat:Setpoints::FixUserLimitDeadband: setting heat limit to absolute max %" PRId16 ", and cool limit to %" PRId16 " to maintain deadband %" PRId16 "", absoluteHeatLimit, absoluteHeatLimit + deadBand, deadBand);
            heatLimit.SetValue(absoluteHeatLimit);
            coolLimit.SetValue(absoluteHeatLimit + deadBand);
            fixedAttributes.Set(coolLimitAttribute);
            fixedAttributes.Set(heatLimitAttribute);
        }
    } else {
        ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimitDeadband: invalid user limits on unknown attribute change: 0x%x", changedAttributes.Raw());
    }
}

void Setpoints::FixRange(SetpointRange & range,
    chip::AttributeId heatingAttribute,
    chip::AttributeId coolingAttribute,
    SetpointAttributes & changedAttributes,
    SetpointAttributes & fixedAttributes) 
{
    if (!userHeatLimits.Valid(heatingAttribute, range.heating)) {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::FixRange: heating setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 "), clamping...", range.heating, userHeatLimits.Minimum(), userHeatLimits.Maximum());
        range.heating = userHeatLimits.Clamp(range.heating);
        fixedAttributes.Set(heatingAttribute);
    }
    if (!userCoolLimits.Valid(coolingAttribute, range.cooling)) {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::FixRange: cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 "), clamping...", range.cooling, userCoolLimits.Minimum(), userCoolLimits.Maximum());
        range.cooling = userCoolLimits.Clamp(range.cooling);
        fixedAttributes.Set(coolingAttribute);
    }
    ChipLogProgress(Zcl, "Thermostat:Setpoints::FixRange: autoSupported: %s, min deadband is %" PRId16 ", actual deadband is %" PRId16 "", autoSupported ? "true" : "false", deadBand, (range.cooling - range.heating));
    if (!autoSupported || static_cast<int16_t>(range.cooling - range.heating) >= deadBand) { 
        return;
    }
    // Our new setpoints violate the deadband
    if (changedAttributes.HasAny(heatingAttribute, Attributes::MinHeatSetpointLimit::Id, Attributes::MaxHeatSetpointLimit::Id)) {
        // If the user was adjusting a heating setpoint, then we assume they 
        // want to keep it at that value, and we'll move the cooling
        // setpoint up...
        temperature newCoolLimit = range.heating + deadBand;
        if (userCoolLimits.Valid(coolingAttribute, newCoolLimit)) {
            range.cooling = newCoolLimit;
            fixedAttributes.Set(coolingAttribute);
        } else {
            // ...unless that violates the cooling limit maximum, in which case we set the cooling setpoint to the limit max,
            // and adjust the heating setpoint downwards to maintain the deadband
            range.cooling = userCoolLimits.Maximum();
            range.heating = range.cooling - deadBand;
            fixedAttributes.Set(coolingAttribute);
            fixedAttributes.Set(heatingAttribute);
        }
    } else if (changedAttributes.HasAny(coolingAttribute, Attributes::MinCoolSetpointLimit::Id, Attributes::MaxCoolSetpointLimit::Id)) {
        // If the user was adjusting the cooling setpoint, then we assume they 
        // want to keep it at that value, and we'll move the heating
        // setpoint down...
        temperature newHeatLimit = range.cooling - deadBand;
        if (userHeatLimits.Valid(heatingAttribute, newHeatLimit)) {
            range.heating = newHeatLimit;
            fixedAttributes.Set(heatingAttribute);
        } else {
            // ...unless that violates the heating limit minimum, in which case we set the heating setpoint to the limit min,
            // and adjust the cooling setpoint upwards to maintain the deadband
            range.heating = userHeatLimits.Minimum();
            range.cooling = range.heating + deadBand;
            fixedAttributes.Set(coolingAttribute);
            fixedAttributes.Set(heatingAttribute);
        }
    } else {
        ChipLogError(Zcl, "Thermostat:Setpoints::FixRange: no setpoint attribute changed: 0x%x", changedAttributes.Raw());
    }
}

/**
 * @brief Attempt to fix any violations in the setpoints
 * @param changedAttributes Bit flags for attributes that were changed prior to this call
 * @return Status::Success if we were able to fix all violations, otherwise an error status code (e.g. Status::ConstraintError)
 */
Status Setpoints::Fix(SetpointAttributes & changedAttributes)
{
    if (Valid()) {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: setpoints are already valid");
        return Status::Success;
    }
    SetpointAttributes fixedAttributes;
    ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing setpoint violations...");
    if (heatSupported)
    {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing heat setpoint violations...");
        FixUserLimits(absoluteHeatLimits,
            userHeatLimits, 
            Attributes::MinHeatSetpointLimit::Id,
            Attributes::MaxHeatSetpointLimit::Id,
            changedAttributes,
            fixedAttributes);
    }
    if (coolSupported) {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing cool setpoint violations...");
        FixUserLimits(absoluteCoolLimits,
            userCoolLimits, 
            Attributes::MinCoolSetpointLimit::Id,
            Attributes::MaxCoolSetpointLimit::Id,
            changedAttributes,
            fixedAttributes);
    }
    if (autoSupported) {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing heat and cool deadband violations...");
        FixUserLimitDeadband(userHeatLimits.userMaximum, 
            userCoolLimits.userMaximum, 
            absoluteHeatLimits.Maximum(), 
            absoluteCoolLimits.Maximum(), 
            Attributes::MaxHeatSetpointLimit::Id, 
            Attributes::MaxCoolSetpointLimit::Id, 
            changedAttributes, 
            fixedAttributes);
        FixUserLimitDeadband(userHeatLimits.userMinimum, 
            userCoolLimits.userMinimum, 
            absoluteHeatLimits.Minimum(), 
            absoluteCoolLimits.Minimum(), 
            Attributes::MinHeatSetpointLimit::Id, 
            Attributes::MinCoolSetpointLimit::Id, 
            changedAttributes,
            fixedAttributes);
    }
    FixRange(occupied, Attributes::OccupiedHeatingSetpoint::Id, Attributes::OccupiedCoolingSetpoint::Id, changedAttributes, fixedAttributes);
    if (occupancySupported) {
        FixRange(unoccupied, Attributes::UnoccupiedHeatingSetpoint::Id, Attributes::UnoccupiedCoolingSetpoint::Id, changedAttributes, fixedAttributes);
    }
    changedAttributes.Set(fixedAttributes);
    return Valid() ? Status::Success : Status::ConstraintError;
}

Status Setpoints::ChangeRange(SetpointRange & range, chip::Optional<int16_t> heat, chip::Optional<int16_t> cool, ClampMode clamp,
                              SetpointAttributes & changedAttributes)
{
    if (!heat.HasValue() && !cool.HasValue())
    {
        return Status::InvalidValue;
    }
    ChipLogProgress(Zcl, "Thermostat: ChangeRange (%d-%d): heat: %d, cool: %d, clamp: %d", range.heating, range.cooling, heat.HasValue() ? heat.Value() : 0, cool.HasValue() ? cool.Value() : 0, to_underlying(clamp));
    if (heatSupported && heat.HasValue())
    {
        if (clamp == ClampMode::kClamp)
        {
            range.heating = userHeatLimits.Clamp(heat.Value());
        }
        else if (!userHeatLimits.Valid(range.HeatingAttribute(), heat.Value()))
        {
            ChipLogError(Zcl, "Thermostat: ChangeRange: heating setpoint %" PRId16 " is outside the allowed limits (%" PRId16 "-%" PRId16 ")", heat.Value(), userHeatLimits.Minimum(), userHeatLimits.Maximum());
            return Status::ConstraintError;
        } else {
            range.heating = heat.Value();
        }
        changedAttributes.Set(range.HeatingAttribute());
    }
    if (coolSupported && cool.HasValue())
    {
        if (clamp == ClampMode::kClamp)
        {
            range.cooling = userCoolLimits.Clamp(cool.Value());
        }
        else if (!userCoolLimits.Valid(range.CoolingAttribute(), cool.Value()))
        {
            ChipLogError(Zcl, "Thermostat: ChangeRange: cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", cool.Value(), userCoolLimits.Minimum(), userCoolLimits.Maximum());
            return Status::ConstraintError;
        } else {
            range.cooling = cool.Value();
        }
        changedAttributes.Set(range.CoolingAttribute());
    }
    return Fix(changedAttributes);
}

Status Setpoints::ChangeLimits(UserSetpointLimits & userLimits, chip::Optional<int16_t> min, chip::Optional<int16_t> max,
                               SetpointAttributes & changedAttributes)
{
    bool settingMin = min.HasValue();
    bool settingMax = max.HasValue();

    if (settingMin && settingMax) {
        return Status::InvalidCommand;
    } else if (settingMin) {
        if (!userLimits.absoluteLimits.Valid(userLimits.MinimumAttribute(), min.Value())) {
            ChipLogError(Zcl, "Thermostat: ChangeLimits: minimum setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", min.Value(), userLimits.absoluteLimits.Minimum(), userLimits.absoluteLimits.Maximum());
            return Status::ConstraintError;
        }
        userLimits.userMinimum.SetValue(min.Value());
        changedAttributes.Set(userLimits.MinimumAttribute());
    } else {
        if (!userLimits.absoluteLimits.Valid(userLimits.MaximumAttribute(), max.Value())) {
            ChipLogError(Zcl, "Thermostat: ChangeLimits: maximum setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", max.Value(), userLimits.absoluteLimits.Minimum(), userLimits.absoluteLimits.Maximum());
            return Status::ConstraintError;
        }
        userLimits.userMaximum.SetValue(max.Value());
        changedAttributes.Set(userLimits.MaximumAttribute());
    }

    return Fix(changedAttributes);
}

Status LoadSetpoints(EndpointId endpoint, Setpoints & setpoints)
{
    BitMask<Feature, uint32_t> featureMap;

    uint32_t flags;
    if (FeatureMap::Get(endpoint, &flags) == Status::Success)
    {
        featureMap.SetRaw(flags);
    }
    else
    {
        ChipLogError(Zcl, "Getsetpoints: could not get feature flags");
        featureMap.Set(Feature::kAutoMode);
        featureMap.Set(Feature::kHeating);
        featureMap.Set(Feature::kCooling);
    }

    setpoints.autoSupported      = featureMap.Has(Feature::kAutoMode);
    setpoints.heatSupported      = featureMap.Has(Feature::kHeating);
    setpoints.coolSupported      = featureMap.Has(Feature::kCooling);
    setpoints.occupancySupported = featureMap.Has(Feature::kOccupancy);
    setpoints.eventsSupported    = featureMap.Has(Feature::kEvents);

    if (setpoints.autoSupported)
    {
        int8_t deadBand;
        if (MinSetpointDeadBand::Get(endpoint, &deadBand) == Status::Success)
        {
            setpoints.deadBand = static_cast<int16_t>(deadBand * 10);
        }
        else
        {
            setpoints.deadBand = kDefaultDeadBand;
        }
    }

    if (AbsMinCoolSetpointLimit::Get(endpoint, &setpoints.absoluteCoolLimits.minimum) != Status::Success)
    {
        setpoints.absoluteCoolLimits.minimum = kDefaultAbsMinCoolSetpointLimit;
    }

    if (AbsMaxCoolSetpointLimit::Get(endpoint, &setpoints.absoluteCoolLimits.maximum) != Status::Success)
    {
        setpoints.absoluteCoolLimits.maximum = kDefaultAbsMaxCoolSetpointLimit;
    }

    int16_t minCoolSetpointLimit;
    if (MinCoolSetpointLimit::Get(endpoint, &minCoolSetpointLimit) == Status::Success)
    {
        setpoints.userCoolLimits.userMinimum.SetValue(minCoolSetpointLimit);
    }

    int16_t maxCoolSetpointLimit;
    if (MaxCoolSetpointLimit::Get(endpoint, &maxCoolSetpointLimit) == Status::Success)
    {
        setpoints.userCoolLimits.userMaximum.SetValue(maxCoolSetpointLimit);
    }

    if (AbsMinHeatSetpointLimit::Get(endpoint, &setpoints.absoluteHeatLimits.minimum) != Status::Success)
    {
        setpoints.absoluteHeatLimits.minimum = kDefaultAbsMinHeatSetpointLimit;
    }

    if (AbsMaxHeatSetpointLimit::Get(endpoint, &setpoints.absoluteHeatLimits.maximum) != Status::Success)
    {
        setpoints.absoluteHeatLimits.maximum = kDefaultAbsMaxHeatSetpointLimit;
    }

    int16_t minHeatSetpointLimit;
    if (MinHeatSetpointLimit::Get(endpoint, &minHeatSetpointLimit) == Status::Success)
    {
        setpoints.userHeatLimits.userMinimum.SetValue(minHeatSetpointLimit);
    }

    int16_t maxHeatSetpointLimit;
    if (MaxHeatSetpointLimit::Get(endpoint, &maxHeatSetpointLimit) == Status::Success)
    {
        setpoints.userHeatLimits.userMaximum.SetValue(maxHeatSetpointLimit);
    }

    if (setpoints.coolSupported)
    {
        if (OccupiedCoolingSetpoint::Get(endpoint, &setpoints.occupied.cooling) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpoints.heatSupported)
    {
        if (OccupiedHeatingSetpoint::Get(endpoint, &setpoints.occupied.heating) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return Status::Failure;
        }
    }

    if (setpoints.coolSupported && setpoints.occupancySupported)
    {
        if (UnoccupiedCoolingSetpoint::Get(endpoint, &setpoints.unoccupied.cooling) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpoints.heatSupported && setpoints.occupancySupported)
    {
        if (UnoccupiedHeatingSetpoint::Get(endpoint, &setpoints.unoccupied.heating) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return Status::Failure;
        }
    }

    ChipLogProgress(Zcl, "Thermostat: Loaded Setpoints");
    ChipLogProgress(Zcl, "Thermostat: Auto: %s Heat: %s Cool: %s Occupancy: %s Events: %s", setpoints.autoSupported ? "True" : "False", setpoints.heatSupported ? "True" : "False", setpoints.coolSupported ? "True" : "False", setpoints.occupancySupported ? "True" : "False", setpoints.eventsSupported ? "True" : "False");
    ChipLogProgress(Zcl, "Thermostat: Absolute Heat Limits: min %" PRIi16 " max %" PRIi16, setpoints.absoluteHeatLimits.minimum, setpoints.absoluteHeatLimits.maximum);
    ChipLogProgress(Zcl, "Thermostat: Absolute Cool Limits: min %" PRIi16 " max %" PRIi16, setpoints.absoluteCoolLimits.minimum, setpoints.absoluteCoolLimits.maximum);
    ChipLogProgress(Zcl, "Thermostat: User Heat Limits: min %" PRIi16 " max %" PRIi16, setpoints.userHeatLimits.userMinimum.Value(), setpoints.userHeatLimits.userMaximum.Value());
    ChipLogProgress(Zcl, "Thermostat: User Cool Limits: min %" PRIi16 " max %" PRIi16, setpoints.userCoolLimits.userMinimum.Value(), setpoints.userCoolLimits.userMaximum.Value());
    ChipLogProgress(Zcl, "Thermostat: Dead Band: %" PRIi16, setpoints.deadBand);
    ChipLogProgress(Zcl, "Thermostat: Occupied Heating Setpoint: %" PRIi16, setpoints.occupied.heating);
    ChipLogProgress(Zcl, "Thermostat: Occupied Cooling Setpoint: %" PRIi16, setpoints.occupied.cooling);
    ChipLogProgress(Zcl, "Thermostat: Unoccupied Heating Setpoint: %" PRIi16, setpoints.unoccupied.heating);
    ChipLogProgress(Zcl, "Thermostat: Unoccupied Cooling Setpoint: %" PRIi16, setpoints.unoccupied.cooling);
    return Status::Success;
}

Status SaveSetpoints(EndpointId endpoint, Setpoints & setpoints, SetpointAttributes & affectedAttributes)
{
    Status status;
    if (affectedAttributes.Has(Attributes::MinSetpointDeadBand::Id))
    {
        int8_t deadband = static_cast<int8_t>(setpoints.deadBand / 10);
        if ((status = MinSetpointDeadBand::Set(endpoint, deadband)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::MinHeatSetpointLimit::Id) && setpoints.userHeatLimits.userMinimum.HasValue())
    {
        if ((status = MinHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.userMinimum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::MaxHeatSetpointLimit::Id) && setpoints.userHeatLimits.userMaximum.HasValue())
    {
        if ((status = MaxHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.userMaximum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::MinCoolSetpointLimit::Id) && setpoints.userCoolLimits.userMinimum.HasValue())
    {
        if ((status = MinCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.userMinimum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::MaxCoolSetpointLimit::Id) && setpoints.userCoolLimits.userMaximum.HasValue())
    {
        if ((status = MaxCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.userMaximum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::OccupiedHeatingSetpoint::Id))
    {
        if ((status = OccupiedHeatingSetpoint::Set(endpoint, setpoints.occupied.heating)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::OccupiedCoolingSetpoint::Id))
    {
        if ((status = OccupiedCoolingSetpoint::Set(endpoint, setpoints.occupied.cooling)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::UnoccupiedHeatingSetpoint::Id))
    {
        if ((status = UnoccupiedHeatingSetpoint::Set(endpoint, setpoints.unoccupied.heating)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::UnoccupiedCoolingSetpoint::Id))
    {
        if ((status = UnoccupiedCoolingSetpoint::Set(endpoint, setpoints.unoccupied.cooling)) != Status::Success)
        {
            return status;
        }
    }
    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
