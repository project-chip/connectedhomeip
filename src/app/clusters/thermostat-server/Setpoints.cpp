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
        if (!userHeatLimits.Valid(occupied.heating))
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: occupied heating setpoint %" PRId16 " is outside the allowed limits", occupied.heating);
            return false;
        }
        if (occupancySupported && !userHeatLimits.Valid(unoccupied.heating)) {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: unoccupied heating setpoint %" PRId16 " is outside the allowed limits", unoccupied.heating);
            return false;
        }
    }
    if (coolSupported)
    {
        if (!userCoolLimits.Valid(occupied.cooling))
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: occupied cooling setpoint %" PRId16 " is outside the allowed limits", occupied.cooling);
            return false;
        }
        if (occupancySupported && !userCoolLimits.Valid(unoccupied.cooling)) {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: unoccupied cooling setpoint %" PRId16 " is outside the allowed limits", unoccupied.cooling);
            return false;
        }
    }
    if (autoSupported)
    {
        if (static_cast<int16_t>(occupied.cooling - occupied.heating) < deadBand)
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: occupied setpoints difference %d is smaller than min setpoint deadband %d", static_cast<int16_t>(occupied.cooling - occupied.heating), deadBand);
            return false;
        }
        if (occupancySupported && static_cast<int16_t>(unoccupied.cooling - unoccupied.heating) < deadBand)
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: unoccupied setpoints difference %d is smaller than min setpoint deadband %d", static_cast<int16_t>(unoccupied.cooling - unoccupied.heating), deadBand);
            return false;
        }
    }
    return true;
}

Status Setpoints::FixUserLimits(SetpointLimits & absoluteLimits, 
    UserSetpointLimits & userLimits, 
    SetpointAttributes minAttribute,
    SetpointAttributes maxAttribute,
    chip::BitFlags<SetpointAttributes> & changedAttributes,
    chip::BitFlags<SetpointAttributes> & fixedAttributes)
{
    if (!absoluteLimits.IsValid()) {
        // Would only happen if the host fed us bad limits, as these are not user-settable
        return Status::ConstraintError;
    }
    if (!absoluteLimits.Valid(userLimits.Minimum())) {
        userLimits.userMinimum.SetValue(absoluteLimits.Minimum());
        fixedAttributes.Set(minAttribute);
    }
    if (!absoluteLimits.Valid(userLimits.Maximum())) {
        userLimits.userMaximum.SetValue(absoluteLimits.Maximum());
        fixedAttributes.Set(maxAttribute);
    }
    if (!userLimits.IsValid()) {
        // We somehow ended up with a user limit maximum that's less than the minimum
        if (changedAttributes.Has(minAttribute)) {
            userLimits.userMaximum.SetValue(userLimits.Minimum());
            fixedAttributes.Set(maxAttribute);
        } else if (changedAttributes.Has(maxAttribute)) {
            userLimits.userMinimum.SetValue(userLimits.Maximum());
            fixedAttributes.Set(minAttribute);
        }
    }
    return userLimits.IsValid() ? Status::Success : Status::ConstraintError;
}

void Setpoints::FixUserLimitDeadband( 
    Optional<temperature> & heatLimit, 
    Optional<temperature> & coolLimit,
    temperature absoluteHeatLimit,
    temperature absoluteCoolLimit,
    SetpointAttributes heatAttribute,
    SetpointAttributes coolAttribute,
    chip::BitFlags<SetpointAttributes> & changedAttributes,
    chip::BitFlags<SetpointAttributes> & fixedAttributes)
{
    temperature effectiveHeatLimit = heatLimit.HasValue() ? heatLimit.Value() : absoluteHeatLimit;
    temperature effectiveCoolLimit = coolLimit.HasValue() ? coolLimit.Value() : absoluteCoolLimit;
    if (effectiveCoolLimit - effectiveHeatLimit >= deadBand) {
        return;
    }
    // Our new limits violate the deadband
    if (changedAttributes.Has(heatAttribute)) {
        // If the user was adjusting the heat limit, then we assume they 
        // want to keep it at that value, and we'll move the cool
        // limit up...
        temperature newCoolLimit = heatLimit.Value() + deadBand;
        if (absoluteCoolLimits.Valid(newCoolLimit)) {
            coolLimit.SetValue(newCoolLimit);
            fixedAttributes.Set(coolAttribute);
        } else {
            // ...unless that violates the absolute maximum, in which case we set the cool max to the absolute max,
            // and adjust the heat max downwards to maintain the deadband
            coolLimit.SetValue(absoluteCoolLimit);
            heatLimit.SetValue(absoluteCoolLimit - deadBand);
            fixedAttributes.Set(coolAttribute);
            fixedAttributes.Set(heatAttribute);
        }
    } else if (changedAttributes.Has(coolAttribute)) {
        // If the user was adjusting the cool limit, then we assume they 
        // want to keep it at that value, and we'll move the heat
        // limit down...
        temperature newHeatLimit = coolLimit.Value() - deadBand;
        if (absoluteHeatLimits.Valid(newHeatLimit)) {
            heatLimit.SetValue(newHeatLimit);
            fixedAttributes.Set(heatAttribute);
        } else {
            // ...unless that violates the absolute maximum, in which case we set the heat max to the absolute max,
            // and adjust the cool max upwards to maintain the deadband
            heatLimit.SetValue(absoluteHeatLimit);
            coolLimit.SetValue(absoluteHeatLimit + deadBand);
            fixedAttributes.Set(coolAttribute);
            fixedAttributes.Set(heatAttribute);
        }
    }
}

