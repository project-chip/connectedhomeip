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
/***************************************************************************//**
 * @file
 * @brief Code to use a HAL battery monitor plugin to act as a server for the power
 * configuration cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#define MILLI_TO_DECI_CONVERSION_FACTOR 100

//------------------------------------------------------------------------------
// Implemented callbacks

void emberAfPluginBatteryMonitorDataCallback(uint16_t voltageMilliV)
{
  EmberAfStatus afStatus;
  uint8_t voltageDeciV;
  uint8_t i;
  uint8_t endpoint;

  emberAfAppPrintln("New voltage reading: %d mV", voltageMilliV);

  // convert from mV to 100 mV, which are the units specified by zigbee spec for
  // the power configuration cluster's voltage attribute.
  voltageDeciV = ((uint8_t) (voltageMilliV / MILLI_TO_DECI_CONVERSION_FACTOR));

  // Cycle through all endpoints, check to see if the endpoint has a power
  // configuration server, and if so update the voltage attribute
  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint, ZCL_POWER_CONFIG_CLUSTER_ID)) {
      afStatus = emberAfWriteServerAttribute(endpoint,
                                             ZCL_POWER_CONFIG_CLUSTER_ID,
                                             ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID,
                                             &voltageDeciV,
                                             ZCL_INT8U_ATTRIBUTE_TYPE);

      if (EMBER_ZCL_STATUS_SUCCESS != afStatus) {
        emberAfAppPrintln("Power Configuration Server: failed to write value "
                          "0x%x to cluster 0x%x attribute ID 0x%x: error 0x%x",
                          voltageDeciV,
                          ZCL_POWER_CONFIG_CLUSTER_ID,
                          ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID,
                          afStatus);
      }
    }
  }
}
