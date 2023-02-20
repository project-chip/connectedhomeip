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

#include <stdbool.h> // For bool
#include <stddef.h>  // For NULL.
#include <stdint.h>  // For various uint*_t types

#include <app/util/af-enums.h>
#include <app/util/basic-types.h>
#include <app/util/types_stub.h> // For various types.

#include <app/util/attribute-metadata.h> // EmberAfAttributeMetadata

#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <lib/support/Variant.h>
#include <messaging/ExchangeContext.h>

#include <app-common/zap-generated/cluster-enums.h>

#ifdef EZSP_HOST
#include "app/util/ezsp/ezsp-enum.h"
#endif

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
#define EMBER_AF_NULL_MANUFACTURER_CODE 0x0000

/**
 * @brief Struct describing cluster
 */
typedef struct
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
} EmberAfCluster;

/**
 * @brief Struct that represents a logical device type consisting
 * of a DeviceID and its version.
 */
typedef struct
{
    uint16_t deviceId;
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

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfEndpointBitmask;
#else
typedef uint8_t EmberAfEndpointBitmask;
enum
#endif
{ EMBER_AF_ENDPOINT_DISABLED = 0x00,
  EMBER_AF_ENDPOINT_ENABLED  = 0x01,
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
    EmberAfEndpointBitmask bitmask = EMBER_AF_ENDPOINT_DISABLED;
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
};

// Cluster specific types

/**
 * @brief This is an enum used to control how the device will poll for a given
 * active cluster-related event.  When the event is scheduled, the application
 * can pass a poll control value which will be stored along with the event.
 * The processor is only allowed to poll according to the most restrictive
 * value for all active event.  For instance, if two events are active, one
 * with EMBER_AF_LONG_POLL and the other with EMBER_AF_SHORT_POLL, then the
 * processor will short poll until the second event is deactivated.
 */
typedef enum
{
    EMBER_AF_LONG_POLL,
    EMBER_AF_SHORT_POLL,
} EmberAfEventPollControl;

/**
 * @brief This is an enum used to control how the device
 *        will sleep for a given active cluster related event.
 *        When the event is scheduled, the scheduling code can
 *        pass a sleep control value which will be stored along
 *        with the event. The processor is only allowed to sleep
 *        according to the most restrictive sleep control value
 *        for any active event. For instance, if two events
 *        are active, one with EMBER_AF_OK_TO_HIBERNATE and the
 *        other with EMBER_AF_OK_TO_NAP, then the processor
 *        will only be allowed to nap until the second event
 *        is deactivated.
 */
typedef enum
{
    EMBER_AF_OK_TO_SLEEP,
    /** deprecated. */
    EMBER_AF_OK_TO_HIBERNATE = EMBER_AF_OK_TO_SLEEP,
    /** deprecated. */
    EMBER_AF_OK_TO_NAP,
    EMBER_AF_STAY_AWAKE,
} EmberAfEventSleepControl;

/**
 * @brief An enum used to track the tasks that the Application
 * framework cares about. These are intended to be tasks
 * that should keep the device out of hibernation like an
 * application level request / response. If the response does
 * not come in as a data ack, then the application will need
 * to stay out of hibernation to wait and poll for it.
 *
 * Of course some tasks do not necessarily have a response. For
 * instance, a ZDO request may or may not have a response. In this
 * case, the application framework cannot rely on the fact that
 * a response will come in to end the wake cycle, so the Application
 * framework must timeout the wake cycle if no expected
 * response is received or no other event can be relied upon to
 * end the wake cycle.
 *
 * Tasks of this type should be added to the wake timeout mask
 * by calling ::emberAfSetWakeTimeoutBitmaskCallback so that they
 * can be governed by a timeout instead of a request / response
 *
 * the current tasks bitmask is an uint32_t bitmask used to
 * track which tasks are active at any given time. The bottom 16 bits,
 * values 0x01 - 0x8000 are reserved for Ember's use. The top
 * 16 bits are reserved for the customer, values 0x10000 -
 * 0x80000000
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfApplicationTask
#else
typedef uint32_t EmberAfApplicationTask;
enum
#endif
{
    // we may be able to remove these top two since they are
    // handled by the stack on the SOC.
    EMBER_AF_WAITING_FOR_DATA_ACK                     = 0x00000001, // not needed?
    EMBER_AF_LAST_POLL_GOT_DATA                       = 0x00000002, // not needed?
    EMBER_AF_WAITING_FOR_SERVICE_DISCOVERY            = 0x00000004,
    EMBER_AF_WAITING_FOR_ZDO_RESPONSE                 = 0x00000008,
    EMBER_AF_WAITING_FOR_ZCL_RESPONSE                 = 0x00000010,
    EMBER_AF_WAITING_FOR_REGISTRATION                 = 0x00000020,
    EMBER_AF_WAITING_FOR_PARTNER_LINK_KEY_EXCHANGE    = 0x00000040,
    EMBER_AF_FORCE_SHORT_POLL                         = 0x00000080,
    EMBER_AF_FRAGMENTATION_IN_PROGRESS                = 0x00000100,
    EMBER_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY = 0x00000200,
};

/**
 * @brief a structure used to keep track of cluster related events and
 * their sleep control values. The cluster code will not know at
 * runtime all of the events that it has access to in the event table
 * This structure is stored by the application framework in an event
 * context table which along with helper functions allows the cluster
 * code to schedule and deactivate its associated events.
 */
typedef struct
{
    /**
     * The endpoint of the associated cluster event.
     */
    chip::EndpointId endpoint;
    /**
     * The cluster id of the associated cluster event.
     */
    chip::ClusterId clusterId;
    /**
     * The server/client identity of the associated cluster event.
     */
    bool isClient;
    /**
     * A poll control value used to control the network polling behavior while
     * the event is active.
     */
    EmberAfEventPollControl pollControl;
    /**
     * A sleep control value used to control the processor's sleep
     * behavior while the event is active.
     */
    EmberAfEventSleepControl sleepControl;
    /**
     * A pointer to the event control value which is stored in the event table
     * and is used to actually schedule the event.
     */
    EmberEventControl * eventControl;
} EmberAfEventContext;

/**
 * @brief Type for referring to the handler for network events.
 */
typedef void (*EmberAfNetworkEventHandler)(void);

/**
 * @brief Type for referring to the handler for endpoint events.
 */
typedef void (*EmberAfEndpointEventHandler)(chip::EndpointId endpoint);

/**
 * @brief Indicates the absence of a Scene table entry.
 */
#define EMBER_AF_SCENE_TABLE_NULL_INDEX 0xFF
/**
 * @brief Value used when setting or getting the endpoint in a Scene table
 * entry.  It indicates that the entry is not in use.
 */
#define EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID 0x00
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
typedef EmberAfStatus (*EmberAfClusterPreAttributeChangedCallback)(const chip::app::ConcreteAttributePath & attributePath,
                                                                   EmberAfAttributeType attributeType, uint16_t size,
                                                                   uint8_t * value);

/**
 * @brief The length of the hardware tag in the Ember Bootloader Query
 *   Response.
 */
#define EMBER_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH 16

/**
 * @brief A data structure used to describe the ZCL Date data type
 */

/* Simple Metering Server Test Code */
#define EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_ELECTRIC_METER 0
#define EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_GAS_METER 1

// Functional Notification Flags
// Also #defined in enums.h under slightly different names
#define EMBER_AF_METERING_FNF_NEW_OTA_FIRMWARE 0x00000001
#define EMBER_AF_METERING_FNF_CBKE_UPDATE_REQUEST 0x00000002
#define EMBER_AF_METERING_FNF_TIME_SYNC 0x00000004
#define EMBER_AF_METERING_FNF_STAY_AWAKE_REQUEST_HAN 0x00000010
#define EMBER_AF_METERING_FNF_STAY_AWAKE_REQUEST_WAN 0x00000020
#define EMBER_AF_METERING_FNF_PUSH_HISTORICAL_METERING_DATA_ATTRIBUTE_SET 0x000001C0
#define EMBER_AF_METERING_FNF_PUSH_HISTORICAL_PREPAYMENT_DATA_ATTRIBUTE_SET 0x00000E00
#define EMBER_AF_METERING_FNF_PUSH_ALL_STATIC_DATA_BASIC_CLUSTER 0x00001000
#define EMBER_AF_METERING_FNF_PUSH_ALL_STATIC_DATA_METERING_CLUSTER 0x00002000
#define EMBER_AF_METERING_FNF_PUSH_ALL_STATIC_DATA_PREPAYMENT_CLUSTER 0x00004000
#define EMBER_AF_METERING_FNF_NETWORK_KEY_ACTIVE 0x00008000
#define EMBER_AF_METERING_FNF_DISPLAY_MESSAGE 0x00010000
#define EMBER_AF_METERING_FNF_CANCEL_ALL_MESSAGES 0x00020000
#define EMBER_AF_METERING_FNF_CHANGE_SUPPLY 0x00040000
#define EMBER_AF_METERING_FNF_LOCAL_CHANGE_SUPPLY 0x00080000
#define EMBER_AF_METERING_FNF_SET_UNCONTROLLED_FLOW_THRESHOLD 0x00100000
#define EMBER_AF_METERING_FNF_TUNNEL_MESSAGE_PENDING 0x00200000
#define EMBER_AF_METERING_FNF_GET_SNAPSHOT 0x00400000
#define EMBER_AF_METERING_FNF_GET_SAMPLED_DATA 0x00800000
#define EMBER_AF_METERING_FNF_NEW_SUB_GHZ_CHANNEL_MASKS_AVAILABLE 0x01000000
#define EMBER_AF_METERING_FNF_ENERGY_SCAN_PENDING 0x02000000
#define EMBER_AF_METERING_FNF_CHANNEL_CHANGE_PENDING 0x04000000

// Notification Flags 2
#define EMBER_AF_METERING_NF2_PUBLISH_PRICE 0x00000001
#define EMBER_AF_METERING_NF2_PUBLISH_BLOCK_PERIOD 0x00000002
#define EMBER_AF_METERING_NF2_PUBLISH_TARIFF_INFORMATION 0x00000004
#define EMBER_AF_METERING_NF2_PUBLISH_CONVERSION_FACTOR 0x00000008
#define EMBER_AF_METERING_NF2_PUBLISH_CALORIFIC_VALUE 0x00000010
#define EMBER_AF_METERING_NF2_PUBLISH_CO2_VALUE 0x00000020
#define EMBER_AF_METERING_NF2_PUBLISH_BILLING_PERIOD 0x00000040
#define EMBER_AF_METERING_NF2_PUBLISH_CONSOLIDATED_BILL 0x00000080
#define EMBER_AF_METERING_NF2_PUBLISH_PRICE_MATRIX 0x00000100
#define EMBER_AF_METERING_NF2_PUBLISH_BLOCK_THRESHOLDS 0x00000200
#define EMBER_AF_METERING_NF2_PUBLISH_CURRENCY_CONVERSION 0x00000400
#define EMBER_AF_METERING_NF2_PUBLISH_CREDIT_PAYMENT_INFO 0x00001000
#define EMBER_AF_METERING_NF2_PUBLISH_CPP_EVENT 0x00002000
#define EMBER_AF_METERING_NF2_PUBLISH_TIER_LABELS 0x00004000
#define EMBER_AF_METERING_NF2_CANCEL_TARIFF 0x00008000

// Notification Flags 3
#define EMBER_AF_METERING_NF3_PUBLISH_CALENDAR 0x00000001
#define EMBER_AF_METERING_NF3_PUBLISH_SPECIAL_DAYS 0x00000002
#define EMBER_AF_METERING_NF3_PUBLISH_SEASONS 0x00000004
#define EMBER_AF_METERING_NF3_PUBLISH_WEEK 0x00000008
#define EMBER_AF_METERING_NF3_PUBLISH_DAY 0x00000010
#define EMBER_AF_METERING_NF3_CANCEL_CALENDAR 0x00000020

// Notification Flags 4
#define EMBER_AF_METERING_NF4_SELECT_AVAILABLE_EMERGENCY_CREDIT 0x00000001
#define EMBER_AF_METERING_NF4_CHANGE_DEBT 0x00000002
#define EMBER_AF_METERING_NF4_EMERGENCY_CREDIT_SETUP 0x00000004
#define EMBER_AF_METERING_NF4_CONSUMER_TOP_UP 0x00000008
#define EMBER_AF_METERING_NF4_CREDIT_ADJUSTMENT 0x00000010
#define EMBER_AF_METERING_NF4_CHANGE_PAYMENT_MODE 0x00000020
#define EMBER_AF_METERING_NF4_GET_PREPAY_SNAPSHOT 0x00000040
#define EMBER_AF_METERING_NF4_GET_TOP_UP_LOG 0x00000080
#define EMBER_AF_METERING_NF4_SET_LOW_CREDIT_WARNING_LEVEL 0x00000100
#define EMBER_AF_METERING_NF4_GET_DEBT_REPAYMENT_LOG 0x00000200
#define EMBER_AF_METERING_NF4_SET_MAXIMUM_CREDIT_LIMIT 0x00000400
#define EMBER_AF_METERING_NF4_SET_OVERALL_DEBT_CAP 0x00000800

// Notification Flags 5
#define EMBER_AF_METERING_NF5_PUBLISH_CHANGE_OF_TENANCY 0x00000001
#define EMBER_AF_METERING_NF5_PUBLISH_CHANGE_OF_SUPPLIER 0x00000002
#define EMBER_AF_METERING_NF5_REQUEST_NEW_PASSWORD_1_RESPONSE 0x00000004
#define EMBER_AF_METERING_NF5_REQUEST_NEW_PASSWORD_2_RESPONSE 0x00000008
#define EMBER_AF_METERING_NF5_REQUEST_NEW_PASSWORD_3_RESPONSE 0x00000010
#define EMBER_AF_METERING_NF5_REQUEST_NEW_PASSWORD_4_RESPONSE 0x00000020
#define EMBER_AF_METERING_NF5_UPDATE_SITE_ID 0x00000040
#define EMBER_AF_METERING_NF5_RESET_BATTERY_COUNTER 0x00000080
#define EMBER_AF_METERING_NF5_UPDATE_CIN 0x00000100

/**
 * @brief Device Management plugin types
 */

#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_NAME_LENGTH (16)
#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_CONTACT_DETAILS_LENGTH (18)
#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH (32)
#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_CIN_LENGTH (24)
#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PASSWORD_LENGTH (10)

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfDeviceManagementPasswordType
#else
typedef uint16_t EmberAfDeviceManagementPasswordType;
enum
#endif
{
    UNUSED_PASSWORD   = 0x00,
    SERVICE_PASSWORD  = 0x01,
    CONSUMER_PASSWORD = 0x02,
};

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfDeviceManagementChangePendingFlags
#else
typedef uint8_t EmberAfDeviceManagementChangePendingFlags;
enum
#endif
{
    EMBER_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK        = 0x01,
    EMBER_AF_DEVICE_MANAGEMENT_CHANGE_OF_SUPPLIER_PENDING_MASK       = 0x02,
    EMBER_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK           = 0x04,
    EMBER_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK               = 0x08,
    EMBER_AF_DEVICE_MANAGEMENT_UPDATE_SERVICE_PASSWORD_PENDING_MASK  = 0x10,
    EMBER_AF_DEVICE_MANAGEMENT_UPDATE_CONSUMER_PASSWORD_PENDING_MASK = 0x20,
};

typedef struct
{
    // Optional fields only used by Gas Proxy Function plugin.
    uint32_t providerId;
    uint32_t issuerEventId;
    uint8_t tariffType;

    // always used fields
    uint32_t implementationDateTime;
    uint32_t tenancy;
} EmberAfDeviceManagementTenancy;

typedef struct
{
    uint32_t proposedProviderId;
    uint32_t implementationDateTime;
    uint32_t providerChangeControl;
    uint8_t proposedProviderName[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_NAME_LENGTH + 1];
    uint8_t proposedProviderContactDetails[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_CONTACT_DETAILS_LENGTH + 1];
} EmberAfDeviceManagementSupplier;

typedef struct
{
    uint32_t requestDateTime;
    uint32_t implementationDateTime;
    uint8_t supplyStatus;
    uint8_t originatorIdSupplyControlBits;
} EmberAfDeviceManagementSupply;

typedef struct
{
    uint8_t siteId[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH + 1];
    uint32_t implementationDateTime;
    uint32_t issuerEventId;
} EmberAfDeviceManagementSiteId;

typedef struct
{
    uint8_t cin[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_CIN_LENGTH + 1];
    uint32_t implementationDateTime;
    uint32_t issuerEventId;
} EmberAfDeviceManagementCIN;

typedef struct
{
    bool supplyTamperState;
    bool supplyDepletionState;
    bool supplyUncontrolledFlowState;
    bool loadLimitSupplyState;
} EmberAfDeviceManagementSupplyStatusFlags;

typedef struct
{
    uint16_t uncontrolledFlowThreshold;
    uint16_t multiplier;
    uint16_t divisor;
    uint16_t measurementPeriod;
    uint8_t unitOfMeasure;
    uint8_t stabilisationPeriod;
} EmberAfDeviceManagementUncontrolledFlowThreshold;

typedef struct
{
    uint32_t implementationDateTime;
    uint8_t supplyStatus;
} EmberAfDeviceManagementSupplyStatus;

typedef struct
{
    uint8_t password[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PASSWORD_LENGTH + 1];
    uint32_t implementationDateTime;
    uint16_t durationInMinutes;
    EmberAfDeviceManagementPasswordType passwordType;
} EmberAfDeviceManagementPassword;

typedef struct
{
    uint8_t startIndex;
    uint8_t endIndex;
} EmberAfDeviceManagementAttributeRange;

// attrRange is a list of attributeId values in a cluster. It's needed to track contigous
// segments of valid attributeId's with gaps in the middle.
// attributeSetId is the value of the upper byte in the attributeId. It ranges from 0x01(Price)
// to 0x08(OTA Event Configuration)
// Eg. {0x00,0x05} and {0x08,0x0A}
// We're betting that there isn't a list of cluster attributes that has more than 5 gaps.
typedef struct
{
    uint8_t attributeSetId;
    EmberAfDeviceManagementAttributeRange attributeRange[7];
} EmberAfDeviceManagementAttributeTable;

typedef struct
{
    bool encryption;

    uint8_t * plainPayload;
    uint16_t plainPayloadLength;

    uint8_t * encryptedPayload;
    uint16_t encryptedPayloadLength;
} EmberAfGbzMessageData;

typedef struct
{
    uint8_t * gbzCommands;
    uint16_t gbzCommandsLength;
    uint8_t * gbzCommandsResponse;
    uint16_t gbzCommandsResponseLength;
    uint16_t messageCode;
} EmberAfGpfMessage;

/**
 * @brief Zigbee Internet Client/Server Remote Cluster Types
 */
typedef uint16_t EmberAfRemoteClusterType;

#define EMBER_AF_REMOTE_CLUSTER_TYPE_NONE 0x0000
#define EMBER_AF_REMOTE_CLUSTER_TYPE_SERVER 0x0001
#define EMBER_AF_REMOTE_CLUSTER_TYPE_CLIENT 0x0002
#define EMBER_AF_REMOTE_CLUSTER_TYPE_INVALID 0xFFFF

/**
 * @brief Zigbee Internet Client/Server remote cluster struct.
 */
typedef struct
{
    chip::ClusterId clusterId;
    uint16_t deviceId;
    chip::EndpointId endpoint;
    EmberAfRemoteClusterType type;
} EmberAfRemoteClusterStruct;

/**
 * @brief Zigbee Internet Client/Server Remote Binding struct
 */
typedef struct
{
    EmberEUI64 targetEUI64;
    chip::EndpointId sourceEndpoint;
    chip::EndpointId destEndpoint;
    chip::ClusterId clusterId;
    EmberEUI64 destEUI64;
    EmberEUI64 sourceEUI64;
} EmberAfRemoteBindingStruct;

typedef struct
{
    chip::ClusterId clusterId;
    bool server;
} EmberAfClusterInfo;

#if !defined(EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT)
#define EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT 3
#endif

/**
 * @brief A struct containing basic information about an endpoint.
 */
typedef struct
{
    EmberAfClusterInfo clusters[EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT];
    uint16_t deviceId;
    chip::EndpointId endpoint;
    uint8_t clusterCount;
} EmberAfEndpointInfoStruct;

#if !defined(EMBER_AF_MAX_ENDPOINTS_PER_DEVICE)
#define EMBER_AF_MAX_ENDPOINTS_PER_DEVICE 1
#endif

// Although we treat this like a bitmap, only 1 bit is set at a time.
// We use the bitmap feature to allow us to find all devices
// with any in a set of status codes using
// emberAfPluginDeviceDatabaseFindDeviceByStatus().
typedef enum
{
    EMBER_AF_DEVICE_DISCOVERY_STATUS_NONE                = 0x00,
    EMBER_AF_DEVICE_DISCOVERY_STATUS_NEW                 = 0x01,
    EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_ENDPOINTS      = 0x02,
    EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_CLUSTERS       = 0x04,
    EMBER_AF_DEVICE_DISCOVERY_STATUS_FIND_STACK_REVISION = 0x08,

    EMBER_AF_DEVICE_DISCOVERY_STATUS_DONE   = 0x40,
    EMBER_AF_DEVICE_DISCOVERY_STATUS_FAILED = 0x80,
} EmberAfDeviceDiscoveryStatus;

/**
 * @brief A struct containing endpoint information about a device.
 */
typedef struct
{
    EmberEUI64 eui64;
    EmberAfEndpointInfoStruct endpoints[EMBER_AF_MAX_ENDPOINTS_PER_DEVICE];
    EmberAfDeviceDiscoveryStatus status;
    uint8_t discoveryFailures;
    uint8_t capabilities;
    uint8_t endpointCount;
    uint8_t stackRevision;
} EmberAfDeviceInfo;

typedef struct
{
    uint16_t deviceIndex;
} EmberAfDeviceDatabaseIterator;

typedef struct
{
    EmberNodeId emberNodeId;
    uint32_t timeStamp;
} EmberAfJoiningDevice;

/**
 * @brief Permit join times
 */
#define EMBER_AF_PERMIT_JOIN_FOREVER 0xFF
#define EMBER_AF_PERMIT_JOIN_MAX_TIMEOUT 0xFE

#define MAX_INT32U_VALUE (0xFFFFFFFFUL)
#define MAX_INT16U_VALUE (0xFFFF)
#define MAX_INT8U_VALUE (0xFF)

/**
 * @brief Returns the elapsed time between two 32 bit values.
 *   Result may not be valid if the time samples differ by more than 2147483647
 */
#define elapsedTimeInt32u(oldTime, newTime) ((uint32_t)((uint32_t)(newTime) - (uint32_t)(oldTime)))

/**
 * @brief The overhead of the ZDO response.
 *   1 byte for the sequence and 1 byte for the status code.
 */
#define EMBER_AF_ZDO_RESPONSE_OVERHEAD 2

/** @} END addtogroup */
