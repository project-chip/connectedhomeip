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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app::Clusters::Thermostat::Attributes;

constexpr int16_t kDefaultAbsMinHeatSetpointLimit    = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultAbsMaxHeatSetpointLimit    = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultMinHeatSetpointLimit       = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultMaxHeatSetpointLimit       = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultAbsMinCoolSetpointLimit    = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultAbsMaxCoolSetpointLimit    = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultMinCoolSetpointLimit       = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultMaxCoolSetpointLimit       = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultHeatingSetpoint            = 2000;
constexpr int16_t kDefaultCoolingSetpoint            = 2600;
constexpr uint8_t kInvalidControlSequenceOfOperation = 0xff;
constexpr uint8_t kInvalidRequestedSystemMode        = 0xff;

void emberAfThermostatClusterServerInitCallback()
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

EmberAfStatus emberAfThermostatClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                        EmberAfAttributeType attributeType, uint16_t size,
                                                                        uint8_t * value)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    int16_t requested;

    switch (attributeId)
    {
    case Ids::OccupiedHeatingSetpoint: {
        int16_t AbsMinHeatSetpointLimit;
        int16_t AbsMaxHeatSetpointLimit;
        int16_t MinHeatSetpointLimit;
        int16_t MaxHeatSetpointLimit;

        status = GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
        }

        status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;
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

        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
            requested > MaxHeatSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;
        else
            status = EMBER_ZCL_STATUS_SUCCESS;

        break;
    }

    case Ids::OccupiedCoolingSetpoint: {
        int16_t AbsMinCoolSetpointLimit;
        int16_t AbsMaxCoolSetpointLimit;
        int16_t MinCoolSetpointLimit;
        int16_t MaxCoolSetpointLimit;

        status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
        }

        status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;
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
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
            requested > MaxCoolSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;
        else
            status = EMBER_ZCL_STATUS_SUCCESS;

        break;
    }

    case Ids::MinHeatSetpointLimit:
    case Ids::MaxHeatSetpointLimit: {
        int16_t AbsMinHeatSetpointLimit;
        int16_t AbsMaxHeatSetpointLimit;

        status = GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
        }

        status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;
        }

        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (requested < AbsMinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;
        else
            status = EMBER_ZCL_STATUS_SUCCESS;

        break;
    }
    case Ids::MinCoolSetpointLimit:
    case Ids::MaxCoolSetpointLimit: {
        int16_t AbsMinCoolSetpointLimit;
        int16_t AbsMaxCoolSetpointLimit;

        status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
        }

        status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;
        }

        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (requested < AbsMinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;
        else
            status = EMBER_ZCL_STATUS_SUCCESS;

        break;
    }

    case Ids::ControlSequenceOfOperation: {
        uint8_t requestedCSO;
        requestedCSO = *value;
        if (requestedCSO > EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_AND_HEATING_WITH_REHEAT)
            status = EMBER_ZCL_STATUS_INVALID_VALUE;
        else
            status = EMBER_ZCL_STATUS_SUCCESS;

        break;
    }

    case Ids::SystemMode: {
        uint8_t ControlSequenceOfOperation = kInvalidControlSequenceOfOperation;
        uint8_t RequestedSystemMode        = kInvalidRequestedSystemMode;
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
    int16_t AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    // Optional User supplied limits
    int16_t MinHeatSetpointLimit = kDefaultMinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit = kDefaultMaxHeatSetpointLimit;

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
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMinHeatSetpointLimit missing using default");
    }

    status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMaxHeatSetpointLimit missing using default");
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
    // if they are not present emberAfReadAttribute() will not update the value so the defaults are used
    EmberAfStatus status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMinCoolSetpointLimit missing using default");
    }

    status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMaxCoolSetpointLimit missing using default");
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
    int16_t HeatingSetpoint = kDefaultHeatingSetpoint, CoolingSetpoint = kDefaultCoolingSetpoint; // Set to defaults to be safe
    EmberAfStatus status                     = EMBER_ZCL_STATUS_FAILURE;
    EmberAfStatus ReadStatus                 = EMBER_ZCL_STATUS_FAILURE;
    EmberAfStatus WriteCoolingSetpointStatus = EMBER_ZCL_STATUS_SUCCESS, WriteHeatingSetpointStatus = EMBER_ZCL_STATUS_SUCCESS;

    switch (mode)
    {
    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_AND_COOL_SETPOINTS:

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
            CoolingSetpoint            = static_cast<int16_t>(CoolingSetpoint + amount * 10);
            CoolingSetpoint            = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
            WriteCoolingSetpointStatus = SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint);
            if (WriteCoolingSetpointStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
            }
        }

        ReadStatus = GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint);
        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            HeatingSetpoint            = static_cast<int16_t>(HeatingSetpoint + amount * 10);
            HeatingSetpoint            = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
            WriteHeatingSetpointStatus = SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint);
            if (WriteHeatingSetpointStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
            }
        }
        // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3801
        // Spec behavior is not defined if one was successfull and the other not what should happen.
        // this implementation leaves the successfull one changed but will return an error.
        if ((WriteCoolingSetpointStatus == EMBER_ZCL_STATUS_SUCCESS) && (WriteHeatingSetpointStatus == EMBER_ZCL_STATUS_SUCCESS))
            status = EMBER_ZCL_STATUS_SUCCESS;
        break;

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_COOL_SETPOINT:
        // In cooling mode we will need to change only the cooling setpoint
        ReadStatus = GetOccupiedCoolingSetpoint(aEndpointId, &CoolingSetpoint);

        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
            CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
            status          = SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint);
        }
        break;

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_SETPOINT:
        // In cooling mode we will need to change only the cooling setpoint
        ReadStatus = GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint);
        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
            HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
            status          = SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint);
        }
        break;

    default:
        // Nothing to do here
        break;
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
