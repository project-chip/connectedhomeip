/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/***************************************************************************/
/**
 * @file
 * @brief Contains the per-endpoint configuration of
 *attribute tables.
 *******************************************************************************
 ******************************************************************************/

#ifndef __AF_ATTRIBUTE_STORAGE__
#define __AF_ATTRIBUTE_STORAGE__

//#include PLATFORM_HEADER
#include "af.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if !defined(EMBER_SCRIPTED_TEST)
#include "gen/att-storage.h"
#endif

#if !defined(ATTRIBUTE_STORAGE_CONFIGURATION) && defined(EMBER_TEST)
#define ATTRIBUTE_STORAGE_CONFIGURATION "attribute-storage-test.h"
#endif

// ATTRIBUTE_STORAGE_CONFIGURATION macro
// contains the file that contains the initial set-up of the
// attribute data structures. If it is missing
// we use the provider sample.
#ifndef ATTRIBUTE_STORAGE_CONFIGURATION
//  #error "Must define ATTRIBUTE_STORAGE_CONFIGURATION to specify the App. Builder default attributes file."
#include "gen/endpoint_config.h"
#else
#include ATTRIBUTE_STORAGE_CONFIGURATION
#endif

// If we have fixed number of endpoints, then max is the same.
#ifdef FIXED_ENDPOINT_COUNT
#define MAX_ENDPOINT_COUNT FIXED_ENDPOINT_COUNT
#endif

#define CLUSTER_TICK_FREQ_ALL (0x00)
#define CLUSTER_TICK_FREQ_QUARTER_SECOND (0x04)
#define CLUSTER_TICK_FREQ_HALF_SECOND (0x08)
#define CLUSTER_TICK_FREQ_SECOND (0x0C)

extern uint8_t attributeData[]; // main storage bucket for all attributes

extern uint8_t attributeDefaults[]; // storage bucked for > 2b default values

void emAfCallInits(void);

#define emberAfClusterIsClient(cluster) ((bool) (((cluster)->mask & CLUSTER_MASK_CLIENT) != 0))
#define emberAfClusterIsServer(cluster) ((bool) (((cluster)->mask & CLUSTER_MASK_SERVER) != 0))
#define emberAfDoesClusterHaveInitFunction(cluster) ((bool) (((cluster)->mask & CLUSTER_MASK_INIT_FUNCTION) != 0))
#define emberAfDoesClusterHaveAttributeChangedFunction(cluster)                                                                    \
    ((bool) (((cluster)->mask & CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION) != 0))
#define emberAfDoesClusterHaveDefaultResponseFunction(cluster)                                                                     \
    ((bool) (((cluster)->mask & CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION) != 0))
#define emberAfDoesClusterHaveMessageSentFunction(cluster) ((bool) (((cluster)->mask & CLUSTER_MASK_MESSAGE_SENT_FUNCTION) != 0))

// Initial configuration
void emberAfEndpointConfigure(void);
bool emberAfExtractCommandIds(bool outgoing, EmberAfClusterCommand * cmd, uint16_t clusterId, uint8_t * buffer,
                              uint16_t bufferLength, uint16_t * bufferIndex, uint8_t startId, uint8_t maxIdCount);

EmberAfStatus emAfReadOrWriteAttribute(EmberAfAttributeSearchRecord * attRecord, EmberAfAttributeMetadata ** metadata,
                                       uint8_t * buffer, uint16_t readLength, bool write);

bool emAfMatchCluster(EmberAfCluster * cluster, EmberAfAttributeSearchRecord * attRecord);
bool emAfMatchAttribute(EmberAfCluster * cluster, EmberAfAttributeMetadata * am, EmberAfAttributeSearchRecord * attRecord);

EmberAfCluster * emberAfFindClusterInTypeWithMfgCode(EmberAfEndpointType * endpointType, EmberAfClusterId clusterId,
                                                     EmberAfClusterMask mask, uint16_t manufacturerCode);

EmberAfCluster * emberAfFindClusterInType(EmberAfEndpointType * endpointType, EmberAfClusterId clusterId, EmberAfClusterMask mask);

// This function returns the index of cluster for the particular endpoint.
// Mask is either CLUSTER_MASK_CLIENT or CLUSTER_MASK_SERVER
// For example, if you have 3 endpoints, 10, 11, 12, and cluster X server is
// located on 11 and 12, and cluster Y server is located only on 10 then
//    clusterIndex(X,11,CLUSTER_MASK_SERVER) returns 0,
//    clusterIndex(X,12,CLUSTER_MASK_SERVER) returns 1,
//    clusterIndex(X,10,CLUSTER_MASK_SERVER) returns 0xFF
//    clusterIndex(Y,10,CLUSTER_MASK_SERVER) returns 0
//    clusterIndex(Y,11,CLUSTER_MASK_SERVER) returns 0xFF
//    clusterIndex(Y,12,CLUSTER_MASK_SERVER) returns 0xFF
uint8_t emberAfClusterIndex(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfClusterMask mask);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t emberAfClusterCount(uint8_t endpoint, bool server);

// Returns the clusterId of Nth server or client cluster,
// depending on server toggle.
EmberAfCluster * emberAfGetNthCluster(uint8_t endpoint, uint8_t n, bool server);

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t emberAfGetClustersFromEndpoint(uint8_t endpoint, EmberAfClusterId * clusterList, uint8_t listLen, bool server);

