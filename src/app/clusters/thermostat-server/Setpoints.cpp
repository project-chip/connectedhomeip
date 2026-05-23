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

SetpointRange & Setpoints::GetRange(OccupancyBitmap occupancy)
{
    if (occupancy == OccupancyBitmap::kOccupied)
    {
        return occupied;
    }
    return unoccupied;
}

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
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid absolute heat limits: %" PRId16 " - %" PRId16,
                         absoluteHeatLimits.Minimum(), absoluteHeatLimits.Maximum());
            return false;
        }
        if (!userHeatLimits.IsValid())
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid heat limits: %" PRId16 " - %" PRId16,
                         userHeatLimits.minimum.Temperature(), userHeatLimits.maximum.Temperature());
            return false;
        }
        if (!userHeatLimits.Valid(occupied.heating))
        {
            ChipLogError(Zcl,
                         "Thermostat: Setpoints::Valid: occupied heating setpoint %" PRId16
                         " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")",
                         occupied.heating.Temperature(), userHeatLimits.Minimum(), userHeatLimits.Maximum());
            return false;
        }
        if (occupancySupported && !userHeatLimits.Valid(unoccupied.heating))
        {
            ChipLogError(Zcl,
                         "Thermostat: Setpoints::Valid: unoccupied heating setpoint %" PRId16
                         " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")",
                         unoccupied.heating.Temperature(), userHeatLimits.Minimum(), userHeatLimits.Maximum());
            return false;
        }
    }
    if (coolSupported)
    {
        if (!absoluteCoolLimits.IsValid())
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid absolute cool limits: %" PRId16 " - %" PRId16,
                         absoluteCoolLimits.Minimum(), absoluteCoolLimits.Maximum());
            return false;
        }
        if (!userCoolLimits.IsValid())
        {
            ChipLogError(Zcl, "Thermostat: Setpoints::Valid: invalid cool limits: %" PRId16 " - %" PRId16,
                         userCoolLimits.minimum.Temperature(), userCoolLimits.maximum.Temperature());
            return false;
        }
        if (!userCoolLimits.Valid(occupied.cooling))
        {
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::Valid: occupied cooling setpoint %" PRId16
                         " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")",
                         occupied.cooling.Temperature(), userCoolLimits.Minimum(), userCoolLimits.Maximum());
            return false;
        }
        if (occupancySupported && !userCoolLimits.Valid(unoccupied.cooling))
        {
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::Valid: unoccupied cooling setpoint %" PRId16
                         " is outside the allowed limits ( %" PRId16 " - %" PRId16 ")",
                         unoccupied.cooling.Temperature(), userCoolLimits.Minimum(), userCoolLimits.Maximum());
            return false;
        }
    }
    if (autoSupported)
    {
        if (userHeatLimits.maximum.HasTemperature() && userCoolLimits.maximum.HasTemperature() &&
            userCoolLimits.maximum.Temperature() - userHeatLimits.maximum.Temperature() < deadBand)
        {
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::Valid: user maximum setpoints difference (cool= %" PRId16 " - heat= %" PRId16
                         " = %" PRId16 ") is smaller than min setpoint deadband %d",
                         userCoolLimits.maximum.Temperature(), userHeatLimits.maximum.Temperature(),
                         userCoolLimits.maximum.Temperature() - userHeatLimits.maximum.Temperature(), deadBand);
            return false;
        }
        if (userHeatLimits.minimum.HasTemperature() && userCoolLimits.minimum.HasTemperature() &&
            userCoolLimits.minimum.Temperature() - userHeatLimits.minimum.Temperature() < deadBand)
        {
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::Valid: user minimum setpoints difference (cool= %" PRId16 " - heat= %" PRId16
                         " = %" PRId16 ") is smaller than min setpoint deadband %d",
                         userCoolLimits.minimum.Temperature(), userHeatLimits.minimum.Temperature(),
                         userCoolLimits.minimum.Temperature() - userHeatLimits.minimum.Temperature(), deadBand);
            return false;
        }
        ChipLogProgress(Zcl,
                        "Thermostat:Setpoints::Valid: occupied setpoints difference (cool= %" PRIi16 " - heat= %" PRIi16
                        " = %" PRIi16 "), deadband= %" PRIi16 ", valid: %s",
                        occupied.cooling.Temperature(), occupied.heating.Temperature(),
                        static_cast<temperature>(occupied.cooling.Temperature() - occupied.heating.Temperature()), deadBand,
                        ((static_cast<temperature>(occupied.cooling.Temperature() - occupied.heating.Temperature()) >= deadBand)
                             ? "true"
                             : "false"));
        if (static_cast<int16_t>(occupied.cooling.Temperature() - occupied.heating.Temperature()) < deadBand)
        {
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::Valid: occupied setpoints difference (cool= %" PRIi16 " - heat= %" PRIi16
                         " = %" PRIi16 ") is smaller than min setpoint deadband %d",
                         occupied.cooling.Temperature(), occupied.heating.Temperature(),
                         static_cast<temperature>(occupied.cooling.Temperature() - occupied.heating.Temperature()), deadBand);
            return false;
        }
        if (occupancySupported &&
            static_cast<int16_t>(unoccupied.cooling.Temperature() - unoccupied.heating.Temperature()) < deadBand)
        {
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::Valid: unoccupied setpoints difference (cool= %" PRIi16 " - heat= %" PRIi16
                         " = %" PRIi16 ") is smaller than min setpoint deadband %d",
                         unoccupied.cooling.Temperature(), unoccupied.heating.Temperature(),
                         static_cast<temperature>(unoccupied.cooling.Temperature() - unoccupied.heating.Temperature()), deadBand);
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
void Setpoints::FixUserLimits(SetpointLimits<AbsoluteSetpoint> & absoluteLimits, UserSetpointLimits & userLimits,
                              SetpointAttributes & changedAttributes, SetpointAttributes & fixedAttributes)
{
    if (!absoluteLimits.IsValid())
    {
        // Would only happen if the host fed us bad limits, as these are not user-settable
        return;
    }
    if (!absoluteLimits.Valid(userLimits.minimum))
    {
        ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s minimum: %" PRId16,
                     userLimits.absoluteLimits.Mode() == SystemModeEnum::kHeat ? "heating" : "cooling",
                     userLimits.minimum.Temperature());
        if (userLimits.minimum.SetTemperature(absoluteLimits.Minimum()))
        {
            fixedAttributes.Set(userLimits.minimum.AttributeId());
        }
    }
    if (!absoluteLimits.Valid(userLimits.maximum))
    {
        ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s maximum: %" PRId16,
                     userLimits.absoluteLimits.Mode() == SystemModeEnum::kHeat ? "heating" : "cooling",
                     userLimits.maximum.Temperature());
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
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::FixUserLimits: invalid user %s limits: %" PRId16 " - %" PRId16
                         ", moving maximum to %" PRId16,
                         userLimits.absoluteLimits.Mode() == SystemModeEnum::kHeat ? "heating" : "cooling",
                         userLimits.minimum.Temperature(), userLimits.maximum.Temperature(), userLimits.Minimum());
            if (userLimits.maximum.SetTemperature(userLimits.Minimum()))
            {
                fixedAttributes.Set(userLimits.maximum.AttributeId());
            }
        }
        else if (changedAttributes.Has(userLimits.maximum.AttributeId()))
        {
            ChipLogError(Zcl,
                         "Thermostat:Setpoints::FixUserLimits: invalid user %s limits: %" PRId16 " - %" PRId16
                         ", moving minimum to %" PRId16,
                         userLimits.absoluteLimits.Mode() == SystemModeEnum::kHeat ? "heating" : "cooling",
                         userLimits.minimum.Temperature(), userLimits.maximum.Temperature(), userLimits.Maximum());
            if (userLimits.minimum.SetTemperature(userLimits.Maximum()))
            {
                fixedAttributes.Set(userLimits.minimum.AttributeId());
            }
        }
        else
        {
            ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimits: invalid user %s limits on unknown attribute change: 0x%x",
                         userLimits.absoluteLimits.Mode() == SystemModeEnum::kHeat ? "heating" : "cooling",
                         changedAttributes.Raw());
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
        ChipLogProgress(Zcl,
                        "Thermostat:Setpoints::FixUserLimitDeadband: effective heat limit (%" PRId16 ") and cool limit (%" PRId16
                        ") are outside deadband (%" PRId16 ")",
                        effectiveHeatLimit, effectiveCoolLimit, deadBand);
        return;
    }
    // Our new limits violate the deadband
    if (changedAttributes.HasAny(Attributes::MinHeatSetpointLimit::Id, Attributes::MaxHeatSetpointLimit::Id))
    {
        // If the user was adjusting the heat limit, then we assume they
        // want to keep it at that value, and we'll move the cool
        // limit up...
        temperature newCoolLimit = heatLimit.Temperature() + deadBand;
        if (absoluteCoolLimits.Valid(newCoolLimit))
        {
            ChipLogProgress(Zcl,
                            "Thermostat:Setpoints::FixUserLimitDeadband: increasing cool limit from %" PRId16 " to %" PRId16
                            " to maintain deadband %" PRId16 "",
                            coolLimit.Temperature(), newCoolLimit, deadBand);
            if (coolLimit.SetTemperature(newCoolLimit))
            {
                fixedAttributes.Set(coolLimit.AttributeId());
            }
        }
        else
        {
            // ...unless that violates the absolute maximum, in which case we set the cool max to the absolute max,
            // and adjust the heat max downwards to maintain the deadband
            ChipLogProgress(Zcl,
                            "Thermostat:Setpoints::FixUserLimitDeadband: setting cool limit to absolute max %" PRId16
                            ", and heat limit to %" PRId16 " to maintain deadband %" PRId16 "",
                            absoluteCoolLimit, absoluteCoolLimit - deadBand, deadBand);
            if (coolLimit.SetTemperature(absoluteCoolLimit))
            {
                fixedAttributes.Set(coolLimit.AttributeId());
            }
            if (heatLimit.SetTemperature(absoluteCoolLimit - deadBand))
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
        temperature newHeatLimit = coolLimit.Temperature() - deadBand;
        if (absoluteHeatLimits.Valid(newHeatLimit))
        {
            ChipLogProgress(Zcl,
                            "Thermostat:Setpoints::FixUserLimitDeadband: decreasing heat limit from %" PRId16 " to %" PRId16
                            " to maintain deadband %" PRId16 "",
                            heatLimit.Temperature(), newHeatLimit, deadBand);
            if (heatLimit.SetTemperature(newHeatLimit))
            {
                fixedAttributes.Set(heatLimit.AttributeId());
            }
        }
        else
        {
            // ...unless that violates the absolute maximum, in which case we set the heat max to the absolute max,
            // and adjust the cool max upwards to maintain the deadband
            ChipLogProgress(Zcl,
                            "Thermostat:Setpoints::FixUserLimitDeadband: setting heat limit to absolute max %" PRId16
                            ", and cool limit to %" PRId16 " to maintain deadband %" PRId16 "",
                            absoluteHeatLimit, absoluteHeatLimit + deadBand, deadBand);
            if (heatLimit.SetTemperature(absoluteHeatLimit))
            {
                fixedAttributes.Set(heatLimit.AttributeId());
            }
            if (coolLimit.SetTemperature(absoluteHeatLimit + deadBand))
            {
                fixedAttributes.Set(coolLimit.AttributeId());
            }
        }
    }
    else
    {
        ChipLogError(Zcl, "Thermostat:Setpoints::FixUserLimitDeadband: invalid user limits on unknown attribute change: 0x%x",
                     changedAttributes.Raw());
    }
}

