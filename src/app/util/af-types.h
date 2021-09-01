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
 * @brief The include file for all the types for Ember
 *ApplicationFramework
 *******************************************************************************
 ******************************************************************************/

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

#include <lib/support/Variant.h>
#include <messaging/ExchangeContext.h>

#ifdef EZSP_HOST
#include "app/util/ezsp/ezsp-enum.h"
#endif

/**
 * @brief Type for referring to ZCL attribute type
 */
typedef uint8_t EmberAfAttributeType;

/**
 * @brief Type for the cluster mask
 */
typedef uint8_t EmberAfClusterMask;

/**
 * @brief Type for the attribute mask
 */
typedef uint8_t EmberAfAttributeMask;

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
 * @brief Type for default values.
 *
 * Default value is either a value itself, if it is 2 bytes or less,
 * or a pointer to the value itself, if attribute type is longer than
 * 2 bytes.
 */
union EmberAfDefaultAttributeValue
{
    constexpr EmberAfDefaultAttributeValue(uint8_t * ptr) : ptrToDefaultValue(ptr) {}
    constexpr EmberAfDefaultAttributeValue(uint16_t val) : defaultValue(val) {}

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
};

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
    EmberAfDefaultAttributeValue defaultValue;
    /**
     * Minimum allowed value
     */
    EmberAfDefaultAttributeValue minValue;
    /**
     * Maximum allowed value.
     */
    EmberAfDefaultAttributeValue maxValue;
} EmberAfAttributeMinMaxValue;

/**
 * @brief Union describing the attribute default/min/max values.
 */
union EmberAfDefaultOrMinMaxAttributeValue
{
    constexpr EmberAfDefaultOrMinMaxAttributeValue(uint8_t * ptr) : ptrToDefaultValue(ptr) {}
    constexpr EmberAfDefaultOrMinMaxAttributeValue(uint16_t val) : defaultValue(val) {}

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
    EmberAfAttributeMinMaxValue * ptrToMinMaxValue;
};

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
    chip::AttributeId attributeId;
    /**
     * Attribute type, according to ZCL specs.
     */
    EmberAfAttributeType attributeType;
    /**
     * Size of this attribute in bytes.
     */
    uint16_t size;
    /**
     * Attribute mask, tagging attribute with specific
     * functionality. See ATTRIBUTE_MASK_ macros defined
     * in att-storage.h.
     */
    EmberAfAttributeMask mask;
    /**
     * Pointer to the default value union. Actual value stored
     * depends on the mask.
     */
    EmberAfDefaultOrMinMaxAttributeValue defaultValue;
} EmberAfAttributeMetadata;

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
    EmberAfAttributeMetadata * attributes;
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
} EmberAfCluster;

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
     * Cluster that the attribute is located on. If the cluster
     * id is inside the manufacturer specific range, 0xfc00 - 0xffff,
     * The manufacturer code should also be set to the code associated
     * with the manufacturer specific cluster.
     */
    chip::ClusterId clusterId;

    /**
     * Cluster mask for the cluster, used to determine if it is
     * the server or client version of the cluster. See CLUSTER_MASK_
     * macros defined in att-storage.h
     */
    EmberAfClusterMask clusterMask;

    /**
     * The two byte identifier for the attribute. If the cluster id is
     * inside the manufacturer specific range 0xfc00 - 0xffff, or the manufacturer
     * code is NOT 0, the attribute is assumed to be manufacturer specific.
     */
    chip::AttributeId attributeId;

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
} EmberAfAttributeSearchRecord;

/**
 * A struct used to construct a table of manufacturer codes for
 * manufacturer specific attributes and clusters.
 */
typedef struct
{
    uint16_t index;
    uint16_t manufacturerCode;
} EmberAfManufacturerCodeEntry;

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
 * @brief a struct containing the superset of values
 * passed to both emberIncomingMessageHandler on the SOC and
 * ezspIncomingMessageHandler on the host.
 */
typedef struct
{
    /**
     * The type of the incoming message
     */
    EmberIncomingMessageType type;
    /**
     * APS frame for the incoming message
     */
    EmberApsFrame * apsFrame;
    /**
     * The message copied into a flat buffer
     */
    uint8_t * message;
    /**
     * Length of the incoming message
     */
    uint16_t msgLen;
    /**
     * Two byte node id of the sending node.
     */
    uint16_t source;
    /**
     * Link quality from the node that last relayed
     * the message.
     */
    uint8_t lastHopLqi;
    /**
     * The energy level (in units of dBm) observed during the reception.
     */
    int8_t lastHopRssi;
    /**
     * The index of a binding that matches the message
     * or 0xFF if there is no matching binding.
     */
    uint8_t bindingTableIndex;
    /**
     * The index of the entry in the address table
     * that matches the sender of the message or 0xFF
     * if there is no matching entry.
     */
    uint8_t addressTableIndex;
    /**
     * The index of the network on which this message was received.
     */
    uint8_t networkIndex;
} EmberAfIncomingMessage;

