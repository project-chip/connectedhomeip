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
#include "SetpointLimits.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class Setpoints;

struct SetpointLimits
{
    virtual ~SetpointLimits()       = default;

    chip::app::Clusters::Thermostat::SystemModeEnum mode;
    int16_t minimum;
    int16_t maximum;

    SetpointLimits(chip::app::Clusters::Thermostat::SystemModeEnum systemMode) : mode(systemMode){};

    SetpointLimits(const SetpointLimits & al) : mode(al.mode), minimum(al.minimum), maximum(al.maximum){};

    virtual int16_t Minimum() const { return minimum; };
    virtual int16_t Maximum() const { return maximum; };

    bool Valid(int16_t value) const {
        ChipLogProgress(Zcl, "Limit Valid: %" PRId16 " <= %" PRId16 " <= %" PRId16,
                        Minimum(), value, Maximum());
        return Minimum() <= value && value <= Maximum();
    };

    int16_t Clamp(int16_t value) const { return std::clamp(value, Minimum(), Maximum()); };
};

struct EffectiveSetpointLimits : SetpointLimits
{
    const Setpoints & setpoints;

    EffectiveSetpointLimits(const Setpoints & sp, chip::app::Clusters::Thermostat::SystemModeEnum mode);
};

struct SetpointLimitOverride : SetpointLimits
{
    const SetpointLimits & absoluteLimits;

    Optional<int16_t> minimum;
    Optional<int16_t> maximum;

    SetpointLimitOverride(const SetpointLimits & al) : SetpointLimits(al.mode), absoluteLimits(al){};
    SetpointLimitOverride(const SetpointLimits & al, const SetpointLimitOverride & override) :
        SetpointLimits(al.mode), absoluteLimits(al), minimum(override.minimum), maximum(override.maximum){};

    int16_t Minimum() const override { return minimum.HasValue() ? minimum.Value() : absoluteLimits.Minimum(); };
    int16_t Maximum() const override { return maximum.HasValue() ? maximum.Value() : absoluteLimits.Maximum(); };

};


} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
