/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Silicon Laboratories Inc. www.silabs.com
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
 *    @file
 *      This file provides the API to the attribute database implementation
 *      used by the CHIP ZCL Application Layer
 *
 */

#ifndef CHIP_ZCL_ATTRIBUTE_DB
#define CHIP_ZCL_ATTRIBUTE_DB

#include "gen-attribute-storage.h"

/**
 * @brief Type for referring to zigbee application profile id
 */
typedef uint16_t ChipZclProfileId;

/**
 * @brief Type for referring to ZCL attribute id
 */
typedef uint16_t ChipZclAttributeId;

/**
 * @brief Type for referring to ZCL cluster id
 */
typedef uint16_t ChipZclClusterId;

/**
 * @brief Type for referring to ZCL attribute type
 */
typedef uint8_t ChipZclAttributeType;

/**
 * @brief Type for the cluster mask
 */
typedef uint8_t ChipZclClusterMask;

/**
 * @brief Type for the attribute mask
 */
typedef uint8_t ChipZclAttributeMask;

/**
 * @brief Generic function type, used for either of the cluster function.
 *
 * This type is used for the array of the cluster functions, and should
 * always be cast into one of the specific functions before being called.
 */
typedef void (*ChipZclGenericClusterFunction)(void);

/**
 * @brief A distinguished manufacturer code that is used to indicate the
 * absence of a manufacturer-specific profile, cluster, command, or attribute.
 */
#define CHIP_ZCL_NULL_MANUFACTURER_CODE 0x0000

/**
 * @brief An invalid profile ID
 * This is a reserved profileId.
 */
#define CHIP_ZCL_INVALID_PROFILE_ID 0xFFFF

/**
 * @brief Type for default values.
 *
 * Default value is either a value itself, if it is 2 bytes or less,
 * or a pointer to the value itself, if attribute type is longer than
 * 2 bytes.
 */
typedef union
{
    /**
     * Points to data if size is more than 2 bytes.
     * If size is more than 2 bytes, and this value is NULL,
     * then the default value is all zeroes.
     */
    uint8_t * ptrToDefaultValue;
    /**
     * Actual default value if the attribute size is 2 bytes or less.
     */
    uint16_t defaultValue;
} ChipZclDefaultAttributeValue;

/**
 * @brief Type describing the attribute default, min and max values.
 *
 * This struct is required if the attribute mask specifies that this
 * attribute has a known min and max values.
 */
typedef struct
{
    /**
     * Default value of the attribute.
     */
    ChipZclDefaultAttributeValue defaultValue;
    /**
     * Minimum allowed value
     */
    ChipZclDefaultAttributeValue minValue;
    /**
     * Maximum allowed value.
     */
    ChipZclDefaultAttributeValue maxValue;
} ChipZclAttributeMinMaxValue;

/**
 * @brief Union describing the attribute default/min/max values.
 */
typedef union
{
    /**
     * Points to data if size is more than 2 bytes.
     * If size is more than 2 bytes, and this value is NULL,
     * then the default value is all zeroes.
     */
    uint8_t * ptrToDefaultValue;
    /**
     * Actual default value if the attribute size is 2 bytes or less.
     */
    uint16_t defaultValue;
    /**
     * Points to the min max attribute value structure, if min/max is
     * supported for this attribute.
     */
    ChipZclAttributeMinMaxValue * ptrToMinMaxValue;
} ChipZclDefaultOrMinMaxAttributeValue;

/**
 * @brief Each attribute has it's metadata stored in such struct.
 *
 * There is only one of these per attribute across all endpoints.
 */
typedef struct
{
    /**
     * Attribute ID, according to ZCL specs.
     */
    ChipZclAttributeId attributeId;
    /**
     * Attribute type, according to ZCL specs.
     */
    ChipZclAttributeType attributeType;
    /**
     * Size of this attribute in bytes.
     */
    uint8_t size;
    /**
     * Attribute mask, tagging attribute with specific
     * functionality. See ATTRIBUTE_MASK_ macros defined
     * in att-storage.h.
     */
    ChipZclAttributeMask mask;
    /**
     * Pointer to the default value union. Actual value stored
     * depends on the mask.
     */
    ChipZclDefaultOrMinMaxAttributeValue defaultValue;
} ChipZclAttributeMetadata;

/**
 * @brief Struct describing cluster
 */
