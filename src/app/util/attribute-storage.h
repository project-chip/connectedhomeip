/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/util/ember-api.h> // used to be defined in this file, include for compatibility

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/config.h>

// NOTE: direct include here even though app/util/config.h is the public header,
//       because MAX_ENDPOINT_COUNT needs FIXED_ENDPOINT_COUNT definitions.
#include <zap-generated/endpoint_config.h>

// If we have fixed number of endpoints, then max is the same.
#ifdef FIXED_ENDPOINT_COUNT
#define MAX_ENDPOINT_COUNT (FIXED_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
#endif

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

// The attrMask must contain the relevant ATTRIBUTE_MASK_* bits from
// attribute-metadata.h.  Specifically:
//
// * Writable attributes must have ATTRIBUTE_MASK_WRITABLE
// * Nullable attributes (have X in the quality column in the spec) must have ATTRIBUTE_MASK_NULLABLE
// * Attributes that have T in the Access column in the spec must have ATTRIBUTE_MASK_MUST_USE_TIMED_WRITE
#define DECLARE_DYNAMIC_ATTRIBUTE(attId, attType, attSizeBytes, attrMask)                                                          \
    {                                                                                                                              \
        ZAP_EMPTY_DEFAULT(), attId, attSizeBytes, ZAP_TYPE(attType), attrMask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)               \
    }
