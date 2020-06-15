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
// Includes needed for ember related functions for the EZSP host
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

// Device Discovery declaractions
#define EVENT_TICK_MS   50
#define PJOIN_BROADCAST_PERIOD 1

// Number of times to attempt device discovery messages
#define MAX_RELAY_COUNT 10

//definitions for task queues
#define DEVICE_QUEUE_NULL 0xFF
#define MAX_QUEUE_SIZE 128

#define DEVICE_DISCOVERY_ENDPONT_RETRY_TIME_QS   4
#define DEVICE_DISCOVERY_SIMPLE_RETRY_TIME_QS    4
#define DEVICE_DISCOVERY_MAX_WAIT_TIME_S       120

#define DEVICE_DISCOVERY_MAX_WAIT_TICKS \
  ((DEVICE_DISCOVERY_MAX_WAIT_TIME_S * 1000) / EVENT_TICK_MS)

#define DEVICE_DISCOVERY_ENDPOINT_RETRY_TICKS \
  ((DEVICE_DISCOVERY_ENDPONT_RETRY_TIME_QS * 250) / EVENT_TICK_MS)
#define DEVICE_DISCOVERY_SIMPLE_RETRY_TICKS \
  ((DEVICE_DISCOVERY_SIMPLE_RETRY_TIME_QS * 250) / EVENT_TICK_MS)

#define APS_OPTION_DISCOVER EMBER_APS_OPTION_RETRY

// ZDO offsets
#define END_DEVICE_ANNOUNCE_IEEE_OFFSET 3
#define NETWORK_ADDRESS_RESPONSE_IEEE_OFFSET 2
#define NETWORK_ADDRESS_RESPONSE_NODE_ID_OFFSET 10 // 1-status, 8-ieee addr
#define IEEE_ADDRESS_RESPONSE_IEEE_OFFSET 2
#define IEEE_ADDRESS_RESPONSE_NODE_ID_OFFSET 10

// in out cluster types
#define CLUSTER_IN               0
#define CLUSTER_OUT              1
#define NUMBER_OF_CLUSTER_IN_OUT 2

#define EMBER_AF_ZDO_RESPONSE_OVERHEAD 2

//for simple descriptor response
#define SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET                           \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD                                            \
   + 2 /* address of interest */                      + 1 /* length value */ \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET        \
   + 1 /* endpoint */                                \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET + 2)

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET                    \
   + 2  /* device ID length */                                    \
   + 1  /* version (4-bits), reserved (4-bits) */                 \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX + 1)

// For Active endpoint response
#define ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD             \
   + 2) // Address of Interest

#define ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD)

#define ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET \
  (ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET   \
   + 3)

typedef struct {
  EmberEUI64 eui64;
  uint16_t nodeId;
  uint8_t endpoint;
  uint8_t state;
  uint16_t delay;
  uint32_t waitTime;
} DeviceTableQueueEntry;

static uint8_t permitJoinBroadcastCounter = (PJOIN_BROADCAST_PERIOD - 1);

#define newDeviceEventControl emberAfPluginDeviceTableNewDeviceEventControl

static void resetRetry(EmberNodeId nodeId);
EmberEventControl newDeviceEventControl;

enum {
  DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND = 0x00,
  DEVICE_DISCOVERY_STATE_ENDPOINTS_WAITING = 0x01,
  DEVICE_DISCOVERY_STATE_SIMPLE_SEND = 0x02,
  DEVICE_DISCOVERY_STATE_SIMPLE_WAITING = 0x03,
};

#define STATUS_STRINGS_ARRAY_LENGTH 4
static const char * const statusStrings[] =
{
  "STANDARD_SECURITY_SECURED_REJOIN",
  "STANDARD_SECURITY_UNSECURED_JOIN",
  "DEVICE_LEFT",
  "STANDARD_SECURITY_UNSECURED_REJOIN",
};

DeviceTableQueueEntry taskQueue[MAX_QUEUE_SIZE];
static uint8_t queueSize;
static uint32_t tick;