/**
 * @brief Interpan Message type: unicast, broadcast, or multicast.
 */
typedef uint8_t EmberAfInterpanMessageType;
#define EMBER_AF_INTER_PAN_UNICAST 0x00
#define EMBER_AF_INTER_PAN_BROADCAST 0x08
#define EMBER_AF_INTER_PAN_MULTICAST 0x0C

// Legacy names
#define INTER_PAN_UNICAST EMBER_AF_INTER_PAN_UNICAST
#define INTER_PAN_BROADCAST EMBER_AF_INTER_PAN_BROADCAST
#define INTER_PAN_MULTICAST EMBER_AF_INTER_PAN_MULTICAST

#define EMBER_AF_INTERPAN_OPTION_NONE 0x0000
#define EMBER_AF_INTERPAN_OPTION_APS_ENCRYPT 0x0001
#define EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS 0x0002

/**
 * @brief The options for sending/receiving interpan messages.
 */
typedef uint16_t EmberAfInterpanOptions;

/**
 * @brief Interpan header used for sending and receiving interpan
 *   messages.
 */
typedef struct
{
    EmberAfInterpanMessageType messageType;

    /**
     * MAC addressing
     * For outgoing messages this is the destination.  For incoming messages
     * it is the source, which always has a long address.
     */
    EmberEUI64 longAddress;
    EmberNodeId shortAddress;
    EmberPanId panId;

    /**
     * APS data
     */
    chip::ClusterId clusterId;
    /**
     * The groupId is only used for
     * EMBER_AF_INTERPAN_MULTICAST
     */
    chip::GroupId groupId;
    EmberAfInterpanOptions options;
} EmberAfInterpanHeader;

// Legacy Name
#define InterPanHeader EmberAfInterpanHeader

/**
 * @brief The options for what interpan messages are allowed.
 */
typedef uint8_t EmberAfAllowedInterpanOptions;

#define EMBER_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER 0x01
#define EMBER_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT 0x02
#define EMBER_AF_INTERPAN_DIRECTION_BOTH 0x03
#define EMBER_AF_INTERPAN_GLOBAL_COMMAND 0x04
#define EMBER_AF_INTERPAN_MANUFACTURER_SPECIFIC 0x08

/**
 * @brief This structure is used define an interpan message that
 *   will be accepted by the interpan filters.
 */
typedef struct
{
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    EmberAfAllowedInterpanOptions options;
} EmberAfAllowedInterPanMessage;

/**
 * @brief The EmberAFClusterCommand is a struct wrapper
 *   for all the data pertaining to a command which comes
 *   in over the air. This enables struct is used to
 *   encapsulate a command in a single place on the stack
 *   and pass a pointer to that location around during
 *   command processing
 */
struct EmberAfClusterCommand
{
    chip::NodeId SourceNodeId() const { return source->GetSecureSession().GetPeerNodeId(); }

    /**
     * APS frame for the incoming message
     */
    EmberApsFrame * apsFrame;
    EmberIncomingMessageType type;
    chip::Messaging::ExchangeContext * source;
    uint8_t * buffer;
    uint16_t bufLen;
    bool clusterSpecific;
    bool mfgSpecific;
    uint16_t mfgCode;
    uint8_t seqNum;
    chip::CommandId commandId;
    uint8_t payloadStartIndex;
    uint8_t direction;
    EmberAfInterpanHeader * interPanHeader;
    uint8_t networkIndex;
};

/**
 * @brief Endpoint type struct describes clusters that are on the endpoint.
 */
typedef struct
{
    /**
     * Pointer to the cluster structs, describing clusters on this
     * endpoint type.
     */
    EmberAfCluster * cluster;
    /**
     * Number of clusters in this endpoint type.
     */
    uint8_t clusterCount;
    /**
     * Size of all non-external, non-singlet attribute in this endpoint type.
     */
    uint16_t endpointSize;
} EmberAfEndpointType;

