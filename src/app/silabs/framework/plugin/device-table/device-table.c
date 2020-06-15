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
 * @brief Routines for the Device Table plugin.
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

#include "af.h"
#include "af-main.h"
#include "attribute-storage.h"
#include "common.h"
#include "hal/hal.h"
#include "app/util/serial/command-interpreter2.h"
#include "stack/include/event.h"

#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/device-table/device-table-internal.h"
#include "app/framework/util/util.h"
#include <stdlib.h>

void emAfDeviceTableSave(void);
void emAfDeviceTableLoad(void);

// Framework message send global data
extern uint8_t appZclBuffer[];
extern uint16_t appZclBufferLen;
extern bool zclCmdIsBuilt;
extern EmberApsFrame globalApsFrame;

extern void emAfApsFrameEndpointSetup(uint8_t srcEndpoint,
                                      uint8_t dstEndpoint);

static EmberAfPluginDeviceTableEntry deviceTable[EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE];

EmberStatus emberAfGetChildData(uint8_t index,
                                EmberChildData *childData);

// Device discovery global declarations
void emAfDeviceTableInitiateRouteRepair(EmberNodeId nodeId);
static void clearDeviceTableIndex(uint16_t index);

EmberAfPluginDeviceTableEntry* emberAfDeviceTablePointer(void)
{
  return deviceTable;
}

uint16_t emberAfDeviceTableGetNodeIdFromIndex(uint16_t index)
{
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  assert(index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);
  return deviceTable[index].nodeId;
}

uint8_t emAfDeviceTableGetFirstEndpointFromIndex(uint16_t index)
{
  assert(index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);
  return deviceTable[index].endpoint;
}

static void matchReverseEui64(EmberEUI64 eui64a, EmberEUI64 eui64b)
{
  uint8_t i;

  for (i = 0; i < EUI64_SIZE; i++) {
    if (eui64a[i] != eui64b[(EUI64_SIZE - 1) - i]) {
      return;
    }
  }

  emberAfCorePrintln("MATCH_EUI:  EUI matches backwards");

  emberAfCorePrint("A:");
  emAfDeviceTablePrintEUI64(eui64a);
  emberAfCorePrint(" B:");
  emAfDeviceTablePrintEUI64(eui64b);
  emberAfCorePrintln("");
}

static void checkNullEui64(EmberEUI64 eui64a, EmberEUI64 eui64b)
{
  uint8_t i;
  for (i = 0; i < EUI64_SIZE; i++) {
    if (eui64a[i] != 0xff
        || eui64b[i] != 0xff) {
      return;
    }
  }

  emberAfCorePrintln("MatchEUI:  two null EUI");
}

static bool matchEui64(EmberEUI64 a, EmberEUI64 b)
{
  checkNullEui64(a, b);

  if (MEMCOMPARE(a, b, EUI64_SIZE) == 0) {
    return true;
  } else {
    // Test to see if the EUI64 is backwards
    matchReverseEui64(a, b);

    return false;
  }
}

bool emberAfDeviceTableMatchEui64(EmberEUI64 eui64a, EmberEUI64 eui64b)
{
  return matchEui64(eui64a, eui64b);
}

static void unsetEui64(EmberEUI64 eui64)
{
  uint8_t i;
  for (i = 0; i < 8; i++) {
    eui64[i] = 0xff;
  }
}

static void clearDeviceTableIndex(uint16_t index)
{
  uint8_t i;

  assert(index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  deviceTable[index].nodeId = EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID;
  unsetEui64(deviceTable[index].eui64);
  deviceTable[index].state = EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_NULL;
  deviceTable[index].endpoint = 0;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; i++) {
    deviceTable[index].clusterIds[i] = ZCL_NULL_CLUSTER_ID;
  }
  deviceTable[index].clusterOutStartPosition = 0;
}

void emAfPluginDeviceTableDeleteEntry(uint16_t index)
{
  uint16_t currentIndex;

  while (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    currentIndex = index;

    // Need to compute the next index before deleting the current one.  Or else
    // the call to next endpoint will yield a bogus result.
    index = emAfDeviceTableFindNextEndpoint(index);

    clearDeviceTableIndex(currentIndex);
    emberAfPluginDeviceTableIndexRemovedCallback(currentIndex);
  }
}

