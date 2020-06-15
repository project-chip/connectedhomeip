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
 * @brief CLI for the DMO Tuning and Testing plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/util/serial/command-interpreter2.h"

#include "app/framework/plugin-soc/ble/ble-interface.h"

#include "dmp-tuning.h"
#include "dmp-tuning-profiles.h"

void emRadioEnableConfigScheduledCallback(bool enable);
bool emRadioConfigScheduledCallbackEnabled(void);
void emRadioConfigScheduledCallback(bool scheduled);

// Zigbee related CLI commands
// Zigbee shceduler priorities
void emberAfPluginDmpTuningSetZigbeeSchedulerPrioritiesCommand(void)
{
  EmberMultiprotocolPriorities pri;
  pri.backgroundRx = emberUnsignedCommandArgument(0);
  pri.activeRx = emberUnsignedCommandArgument(1);
  pri.tx = emberUnsignedCommandArgument(2);
  if (emberRadioSetSchedulerPriorities(&pri) == EMBER_SUCCESS) {
    emRadioSleep();
    emRadioWakeUp();
    emberAfCorePrintln("DmpTuning - SetZigbeeSchedulerPrioritiesCommand: success");
  } else {
    emberAfCorePrintln("invalid call");
  }
}

void emberAfPluginDmpTuningGetZigbeeSchedulerPrioritiesCommand(void)
{
  EmberMultiprotocolPriorities pri;
  emberRadioGetSchedulerPriorities(&pri);
  emberAfCorePrintln("DmpTuning - GetZigbeeSchedulerPrioritiesCommand: backgroundRx:%d activeRx:%d tx:%d",
                     pri.backgroundRx,
                     pri.activeRx,
                     pri.tx);
}

// Zigbee Slip Time
void emberAfPluginDmpTuningSetZigbeeSlipTimeCommand(void)
{
  EmberStatus status = emberRadioSetSchedulerSliptime((uint32_t)emberUnsignedCommandArgument(0));
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("DmpTuning - SetZigbeeSlipTime: success");
  } else {
    emberAfCorePrintln("DmpTuning - SetZigbeeSlipTime should only be used in multiprotocol");
  }
}

void emberAfPluginDmpTuningGetZigbeeSlipTimeCommand(void)
{
  uint32_t slipTime;
  EmberStatus status = emberRadioGetSchedulerSliptime(&slipTime);
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("DmpTuning - GetZigbeeSlipTime: slip_time:%d", slipTime);
  } else {
    emberAfCorePrintln("DmpTuning - GetZigbeeSlipTime should only be used in multiprotocol");
  }
}

// BLE related CLI commands
// BLE Scan Priorities
void emberAfPluginDmpTuningSetBleScanPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  ll_priorities->scan_min = emberUnsignedCommandArgument(0);
  ll_priorities->scan_max = emberUnsignedCommandArgument(1);

  emberAfCorePrintln("DmpTuning - SetBleScanPriorities: success");
}

void emberAfPluginDmpTuningGetBleScanPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  emberAfCorePrintln("DmpTuning - GetBleScanPriorities: scan_min:%d scan_max:%d",
                     ll_priorities->scan_min,
                     ll_priorities->scan_max);
}

// BLE Advertisement Priorities
void emberAfPluginDmpTuningSetBleAdvertisementPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  ll_priorities->adv_min = emberUnsignedCommandArgument(0);
  ll_priorities->adv_max = emberUnsignedCommandArgument(1);

  emberAfCorePrintln("DmpTuning - SetBleAdvertisementPriorities: success");
}

void emberAfPluginDmpTuningGetBleAdvertisementPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  emberAfCorePrintln("DmpTuning - GetBleAdvertisementPriorities: adv_min:%d adv_max:%d",
                     ll_priorities->adv_min,
                     ll_priorities->adv_max);
}

// BLE Connection Init Priorities
void emberAfPluginDmpTuningSetBleConnectionInitPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  ll_priorities->init_min = emberUnsignedCommandArgument(0);
  ll_priorities->init_max = emberUnsignedCommandArgument(1);

  emberAfCorePrintln("DmpTuning - SetBleConnectionInitPriorities: success");
}

void emberAfPluginDmpTuningGetBleConnectionInitPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  emberAfCorePrintln("DmpTuning - GetBleConnectionInitPriorities: init_min:%d init_max:%d",
                     ll_priorities->init_min,
                     ll_priorities->init_max);
}

// BLE Connection Priorities
void emberAfPluginDmpTuningSetBleConnectionPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  ll_priorities->conn_min = emberUnsignedCommandArgument(0);
  ll_priorities->conn_max = emberUnsignedCommandArgument(1);

  emberAfCorePrintln("DmpTuning - SetBleConnectionPriorities: success");
}

void emberAfPluginDmpTuningGetBleConnectionPrioritiesCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  emberAfCorePrintln("DmpTuning - GetBleConnectionPriorities: conn_min:%d conn_max:%d",
                     ll_priorities->conn_min,
                     ll_priorities->conn_max);
}

// BLE RAIL Mapping
void emberAfPluginDmpTuningSetBleRailMappingCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  ll_priorities->rail_mapping_offset = emberUnsignedCommandArgument(0);
  ll_priorities->rail_mapping_range = emberUnsignedCommandArgument(1);

  emberAfCorePrintln("DmpTuning - SetBleRailMapping: success");
}

void emberAfPluginDmpTuningGetBleRailMappingCommand(void)
{
  gecko_bluetooth_ll_priorities *ll_priorities = emberAfPluginBleGetLlPrioritiesConfig();

  emberAfCorePrintln("DmpTuning - GetBleRailMapping: rail_mapping_offset:%d rail_mapping_range:%d",
                     ll_priorities->rail_mapping_offset,
                     ll_priorities->rail_mapping_range);
}

// CLI for protocol switch related RAIL events.
void emberAfPluginDmpTuningGetRailScheduledEventCounters(void)
{
  emberAfCorePrintln("Scheduled event counter:%d Unscheduled event counter:%d",
                     railScheduledEventCntr,
                     railUnscheduledEventCntr);
}

void emberAfPluginDmpTuningEnableRailConfigSchedulerEvents(void)
{
  emRadioEnableConfigScheduledCallback(true);
  emberAfCorePrintln("DmpTuning - EnableRailConfigSchedulerEvents: success");
}

void emberAfPluginDmpTuningDisableRailConfigSchedulerEvents(void)
{
  emRadioEnableConfigScheduledCallback(false);
  emberAfCorePrintln("DmpTuning - DisableRailConfigSchedulerEvents: success");
}

void emberAfPluginDmpTuningGetRailConfigSchedulerEventsEnabled(void)
{
  emberAfCorePrintln("RAIL config scheduler event enabled: %d",
                     emRadioConfigScheduledCallbackEnabled());
}