void emberAfPluginDeviceTableIndexAddedCallback(uint16_t index);

// Bookkeeping callbacks
void emAfPluginDeviceTableDeviceLeftCallback(EmberEUI64 newNodeEui64);

static uint8_t getQueueIndexFromNodeAndEndpoint(DeviceTableQueueEntry * queue,
                                                uint16_t nodeId,
                                                uint8_t endpoint)
{
  uint8_t i;

  if ((queue == NULL) || (queueSize == 0)) {
    return DEVICE_QUEUE_NULL;
  }
  for (i = 0; i < queueSize; i++) {
    if ((queue[i].nodeId == nodeId) && (queue[i].endpoint == endpoint)) {
      return i;
    }
  }
  return DEVICE_QUEUE_NULL;
}

static void deleteQueueEntry(DeviceTableQueueEntry * queue, uint8_t index)
{
  uint8_t i;

  if (queueSize == 0) {
    return;
  }

  for (i = index; i < queueSize; i++) {
    queue[i] = queue[i + 1];
  }
  queueSize--;
  queue[queueSize].nodeId = 0;
  queue[queueSize].state = 0;
  queue[queueSize].delay = 0;
  queue[queueSize].waitTime = 0;
  MEMSET(queue[queueSize].eui64, 0, EUI64_SIZE);
}

static void bubbleSortQueue(DeviceTableQueueEntry * queue)
{
  uint8_t passesIndex, stepsIndex;
  DeviceTableQueueEntry swapDeviceEntry;

  if (queueSize == 0) {
    return;
  }
  for (passesIndex = 0; passesIndex < (queueSize - 1); passesIndex++) {
    for (stepsIndex = 0;
         stepsIndex < (queueSize - passesIndex - 1);
         stepsIndex++) {
      if (queue[stepsIndex].delay > queue[stepsIndex + 1].delay) {
        swapDeviceEntry = queue[stepsIndex];
        queue[stepsIndex] = queue[stepsIndex + 1];
        queue[stepsIndex + 1] = swapDeviceEntry;
      }
    }
  }
}

void printQueue(void)
{
  uint8_t i;

  emberAfCorePrintln("index    node  ep  state  delay  wait");
  for (i = 0; i < queueSize; i++) {
    emberAfCorePrintln("  %d   0x%2X  %d    %d     %d    %d",
                       i,
                       taskQueue[i].nodeId,
                       taskQueue[i].endpoint,
                       taskQueue[i].state,
                       taskQueue[i].delay,
                       taskQueue[i].waitTime);
  }
}

static void updateQueueEntry(DeviceTableQueueEntry * queue,
                             uint8_t index,
                             uint8_t nextState,
                             uint8_t delay)
{
  queue[index].state = nextState;
  queue[index].delay = delay;
  bubbleSortQueue(queue);
}

static bool addQueueEntry(DeviceTableQueueEntry * queue,
                          DeviceTableQueueEntry * newEntryPtr)
{
  if (queueSize == MAX_QUEUE_SIZE) {
    return false;
  }

  queue[queueSize] = *newEntryPtr;
  queueSize++;
  bubbleSortQueue(queue);

  if (queueSize == 1) { // this is the first entry and the state machine is inactive
                        // we have to activated it.
    emberEventControlSetActive(newDeviceEventControl);
  }
  return true;
}

static void updateTaskQueueByTick(void)
{
  uint8_t i = 0;

  while (i < queueSize) {
    taskQueue[i].waitTime++;
    if (taskQueue[i].delay != 0) {
      taskQueue[i].delay--;
    }
    if (taskQueue[i].waitTime > DEVICE_DISCOVERY_MAX_WAIT_TICKS) {
      deleteQueueEntry(taskQueue, i);
    } else {
      i++;
    }
  }
}

static void routeRepairReturn(const EmberAfServiceDiscoveryResult* result)
{
  emberAfCorePrintln("DISCOVERY ROUTE REPAIR RETURN RESULT: status=%d",
                     result->status);
}

