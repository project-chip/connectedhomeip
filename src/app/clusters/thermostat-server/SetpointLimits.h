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

#include <algorithm>

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

/*
SetpointLimits provides a maximum and minimum value for a type of setpoint (either an absolute or optional).
*/
template <typename T>
struct SetpointLimits
{
    ~SetpointLimits() = default;

    /*
    Constructor to create a SetpointLimits from a min and max setpoint.
    */
    SetpointLimits(T min, T max) : minimum(min), maximum(max){};

    /*
    Copy constructor to create a SetpointLimits from another SetpointLimits.
    */
    SetpointLimits(const SetpointLimits<T> &)             = default;
    SetpointLimits & operator=(const SetpointLimits<T> &) = default;

    /*
    Return the minimum setpoint.
    */
    T minimum;

    /*
    Return the maximum setpoint.
    */
    T maximum;

    /*
    Get the minimum value for this setpoint limit
    */
    temperature Minimum() const { return minimum.Temperature(); };

    /*
    Get the maximum value for this setpoint limit
    */
    temperature Maximum() const { return maximum.Temperature(); };

    /*
    Check if the setpoint limits are valid
    */
    bool IsValid() const { return Minimum() <= Maximum(); }

    /*
    Check if a temperature is within the setpoint limits
    */
    bool Valid(const temperature temp) const { return (Minimum() <= temp && temp <= Maximum()); };

    /*
    Check if a setpoint is within the setpoint limits
    */
    bool Valid(const Setpoint & setpoint) const
    {
        return setpoint.HasTemperature() && (Minimum() <= setpoint.Temperature() && setpoint.Temperature() <= Maximum());
    };

    /*
    Clamp a temperature to the setpoint limits
    */
    temperature Clamp(const temperature temp) const { return std::clamp(temp, Minimum(), Maximum()); };
};

typedef SetpointLimits<AbsoluteSetpoint> AbsoluteSetpointLimits;
typedef SetpointLimits<OptionalSetpoint> UserSetpointLimits;

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