typedef struct
{
    /**
     *  ID of cluster according to ZCL spec
     */
    ChipZclClusterId clusterId;
    /**
     * Pointer to attribute metadata array for this cluster.
     */
    ChipZclAttributeMetadata * attributes;
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
    ChipZclClusterMask mask;

    /**
     * An array into the cluster functions. The length of the array
     * is determined by the function bits in mask. This may be null
     * if this cluster has no functions.
     */
    const ChipZclGenericClusterFunction * functions;
} ChipZclCluster;

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
    uint8_t endpoint;

    /**
     * Cluster that the attribute is located on. If the cluster
     * id is inside the manufacturer specific range, 0xfc00 - 0xffff,
     * The manufacturer code should also be set to the code associated
     * with the manufacturer specific cluster.
     */
    ChipZclClusterId clusterId;

    /**
     * Cluster mask for the cluster, used to determine if it is
     * the server or client version of the cluster. See CLUSTER_MASK_
     * macros defined in att-storage.h
     */
    ChipZclClusterMask clusterMask;

    /**
     * The two byte identifier for the attribute. If the cluster id is
     * inside the manufacturer specific range 0xfc00 - 0xffff, or the manufacturer
     * code is NOT 0, the attribute is assumed to be manufacturer specific.
     */
    ChipZclAttributeId attributeId;

    /**
     * Manufacturer Code associated with the cluster and or attribute.
     * If the cluster id is inside the manufacturer specific
     * range, this value should indicate the manufacturer code for the
     * manufacturer specific cluster. Otherwise if this value is non zero,
     * and the cluster id is a standard ZCL cluster,
     * it is assumed that the attribute being sought is a manufacturer specific
     * extension to the standard ZCL cluster indicated by the cluster id.
     */
    uint16_t manufacturerCode;
} ChipZclAttributeSearchRecord;

/**
 * A struct used to construct a table of manufacturer codes for
 * manufacturer specific attributes and clusters.
 */
typedef struct
{
    uint16_t index;
    uint16_t manufacturerCode;
} ChipZclManufacturerCodeEntry;

/**
 * This type is used to compare two ZCL attribute values. The size of this data
 * type depends on the platform.
 */
typedef uint32_t ChipZclDifferenceType;

/**
 * @brief Endpoint type struct describes clusters that are on the endpoint.
 */
typedef struct
{
    /**
     * Pointer to the cluster structs, describing clusters on this
     * endpoint type.
     */
    ChipZclCluster * cluster;
    /**
     * Number of clusters in this endpoint type.
     */
    uint8_t clusterCount;
    /**
     * Size of all non-external, non-singlet attribute in this endpoint type.
     */
    uint16_t endpointSize;
} ChipZclEndpointType;

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclEndpointBitmask;
#else
typedef uint8_t ChipZclEndpointBitmask;
enum
#endif
{ CHIP_ZCL_ENDPOINT_DISABLED = 0x00,
  CHIP_ZCL_ENDPOINT_ENABLED  = 0x01,
};

/**
 * @brief Struct that maps actual endpoint type, onto a specific endpoint.
 */
typedef struct
{
    /**
     * Actual zigbee endpoint number.
     */
    uint8_t endpoint;
    /**
     * Profile ID of the device on this endpoint.
     */
    ChipZclProfileId profileId;
    /**
     * Device ID of the device on this endpoint.
     */
    uint16_t deviceId;
    /**
     * Version of the device.
     */
    uint8_t deviceVersion;
    /**
     * Endpoint type for this endpoint.
     */
    ChipZclEndpointType * endpointType;
    /**
     * Network index for this endpoint.
     */
    uint8_t networkIndex;
    /**
     * Meta-data about the endpoint
     */
    ChipZclEndpointBitmask bitmask;
} ChipZclDefinedEndpoint;

/**
 * @brief Type for referring to the tick callback for cluster.
 *
 * Tick function will be called once for each tick for each endpoint in
 * the cluster. The rate of tick is determined by the metadata of the
 * cluster.
 */
typedef void (*ChipZclTickFunction)(uint8_t endpoint);

/**
 * @brief Type for referring to the init callback for cluster.
 *
 * Init function is called when the application starts up, once for
 * each cluster/endpoint combination.
 */
typedef void (*ChipZclInitFunction)(uint8_t endpoint);

/**
 * @brief Type for referring to the attribute changed callback function.
 *
 * This function is called just after an attribute changes.
 */
typedef void (*ChipZclClusterAttributeChangedCallback)(uint8_t endpoint, ChipZclAttributeId attributeId);

/**
 * @brief Type for referring to the manufacturer specific
 *        attribute changed callback function.
 *
 * This function is called just after a manufacturer specific attribute changes.
 */
typedef void (*ChipZclManufacturerSpecificClusterAttributeChangedCallback)(uint8_t endpoint, ChipZclAttributeId attributeId,
                                                                           uint16_t manufacturerCode);

/**
 * @brief Type for referring to the pre-attribute changed callback function.
 *
 * This function is called before an attribute changes.
 */
typedef ChipZclStatus_t (*ChipZclClusterPreAttributeChangedCallback)(uint8_t endpoint, ChipZclAttributeId attributeId,
                                                                     ChipZclAttributeType attributeType, uint8_t size,
                                                                     uint8_t * value);

/**
 * @brief Type for referring to the default response callback function.
 *
 * This function is called when default response is received, before
 * the global callback. Global callback is called immediately afterwards.
 */
typedef void (*ChipZclDefaultResponseFunction)(uint8_t endpoint, uint8_t commandId, ChipZclStatus_t status);

/**
 * @brief A data struct used to keep track of incoming and outgoing
 *   commands for command discovery
 */
typedef struct
{
    uint16_t clusterId;
    uint8_t commandId;
    uint8_t mask;
} ChipZclCommandMetadata;

/**
 * @brief A data structure used to describe the time in a human
 * understandable format (as opposed to 32-bit UTC)
 */

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} ChipZclTimeStruct;