// We have a new endpoint.
static void newEndpointDiscovered(EmberAfPluginDeviceTableEntry *p_entry)
{
  // Figure out if we need to do anything, like write the CIE address to it.
  if (p_entry->deviceId == DEVICE_ID_IAS_ZONE) {
    // write IEEE address to CIE address location
    emAfDeviceTableSendCieAddressWrite(p_entry->nodeId, p_entry->endpoint);
  }
  p_entry->state = EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED;
  // New device is set, time to make the callback to indicate a new device
  // has joined.
  emberAfPluginDeviceTableNewDeviceCallback(p_entry->eui64);
  emAfDeviceTableSave();
}

void emberAfPluginDeviceTableNewDeviceEventHandler(void)
{
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  DeviceTableQueueEntry * currentEntryPtr = &taskQueue[0];
  EmberStatus status;
  uint16_t newEndpoint;
  uint8_t taskQueueIndex, nextState, delay;

  if ((currentEntryPtr->delay == 0) && (queueSize > 0)) {
    emberAfPrintBigEndianEui64(currentEntryPtr->eui64);
    taskQueueIndex = getQueueIndexFromNodeAndEndpoint(taskQueue,
                                                      currentEntryPtr->nodeId,
                                                      currentEntryPtr->endpoint);
    switch (currentEntryPtr->state) {
      case DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND:
        // send out active endpoints request.
        status = emberActiveEndpointsRequest(currentEntryPtr->nodeId,
                                             APS_OPTION_DISCOVER);
        nextState = DEVICE_DISCOVERY_STATE_ENDPOINTS_WAITING;
        delay = DEVICE_DISCOVERY_ENDPOINT_RETRY_TICKS;
        break;
      case DEVICE_DISCOVERY_STATE_ENDPOINTS_WAITING:
        emberAfFindNodeId(currentEntryPtr->eui64, routeRepairReturn);
        nextState = DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND;
        delay = DEVICE_DISCOVERY_ENDPOINT_RETRY_TICKS;
        break;
      case DEVICE_DISCOVERY_STATE_SIMPLE_SEND:
        emberSimpleDescriptorRequest(currentEntryPtr->nodeId,
                                     currentEntryPtr->endpoint,
                                     EMBER_AF_DEFAULT_APS_OPTIONS);
        nextState = DEVICE_DISCOVERY_STATE_SIMPLE_WAITING;
        delay = DEVICE_DISCOVERY_SIMPLE_RETRY_TICKS;
        break;
      case DEVICE_DISCOVERY_STATE_SIMPLE_WAITING:
        emberAfFindNodeId(currentEntryPtr->eui64, routeRepairReturn);
        nextState = DEVICE_DISCOVERY_STATE_SIMPLE_SEND;
        delay = DEVICE_DISCOVERY_SIMPLE_RETRY_TICKS;
        break;
      default:
        break;
    }
    updateQueueEntry(taskQueue, taskQueueIndex, nextState, delay);
  }
  if (queueSize > 0) {
    emberEventControlSetDelayMS(newDeviceEventControl, EVENT_TICK_MS);
    updateTaskQueueByTick();
  } else {
    emberEventControlSetInactive(newDeviceEventControl);
  }
}

static void newDeviceParseActiveEndpointsResponse(EmberNodeId emberNodeId,
                                                  EmberApsFrame* apsFrame,
                                                  uint8_t* message,
                                                  uint16_t length)
{
  uint8_t i;
  DeviceTableQueueEntry entry;

  // Make sure I have not used the redundant endpoint response
  // check if there is any active endpoint that is being processed
  i = getQueueIndexFromNodeAndEndpoint(taskQueue,
                                       emberNodeId,
                                       DEVICE_TABLE_UNKNOWN_ENDPOINT);
  if (i != DEVICE_QUEUE_NULL) {
    MEMCOPY(entry.eui64, taskQueue[i].eui64, EUI64_SIZE);
    deleteQueueEntry(taskQueue, i);//delete the ep initial task
    entry.nodeId = emberNodeId;
    entry.state = DEVICE_DISCOVERY_STATE_SIMPLE_SEND;
    entry.delay = 0;
    entry.waitTime = 0;
    emberAfCorePrintln("number of ep: %d",
                       message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET]);
    for (i = 0; i < message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET]; i++) {
      entry.endpoint = message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i];
      emberAfCorePrintln("ep: %d", message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i]);
      addQueueEntry(taskQueue, &entry);
    }
  }
}

