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

#include <algorithm>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <clusters/Thermostat/Metadata.h>

#include "Setpoint.h"
#include "SetpointAttributes.h"
#include "SetpointLimits.h"
#include "SetpointRange.h"
#include "Setpoints.h"
#include "Temperature.h"

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

SetpointLimits<AbsoluteSetpoint> Setpoints::GetLimits(SystemModeEnum mode)
{
    temperature minHeat = userHeatLimits.Minimum();
    temperature maxHeat = userHeatLimits.Maximum();
    temperature minCool = userCoolLimits.Minimum();
    temperature maxCool = userCoolLimits.Maximum();

    switch (mode)
    {
    case SystemModeEnum::kHeat:
        maxHeat = autoSupported ? std::min(maxHeat, static_cast<int16_t>(maxCool - deadBand)) : maxHeat;
        return SetpointLimits<AbsoluteSetpoint>(AbsoluteSetpoint(Attributes::MinHeatSetpointLimit::Id, minHeat),
                                                AbsoluteSetpoint(Attributes::MaxHeatSetpointLimit::Id, maxHeat));
    default:
        minCool = autoSupported ? std::max(minCool, static_cast<int16_t>(minHeat + deadBand)) : minCool;
        return SetpointLimits<AbsoluteSetpoint>(AbsoluteSetpoint(Attributes::MinCoolSetpointLimit::Id, minCool),
                                                AbsoluteSetpoint(Attributes::MaxCoolSetpointLimit::Id, maxCool));
    }
}