// Returns cluster within the endpoint, or NULL if it isn't there
EmberAfCluster * emberAfFindClusterWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfClusterMask mask,
                                               uint16_t manufacturerCode);

// Returns cluster within the endpoint, or NULL if it isn't there
// This wraps emberAfFindClusterWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
EmberAfCluster * emberAfFindCluster(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfClusterMask mask);

// Returns cluster within the endpoint; Does not ignore disabled endpoints
EmberAfCluster * emberAfFindClusterIncludingDisabledEndpointsWithMfgCode(uint8_t endpoint, EmberAfClusterId clusterId,
                                                                         EmberAfClusterMask mask, uint16_t manufacturerCode);

// Returns cluster within the endpoint; Does not ignore disabled endpoints
// This wraps emberAfFindClusterIncludingDisabledEndpointsWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
EmberAfCluster * emberAfFindClusterIncludingDisabledEndpoints(uint8_t endpoint, EmberAfClusterId clusterId,
                                                              EmberAfClusterMask mask);

// Function mask must contain one of the CLUSTER_MASK function macros,
// then this method either returns the function pointer or null if
// function doesn't exist. Before you call the function, you must
// cast it.
EmberAfGenericClusterFunction emberAfFindClusterFunction(EmberAfCluster * cluster, EmberAfClusterMask functionMask);

// Public APIs for loading attributes
void emberAfInitializeAttributes(uint8_t endpoint);
void emberAfResetAttributes(uint8_t endpoint);

// Loads the attributes from built-in default and / or tokens
void emAfLoadAttributeDefaults(uint8_t endpoint, bool writeTokens);

// This function loads from tokens all the attributes that
// are defined to be stored in tokens.
void emAfLoadAttributesFromTokens(uint8_t endpoint);

// After the RAM value has changed, code should call this
// function. If this attribute has been
// tagged as stored-to-token, then code will store
// the attribute to token.
void emAfSaveAttributeToToken(uint8_t * data, uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeMetadata * metadata);

// Calls the attribute changed callback
void emAfClusterAttributeChangedCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId,
                                         uint8_t clientServerMask, uint16_t manufacturerCode);

// Calls the attribute changed callback for a specific cluster.
EmberAfStatus emAfClusterPreAttributeChangedCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId,
                                                     uint8_t clientServerMask, uint16_t manufacturerCode,
                                                     EmberAfAttributeType attributeType, uint8_t size, uint8_t * value);

// Calls the default response callback for a specific cluster, and wraps emberAfClusterDefaultResponseWithMfgCodeCallback
// with the EMBER_NULL_MANUFACTURER_CODE
void emberAfClusterDefaultResponseCallback(uint8_t endpoint, EmberAfClusterId clusterId, uint8_t commandId, EmberAfStatus status,
                                           uint8_t clientServerMask);

// Calls the default response callback for a specific cluster.
void emberAfClusterDefaultResponseWithMfgCodeCallback(uint8_t endpoint, EmberAfClusterId clusterId, uint8_t commandId,
                                                      EmberAfStatus status, uint8_t clientServerMask, uint16_t manufacturerCode);

// Calls the message sent callback for a specific cluster, and wraps emberAfClusterMessageSentWithMfgCodeCallback
void emberAfClusterMessageSentCallback(EmberOutgoingMessageType type, uint16_t indexOrDestination, EmberApsFrame * apsFrame,
                                       uint16_t msgLen, uint8_t * message, EmberStatus status);

// Calls the message sent callback for a specific cluster.
void emberAfClusterMessageSentWithMfgCodeCallback(EmberOutgoingMessageType type, uint16_t indexOrDestination,
                                                  EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message, EmberStatus status,
                                                  uint16_t manufacturerCode);

// Used to retrieve a manufacturer code from an attribute metadata
uint16_t emAfGetManufacturerCodeForCluster(EmberAfCluster * cluster);
uint16_t emAfGetManufacturerCodeForAttribute(EmberAfCluster * cluster, EmberAfAttributeMetadata * attMetaData);

// Checks a cluster mask byte against ticks passed bitmask
// returns true if the mask matches a passed interval
bool emberAfCheckTick(EmberAfClusterMask mask, uint8_t passedMask);

bool emberAfEndpointIsEnabled(uint8_t endpoint);

// Note the difference in implementation from emberAfGetNthCluster().
// emberAfGetClusterByIndex() retrieves the cluster by index regardless of server/client
// and those indexes may be DIFFERENT than the indexes returned from
// emberAfGetNthCluster().  In other words:
//
//  - Use emberAfGetClustersFromEndpoint()  with emberAfGetNthCluster()
//  - Use emberAfGetClusterCountForEndpoint() with emberAfGetClusterByIndex()
//
// Don't mix them.
uint8_t emberAfGetClusterCountForEndpoint(uint8_t endpoint);
EmberAfCluster * emberAfGetClusterByIndex(uint8_t endpoint, uint8_t clusterIndex);

EmberAfProfileId emberAfGetProfileIdForEndpoint(uint8_t endpoint);
uint16_t emberAfGetDeviceIdForEndpoint(uint8_t endpoint);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // __AF_ATTRIBUTE_STORAGE__