#ifdef EZSP_HOST
typedef EzspDecisionId EmberAfTcLinkKeyRequestPolicy;
typedef EzspDecisionId EmberAfAppLinkKeyRequestPolicy;
#define EMBER_AF_ALLOW_TC_KEY_REQUESTS EZSP_ALLOW_TC_KEY_REQUESTS_AND_SEND_CURRENT_KEY
#define EMBER_AF_DENY_TC_KEY_REQUESTS EZSP_DENY_TC_KEY_REQUESTS
#define EMBER_AF_ALLOW_APP_KEY_REQUESTS EZSP_ALLOW_APP_KEY_REQUESTS
#define EMBER_AF_DENY_APP_KEY_REQUESTS EZSP_DENY_APP_KEY_REQUESTS
#else
typedef EmberTcLinkKeyRequestPolicy EmberAfTcLinkKeyRequestPolicy;
typedef EmberAppLinkKeyRequestPolicy EmberAfAppLinkKeyRequestPolicy;
#define EMBER_AF_ALLOW_TC_KEY_REQUESTS EMBER_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY
#define EMBER_AF_DENY_TC_KEY_REQUESTS EMBER_DENY_TC_LINK_KEY_REQUESTS
#define EMBER_AF_ALLOW_APP_KEY_REQUESTS EMBER_ALLOW_APP_LINK_KEY_REQUEST
#define EMBER_AF_DENY_APP_KEY_REQUESTS EMBER_DENY_APP_LINK_KEY_REQUESTS
#endif

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
typedef struct
{
    /**
     * Actual zigbee endpoint number.
     */
    chip::EndpointId endpoint;
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
    EmberAfEndpointType * endpointType;
    /**
     * Network index for this endpoint.
     */
    uint8_t networkIndex;
    /**
     * Meta-data about the endpoint
     */
    EmberAfEndpointBitmask bitmask;
} EmberAfDefinedEndpoint;

// Cluster specific types

/**
 * @brief Bitmask data type for storing one bit of information for each ESI in
 * the ESI table.
 */
#if (EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE <= 8)
typedef uint8_t EmberAfPluginEsiManagementBitmask;
#elif (EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE <= 16)
typedef uint16_t EmberAfPluginEsiManagementBitmask;
#elif (EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE <= 32)
typedef uint32_t EmberAfPluginEsiManagementBitmask;
#else
#error "EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE cannot exceed 32"
#endif

/**
 * @brief Struct that describes a load control event.
 *
 * This is used in the load control event callback and
 * within the demand response load control cluster code.
 */
typedef struct
{
    uint32_t eventId;
#ifdef EMBER_AF_PLUGIN_DRLC_SERVER
    EmberEUI64 source;
    chip::EndpointId sourceEndpoint;
#endif // EMBER_AF_PLUGIN_DRLC_SERVER

#ifdef EMBER_AF_PLUGIN_DRLC
    EmberAfPluginEsiManagementBitmask esiBitmask;
#endif // EMBER_AF_PLUGIN_DRLC

    chip::EndpointId destinationEndpoint;
    uint16_t deviceClass;
    uint8_t utilityEnrollmentGroup;
    /**
     * Start time in seconds
     */
    uint32_t startTime;
    /**
     * Duration in minutes
     */
    uint16_t duration;
    uint8_t criticalityLevel;
    uint8_t coolingTempOffset;
    uint8_t heatingTempOffset;
    int16_t coolingTempSetPoint;
    int16_t heatingTempSetPoint;
    int8_t avgLoadPercentage;
    uint8_t dutyCycle;
    uint8_t eventControl;
    uint32_t startRand;
    uint32_t durationRand;
    uint8_t optionControl;
} EmberAfLoadControlEvent;

/**
 * @brief This is an enum used to indicate the result of the
 *   service discovery.  Unicast discoveries are completed
 *   as soon as a response is received.  Broadcast discoveries
 *   wait a period of time for multiple responses to be received.
 */
typedef enum
{
    EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE                     = 0x00,
    EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED            = 0x01,
    EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT                        = 0x02,
    EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE         = 0x03,
    EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE       = 0x04,
    EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE   = 0x05,
    EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_EMPTY_RESPONSE = 0x06,
} EmberAfServiceDiscoveryStatus;

#define EM_AF_DISCOVERY_RESPONSE_MASK (0x05)

/**
 * @brief A simple way to determine if the service discovery callback
 *   has a response.
 */
#define emberAfHaveDiscoveryResponseStatus(status) ((status) &EM_AF_DISCOVERY_RESPONSE_MASK)

/**
 * @brief A structure containing general information about the service discovery.
 */
