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
#include "app/util/zigbee-framework/zigbee-device-host.h"
#include <stdlib.h>
#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/device-table/device-table-internal.h"

// Device tracking declarations
static void optionallyChangeState(EmberNodeId nodeId,
                                  EmberAfPluginDeviceTableDeviceState state);
static EmberAfPluginDeviceTableDeviceState getCurrentState(EmberNodeId nodeId);

// --------------------------------
// Route Repair functions
static void serviceReturn(const EmberAfServiceDiscoveryResult* result)
{
  emberAfCorePrintln("ROUTE REPAIR SERVICE RETURN RESULT: status=%d",
                     result->status);
}

bool emAfPluginDeviceTablePreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
  uint16_t nodeId = cmd->source;
  uint32_t currentTimestamp = halCommonGetInt32uMillisecondTick();
  uint32_t index = emAfDeviceTableFindFirstEndpointNodeId(nodeId);
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();

  if (getCurrentState(nodeId)
      >= EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT
      && getCurrentState(nodeId) != EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN) {
    // We have heard from a device that should have left but hasn't.  Try
    // re-sending the leave message.
    emberAfPluginDeviceTableSendLeave(emberAfDeviceTableGetIndexFromNodeId(nodeId));
    return false;
  }

  optionallyChangeState(nodeId, EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED);

  while (index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    deviceTable[index].lastMsgTimestamp = currentTimestamp;
    index = emAfDeviceTableFindNextEndpoint(index);
  }

  return false;
}

void emberAfPluginDeviceTableInitiateRouteRepair(EmberNodeId nodeId)
{
  EmberEUI64 eui64;

  // We only attempt to repair the situation where the node ID has changed.
  // To do that, simply send out a node ID request.
  if (emberAfDeviceTableGetEui64FromNodeId(nodeId, eui64)) {
    emberAfFindNodeId(eui64, serviceReturn);
  }
}

// --------------------------------
// device state tracking
void emberAfPluginDeviceTableSendLeave(uint16_t index)
{
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  uint16_t nodeId = deviceTable[index].nodeId;
  EmberEUI64 destinationEui64;

  // make sure this is a valid index value
  assert(index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);
  // make sure this is a valid device table entry
  assert(nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID);

  EmberApsOption apsOptions = EMBER_APS_OPTION_RETRY
                              | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY
                              | EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY;

  // Copy elements from device table
  MEMCOPY(destinationEui64, deviceTable[index].eui64, sizeof(EmberEUI64));

  // Use the ZDO command to remove the device
  emberLeaveRequest(nodeId,
                    destinationEui64,
                    0x00,        // Just leave.  Do not remove children, if any.
                    apsOptions);
  emberAfCorePrintln("LEAVE_SENT");

  optionallyChangeState(deviceTable[index].nodeId, EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT);
}

static void handleUnknownDevice(EmberNodeId nodeId)
{
  // To do that, we need to send an IEEE request based on the node ID.
  emberIeeeAddressRequest(nodeId,
                          false,         // report kids?
                          0,             // child start index
                          EMBER_APS_OPTION_RETRY);
}

static EmberAfPluginDeviceTableDeviceState getCurrentState(EmberNodeId nodeId)
{
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  uint16_t index = emberAfDeviceTableGetIndexFromNodeId(nodeId);

  if (index == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    return EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN;
  }

  return deviceTable[index].state;
}

// Handle state transitions.  If the requested state is different from the
// current state, we need to call the state change callback.
static void optionallyChangeState(EmberNodeId nodeId,
                                  EmberAfPluginDeviceTableDeviceState state)
{
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  uint16_t index = emberAfDeviceTableGetIndexFromNodeId(nodeId);
  EmberAfPluginDeviceTableDeviceState originalState;

  //if we have a 0x0000 NodeID, we shouldn't do anything
  if (nodeId == 0x0000) {
    return;
  }
  if (index == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
    // We don't know this device.  Kick off a discovery process here.
    handleUnknownDevice(nodeId);
    return;
  }

  originalState = deviceTable[index].state;

  if (originalState < EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED) {
    // Still in discovery mode...do nothing
    return;
  }

  if (originalState != state) {
    // Need to change state on all endpoints
    emAfDeviceTableUpdateDeviceState(index, state);
    emberAfPluginDeviceTableStateChangeCallback(deviceTable[index].nodeId,
                                                state);
  }
}

static bool isZdoLeaveCommand(uint16_t profileId,
                              uint16_t clusterId)
{
  return profileId == EMBER_ZDO_PROFILE_ID
         && clusterId == LEAVE_REQUEST ? true : false;
}

// If we send a message to a device and it was successful, then we label the
// device as joined.  If we send a message to a devcie and it was unsuccessful,
// change state to unresponsive.  Track whether it was successful here.
void emberAfPluginDeviceTableMessageSentStatus(EmberNodeId nodeId,
                                               EmberStatus status,
                                               uint16_t profileId,
                                               uint16_t clusterId)
{
  // Do not transition if we are in the leaving state or if we are sending to an
  // unknown node.
  if ((getCurrentState(nodeId)
       == EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT)
      || (getCurrentState(nodeId)
          == EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN)) {
    return;
  }

  // See if we sent a leave command, if so always change the state to LEAVE_SENT
  // (because we still want to keep track of this even if the leave didn't
  // succeed). Otherwise check the status of the last message and determine if
  // the device is JOINED or UNRESPONSIVE.
  if (isZdoLeaveCommand(profileId, clusterId)) {
    optionallyChangeState(nodeId,
                          EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT);
  } else {
    if (status == EMBER_SUCCESS) {
      optionallyChangeState(nodeId, EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED);
    } else {
      optionallyChangeState(nodeId,
                            EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_UNRESPONSIVE);
    }
  }
}

static bool shouldDeviceLeave(EmberNodeId nodeId)
{
  if (getCurrentState(nodeId)
      == EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT) {
    emberAfPluginDeviceTableSendLeave(emberAfDeviceTableGetIndexFromNodeId(nodeId));
    return true;
  }

  return false;
}

bool emAfDeviceTableShouldDeviceLeave(EmberNodeId nodeId)
{
  return shouldDeviceLeave(nodeId);
}
