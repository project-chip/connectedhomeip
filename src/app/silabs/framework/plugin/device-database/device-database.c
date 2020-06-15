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
 * @brief A list of all devices known in the network and their services.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/device-database/device-database.h"

//============================================================================
// Globals

static EmberAfDeviceInfo deviceDatabase[EMBER_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES];

#define INVALID_INDEX 0xFFFF

#define PLUGIN_NAME "Device-DB"

//============================================================================
// Forward Declarations

void emberAfPluginDeviceDatabaseDiscoveryCompleteCallback(const EmberAfDeviceInfo* device);

//============================================================================

void emberAfPluginDeviceDatabaseInitCallback(void)
{
  MEMSET(deviceDatabase, 0xFF, sizeof(EmberAfDeviceInfo) * EMBER_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES);
}

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseGetDeviceByIndex(uint16_t index)
{
  if (index > EMBER_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES) {
    return NULL;
  }

  if (emberAfMemoryByteCompare(deviceDatabase[index].eui64, EUI64_SIZE, 0xFF)) {
    return NULL;
  }

  return &(deviceDatabase[index]);
}

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseFindDeviceByStatus(EmberAfDeviceDiscoveryStatus status)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES; i++) {
    if (!emberAfMemoryByteCompare(deviceDatabase[i].eui64, EUI64_SIZE, 0xFF)
        && (deviceDatabase[i].status & status)) {
      return &(deviceDatabase[i]);
    }
  }
  return NULL;
}

static EmberAfDeviceInfo* findDeviceByEui64(const EmberEUI64 eui64)
{
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES; i++) {
    if (0 == MEMCOMPARE(eui64, deviceDatabase[i].eui64, EUI64_SIZE)) {
      return &(deviceDatabase[i]);
    }
  }
  return NULL;
}

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseFindDeviceByEui64(EmberEUI64 eui64)
{
  return findDeviceByEui64(eui64);
}

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseAddDeviceWithAllInfo(const EmberAfDeviceInfo* newDevice)
{
  if (NULL != findDeviceByEui64(newDevice->eui64)) {
    emberAfCorePrint("Error: %p cannot add device that already exists: ", PLUGIN_NAME);
    emberAfPrintLittleEndianEui64(newDevice->eui64);
    emberAfCorePrintln("");
    return NULL;
  }
  EmberEUI64 nullEui64;
  MEMSET(nullEui64, 0xFF, EUI64_SIZE);
  EmberAfDeviceInfo* device = findDeviceByEui64(nullEui64);
  if (device != NULL) {
    MEMMOVE(device, newDevice, sizeof(EmberAfDeviceInfo));
    emberAfPluginDeviceDatabaseDiscoveryCompleteCallback(device);
  }
  return device;
}

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseAdd(EmberEUI64 eui64, uint8_t zigbeeCapabalities)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(eui64);
  if (device == NULL) {
    EmberEUI64 nullEui64;
    MEMSET(nullEui64, 0xFF, EUI64_SIZE);
    device = findDeviceByEui64(nullEui64);
    if (device != NULL) {
      MEMMOVE(device->eui64, eui64, EUI64_SIZE);
      device->status = EMBER_AF_DEVICE_DISCOVERY_STATUS_NEW;
      device->discoveryFailures = 0;
      device->capabilities = zigbeeCapabalities;
      device->endpointCount = 0;
    }
  }
  return device;
}

bool emberAfPluginDeviceDatabaseEraseDevice(EmberEUI64 eui64)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(eui64);
  if (device != NULL) {
    MEMSET(device, 0xFF, sizeof(EmberAfDeviceInfo));
    return true;
  }
  return false;
}

bool emberAfPluginDeviceDatabaseSetEndpoints(const EmberEUI64 eui64,
                                             const uint8_t* endpointList,
                                             uint8_t endpointCount)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(eui64);
  if (device == NULL) {
    emberAfCorePrint("Error: %p cannot add endpoints.  No such device in database: ");
    emberAfPrintLittleEndianEui64(eui64);
    emberAfCorePrintln("");
    return false;
  }

  // Clear all existing endpoints so there is no leftover clusters or endpoints.
  MEMSET(device->endpoints,
         0xFF,
         sizeof(EmberAfEndpointInfoStruct) * EMBER_AF_MAX_ENDPOINTS_PER_DEVICE);

  device->endpointCount = (endpointCount < EMBER_AF_MAX_ENDPOINTS_PER_DEVICE
                           ? endpointCount
                           : EMBER_AF_MAX_ENDPOINTS_PER_DEVICE);

  uint8_t i;
  for (i = 0; i < device->endpointCount; i++) {
    device->endpoints[i].endpoint = *endpointList;
    device->endpoints[i].clusterCount = 0;
    endpointList++;
  }
  return true;
}

static uint8_t findEndpoint(EmberAfDeviceInfo* device, uint8_t endpoint)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_MAX_ENDPOINTS_PER_DEVICE; i++) {
    if (endpoint == device->endpoints[i].endpoint) {
      return i;
    }
  }
  return 0xFF;
}

uint8_t emberAfPluginDeviceDatabaseGetDeviceEndpointFromIndex(const EmberEUI64 eui64,
                                                              uint8_t index)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(eui64);
  if (device != NULL
      && index < EMBER_AF_MAX_ENDPOINTS_PER_DEVICE) {
    return device->endpoints[index].endpoint;
  }
  return 0xFF;
}

uint8_t emberAfPluginDeviceDatabaseGetIndexFromEndpoint(uint8_t endpoint,
                                                        const EmberEUI64 eui64)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(eui64);
  uint8_t index = (device != NULL
                   ? findEndpoint(device, endpoint)
                   : 0xFF);
  return index;
}

