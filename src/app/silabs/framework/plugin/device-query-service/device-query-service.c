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
 * @brief Device Query Service
 *
 * 1. After each device announce, add the device to a list of knonwn devices.
 * 2. Query the device for all Active Endpoints (Active Endpoint Request)
 * 3. Query each endpoint for its list of clusters and device information.
 *
 * Also periodically map the network to find all devices.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/device-database/device-database.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

//============================================================================
// Globals

EmberEventControl emberAfPluginDeviceQueryServiceMyEventControl;
static EmberEUI64 currentEui64;
static EmberNodeId currentNodeId = EMBER_NULL_NODE_ID;
static uint8_t currentEndpointIndex;

#define DISCOVERY_DELAY_QS (5 << 2)

#define PLUGIN_NAME "Device-Query-Service"

#if defined(EMBER_AF_PLUGIN_DEVICE_QUERY_SERVICE_AUTO_START)
  #define EMBER_AF_PLUGIN_DEVICE_QUERY_SERVICE_AUTO_START_BOOLEAN true
#else
  #define EMBER_AF_PLUGIN_DEVICE_QUERY_SERVICE_AUTO_START_BOOLEAN false
#endif
static bool enabled = EMBER_AF_PLUGIN_DEVICE_QUERY_SERVICE_AUTO_START_BOOLEAN;

//#define DEBUG_ON
#if defined(DEBUG_ON)
  #define debugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
  #define debugPrint(...) emberAfCorePrint(__VA_ARGS__)
  #define debugPrintEui64(eui64ToPrint) emberAfPrintLittleEndianEui64(eui64ToPrint)
#else
  #define debugPrintln(...)
  #define debugPrint(...)
  #define debugPrintEui64(eui64ToPrint)
#endif

// With device announce there is only the ZDO sequence number, there is no status code.
#define DEVICE_ANNOUNCE_NODE_ID_OFFSET 1
#define DEVICE_ANNOUNCE_EUI64_OFFSET   (DEVICE_ANNOUNCE_NODE_ID_OFFSET + 2)
#define DEVICE_ANNOUNCE_CAPABILITIES_OFFSET (DEVICE_ANNOUNCE_EUI64_OFFSET + EUI64_SIZE)

// Nice names for scheduleEvent()
#define RIGHT_NOW  false
#define WITH_DELAY true

#define RECEIVER_ON_WHEN_IDLE 0x08

// Bit mask in Node Descriptor response signifying which bits convey the
// compliance revision (bits 9-15)
#define SERVER_MASK_STACK_COMPLIANCE_REVISION_MASK          0xFE00
#define SERVER_MASK_STACK_COMPLIANCE_REVISION_BIT_POSITION  9

// We'll try sending up to 3 Node Descriptor requests in hopes of getting a resp
#define NODE_DESCRIPTOR_ATTEMPTS_MAX  3

static uint8_t gNodeDescriptorAttempts;

//============================================================================
// Forward declarations

static void sendActiveEndpointRequest(const EmberAfDeviceInfo* device);
static void sendSimpleDescriptorRequest(const EmberAfDeviceInfo* device);
static void sendNodeDescriptorRequest(const EmberAfDeviceInfo* device);

//============================================================================

void emberAfPluginDeviceQueryServiceGetCurrentDiscoveryTargetEui64(EmberEUI64 returnEui64)
{
  MEMMOVE(returnEui64, currentEui64, EUI64_SIZE);
}

bool emberAfPluginDeviceQueryServiceGetEnabledState(void)
{
  return enabled;
}

void emberAfPluginDeviceQueryServiceEnableDisable(bool enable)
{
  enabled = enable;
  emberEventControlSetInactive(emberAfPluginDeviceQueryServiceMyEventControl);
}

static void clearCurrentDevice(void)
{
  MEMSET(currentEui64, 0xFF, EUI64_SIZE);
  currentNodeId = EMBER_NULL_NODE_ID;
}

void emberAfPluginDeviceQueryServiceInitCallback(void)
{
  debugPrintln("%p init called.", PLUGIN_NAME);
  clearCurrentDevice();
}

