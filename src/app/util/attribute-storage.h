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

#pragma once

//#include PLATFORM_HEADER
#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

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
#define MAX_ENDPOINT_COUNT (FIXED_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
#endif

#include <app-common/zap-generated/attribute-type.h>

#define DECLARE_DYNAMIC_ENDPOINT(endpointName, clusterList)                                                                        \
    EmberAfEndpointType endpointName = { clusterList, ArraySize(clusterList), 0 }

#define DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(clusterListName) EmberAfCluster clusterListName[] = {

#define DECLARE_DYNAMIC_CLUSTER(clusterId, clusterAttrs, incomingCommands, outgoingCommands)                                       \
    {                                                                                                                              \
        clusterId, clusterAttrs, ArraySize(clusterAttrs), 0, ZAP_CLUSTER_MASK(SERVER), NULL, incomingCommands, outgoingCommands    \
    }

#define DECLARE_DYNAMIC_CLUSTER_LIST_END }

#define DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(attrListName) EmberAfAttributeMetadata attrListName[] = {

#define DECLARE_DYNAMIC_ATTRIBUTE_LIST_END()                                                                                       \
    {                                                                                                                              \
        ZAP_EMPTY_DEFAULT(), 0xFFFD, 2, ZAP_TYPE(INT16U), ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)                                     \
    } /* cluster revision */                                                                                                       \
    }

#define DECLARE_DYNAMIC_ATTRIBUTE(attId, attType, attSizeBytes, attrMask)                                                          \
    {                                                                                                                              \
        ZAP_EMPTY_DEFAULT(), attId, attSizeBytes, ZAP_TYPE(attType), attrMask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)               \
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

// Initial configuration
void emberAfEndpointConfigure(void);

EmberAfStatus emAfReadOrWriteAttribute(EmberAfAttributeSearchRecord * attRecord, const EmberAfAttributeMetadata ** metadata,
                                       uint8_t * buffer, uint16_t readLength, bool write);

bool emAfMatchCluster(const EmberAfCluster * cluster, EmberAfAttributeSearchRecord * attRecord);
bool emAfMatchAttribute(const EmberAfCluster * cluster, const EmberAfAttributeMetadata * am,
                        EmberAfAttributeSearchRecord * attRecord);

// Returns endpoint type for the given endpoint id if there is an enabled
// endpoint with that endpoint id.  Otherwise returns null.
const EmberAfEndpointType * emberAfFindEndpointType(chip::EndpointId endpointId);

// Check if a cluster is implemented or not. If yes, the cluster is returned.
//
// mask = 0 -> find either client or server
// mask = CLUSTER_MASK_CLIENT -> find client
// mask = CLUSTER_MASK_SERVER -> find server
//
// If a pointer to an index is provided, it will be updated to point to the relative index of the cluster
// within the set of clusters that match the mask criteria.
//
const EmberAfCluster * emberAfFindClusterInType(const EmberAfEndpointType * endpointType, chip::ClusterId clusterId,
                                                EmberAfClusterMask mask, uint8_t * index = nullptr);

//
// Given a cluster ID, endpoint ID and a cluster mask, finds a matching cluster within that endpoint
// with a matching mask. If one is found, the relative index of that cluster within the list of clusters on that
// endpoint is returned. Otherwise, 0xFF is returned.
//
uint8_t emberAfClusterIndex(chip::EndpointId endpoint, chip::ClusterId clusterId, EmberAfClusterMask mask);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t emberAfClusterCount(chip::EndpointId endpoint, bool server);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on the endpoint at the given index.
uint8_t emberAfClusterCountByIndex(uint16_t endpointIndex, bool server);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on the endpoint at the given index.
uint8_t emberAfClusterCountForEndpointType(const EmberAfEndpointType * endpointType, bool server);

// Returns the cluster of Nth server or client cluster,
// depending on server toggle.
const EmberAfCluster * emberAfGetNthCluster(chip::EndpointId endpoint, uint8_t n, bool server);

// Returns the clusterId of Nth server or client cluster,
// depending on server toggle.
// Returns Optional<ClusterId>::Missing if cluster does not exist.
chip::Optional<chip::ClusterId> emberAfGetNthClusterId(chip::EndpointId endpoint, uint8_t n, bool server);

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t emberAfGetClustersFromEndpoint(chip::EndpointId endpoint, chip::ClusterId * clusterList, uint8_t listLen, bool server);

// Returns server cluster within the endpoint, or NULL if it isn't there
const EmberAfCluster * emberAfFindServerCluster(chip::EndpointId endpoint, chip::ClusterId clusterId);

// Returns cluster within the endpoint; Does not ignore disabled endpoints
const EmberAfCluster * emberAfFindClusterIncludingDisabledEndpoints(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                    EmberAfClusterMask mask);

// Function mask must contain one of the CLUSTER_MASK function macros,
// then this method either returns the function pointer or null if
// function doesn't exist. Before you call the function, you must
// cast it.
EmberAfGenericClusterFunction emberAfFindClusterFunction(const EmberAfCluster * cluster, EmberAfClusterMask functionMask);

