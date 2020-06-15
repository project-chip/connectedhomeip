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
 * @brief Plugin code for better DMP Tuning and Testing.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "dmp-tuning-profiles.h"

uint32_t railScheduledEventCntr = 0;
uint32_t railUnscheduledEventCntr = 0;

void emberAfPluginBleGetConfigCallback(gecko_configuration_t* config)
{
  // Make sure BLE scheduler priorities are set up according to the plugin options.
  config->bluetooth.linklayer_priorities->rail_mapping_offset =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_RAIL_MAPPING_OFFSET;
  config->bluetooth.linklayer_priorities->rail_mapping_range =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_RAIL_MAPPING_RANGE;
  config->bluetooth.linklayer_priorities->scan_min =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_SCAN_PRIORITY_MIN;
  config->bluetooth.linklayer_priorities->scan_max =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_SCAN_PRIORITY_MAX;
  config->bluetooth.linklayer_priorities->adv_min =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_ADVERTISEMENT_PRIORITY_MIN;
  config->bluetooth.linklayer_priorities->adv_max =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_ADVERTISEMENT_PRIORITY_MAX;
  config->bluetooth.linklayer_priorities->init_min =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_CONNECTION_INIT_PRIORITY_MIN;
  config->bluetooth.linklayer_priorities->init_max =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_CONNECTION_INIT_PRIORITY_MAX;
  config->bluetooth.linklayer_priorities->conn_min =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_CONNECTION_PRIORITY_MIN;
  config->bluetooth.linklayer_priorities->conn_max =
    EMBER_AF_PLUGIN_DMP_TUNING_BLE_CONNECTION_PRIORITY_MAX;

  emberAfCorePrintln("BLE scheduler priorities initialized.");
}

void emberAfPluginDmpTuningInitCallback(void)
{
  // Configure the zigbee TX and RX priorities
  EmberMultiprotocolPriorities pri;
  pri.backgroundRx = EMBER_AF_PLUGIN_DMP_TUNING_ZB_BACKGROUND_RX_PRIORIY;
  pri.activeRx = EMBER_AF_PLUGIN_DMP_TUNING_ZB_ACTIVE_RX_PRIORITY;
  pri.tx = EMBER_AF_PLUGIN_DMP_TUNING_ZB_ACTIVE_TX_PRIORITY;
  if (emberRadioSetSchedulerPriorities(&pri) == EMBER_SUCCESS) {
    emberAfCorePrintln("Zigbee TX/RX priorities initialized.");
  } else {
    emberAfCorePrintln("Warning: zigbee TX/RX priorities cannot be initialized.");
  }
}

void emRadioConfigScheduledCallback(bool scheduled)
{
  if (scheduled) {
    railScheduledEventCntr++;
  } else {
    railUnscheduledEventCntr++;
  }
}