Status Setpoints::Fix(chip::BitFlags<SetpointAttributes> & changedAttributes)
{
    if (Valid()) {
        return Status::Success;
    }
    chip::BitFlags<SetpointAttributes> fixedAttributes;
    if (heatSupported)
    {
        auto status = FixUserLimits(absoluteHeatLimits,
            userHeatLimits, 
            SetpointAttributes::kMinHeatSetpointLimit,
            SetpointAttributes::kMaxHeatSetpointLimit,
            changedAttributes,
            fixedAttributes);
        if (status != Status::Success) {
            return status;
        }
    }
    if (coolSupported) {
        auto status = FixUserLimits(absoluteCoolLimits,
            userCoolLimits, 
            SetpointAttributes::kMinCoolSetpointLimit,
            SetpointAttributes::kMaxCoolSetpointLimit,
            changedAttributes,
            fixedAttributes);
        if (status != Status::Success) {
            return status;
        }
    }
    if (autoSupported) {
        FixUserLimitDeadband(userHeatLimits.userMaximum, 
            userCoolLimits.userMaximum, 
            absoluteHeatLimits.Maximum(), 
            absoluteCoolLimits.Maximum(), 
            SetpointAttributes::kMaxHeatSetpointLimit, 
            SetpointAttributes::kMaxCoolSetpointLimit, 
            changedAttributes, 
            fixedAttributes);
        FixUserLimitDeadband(userHeatLimits.userMinimum, 
            userCoolLimits.userMinimum, 
            absoluteHeatLimits.Minimum(), 
            absoluteCoolLimits.Minimum(), 
            SetpointAttributes::kMinHeatSetpointLimit, 
            SetpointAttributes::kMinCoolSetpointLimit, 
            changedAttributes,
            fixedAttributes);
    }
    changedAttributes.Set(fixedAttributes);
    return Valid() ? Status::Success : Status::ConstraintError;
}

