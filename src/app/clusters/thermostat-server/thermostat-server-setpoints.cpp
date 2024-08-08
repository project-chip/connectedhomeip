/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "thermostat-server-setpoints.h"

#include <app-common/zap-generated/attributes/Accessors.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;

int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    // Optional User supplied limits
    int16_t MinHeatSetpointLimit = kDefaultMinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit = kDefaultMaxHeatSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    imcode status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinHeatSetpointLimit missing using default");
    }

    status = AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxHeatSetpointLimit missing using default");
    }
    status = MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit);
    if (status != imcode::Success)
    {
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }

    status = MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit);
    if (status != imcode::Success)
    {
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacturer imposed limits

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinHeatSetpointLimit < AbsMinHeatSetpointLimit)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit > AbsMaxHeatSetpointLimit)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (HeatingSetpoint < MinHeatSetpointLimit)
        HeatingSetpoint = MinHeatSetpointLimit;

    if (HeatingSetpoint > MaxHeatSetpointLimit)
        HeatingSetpoint = MaxHeatSetpointLimit;

    return HeatingSetpoint;
}

int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
    int16_t AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    // Optional User supplied limits
    int16_t MinCoolSetpointLimit = kDefaultMinCoolSetpointLimit;
    int16_t MaxCoolSetpointLimit = kDefaultMaxCoolSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    imcode status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinCoolSetpointLimit missing using default");
    }

    status = AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != imcode::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxCoolSetpointLimit missing using default");
    }

    status = MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit);
    if (status != imcode::Success)
    {
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    status = MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit);
    if (status != imcode::Success)
    {
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacture imposed limits
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinCoolSetpointLimit < AbsMinCoolSetpointLimit)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit > AbsMaxCoolSetpointLimit)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (CoolingSetpoint < MinCoolSetpointLimit)
        CoolingSetpoint = MinCoolSetpointLimit;

    if (CoolingSetpoint > MaxCoolSetpointLimit)
        CoolingSetpoint = MaxCoolSetpointLimit;

    return CoolingSetpoint;
}