typedef struct
{
    /**
     * The status indicates both the type of request (broadcast or unicast)
     * and whether a response has been received.
     */
    EmberAfServiceDiscoveryStatus status;

    /**
     * This indicates what ZDO request cluster was associated with the request.
     * It is helpful for a callback that may be used for multiple ZDO request types
     * to determine the type of data returned.  This will be based on the
     * ZDO cluster values defined in ember-types.h.
     */
    uint16_t zdoRequestClusterId;

    /**
     * This is the address of the device that matched the request, which may
     * be different than the device that *actually* is responding.  This occurs
     * when parents respond on behalf of their children.
     */
    EmberNodeId matchAddress;

    /**
     * Only if the status code indicates a response will this data be non-NULL.
     * When there is data, the type is according to the ZDO cluster ID sent out.
     * For NETWORK_ADDRESS_REQUEST or IEEE_ADDRESS_REQUEST, the long ID will
     * be contained in the responseData,  so it will be a value of type ::EmberEUI64.
     * The short ID will be in the matchAddress parameter field.
     * For the MATCH_DESCRIPTORS_REQUEST the responseData will point
     * to an ::EmberAfEndpointList structure.
     */
    const void * responseData;
} EmberAfServiceDiscoveryResult;

/**
 * @brief A list of endpoints received during a service discovery attempt.
 *   This will be returned for a match descriptor request and a
 *   active endpoint request.
 */
typedef struct
{
    uint8_t count;
    const chip::EndpointId * list;
} EmberAfEndpointList;

/**
 * @brief A list of clusters received during a service discovery attempt.
 * This will be returned for a simple descriptor request.
 */
typedef struct
{
    uint8_t inClusterCount;
    const chip::ClusterId * inClusterList;
    uint8_t outClusterCount;
    const chip::ClusterId * outClusterList;
    uint16_t deviceId;
    chip::EndpointId endpoint;
} EmberAfClusterList;

/**
 * @brief This defines a callback where a code element or cluster can be informed
 * as to the result of a service discovery they have requested.
 * For each match, the callback is fired with all the resulting matches from
 * that source.  If the discovery was unicast to a specific device, then
 * the callback will only be fired once with either MATCH_FOUND or COMPLETE
 * (no matches found).  If the discovery is broadcast then multiple callbacks
 * may be fired with ::EMBER_AF_SERVICE_DISCOVERY_RESPONSE_RECEIVED.
 * After a couple seconds the callback will then be fired with
 * ::EMBER_AF_SERVICE_DISCOVERY_COMPLETE as the result.
 */
typedef void(EmberAfServiceDiscoveryCallback)(const EmberAfServiceDiscoveryResult * result);

/**
 * @brief This defines a callback where a code element or cluster can be
 * informed as to the result of a request to initiate a partner link key
 * exchange.  The callback will be triggered with success equal to true if the
 * exchange completed successfully.
 */
typedef void(EmberAfPartnerLinkKeyExchangeCallback)(bool success);

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

#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
/**
 * @brief Indicates the absence of a Group table entry.
 */
#define EMBER_AF_GROUP_TABLE_NULL_INDEX 0xFF
/**
 * @brief Value used when setting or getting the endpoint in a Group table
 * entry.  It indicates that the entry is not in use.
 */
#define EMBER_AF_GROUP_TABLE_UNUSED_ENDPOINT_ID 0x00
/**
 * @brief Maximum length of Group names, not including the length byte.
 */
#define ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH 16
/**
 * @brief A structure used to store group table entries in RAM or in tokens,
 * depending on the platform.  If the endpoint field is
 * ::EMBER_AF_GROUP_TABLE_UNUSED_ENDPOINT_ID, the entry is unused.
 */
typedef struct
{
    chip::EndpointId endpoint; // 0x00 when not in use
    chip::GroupId groupId;
    uint8_t bindingIndex;
#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER_NAME_SUPPORT
    uint8_t name[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1];
#endif
} EmberAfGroupTableEntry;
#endif // EMBER_AF_PLUGIN_GROUPS_SERVER

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
 * @brief A structure used to store scene table entries in RAM or in tokens,
 * depending on a plugin setting.  If endpoint field is
 * ::EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID, the entry is unused.
 */
