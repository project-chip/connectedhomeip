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

 #include "SetpointAttributes.h"

 namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

SetpointAttributes & SetpointAttributes::Set(chip::AttributeId attribute)
{
    mValue |= static_cast<uint32_t>(1 << attribute);
    return *this;
}

SetpointAttributes & SetpointAttributes::Set(const SetpointAttributes & other)
{
    mValue |= other.mValue;
    return *this;
}

bool SetpointAttributes::Has(chip::AttributeId attribute) const
{
    return (mValue & static_cast<uint32_t>(1 << attribute)) != 0;
}

SetpointAttributes & SetpointAttributes::Clear(chip::AttributeId attribute)
{
    mValue &= ~static_cast<uint32_t>(1 << attribute);
    return *this;
}

SetpointAttributes & SetpointAttributes::ClearAll()
{
    mValue = 0;
    return *this;
}

void SetpointAttributes::Log(char const * prefix)
{
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((mValue & (1 << i)) != 0)
        {
            ChipLogProgress(Zcl, " %s %s", prefix, SetpointAttributeName(i));
        }
    }
}

char const * SetpointAttributeName(chip::AttributeId id) {
    switch (id) {
        case OccupiedHeatingSetpoint::Id:
            return "OccupiedHeatingSetpoint";
        case OccupiedCoolingSetpoint::Id:
            return "OccupiedCoolingSetpoint";
        case UnoccupiedHeatingSetpoint::Id:
            return "UnoccupiedHeatingSetpoint";
        case UnoccupiedCoolingSetpoint::Id:
            return "UnoccupiedCoolingSetpoint";
        case MinHeatSetpointLimit::Id:
            return "MinHeatSetpointLimit";
        case MaxHeatSetpointLimit::Id:
            return "MaxHeatSetpointLimit";
        case MinCoolSetpointLimit::Id:
            return "MinCoolSetpointLimit";
        case MaxCoolSetpointLimit::Id:
            return "MaxCoolSetpointLimit";
        default:
            return "Unknown";
    }
}

}
}
}
}
