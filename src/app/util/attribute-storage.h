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

#include <app/util/af-types.h>
#include <app/util/att-storage.h>
#include <app/util/attribute-metadata.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-defines.h>
#include <lib/support/CodeUtils.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-objects.h>

// NOTE: direct include here even though app/util/config.h is the public header,
//       because MAX_ENDPOINT_COUNT needs FIXED_ENDPOINT_COUNT definitions.
#include <zap-generated/endpoint_config.h>

static constexpr uint16_t kEmberInvalidEndpointIndex = 0xFFFF;

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

/**
 * @brief locate attribute metadata
 *
 * Function returns pointer to the attribute metadata structure,
 * or NULL if attribute was not found.
 *
 * @param endpoint Zigbee endpoint number.
 * @param clusterId Cluster ID of the sought cluster.
 * @param attributeId Attribute ID of the sought attribute.
 *
 * @return Returns pointer to the attribute metadata location.
 */
const EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                chip::AttributeId attributeId);

/**
 * @brief Returns true if endpoint contains the ZCL server with specified id.
 *
 * This function returns true if
 * the endpoint contains server of a given cluster.
 */
bool emberAfContainsServer(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief Returns true if endpoint of given index contains the ZCL server with specified id.
 *
 * This function returns true if
 * the endpoint of given index contains server of a given cluster.
 * If this function is used with a manufacturer specific clusterId
 * then this will return the first cluster that it finds in the Cluster table.
 * and will not return any other clusters that share that id.
 */
bool emberAfContainsServerFromIndex(uint16_t index, chip::ClusterId clusterId);

/**
 * @brief Returns true if endpoint contains the ZCL client with specified id.
 *
 * This function returns true if
 * the endpoint contains client of a given cluster.
 */
bool emberAfContainsClient(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * @brief macro that returns size of attribute in bytes.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define emberAfAttributeSize(metadata) ((metadata)->size)

/**
 * Returns the index of a given endpoint.  Will return 0xFFFF if this is not a
 * valid endpoint id or if the endpoint is disabled.
 */
uint16_t emberAfIndexFromEndpoint(chip::EndpointId endpoint);

/**
 * @brief Returns parent endpoint for a given endpoint index
 */
chip::EndpointId emberAfParentEndpointFromIndex(uint16_t index);

/**
 *  @brief Returns the index of the given endpoint in the list of all endpoints that might support the given cluster server.
 *
 * Returns kEmberInvalidEndpointIndex if the given endpoint does not support the
 * given cluster or if the given endpoint is disabled.
 *
 * This function always returns the same index for a given endpointId instance, fixed or dynamic.
 *
 * The return index for fixed endpoints will range from 0 to (fixedClusterServerEndpointCount - 1),
 * For dynamic endpoints the indexing assumes that any dynamic endpoint could start supporting
 * the given server cluster and their index will range from fixedClusterServerEndpointCount to
 * (fixedClusterServerEndpointCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT - 1).
 *
 * For example, if a device has 4 fixed endpoints (ids 0-3) and 2 dynamic
 * endpoints, and cluster X is supported on endpoints 1 and 3, then
 * fixedClusterServerEndpointCount should be 2 and
 *
 * 1) emberAfGetClusterServerEndpointIndex(0, X) returns kEmberInvalidEndpointIndex
 * 2) emberAfGetClusterServerEndpointIndex(1, X) returns 0
 * 3) emberAfGetClusterServerEndpointIndex(2, X) returns kEmberInvalidEndpointIndex
 * 4) emberAfGetClusterServerEndpointIndex(3, X) returns 1

 * The Dynamic endpoints are placed after the fixed ones;
 * therefore their return index will always be >= to fixedClusterServerEndpointCount
 *
 * If a dynamic endpoint, supporting cluster X, is defined to dynamic index 1 with endpoint id 7,
 * (via emberAfSetDynamicEndpoint(1, 7, ...))
 * then emberAfGetClusterServerEndpointIndex(7, X) returns 3 (fixedClusterServerEndpointCount{2} + DynamicEndpointIndex {1}).
 *
 * If now a second dynamic endpoint, also supporting cluster X, is defined to dynamic index 0
 * with endpoint id 9  (via emberAfSetDynamicEndpoint(0, 9, ...)),
 * emberAfGetClusterServerEndpointIndex(9, X) returns 2. (fixedClusterServerEndpointCount{2} + DynamicEndpointIndex {0}).
 * and emberAfGetClusterServerEndpointIndex(7, X) still returns 3
 *
 * @param endpoint Endpoint number
 * @param cluster Id the of the Cluster server you are interrested on
 * @param fixedClusterServerEndpointCount The number of fixed endpoints containing this cluster server.  Typically one of the
 MATTER_DM_*_CLUSTER_SERVER_ENDPOINT_COUNT constants.
 */
uint16_t emberAfGetClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                              uint16_t fixedClusterServerEndpointCount);

/**
 * Returns the pointer to the data version storage for the given endpoint and
 * cluster.  Can return null in the following cases:
 *
 * 1) There is no such endpoint.
 * 2) There is no such server cluster on the given endpoint.
 * 3) No storage for a data version was provided for the endpoint.
 */
chip::DataVersion * emberAfDataVersionStorage(const chip::app::ConcreteClusterPath & aConcreteClusterPath);

/**
 * @brief Returns the number of pre-compiled endpoints.
 */
uint16_t emberAfFixedEndpointCount();

/**
 * Get the semantic tags of the endpoint.
 * Fills in the provided SemanticTagStruct with tag at index `index` if there is one,
 * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of tag,
 * or returns CHIP_ERROR_NOT_FOUND if the endpoint is invalid.
 * @param endpoint The target endpoint.
 * @param index The index of the tag, with 0 representing the first tag.
 * @param tag  The SemanticTagStruct is filled.
 */
CHIP_ERROR GetSemanticTagForEndpointAtIndex(chip::EndpointId endpoint, size_t index,
                                            chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type & tag);

//
// Override the tag list current associated with an endpoint with a user-provided list. The buffers backing
// that list have to live as long as the endpoint is enabled.
//
// NOTE: It is the application's responsibility to free the existing list that is being replaced if needed.
//
CHIP_ERROR SetTagList(chip::EndpointId endpoint,
                      chip::Span<const chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> tagList);

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t emberAfGetClustersFromEndpoint(chip::EndpointId endpoint, chip::ClusterId * clusterList, uint8_t listLen, bool server);

// Note the difference in for server filtering.
// This method will return the cluster count for BOTH client and server
// and those do NOT work with NthCluster/NthClusterId
//  - Use emberAfGetClustersFromEndpoint()  with emberAfGetNthCluster() emberAfGetNthClusterId()
//
uint8_t emberAfGetClusterCountForEndpoint(chip::EndpointId endpoint);

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

// Initial configuration
void emberAfEndpointConfigure();

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
/**
 * @brief Loads attribute defaults and any non-volatile attributes stored
 *
 * @param endpoint EnpointId. Use chip::kInvalidEndpointId to initialize all endpoints
 */
void emberAfInitializeAttributes(chip::EndpointId endpoint);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t emberAfClusterCount(chip::EndpointId endpoint, bool server);

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on the endpoint at the given index.
uint8_t emberAfClusterCountForEndpointType(const EmberAfEndpointType * endpointType, bool server);

// Returns the cluster of Nth server or client cluster,
// depending on server toggle.
const EmberAfCluster * emberAfGetNthCluster(chip::EndpointId endpoint, uint8_t n, bool server);

//
// Retrieve the device type list associated with a specific endpoint.
//
chip::Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpoint(chip::EndpointId endpoint, CHIP_ERROR & err);
chip::Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpointIndex(unsigned endpointIndex, CHIP_ERROR & err);

//
// Override the device type list current associated with an endpoint with a user-provided list. The buffers backing
// that list have to live as long as the endpoint is enabled.
//
// NOTE: It is the application's responsibility to free the existing list that is being replaced if needed.
//
CHIP_ERROR emberAfSetDeviceTypeList(chip::EndpointId endpoint, chip::Span<const EmberAfDeviceType> deviceTypeList);

/// Returns a change listener that uses the global InteractionModelEngine
/// instance to report dirty paths
chip::app::AttributesChangedListener * emberAfGlobalInteractionModelAttributesChangedListener();

/// Mark the given attribute as having changed:
///   - increases the cluster data version for the given cluster
///   - uses `listener` to `MarkDirty` the given path. This is typically done to mark an
///     attribute as dirty within the matter attribute reporting engine, so that subscriptions
///     receive updated attribute values for a cluster.
///
/// This is a convenience function to make it clear when a `emberAfDataVersionStorage` increase
/// and a `AttributesChangeListener::MarkDirty` always occur in lock-step.
void emberAfAttributeChanged(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                             chip::app::AttributesChangedListener * listener);

/// Mark attributes on the given endpoint as having changed.
///
/// Schedules reporting engine to consider the endpoint dirty, however does NOT increase/alter
/// any cluster data versions.
void emberAfEndpointChanged(chip::EndpointId endpoint, chip::app::AttributesChangedListener * listener);

/// Maintains a increasing index of structural changes within ember
/// that determine if existing "indexes" and metadata pointers within ember
/// are still valid or not.
///
/// Changes to metadata structure (e.g. endpoint enable/disable and dynamic endpoint changes)
/// are reflected in this generation count changing.
unsigned emberAfMetadataStructureGeneration();

namespace chip {
namespace app {

class EnabledEndpointsWithServerCluster
{
public:
    EnabledEndpointsWithServerCluster(ClusterId clusterId);

    // Instead of having a separate Iterator class, optimize for codesize by
    // just reusing ourselves as our own iterator.  We could do a bit better
    // here with C++17 and using a different type for the end iterator, but this
    // is the best I've found with C++14 so far.
    //
    // This does mean that you can only iterate a given
    // EnabledEndpointsWithServerCluster once, but that's OK given how we use it
    // in practice.
    EnabledEndpointsWithServerCluster & begin() { return *this; }
    const EnabledEndpointsWithServerCluster & end() const { return *this; }

    bool operator!=(const EnabledEndpointsWithServerCluster & other) const { return mEndpointIndex != mEndpointCount; }

    EnabledEndpointsWithServerCluster & operator++();

    EndpointId operator*() const;

private:
    void EnsureMatchingEndpoint();

    uint16_t mEndpointIndex = 0;
    uint16_t mEndpointCount = 0;
    ClusterId mClusterId;
};

/**
 * @brief Sets the parent endpoint for a given endpoint
 */
CHIP_ERROR SetParentEndpointForEndpoint(EndpointId childEndpoint, EndpointId parentEndpoint);

/**
 * @brief Sets an Endpoint to use Flat Composition
 */
CHIP_ERROR SetFlatCompositionForEndpoint(EndpointId endpoint);

/**
 * @brief Sets an Endpoint to use Tree Composition
 */
CHIP_ERROR SetTreeCompositionForEndpoint(EndpointId endpoint);

/**
 * @brief Returns true is an Endpoint has flat composition
 */
bool IsFlatCompositionForEndpoint(EndpointId endpoint);

/**
 * @brief Returns true is an Endpoint has tree composition
 */
bool IsTreeCompositionForEndpoint(EndpointId endpoint);

enum class EndpointComposition : uint8_t
{
    kFullFamily,
    kTree,
    kInvalid,
};

/**
 * @brief Returns the composition for a given endpoint index
 */
EndpointComposition GetCompositionForEndpointIndex(uint16_t index);

} // namespace app
} // namespace chip