void emAfDeviceTableInit(void)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    clearDeviceTableIndex(i);
  }
}

void emberAfDeviceTableClear(void)
{
  emAfDeviceTableInit();
  emAfDeviceTableSave();
  emberAfPluginDeviceTableClearedCallback();
}

uint16_t emberAfDeviceTableGetIndexFromEui64AndEndpoint(EmberEUI64 eui64,
                                                        uint8_t endpoint)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (matchEui64(deviceTable[i].eui64, eui64)
        && deviceTable[i].endpoint == endpoint) {
      return i;
    }
  }
  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t emberAfDeviceTableGetNodeIdFromEui64(EmberEUI64 eui64)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (matchEui64(deviceTable[i].eui64, eui64) ) {
      return deviceTable[i].nodeId;
    }
  }
  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID;
}

bool emberAfDeviceTableGetEui64FromNodeId(EmberNodeId emberNodeId, EmberEUI64 eui64)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == emberNodeId) {
      MEMCOPY(eui64, deviceTable[i].eui64, EUI64_SIZE);
      return true;
    }
  }
  return false;
}

uint16_t emberAfDeviceTableGetIndexFromNodeId(EmberNodeId emberNodeId)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == emberNodeId) {
      return i;
    }
  }
  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t emAfDeviceTableFindFreeDeviceTableIndex(void)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      return i;
    }
  }
  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t emberAfDeviceTableGetEndpointFromNodeIdAndEndpoint(EmberNodeId emberNodeId,
                                                            uint8_t endpoint)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == emberNodeId
        && deviceTable[i].endpoint == endpoint) {
      return i;
    }
  }
  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

void emAfDeviceTableCopyDeviceTableEntry(uint16_t fromIndex, uint16_t toIndex)
{
  EmberAfPluginDeviceTableEntry* from = &(deviceTable[fromIndex]);
  EmberAfPluginDeviceTableEntry* to = &(deviceTable[toIndex]);

  // make sure the fromIndex is in the valud range.
  assert(fromIndex < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  // make sure the fromIndex has a valid entry
  assert(deviceTable[fromIndex].nodeId
         != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID);

  // make sure the toIndex is in the valud range.
  assert(toIndex < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  MEMCOPY(to, from, sizeof(EmberAfPluginDeviceTableEntry));
}

uint8_t emAfDeviceTableNumberOfEndpointsFromIndex(uint16_t index)
{
  uint8_t count = 0;
  uint16_t currentNodeId = emberAfDeviceTableGetNodeIdFromIndex(index);
  uint16_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == currentNodeId) {
      count++;
    }
  }
  return count;
}

static uint16_t findIndexFromNodeIdAndIndex(uint16_t nodeId, uint16_t index)
{
  uint16_t i;
  for (i = index; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (nodeId == emberAfDeviceTableGetNodeIdFromIndex(i)) {
      return i;
    }
  }
  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

static uint16_t findIndexFromEui64AndIndex(EmberEUI64 eui64, uint16_t index)
{
  uint16_t i;
  for (i = index; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (matchEui64(eui64, deviceTable[i].eui64)) {
      return i;
    }
  }
  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

uint16_t emAfDeviceTableFindFirstEndpointNodeId(uint16_t nodeId)
{
  return findIndexFromNodeIdAndIndex(nodeId, 0);
}

uint16_t emAfDeviceTableFindNextEndpoint(uint16_t index)
{
  return findIndexFromEui64AndIndex(deviceTable[index].eui64,
                                    index + 1);
}

uint16_t emAfDeviceTableFindFirstEndpointIeee(EmberEUI64 eui64)
{
  return findIndexFromEui64AndIndex(eui64, 0);
}

uint16_t emberAfDeviceTableGetFirstIndexFromEui64(EmberEUI64 eui64)
{
  return emAfDeviceTableFindFirstEndpointIeee(eui64);
}

EmberAfStatus emAfDeviceTableAddNewEndpoint(uint16_t index, uint8_t newEndpoint)
{
  uint16_t newIndex = emAfDeviceTableFindFreeDeviceTableIndex();

  if (newIndex == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    return EMBER_ZCL_STATUS_FAILURE;
  }

  emAfDeviceTableCopyDeviceTableEntry(index, newIndex);

  deviceTable[newIndex].endpoint = newEndpoint;

  return EMBER_ZCL_STATUS_SUCCESS;
}

uint16_t emAfDeviceTableFindIndexNodeIdEndpoint(uint16_t nodeId,
                                                uint8_t endpoint)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId == nodeId
        && deviceTable[i].endpoint == endpoint) {
      return i;
    }
  }

  return EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX;
}

