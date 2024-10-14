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

/** @addtogroup aftypes Zigbee Application Framework Types Reference
 * This documentation describes the types used by the Zigbee
 * Application Framework.
 * @{
 */

#include "att-storage.h"
#include <stdbool.h> // For bool
#include <stdint.h>  // For various uint*_t types

#include <app/util/basic-types.h>
#include <app/util/types_stub.h> // For various types.

#include <app/util/attribute-metadata.h> // EmberAfAttributeMetadata

#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Variant.h>
#include <messaging/ExchangeContext.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

/**
 * @brief Type for the cluster mask
 */
typedef uint8_t EmberAfClusterMask;

/**
 * @brief Generic function type, used for either of the cluster function.
 *
 * This type is used for the array of the cluster functions, and should
 * always be cast into one of the specific functions before being called.
 */
typedef void (*EmberAfGenericClusterFunction)(void);

/**
 * @brief A distinguished manufacturer code that is used to indicate the
 * absence of a manufacturer-specific cluster, command, or attribute.
 */
#define MATTER_DM_NULL_MANUFACTURER_CODE 0x0000

/**
 * @brief Struct describing cluster
 */
struct EmberAfCluster
{
    /**
     *  ID of cluster according to ZCL spec
     */
    chip::ClusterId clusterId;
    /**
     * Pointer to attribute metadata array for this cluster.
     */
    const EmberAfAttributeMetadata * attributes;
    /**
     * Total number of attributes
     */
    uint16_t attributeCount;
    /**
     * Total size of non-external, non-singleton attribute for this cluster.
     */
    uint16_t clusterSize;
    /**
     * Mask with additional functionality for cluster. See CLUSTER_MASK
     * macros.
     */
    EmberAfClusterMask mask;

    /**
     * An array into the cluster functions. The length of the array
     * is determined by the function bits in mask. This may be null
     * if this cluster has no functions.
     */
    const EmberAfGenericClusterFunction * functions;

    /**
     * A list of client generated commands. A client generated command
     * is a client to server command. Can be nullptr or terminated by 0xFFFF_FFFF.
     */
    const chip::CommandId * acceptedCommandList;

    /**
     * A list of server generated commands. A server generated command
     * is a response to client command request. Can be nullptr or terminated by 0xFFFF_FFFF.
     */
    const chip::CommandId * generatedCommandList;

    /**
     * Pointer to an array of event IDs of the events supported by the cluster instance.
     * Can be nullptr.
     */
    const chip::EventId * eventList;

    /**
     * Total number of events supported by the cluster instance (in eventList array).
     */
    uint16_t eventCount;

    bool IsServer() const { return (mask & CLUSTER_MASK_SERVER) != 0; }
};

/**
 * @brief Struct that represents a logical device type consisting
 * of a DeviceID and its version.
 */
typedef struct
{
    chip::DeviceTypeId deviceId;
    uint8_t deviceVersion;
} EmberAfDeviceType;

/**
 * @brief Struct used to find an attribute in storage. Together the elements
 * in this search record constitute the "primary key" used to identify a unique
 * attribute value in attribute storage.
 */
typedef struct
{
    /**
     * Endpoint that the attribute is located on
     */
    chip::EndpointId endpoint;

    /**
     * Cluster that the attribute is located on.
     */
    chip::ClusterId clusterId;

    /**
     * The identifier for the attribute.
     */
    chip::AttributeId attributeId;
} EmberAfAttributeSearchRecord;

/**
 * This type is used to compare two ZCL attribute values. The size of this data
 * type depends on the platform.
 */
#ifdef HAL_HAS_INT64
typedef uint64_t EmberAfDifferenceType;
#else
typedef uint32_t EmberAfDifferenceType;
#endif

/**
 * @brief Endpoint type struct describes clusters that are on the endpoint.
 */
typedef struct
{
    /**
     * Pointer to the cluster structs, describing clusters on this
     * endpoint type.
     */
    const EmberAfCluster * cluster;
    /**
     * Number of clusters in this endpoint type.
     */
    uint8_t clusterCount;
    /**
     * Size of all non-external, non-singlet attribute in this endpoint type.
     */
    uint16_t endpointSize;
} EmberAfEndpointType;

