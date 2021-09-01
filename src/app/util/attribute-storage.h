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

#pragma once

//#include PLATFORM_HEADER
#include <app/util/af.h>

#if !defined(EMBER_SCRIPTED_TEST)
#include <app-common/zap-generated/att-storage.h>
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
#include <zap-generated/endpoint_config.h>
#else
#include ATTRIBUTE_STORAGE_CONFIGURATION
#endif

// If we have fixed number of endpoints, then max is the same.
#ifdef FIXED_ENDPOINT_COUNT
#ifdef DYNAMIC_ENDPOINT_COUNT
#define MAX_ENDPOINT_COUNT (FIXED_ENDPOINT_COUNT + DYNAMIC_ENDPOINT_COUNT)
#else
#define MAX_ENDPOINT_COUNT FIXED_ENDPOINT_COUNT
#endif
#endif

#include <app-common/zap-generated/attribute-type.h>

#define DECLARE_DYNAMIC_ENDPOINT(endpointName, clusterList)                                                                        \
    EmberAfEndpointType endpointName = { clusterList, sizeof(clusterList) / sizeof(EmberAfCluster), 0 }

#define DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(clusterListName) EmberAfCluster clusterListName[] = {

#define DECLARE_DYNAMIC_CLUSTER(clusterId, clusterAttrs)                                                                           \
    {                                                                                                                              \
        clusterId, clusterAttrs, sizeof(clusterAttrs) / sizeof(EmberAfAttributeMetadata), 0, ZAP_CLUSTER_MASK(SERVER), NULL        \
    }

#define DECLARE_DYNAMIC_CLUSTER_LIST_END }

#define DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(attrListName) EmberAfAttributeMetadata attrListName[] = {

#define DECLARE_DYNAMIC_ATTRIBUTE_LIST_END(clusterRevision)                                                                        \
    , { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(clusterRevision) } /* cluster revision */                               \
    }

#define DECLARE_DYNAMIC_ATTRIBUTE(attId, attType, attSizeBytes, attrMask)                                                          \
    {                                                                                                                              \
        attId, ZAP_TYPE(attType), attSizeBytes, attrMask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT()               \
    }

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
bool emberAfExtractCommandIds(bool outgoing, EmberAfClusterCommand * cmd, chip::ClusterId clusterId, uint8_t * buffer,
                              uint16_t bufferLength, uint16_t * bufferIndex, uint8_t startId, uint8_t maxIdCount);

EmberAfStatus emAfReadOrWriteAttribute(EmberAfAttributeSearchRecord * attRecord, EmberAfAttributeMetadata ** metadata,
                                       uint8_t * buffer, uint16_t readLength, bool write, int32_t index = -1);

bool emAfMatchCluster(EmberAfCluster * cluster, EmberAfAttributeSearchRecord * attRecord);
bool emAfMatchAttribute(EmberAfCluster * cluster, EmberAfAttributeMetadata * am, EmberAfAttributeSearchRecord * attRecord);

EmberAfCluster * emberAfFindClusterInTypeWithMfgCode(EmberAfEndpointType * endpointType, chip::ClusterId clusterId,
                                                     EmberAfClusterMask mask, uint16_t manufacturerCode);

EmberAfCluster * emberAfFindClusterInType(EmberAfEndpointType * endpointType, chip::ClusterId clusterId, EmberAfClusterMask mask);

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
uint8_t emberAfClusterIndex(chip::EndpointId endpoint, chip::ClusterId clusterId, EmberAfClusterMask mask);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t emberAfClusterCount(chip::EndpointId endpoint, bool server);

// Returns the clusterId of Nth server or client cluster,
// depending on server toggle.
EmberAfCluster * emberAfGetNthCluster(chip::EndpointId endpoint, uint8_t n, bool server);

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t emberAfGetClustersFromEndpoint(chip::EndpointId endpoint, chip::ClusterId * clusterList, uint8_t listLen, bool server);

// Returns cluster within the endpoint, or NULL if it isn't there
EmberAfCluster * emberAfFindClusterWithMfgCode(chip::EndpointId endpoint, chip::ClusterId clusterId, EmberAfClusterMask mask,
                                               uint16_t manufacturerCode);

// Returns cluster within the endpoint, or NULL if it isn't there
// This wraps emberAfFindClusterWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
EmberAfCluster * emberAfFindCluster(chip::EndpointId endpoint, chip::ClusterId clusterId, EmberAfClusterMask mask);

// Returns cluster within the endpoint; Does not ignore disabled endpoints
EmberAfCluster * emberAfFindClusterIncludingDisabledEndpointsWithMfgCode(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                         EmberAfClusterMask mask, uint16_t manufacturerCode);

