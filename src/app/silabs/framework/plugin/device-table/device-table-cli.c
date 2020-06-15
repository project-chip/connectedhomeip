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
 * @brief CLI for the Device Table plugin.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#ifdef EZSP_HOST
// Includes needed for functions related to the EZSP host
  #include "stack/include/error.h"
  #include "stack/include/ember-types.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/serial-interface.h"
  #include "app/util/zigbee-framework/zigbee-device-common.h"
#else
  #include "stack/include/ember.h"
#endif

#include "hal/hal.h"
#include "app/util/serial/command-interpreter2.h"
#include "af.h"
#include "stack/include/event.h"

#include <stdlib.h>
#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/device-table/device-table-internal.h"

static EmberNodeId discoverNode;

// --------------------------------
// Print commands
void emAfDeviceTablePrintEUI64(uint8_t *eui64)
{
  uint8_t i;
  for (i = 8; i > 0; i--) {
    emberAfCorePrint("%X", eui64[i - 1]);
  }
}

static void printState(EmberAfPluginDeviceTableDeviceState state)
{
  switch (state) {
    case EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED:
      emberAfCorePrint(" JOINED");
      break;
    case EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_UNRESPONSIVE:
      emberAfCorePrint(" UNRESPONSIVE");
      break;
    case EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT:
      emberAfCorePrint(" LEAVE_SENT");
      break;
    case EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEFT:
      emberAfCorePrint(" LEFT");
      break;
    default:
      break;
  }
}

static void printDeviceId(uint16_t deviceId)
{
  switch (deviceId) {
    case DEVICE_ID_ON_OFF_SWITCH:
      emberAfCorePrint("ON_OFF_SWITCH");
      break;

    case DEVICE_ID_LEVEL_CONTROL_SWITCH:
      emberAfCorePrint("LEVEL_CONTROL_SWITCH");
      break;

    case DEVICE_ID_ON_OFF_OUTPUT:
      emberAfCorePrint("ON_OFF_OUTPUT");
      break;

    case DEVICE_ID_LEVEL_CONTROL_OUTPUT:
      emberAfCorePrint("LEVEL_CONTROL_OUTPUT");
      break;

    case DEVICE_ID_SCENE_SELECTOR:
      emberAfCorePrint("SCENE_SELECTOR");
      break;

    case DEVICE_ID_CONFIG_TOOL:
      emberAfCorePrint("CONFIG_TOOL");
      break;

    case DEVICE_ID_REMOTE_CONTROL:
      emberAfCorePrint("REMOTE_CONTROL");
      break;

    case DEVICE_ID_COMBINED_INTERFACE:
      emberAfCorePrint("COMBINED_INTERFACE");
      break;

    case DEVICE_ID_RANGE_EXTENDER:
      emberAfCorePrint("RANGE_EXTENDER");
      break;

    case DEVICE_ID_MAINS_POWER_OUTLET:
      emberAfCorePrint("MAINS_POWER_OUTLET");
      break;

    case DEVICE_ID_DOOR_LOCK:
      emberAfCorePrint("DOOR_LOCK");
      break;

    case DEVICE_ID_DOOR_LOCK_CONTROLLER:
      emberAfCorePrint("DOOR_LOCK_CONTROLLER");
      break;

    case DEVICE_ID_SIMPLE_SENSOR:
      emberAfCorePrint("SIMPLE_SENSOR");
      break;

    case DEVICE_ID_CONSUMPTION_AWARENESS_DEVICE:
      emberAfCorePrint("CONSUMPTION_AWARENESS_DEVICE");
      break;

    case DEVICE_ID_HOME_GATEWAY:
      emberAfCorePrint("HOME_GATEWAY");
      break;

    case DEVICE_ID_SMART_PLUG:
      emberAfCorePrint("SMART_PLUG");
      break;

    case DEVICE_ID_WHITE_GOODS:
      emberAfCorePrint("WHITE_GOODS");
      break;

    case DEVICE_ID_METER_INTERFACE:
      emberAfCorePrint("METER_INTERFACE");
      break;

    case DEVICE_ID_ON_OFF_LIGHT:
      emberAfCorePrint("ON_OFF_LIGHT");
      break;

    case DEVICE_ID_DIMMABLE_LIGHT:
      emberAfCorePrint("DIMMABLE_LIGHT");
      break;

    case DEVICE_ID_COLOR_DIMMABLE_LIGHT:
      emberAfCorePrint("COLOR_DIMMABLE_LIGHT");
      break;

    case DEVICE_ID_ON_OFF_LIGHT_SWITCH:
      emberAfCorePrint("ON_OFF_LIGHT_SWITCH");
      break;

    case DEVICE_ID_DIMMER_SWITCH:
      emberAfCorePrint("DIMMER_SWITCH");
      break;

    case DEVICE_ID_COLOR_DIMMER_SWITCH:
      emberAfCorePrint("COLOR_DIMMER_SWITCH");
      break;

    case DEVICE_ID_LIGHT_SENSOR:
      emberAfCorePrint("LIGHT_SENSOR");
      break;

    case DEVICE_ID_OCCUPANCY_SENSOR:
      emberAfCorePrint("OCCUPANCY_SENSOR");
      break;

    case DEVICE_ID_SHADE:
      emberAfCorePrint("SHADE");
      break;

    case DEVICE_ID_SHADE_CONTROLLER:
      emberAfCorePrint("SHADE_CONTROLLER");
      break;

    case DEVICE_ID_WINDOW_COVERING_DEVICE:
      emberAfCorePrint("WINDOW_COVERING_DEVICE");
      break;

    case DEVICE_ID_WINDOW_COVERING_CONTROLLER:
      emberAfCorePrint("WINDOW_COVERING_CONTROLLER");
      break;

    case DEVICE_ID_HEATING_COOLING_UNIT:
      emberAfCorePrint("HEATING_COOLING_UNIT");
      break;

    case DEVICE_ID_THERMOSTAT:
      emberAfCorePrint("THERMOSTAT");
      break;

    case DEVICE_ID_TEMPERATURE_SENSOR:
      emberAfCorePrint("TEMPERATURE_SENSOR");
      break;

    case DEVICE_ID_PUMP:
      emberAfCorePrint("PUMP");
      break;

    case DEVICE_ID_PUMP_CONTROLLER:
      emberAfCorePrint("PUMP_CONTROLLER");
      break;

    case DEVICE_ID_PRESSURE_SENSOR:
      emberAfCorePrint("PRESSURE_SENSOR");
      break;

    case DEVICE_ID_FLOW_SENSOR:
      emberAfCorePrint("FLOW_SENSOR");
      break;

    case DEVICE_ID_MINI_SPLIT_AC:
      emberAfCorePrint("MINI_SPLIT_AC");
      break;

    case DEVICE_ID_IAS_CIE:
      emberAfCorePrint("IAS_CIE");
      break;

    case DEVICE_ID_IAS_ANCILLARY_CONTROL:
      emberAfCorePrint("IAS_ANCILLARY_CONTROL");
      break;

    case DEVICE_ID_IAS_ZONE:
      emberAfCorePrint("IAS_ZONE");
      break;

    case DEVICE_ID_IAS_WARNING:
      emberAfCorePrint("IAS_WARNING");
      break;

    default:
      break;
  }
}