Status Setpoints::ChangeRange(SetpointRange range, chip::Optional<int16_t> heat, chip::Optional<int16_t> cool, ClampMode clamp,
                              chip::BitFlags<SetpointAttributes> & affectedAttributes)
{
    if (!heat.HasValue() && !cool.HasValue())
    {
        return Status::InvalidValue;
    }
    ChipLogProgress(Zcl, "Thermostat: ChangeRange (%d-%d): heat: %d, cool: %d, clamp: %d", range.heating, range.cooling, heat.HasValue() ? heat.Value() : 0, cool.HasValue() ? cool.Value() : 0, to_underlying(clamp));
    auto heatLimits = EffectiveSetpointLimits(*this, SystemModeEnum::kHeat);
    auto coolLimits = EffectiveSetpointLimits(*this, SystemModeEnum::kCool);
    if (heatSupported && heat.HasValue())
    {
        if (clamp == ClampMode::kClamp)
        {
            range.heating = heatLimits.Clamp(heat.Value());
        }
        else if (!heatLimits.Valid(heat.Value()))
        {
            ChipLogError(Zcl, "Thermostat: ChangeRange: heating setpoint %" PRId16 " is outside the allowed limits (%" PRId16 "-%" PRId16 ")", heat.Value(), heatLimits.Minimum(), heatLimits.Maximum());
            return Status::ConstraintError;
        }
        affectedAttributes.Set(range.heatingAttribute);
    }
    if (coolSupported && cool.HasValue())
    {
        if (clamp == ClampMode::kClamp)
        {
            range.cooling = coolLimits.Clamp(cool.Value());
        }
        else if (!coolLimits.Valid(cool.Value()))
        {
            ChipLogError(Zcl, "Thermostat: ChangeRange: cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")", cool.Value(), coolLimits.Minimum(), coolLimits.Maximum());
            return Status::ConstraintError;
        }
        affectedAttributes.Set(range.coolingAttribute);
    }
    if (!autoSupported)
    {
        return Status::Success;
    }
    // Check if we need to shift setpoints to preserve the deadband
   /* int16_t diff = static_cast<int16_t>(range.cooling - range.heating);
    if (diff >= deadBand)
    {
        return Status::Success;
    }
    if (heat.HasValue() && !cool.HasValue())
    {
        // We're only adjusting the heating setpoint, so try moving the cooling setpoint
        ChipLogProgress(Zcl, "Thermostat: ChangeRange: adjusting cooling setpoint from %" PRId16 " to %" PRId16 " to preserve deadband", range.cooling, coolLimits.Clamp(static_cast<int16_t>(range.heating + deadBand)));
        range.cooling = coolLimits.Clamp(static_cast<int16_t>(range.heating + deadBand));
        affectedAttributes.Set(range.coolingAttribute);

        diff = static_cast<int16_t>(range.cooling - range.heating);
        if (diff > deadBand)
        {
            return Status::Success;
        }
        // We're still invalid, presumably because we were constrained by the max cooling setpoint
        // We'll shift the heating setpoint down by the amount needed to preserve the deadband
        range.heating = heatLimits.Clamp(static_cast<int16_t>(range.heating - diff));
        affectedAttributes.Set(range.heatingAttribute);
        return Status::Success;
    }
    if (cool.HasValue() && !heat.HasValue())
    {
        // We're only adjusting the cooling setpoint, so try moving the heating setpoint
        ChipLogProgress(Zcl, "Thermostat: ChangeRange: adjusting heating setpoint from %" PRId16 " to %" PRId16 " to preserve deadband", range.heating, heatLimits.Clamp(static_cast<int16_t>(range.cooling - deadBand)));
        range.heating = heatLimits.Clamp(static_cast<int16_t>(range.cooling - deadBand));
        affectedAttributes.Set(range.heatingAttribute);

        diff = static_cast<int16_t>(range.cooling - range.heating);
        if (diff > deadBand)
        {
            return Status::Success;
        }
        // We're still invalid, presumably because we were constrained by the min heating setpoint
        // We'll shift the cooling setpoint up by the amount needed to preserve the deadband
        range.cooling = coolLimits.Clamp(static_cast<int16_t>(range.cooling + diff));
        affectedAttributes.Set(range.coolingAttribute);
        return Status::Success;
    }
    // We're adjusting both at the same time, so let's split the difference
    diff              = static_cast<int16_t>(deadBand - diff);
    int16_t shiftUp   = static_cast<int16_t>(diff / 2);
    int16_t shiftDown = static_cast<int16_t>(shiftUp + (diff % 2)); // If the diff is odd, shift down by an extra 0.01C.

    range.heating = heatLimits.Clamp(static_cast<int16_t>(range.heating - shiftDown));
    range.cooling = coolLimits.Clamp(static_cast<int16_t>(range.cooling + shiftUp));
    affectedAttributes.Set(range.heatingAttribute);
    affectedAttributes.Set(range.coolingAttribute);

    diff = static_cast<int16_t>(range.cooling - range.heating);
    if (diff >= deadBand)
    {
        return Status::Success;
    }
    // We're still invalid, probably because we hit one of the limits.
    if (range.heating == heatLimits.minimum) // We're at the bottom of the heat limit
    {
        range.cooling = coolLimits.Clamp(static_cast<int16_t>(range.heating + deadBand));
        return Status::Success;
    }
    if (range.cooling == coolLimits.maximum)
    {
        range.heating = heatLimits.Clamp(static_cast<int16_t>(range.cooling - deadBand));
        return Status::Success;
    }
    // At this point, we're invalid because it's not possible to change to the given setpoints and maintain the deadband
    ChipLogError(Zcl, "Thermostat: ChangeRange: Not possible to change to the given setpoints and maintain the deadband");
    return Status::InvalidValue;
    */
    return Fix(affectedAttributes);
}