// Returns cluster within the endpoint; Does not ignore disabled endpoints
// This wraps emberAfFindClusterIncludingDisabledEndpointsWithMfgCode with EMBER_AF_NULL_MANUFACTURER_CODE
EmberAfCluster * emberAfFindClusterIncludingDisabledEndpoints(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                              EmberAfClusterMask mask);

// Function mask must contain one of the CLUSTER_MASK function macros,
// then this method either returns the function pointer or null if
// function doesn't exist. Before you call the function, you must
// cast it.
EmberAfGenericClusterFunction emberAfFindClusterFunction(EmberAfCluster * cluster, EmberAfClusterMask functionMask);

// Public APIs for loading attributes
void emberAfInitializeAttributes(chip::EndpointId endpoint);
void emberAfResetAttributes(chip::EndpointId endpoint);

// Loads the attributes from built-in default and / or tokens
void emAfLoadAttributeDefaults(chip::EndpointId endpoint, bool writeTokens);

// This function loads from tokens all the attributes that
// are defined to be stored in tokens.
void emAfLoadAttributesFromTokens(chip::EndpointId endpoint);

// After the RAM value has changed, code should call this
// function. If this attribute has been
// tagged as stored-to-token, then code will store
// the attribute to token.
void emAfSaveAttributeToToken(uint8_t * data, chip::EndpointId endpoint, chip::ClusterId clusterId,
                              EmberAfAttributeMetadata * metadata);

// Calls the attribute changed callback
void emAfClusterAttributeChangedCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                         uint8_t clientServerMask, uint16_t manufacturerCode);

// Calls the attribute changed callback for a specific cluster.
EmberAfStatus emAfClusterPreAttributeChangedCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                     chip::AttributeId attributeId, uint8_t clientServerMask,
                                                     uint16_t manufacturerCode, EmberAfAttributeType attributeType, uint16_t size,
                                                     uint8_t * value);

// Calls the default response callback for a specific cluster, and wraps emberAfClusterDefaultResponseWithMfgCodeCallback
// with the EMBER_NULL_MANUFACTURER_CODE
void emberAfClusterDefaultResponseCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::CommandId commandId,
                                           EmberAfStatus status, uint8_t clientServerMask);

// Calls the default response callback for a specific cluster.
void emberAfClusterDefaultResponseWithMfgCodeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                      chip::CommandId commandId, EmberAfStatus status, uint8_t clientServerMask,
                                                      uint16_t manufacturerCode);

// Calls the message sent callback for a specific cluster, and wraps emberAfClusterMessageSentWithMfgCodeCallback
void emberAfClusterMessageSentCallback(const chip::MessageSendDestination & destination, EmberApsFrame * apsFrame, uint16_t msgLen,
                                       uint8_t * message, EmberStatus status);

// Calls the message sent callback for a specific cluster.
void emberAfClusterMessageSentWithMfgCodeCallback(const chip::MessageSendDestination & destination, EmberApsFrame * apsFrame,
                                                  uint16_t msgLen, uint8_t * message, EmberStatus status,
                                                  uint16_t manufacturerCode);

// Used to retrieve a manufacturer code from an attribute metadata
uint16_t emAfGetManufacturerCodeForCluster(EmberAfCluster * cluster);
uint16_t emAfGetManufacturerCodeForAttribute(EmberAfCluster * cluster, EmberAfAttributeMetadata * attMetaData);

// Checks a cluster mask byte against ticks passed bitmask
// returns true if the mask matches a passed interval
bool emberAfCheckTick(EmberAfClusterMask mask, uint8_t passedMask);

bool emberAfEndpointIsEnabled(chip::EndpointId endpoint);

// Note the difference in implementation from emberAfGetNthCluster().
// emberAfGetClusterByIndex() retrieves the cluster by index regardless of server/client
// and those indexes may be DIFFERENT than the indexes returned from
// emberAfGetNthCluster().  In other words:
//
//  - Use emberAfGetClustersFromEndpoint()  with emberAfGetNthCluster()
//  - Use emberAfGetClusterCountForEndpoint() with emberAfGetClusterByIndex()
//
// Don't mix them.
uint8_t emberAfGetClusterCountForEndpoint(chip::EndpointId endpoint);
EmberAfCluster * emberAfGetClusterByIndex(chip::EndpointId endpoint, uint8_t clusterIndex);

uint16_t emberAfGetDeviceIdForEndpoint(chip::EndpointId endpoint);
EmberAfStatus emberAfSetDynamicEndpoint(uint16_t index, chip::EndpointId id, EmberAfEndpointType * ep, uint16_t deviceId,
                                        uint8_t deviceVersion);
chip::EndpointId emberAfClearDynamicEndpoint(uint16_t index);
uint16_t emberAfGetDynamicIndexFromEndpoint(chip::EndpointId id);
