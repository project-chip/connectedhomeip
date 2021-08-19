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
#include <app/common/gen/callback.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/common/gen/ids/Attributes.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app::Clusters::Thermostat::Attributes;

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

/** @brief Thermostat Cluster Server Pre Attribute Changed
 *
 * Server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfThermostatClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                        EmberAfAttributeType attributeType, uint16_t size,
                                                                        uint8_t * value)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    int16_t Requested;

    switch (attributeId)
    {
    case Ids::OccupiedHeatingSetpoint: {
        int16_t AbsMinHeatSetpointLimit; // 7C (44.5 F) is the default
        int16_t AbsMaxHeatSetpointLimit; // 30C (86 F) is the default
        int16_t MinHeatSetpointLimit;    // 7C (44.5 F) is the default
        int16_t MaxHeatSetpointLimit;    // 30C (86 F) is the default

        status = GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinHeatSetpointLimit = 700;
        }

        status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxHeatSetpointLimit = 3000;
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

        Requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (Requested < AbsMinHeatSetpointLimit || Requested < MinHeatSetpointLimit || Requested > AbsMaxHeatSetpointLimit ||
            Requested > MaxHeatSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;

        break;
    }

    case Ids::OccupiedCoolingSetpoint: {
        int16_t AbsMinCoolSetpointLimit; // 16C (61 F) is the default
        int16_t AbsMaxCoolSetpointLimit; // 32C (90 F) is the default
        int16_t MinCoolSetpointLimit;    // 16C (61 F) is the default
        int16_t MaxCoolSetpointLimit;    // 32C (90 F) is the default

        status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinCoolSetpointLimit = 1600;
        }

        status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxCoolSetpointLimit = 3200;
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
        Requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (Requested < AbsMinCoolSetpointLimit || Requested < MinCoolSetpointLimit || Requested > AbsMaxCoolSetpointLimit ||
            Requested > MaxCoolSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;

        break;
    }

    case Ids::MinHeatSetpointLimit:
    case Ids::MaxHeatSetpointLimit: {
        int16_t AbsMinHeatSetpointLimit; // 7C (44.5 F) is the default
        int16_t AbsMaxHeatSetpointLimit; // 30C (86 F) is the default

        status = GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinHeatSetpointLimit = 700;
        }

        status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxHeatSetpointLimit = 3000;
        }

        Requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (Requested < AbsMinHeatSetpointLimit || Requested > AbsMaxHeatSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;

        break;
    }
    case Ids::MinCoolSetpointLimit:
    case Ids::MaxCoolSetpointLimit: {
        int16_t AbsMinCoolSetpointLimit; // 7C (44.5 F) is the default
        int16_t AbsMaxCoolSetpointLimit; // 30C (86 F) is the default

        status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinCoolSetpointLimit = 700;
        }

        status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxCoolSetpointLimit = 3000;
        }

        Requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (Requested < AbsMinCoolSetpointLimit || Requested > AbsMaxCoolSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;

        break;
    }

    case Ids::ControlSequenceOfOperation: {
        uint8_t RequestedCSO;
        RequestedCSO = *value;
        if (RequestedCSO > EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_AND_HEATING_WITH_REHEAT)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;
        break;
    }

    case Ids::SystemMode: {
        uint8_t ControlSequenceOfOperation = 0xff; // Invalid value
        uint8_t RequestedSystemMode        = 0xff; // Invalid value
        GetControlSequenceOfOperation(endpoint, &ControlSequenceOfOperation);
        RequestedSystemMode = *value;
        if (ControlSequenceOfOperation > EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_AND_HEATING_WITH_REHEAT ||
            RequestedSystemMode > EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_FAN_ONLY)
        {
            status = EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        else
        {
            switch (ControlSequenceOfOperation)
            {
            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_ONLY:
            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_WITH_REHEAT:
                if (RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_HEAT ||
                    RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_EMERGENCY_HEATING)
                    status = EMBER_ZCL_STATUS_INVALID_VALUE;
                break;

            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_HEATING_ONLY:
            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_HEATING_WITH_REHEAT:
                if (RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_COOL ||
                    RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_PRECOOLING)
                    status = EMBER_ZCL_STATUS_INVALID_VALUE;
                break;
            default:
                break;
            }
        }
    }
    default:
        break;
    }

    return status;
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

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit);
    status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);

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

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
    status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);

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

bool emberAfThermostatClusterSetpointRaiseLowerCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj,
                                                        uint8_t mode, int8_t amount)
{
    int16_t HeatingSetpoint = 2000, CoolingSetpoint = 2600; // Set to defaults to be safe
    EmberAfStatus status     = EMBER_ZCL_STATUS_FAILURE;
    EmberAfStatus ReadStatus = EMBER_ZCL_STATUS_FAILURE;

    switch (mode)
    {
    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_AND_COOL_SETPOINTS: {

        // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3726
        // Behavior is not specified if the device only contains one mode
        // and the both mode command is sent.
        // Implemented behavior is not perfect, but spec needs to be clear before changing the implementaion

        // 4.3.7.2.1. Both
        // [4.172] The client MAY indicate Both regardless of the server feature support. The server SHALL
        // only adjust the setpoint that it supports and not respond with an error.

        // Implementation assumes that the attribute will NOT be present if the device does not support that mode.

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