bool emberAfPluginDeviceDatabaseSetClustersForEndpoint(const EmberEUI64 eui64,
                                                       const EmberAfClusterList* clusterList)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(eui64);
  uint8_t index =  emberAfPluginDeviceDatabaseGetIndexFromEndpoint(clusterList->endpoint, eui64);
  if (index == 0xFF) {
    emberAfCorePrintln("Error: %p endpoint %d does not exist for device.", PLUGIN_NAME, clusterList->endpoint);
    return false;
  }

  uint8_t doServer;
  device->endpoints[index].profileId = clusterList->profileId;
  device->endpoints[index].deviceId = clusterList->deviceId;
  device->endpoints[index].clusterCount = clusterList->inClusterCount + clusterList->outClusterCount;
  if (device->endpoints[index].clusterCount > EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT) {
    emberAfCorePrintln("%p too many clusters (%d) for endpoint.  Limiting to %d",
                       PLUGIN_NAME,
                       device->endpoints[index].clusterCount,
                       EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT);
    device->endpoints[index].clusterCount = EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT;
  }
  uint8_t deviceClusterIndex = 0;
  for (doServer = 0; doServer < 2; doServer++) {
    uint8_t clusterPointerIndex;
    uint8_t count = (doServer ? clusterList->inClusterCount : clusterList->outClusterCount);
    const uint16_t* clusterPointer = (doServer ? clusterList->inClusterList : clusterList->outClusterList);

    for (clusterPointerIndex = 0;
         (clusterPointerIndex < count)
         && (deviceClusterIndex < device->endpoints[index].clusterCount);
         clusterPointerIndex++) {
      device->endpoints[index].clusters[deviceClusterIndex].clusterId = clusterPointer[clusterPointerIndex];
      device->endpoints[index].clusters[deviceClusterIndex].server = (doServer ? true : false);
      deviceClusterIndex++;
    }
  }
  return true;
}

bool emberAfPluginDeviceDatabaseClearAllFailedDiscoveryStatus(uint8_t maxFailureCount)
{
  bool atLeastOneCleared = false;
  uint16_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES; i++) {
    if (emberAfMemoryByteCompare(deviceDatabase[i].eui64, EUI64_SIZE, 0xFF)) {
      continue;
    }
    if (EMBER_AF_DEVICE_DISCOVERY_STATUS_FAILED == deviceDatabase[i].status
        && deviceDatabase[i].discoveryFailures < maxFailureCount) {
      deviceDatabase[i].status = EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_ENDPOINTS;
      atLeastOneCleared = true;
    }
  }
  return atLeastOneCleared;
}

bool emberAfPluginDeviceDatabaseSetStatus(const EmberEUI64 deviceEui64, EmberAfDeviceDiscoveryStatus newStatus)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(deviceEui64);
  if (device != NULL) {
    device->status = newStatus;
    if (device->status == EMBER_AF_DEVICE_DISCOVERY_STATUS_FAILED) {
      device->discoveryFailures++;
    } else if (device->status == EMBER_AF_DEVICE_DISCOVERY_STATUS_DONE) {
      emberAfPluginDeviceDatabaseDiscoveryCompleteCallback(device);
    }
    return true;
  }
  return false;
}

static void doesDeviceHaveCluster(const EmberAfDeviceInfo* device,
                                  EmberAfClusterId clusterToFind,
                                  bool server,
                                  uint8_t* returnEndpoint)
{
  uint8_t i;
  *returnEndpoint = EMBER_AF_INVALID_ENDPOINT;
  for (i = 0; i < EMBER_AF_MAX_ENDPOINTS_PER_DEVICE; i++) {
    uint8_t j;
    for (j = 0; j < EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT; j++) {
      if (device->endpoints[i].clusters[j].clusterId == clusterToFind
          && device->endpoints[i].clusters[j].server == server) {
        *returnEndpoint = device->endpoints[i].endpoint;
        return;
      }
    }
  }
}

EmberStatus emberAfPluginDeviceDatabaseDoesDeviceHaveCluster(EmberEUI64 deviceEui64,
                                                             EmberAfClusterId clusterToFind,
                                                             bool server,
                                                             uint8_t* returnEndpoint)
{
  EmberAfDeviceInfo* device = findDeviceByEui64(deviceEui64);
  if (device == NULL) {
    return EMBER_INVALID_CALL;
  }
  doesDeviceHaveCluster(device, clusterToFind, server, returnEndpoint);
  return EMBER_SUCCESS;
}

void emberAfPluginDeviceDatabaseCreateNewSearch(EmberAfDeviceDatabaseIterator* iterator)
{
  iterator->deviceIndex = 0;
}

EmberStatus emberAfPluginDeviceDatabaseFindDeviceSupportingCluster(EmberAfDeviceDatabaseIterator* iterator,
                                                                   EmberAfClusterId clusterToFind,
                                                                   bool server,
                                                                   uint8_t* returnEndpoint)
{
  if (iterator->deviceIndex >= EMBER_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES) {
    // This was the most appropriate error code I could come up with to say, "Search Complete".
    return EMBER_INDEX_OUT_OF_RANGE;
  }

  doesDeviceHaveCluster(&(deviceDatabase[iterator->deviceIndex]), clusterToFind, server, returnEndpoint);
  iterator->deviceIndex++;
  return EMBER_SUCCESS;
}

void emAfPluginDeviceDatabaseUpdateNodeStackRevision(EmberEUI64 eui64,
                                                     uint8_t stackRevision)
{
  EmberAfDeviceInfo *device = findDeviceByEui64(eui64);
  if (device != NULL) {
    device->stackRevision = stackRevision;
  }
}