static void scheduleEvent(bool withDelay)
{
  if (withDelay) {
    debugPrintln("%p scheduled event for %d qs", PLUGIN_NAME, DISCOVERY_DELAY_QS);
    emberEventControlSetDelayQS(emberAfPluginDeviceQueryServiceMyEventControl,
                                DISCOVERY_DELAY_QS);
  } else {
    emberEventControlSetActive(emberAfPluginDeviceQueryServiceMyEventControl);
  }
}

bool emAfPluginDeviceQueryPreZDOMessageReceived(EmberNodeId sender,
                                                EmberApsFrame* apsFrame,
                                                uint8_t* message,
                                                uint16_t length)
{
  EmberEUI64 tempEui64;
  const EmberAfDeviceInfo *device;
  uint16_t serverMask;
  uint8_t stackRevision;

  if (!enabled) {
    return false;
  }

  if (apsFrame->clusterId == END_DEVICE_ANNOUNCE) {
    MEMMOVE(tempEui64, &(message[DEVICE_ANNOUNCE_EUI64_OFFSET]), EUI64_SIZE);
    // If the device already exists, this call won't overwrite it and will
    // leave its status alone.  Maybe it rejoined and we already know about,
    // in which case we won't bother re-interrogating it.
    device = emberAfPluginDeviceDatabaseAdd(tempEui64,
                                            message[DEVICE_ANNOUNCE_CAPABILITIES_OFFSET]);
    if (device == NULL) {
      emberAfCorePrint("Error: %p failed to add device to database: ",
                       PLUGIN_NAME);
      emberAfPrintLittleEndianEui64(tempEui64);
      emberAfCorePrintln("");
    } else {
      if (device->status == EMBER_AF_DEVICE_DISCOVERY_STATUS_NEW) {
        emberAfCorePrint("%p added device to database: ", PLUGIN_NAME);
        emberAfPrintLittleEndianEui64(tempEui64);
        emberAfCorePrintln(", capabilities: 0x%X", device->capabilities);
        scheduleEvent(WITH_DELAY);
      }
    }

    // returning true here will break the ias-zone-client.
    return false;
  } else if (apsFrame->clusterId == NODE_DESCRIPTOR_RESPONSE) {
    device = emberAfPluginDeviceDatabaseFindDeviceByEui64(currentEui64);
    // This is state machine-driven so we must only process the Node Descriptor
    // response if we were expecting one
    if (device
        && (currentNodeId == sender)
        && (EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_STACK_REVISION == device->status)) {
      serverMask = message[12] | (message[13] << 8);
      serverMask &= SERVER_MASK_STACK_COMPLIANCE_REVISION_MASK;
      stackRevision = (serverMask
                       >> SERVER_MASK_STACK_COMPLIANCE_REVISION_BIT_POSITION);

      debugPrintln("%p got Node Descriptor response from 0x%2X",
                   PLUGIN_NAME,
                   currentNodeId);

      emAfPluginDeviceDatabaseUpdateNodeStackRevision(currentEui64,
                                                      stackRevision);

      // This is the last state - we're done
      emberAfPluginDeviceDatabaseSetStatus(currentEui64,
                                           EMBER_AF_DEVICE_DISCOVERY_STATUS_DONE);
      scheduleEvent(WITH_DELAY);
      clearCurrentDevice();
    }
  }
  return false;
}

static void noteFailedDiscovery(const EmberAfDeviceInfo* device)
{
  emberAfPluginDeviceDatabaseSetStatus(device->eui64, EMBER_AF_DEVICE_DISCOVERY_STATUS_FAILED);
  EMBER_TEST_ASSERT(0 == MEMCOMPARE(device->eui64, currentEui64, EUI64_SIZE));
  clearCurrentDevice();
}