// Public APIs for loading attributes
void emberAfInitializeAttributes(chip::EndpointId endpoint);
void emberAfResetAttributes(chip::EndpointId endpoint);

// Loads the attributes from built-in default and / or storage.  If
// ignoreStorage is true, only defaults will be read, and the storage for
// non-volatile attributes will be overwritten with those defaults.
void emAfLoadAttributeDefaults(chip::EndpointId endpoint, bool ignoreStorage, chip::Optional<chip::ClusterId> = chip::NullOptional);

// After the RAM value has changed, code should call this function. If this
// attribute has been tagged as non-volatile, its value will be stored.
void emAfSaveAttributeToStorageIfNeeded(uint8_t * data, chip::EndpointId endpoint, chip::ClusterId clusterId,
                                        const EmberAfAttributeMetadata * metadata);

// Calls the attribute changed callback
void emAfClusterAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath);

// Calls the attribute changed callback for a specific cluster.
EmberAfStatus emAfClusterPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                     EmberAfAttributeType attributeType, uint16_t size, uint8_t * value);

// Checks a cluster mask byte against ticks passed bitmask
// returns true if the mask matches a passed interval
bool emberAfCheckTick(EmberAfClusterMask mask, uint8_t passedMask);

// Check whether there is an endpoint defined with the given endpoint id that is
// enabled.
bool emberAfEndpointIsEnabled(chip::EndpointId endpoint);

// Note the difference in implementation from emberAfGetNthCluster().
// emberAfGetClusterByIndex() retrieves the cluster by index regardless of server/client
// and those indexes may be DIFFERENT than the indexes returned from
// emberAfGetNthCluster().  In other words:
//
//  - Use emberAfGetClustersFromEndpoint()  with emberAfGetNthCluster() emberAfGetNthClusterId()
//  - Use emberAfGetClusterCountForEndpoint() with emberAfGetClusterByIndex()
//
// Don't mix them.
uint8_t emberAfGetClusterCountForEndpoint(chip::EndpointId endpoint);
const EmberAfCluster * emberAfGetClusterByIndex(chip::EndpointId endpoint, uint8_t clusterIndex);

//
// Retrieve the device type list associated with a specific endpoint.
//
chip::Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpoint(chip::EndpointId endpoint, CHIP_ERROR & err);

//
// Over-ride the device type list current associated with an endpoint with a user-provided list. The buffers backing
// that list have to live as long as the endpoint is enabled.
//
// NOTE: It is the application's responsibility to free the existing list that is being replaced if needed.
//
CHIP_ERROR emberAfSetDeviceTypeList(chip::EndpointId endpoint, chip::Span<const EmberAfDeviceType> deviceTypeList);

// Register a dynamic endpoint. This involves registering descriptors that describe
// the composition of the endpoint (encapsulated in the 'ep' argument) as well as providing
// storage for data versions.
//
// dataVersionStorage.size() needs to be at least as large as the number of
// server clusters on this endpoint.  If it's not, the endpoint will not be able
// to store data versions, which may break consumers.
//
// The memory backing dataVersionStorage needs to remain allocated until this dynamic
// endpoint is cleared.
//
// An optional device type list can be passed in as well. If provided, the memory
// backing the list needs to remain allocated until this dynamic endpoint is cleared.
//
// An optional parent endpoint id should be passed for child endpoints of composed device.
//
EmberAfStatus emberAfSetDynamicEndpoint(uint16_t index, chip::EndpointId id, const EmberAfEndpointType * ep,
                                        const chip::Span<chip::DataVersion> & dataVersionStorage,
                                        chip::Span<const EmberAfDeviceType> deviceTypeList = {},
                                        chip::EndpointId parentEndpointId                  = chip::kInvalidEndpointId);
chip::EndpointId emberAfClearDynamicEndpoint(uint16_t index);
uint16_t emberAfGetDynamicIndexFromEndpoint(chip::EndpointId id);

// Get the number of attributes of the specific cluster under the endpoint.
// Returns 0 if the cluster does not exist.
uint16_t emberAfGetServerAttributeCount(chip::EndpointId endpoint, chip::ClusterId cluster);

// Get the index of the given attribute of the specific cluster under the endpoint.
// Returns UINT16_MAX if the attribute does not exist.
uint16_t emberAfGetServerAttributeIndexByAttributeId(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                     chip::AttributeId attributeId);

// Get the attribute id at the attributeIndex of the cluster under the endpoint. This function is useful for iterating over the
// attributes.
// Returns Optional<chip::AttributeId>::Missing() if the attribute does not exist.
chip::Optional<chip::AttributeId> emberAfGetServerAttributeIdByIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                     uint16_t attributeIndex);

/**
 * Register an attribute access override.  It will remain registered until
 * the endpoint it's registered for is disabled (or until shutdown if it's
 * registered for all endpoints).  Registration will fail if there is an
 * already-registered override for the same set of attributes.
 *
 * @return false if there is an existing override that the new one would
 *               conflict with.  In this case the override is not registered.
 * @return true if registration was successful.
 */
bool registerAttributeAccessOverride(chip::app::AttributeAccessInterface * attrOverride);