typedef struct
{
    chip::EndpointId endpoint; // 0x00 when this record is not in use
    chip::GroupId groupId;     // 0x0000 if not associated with a group
    uint8_t sceneId;
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
    uint8_t name[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1];
#endif
    uint16_t transitionTime;     // in seconds
    uint8_t transitionTime100ms; // in tenths of a seconds
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
    bool hasOnOffValue;
    bool onOffValue;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
    bool hasCurrentLevelValue;
    uint8_t currentLevelValue;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
    bool hasOccupiedCoolingSetpointValue;
    int16_t occupiedCoolingSetpointValue;
    bool hasOccupiedHeatingSetpointValue;
    int16_t occupiedHeatingSetpointValue;
    bool hasSystemModeValue;
    uint8_t systemModeValue;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    bool hasCurrentXValue;
    uint16_t currentXValue;
    bool hasCurrentYValue;
    uint16_t currentYValue;
    bool hasEnhancedCurrentHueValue;
    uint16_t enhancedCurrentHueValue;
    bool hasCurrentSaturationValue;
    uint8_t currentSaturationValue;
    bool hasColorLoopActiveValue;
    uint8_t colorLoopActiveValue;
    bool hasColorLoopDirectionValue;
    uint8_t colorLoopDirectionValue;
    bool hasColorLoopTimeValue;
    uint16_t colorLoopTimeValue;
    bool hasColorTemperatureMiredsValue;
    uint16_t colorTemperatureMiredsValue;
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
    bool hasLockStateValue;
    uint8_t lockStateValue;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
    bool hasCurrentPositionLiftPercentageValue;
    uint8_t currentPositionLiftPercentageValue;
    bool hasCurrentPositionTiltPercentageValue;
    uint8_t currentPositionTiltPercentageValue;
    bool hasTargetPositionLiftPercent100thsValue;
    uint16_t targetPositionLiftPercent100thsValue;
    bool hasTargetPositionTiltPercent100thsValue;
    uint16_t targetPositionTiltPercent100thsValue;
#endif
} EmberAfSceneTableEntry;

#if !defined(EMBER_AF_PLUGIN_MESSAGING_CLIENT)
// In order to be able to forward declare callbacks regardless of whether the plugin
// is enabled, we need to define all data structures.  In order to be able to define
// the messaging client data struct, we need to declare this variable.
#define EMBER_AF_PLUGIN_MESSAGING_CLIENT_MESSAGE_SIZE 0
#endif

typedef struct
{
    bool valid;
    bool active;
    EmberAfPluginEsiManagementBitmask esiBitmask;
    chip::EndpointId clientEndpoint;
    uint32_t messageId;
    uint8_t messageControl;
    uint32_t startTime;
    uint32_t endTime;
    uint16_t durationInMinutes;
    uint8_t message[EMBER_AF_PLUGIN_MESSAGING_CLIENT_MESSAGE_SIZE + 1];
} EmberAfPluginMessagingClientMessage;

#define ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH 11
typedef struct
{
    bool valid;
    bool active;
    chip::EndpointId clientEndpoint;
    uint32_t providerId;
    uint8_t rateLabel[ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH + 1];
    uint32_t issuerEventId;
    uint32_t currentTime;
    uint8_t unitOfMeasure;
    uint16_t currency;
    uint8_t priceTrailingDigitAndPriceTier;
    uint8_t numberOfPriceTiersAndRegisterTier;
    uint32_t startTime;
    uint32_t endTime;
    uint16_t durationInMinutes;
    uint32_t price;
    uint8_t priceRatio;
    uint32_t generationPrice;
    uint8_t generationPriceRatio;
    uint32_t alternateCostDelivered;
    uint8_t alternateCostUnit;
    uint8_t alternateCostTrailingDigit;
    uint8_t numberOfBlockThresholds;
    uint8_t priceControl;
} EmberAfPluginPriceClientPrice;

/**
 * @brief Specifies CPP Authorization values
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfPluginPriceCppAuth
#else
typedef uint8_t EmberAfPluginPriceCppAuth;
enum
#endif
{
    EMBER_AF_PLUGIN_PRICE_CPP_AUTH_PENDING  = 0,
    EMBER_AF_PLUGIN_PRICE_CPP_AUTH_ACCEPTED = 1,
    EMBER_AF_PLUGIN_PRICE_CPP_AUTH_REJECTED = 2,
    EMBER_AF_PLUGIN_PRICE_CPP_AUTH_FORCED   = 3,
    EMBER_AF_PLUGIN_PRICE_CPP_AUTH_RESERVED = 4
};

/**
 * @brief Value used when setting or getting the endpoint in a report table
 * entry.  It indicates that the entry is not in use.
 */
#define EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID 0x00
/**
 * @brief A structure used to store reporting configurations.  If endpoint
 * field is ::EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID, the entry is
 * unused.
 */