#ifdef EMBER_AF_PLUGIN_GATEWAY_RELAY_COAP
void emberAfGatewayRelayCoapParseClusterList(uint16_t device_index,
                                             uint8_t *message,
                                             uint16_t length);
#endif

static void newDeviceParseSimpleDescriptorResponse(EmberNodeId nodeId,
                                                   uint8_t* message,
                                                   uint16_t length)
{
  uint8_t endpoint;
  EmberAfPluginDeviceTableEntry *pEntry;
  uint8_t clusterIndex = 0;
  uint16_t endpointIndex;
  uint8_t i, currentClusterType, ClusterCount;
  uint8_t msgArrayIndex = SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX;

  endpoint = message[SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET];

  //if we can find a entry in device table with same nodeId and ep, then update
  //it else we add a new one.
  endpointIndex = emberAfDeviceTableGetEndpointFromNodeIdAndEndpoint(nodeId,
                                                                     endpoint);
  if (endpointIndex == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    endpointIndex = emAfDeviceTableFindFreeDeviceTableIndex();
    if (endpointIndex == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
      // Error case... no more room in the index table
      emberAfCorePrintln("Error: Device Table Full");
      return;
    }
    emberAfPluginDeviceTableIndexAddedCallback(endpointIndex);
  }
  pEntry = emberAfDeviceTableFindDeviceTableEntry(endpointIndex);
  pEntry->deviceId =
    emberFetchLowHighInt16u(message
                            + SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET);

  ClusterCount = message[SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX];

  for (currentClusterType = CLUSTER_IN;
       currentClusterType < NUMBER_OF_CLUSTER_IN_OUT;
       currentClusterType++) {
    for (i = 0; i < ClusterCount; i++) {
      pEntry->clusterIds[clusterIndex] =
        HIGH_LOW_TO_INT(message[msgArrayIndex + 1], message[msgArrayIndex]);
      clusterIndex++;
      msgArrayIndex += 2; //advance the index by 2 bytes for each 16 bit
                          //cluster id
    }
    if (currentClusterType == CLUSTER_IN) {
      pEntry->clusterOutStartPosition = clusterIndex;
      // This is the output cluster count
      ClusterCount = message[msgArrayIndex++];
    }
  }

  i = getQueueIndexFromNodeAndEndpoint(taskQueue, nodeId, endpoint);
  if (i != DEVICE_QUEUE_NULL) {
    MEMCOPY(pEntry->eui64, taskQueue[i].eui64, EUI64_SIZE);
    pEntry->nodeId = taskQueue[i].nodeId;
    pEntry->endpoint = taskQueue[i].endpoint;
    deleteQueueEntry(taskQueue, i);
    newEndpointDiscovered(pEntry);
  }
}

