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
 * @brief APIs and defines for the Device Database plugin.
 *******************************************************************************
   ******************************************************************************/

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseGetDeviceByIndex(uint16_t index);

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseFindDeviceByStatus(EmberAfDeviceDiscoveryStatus status);

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseFindDeviceByEui64(EmberEUI64 eui64);

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseAdd(EmberEUI64 eui64, uint8_t zigbeeCapabilities);

bool emberAfPluginDeviceDatabaseEraseDevice(EmberEUI64 eui64);

bool emberAfPluginDeviceDatabaseSetEndpoints(const EmberEUI64 eui64,
                                             const uint8_t* endpointList,
                                             uint8_t endpointCount);

uint8_t emberAfPluginDeviceDatabaseGetDeviceEndpointFromIndex(const EmberEUI64 eui64,
                                                              uint8_t index);

// Explicitly made the eui64 the second argument to prevent confusion between
// this function and the emberAfPluginDeviceDatabaseGetDeviceEndpointsFromIndex()
uint8_t emberAfPluginDeviceDatabaseGetIndexFromEndpoint(uint8_t endpoint,
                                                        const EmberEUI64 eui64);

bool emberAfPluginDeviceDatabaseSetClustersForEndpoint(const EmberEUI64 eui64,
                                                       const EmberAfClusterList* clusterList);

bool emberAfPluginDeviceDatabaseClearAllFailedDiscoveryStatus(uint8_t maxFailureCount);

const char* emberAfPluginDeviceDatabaseGetStatusString(EmberAfDeviceDiscoveryStatus status);

bool emberAfPluginDeviceDatabaseSetStatus(const EmberEUI64 deviceEui64, EmberAfDeviceDiscoveryStatus newStatus);

const EmberAfDeviceInfo* emberAfPluginDeviceDatabaseAddDeviceWithAllInfo(const EmberAfDeviceInfo* newDevice);

EmberStatus emberAfPluginDeviceDatabaseDoesDeviceHaveCluster(EmberEUI64 deviceEui64,
                                                             EmberAfClusterId clusterToFind,
                                                             bool server,
                                                             uint8_t* returnEndpoint);

void emberAfPluginDeviceDatabaseCreateNewSearch(EmberAfDeviceDatabaseIterator* iterator);

EmberStatus emberAfPluginDeviceDatabaseFindDeviceSupportingCluster(EmberAfDeviceDatabaseIterator* iterator,
                                                                   EmberAfClusterId clusterToFind,
                                                                   bool server,
                                                                   uint8_t* returnEndpoint);

void emAfPluginDeviceDatabaseUpdateNodeStackRevision(EmberEUI64 eui64,
                                                     uint8_t stackRevision);