static void serviceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  debugPrintln("%p serviceDiscoveryCallback()", PLUGIN_NAME);
  emberEventControlSetInactive(emberAfPluginDeviceQueryServiceMyEventControl);
  if (!enabled) {
    return;
  }

  const EmberAfDeviceInfo* device = emberAfPluginDeviceDatabaseFindDeviceByEui64(currentEui64);
  if (device == NULL) {
    emberAfCorePrint("Error:  %p could not find device in database with EUI64: ", PLUGIN_NAME);
    emberAfPrintLittleEndianEui64(currentEui64);
    emberAfCorePrintln("");
    clearCurrentDevice();
    scheduleEvent(WITH_DELAY);
    return;
  }

  if (!emberAfHaveDiscoveryResponseStatus(result->status)) {
    emberAfCorePrintln("Error: %p service discovery returned no results.", PLUGIN_NAME);
    noteFailedDiscovery(device);
    scheduleEvent(WITH_DELAY);
    return;
  } else if (result->zdoRequestClusterId == NETWORK_ADDRESS_REQUEST) {
    currentNodeId = result->matchAddress;
    emberAfPluginDeviceDatabaseSetStatus(device->eui64, EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_ENDPOINTS);
    scheduleEvent(RIGHT_NOW);
  } else if (result->zdoRequestClusterId == ACTIVE_ENDPOINTS_REQUEST) {
    const EmberAfEndpointList* listStruct = (const EmberAfEndpointList*)(result->responseData);
    debugPrintln("%p found %d active endpoints.", PLUGIN_NAME, listStruct->count);
    if (!emberAfPluginDeviceDatabaseSetEndpoints(device->eui64,
                                                 listStruct->list,
                                                 listStruct->count)) {
      emberAfCorePrint("Error: %p failed to set endpoints in device database for:", PLUGIN_NAME);
      emberAfPrintLittleEndianEui64(device->eui64);
      emberAfCorePrintln("");
      noteFailedDiscovery(device);
      scheduleEvent(WITH_DELAY);
    } else {
      currentEndpointIndex = 0;
      emberAfPluginDeviceDatabaseSetStatus(device->eui64, EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_CLUSTERS);
      scheduleEvent(RIGHT_NOW);
    }
  } else if (result->zdoRequestClusterId == SIMPLE_DESCRIPTOR_REQUEST) {
    const EmberAfClusterList* clusterInfo = (const EmberAfClusterList*)result->responseData;
    debugPrintln("%p found %d server clusters and %d client clusters on EP %d",
                 PLUGIN_NAME,
                 clusterInfo->inClusterCount,
                 clusterInfo->outClusterCount,
                 clusterInfo->endpoint);
    if (!emberAfPluginDeviceDatabaseSetClustersForEndpoint(device->eui64,
                                                           clusterInfo)) {
      emberAfCorePrintln("Error: %p failed to set clusters for device.", PLUGIN_NAME);
      noteFailedDiscovery(device);
      scheduleEvent(WITH_DELAY);
    } else {
      currentEndpointIndex++;
      scheduleEvent(RIGHT_NOW);
    }
  }
}

static void sendActiveEndpointRequest(const EmberAfDeviceInfo* device)
{
  debugPrintln("%p initiating active endpoint request for: 0x%2X", PLUGIN_NAME, currentNodeId);
  EmberStatus status = emberAfFindActiveEndpoints(currentNodeId, serviceDiscoveryCallback);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error: %p failed to initiate Active EP request (0x%X)", PLUGIN_NAME, status);
    noteFailedDiscovery(device);
    scheduleEvent(WITH_DELAY);
  } else {
    // Don't schedule event, since service discovery returns the results via callback.
    emberAfPluginDeviceDatabaseSetStatus(device->eui64, EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_ENDPOINTS);
  }
}

static void sendSimpleDescriptorRequest(const EmberAfDeviceInfo* device)
{
  uint8_t endpoint = emberAfPluginDeviceDatabaseGetDeviceEndpointFromIndex(device->eui64, currentEndpointIndex);
  if (endpoint == 0xFF) {
    emberAfCorePrintln("%p All endpoints discovered for 0x%2X", PLUGIN_NAME, currentNodeId);
    emberAfPluginDeviceDatabaseSetStatus(device->eui64,
                                         EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_STACK_REVISION);
    gNodeDescriptorAttempts = 0;
    scheduleEvent(RIGHT_NOW);
    return;
  }
  debugPrintln("%p initiating simple descriptor request for: 0x%2X EP %d", PLUGIN_NAME, currentNodeId, endpoint);
  EmberStatus status = emberAfFindClustersByDeviceAndEndpoint(currentNodeId, endpoint, serviceDiscoveryCallback);
  if (status != EMBER_SUCCESS) {
    noteFailedDiscovery(device);
    scheduleEvent(WITH_DELAY);
    return;
  }
  // Don't schedule event, since service discovery returns the results via callback.
}

