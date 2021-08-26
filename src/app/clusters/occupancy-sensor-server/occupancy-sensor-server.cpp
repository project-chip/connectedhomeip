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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief Routines for the Occupancy plugin, which
 *implements the Occupancy server cluster.
 *******************************************************************************
 ******************************************************************************/
#include "occupancy-sensor-server.h"

#include <app/util/af.h>

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include "occupancy-hal.h"

#ifdef EMBER_AF_PLUGIN_REPORTING
#include <app/reporting/reporting.h>
#endif

using namespace chip;

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfOccupancySensingClusterServerInitCallback(EndpointId endpoint)
{
    HalOccupancySensorType deviceType;

    deviceType = halOccupancyGetSensorType(endpoint);

    emberAfWriteAttribute(endpoint, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, ZCL_OCCUPANCY_SENSOR_TYPE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                          (uint8_t *) &deviceType, ZCL_ENUM8_ATTRIBUTE_TYPE);

    uint8_t deviceTypeBitmap = 0;
    switch (deviceType)
    {
    case HAL_OCCUPANCY_SENSOR_TYPE_PIR:
        deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PIR;
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC:
        deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_ULTRASONIC;
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC:
        deviceTypeBitmap = (EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PIR | EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_ULTRASONIC);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PHYSICAL:
        deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PHYSICAL_CONTACT;
        break;

    default:
        break;
    }
    emberAfWriteAttribute(endpoint, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, ZCL_OCCUPANCY_SENSOR_TYPE_BITMAP_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER, &deviceTypeBitmap, ZCL_BITMAP8_ATTRIBUTE_TYPE);

    emberAfPluginOccupancyClusterServerPostInitCallback(endpoint);
}

//******************************************************************************
// Notification callback from the HAL plugin
//******************************************************************************
void halOccupancyStateChangedCallback(EndpointId endpoint, HalOccupancyState occupancyState)
{
    if (occupancyState == HAL_OCCUPANCY_STATE_OCCUPIED)
    {
        emberAfOccupancySensingClusterPrintln("Occupancy detected");
    }
    else
    {
        emberAfOccupancySensingClusterPrintln("Occupancy no longer detected");
    }

    emberAfWriteAttribute(endpoint, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, ZCL_OCCUPANCY_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                          (uint8_t *) &occupancyState, ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

void emberAfPluginOccupancyClusterServerPostInitCallback(EndpointId endpoint) {}

HalOccupancySensorType __attribute__((weak)) halOccupancyGetSensorType(EndpointId endpoint)
{
    return HAL_OCCUPANCY_SENSOR_TYPE_PIR;
}