enum class EmberAfEndpointOptions : uint8_t
{
    isEnabled         = 0x1,
    isFlatComposition = 0x2,
    isTreeComposition = 0x3,
};

/**
 * @brief Struct that maps actual endpoint type, onto a specific endpoint.
 */
struct EmberAfDefinedEndpoint
{
    /**
     * Actual zigbee endpoint number.
     */
    chip::EndpointId endpoint = chip::kInvalidEndpointId;

    /**
     * Span pointing to a list of supported device types
     */
    chip::Span<const EmberAfDeviceType> deviceTypeList;

    /**
     * Meta-data about the endpoint
     */
    chip::BitMask<EmberAfEndpointOptions> bitmask;
    /**
     * Endpoint type for this endpoint.
     */
    const EmberAfEndpointType * endpointType = nullptr;
    /**
     * Pointer to the DataVersion storage for the server clusters on this
     * endpoint
     */
    chip::DataVersion * dataVersions = nullptr;

    /**
     * Root endpoint id for composed device type.
     */
    chip::EndpointId parentEndpointId = chip::kInvalidEndpointId;

    /**
     * Span pointing to a list of tags. Lifetime has to outlive usage, and data is owned by callers.
     */
    chip::Span<const chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> tagList;
};

// Cluster specific types

/**
 * @brief Indicates the absence of a Scene table entry.
 */
#define MATTER_DM_SCENE_TABLE_NULL_INDEX 0xFF
/**
 * @brief Value used when setting or getting the endpoint in a Scene table
 * entry.  It indicates that the entry is not in use.
 */
#define MATTER_DM_SCENE_TABLE_UNUSED_ENDPOINT_ID 0x00
/**
 * @brief Maximum length of Scene names, not including the length byte.
 */
#define ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH 16
/**
 * @brief The group identifier for the global scene.
 */
#define ZCL_SCENES_GLOBAL_SCENE_GROUP_ID 0x0000
/**
 * @brief The scene identifier for the global scene.
 */
#define ZCL_SCENES_GLOBAL_SCENE_SCENE_ID 0x00

/**
 * @brief Type for referring to the tick callback for cluster.
 *
 * Tick function will be called once for each tick for each endpoint in
 * the cluster. The rate of tick is determined by the metadata of the
 * cluster.
 */
typedef void (*EmberAfTickFunction)(chip::EndpointId endpoint);

/**
 * @brief Type for referring to the init callback for cluster.
 *
 * Init function is called when the application starts up, once for
 * each cluster/endpoint combination.
 */
typedef void (*EmberAfInitFunction)(chip::EndpointId endpoint);

/**
 * @brief Type for referring to the shutdown callback for cluster.
 *
 * Init function is called when the cluster is shut down, for example
 * when an endpoint is disabled
 */
typedef void (*EmberAfShutdownFunction)(chip::EndpointId endpoint);

/**
 * @brief Type for referring to the attribute changed callback function.
 *
 * This function is called just after an attribute changes.
 */
typedef void (*EmberAfClusterAttributeChangedCallback)(const chip::app::ConcreteAttributePath & attributePath);

/**
 * @brief Type for referring to the pre-attribute changed callback function.
 *
 * This function is called before an attribute changes.
 */
typedef chip::Protocols::InteractionModel::Status (*EmberAfClusterPreAttributeChangedCallback)(
    const chip::app::ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value);

#define MAX_INT32U_VALUE (0xFFFFFFFFUL)
#define MAX_INT16U_VALUE (0xFFFF)

/** @} END addtogroup */

namespace chip {
namespace app {

enum class MarkAttributeDirty
{
    kIfChanged,
    kNo,
    // kYes might need to be used if the attribute value was previously changed
    // without reporting, and now is being set in a situation where we know
    // reporting needs to be triggered (e.g. because QuieterReportingAttribute
    // indicated that).
    kYes,
};

/// Notification object of a specific path being changed
class AttributesChangedListener
{
public:
    virtual ~AttributesChangedListener() = default;

    /// Called when the set of attributes identified by AttributePathParams (which may contain wildcards) is to be considered dirty.
    virtual void MarkDirty(const AttributePathParams & path) = 0;
};

} // namespace app
} // namespace chip
