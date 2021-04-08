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

#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include "gen/att-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"
#include "gen/enums.h"

#include "occupancy-hal.h"

#ifdef EMBER_AF_PLUGIN_REPORTING
#include <app/reporting/reporting.h>
#endif


using namespace chip;

//------------------------------------------------------------------------------
// Plugin private function prototypes
static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
/*
// The default configuration to be used if no reporting has been set up
static EmberAfPluginReportingEntry defaultConfiguration = {
  EMBER_ZCL_REPORTING_DIRECTION_REPORTED, //direction
  0, //endpoint, which will be set on a per-use basis
  ZCL_OCCUPANCY_SENSING_CLUSTER_ID, //clusterId
  ZCL_OCCUPANCY_ATTRIBUTE_ID, //attributeId
  CLUSTER_MASK_SERVER, //mask
  EMBER_AF_NULL_MANUFACTURER_CODE, //manufacturerCode
  .data.reported = {
    1, //minInterval
    EMBER_AF_PLUGIN_OCCUPANCY_SENSOR_SERVER_DEFAULT_OCCUPANCY_MAX_REPORT_PERIOD_S, //maxInterval
    1 //reportableChange
  }
};
*/
//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfOccupancySensingClusterServerInitCallback(chip::EndpointId endpoint)
{
  HalOccupancySensorType deviceType;

  checkForReportingConfig();
  deviceType = halOccupancyGetSensorType(endpoint);

  emberAfWriteAttribute(endpoint, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, ZCL_OCCUPANCY_SENSOR_TYPE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                               (uint8_t *) &deviceType, ZCL_ENUM8_ATTRIBUTE_TYPE);

  uint8_t deviceTypeBitmap = 0;
  switch (deviceType) {
    case HAL_OCCUPANCY_SENSOR_TYPE_PIR:
      deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PIR;
      break;

    case HAL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC:
      deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_ULTRASONIC;
      break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC:
      deviceTypeBitmap = (EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PIR
                          | EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_ULTRASONIC);
      break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PHYSICAL:
      deviceTypeBitmap = EMBER_AF_OCCUPANCY_SENSOR_TYPE_BITMAP_PHYSICAL_CONTACT;
      break;

    default:
      break;
  }
  emberAfWriteAttribute(endpoint, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, ZCL_OCCUPANCY_SENSOR_TYPE_BITMAP_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                               (uint8_t *) &deviceTypeBitmap, ZCL_BITMAP8_ATTRIBUTE_TYPE);

  emberAfPluginOccupancyClusterServerPostInitCallback(endpoint);
}

//******************************************************************************
// Notification callback from the HAL plugin
//******************************************************************************
void halOccupancyStateChangedCallback(EndpointId endpoint, HalOccupancyState occupancyState)
{
  if (occupancyState == HAL_OCCUPANCY_STATE_OCCUPIED) {
    emberAfOccupancySensingClusterPrintln("Occupancy detected");
  } else {
    emberAfOccupancySensingClusterPrintln("Occupancy no longer detected");
  }

  emberAfWriteAttribute(endpoint, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, ZCL_OCCUPANCY_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                               (uint8_t *) &occupancyState, ZCL_BITMAP8_ATTRIBUTE_TYPE);
  // emberAfPluginOccupancySensorServerOccupancyStateChangedCallback(
  //   endpoint, occupancyState);
}

void emberAfPluginOccupancySensorServerStackStatusCallback(
  EndpointId endpoint, EmberStatus status)
{
  // On network connect, verify that a reporting entry is set up for the
  // occupancy sensor
  if (status == EMBER_NETWORK_UP) {
    checkForReportingConfig();
  }
}

//------------------------------------------------------------------------------
// Plugin private functions

static void checkForReportingConfig(void)
{
#ifdef EMBER_AF_PLUGIN_REPORTING
  // uint16_t i;
  // EmberAfPluginReportingEntry entry;
  // uint8_t endpoint;
  // bool existingEntry = false;

  // // On initialization, cycle through the reporting table to determine if a
  // // reporting table entry has been created for the occupancy state attribute
  // for (i = 0; i < emAfPluginReportingNumEntries(); i++) {
  //   emAfPluginReportingGetEntry(i, &entry);
  //   if ((entry.clusterId == ZCL_OCCUPANCY_SENSING_CLUSTER_ID)
  //       && (entry.attributeId == ZCL_OCCUPANCY_ATTRIBUTE_ID)
  //       && (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
  //       && (entry.manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE)) {
  //     existingEntry = true;
  //   }
  // }

  // // If no entry is found for the occupancy sensor, a default reporting
  // // configuration should be created using the plugin defined options.  This
  // // needs to be done for all endpoints that support an occupancy sensor server.
  // if (!existingEntry) {
  //   for (i = 0; i < emberAfEndpointCount(); i++) {
  //     endpoint = emberAfEndpointFromIndex(i);
  //     defaultConfiguration.endpoint = endpoint;
  //     if (emberAfContainsServer(endpoint, ZCL_OCCUPANCY_SENSING_CLUSTER_ID)) {
  //       emAfPluginReportingAppendEntry(&defaultConfiguration);
  //     }
  //   }
  // }
#endif
}

void emberAfPluginOccupancyClusterServerPostInitCallback(EndpointId endpoint) {}

HalOccupancySensorType __attribute__((weak)) halOccupancyGetSensorType(chip::EndpointId endpoint) {
    return HAL_OCCUPANCY_SENSOR_TYPE_PIR;
}
