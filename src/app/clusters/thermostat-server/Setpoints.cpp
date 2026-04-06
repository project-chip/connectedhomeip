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

SetpointRange Setpoints::GetRange(OccupancyBitmap occupancy)
{
    if (occupancy == OccupancyBitmap::kOccupied)
    {
        return SetpointRange(*this, occupiedHeatingSetpoint, occupiedCoolingSetpoint, SetpointAttributes::kOccupiedHeating,
                             SetpointAttributes::kOccupiedCooling);
    }
    return SetpointRange(*this, unoccupiedHeatingSetpoint, unoccupiedCoolingSetpoint, SetpointAttributes::kUnoccupiedHeating,
                         SetpointAttributes::kUnoccupiedCooling);
}

bool Setpoints::Valid()
{
    if (heatSupported)
    {
        if (!heatLimitsOverride.Valid(occupiedHeatingSetpoint))
        {
            return false;
        }
        if (occupancySupported && !heatLimitsOverride.Valid(unoccupiedHeatingSetpoint))
        {
            return false;
        }
    }
    if (coolSupported)
    {
        if (!coolLimitsOverride.Valid(occupiedCoolingSetpoint))
        {
            return false;
        }
        if (occupancySupported && !coolLimitsOverride.Valid(unoccupiedCoolingSetpoint))
        {
            return false;
        }
    }
    if (autoSupported)
    {
        if (static_cast<int16_t>(occupiedCoolingSetpoint - occupiedHeatingSetpoint) < deadBand)
        {
            return false;
        }
        if (occupancySupported)
        {
            if (static_cast<int16_t>(unoccupiedCoolingSetpoint - unoccupiedHeatingSetpoint) < deadBand)
            {
                return false;
            }
        }
    }
    return true;
}