typedef struct
{
    /** EMBER_ZCL_REPORTING_DIRECTION_REPORTED for reports sent from the local
     *  device or EMBER_ZCL_REPORTING_DIRECTION_RECEIVED for reports received
     *  from a remote device.
     */
    EmberAfReportingDirection direction;
    /** The local endpoint from which the attribute is reported or to which the
     * report is received.  If ::EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID,
     * the entry is unused.
     */
    chip::EndpointId endpoint;
    /** The cluster where the attribute is located. */
    chip::ClusterId clusterId;
    /** The id of the attribute being reported or received. */
    chip::AttributeId attributeId;
    /** CLUSTER_MASK_SERVER for server-side attributes or CLUSTER_MASK_CLIENT for
     *  client-side attributes.
     */
    uint8_t mask;
    /** Manufacturer code associated with the cluster and/or attribute.  If the
     *  cluster id is inside the manufacturer-specific range, this value
     *  indicates the manufacturer code for the cluster.  Otherwise, if this
     *  value is non-zero and the cluster id is a standard ZCL cluster, it
     *  indicates the manufacturer code for attribute.
     */
    uint16_t manufacturerCode;
    union
    {
        struct
        {
            /** The minimum reporting interval, measured in seconds. */
            uint16_t minInterval;
            /** The maximum reporting interval, measured in seconds. */
            uint16_t maxInterval;
            /** The minimum change to the attribute that will result in a report
             *  being sent.
             */
            uint32_t reportableChange;
        } reported;
        struct
        {
            /** The node id of the source of the received reports. */
            chip::NodeId source;
            /** The remote endpoint from which the attribute is reported. */
            chip::EndpointId endpoint;
            /** The maximum expected time between reports, measured in seconds. */
            uint16_t timeout;
        } received;
    } data;
} EmberAfPluginReportingEntry;

typedef enum
{
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS                          = 0x00,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_BUSY                             = 0x01,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_NO_MORE_TUNNEL_IDS               = 0x02,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_PROTOCOL_NOT_SUPPORTED           = 0x03,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_FLOW_CONTROL_NOT_SUPPORTED       = 0x04,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_REQUEST_FAILED      = 0xF9,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_NOT_FOUND           = 0xFA,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_ADDRESS_TABLE_FULL               = 0xFB,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_REQUEST_FAILED = 0xFC,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_FAILED         = 0xFD,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED            = 0xFE,
    EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_TIMEOUT           = 0xFF,
} EmberAfPluginTunnelingClientStatus;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON
/**
 * @brief Status codes used by the ZLL Commissioning plugin.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfZllCommissioningStatus
#else
typedef uint8_t EmberAfZllCommissioningStatus;
enum
#endif
{
    EMBER_AF_ZLL_ABORTED_BY_APPLICATION                      = 0x00,
    EMBER_AF_ZLL_CHANNEL_CHANGE_FAILED                       = 0x01,
    EMBER_AF_ZLL_JOINING_FAILED                              = 0x02,
    EMBER_AF_ZLL_NO_NETWORKS_FOUND                           = 0x03,
    EMBER_AF_ZLL_PREEMPTED_BY_STACK                          = 0x04,
    EMBER_AF_ZLL_SENDING_START_JOIN_FAILED                   = 0x05,
    EMBER_AF_ZLL_SENDING_DEVICE_INFORMATION_REQUEST_FAILED   = 0x06,
    EMBER_AF_ZLL_SENDING_IDENTIFY_REQUEST_FAILED             = 0x07,
    EMBER_AF_ZLL_SENDING_RESET_TO_FACTORY_NEW_REQUEST_FAILED = 0x08,
    EMBER_AF_ZLL_NETWORK_FORMATION_FAILED                    = 0x09,
    EMBER_AF_ZLL_NETWORK_UPDATE_OPERATION                    = 0x0A,
};

/**
 * @brief A structure used to represent Group Information Records used by ZLL
 * Commissioning.
 */
typedef struct
{
    chip::GroupId groupId;
    uint8_t groupType;
} EmberAfPluginZllCommissioningGroupInformationRecord;

/**
 * @brief A structure used to represent Endpoint Information Records used by
 * ZLL Commissioning.
 */
typedef struct
{
    EmberNodeId networkAddress;
    chip::EndpointId endpointId;
    uint16_t deviceId;
    uint8_t version;
} EmberAfPluginZllCommissioningEndpointInformationRecord;
#endif

