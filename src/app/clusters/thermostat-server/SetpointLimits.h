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

#include "Setpoint.h"
#include "SetpointAttributes.h"
#include "Temperature.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class Setpoints;

template <typename T>
struct SetpointLimits
{
    virtual ~SetpointLimits()       = default;

    SetpointLimits(T minimum, T maximum) : minimum(minimum), maximum(maximum){};
    SetpointLimits(const SetpointLimits<T> & al) : minimum(al.minimum), maximum(al.maximum){};

    T minimum;
    T maximum;

    chip::app::Clusters::Thermostat::SystemModeEnum Mode() const { return minimum.Mode(); };

    virtual temperature Minimum() const { return minimum.Temperature(); };
    virtual temperature Maximum() const { return maximum.Temperature(); };

    virtual bool IsValid() const { return Minimum() <= Maximum(); }

    bool Valid(const temperature temp) const { return (Minimum() <= temp && temp <= Maximum()); };
    bool Valid(const Setpoint & setpoint) const
    {
        if (setpoint.HasTemperature())
        {
            return (Minimum() <= setpoint.Temperature() && setpoint.Temperature() <= Maximum());
        }
        return false;
    };

    temperature Clamp(const temperature temp) const { return std::clamp(temp, Minimum(), Maximum()); };
};
struct UserSetpointLimits : public SetpointLimits<OptionalSetpoint>
{
    const SetpointLimits<AbsoluteSetpoint> & absoluteLimits;

    UserSetpointLimits(UserSetpointLimits & override) :
        SetpointLimits(override.minimum, override.maximum), absoluteLimits(override.absoluteLimits){};
    UserSetpointLimits(const SetpointLimits<AbsoluteSetpoint> & al, OptionalSetpoint min, OptionalSetpoint max) :
        SetpointLimits(min, max), absoluteLimits(al){};
    UserSetpointLimits(const SetpointLimits<AbsoluteSetpoint> & al, const UserSetpointLimits & override) :
        SetpointLimits(override.minimum, override.maximum), absoluteLimits(al){};

    bool IsValid() const override
    {
        if (!absoluteLimits.IsValid())
        {
            return false;
        }
        if (minimum.HasTemperature() && !absoluteLimits.Valid(minimum.Temperature()))
        {
            return false;
        }
        if (maximum.HasTemperature() && !absoluteLimits.Valid(maximum.Temperature()))
        {
            return false;
        }
        return SetpointLimits::IsValid();
    }

    temperature Minimum() const override { return minimum.HasTemperature() ? minimum.Temperature() : absoluteLimits.Minimum(); };
    temperature Maximum() const override { return maximum.HasTemperature() ? maximum.Temperature() : absoluteLimits.Maximum(); };
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