static void sendNodeDescriptorRequest(const EmberAfDeviceInfo* device)
{
  EmberStatus status;

  if (!device) {
    return;
  }

  // We'll try up to three times to send a Node Descriptor in hopes to receive a
  // response
  if (gNodeDescriptorAttempts >= NODE_DESCRIPTOR_ATTEMPTS_MAX) {
    noteFailedDiscovery(device);
    scheduleEvent(WITH_DELAY);
    return;
  }

  gNodeDescriptorAttempts++;

  debugPrintln("%p initiating node descriptor request for: 0x%2X",
               PLUGIN_NAME,
               currentNodeId);

  status = emberNodeDescriptorRequest(currentNodeId,
                                      EMBER_AF_DEFAULT_APS_OPTIONS);

  if (status != EMBER_SUCCESS) {
    noteFailedDiscovery(device);
    scheduleEvent(WITH_DELAY);
    return;
  }

  scheduleEvent(WITH_DELAY);
}

void emberAfPluginDeviceQueryServiceMyEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginDeviceQueryServiceMyEventControl);
  debugPrintln("%p event", PLUGIN_NAME);
  const EmberAfDeviceInfo* device;
  if (emberAfMemoryByteCompare(currentEui64, EUI64_SIZE, 0xFF)) {
    debugPrintln("%p no current device being queried, looking in database for one.", PLUGIN_NAME);
    device = emberAfPluginDeviceDatabaseFindDeviceByStatus(EMBER_AF_DEVICE_DISCOVERY_STATUS_NEW
                                                           | EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_ENDPOINTS
                                                           | EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_CLUSTERS
                                                           | EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_STACK_REVISION);
    if (device != NULL) {
      debugPrintln("%p found device with status (0x%X): %p",
                   PLUGIN_NAME,
                   device->status,
                   emberAfPluginDeviceDatabaseGetStatusString(device->status));

      if ((device->capabilities & RECEIVER_ON_WHEN_IDLE) == 0) {
        emberAfPluginDeviceDatabaseSetStatus(device->eui64, EMBER_AF_DEVICE_DISCOVERY_STATUS_DONE);
        debugPrintln("%p ignoring sleepy device.", PLUGIN_NAME);
        scheduleEvent(RIGHT_NOW);
        return;
      } else {
        MEMMOVE(currentEui64, device->eui64, EUI64_SIZE);
      }
    }
  } else {
    device = emberAfPluginDeviceDatabaseFindDeviceByEui64(currentEui64);
    if (device == NULL) {
      emberAfCorePrint("Error: %p the current EUI64 does not correspond to any known device: ", PLUGIN_NAME);
      emberAfPrintLittleEndianEui64(currentEui64);
      emberAfCorePrintln("");
      clearCurrentDevice();
      scheduleEvent(WITH_DELAY);
      return;
    }
  }

  if (device == NULL) {
    clearCurrentDevice();

    if (emberAfPluginDeviceDatabaseClearAllFailedDiscoveryStatus(EMBER_AF_PLUGIN_DEVICE_QUERY_SERVICE_MAX_FAILURES)) {
      emberAfCorePrintln("%p Retrying failed discoveries.", PLUGIN_NAME);
      scheduleEvent(WITH_DELAY);
    } else {
      emberAfCorePrintln("%p All known devices discovered.", PLUGIN_NAME);
    }
    return;
  }

  // Although we could consult our local tables for addresses, we perform a broadcast
  // lookup here to insure that we have a current source route back to the destination.
  // The target of the discovery will unicast the result, along with a route record.
  if (currentNodeId == EMBER_NULL_NODE_ID) {
    debugPrint("%p initiating node ID discovery for: ", PLUGIN_NAME);
    debugPrintEui64(currentEui64);
    debugPrintln("");
    EmberStatus status = emberAfFindNodeId(currentEui64, serviceDiscoveryCallback);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("%p failed to initiate node ID discovery.", PLUGIN_NAME);
      noteFailedDiscovery(device);
      scheduleEvent(WITH_DELAY);
    }
    // Else
    //   Don't schedule event, since service discovery callback returns the results.
    return;
  }

  switch (device->status) {
    case EMBER_AF_DEVICE_DISCOVERY_STATUS_NEW:
    case EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_ENDPOINTS:
      sendActiveEndpointRequest(device);
      break;

    case EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_CLUSTERS:
      sendSimpleDescriptorRequest(device);
      break;

    case EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_STACK_REVISION:
      sendNodeDescriptorRequest(device);
      break;

    default:
      break;
  }
}