Status Setpoints::ChangeLimits(UserSetpointLimits & userLimits, chip::Optional<int16_t> min, chip::Optional<int16_t> max,
                               chip::BitFlags<SetpointAttributes> & affectedAttributes)
{
    bool settingMin = min.HasValue();
    bool settingMax = max.HasValue();

    ChipLogProgress(Zcl, "Thermostat: ChangeLimits (min: %d max: %d)", min.ValueOr(userLimits.userMinimum.Value()), max.ValueOr(userLimits.userMaximum.Value()));

    if (settingMin && settingMax) {
        return Status::InvalidCommand;
    } else if (settingMin) {
       userLimits.userMinimum.SetValue(min.Value());
       affectedAttributes.Set(userLimits.MinimumAttribute());
    } else {
        userLimits.userMaximum.SetValue(max.Value());
        affectedAttributes.Set(userLimits.MaximumAttribute());
    }

    return Fix(affectedAttributes);

    /*
    if (settingMin && settingMax)
    {
        int16_t minVal = min.Value();
        int16_t maxVal = max.Value();
        int16_t diff   = static_cast<int16_t>(maxVal - minVal);
        if (diff < 0)
        {
            // We're setting both, and the minimum is larger than the maximum
            ChipLogError(Zcl,
                         "Thermostat: ChangeLimits: Cannot set both a minimum and a maximum setpoint when the minimum is greater than the maximum");
            return Status::ConstraintError;
        }
        if (!limitOverride.absoluteLimits.Valid(minVal) || !limitOverride.absoluteLimits.Valid(maxVal))
        {
            // The limits we're trying to set are outside of the device's absolute limits
            ChipLogError(Zcl,
                         "Thermostat: ChangeLimits: Cannot set both a minimum and a maximum setpoint when the minimum or maximum is outside the absolute limits");
            return Status::ConstraintError;
        }
        if (autoSupported && diff < deadBand)
        {
            // If we set these limits, then it will be impossible to maintain the deadband
            // Let's try to shift the limits to maintain the deadband
            int16_t maxDelta = static_cast<int16_t>(diff / 2);
            int16_t minDelta = static_cast<int16_t>(diff - maxDelta);
            maxVal           = limitOverride.absoluteLimits.Clamp(static_cast<int16_t>(maxVal - maxDelta));
            minVal           = limitOverride.absoluteLimits.Clamp(static_cast<int16_t>(minVal + minDelta));
            diff             = static_cast<int16_t>(maxVal - minVal);
            if (diff < deadBand)
            {
                // Still violates the deadband; these limits are not allowed
                ChipLogError(Zcl,
                             "Thermostat: ChangeLimits: Cannot set both a minimum and a maximum setpoint when the minimum or maximum is outside the absolute limits");
                return Status::ConstraintError;
            }
        }
        limitOverride.minimum.SetValue(minVal);
        limitOverride.maximum.SetValue(maxVal);
    }
    else if (settingMin)
    {
        if (!limitOverride.absoluteLimits.Valid(min.Value()))
        {
            return Status::ConstraintError;
        }
        auto minVal = min.Value();
        auto maxVal = limitOverride.absoluteLimits.maximum;
        if (limitOverride.maximum.HasValue())
        {
            maxVal = limitOverride.maximum.Value();
        }
        auto diff = static_cast<int16_t>(maxVal - minVal);
        if (diff < 0)
        {
            // The new minimum is greater than the effective maximum
            return Status::ConstraintError;
        }
        if (autoSupported && diff < deadBand)
        {
            // If we set this limit, then it will be impossible to maintain the deadband
            // Let's try to shift the limit to maintain the deadband
            minVal = limitOverride.absoluteLimits.Clamp(static_cast<int16_t>(maxVal - deadBand));
            diff   = static_cast<int16_t>(maxVal - minVal);
            if (diff < deadBand)
            {
                // Still violates the deadband; this limit is not allowed
                ChipLogError(Zcl,
                             "Thermostat: ChangeLimits: Cannot set both a minimum and a maximum setpoint when the minimum or maximum is outside the absolute limits");
                return Status::ConstraintError;
            }
        }
        limitOverride.minimum.SetValue(minVal);
    }
    else if (settingMax)
    {
        if (!limitOverride.absoluteLimits.Valid(max.Value()))
        {
            return Status::ConstraintError;
        }
        auto maxVal = max.Value();
        auto minVal = limitOverride.absoluteLimits.minimum;
        if (limitOverride.minimum.HasValue())
        {
            minVal = limitOverride.minimum.Value();
        }
        auto diff = static_cast<int16_t>(maxVal - minVal);
        if (diff < 0)
        {
            // The new maximum is less than the effective minimum
            return Status::ConstraintError;
        }
        if (autoSupported && diff < deadBand)
        {
            // If we set this limit, then it will be impossible to maintain the deadband
            // Let's try to shift the limit to maintain the deadband
            maxVal = limitOverride.absoluteLimits.Clamp(static_cast<int16_t>(minVal + deadBand));
            diff   = static_cast<int16_t>(maxVal - minVal);
            if (diff < deadBand)
            {
                // Still violates the deadband; this limit is not allowed
                ChipLogError(Zcl,
                             "Thermostat: ChangeLimits: Cannot set both a minimum and a maximum setpoint when the minimum or maximum is outside the absolute limits");
                return Status::ConstraintError;
            }
        }
        limitOverride.maximum.SetValue(maxVal);
    }
    else
    {
        // Need to be setting at least one of these values
        ChipLogError(Zcl, "Thermostat: ChangeLimits: Need to be setting at least one of the minimum or maximum setpoint limits");
        return Status::InvalidCommand;
    }

    auto occupiedRange = GetRange(OccupancyBitmap::kOccupied);

    auto status = ChangeRange(occupiedRange, MakeOptional(occupiedRange.heatingSetpoint),
                              MakeOptional(occupiedRange.coolingSetpoint), ClampMode::kClamp, affectedAttributes);

    if (status != Status::Success)
    {
        return status;
    }

    if (occupancySupported)
    {
        auto unoccupiedRange = GetRange(OccupancyBitmap(0));
        status               = ChangeRange(unoccupiedRange, MakeOptional(unoccupiedRange.heatingSetpoint),
                                           MakeOptional(unoccupiedRange.coolingSetpoint), ClampMode::kClamp, affectedAttributes);
    }

    */

    return Status::Success;
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
    ChipLogProgress(Zcl, "Thermostat: Heat Limits: min %" PRIi16 " max %" PRIi16, setpoints.userHeatLimits.userMinimum.Value(), setpoints.userHeatLimits.userMaximum.Value());
    ChipLogProgress(Zcl, "Thermostat: Cool Limits: min %" PRIi16 " max %" PRIi16, setpoints.userCoolLimits.userMinimum.Value(), setpoints.userCoolLimits.userMaximum.Value());
    ChipLogProgress(Zcl, "Thermostat: Dead Band: %" PRIi16, setpoints.deadBand);
    ChipLogProgress(Zcl, "Thermostat: Occupied Heating Setpoint: %" PRIi16, setpoints.occupied.heating);
    ChipLogProgress(Zcl, "Thermostat: Occupied Cooling Setpoint: %" PRIi16, setpoints.occupied.cooling);
    ChipLogProgress(Zcl, "Thermostat: Unoccupied Heating Setpoint: %" PRIi16, setpoints.unoccupied.heating);
    ChipLogProgress(Zcl, "Thermostat: Unoccupied Cooling Setpoint: %" PRIi16, setpoints.unoccupied.cooling);
    return Status::Success;
}

