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

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/CodeUtils.h>

#include <app/util/att-storage.h>
#include <app/util/attribute-metadata.h>
#include <zap-generated/endpoint_config.h>

#include <protocols/interaction_model/StatusCode.h>

// If we have fixed number of endpoints, then max is the same.
#ifdef FIXED_ENDPOINT_COUNT
#define MAX_ENDPOINT_COUNT (FIXED_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
#endif

#include <app-common/zap-generated/attribute-type.h>

#define DECLARE_DYNAMIC_ENDPOINT(endpointName, clusterList)                                                                        \
    EmberAfEndpointType endpointName = { clusterList, ArraySize(clusterList), 0 }

#define DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(clusterListName) EmberAfCluster clusterListName[] = {

// The role argument should be used to determine whether cluster works as a server or a client.
// It can be assigned with the ZAP_CLUSTER_MASK(SERVER) or ZAP_CLUSTER_MASK(CLUSTER) values.
#define DECLARE_DYNAMIC_CLUSTER(clusterId, clusterAttrs, role, incomingCommands, outgoingCommands)                                 \
    {                                                                                                                              \
        clusterId, clusterAttrs, ArraySize(clusterAttrs), 0, role, NULL, incomingCommands, outgoingCommands                        \
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

extern uint8_t attributeData[]; // main storage bucket for all attributes

void emAfCallInits();

chip::Protocols::InteractionModel::Status emAfReadOrWriteAttribute(const EmberAfAttributeSearchRecord * attRecord,
                                                                   const EmberAfAttributeMetadata ** metadata, uint8_t * buffer,
                                                                   uint16_t readLength, bool write);

//
// Given a cluster ID, endpoint ID and a cluster mask, finds a matching cluster within that endpoint
// with a matching mask. If one is found, the relative index of that cluster within the list of clusters on that
// endpoint is returned. Otherwise, 0xFF is returned.
//
uint8_t emberAfClusterIndex(chip::EndpointId endpoint, chip::ClusterId clusterId, EmberAfClusterMask mask);

// Returns the clusterId of Nth server or client cluster,
// depending on server toggle.
// Returns Optional<ClusterId>::Missing if cluster does not exist.
chip::Optional<chip::ClusterId> emberAfGetNthClusterId(chip::EndpointId endpoint, uint8_t n, bool server);

// Returns cluster within the endpoint; Does not ignore disabled endpoints
const EmberAfCluster * emberAfFindClusterIncludingDisabledEndpoints(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                    EmberAfClusterMask mask);

// Function mask must contain one of the CLUSTER_MASK function macros,
// then this method either returns the function pointer or null if
// function doesn't exist. Before you call the function, you must
// cast it.
EmberAfGenericClusterFunction emberAfFindClusterFunction(const EmberAfCluster * cluster, EmberAfClusterMask functionMask);

// After the RAM value has changed, code should call this function. If this
// attribute has been tagged as non-volatile, its value will be stored.
void emAfSaveAttributeToStorageIfNeeded(uint8_t * data, chip::EndpointId endpoint, chip::ClusterId clusterId,
                                        const EmberAfAttributeMetadata * metadata);

// Calls the attribute changed callback
void emAfClusterAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath);

// Calls the attribute changed callback for a specific cluster.
chip::Protocols::InteractionModel::Status
emAfClusterPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType,
                                       uint16_t size, uint8_t * value);

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
// Returns  CHIP_NO_ERROR                   No error.
//          CHIP_ERROR_NO_MEMORY            MAX_ENDPOINT_COUNT is reached or when no storage is left for clusters
//          CHIP_ERROR_INVALID_ARGUMENT     The EndpointId value passed is kInvalidEndpointId
//          CHIP_ERROR_ENDPOINT_EXISTS      If the EndpointId value passed already exists
//
CHIP_ERROR emberAfSetDynamicEndpoint(uint16_t index, chip::EndpointId id, const EmberAfEndpointType * ep,
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