bool Setpoints::Valid()
{
    if (heatSupported)
    {
        if (!absoluteHeatLimits.IsValid())
        {
            return false;
        }
        if (!userHeatLimits.IsValid())
        {
            return false;
        }
        if (userHeatLimits.minimum.HasTemperature() && !absoluteHeatLimits.Valid(userHeatLimits.minimum))
        {
            return false;
        }
        if (userHeatLimits.maximum.HasTemperature() && !absoluteHeatLimits.Valid(userHeatLimits.maximum))
        {
            return false;
        }
        if (!userHeatLimits.Valid(occupiedRange.heating))
        {
            return false;
        }
        if (occupancySupported && !userHeatLimits.Valid(unoccupiedRange.heating))
        {
            return false;
        }
    }
    if (coolSupported)
    {
        if (!absoluteCoolLimits.IsValid())
        {
            return false;
        }
        if (!userCoolLimits.IsValid())
        {
            return false;
        }
        if (userCoolLimits.minimum.HasTemperature() && !absoluteCoolLimits.Valid(userCoolLimits.minimum))
        {
            return false;
        }
        if (userCoolLimits.maximum.HasTemperature() && !absoluteCoolLimits.Valid(userCoolLimits.maximum))
        {
            return false;
        }
        if (!userCoolLimits.Valid(occupiedRange.cooling))
        {
            return false;
        }
        if (occupancySupported && !userCoolLimits.Valid(unoccupiedRange.cooling))
        {
            return false;
        }
    }
    if (autoSupported)
    {
        if (ViolatesDeadband(userCoolLimits.maximum, userHeatLimits.maximum))
        {
            return false;
        }
        if (ViolatesDeadband(userCoolLimits.minimum, userHeatLimits.minimum))
        {
            return false;
        }
        if (ViolatesDeadband(occupiedRange.cooling, occupiedRange.heating))
        {
            return false;
        }
        if (occupancySupported && ViolatesDeadband(unoccupiedRange.cooling, unoccupiedRange.heating))
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief Fix user limits if they are outside the absolute limits or are inverted
 * @param absoluteLimits The absolute limits for this setpoint mode.
 * @param userLimits The user limits to fix.
 * @param minAttribute The attribute ID of the minimum setpoint.
 * @param maxAttribute The attribute ID of the maximum setpoint.
 * @param changedAttributes Bit flags for attributes that were changed prior to this call
 * @param fixedAttributes Bit flags for attributes that were fixed by this call
 */
void Setpoints::FixUserLimits(AbsoluteSetpointLimits & absoluteLimits, UserSetpointLimits & userLimits,
                              SetpointAttributes & changedAttributes, SetpointAttributes & fixedAttributes)
{
    if (!absoluteLimits.IsValid())
    {
        // Would only happen if the host fed us bad limits, as these are not user-settable
        return;
    }
    if (userLimits.minimum.HasTemperature() && !absoluteLimits.Valid(userLimits.minimum))
    {
        if (userLimits.minimum.SetTemperature(absoluteLimits.Minimum()))
        {
            fixedAttributes.Set(userLimits.minimum.AttributeId());
        }
    }
    if (userLimits.maximum.HasTemperature() && !absoluteLimits.Valid(userLimits.maximum))
    {
        if (userLimits.maximum.SetTemperature(absoluteLimits.Maximum()))
        {
            fixedAttributes.Set(userLimits.maximum.AttributeId());
        }
    }
    if (!userLimits.IsValid())
    {
        // We somehow ended up with a user limit maximum that's less than the minimum
        if (changedAttributes.Has(userLimits.minimum.AttributeId()))
        {
            if (userLimits.maximum.SetTemperature(userLimits.Minimum()))
            {
                fixedAttributes.Set(userLimits.maximum.AttributeId());
            }
        }
        else if (changedAttributes.Has(userLimits.maximum.AttributeId()))
        {
            if (userLimits.minimum.SetTemperature(userLimits.Maximum()))
            {
                fixedAttributes.Set(userLimits.minimum.AttributeId());
            }
        }
    }
}

/**
 * @brief Fix the user limits to maintain the deadband
 * @param heatLimit The user heat limit.
 * @param coolLimit The user cool limit.
 * @param absoluteHeatLimit The absolute heat limit.
 * @param absoluteCoolLimit The absolute cool limit.
 * @param changedAttributes Bit flags for attributes that were changed prior to this call.
 * @param fixedAttributes Bit flags for attributes that were fixed by this call.
 */
void Setpoints::FixUserLimitDeadband(OptionalSetpoint & heatLimit, OptionalSetpoint & coolLimit, temperature absoluteHeatLimit,
                                     temperature absoluteCoolLimit, SetpointAttributes & changedAttributes,
                                     SetpointAttributes & fixedAttributes)
{
    temperature effectiveHeatLimit = heatLimit.HasTemperature() ? heatLimit.Temperature() : absoluteHeatLimit;
    temperature effectiveCoolLimit = coolLimit.HasTemperature() ? coolLimit.Temperature() : absoluteCoolLimit;
    if (effectiveCoolLimit - effectiveHeatLimit >= deadBand)
    {
        return;
    }
    // Our new limits violate the deadband
    if (changedAttributes.HasAny(Attributes::MinHeatSetpointLimit::Id, Attributes::MaxHeatSetpointLimit::Id))
    {
        // If the user was adjusting the heat limit, then we assume they
        // want to keep it at that value, and we'll move the cool
        // limit up...
        temperature newCoolLimit = static_cast<temperature>(heatLimit.Temperature() + deadBand);
        if (absoluteCoolLimits.Valid(newCoolLimit))
        {
            if (coolLimit.SetTemperature(newCoolLimit))
            {
                fixedAttributes.Set(coolLimit.AttributeId());
            }
        }
        else
        {
            // ...unless that violates the absolute maximum, in which case we set the cool max to the absolute max,
            // and adjust the heat max downwards to maintain the deadband
            if (coolLimit.SetTemperature(absoluteCoolLimit))
            {
                fixedAttributes.Set(coolLimit.AttributeId());
            }
            if (heatLimit.SetTemperature(static_cast<temperature>(absoluteCoolLimit - deadBand)))
            {
                fixedAttributes.Set(heatLimit.AttributeId());
            }
        }
    }
    else if (changedAttributes.HasAny(Attributes::MinCoolSetpointLimit::Id, Attributes::MaxCoolSetpointLimit::Id))
    {
        // If the user was adjusting the cool limit, then we assume they
        // want to keep it at that value, and we'll move the heat
        // limit down...
        temperature newHeatLimit = static_cast<temperature>(coolLimit.Temperature() - deadBand);
        if (absoluteHeatLimits.Valid(newHeatLimit))
        {
            if (heatLimit.SetTemperature(newHeatLimit))
            {
                fixedAttributes.Set(heatLimit.AttributeId());
            }
        }
        else
        {
            // ...unless that violates the absolute maximum, in which case we set the heat max to the absolute max,
            // and adjust the cool max upwards to maintain the deadband
            if (heatLimit.SetTemperature(absoluteHeatLimit))
            {
                fixedAttributes.Set(heatLimit.AttributeId());
            }
            if (coolLimit.SetTemperature(static_cast<temperature>(absoluteHeatLimit + deadBand)))
            {
                fixedAttributes.Set(coolLimit.AttributeId());
            }
        }
    }
}

void Setpoints::FixRange(SetpointRange & range, SetpointAttributes & changedAttributes, SetpointAttributes & fixedAttributes)
{
    if (!userHeatLimits.Valid(range.heating))
    {
        if (range.heating.SetTemperature(userHeatLimits.Clamp(range.heating.Temperature())))
        {
            fixedAttributes.Set(range.heating.AttributeId());
        }
    }
    if (!userCoolLimits.Valid(range.cooling))
    {
        if (range.cooling.SetTemperature(userCoolLimits.Clamp(range.cooling.Temperature())))
        {
            fixedAttributes.Set(range.cooling.AttributeId());
        }
    }
    if (!autoSupported || !ViolatesDeadband(range.cooling, range.heating))
    {
        return;
    }
    // Our new setpoints violate the deadband
    if (changedAttributes.HasAny(range.heating.AttributeId(), Attributes::MinHeatSetpointLimit::Id,
                                 Attributes::MaxHeatSetpointLimit::Id))
    {
        // If the user was adjusting a heating setpoint, then we assume they
        // want to keep it at that value, and we'll move the cooling
        // setpoint up...
        temperature newCoolLimit = static_cast<temperature>(range.heating.Temperature() + deadBand);
        if (userCoolLimits.Valid(newCoolLimit))
        {
            if (range.cooling.SetTemperature(newCoolLimit))
            {
                fixedAttributes.Set(range.cooling.AttributeId());
            }
        }
        else
        {
            // ...unless that violates the cooling limit maximum, in which case we set the cooling setpoint to the limit max,
            // and adjust the heating setpoint downwards to maintain the deadband
            if (range.cooling.SetTemperature(userCoolLimits.Maximum()))
            {
                fixedAttributes.Set(range.cooling.AttributeId());
            }
            if (range.heating.SetTemperature(static_cast<temperature>(range.cooling.Temperature() - deadBand)))
            {
                fixedAttributes.Set(range.heating.AttributeId());
            }
        }
    }
    else if (changedAttributes.HasAny(range.cooling.AttributeId(), Attributes::MinCoolSetpointLimit::Id,
                                      Attributes::MaxCoolSetpointLimit::Id))
    {
        // If the user was adjusting the cooling setpoint, then we assume they
        // want to keep it at that value, and we'll move the heating
        // setpoint down...
        temperature newHeatLimit = static_cast<temperature>(range.cooling.Temperature() - deadBand);
        if (userHeatLimits.Valid(newHeatLimit))
        {
            if (range.heating.SetTemperature(newHeatLimit))
            {
                fixedAttributes.Set(range.heating.AttributeId());
            }
        }
        else
        {
            // ...unless that violates the heating limit minimum, in which case we set the heating setpoint to the limit min,
            // and adjust the cooling setpoint upwards to maintain the deadband
            if (range.heating.SetTemperature(userHeatLimits.Minimum()))
            {
                fixedAttributes.Set(range.heating.AttributeId());
            }
            if (range.cooling.SetTemperature(static_cast<temperature>(range.heating.Temperature() + deadBand)))
            {
                fixedAttributes.Set(range.cooling.AttributeId());
            }
        }
    }
}

/**
 * @brief Attempt to fix any violations in the setpoints
 * @param changedAttributes Bit flags for attributes that were changed prior to this call
 * @return Status::Success if we were able to fix all violations, otherwise an error status code (e.g. Status::ConstraintError)
 */
Status Setpoints::Fix(SetpointAttributes & changedAttributes)
{
    if (Valid())
    {
        return Status::Success;
    }
    SetpointAttributes fixedAttributes;
    if (heatSupported)
    {
        FixUserLimits(absoluteHeatLimits, userHeatLimits, changedAttributes, fixedAttributes);
    }
    if (coolSupported)
    {
        FixUserLimits(absoluteCoolLimits, userCoolLimits, changedAttributes, fixedAttributes);
    }
    if (autoSupported)
    {
        FixUserLimitDeadband(userHeatLimits.maximum, userCoolLimits.maximum, absoluteHeatLimits.Maximum(),
                             absoluteCoolLimits.Maximum(), changedAttributes, fixedAttributes);
        FixUserLimitDeadband(userHeatLimits.minimum, userCoolLimits.minimum, absoluteHeatLimits.Minimum(),
                             absoluteCoolLimits.Minimum(), changedAttributes, fixedAttributes);
    }
    FixRange(occupiedRange, changedAttributes, fixedAttributes);
    if (occupancySupported)
    {
        FixRange(unoccupiedRange, changedAttributes, fixedAttributes);
    }
    changedAttributes.Set(fixedAttributes);
    return Valid() ? Status::Success : Status::ConstraintError;
}

Protocols::InteractionModel::Status Setpoints::ChangeRangeHeating(SetpointRange & range, temperature heat, ClampMode clamp,
                                                                  SetpointAttributes & changedAttributes)
{
    if (!heatSupported)
    {
        return Status::UnsupportedAttribute;
    }
    return ChangeRange(range, MakeOptional(heat), Optional<temperature>::Missing(), clamp, changedAttributes);
}

Protocols::InteractionModel::Status Setpoints::ChangeRangeCooling(SetpointRange & range, temperature cool, ClampMode clamp,
                                                                  SetpointAttributes & changedAttributes)
{
    if (!coolSupported)
    {
        return Status::UnsupportedAttribute;
    }
    return ChangeRange(range, Optional<temperature>::Missing(), MakeOptional(cool), clamp, changedAttributes);
}

Status Setpoints::ChangeRange(SetpointRange & range, Optional<temperature> heat, Optional<temperature> cool, ClampMode clamp,
                              SetpointAttributes & changedAttributes)
{
    if (!heat.HasValue() && !cool.HasValue())
    {
        return Status::InvalidValue;
    }
    if (heatSupported && heat.HasValue())
    {
        temperature heatVal = heat.Value();
        if (clamp == ClampMode::kClamp)
        {
            heatVal = userHeatLimits.Clamp(heatVal);
        }
        else if (!userHeatLimits.Valid(heatVal))
        {
            return Status::ConstraintError;
        }
        if (range.heating.SetTemperature(heatVal))
        {
            changedAttributes.Set(range.heating.AttributeId());
        }
    }
    if (coolSupported && cool.HasValue())
    {
        temperature coolVal = cool.Value();
        if (clamp == ClampMode::kClamp)
        {
            coolVal = userCoolLimits.Clamp(coolVal);
        }
        else if (!userCoolLimits.Valid(coolVal))
        {
            return Status::ConstraintError;
        }
        if (range.cooling.SetTemperature(coolVal))
        {
            changedAttributes.Set(range.cooling.AttributeId());
        }
    }
    return Fix(changedAttributes);
}

Status Setpoints::Setpoints::ChangeLimitMinimum(UserSetpointLimits & userLimits, AbsoluteSetpointLimits & absoluteLimits,
                                                temperature min, SetpointAttributes & changedAttributes)
{
    return ChangeLimits(userLimits, absoluteLimits, MakeOptional(min), Optional<temperature>::Missing(), changedAttributes);
}

Status Setpoints::ChangeLimitMaximum(UserSetpointLimits & userLimits, AbsoluteSetpointLimits & absoluteLimits, temperature max,
                                     SetpointAttributes & changedAttributes)
{
    return ChangeLimits(userLimits, absoluteLimits, Optional<temperature>::Missing(), MakeOptional(max), changedAttributes);
}

Status Setpoints::ChangeLimits(UserSetpointLimits & userLimits, AbsoluteSetpointLimits & absoluteLimits, Optional<temperature> min,
                               Optional<temperature> max, SetpointAttributes & changedAttributes)
{
    bool settingMin = min.HasValue();
    bool settingMax = max.HasValue();

    if (settingMin && settingMax)
    {
        return Status::InvalidCommand;
    }
    if (settingMin)
    {
        if (!absoluteLimits.Valid(min.Value()))
        {
            return Status::ConstraintError;
        }
        if (userLimits.minimum.SetTemperature(min.Value()))
        {
            changedAttributes.Set(userLimits.minimum.AttributeId());
        }
    }
    else
    {
        if (!absoluteLimits.Valid(max.Value()))
        {
            return Status::ConstraintError;
        }
        if (userLimits.maximum.SetTemperature(max.Value()))
        {
            changedAttributes.Set(userLimits.maximum.AttributeId());
        }
    }

    return Fix(changedAttributes);
}

typedef Status (*SetpointGetter)(EndpointId endpoint, temperature * value);
temperature ReadSetpointAttribute(EndpointId endpoint, SetpointGetter getter, temperature defaultValue)
{
    temperature temp;
    if (getter(endpoint, &temp) != Status::Success)
    {
        temp = defaultValue;
    }
    return temp;
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

    if (setpoints.coolSupported)
    {
        setpoints.absoluteCoolLimits.minimum.SetTemperature(
            ReadSetpointAttribute(endpoint, AbsMinCoolSetpointLimit::Get, kDefaultAbsMinCoolSetpointLimit));

        setpoints.absoluteCoolLimits.maximum.SetTemperature(
            ReadSetpointAttribute(endpoint, AbsMaxCoolSetpointLimit::Get, kDefaultAbsMaxCoolSetpointLimit));

        temperature limit;
        if (Attributes::MinCoolSetpointLimit::Get(endpoint, &limit) == Status::Success)
        {
            setpoints.userCoolLimits.minimum.SetTemperature(limit);
        }

        if (Attributes::MaxCoolSetpointLimit::Get(endpoint, &limit) == Status::Success)
        {
            setpoints.userCoolLimits.maximum.SetTemperature(limit);
        }

        temperature occupiedCoolingSetpoint;
        if (OccupiedCoolingSetpoint::Get(endpoint, &occupiedCoolingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return Status::Failure;
        }
        setpoints.occupiedRange.cooling.SetTemperature(occupiedCoolingSetpoint);

        if (setpoints.occupancySupported)
        {
            temperature unoccupiedCoolingSetpoint;
            if (UnoccupiedCoolingSetpoint::Get(endpoint, &unoccupiedCoolingSetpoint) != Status::Success)
            {
                // We're substituting the failure code here for backwards-compatibility reasons
                ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
                return Status::Failure;
            }
            setpoints.unoccupiedRange.cooling.SetTemperature(unoccupiedCoolingSetpoint);
        }
    }

    if (setpoints.heatSupported)
    {
        setpoints.absoluteHeatLimits.minimum.SetTemperature(
            ReadSetpointAttribute(endpoint, AbsMinHeatSetpointLimit::Get, kDefaultAbsMinHeatSetpointLimit));
        setpoints.absoluteHeatLimits.maximum.SetTemperature(
            ReadSetpointAttribute(endpoint, AbsMaxHeatSetpointLimit::Get, kDefaultAbsMaxHeatSetpointLimit));

        temperature limit;
        if (Attributes::MinHeatSetpointLimit::Get(endpoint, &limit) == Status::Success)
        {
            setpoints.userHeatLimits.minimum.SetTemperature(limit);
        }
        if (Attributes::MaxHeatSetpointLimit::Get(endpoint, &limit) == Status::Success)
        {
            setpoints.userHeatLimits.maximum.SetTemperature(limit);
        }

        temperature occupiedHeatingSetpoint;
        if (OccupiedHeatingSetpoint::Get(endpoint, &occupiedHeatingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return Status::Failure;
        }
        setpoints.occupiedRange.heating.SetTemperature(occupiedHeatingSetpoint);

        if (setpoints.occupancySupported)
        {
            temperature unoccupiedHeatingSetpoint;
            if (UnoccupiedHeatingSetpoint::Get(endpoint, &unoccupiedHeatingSetpoint) != Status::Success)
            {
                // We're substituting the failure code here for backwards-compatibility reasons
                ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
                return Status::Failure;
            }
            setpoints.unoccupiedRange.heating.SetTemperature(unoccupiedHeatingSetpoint);
        }
    }

    return Status::Success;
}

Status SaveFirstDirtySetpoint(EndpointId endpoint, Setpoints & setpoints, SetpointAttributes & dirtyAttributes)
{
    AttributeId firstDirty = dirtyAttributes.FirstDirtyAttribute();
    if (firstDirty == chip::kInvalidAttributeId)
    {
        return Status::Success;
    }
    SetpointAttributes firstDirtySet;
    firstDirtySet.Set(firstDirty);
    return SaveSetpoints(endpoint, setpoints, firstDirtySet);
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
    if (affectedAttributes.Has(Attributes::MinHeatSetpointLimit::Id) && setpoints.userHeatLimits.minimum.HasTemperature())
    {
        if ((status = MinHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.minimum.Temperature())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::MaxHeatSetpointLimit::Id) && setpoints.userHeatLimits.maximum.HasTemperature())
    {
        if ((status = MaxHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.maximum.Temperature())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::MinCoolSetpointLimit::Id) && setpoints.userCoolLimits.minimum.HasTemperature())
    {
        if ((status = MinCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.minimum.Temperature())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::MaxCoolSetpointLimit::Id) && setpoints.userCoolLimits.maximum.HasTemperature())
    {
        if ((status = MaxCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.maximum.Temperature())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::OccupiedHeatingSetpoint::Id))
    {
        if ((status = OccupiedHeatingSetpoint::Set(endpoint, setpoints.occupiedRange.heating.Temperature())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::OccupiedCoolingSetpoint::Id))
    {
        if ((status = OccupiedCoolingSetpoint::Set(endpoint, setpoints.occupiedRange.cooling.Temperature())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::UnoccupiedHeatingSetpoint::Id))
    {
        if ((status = UnoccupiedHeatingSetpoint::Set(endpoint, setpoints.unoccupiedRange.heating.Temperature())) != Status::Success)
        {
            return status;
        }
    }
    if (affectedAttributes.Has(Attributes::UnoccupiedCoolingSetpoint::Id))
    {
        if ((status = UnoccupiedCoolingSetpoint::Set(endpoint, setpoints.unoccupiedRange.cooling.Temperature())) != Status::Success)
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