/**
 * @brief A data structure used to describe the ZCL Date data type
 */

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t dayOfMonth;
    uint8_t dayOfWeek;
} ChipZclDate;

/**
 * @brief Zigbee Internet Client/Server Remote Cluster Types
 */
typedef uint16_t ChipZclRemoteClusterType;

#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_NONE 0x0000
#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_SERVER 0x0001
#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_CLIENT 0x0002
#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_INVALID 0xFFFF

/**
 * @brief Zigbee Internet Client/Server remote cluster struct.
 */
typedef struct
{
    ChipZclClusterId clusterId;
    ChipZclProfileId profileId;
    uint16_t deviceId;
    uint8_t endpoint;
    ChipZclRemoteClusterType type;
} ChipZclRemoteClusterStruct;

typedef struct
{
    ChipZclClusterId clusterId;
    bool server;
} ChipZclClusterInfo;

#if !defined(CHIP_ZCL_MAX_CLUSTERS_PER_ENDPOINT)
#define CHIP_ZCL_MAX_CLUSTERS_PER_ENDPOINT 3
#endif

/**
 * @brief A struct containing basic information about an endpoint.
 */
typedef struct
{
    ChipZclClusterInfo clusters[CHIP_ZCL_MAX_CLUSTERS_PER_ENDPOINT];
    ChipZclProfileId profileId;
    uint16_t deviceId;
    uint8_t endpoint;
    uint8_t clusterCount;
} ChipZclEndpointInfoStruct;

#define CHIP_ZCL_INVALID_CLUSTER_ID 0xFFFF

#define CHIP_ZCL_INVALID_ENDPOINT 0xFF

#define CHIP_ZCL_INVALID_PAN_ID 0xFFFF

/**
 * @brief macro that returns true if the cluster is in the manufacturer specific range
 *
 * @param cluster EmberAfCluster* to consider
 */
#define chipZclClusterIsManufacturerSpecific(cluster) ((cluster)->clusterId >= 0xFC00)

/**
 * @brief macro that returns true if attribute is read only.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsReadOnly(metadata) (((metadata)->mask & ATTRIBUTE_MASK_WRITABLE) == 0)

/**
 * @brief macro that returns true if client attribute, and false if server.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsClient(metadata) (((metadata)->mask & ATTRIBUTE_MASK_CLIENT) != 0)

/**
 * @brief macro that returns true if attribute is saved to token.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsTokenized(metadata) (((metadata)->mask & ATTRIBUTE_MASK_TOKENIZE) != 0)

/**
 * @brief macro that returns true if attribute is saved in external storage.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsExternal(metadata) (((metadata)->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE) != 0)

/**
 * @brief macro that returns true if attribute is a singleton
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsSingleton(metadata) (((metadata)->mask & ATTRIBUTE_MASK_SINGLETON) != 0)

/**
 * @brief macro that returns true if attribute is manufacturer specific
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsManufacturerSpecific(metadata) (((metadata)->mask & ATTRIBUTE_MASK_MANUFACTURER_SPECIFIC) != 0)

/**
 * @brief macro that returns size of attribute in bytes.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeSize(metadata) ((metadata)->size)

/**
 *@brief Returns true if type is signed, false otherwise.
 */
bool chipZclIsTypeSigned(ChipZclAttributeType dataType);

/**
 * @brief Simple integer comparison function.
 * Compares two values of a known length as integers.
 * Signed integer comparison are supported for numbers with length of
 * 4 (bytes) or less.
 * The integers are in native endianness.
 *
 * @return -1, if val1 is smaller
 *          0, if they are the same or if two negative numbers with length
 *          greater than 4 is being compared
 *          1, if val2 is smaller.
 */
int8_t chipZclCompareValues(uint8_t * val1, uint8_t * val2, uint8_t len, bool signedNumber);

/**
 * @brief Returns the total number of endpoints (dynamic and pre-compiled).
 */
uint8_t chipZclEndpointCount(void);

/*
 * @brief Function that determines the length of a zigbee Cluster Library string
 *   (where the first byte is assumed to be the length).
 */
uint8_t chipZclStringLength(const uint8_t * buffer);
/*
 * @brief Function that determines the length of a zigbee Cluster Library long string.
 *   (where the first two bytes are assumed to be the length).
 */
uint16_t chipZclLongStringLength(const uint8_t * buffer);

/*
 * @brief Function that returns a metadata structure for the given attribute.
 * Returns null if none is found.
 */
ChipZclAttributeMetadata * chipZclLocateAttributeMetadata(uint8_t endpoint, ChipZclClusterId clusterId,
                                                          ChipZclAttributeId attributeId, uint8_t mask, uint16_t manufacturerCode);

/*
 * @brief Initialization of the endpoint structure. Before this call, endpoints are not enabled. After this call endpoints
 * are enabled and linked together properly for use.
 */
void chipZclEndpointInit(void);

// Endpoint Management
ChipZclEndpointId_t chipZclEndpointIndexToId(ChipZclEndpointIndex_t index, const ChipZclClusterSpec_t * clusterSpec);

#endif // SILABS_AF_API_TYPES
