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

int32_t EnforceHeatingSetpointLimits(int32_t HeatingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinHeatSetpointLimit = 0x02bc; // 7C (44.5 F) is the default
    int16_t AbsMaxHeatSetpointLimit = 0x0bb8; // 30C (86 F) is the default

    // Optional User supplied limits
    int16_t MinHeatSetpointLimit = 0x02bc; // 7C (44.5 F) is the default
    int16_t MaxHeatSetpointLimit = 0x0bb8; // 30C (86 F) is the default

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present emberAfReadAttribute() will not update the value so the defaults are used

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_ABS_MIN_HEAT_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &AbsMinHeatSetpointLimit, sizeof(AbsMinHeatSetpointLimit), NULL);

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_ABS_MAX_HEAT_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &AbsMaxHeatSetpointLimit, sizeof(AbsMaxHeatSetpointLimit), NULL);

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_MIN_HEAT_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &MinHeatSetpointLimit, sizeof(MinHeatSetpointLimit), NULL);

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_MAX_HEAT_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &MaxHeatSetpointLimit, sizeof(MaxHeatSetpointLimit), NULL);

    // Make sure the user imposed limits are within the manufacture imposed limits

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

int32_t EnforceCoolingSetpointLimits(int32_t CoolingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinCoolSetpointLimit = 0x0640; // 16C (61 F) is the default
    int16_t AbsMaxCoolSetpointLimit = 0x0c80; // 32C (90 F) is the default

    // Optional User supplied limits
    int16_t MinCoolSetpointLimit = 0x0640; // 16C (61 F) is the default
    int16_t MaxCoolSetpointLimit = 0x0c80; // 32C (90 F) is the default

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present emberAfReadAttribute() will not update the value so the defaults are used

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_ABS_MIN_COOL_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &AbsMinCoolSetpointLimit, sizeof(AbsMinCoolSetpointLimit), NULL);

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_ABS_MAX_COOL_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &AbsMaxCoolSetpointLimit, sizeof(AbsMaxCoolSetpointLimit), NULL);

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_MIN_COOL_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &MinCoolSetpointLimit, sizeof(MinCoolSetpointLimit), NULL);

    emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_MAX_COOL_SETPOINT_LIMIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *) &MaxCoolSetpointLimit, sizeof(MaxCoolSetpointLimit), NULL);

    // Make sure the user imposed limits are within the manufacture imposed limits

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
    int32_t HeatingSetpoint = 2000, CoolingSetpoint = 2600; // Set to defaults to be safe
    EmberAfStatus status     = EMBER_ZCL_STATUS_FAILURE;
    EmberAfStatus ReadStatus = EMBER_ZCL_STATUS_FAILURE;

    switch (mode)
    {
    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_AND_COOL_SETPOINTS: {

        // In auto mode we will need to change both the heating and cooling setpoints
        ReadStatus = emberAfReadAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, sizeof(CoolingSetpoint), NULL);
        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            CoolingSetpoint = CoolingSetpoint + static_cast<int16_t>(amount * 10);
            CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
            status = emberAfWriteAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, ZCL_INT16S_ATTRIBUTE_TYPE);
        }
        ReadStatus = emberAfReadAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, sizeof(HeatingSetpoint), NULL);
        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            HeatingSetpoint += (amount * 10);
            HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
            status = emberAfWriteAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, ZCL_INT16S_ATTRIBUTE_TYPE);
        }

        break;
    }

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_COOL_SETPOINT: {
        // In cooling mode we will need to change only the cooling setpoint
        ReadStatus = emberAfReadAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, sizeof(CoolingSetpoint), NULL);
        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            CoolingSetpoint += (amount * 10);
            CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
            status = emberAfWriteAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, ZCL_INT16U_ATTRIBUTE_TYPE);
        }
        break;
    }

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_SETPOINT: {
        // In cooling mode we will need to change only the cooling setpoint
        ReadStatus = emberAfReadAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, sizeof(CoolingSetpoint), NULL);
        if (ReadStatus == EMBER_ZCL_STATUS_SUCCESS)
        {
            HeatingSetpoint += (amount * 10);
            HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
            status = emberAfWriteAttribute(aEndpointId, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, ZCL_INT16U_ATTRIBUTE_TYPE);
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