typedef enum
{
    NO_APP_MESSAGE               = 0,
    RECEIVED_PARTNER_CERTIFICATE = 1,
    GENERATING_EPHEMERAL_KEYS    = 2,
    GENERATING_SHARED_SECRET     = 3,
    KEY_GENERATION_DONE          = 4,
    GENERATE_SHARED_SECRET_DONE  = 5,
    /**
     * LINK_KEY_ESTABLISHED indicates Success,
     * key establishment done.
     */
    LINK_KEY_ESTABLISHED = 6,

    /**
     * Error codes:
     * Transient failures where Key Establishment could be retried
     */
    NO_LOCAL_RESOURCES                  = 7,
    PARTNER_NO_RESOURCES                = 8,
    TIMEOUT_OCCURRED                    = 9,
    INVALID_APP_COMMAND                 = 10,
    MESSAGE_SEND_FAILURE                = 11,
    PARTNER_SENT_TERMINATE              = 12,
    INVALID_PARTNER_MESSAGE             = 13,
    PARTNER_SENT_DEFAULT_RESPONSE_ERROR = 14,

    /**
     * Fatal Errors:
     * These results are not worth retrying because the outcome
     * will not change
     */
    BAD_CERTIFICATE_ISSUER      = 15,
    KEY_CONFIRM_FAILURE         = 16,
    BAD_KEY_ESTABLISHMENT_SUITE = 17,

    KEY_TABLE_FULL = 18,

    /**
     * Neither initiator nor responder is an
     * ESP/TC so the key establishment is not
     * allowed per the spec.
     */
    NO_ESTABLISHMENT_ALLOWED = 19,

    /* 283k1 certificates need to have valid key usage
     */
    INVALID_CERTIFICATE_KEY_USAGE = 20,
} EmberAfKeyEstablishmentNotifyMessage;

#define APP_NOTIFY_ERROR_CODE_START NO_LOCAL_RESOURCES
#define APP_NOTIFY_MESSAGE_TEXT                                                                                                    \
    {                                                                                                                              \
        "None", "Received Cert", "Generate keys", "Generate secret", "Key generate done", "Generate secret done",                  \
            "Link key verified",                                                                                                   \
                                                                                                                                   \
            /* Transient Error codes */                                                                                            \
            "No local resources", "Partner no resources", "Timeout", "Invalid app. command", "Message send failure",               \
            "Partner sent terminate", "Bad message", "Partner sent Default Rsp",                                                   \
                                                                                                                                   \
            /* Fatal errors */                                                                                                     \
            "Bad cert issuer", "Key confirm failure", "Bad key est. suite", "Key table full", "Not allowed", "Invalid Key Usage",  \
    }

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
typedef void (*EmberAfClusterAttributeChangedCallback)(chip::EndpointId endpoint, chip::AttributeId attributeId);

/**
 * @brief Type for referring to the manufacturer specific
 *        attribute changed callback function.
 *
 * This function is called just after a manufacturer specific attribute changes.
 */
typedef void (*EmberAfManufacturerSpecificClusterAttributeChangedCallback)(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                           uint16_t manufacturerCode);

/**
 * @brief Type for referring to the pre-attribute changed callback function.
 *
 * This function is called before an attribute changes.
 */
typedef EmberAfStatus (*EmberAfClusterPreAttributeChangedCallback)(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                   EmberAfAttributeType attributeType, uint16_t size,
                                                                   uint8_t * value);

/**
 * @brief Type for referring to the default response callback function.
 *
 * This function is called when default response is received, before
 * the global callback. Global callback is called immediately afterwards.
 */
typedef void (*EmberAfDefaultResponseFunction)(chip::EndpointId endpoint, chip::CommandId commandId, EmberAfStatus status);

namespace chip {
/**
 * @brief a type that represents where we are trying to send a message.
 *        The variant type identifies which arm of the union is in use.
 */
class MessageSendDestination
{
public:
    MessageSendDestination(MessageSendDestination & that)       = default;
    MessageSendDestination(const MessageSendDestination & that) = default;
    MessageSendDestination(MessageSendDestination && that)      = default;

    static MessageSendDestination ViaBinding(uint8_t bindingIndex)
    {
        return MessageSendDestination(VariantViaBinding(bindingIndex));
    }

    static MessageSendDestination Direct(NodeId nodeId) { return MessageSendDestination(VariantDirect(nodeId)); }

    static MessageSendDestination Multicast(GroupId groupId) { return MessageSendDestination(VariantMulticast(groupId)); }

    static MessageSendDestination MulticastWithAlias(GroupId groupId)
    {
        return MessageSendDestination(VariantMulticastWithAlias(groupId));
    }

    static MessageSendDestination ViaExchange(Messaging::ExchangeContext * exchangeContext)
    {
        return MessageSendDestination(VariantViaExchange(exchangeContext));
    }

    bool IsViaBinding() const { return mDestination.Is<VariantViaBinding>(); }
    bool IsDirect() const { return mDestination.Is<VariantDirect>(); }
    bool IsViaExchange() const { return mDestination.Is<VariantViaExchange>(); }