// two things have to be handled here:
// 1.) new device joined
// 2.) node Id has been changed.
void emberAfDeviceTableNewDeviceJoinHandler(EmberNodeId newNodeId,
                                            EmberEUI64 newNodeEui64)
{
  uint16_t deviceTableIndex = emberAfDeviceTableGetFirstIndexFromEui64(newNodeEui64);
  uint8_t i;
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  DeviceTableQueueEntry entry;

  if (deviceTableIndex == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    //search is there any old task in the taskQueue that matches the eui64,
    //then we should not do anything in that case
    for (i = 0; i < queueSize; i++) {
      if (emberAfDeviceTableMatchEui64(taskQueue[i].eui64, newNodeEui64)) {
        deleteQueueEntry(taskQueue, i);
      }
    }
    //add new task
    entry.endpoint = DEVICE_TABLE_UNKNOWN_ENDPOINT;
    entry.nodeId = newNodeId;
    entry.state = DEVICE_DISCOVERY_STATE_ENDPOINTS_SEND;
    entry.delay = 0;
    entry.waitTime = 0;
    MEMCOPY(entry.eui64, newNodeEui64, EUI64_SIZE);
    addQueueEntry(taskQueue, &entry);
  } else {
    // Is this a new node ID?
    if (newNodeId != deviceTable[deviceTableIndex].nodeId) {
      emberAfCorePrintln("Node ID Change:  was %2x, is %2x",
                         deviceTable[deviceTableIndex].nodeId,
                         newNodeId);

      emAfDeviceTableUpdateNodeId(deviceTable[deviceTableIndex].nodeId,
                                  newNodeId);

      // Test code for failure to see leave request.
      uint16_t endpointIndex =
        emberAfDeviceTableGetEndpointFromNodeIdAndEndpoint(
          deviceTable[deviceTableIndex].nodeId,
          deviceTable[deviceTableIndex].endpoint);

      if (endpointIndex == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
        return;
      }

      // New device is set, time to make the callback to indicate a new device
      // has joined.
      emberAfPluginDeviceTableRejoinDeviceCallback(deviceTable[deviceTableIndex].eui64);
      // Need to save when the node ID changes.
      emAfDeviceTableSave();
    }
  }
}

static void newDeviceLeftHandler(EmberEUI64 newNodeEui64)
{
  uint16_t index = emberAfDeviceTableGetFirstIndexFromEui64(newNodeEui64);

  if (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    emAfPluginDeviceTableDeviceLeftCallback(newNodeEui64);
    emAfPluginDeviceTableDeleteEntry(index);
    // Save on Node Left
    emAfDeviceTableSave();
  }

  //search and delete any pending task in the taskQueue that matches the eui64
  for (index = 0; index < queueSize; index++) {
    if (emberAfDeviceTableMatchEui64(taskQueue[index].eui64, newNodeEui64)) {
      deleteQueueEntry(taskQueue, index);
    }
  }
}

static EmberStatus broadcastPermitJoin(uint8_t duration)
{
  permitJoinBroadcastCounter++;
  EmberStatus status;

  if (permitJoinBroadcastCounter == PJOIN_BROADCAST_PERIOD) {
    uint8_t data[3] = { 0,   // sequence number (filled in later)
                        0,   // duration (filled in below)
                        0 }; // TC significance (not used)
    permitJoinBroadcastCounter = 0;

    data[1] = duration;
    status = emberSendZigDevRequest(EMBER_BROADCAST_ADDRESS,
                                    PERMIT_JOINING_REQUEST,
                                    0,   // APS options
                                    data,
                                    3);  // length
  } else {
    status = 0;
  }

  return status;
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or
 * ezspTrustCenterJoinHandler. This callback provides the same arguments
 * passed to the TrustCenterJoinHandler. For more information about the
 * TrustCenterJoinHandler please see documentation included in
 * stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  uint8_t i;

  emberAfCorePrintln("TC Join Callback %2x , decision: %d, status: %d",
                     newNodeId,
                     decision,
                     status);

  for (i = 0; i < 8; i++) {
    emberAfCorePrint("%x",
                     newNodeEui64[7 - i]);
  }
  if (status < STATUS_STRINGS_ARRAY_LENGTH) {
    emberAfCorePrintln(" %s", statusStrings[status]);
  } else {
    emberAfCorePrintln(" %d", status);
  }

  switch (status) {
    case EMBER_STANDARD_SECURITY_UNSECURED_JOIN:
      // Broadcast permit joining to new router as it joins.
      broadcastPermitJoin(254);
      emberAfCorePrintln("new device line %d", __LINE__);
      emberAfDeviceTableNewDeviceJoinHandler(newNodeId, newNodeEui64);
      break;
    case EMBER_DEVICE_LEFT:
      newDeviceLeftHandler(newNodeEui64);
      break;
    default:
      // If the device is in the left sent state, we want to send another
      // left message.
      if (emAfDeviceTableShouldDeviceLeave(newNodeId)) {
        return;
      } else {
        emberAfCorePrintln("new device line %d", __LINE__);
        emberAfDeviceTableNewDeviceJoinHandler(newNodeId, newNodeEui64);
      }
      break;
  }

  // If a new device did an unsecure join, we need to turn on permit joining,
  // as there may be more coming
  if (status == EMBER_STANDARD_SECURITY_UNSECURED_JOIN) {
    // Broadcast permit joining to new router as it joins.
    broadcastPermitJoin(254);
  }
}