EmberAfPluginDeviceTableEntry *emberAfDeviceTableFindDeviceTableEntry(uint16_t index)
{
  assert(index < EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID);
  return &(deviceTable[index]);
}

void emAfDeviceTableUpdateNodeId(uint16_t currentNodeId, uint16_t newNodeId)
{
  uint16_t index = emAfDeviceTableFindFirstEndpointNodeId(currentNodeId);

  while (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    deviceTable[index].nodeId = newNodeId;

    index = emAfDeviceTableFindNextEndpoint(index);
  }
}

void emAfDeviceTableUpdateDeviceState(uint16_t index, uint8_t newState)
{
  while (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    deviceTable[index].state = newState;

    index = emAfDeviceTableFindNextEndpoint(index);
  }
}

uint32_t emberAfDeviceTableTimeSinceLastMessage(uint16_t index)
{
  uint32_t timeSinceLastMessage = halCommonGetInt32uMillisecondTick();

  timeSinceLastMessage -= deviceTable[index].lastMsgTimestamp;
  timeSinceLastMessage /= MILLISECOND_TICKS_PER_SECOND;

  return timeSinceLastMessage;
}

// AF Framework callbacks.  This is where the plugin implements the callbacks.
void emberAfPluginDeviceTableInitCallback(void)
{
  emAfDeviceTableInit();

  // Load on Init
  emAfDeviceTableLoad();

  emberAfPluginDeviceTableInitialized();
}

void emberAfPluginDeviceTableStackStatusCallback(EmberStatus status)
{
  // If we leave the network, this plugin needs to clear out all of its device
  // state.

  emberAfCorePrintln("%d %d", status, emberNetworkState());

  if (status == EMBER_NETWORK_DOWN
      && emberNetworkState() == EMBER_NO_NETWORK) {
    emberAfCorePrintln("DeviceTable: Clear State");

    emberAfDeviceTableClear();
  }
}

// --------------------------------
// Save/Load the devices
void emAfDeviceTableSave(void)
{
#if defined(EZSP_HOST) && !defined(EMBER_TEST)
  FILE *fp;
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  uint8_t i;
  uint8_t j;

  // Save device table
  fp = fopen("devices.txt", "w");

  for (i = 0;
       i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
       i++) {
    if (deviceTable[i].nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      fprintf(fp,
              "%x %x %x ",
              deviceTable[i].nodeId,
              deviceTable[i].endpoint,
              deviceTable[i].deviceId);
      for (j = 0; j < 8; j++) {
        fprintf(fp, "%x ", deviceTable[i].eui64[j]);
      }
      for (j = 0; j < EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; j++) {
        fprintf(fp, "%x ", deviceTable[i].clusterIds[j]);
      }
      fprintf(fp, "%d ", deviceTable[i].clusterOutStartPosition);
    }
  }

  // Write ffffffff to mark the end
  fprintf(fp, "\r\nffffffff\r\n");
  fclose(fp);

#endif // defined(EZSP_HOST) && !defined(EMBER_TEST)
}

void emAfDeviceTableLoad(void)
{
#if defined(EZSP_HOST) && !defined(EMBER_TEST)
  uint16_t i;
  uint16_t j;
  FILE *fp;
  unsigned int data, data2, data3;
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();

  fp = fopen("devices.txt", "r");

  if (!fp) {
    return;
  }

  for (i = 0;
       i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE && feof(fp) == false;
       i++) {
    fscanf(fp, "%x %x %x", &data2, &data, &data3);
    deviceTable[i].endpoint = (uint8_t) data;
    deviceTable[i].nodeId = (uint16_t) data2;
    deviceTable[i].deviceId = (uint16_t) data3;

    if (deviceTable[i].nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      for (j = 0; j < 8; j++) {
        fscanf(fp, "%x", &data);
        deviceTable[i].eui64[j] = (uint8_t) data;
      }
      for (j = 0; j < EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; j++) {
        fscanf(fp, "%x", &data);
        deviceTable[i].clusterIds[j] = (uint16_t) data;
      }
      fscanf(fp, "%d", &data);
      deviceTable[i].clusterOutStartPosition = (uint16_t) data;

      deviceTable[i].state = EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED;
    }

    deviceTable[i].lastMsgTimestamp = halCommonGetInt32uMillisecondTick();
  }

  fclose(fp);

  // Set the rest of the device table to null.
  for (; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    deviceTable[i].nodeId = EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID;
  }

#endif // #if defined(EZSP_HOST) && !defined(EMBER_TEST)
}