Status SaveSetpoints(EndpointId endpoint, Setpoints setpoints, chip::BitFlags<SetpointAttributes> affectedAttributes)
{
    Status status;
    if (affectedAttributes.Has(SetpointAttributes::kOccupiedHeating))
    {
        if ((status = OccupiedHeatingSetpoint::Set(endpoint, setpoints.occupied.heating)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kOccupiedCooling))
    {
        if ((status = OccupiedCoolingSetpoint::Set(endpoint, setpoints.occupied.cooling)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kUnoccupiedHeating))
    {
        if ((status = UnoccupiedHeatingSetpoint::Set(endpoint, setpoints.unoccupied.heating)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kUnoccupiedCooling))
    {
        if ((status = UnoccupiedCoolingSetpoint::Set(endpoint, setpoints.unoccupied.cooling)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMinSetpointDeadBand))
    {
        int8_t deadband = static_cast<int8_t>(setpoints.deadBand / 10);
        if ((status = MinSetpointDeadBand::Set(endpoint, deadband)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMinHeatSetpointLimit) && setpoints.userHeatLimits.userMinimum.HasValue())
    {
        if ((status = MinHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.userMinimum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMaxHeatSetpointLimit) && setpoints.userHeatLimits.userMaximum.HasValue())
    {
        if ((status = MaxHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.userMaximum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMinCoolSetpointLimit) && setpoints.userCoolLimits.userMinimum.HasValue())
    {
        if ((status = MinCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.userMinimum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMaxCoolSetpointLimit) && setpoints.userCoolLimits.userMaximum.HasValue())
    {
        if ((status = MaxCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.userMaximum.Value())) != Status::Success)
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