    uint8_t GetBindingIndex() const { return mDestination.Get<VariantViaBinding>().mBindingIndex; }
    NodeId GetDirectNodeId() const { return mDestination.Get<VariantDirect>().mNodeId; }
    Messaging::ExchangeContext * GetExchangeContext() const { return mDestination.Get<VariantViaExchange>().mExchangeContext; }

private:
    struct VariantViaBinding
    {
        explicit VariantViaBinding(uint8_t bindingIndex) : mBindingIndex(bindingIndex) {}
        uint8_t mBindingIndex;
    };

    struct VariantViaAddressTable
    {
    };

    struct VariantDirect
    {
        explicit VariantDirect(NodeId nodeId) : mNodeId(nodeId) {}
        NodeId mNodeId;
    };

    struct VariantMulticast
    {
        explicit VariantMulticast(GroupId groupId) : mGroupId(groupId) {}
        GroupId mGroupId;
    };

    struct VariantMulticastWithAlias
    {
        explicit VariantMulticastWithAlias(GroupId groupId) : mGroupId(groupId) {}
        GroupId mGroupId;
    };

    struct VariantBroadcast
    {
    };

    struct VariantBroadcastWithAlias
    {
    };

    struct VariantViaExchange
    {
        explicit VariantViaExchange(Messaging::ExchangeContext * exchangeContext) : mExchangeContext(exchangeContext) {}
        Messaging::ExchangeContext * mExchangeContext;
    };

    template <typename Destination>
    MessageSendDestination(Destination && destination)
    {
        mDestination.Set<Destination>(std::forward<Destination>(destination));
    }

    Variant<VariantViaBinding, VariantViaAddressTable, VariantDirect, VariantMulticast, VariantMulticastWithAlias, VariantBroadcast,
            VariantBroadcastWithAlias, VariantViaExchange>
        mDestination;
};
} // namespace chip

/**
 * @brief Type for referring to the message sent callback function.
 *
 * This function is called when a message is sent.
 */
typedef void (*EmberAfMessageSentFunction)(const chip::MessageSendDestination & destination, EmberApsFrame * apsFrame,
                                           uint16_t msgLen, uint8_t * message, EmberStatus status);

/**
 * @brief The EmberAfMessageStruct is a struct wrapper that
 *   contains all the data about a low-level message to be
 *   sent (it may be ZCL or may be some other protocol).
 */
typedef struct
{
    EmberAfMessageSentFunction callback;
    EmberApsFrame * apsFrame;
    uint8_t * message;
    const chip::MessageSendDestination destination;
    uint16_t messageLength;
    bool broadcast;
} EmberAfMessageStruct;

/**
 * @brief A data struct for a link key backup.
 *
 * Each entry notes the EUI64 of the device it is paired to and the key data.
 *   This key may be hashed and not the actual link key currently in use.
 */

typedef struct
{
    EmberEUI64 deviceId;
    EmberKeyData key;
} EmberAfLinkKeyBackupData;

/**
 * @brief A data struct for all the trust center backup data.
 *
 * The 'keyList' pointer must point to an array and 'maxKeyListLength'
 * must be populated with the maximum number of entries the array can hold.
 *
 * Functions that modify this data structure will populate 'keyListLength'
 * indicating how many keys were actually written into 'keyList'.
 */

typedef struct
{
    EmberEUI64 extendedPanId;
    uint8_t keyListLength;
    uint8_t maxKeyListLength;
    EmberAfLinkKeyBackupData * keyList;
} EmberAfTrustCenterBackupData;

/**
 * @brief The length of the hardware tag in the Ember Bootloader Query
 *   Response.
 */
#define EMBER_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH 16

/**
 * @brief A data struct for the information retrieved during a response
 *   to an Ember Bootloader over-the-air query.
 */
typedef struct
{
    uint8_t hardwareTag[EMBER_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH];
    uint8_t eui64[EUI64_SIZE];
    uint16_t mfgId;
    uint16_t bootloaderVersion;
    uint8_t capabilities;
    uint8_t platform;
    uint8_t micro;
    uint8_t phy;
    bool bootloaderActive;
} EmberAfStandaloneBootloaderQueryResponseData;

/**
 * @brief A data struct used to keep track of incoming and outgoing
 *   commands for command discovery
 */
typedef struct
{
    uint16_t clusterId;
    chip::CommandId commandId;
    uint8_t mask;
} EmberAfCommandMetadata;

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
} EmberAfTimeStruct;

/**
 * @brief A data structure used to describe the ZCL Date data type
 */

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t dayOfMonth;
    uint8_t dayOfWeek;
} EmberAfDate;

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

#define EMBER_AF_INVALID_CLUSTER_ID 0xFFFF

#define EMBER_AF_INVALID_ENDPOINT 0xFF

#define EMBER_AF_INVALID_PAN_ID 0xFFFF

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
