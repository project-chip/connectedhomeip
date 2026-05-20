/*
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

#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

#include "SetpointAttributes.h"
#include "Temperature.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class Setpoints;

struct SetpointLimits
{
    virtual ~SetpointLimits()       = default;

    SetpointLimits(SystemModeEnum systemMode) : mode(systemMode){};
    SetpointLimits(SystemModeEnum systemMode, temperature minimum, temperature maximum) : mode(systemMode), minimum(minimum), maximum(maximum){};
    SetpointLimits(const SetpointLimits & al) : mode(al.mode), minimum(al.minimum), maximum(al.maximum){};

    chip::app::Clusters::Thermostat::SystemModeEnum mode;
    temperature minimum;
    temperature maximum;

    virtual temperature Minimum() const { return minimum; };
    virtual temperature Maximum() const { return maximum; };

    virtual bool IsValid() { return Minimum() <= Maximum(); }

    bool Valid(chip::AttributeId id, temperature value) const {
        if (Minimum() <= value && value <= Maximum()) {
            ChipLogProgress(Zcl, "%s Limit Valid: %" PRId16 " <= %" PRId16 " <= %" PRId16, SetpointAttributeName(id), Minimum(), value, Maximum());
            return true;
        }
        ChipLogError(Zcl, "%s Limit Invalid: %" PRId16 " <= %" PRId16 " <= %" PRId16,
                        SetpointAttributeName(id), Minimum(), value, Maximum());
        return false;
    };

    temperature Clamp(temperature value) const { return std::clamp(value, Minimum(), Maximum()); };

    
};

struct EffectiveSetpointLimits : SetpointLimits
{
    const Setpoints & setpoints;

    EffectiveSetpointLimits(const Setpoints & sp, chip::app::Clusters::Thermostat::SystemModeEnum mode);

};

struct UserSetpointLimits : SetpointLimits
{
    const SetpointLimits & absoluteLimits;

    Optional<temperature> userMinimum;
    Optional<temperature> userMaximum;

    UserSetpointLimits(const SetpointLimits & al) : SetpointLimits(al.mode), absoluteLimits(al){};
    UserSetpointLimits(const SetpointLimits & al, const UserSetpointLimits & override) :
        SetpointLimits(al.mode), absoluteLimits(al), userMinimum(override.userMinimum), userMaximum(override.userMaximum){};

    temperature Minimum() const override { return userMinimum.HasValue() ? userMinimum.Value() : absoluteLimits.Minimum(); };
    temperature Maximum() const override { return userMaximum.HasValue() ? userMaximum.Value() : absoluteLimits.Maximum(); }    ;

    chip::AttributeId MinimumAttribute() {return absoluteLimits.mode == SystemModeEnum::kHeat ? Attributes::MinHeatSetpointLimit::Id : Attributes::MinCoolSetpointLimit::Id; }
    chip::AttributeId MaximumAttribute() {return absoluteLimits.mode == SystemModeEnum::kHeat ? Attributes::MaxHeatSetpointLimit::Id : Attributes::MaxCoolSetpointLimit::Id; }
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