// --------------------------------
// Message send section
// Command to send the CIE IEEE address to the IAS Zone cluster
void emAfDeviceTableSendCieAddressWrite(EmberNodeId nodeId, uint8_t endpoint)
{
  EmberEUI64 eui64;
  uint8_t outgoingBuffer[15];
  uint32_t i;

  emberAfGetEui64(eui64);

  globalApsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS;
  globalApsFrame.clusterId = ZCL_IAS_ZONE_CLUSTER_ID;
  globalApsFrame.sourceEndpoint = 0x01;
  globalApsFrame.destinationEndpoint = endpoint;

  outgoingBuffer[0] = 0x00;
  outgoingBuffer[1] = emberAfNextSequence();
  outgoingBuffer[2] = ZCL_WRITE_ATTRIBUTES_COMMAND_ID;
  outgoingBuffer[3] = LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID);
  outgoingBuffer[4] = HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID);
  outgoingBuffer[5] = ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE;

  for (i = 0; i < 8; i++) {
    outgoingBuffer[6 + i] = eui64[i];
  }

  emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                     nodeId,
                     &globalApsFrame,
                     14,
                     outgoingBuffer);
}

void emberAfDeviceTableCliIndexSendWithEndpoint(uint16_t index,
                                                uint8_t endpoint)
{
  EmberNodeId nodeId;
  EmberStatus status;

  nodeId = emberAfDeviceTableGetNodeIdFromIndex(index);
  emAfApsFrameEndpointSetup(emberAfPrimaryEndpoint(), endpoint);
  status = emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                              nodeId,
                              &globalApsFrame,
                              appZclBufferLen,
                              appZclBuffer);

  zclCmdIsBuilt = false;
}

void emberAfDeviceTableCliIndexSend(uint16_t index)
{
  uint8_t endpoint = emAfDeviceTableGetFirstEndpointFromIndex(index);
  emberAfDeviceTableCliIndexSendWithEndpoint(index, endpoint);
}

void emberAfDeviceTableSend(EmberEUI64 eui64, uint8_t endpoint)
{
  uint16_t index = emberAfDeviceTableGetFirstIndexFromEui64(eui64);

  if (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    emberAfDeviceTableCliIndexSendWithEndpoint(index, endpoint);
  }
}

void emberAfDeviceTableCommandIndexSendWithEndpoint(uint16_t index,
                                                    uint8_t endpoint)
{
  EmberNodeId nodeId;
  EmberStatus status;

  nodeId = emberAfDeviceTableGetNodeIdFromIndex(index);

  if (emberAfCurrentCommand() == NULL) {
    emAfCommandApsFrame->sourceEndpoint = emberAfPrimaryEndpoint();
  } else {
    emAfCommandApsFrame->sourceEndpoint = emberAfCurrentEndpoint();
  }

  emAfCommandApsFrame->destinationEndpoint = endpoint;
  emberAfCorePrintln("device table send with ep: 0x%2X, %d",
                     nodeId,
                     endpoint);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);

  zclCmdIsBuilt = false;
}

void emberAfDeviceTableCommandIndexSend(uint16_t index)
{
  uint8_t endpoint = emAfDeviceTableGetFirstEndpointFromIndex(index);
  emberAfDeviceTableCommandIndexSendWithEndpoint(index, endpoint);
}

void emberAfDeviceTableCommandSendWithEndpoint(EmberEUI64 eui64,
                                               uint8_t endpoint)
{
  uint16_t index = emberAfDeviceTableGetFirstIndexFromEui64(eui64);
  emberAfDeviceTableCommandIndexSendWithEndpoint(index, endpoint);
}
