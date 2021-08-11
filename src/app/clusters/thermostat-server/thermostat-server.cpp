/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/util/af.h>

#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include <app/CommandHandler.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/attributes/Accessors.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>

using namespace chip;

void emberAfThermostatClusterServerInitCallback(void)
{
    // TODO
    // Get from the "real thermostat"
    // current mode
    // current occupied heating setpoint
    // current unoccupied heating setpoint
    // current occupied cooling setpoint
    // current unoccupied cooling setpoint
    // and update the zcl cluster values
    // This should be a callback defined function
    // with weak binding so that real thermostat
    // can get the values.
    // or should this just be the responsibility of the thermostat application?
}

bool emberAfThermostatClusterClearWeeklyScheduleCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj)
{
    // TODO
    return false;
}
bool emberAfThermostatClusterGetRelayStatusLogCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterGetWeeklyScheduleCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj,
                                                       uint8_t daysToReturn, uint8_t modeToReturn)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetWeeklyScheduleCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj,
                                                       uint8_t numberOfTransitionsForSequence, uint8_t daysOfWeekForSequence,
                                                       uint8_t modeForSequence, uint8_t * payload)
{
    // TODO
    return false;
}

using namespace chip::app::Clusters::Thermostat::Attributes;

int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinHeatSetpointLimit = 700;  // 7C (44.5 F) is the default
    int16_t AbsMaxHeatSetpointLimit = 3000; // 30C (86 F) is the default

    // Optional User supplied limits
    int16_t MinHeatSetpointLimit = 700;  // 7C (44.5 F) is the default
    int16_t MaxHeatSetpointLimit = 3000; // 30C (86 F) is the default

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present emberAfReadAttribute() will not update the value so the defaults are used
    EmberAfStatus status;
    bool bAcceptMin = false;
    bool bAcceptMax = false;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    status = GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        bAcceptMin = true;
    }

    status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        bAcceptMax = true;
    }

    status = GetMinHeatSetpointLimit(endpoint, &MinHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }

    status = GetMaxHeatSetpointLimit(endpoint, &MaxHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacturer imposed limits

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    if (bAcceptMin == false)
    {
        if (MinHeatSetpointLimit < AbsMinHeatSetpointLimit)
            MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }
    if (bAcceptMax == false)
    {
        if (MaxHeatSetpointLimit > AbsMaxHeatSetpointLimit)
            MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;
    }
    if (HeatingSetpoint < MinHeatSetpointLimit)
        HeatingSetpoint = MinHeatSetpointLimit;

    if (HeatingSetpoint > MaxHeatSetpointLimit)
        HeatingSetpoint = MaxHeatSetpointLimit;

    return HeatingSetpoint;
}

int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinCoolSetpointLimit = 1600; // 16C (61 F) is the default
    int16_t AbsMaxCoolSetpointLimit = 3200; // 32C (90 F) is the default

    // Optional User supplied limits
    int16_t MinCoolSetpointLimit = 1600; // 16C (61 F) is the default
    int16_t MaxCoolSetpointLimit = 3200; // 32C (90 F) is the default

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present emberAfReadAttribute() will not update the value so the defaults are used
    EmberAfStatus status;
    bool bAcceptMin = false;
    bool bAcceptMax = false;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        bAcceptMin = true;
    }

    status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        bAcceptMax = true;
    }

    status = GetMinCoolSetpointLimit(endpoint, &MinCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    status = GetMaxCoolSetpointLimit(endpoint, &MaxCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacture imposed limits
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    if (bAcceptMin == false)
    {
        if (MinCoolSetpointLimit < AbsMinCoolSetpointLimit)
            MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    if (bAcceptMax == false)
    {
        if (MaxCoolSetpointLimit > AbsMaxCoolSetpointLimit)
            MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;
    }
    if (CoolingSetpoint < MinCoolSetpointLimit)
        CoolingSetpoint = MinCoolSetpointLimit;

    if (CoolingSetpoint > MaxCoolSetpointLimit)
        CoolingSetpoint = MaxCoolSetpointLimit;

    return CoolingSetpoint;
}

bool emberAfThermostatClusterSetpointRaiseLowerCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj,
                                                        uint8_t mode, int8_t amount)
{
    int16_t HeatingSetpoint = 2000, CoolingSetpoint = 2600; // Set to defaults to be safe
    EmberAfStatus status     = EMBER_ZCL_STATUS_FAILURE;
    EmberAfStatus ReadStatus = EMBER_ZCL_STATUS_FAILURE;

    switch (mode)
    {
    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_AND_COOL_SETPOINTS: {

    https: // github.com/CHIP-Specifications/connectedhomeip-spec/issues/3726
        // Behavior is not specified if the device only contains one mode
        // and the both mode command is sent.
        // Implemented behavior is not perfect, but spec needs to be clear before changing the implementaion

        // In auto mode we will need to change both the heating and cooling setpoints
        ReadStatus = GetOccupiedCoolingSetpoint(aEndpointId, &CoolingSetpoint);

        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
            CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
            status          = SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint);
        }
        ReadStatus = GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint);

        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
            HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
            status          = SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint);
        }

        break;
    }

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_COOL_SETPOINT: {
        // In cooling mode we will need to change only the cooling setpoint
        ReadStatus = GetOccupiedCoolingSetpoint(aEndpointId, &CoolingSetpoint);

        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
            CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
            status          = SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint);
        }
        break;
    }

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_SETPOINT: {
        // In cooling mode we will need to change only the cooling setpoint
        ReadStatus = GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint);
        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
            HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
            status          = SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint);
        }
        break;
    }

    default:
        // Nothing to do here
        break;
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
