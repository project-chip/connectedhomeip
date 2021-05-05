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

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/enums.h"

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

bool emberAfThermostatClusterClearWeeklyScheduleCallback()
{
    // TODO
    return false;
}
bool emberAfThermostatClusterGetRelayStatusLogCallback()
{
    // TODO
    return false;
}

bool emberAfThermostatClusterGetWeeklyScheduleCallback(uint8_t daysToReturn, uint8_t modeToReturn)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetWeeklyScheduleCallback(uint8_t numberOfTransitionsForSequence, uint8_t daysOfWeekForSequence,
                                                       uint8_t modeForSequence, uint8_t * payload)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetpointRaiseLowerCallback(uint8_t mode, int8_t amount)
{
    bool result             = false;
    EndpointId endpoint     = 1;                            // Hard code to 1 for now/
    int32_t HeatingSetpoint = 2000, CoolingSetpoint = 2600; // Set to defaults to be safe
    EmberAfStatus status;
    switch (mode)
    {
    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_AND_COOL_SETPOINTS: {
        // In auto mode we will need to change both the heating and cooling setpoints
        status = emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, sizeof(CoolingSetpoint), NULL);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            status = emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, sizeof(HeatingSetpoint), NULL);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                HeatingSetpoint += (amount * 10);
                CoolingSetpoint += (amount * 10);
                // TODO should check against maximum/minimum and absolute max and min values
                status = emberAfWriteAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                               CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, ZCL_INT16U_ATTRIBUTE_TYPE);
                if (status == EMBER_ZCL_STATUS_SUCCESS)
                {
                    status = emberAfWriteAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                                   CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, ZCL_INT16U_ATTRIBUTE_TYPE);
                    if (status == EMBER_ZCL_STATUS_SUCCESS)
                    {
                        result = true;
                    }
                    else
                    {
                        // Todo roll back the setpoints
                    }
                }
                else
                {
                    // Todo roll back the setpoints
                }
            }
        }
        break;
    }

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_COOL_SETPOINT: {
        // In cooling mode we will need to change only the cooling setpoint
        status = emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, sizeof(CoolingSetpoint), NULL);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            CoolingSetpoint += (amount * 10);
            // TODO should check against maximum/minimum and absolute max and min values
            status = emberAfWriteAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_COOLING_SETPOINT_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER, (uint8_t *) &CoolingSetpoint, ZCL_INT16U_ATTRIBUTE_TYPE);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                result = true;
            }
            else
            {
                // Todo roll back the setpoints
            }
        }
        break;
    }

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_SETPOINT: {
        // In cooling mode we will need to change only the cooling setpoint
        status = emberAfReadAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, sizeof(CoolingSetpoint), NULL);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            HeatingSetpoint += (amount * 10);
            // TODO should check against maximum/minimum and absolute max and min values
            status = emberAfWriteAttribute(endpoint, ZCL_THERMOSTAT_CLUSTER_ID, ZCL_OCCUPIED_HEATING_SETPOINT_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER, (uint8_t *) &HeatingSetpoint, ZCL_INT16U_ATTRIBUTE_TYPE);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                result = true;
            }
            else
            {
                // Todo roll back the setpoints
            }
        }
        break;
    }

    default:
        // Nothing to do here
        break;
    }
    emberAfSendImmediateDefaultResponse(status);
    return result;
}