void Setpoints::FixRange(SetpointRange & range, SetpointAttributes & changedAttributes, SetpointAttributes & fixedAttributes)
{
    if (!userHeatLimits.Valid(range.heating))
    {
        ChipLogProgress(Zcl,
                        "Thermostat:Setpoints::FixRange: heating setpoint %" PRId16 " is outside the allowed limits ( %" PRId16
                        " - %" PRId16 "), clamping...",
                        range.heating.Temperature(), userHeatLimits.Minimum(), userHeatLimits.Maximum());
        if (range.heating.SetTemperature(userHeatLimits.Clamp(range.heating.Temperature())))
        {
            fixedAttributes.Set(range.heating.AttributeId());
        }
    }
    if (!userCoolLimits.Valid(range.cooling))
    {
        ChipLogProgress(Zcl,
                        "Thermostat:Setpoints::FixRange: cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16
                        " - %" PRId16 "), clamping...",
                        range.cooling.Temperature(), userCoolLimits.Minimum(), userCoolLimits.Maximum());
        if (range.cooling.SetTemperature(userCoolLimits.Clamp(range.cooling.Temperature())))
        {
            fixedAttributes.Set(range.cooling.AttributeId());
        }
    }
    ChipLogProgress(
        Zcl, "Thermostat:Setpoints::FixRange: autoSupported: %s, min deadband is %" PRId16 ", actual deadband is %" PRId16 "",
        autoSupported ? "true" : "false", deadBand, (range.cooling.Temperature() - range.heating.Temperature()));
    if (!autoSupported || static_cast<int16_t>(range.cooling.Temperature() - range.heating.Temperature()) >= deadBand)
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
        temperature newCoolLimit = range.heating.Temperature() + deadBand;
        if (userCoolLimits.Valid(newCoolLimit))
        {
            ChipLogProgress(Zcl,
                            "Thermostat:Setpoints::FixRange: increasing cooling setpoint from %" PRId16 " to %" PRId16
                            " to maintain deadband %" PRId16 "",
                            range.cooling.Temperature(), newCoolLimit, deadBand);
            if (range.cooling.SetTemperature(newCoolLimit))
            {
                fixedAttributes.Set(range.cooling.AttributeId());
            }
        }
        else
        {
            // ...unless that violates the cooling limit maximum, in which case we set the cooling setpoint to the limit max,
            // and adjust the heating setpoint downwards to maintain the deadband
            ChipLogProgress(Zcl,
                            "Thermostat:Setpoints::FixRange: cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16
                            " - %" PRId16 "), clamping to cooling limit max to maintain deadband %" PRId16 "",
                            range.cooling.Temperature(), userCoolLimits.Minimum(), userCoolLimits.Maximum(), deadBand);
            if (range.cooling.SetTemperature(userCoolLimits.Maximum()))
            {
                fixedAttributes.Set(range.cooling.AttributeId());
            }
            if (range.heating.SetTemperature(range.cooling.Temperature() - deadBand))
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
        temperature newHeatLimit = range.cooling.Temperature() - deadBand;
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
            if (range.cooling.SetTemperature(range.heating.Temperature() + deadBand))
            {
                fixedAttributes.Set(range.cooling.AttributeId());
            }
        }
    }
    else
    {
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
    if (Valid())
    {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: setpoints are already valid");
        return Status::Success;
    }
    SetpointAttributes fixedAttributes;
    ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing setpoint violations...");
    if (heatSupported)
    {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing heat setpoint violations...");
        FixUserLimits(absoluteHeatLimits, userHeatLimits, changedAttributes, fixedAttributes);
    }
    if (coolSupported)
    {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing cool setpoint violations...");
        FixUserLimits(absoluteCoolLimits, userCoolLimits, changedAttributes, fixedAttributes);
    }
    if (autoSupported)
    {
        ChipLogProgress(Zcl, "Thermostat:Setpoints::Fix: fixing heat and cool deadband violations...");
        FixUserLimitDeadband(userHeatLimits.maximum, userCoolLimits.maximum, absoluteHeatLimits.Maximum(),
                             absoluteCoolLimits.Maximum(), changedAttributes, fixedAttributes);
        FixUserLimitDeadband(userHeatLimits.minimum, userCoolLimits.minimum, absoluteHeatLimits.Minimum(),
                             absoluteCoolLimits.Minimum(), changedAttributes, fixedAttributes);
    }
    FixRange(occupied, changedAttributes, fixedAttributes);
    if (occupancySupported)
    {
        FixRange(unoccupied, changedAttributes, fixedAttributes);
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
    ChipLogProgress(Zcl, "Thermostat: ChangeRange (%d-%d): heat: %d, cool: %d, clamp: %d", range.heating.Temperature(),
                    range.cooling.Temperature(), heat.HasValue() ? heat.Value() : 0, cool.HasValue() ? cool.Value() : 0,
                    to_underlying(clamp));
    if (heatSupported && heat.HasValue())
    {
        temperature heatVal = heat.Value();
        if (clamp == ClampMode::kClamp)
        {
            heatVal = userHeatLimits.Clamp(heatVal);
        }
        else if (!userHeatLimits.Valid(heatVal))
        {
            ChipLogError(Zcl,
                         "Thermostat: ChangeRange: heating setpoint %" PRId16 " is outside the allowed limits (%" PRId16 "-%" PRId16
                         ")",
                         heat.Value(), userHeatLimits.Minimum(), userHeatLimits.Maximum());
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
            ChipLogError(Zcl,
                         "Thermostat: ChangeRange: cooling setpoint %" PRId16 " is outside the allowed limits ( %" PRId16
                         " - %" PRId16 ")",
                         cool.Value(), userCoolLimits.Minimum(), userCoolLimits.Maximum());
            return Status::ConstraintError;
        }
        if (range.cooling.SetTemperature(coolVal))
        {
            changedAttributes.Set(range.cooling.AttributeId());
        }
    }
    return Fix(changedAttributes);
}

Status Setpoints::ChangeLimitMinimum(UserSetpointLimits & userLimits, temperature min, SetpointAttributes & changedAttributes)
{
    return ChangeLimits(userLimits, MakeOptional(min), Optional<temperature>::Missing(), changedAttributes);
}

Status Setpoints::ChangeLimitMaximum(UserSetpointLimits & userLimits, temperature max, SetpointAttributes & changedAttributes)
{
    return ChangeLimits(userLimits, Optional<temperature>::Missing(), MakeOptional(max), changedAttributes);
}

Status Setpoints::ChangeLimits(UserSetpointLimits & userLimits, Optional<temperature> min, Optional<temperature> max,
                               SetpointAttributes & changedAttributes)
{
    bool settingMin = min.HasValue();
    bool settingMax = max.HasValue();

    if (settingMin && settingMax)
    {
        return Status::InvalidCommand;
    }
    else if (settingMin)
    {
        if (!userLimits.absoluteLimits.Valid(min.Value()))
        {
            ChipLogError(Zcl,
                         "Thermostat: ChangeLimits: minimum setpoint %" PRId16 " is outside the allowed limits ( %" PRId16
                         " - %" PRId16 ")",
                         min.Value(), userLimits.absoluteLimits.Minimum(), userLimits.absoluteLimits.Maximum());
            return Status::ConstraintError;
        }
        if (userLimits.minimum.SetTemperature(min.Value()))
        {
            changedAttributes.Set(userLimits.minimum.AttributeId());
        }
    }
    else
    {
        if (!userLimits.absoluteLimits.Valid(max.Value()))
        {
            ChipLogError(Zcl,
                         "Thermostat: ChangeLimits: maximum setpoint %" PRId16 " is outside the allowed limits ( %" PRId16
                         " - %" PRId16 ")",
                         max.Value(), userLimits.absoluteLimits.Minimum(), userLimits.absoluteLimits.Maximum());
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
        setpoints.occupied.cooling.SetTemperature(occupiedCoolingSetpoint);

        if (setpoints.occupancySupported)
        {
            temperature unoccupiedCoolingSetpoint;
            if (UnoccupiedCoolingSetpoint::Get(endpoint, &unoccupiedCoolingSetpoint) != Status::Success)
            {
                // We're substituting the failure code here for backwards-compatibility reasons
                ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
                return Status::Failure;
            }
            setpoints.unoccupied.cooling.SetTemperature(unoccupiedCoolingSetpoint);
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
        setpoints.occupied.heating.SetTemperature(occupiedHeatingSetpoint);

        if (setpoints.occupancySupported)
        {
            temperature unoccupiedHeatingSetpoint;
            if (UnoccupiedHeatingSetpoint::Get(endpoint, &unoccupiedHeatingSetpoint) != Status::Success)
            {
                // We're substituting the failure code here for backwards-compatibility reasons
                ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
                return Status::Failure;
            }
            setpoints.unoccupied.heating.SetTemperature(unoccupiedHeatingSetpoint);
        }
    }

    setpoints.Log("Loaded");
    return Status::Success;
}

void Setpoints::Log(char const * prefix)
{
    ChipLogProgress(Zcl, "Thermostat: %s Auto: %s Heat: %s Cool: %s Occupancy: %s Events: %s", prefix,
                    autoSupported ? "True" : "False", heatSupported ? "True" : "False", coolSupported ? "True" : "False",
                    occupancySupported ? "True" : "False", eventsSupported ? "True" : "False");
    ChipLogProgress(Zcl, "Thermostat: %s Absolute Heat Limits: min %" PRIi16 " max %" PRIi16, prefix, absoluteHeatLimits.Minimum(),
                    absoluteHeatLimits.Maximum());
    ChipLogProgress(Zcl, "Thermostat: %s Absolute Cool Limits: min %" PRIi16 " max %" PRIi16, prefix, absoluteCoolLimits.Minimum(),
                    absoluteCoolLimits.Maximum());
    ChipLogProgress(Zcl, "Thermostat: %s User Heat Limits: min %" PRIi16 " max %" PRIi16, prefix, userHeatLimits.Minimum(),
                    userHeatLimits.Maximum());
    ChipLogProgress(Zcl, "Thermostat: %s User Cool Limits: min %" PRIi16 " max %" PRIi16, prefix, userCoolLimits.Minimum(),
                    userCoolLimits.Maximum());
    ChipLogProgress(Zcl, "Thermostat: %s Dead Band: %" PRIi16, prefix, deadBand);
    ChipLogProgress(Zcl, "Thermostat: %s Occupied Heating Setpoint: %" PRIi16, prefix, occupied.heating.Temperature());
    ChipLogProgress(Zcl, "Thermostat: %s Occupied Cooling Setpoint: %" PRIi16, prefix, occupied.cooling.Temperature());
    ChipLogProgress(Zcl, "Thermostat: %s Unoccupied Heating Setpoint: %" PRIi16, prefix, unoccupied.heating.Temperature());
    ChipLogProgress(Zcl, "Thermostat: %s Unoccupied Cooling Setpoint: %" PRIi16, prefix, unoccupied.cooling.Temperature());
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
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Dead Band: %" PRIi8, deadband);
    }
    if (affectedAttributes.Has(Attributes::MinHeatSetpointLimit::Id) && setpoints.userHeatLimits.minimum.HasTemperature())
    {
        if ((status = MinHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.minimum.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Min Heat Setpoint Limit: %" PRIi16,
                        setpoints.userHeatLimits.minimum.Temperature());
    }
    if (affectedAttributes.Has(Attributes::MaxHeatSetpointLimit::Id) && setpoints.userHeatLimits.maximum.HasTemperature())
    {
        if ((status = MaxHeatSetpointLimit::Set(endpoint, setpoints.userHeatLimits.maximum.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Max Heat Setpoint Limit: %" PRIi16,
                        setpoints.userHeatLimits.maximum.Temperature());
    }
    if (affectedAttributes.Has(Attributes::MinCoolSetpointLimit::Id) && setpoints.userCoolLimits.minimum.HasTemperature())
    {
        if ((status = MinCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.minimum.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Min Cool Setpoint Limit: %" PRIi16,
                        setpoints.userCoolLimits.minimum.Temperature());
    }
    if (affectedAttributes.Has(Attributes::MaxCoolSetpointLimit::Id) && setpoints.userCoolLimits.maximum.HasTemperature())
    {
        if ((status = MaxCoolSetpointLimit::Set(endpoint, setpoints.userCoolLimits.maximum.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Max Cool Setpoint Limit: %" PRIi16,
                        setpoints.userCoolLimits.maximum.Temperature());
    }
    if (affectedAttributes.Has(Attributes::OccupiedHeatingSetpoint::Id))
    {
        if ((status = OccupiedHeatingSetpoint::Set(endpoint, setpoints.occupied.heating.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Occupied Heating Setpoint: %" PRIi16,
                        setpoints.occupied.heating.Temperature());
    }
    if (affectedAttributes.Has(Attributes::OccupiedCoolingSetpoint::Id))
    {
        if ((status = OccupiedCoolingSetpoint::Set(endpoint, setpoints.occupied.cooling.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Occupied Cooling Setpoint: %" PRIi16,
                        setpoints.occupied.cooling.Temperature());
    }
    if (affectedAttributes.Has(Attributes::UnoccupiedHeatingSetpoint::Id))
    {
        if ((status = UnoccupiedHeatingSetpoint::Set(endpoint, setpoints.unoccupied.heating.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Unoccupied Heating Setpoint: %" PRIi16,
                        setpoints.unoccupied.heating.Temperature());
    }
    if (affectedAttributes.Has(Attributes::UnoccupiedCoolingSetpoint::Id))
    {
        if ((status = UnoccupiedCoolingSetpoint::Set(endpoint, setpoints.unoccupied.cooling.Temperature())) != Status::Success)
        {
            return status;
        }
        ChipLogProgress(Zcl, "Thermostat: SaveSetpoints: Set Unoccupied Cooling Setpoint: %" PRIi16,
                        setpoints.unoccupied.cooling.Temperature());
    }
    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