void emberAfDeviceTablePrintDeviceTable(void)
{
  uint16_t totalDevices = 0;
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  uint16_t index;

  for (index = 0;
       index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
       index++) {
    if (deviceTable[index].nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      emberAfCorePrint("%d %2x:  ", totalDevices, deviceTable[index].nodeId);
      emAfDeviceTablePrintEUI64(deviceTable[index].eui64);
      emberAfCorePrint(" %d ", deviceTable[index].endpoint);
      printDeviceId(deviceTable[index].deviceId);
      printState(deviceTable[index].state);
      emberAfCorePrintln(" %l", emberAfDeviceTableTimeSinceLastMessage(index));
      totalDevices++;
    }
  }
  emberAfCorePrintln("Total Devices %d", totalDevices);
}

void emAfDeviceTablePrintBuffer(uint8_t *buffer, uint16_t bufLen)
{
  int i;
  for (i = 0; i < bufLen; i++) {
    emberAfCorePrint("%x ", buffer[i]);
  }
  emberAfCorePrintln("");
}

// --------------------------------
// CLI section
void deviceTabeCliServiceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  if (result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST) {
    if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT) {
      emberAfCorePrintln("Unicast Timeout");
      return;
    }

    uint8_t* eui64ptr = (uint8_t*)(result->responseData);
    emberAfDeviceTableNewDeviceJoinHandler(discoverNode, eui64ptr);
  }
}

void emAfDeviceTableIndexRemoveCommand(void)
{
  uint16_t index = (uint8_t)emberUnsignedCommandArgument(0);
  emAfPluginDeviceTableDeleteEntry(index);
}

void emAfDeviceTableRemoveCommand(void)
{
  EmberEUI64 eui64;
  uint16_t index;

  emberCopyBigEndianEui64Argument(0, eui64);
  index = emberAfDeviceTableGetFirstIndexFromEui64(eui64);

  while (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    emAfPluginDeviceTableDeleteEntry(index);
    index = emAfDeviceTableFindNextEndpoint(index);
  }
}

void emAfDeviceTableIndexRouteRepairCommand(void)
{
  uint16_t index = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t nodeId = emberAfDeviceTableGetNodeIdFromIndex(index);

  if (nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    emberAfCorePrintln("ROUTE REPAIR:  CLI");
    emberAfPluginDeviceTableInitiateRouteRepair(nodeId);
  }
}

void emAfDeviceTableRouteRepairCommand(void)
{
  uint16_t nodeId;
  EmberEUI64 eui64;

  emberCopyBigEndianEui64Argument(0, eui64);
  nodeId = emberAfDeviceTableGetNodeIdFromEui64(eui64);

  if (nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    emberAfCorePrintln("ROUTE REPAIR:  CLI");
    emberAfPluginDeviceTableInitiateRouteRepair(nodeId);
  }
}

void emAfDeviceTableDiscoverPresentNodeCommand(void)
{
  discoverNode = (EmberNodeId)emberUnsignedCommandArgument(0);
  emberAfFindIeeeAddress(discoverNode,
                         deviceTabeCliServiceDiscoveryCallback);
}

void emAfDeviceTableIndexSendCommand(void)
{
  uint16_t index = (uint16_t)emberUnsignedCommandArgument(0);
  emberAfDeviceTableCliIndexSend(index);
}

void emAfDeviceTableSendCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(1);
  EmberEUI64 eui64;

  emberCopyBigEndianEui64Argument(0, eui64);
  emberAfDeviceTableSend(eui64, endpoint);
}

void emAfDeviceTableSaveCommand(void)
{
  emAfDeviceTableSave();
}

void emAfDeviceTableLoadCommand(void)
{
  emAfDeviceTableLoad();
}

void emAfPluginDeviceTableSendLeaveCommand(void)
{
  EmberEUI64 eui64;
  emberCopyBigEndianEui64Argument(0, eui64);
}