Status Setpoints::ChangeRange(SetpointRange range, chip::Optional<int16_t> heat, chip::Optional<int16_t> cool, ClampMode clamp,
                              chip::BitFlags<SetpointAttributes> & affectedAttributes)
{
    if (!heat.HasValue() && !cool.HasValue())
    {
        return Status::InvalidValue;
    }
    ChipLogProgress(Zcl, "Thermostat: ChangeRange: heat: %d, cool: %d, clamp: %d", heat.HasValue() ? heat.Value() : 0, cool.HasValue() ? cool.Value() : 0, to_underlying(clamp));
    auto heatLimits = EffectiveSetpointLimits(*this, SystemModeEnum::kHeat);
    auto coolLimits = EffectiveSetpointLimits(*this, SystemModeEnum::kCool);
    if (heatSupported && heat.HasValue())
    {
        if (clamp == ClampMode::kClamp)
        {
            range.heatingSetpoint = heatLimits.Clamp(heat.Value());
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
            range.coolingSetpoint = coolLimits.Clamp(cool.Value());
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
    int16_t diff = static_cast<int16_t>(range.coolingSetpoint - range.heatingSetpoint);
    if (diff >= deadBand)
    {
        return Status::Success;
    }
    if (heat.HasValue() && !cool.HasValue())
    {
        // We're only adjusting the heating setpoint, so try moving the cooling setpoint
        ChipLogProgress(Zcl, "Thermostat: ChangeRange: adjusting cooling setpoint from %" PRId16 " to %" PRId16 " to preserve deadband", range.coolingSetpoint, coolLimits.Clamp(static_cast<int16_t>(range.heatingSetpoint + deadBand)));
        range.coolingSetpoint = coolLimits.Clamp(static_cast<int16_t>(range.heatingSetpoint + deadBand));
        affectedAttributes.Set(range.coolingAttribute);

        diff = static_cast<int16_t>(range.coolingSetpoint - range.heatingSetpoint);
        if (diff > deadBand)
        {
            return Status::Success;
        }
        // We're still invalid, presumably because we were constrained by the max cooling setpoint
        // We'll shift the heating setpoint down by the amount needed to preserve the deadband
        range.heatingSetpoint = heatLimits.Clamp(static_cast<int16_t>(range.heatingSetpoint - diff));
        affectedAttributes.Set(range.heatingAttribute);
        return Status::Success;
    }
    if (cool.HasValue() && !heat.HasValue())
    {
        // We're only adjusting the cooling setpoint, so try moving the heating setpoint
        ChipLogProgress(Zcl, "Thermostat: ChangeRange: adjusting heating setpoint from %" PRId16 " to %" PRId16 " to preserve deadband", range.heatingSetpoint, heatLimits.Clamp(static_cast<int16_t>(range.coolingSetpoint - deadBand)));
        range.heatingSetpoint = heatLimits.Clamp(static_cast<int16_t>(range.coolingSetpoint - deadBand));
        affectedAttributes.Set(range.heatingAttribute);

        diff = static_cast<int16_t>(range.coolingSetpoint - range.heatingSetpoint);
        if (diff > deadBand)
        {
            return Status::Success;
        }
        // We're still invalid, presumably because we were constrained by the min heating setpoint
        // We'll shift the cooling setpoint up by the amount needed to preserve the deadband
        range.coolingSetpoint = coolLimits.Clamp(static_cast<int16_t>(range.coolingSetpoint + diff));
        affectedAttributes.Set(range.coolingAttribute);
        return Status::Success;
    }
    // We're adjusting both at the same time, so let's split the difference
    diff              = static_cast<int16_t>(deadBand - diff);
    int16_t shiftUp   = static_cast<int16_t>(diff / 2);
    int16_t shiftDown = static_cast<int16_t>(shiftUp + (diff % 2)); // If the diff is odd, shift down by an extra 0.01C.

    range.heatingSetpoint = heatLimits.Clamp(static_cast<int16_t>(range.heatingSetpoint - shiftDown));
    range.coolingSetpoint = coolLimits.Clamp(static_cast<int16_t>(range.coolingSetpoint + shiftUp));
    affectedAttributes.Set(range.heatingAttribute);
    affectedAttributes.Set(range.coolingAttribute);

    diff = static_cast<int16_t>(range.coolingSetpoint - range.heatingSetpoint);
    if (diff >= deadBand)
    {
        return Status::Success;
    }
    // We're still invalid, probably because we hit one of the limits.
    if (range.heatingSetpoint == heatLimits.minimum) // We're at the bottom of the heat limit
    {
        range.coolingSetpoint = coolLimits.Clamp(static_cast<int16_t>(range.heatingSetpoint + deadBand));
        return Status::Success;
    }
    if (range.coolingSetpoint == coolLimits.maximum)
    {
        range.heatingSetpoint = heatLimits.Clamp(static_cast<int16_t>(range.coolingSetpoint - deadBand));
        return Status::Success;
    }
    // At this point, we're invalid because it's not possible to change to the given setpoints and maintain the deadband
    ChipLogError(Zcl, "Thermostat: ChangeRange: Not possible to change to the given setpoints and maintain the deadband");
    return Status::InvalidValue;
}

Status Setpoints::ChangeLimits(SetpointLimitOverride & limitOverride, chip::Optional<int16_t> min, chip::Optional<int16_t> max,
                               chip::BitFlags<SetpointAttributes> & affectedAttributes)
{
    bool settingMin = min.HasValue();
    bool settingMax = max.HasValue();

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
        setpoints.coolLimitsOverride.minimum.SetValue(minCoolSetpointLimit);
    }

    int16_t maxCoolSetpointLimit;
    if (MaxCoolSetpointLimit::Get(endpoint, &maxCoolSetpointLimit) == Status::Success)
    {
        setpoints.coolLimitsOverride.maximum.SetValue(maxCoolSetpointLimit);
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
        setpoints.heatLimitsOverride.minimum.SetValue(minHeatSetpointLimit);
    }

    int16_t maxHeatSetpointLimit;
    if (MaxHeatSetpointLimit::Get(endpoint, &maxHeatSetpointLimit) == Status::Success)
    {
        setpoints.heatLimitsOverride.maximum.SetValue(maxHeatSetpointLimit);
    }

    if (setpoints.coolSupported)
    {
        if (OccupiedCoolingSetpoint::Get(endpoint, &setpoints.occupiedCoolingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpoints.heatSupported)
    {
        if (OccupiedHeatingSetpoint::Get(endpoint, &setpoints.occupiedHeatingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return Status::Failure;
        }
    }

    if (setpoints.coolSupported && setpoints.occupancySupported)
    {
        if (UnoccupiedCoolingSetpoint::Get(endpoint, &setpoints.unoccupiedCoolingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpoints.heatSupported && setpoints.occupancySupported)
    {
        if (UnoccupiedHeatingSetpoint::Get(endpoint, &setpoints.unoccupiedHeatingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return Status::Failure;
        }
    }
    return Status::Success;
}

Status SaveSetpoints(EndpointId endpoint, Setpoints setpoints, chip::BitFlags<SetpointAttributes> affectedAttributes)
{
    Status status;
    if (affectedAttributes.Has(SetpointAttributes::kOccupiedHeating))
    {
        if ((status = OccupiedHeatingSetpoint::Set(endpoint, setpoints.occupiedHeatingSetpoint)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kOccupiedCooling))
    {
        if ((status = OccupiedCoolingSetpoint::Set(endpoint, setpoints.occupiedCoolingSetpoint)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kUnoccupiedHeating))
    {
        if ((status = UnoccupiedHeatingSetpoint::Set(endpoint, setpoints.unoccupiedHeatingSetpoint)) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kUnoccupiedCooling))
    {
        if ((status = UnoccupiedCoolingSetpoint::Set(endpoint, setpoints.unoccupiedCoolingSetpoint)) != Status::Success)
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
    if (affectedAttributes.Has(SetpointAttributes::kMinHeatSetpointLimit) && setpoints.heatLimitsOverride.minimum.HasValue())
    {
        if ((status = MinHeatSetpointLimit::Set(endpoint, setpoints.heatLimitsOverride.minimum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMaxHeatSetpointLimit) && setpoints.heatLimitsOverride.maximum.HasValue())
    {
        if ((status = MaxHeatSetpointLimit::Set(endpoint, setpoints.heatLimitsOverride.maximum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMinCoolSetpointLimit) && setpoints.coolLimitsOverride.minimum.HasValue())
    {
        if ((status = MinCoolSetpointLimit::Set(endpoint, setpoints.coolLimitsOverride.minimum.Value())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(SetpointAttributes::kMaxCoolSetpointLimit) && setpoints.coolLimitsOverride.maximum.HasValue())
    {
        if ((status = MaxCoolSetpointLimit::Set(endpoint, setpoints.coolLimitsOverride.maximum.Value())) != Status::Success)
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