/** @brief Pre ZDO Message Received
 *
 * This function passes the application an incoming ZDO message and gives the
 * appictation the opportunity to handle it. By default, this callback returns
 * false indicating that the incoming ZDO message has not been handled and
 * should be handled by the Application Framework.
 *
 * @param emberNodeId   Ver.: always
 * @param apsFrame   Ver.: always
 * @param message   Ver.: always
 * @param length   Ver.: always
 */
bool emAfPluginDeviceTablePreZDOMessageReceived(EmberNodeId emberNodeId,
                                                EmberApsFrame* apsFrame,
                                                uint8_t* message,
                                                uint16_t length)
{
  EmberNodeId ieeeSourceNode, remoteDeviceNwkId;

  emberAfCorePrintln("%2x:  ", emberNodeId);
  switch (apsFrame->clusterId) {
    case ACTIVE_ENDPOINTS_RESPONSE:
      emberAfCorePrintln("Active Endpoints Response");
      newDeviceParseActiveEndpointsResponse(emberNodeId, apsFrame, message, length);
      return false;
      break;
    case SIMPLE_DESCRIPTOR_RESPONSE:
      emberAfCorePrintln("Simple Descriptor Response");

      newDeviceParseSimpleDescriptorResponse(emberNodeId,
                                             message,
                                             length);
      return false;
      break;
    case END_DEVICE_ANNOUNCE:
      // Any time an end device announces, we need to see if we have to update
      // the device handler.
      emberAfCorePrintln("new device line %d", __LINE__);
      emberAfDeviceTableNewDeviceJoinHandler(emberNodeId,
                                             message + END_DEVICE_ANNOUNCE_IEEE_OFFSET);
      break;
    case PERMIT_JOINING_RESPONSE:
      break;
    case LEAVE_RESPONSE:
      break;
    case BIND_RESPONSE:
      break;
    case BINDING_TABLE_RESPONSE:
      break;
    case NETWORK_ADDRESS_RESPONSE:
      emberAfCorePrintln("new device line %d", __LINE__);
      remoteDeviceNwkId =
        emberFetchLowHighInt16u(message + NETWORK_ADDRESS_RESPONSE_NODE_ID_OFFSET);
      emberAfDeviceTableNewDeviceJoinHandler(remoteDeviceNwkId,
                                             message + NETWORK_ADDRESS_RESPONSE_IEEE_OFFSET);
      break;
    case IEEE_ADDRESS_RESPONSE:
      emberAfCorePrintln("new device line %d", __LINE__);
      ieeeSourceNode =
        emberFetchLowHighInt16u(message + IEEE_ADDRESS_RESPONSE_NODE_ID_OFFSET);
      emberAfCorePrintln("Ieee source node %2x", ieeeSourceNode);

      emberAfDeviceTableNewDeviceJoinHandler(ieeeSourceNode,
                                             message + IEEE_ADDRESS_RESPONSE_IEEE_OFFSET);
      break;
    default:
      emberAfCorePrintln("Untracked ZDO %2x", apsFrame->clusterId);
      break;
  }

  emberAfCorePrint("%2x ", emberNodeId);
  emAfDeviceTablePrintBuffer(message, length);

  return false;
}
