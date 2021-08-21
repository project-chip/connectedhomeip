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
 * @brief This is a stub file for types that are external to the ZCL
 *        application framework. These can be redefined/modified as needed in
 *        the CHIP project
 ******************************************************************************/

#pragma once

#include <string.h> // For mem* functions.

#include <app/chip-zcl-zpro-codec.h> // For EmberApsFrame

#include <app/util/basic-types.h>

#include <transport/raw/MessageHeader.h>
static_assert(sizeof(chip::NodeId) == sizeof(uint64_t), "Unexpected node if size");

#include <zap-generated/endpoint_config.h>
#include <zap-generated/gen_config.h>

/**
 * @brief Defines binding types.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberBindingType
#else
typedef uint8_t EmberBindingType;
enum
#endif
{
    /** A binding that is currently not in use. */
    EMBER_UNUSED_BINDING = 0,
    /** A unicast binding whose 64-bit identifier is the destination EUI64. */
    EMBER_UNICAST_BINDING = 1,
    /** A unicast binding whose 64-bit identifier is the many-to-one
     * destination EUI64.  Route discovery should be disabled when sending
     * unicasts via many-to-one bindings. */
    EMBER_MANY_TO_ONE_BINDING = 2,
    /** A multicast binding whose 64-bit identifier is the group address. This
     * binding can be used to send messages to the group and to receive
     * messages sent to the group. */
    EMBER_MULTICAST_BINDING = 3,
};
/** @brief The type of method used for joining.
 *
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberJoinMethod
#else
typedef uint8_t EmberJoinMethod;
enum
#endif
{
    /** Devices normally use MAC association to join a network, which respects
     *  the "permit joining" flag in the MAC beacon.
     *  This value should be used by default.
     */
    EMBER_USE_MAC_ASSOCIATION = 0,

    /** For networks where the "permit joining" flag is never turned
     *  on, devices will need to use a ZigBee NWK Rejoin.  This value causes the
     *  rejoin to be sent withOUT NWK security and the Trust Center will be
     *  asked to send the NWK key to the device.  The NWK key sent to the device
     *  can be encrypted with the device's corresponding Trust Center link key.
     *  That is determined by the ::EmberJoinDecision on the Trust Center
     *  returned by the ::emberTrustCenterJoinHandler().
     */
    EMBER_USE_NWK_REJOIN = 1,

    /* For networks where the "permit joining" flag is never turned
     * on, devices will need to use a NWK Rejoin.  If those devices have been
     * preconfigured with the  NWK key (including sequence number), they can use
     * a secured rejoin.  This is only necessary for end devices since they need
     * a parent.  Routers can simply use the ::EMBER_USE_CONFIGURED_NWK_STATE
     * join method below.
     */
    EMBER_USE_NWK_REJOIN_HAVE_NWK_KEY = 2,

    /** For networks where all network and security information is known
        ahead of time, a router device may be commissioned such that it does
        not need to send any messages to begin communicating on the network.
     */
    EMBER_USE_CONFIGURED_NWK_STATE = 3,
};

/**
 * @brief Type of Ember software version
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberVersionType
#else
typedef uint8_t EmberVersionType;
enum
#endif
{
    EMBER_VERSION_TYPE_PRE_RELEASE = 0x00,

    // Alpha, should be used rarely
    EMBER_VERSION_TYPE_ALPHA_1 = 0x11,
    EMBER_VERSION_TYPE_ALPHA_2 = 0x12,
    EMBER_VERSION_TYPE_ALPHA_3 = 0x13,
    // Leave space in case we decide to add other types in the future.
    EMBER_VERSION_TYPE_BETA_1 = 0x21,
    EMBER_VERSION_TYPE_BETA_2 = 0x22,
    EMBER_VERSION_TYPE_BETA_3 = 0x23,

    // Anything other than 0xAA is considered pre-release
    // Silicon Labs may define other types in the future (e.g. beta, alpha)
    // Silicon Labs chose an arbitrary number (0xAA) to allow for expansion, but
    // to prevent ambiguity in case 0x00 or 0xFF is accidentally retrieved
    // as the version type.
    EMBER_VERSION_TYPE_GA = 0xAA,
};

/**
 * @brief Either marks an event as inactive or specifies the units for the
 * event execution time.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberEventUnits
#else
typedef uint8_t EmberEventUnits;
enum
#endif
{
    /** The event is not scheduled to run. */
    EMBER_EVENT_INACTIVE = 0,
    /** The execution time is in approximate milliseconds.  */
    EMBER_EVENT_MS_TIME,
    /** The execution time is in 'binary' quarter seconds (256 timer ticks). */
    EMBER_EVENT_QS_TIME,
    /** The execution time is in 'binary' minutes (65536 timer ticks). */
    EMBER_EVENT_MINUTE_TIME,
    /** The event is scheduled to run at the earliest opportunity. */
    EMBER_EVENT_ZERO_DELAY
};
/** @brief An identifier for a task */
typedef uint8_t EmberTaskId;

/**
 * @brief Options to use when sending a message.
 *
 * The discover-route, APS-retry, and APS-indirect options may be used together.
 * Poll response cannot be combined with any other options.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberApsOption
#else
typedef uint16_t EmberApsOption;
enum
#endif
{
    /** No options. */
    EMBER_APS_OPTION_NONE = 0x0000,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    EMBER_APS_OPTION_ENCRYPT_WITH_TRANSIENT_KEY = 0x0001,
    EMBER_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER  = 0x0002,
#endif

    /** This signs the application layer message body (APS Frame not included)
        and appends the ECDSA signature to the end of the message, which is needed by
        Smart Energy applications and requires the CBKE and ECC libraries.
        The ::emberDsaSignHandler() function is called after DSA signing
        is complete but before the message has been sent by the APS layer.
        Note that when passing a buffer to the stack for DSA signing, the final
        byte in the buffer has a special significance as an indicator of how many
        leading bytes should be ignored for signature purposes. See the API
        documentation of emberDsaSign() or the dsaSign EZSP command for more
        details about this requirement.
     */
    EMBER_APS_OPTION_DSA_SIGN = 0x0010,
    /** Resend the message using the APS retry mechanism.
        This option and the enable route discovery option must be enabled for
        an existing route to be repaired automatically. */
    EMBER_APS_OPTION_RETRY = 0x0040,
    /** Send the message with the NWK 'enable route discovery' flag, which
        causes a route discovery to be initiated if no route to the destination
        is known.  Note that in the mesh stack, this option and the APS retry
        option must be enabled an existing route to be repaired
        automatically. */
    EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY = 0x0100,
    /** Send the message with the NWK 'force route discovery' flag, which causes
        a route discovery to be initiated even if one is known. */
    EMBER_APS_OPTION_FORCE_ROUTE_DISCOVERY = 0x0200,
    /** Include the source EUI64 in the network frame. */
    EMBER_APS_OPTION_SOURCE_EUI64 = 0x0400,
    /** Include the destination EUI64 in the network frame. */
    EMBER_APS_OPTION_DESTINATION_EUI64 = 0x0800,
    /** Send a ZDO request to discover the node ID of the destination if it is
        not already known. */
    EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY = 0x1000,
    /** This message is being sent in response to a call to
        ::emberPollHandler().  It causes the message to be sent
        immediately instead of being queued up until the next poll from the
        (end device) destination. */
    EMBER_APS_OPTION_POLL_RESPONSE = 0x2000,
    /** This incoming message is a valid ZDO request and the application
     *   is responsible for sending a ZDO response. This flag is used only
     *   within emberIncomingMessageHandler() when
     *   EMBER_APPLICATION_RECEIVES_UNSUPPORTED_ZDO_REQUESTS is defined. */
    EMBER_APS_OPTION_ZDO_RESPONSE_REQUIRED = 0x4000,
    /** This message is part of a fragmented message.  This option may only
        be set for unicasts.  The groupId field gives the index of this
        fragment in the low-order byte.  If the low-order byte is zero this
        is the first fragment and the high-order byte contains the number
        of fragments in the message. */
    EMBER_APS_OPTION_FRAGMENT = 0x8000
};

/**
 * @brief Size of EUI64 (an IEEE address) in bytes (8).
 */
#define EUI64_SIZE 8

/**
 * @brief Size of an encryption key in bytes (16).
 */
#define EMBER_ENCRYPTION_KEY_SIZE 16

/**
 * @brief Size of an extended PAN identifier in bytes (8).
 */
#define EXTENDED_PAN_ID_SIZE 8

/**
 * @brief EUI 64-bit ID (an IEEE address).
 */
typedef uint8_t EmberEUI64[EUI64_SIZE];

/**
 * @brief 16-bit ZigBee network address.
 */
typedef uint16_t EmberNodeId;

/**
 * @brief 802.15.4 PAN ID.
 */
typedef uint16_t EmberPanId;

/** @brief This enumeration determines whether or not a Trust Center
 *  answers trust center link key requests.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberTcLinkKeyRequestPolicy
#else
typedef uint8_t EmberTcLinkKeyRequestPolicy;
enum
#endif
{
    EMBER_DENY_TC_LINK_KEY_REQUESTS                      = 0x00,
    EMBER_ALLOW_TC_LINK_KEY_REQUEST_AND_SEND_CURRENT_KEY = 0x01,
    // When using the following mode a unique random link key is created.
    // The key which is generated due to this mode is added to the link
    // key table. Therefore make sure that the link key table size is not
    // zero as this can result in the newly generated key not being saved
    // and communication breaking between the trust center and the nodes.
    EMBER_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY = 0x02
};

/** @brief This enumeration determines whether or not a Trust Center
 *  answers app link key requests.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAppLinkKeyRequestPolicy
#else
typedef uint8_t EmberAppLinkKeyRequestPolicy;
enum
#endif
{
    EMBER_DENY_APP_LINK_KEY_REQUESTS = 0x00,
    EMBER_ALLOW_APP_LINK_KEY_REQUEST = 0x01
};

/** @brief This data structure contains the key data that is passed
 *   into various other functions. */
typedef struct
{
    /** This is the key byte data. */
    uint8_t contents[EMBER_ENCRYPTION_KEY_SIZE];
} EmberKeyData;
/**
 * @brief Defines the possible types of nodes and the roles that a
 * node might play in a network.
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNodeType
#else
typedef uint8_t EmberNodeType;
enum
#endif
{
    /** The device is not joined. */
    EMBER_UNKNOWN_DEVICE = 0,
    /** Will relay messages and can act as a parent to other nodes. */
    EMBER_COORDINATOR = 1,
    /** Will relay messages and can act as a parent to other nodes. */
    EMBER_ROUTER = 2,
    /** Communicates only with its parent and will not relay messages. */
    EMBER_END_DEVICE = 3,
    /** An end device whose radio can be turned off to save power.
     *  The application must call ::emberPollForData() to receive messages.
     */
    EMBER_SLEEPY_END_DEVICE = 4,
};

/**
 * @brief Defines the possible incoming message types.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberIncomingMessageType
#else
typedef uint8_t EmberIncomingMessageType;
enum
#endif
{
    /** Unicast. */
    EMBER_INCOMING_UNICAST,
    /** Unicast reply. */
    EMBER_INCOMING_UNICAST_REPLY,
    /** Multicast. */
    EMBER_INCOMING_MULTICAST,
    /** Multicast sent by the local device. */
    EMBER_INCOMING_MULTICAST_LOOPBACK,
    /** Broadcast. */
    EMBER_INCOMING_BROADCAST,
    /** Broadcast sent by the local device. */
    EMBER_INCOMING_BROADCAST_LOOPBACK
};

/** @brief Endpoint information (a ZigBee Simple Descriptor).
 *
 * This is a ZigBee Simple Descriptor and contains information
 * about an endpoint.  This information is shared with other nodes in the
 * network by the ZDO.
 */

typedef struct
{
    /** The endpoint's device ID within the application profile. */
    uint16_t deviceId;
    /** The endpoint's device version. */
    uint8_t deviceVersion;
    /** The number of input clusters. */
    uint8_t inputClusterCount;
    /** The number of output clusters. */
    uint8_t outputClusterCount;
} EmberEndpointDescription;

/** @brief A type of packet received by the stack
 *
 * This enum provides a way to indicate which protocol layer in the Ember
 * stack an incoming packet is meant for, or from which protocol layer
 * an outgoing command is being sent from.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZigbeePacketType
#else
typedef uint8_t EmberZigbeePacketType;
enum
#endif
{
    /** Describes an 802.15.4 raw MAC message, unprocessed by the stack. */
    EMBER_ZIGBEE_PACKET_TYPE_RAW_MAC,
    /** Describes an 802.15.4 MAC layer command. */
    EMBER_ZIGBEE_PACKET_TYPE_MAC_COMMAND,
    /** Describes a ZigBee Network layer data message. */
    EMBER_ZIGBEE_PACKET_TYPE_NWK_DATA,
    /** Describes a ZigBee Network layer command. */
    EMBER_ZIGBEE_PACKET_TYPE_NWK_COMMAND,
    /** Describes a ZigBee Application Support layer data message. */
    EMBER_ZIGBEE_PACKET_TYPE_APS_DATA,
    /** Describes a ZigBee Application Support layer command. */
    EMBER_ZIGBEE_PACKET_TYPE_APS_COMMAND,
    /** Describes a ZigBee Device Object command. */
    EMBER_ZIGBEE_PACKET_TYPE_ZDO,
    /** Describes a ZigBee Cluster Library command. */
    EMBER_ZIGBEE_PACKET_TYPE_ZCL,

    /** Distinguishing between raw MAC and beacons for simplicity */
    EMBER_ZIGBEE_PACKET_TYPE_BEACON,
};

/**
 * @brief Defines a ZigBee network and the associated parameters.
 */
typedef struct
{
    uint16_t panId;
    uint8_t channel;
    bool allowingJoin;
    uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE];
    uint8_t stackProfile;
    uint8_t nwkUpdateId;
} EmberZigbeeNetwork;

/** @brief indication of the action taken on a packet */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberPacketAction
#else
typedef uint8_t EmberPacketAction;
enum
#endif
{
    EMBER_DROP_PACKET   = 0,
    EMBER_ACCEPT_PACKET = 1,
    EMBER_MANGLE_PACKET,
};

/**
 * @name ZigBee Device Object (ZDO) Definitions
 */
//@{

/** @name ZDO response status.
 *
 * Most responses to ZDO commands contain a status byte.
 * The meaning of this byte is defined by the ZigBee Device Profile.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZdoStatus
#else
typedef uint8_t EmberZdoStatus;
enum
#endif
{
    // These values are taken from Table 48 of ZDP Errata 043238r003 and Table 2
    // of NWK 02130r10.
    EMBER_ZDP_SUCCESS = 0x00,
    // 0x01 to 0x7F are reserved
    EMBER_ZDP_INVALID_REQUEST_TYPE = 0x80,
    EMBER_ZDP_DEVICE_NOT_FOUND     = 0x81,
    EMBER_ZDP_INVALID_ENDPOINT     = 0x82,
    EMBER_ZDP_NOT_ACTIVE           = 0x83,
    EMBER_ZDP_NOT_SUPPORTED        = 0x84,
    EMBER_ZDP_TIMEOUT              = 0x85,
    EMBER_ZDP_NO_MATCH             = 0x86,
    // 0x87 is reserved                 = 0x87,
    EMBER_ZDP_NO_ENTRY                  = 0x88,
    EMBER_ZDP_NO_DESCRIPTOR             = 0x89,
    EMBER_ZDP_INSUFFICIENT_SPACE        = 0x8a,
    EMBER_ZDP_NOT_PERMITTED             = 0x8b,
    EMBER_ZDP_TABLE_FULL                = 0x8c,
    EMBER_ZDP_NOT_AUTHORIZED            = 0x8d,
    EMBER_ZDP_DEVICE_BINDING_TABLE_FULL = 0x8e,
    EMBER_ZDP_INVALID_INDEX             = 0x8f,

    EMBER_NWK_ALREADY_PRESENT = 0xC5,
    EMBER_NWK_TABLE_FULL      = 0xC7,
    EMBER_NWK_UNKNOWN_DEVICE  = 0xC8
};

/** @brief Defines an entry in the binding table.
 *
 * A binding entry specifies a local endpoint, a remote endpoint, a
 * cluster ID and either the destination EUI64 (for unicast bindings) or the
 * 64-bit group address (for multicast bindings).
 */
struct EmberBindingTableEntry
{
    /** The type of binding. */
    EmberBindingType type;
    /** The endpoint on the local node. */
    chip::EndpointId local;
    /** A cluster ID that matches one from the local endpoint's simple descriptor.
     * This cluster ID is set by the provisioning application to indicate which
     * part an endpoint's functionality is bound to this particular remote node
     * and is used to distinguish between unicast and multicast bindings. Note
     * that a binding can be used to to send messages with any cluster ID, not
     * just that listed in the binding.
     */
    chip::ClusterId clusterId;
    /** The endpoint on the remote node (specified by \c identifier). */
    chip::EndpointId remote;
    /** A 64-bit destination identifier.  This is either:
     * - The destination chip::NodeId, for unicasts.
     * - A multicast ChipGroupId, for multicasts.
     * Which one is being used depends on the type of this binding.
     */
    union
    {
        chip::NodeId nodeId;
        chip::GroupId groupId;
    };
    /** The index of the network the binding belongs to. */
    uint8_t networkIndex;

    bool operator==(EmberBindingTableEntry const & other) const
    {
        if (type != other.type)
        {
            return false;
        }

        if (type == EMBER_MULTICAST_BINDING && groupId != other.groupId)
        {
            return false;
        }

        if (type == EMBER_UNICAST_BINDING && nodeId != other.nodeId)
        {
            return false;
        }

        return local == other.local && clusterId == other.clusterId && remote == other.remote && networkIndex == other.networkIndex;
    }
};

/**
 * @brief The decision made by the Trust Center when a node attempts to join.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberJoinDecision
#else
typedef uint8_t EmberJoinDecision;
enum
#endif
{
    /** Allow the node to join. The node has the key. */
    EMBER_USE_PRECONFIGURED_KEY = 0,
    /** Allow the node to join. Send the key to the node. */
    EMBER_SEND_KEY_IN_THE_CLEAR,
    /** Deny join. */
    EMBER_DENY_JOIN,
    /** Take no action. */
    EMBER_NO_ACTION
};

/** @brief This denotes the status of an attempt to establish
 *  a key with another device.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberKeyStatus
#else
typedef uint8_t EmberKeyStatus;
enum
#endif
{
    EMBER_KEY_STATUS_NONE                   = 0x00,
    EMBER_APP_LINK_KEY_ESTABLISHED          = 0x01,
    EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED = 0x03,

    EMBER_KEY_ESTABLISHMENT_TIMEOUT = 0x04,
    EMBER_KEY_TABLE_FULL            = 0x05,

    // These are success status values applying only to the
    // Trust Center answering key requests.
    EMBER_TC_RESPONDED_TO_KEY_REQUEST  = 0x06,
    EMBER_TC_APP_KEY_SENT_TO_REQUESTER = 0x07,

    // These are failure status values applying only to the
    // Trust Center answering key requests.
    EMBER_TC_RESPONSE_TO_KEY_REQUEST_FAILED             = 0x08,
    EMBER_TC_REQUEST_KEY_TYPE_NOT_SUPPORTED             = 0x09,
    EMBER_TC_NO_LINK_KEY_FOR_REQUESTER                  = 0x0A,
    EMBER_TC_REQUESTER_EUI64_UNKNOWN                    = 0x0B,
    EMBER_TC_RECEIVED_FIRST_APP_KEY_REQUEST             = 0x0C,
    EMBER_TC_TIMEOUT_WAITING_FOR_SECOND_APP_KEY_REQUEST = 0x0D,
    EMBER_TC_NON_MATCHING_APP_KEY_REQUEST_RECEIVED      = 0x0E,
    EMBER_TC_FAILED_TO_SEND_APP_KEYS                    = 0x0F,
    EMBER_TC_FAILED_TO_STORE_APP_KEY_REQUEST            = 0x10,
    EMBER_TC_REJECTED_APP_KEY_REQUEST                   = 0x11,
    EMBER_TC_FAILED_TO_GENERATE_NEW_KEY                 = 0x12,
    EMBER_TC_FAILED_TO_SEND_TC_KEY                      = 0x13,

    // These are generic status values for a key requester.
    EMBER_TRUST_CENTER_IS_PRE_R21 = 0x1E,

    // These are status values applying only to the Trust Center
    // verifying link keys.
    EMBER_TC_REQUESTER_VERIFY_KEY_TIMEOUT = 0x32,
    EMBER_TC_REQUESTER_VERIFY_KEY_FAILURE = 0x33,
    EMBER_TC_REQUESTER_VERIFY_KEY_SUCCESS = 0x34,

    // These are status values applying only to the key requester
    // verifying link keys.
    EMBER_VERIFY_LINK_KEY_FAILURE = 0x64,
    EMBER_VERIFY_LINK_KEY_SUCCESS = 0x65,
};

typedef uint8_t EmberAfPluginNetworkSteeringJoiningState;

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum SleepModes
#else
typedef uint8_t SleepModes;
enum
#endif
{
    SLEEPMODE_IDLE = 1,
};

/**
 * @brief Defines the possible join states for a node.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNetworkStatus
#else
typedef uint8_t EmberNetworkStatus;
enum
#endif
{
    /** The node is not associated with a network in any way. */
    EMBER_NO_NETWORK,
    /** The node is currently attempting to join a network. */
    EMBER_JOINING_NETWORK,
    /** The node is joined to a network. */
    EMBER_JOINED_NETWORK,
    /** The node is an end device joined to a network but its parent
        is not responding. */
    EMBER_JOINED_NETWORK_NO_PARENT,
    /** The node is in the process of leaving its current network. */
    EMBER_LEAVING_NETWORK
};

/** @brief Holds network parameters.
 *
 * For information about power settings and radio channels,
 * see the technical specification for the
 * RF communication module in your Developer Kit.
 */
typedef struct
{
    /** The network's extended PAN identifier.*/
    uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE];
    /** The network's PAN identifier.*/
    uint16_t panId;
    /** A power setting, in dBm.*/
    int8_t radioTxPower;
    /** A radio channel. Be sure to specify a channel supported by the radio. */
    uint8_t radioChannel;
    /** Join method: The protocol messages used to establish an initial parent.  It is
     *  ignored when forming a ZigBee network, or when querying the stack for its
        network parameters.
     */
    EmberJoinMethod joinMethod;

    /** NWK Manager ID.  The ID of the network manager in the current network.
        This may only be set at joining when using EMBER_USE_CONFIGURED_NWK_STATE
        as the join method.
     */
    EmberNodeId nwkManagerId;
    /** An NWK Update ID.  The value of the ZigBee nwkUpdateId known by the stack.
        It is used to determine the newest instance of the network after a PAN
        ID or channel change.  This may only be set at joining when using
        EMBER_USE_CONFIGURED_NWK_STATE as the join method.
     */
    uint8_t nwkUpdateId;
    /** The NWK channel mask.  The list of preferred channels that the NWK manager
        has told this device to use when searching for the network.
        This may only be set at joining when using EMBER_USE_CONFIGURED_NWK_STATE
        as the join method.
     */
    uint32_t channels;
} EmberNetworkParameters;

/**
 * @brief Incoming and outgoing messages are stored in buffers.
 * These buffers are allocated and freed as needed.
 *
 * Buffers are 32 bytes in length and can be linked together to hold
 * longer messages.
 *
 * See packet-buffer.h for APIs related to stack and linked buffers.
 */
typedef uint8_t EmberMessageBuffer;

/**
 * @brief A version structure containing all version information.
 */
typedef struct
{
    uint16_t build;
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t special;
    EmberVersionType type;
} EmberVersion;

/** @brief This structure contains information about child nodes.
 *
 */
typedef struct
{
    EmberEUI64 eui64;
    EmberNodeType type;
    EmberNodeId id;
    uint8_t phy;
    uint8_t power;
    uint8_t timeout;
} EmberChildData;

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node.  This value is used when getting the remote node ID
 * from the address or binding tables.  It indicates that the address
 * or binding table entry is currently in use but the node ID
 * corresponding to the EUI64 in the table is currently unknown.
 */
#define EMBER_UNKNOWN_NODE_ID 0xFFFD

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node. Used to indicate the absence of a node ID.
 */
#define EMBER_NULL_NODE_ID 0xFFFF

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberStatus
#else
typedef uint8_t EmberStatus;
enum
#endif // DOXYGEN_SHOULD_SKIP_THIS
{
    /**
     * @name Generic Messages
     * These messages are system wide.
     */
    //@{

    /**
     * @brief The generic "no error" message.
     */
    EMBER_SUCCESS = 0x00,

    /**
     * @brief The generic "fatal error" message.
     */
    EMBER_ERR_FATAL = 0x01,

    /**
     * @brief An invalid value was passed as an argument to a function.
     */
    EMBER_BAD_ARGUMENT = 0x02,

    /**
     * @brief The requested information was not found.
     */
    EMBER_NOT_FOUND = 0x03,

    /**
     * @brief The manufacturing and stack token format in non-volatile memory
     * is different than what the stack expects (returned at initialization).
     */
    EMBER_EEPROM_MFG_STACK_VERSION_MISMATCH = 0x04,

    /**
     * @brief The static memory definitions in ember-static-memory.h
     * are incompatible with this stack version.
     */
    EMBER_INCOMPATIBLE_STATIC_MEMORY_DEFINITIONS = 0x05,

    /**
     * @brief The manufacturing token format in non-volatile memory is
     * different than what the stack expects (returned at initialization).
     */
    EMBER_EEPROM_MFG_VERSION_MISMATCH = 0x06,

    /**
     * @brief The stack token format in non-volatile memory is different
     * than what the stack expects (returned at initialization).
     */
    EMBER_EEPROM_STACK_VERSION_MISMATCH = 0x07,

    //@} // END Generic Messages

    /**
     * @name Packet Buffer Module Errors
     */
    //@{

    /**
     * @brief There are no more buffers.
     */
    EMBER_NO_BUFFERS = 0x18,

    //@} / END Packet Buffer Module Errors

    /**
     * @name Serial Manager Errors
     */
    //@{

    /**
     * @brief Specifies an invalid baud rate.
     */
    EMBER_SERIAL_INVALID_BAUD_RATE = 0x20,

    /**
     * @brief Specifies an invalid serial port.
     */
    EMBER_SERIAL_INVALID_PORT = 0x21,

    /**
     * @brief Tried to send too much data.
     */
    EMBER_SERIAL_TX_OVERFLOW = 0x22,

    /**
     * @brief There wasn't enough space to store a received character
     * and the character was dropped.
     */
    EMBER_SERIAL_RX_OVERFLOW = 0x23,

    /**
     * @brief Detected a UART framing error.
     */
    EMBER_SERIAL_RX_FRAME_ERROR = 0x24,

    /**
     * @brief Detected a UART parity error.
     */
    EMBER_SERIAL_RX_PARITY_ERROR = 0x25,

    /**
     * @brief There is no received data to process.
     */
    EMBER_SERIAL_RX_EMPTY = 0x26,

    /**
     * @brief The receive interrupt was not handled in time and a
     * character was dropped.
     */
    EMBER_SERIAL_RX_OVERRUN_ERROR = 0x27,

    //@}

    /**
     * @name MAC Errors
     */
    //@{

    /**
     * @brief The MAC transmit queue is full.
     */
    EMBER_MAC_TRANSMIT_QUEUE_FULL = 0x39,
    // Internal

    /**
     * @brief MAC header FCF error on receive.
     */
    EMBER_MAC_UNKNOWN_HEADER_TYPE = 0x3A,

    /**
     * @brief MAC ACK header received.
     */
    EMBER_MAC_ACK_HEADER_TYPE = 0x3B,

    /**
     * @brief The MAC can't complete this task because it is scanning.
     */
    EMBER_MAC_SCANNING = 0x3D,

    /**
     * @brief No pending data exists for a data poll.
     */
    EMBER_MAC_NO_DATA = 0x31,

    /**
     * @brief Attempts to scan when joined to a network.
     */
    EMBER_MAC_JOINED_NETWORK = 0x32,

    /**
     * @brief Scan duration must be 0 to 14 inclusive. Tried to
     * scan with an incorrect duration value.
     */
    EMBER_MAC_BAD_SCAN_DURATION = 0x33,

    /**
     * @brief emberStartScan was called with an incorrect scan type.
     */
    EMBER_MAC_INCORRECT_SCAN_TYPE = 0x34,

    /**
     * @brief emberStartScan was called with an invalid channel mask.
     */
    EMBER_MAC_INVALID_CHANNEL_MASK = 0x35,

    /**
     * @brief Failed to scan the current channel because
     * the relevant MAC command could not be transmitted.
     */
    EMBER_MAC_COMMAND_TRANSMIT_FAILURE = 0x36,

    /**
     * @brief An ACK was expected following the transmission but
     * the MAC level ACK was never received.
     */
    EMBER_MAC_NO_ACK_RECEIVED = 0x40,

    /**
     * @brief MAC failed to transmit a message because it could not successfully
     * perform a radio network switch.
     */
    EMBER_MAC_RADIO_NETWORK_SWITCH_FAILED = 0x41,

    /**
     * @brief An indirect data message timed out before a poll requested it.
     */
    EMBER_MAC_INDIRECT_TIMEOUT = 0x42,

    //@}

    /**
     * @name  Simulated EEPROM Errors
     */
    //@{

    /**
     * @brief The Simulated EEPROM is telling the application that
     * at least one flash page to be erased. The GREEN status means the
     * current page has not filled above the ::ERASE_CRITICAL_THRESHOLD.
     *
     * The application should call the function ::halSimEepromErasePage()
     * when it can to erase a page.
     */
    EMBER_SIM_EEPROM_ERASE_PAGE_GREEN = 0x43,

    /**
     * @brief The Simulated EEPROM is telling the application that
     * at least one flash page must be erased.  The RED status means the
     * current page has filled above the ::ERASE_CRITICAL_THRESHOLD.
     *
     * Due to the shrinking availability of write space, data could
     * be lost. The application must call the function ::halSimEepromErasePage()
     * as soon as possible to erase a page.
     */
    EMBER_SIM_EEPROM_ERASE_PAGE_RED = 0x44,

    /**
     * @brief The Simulated EEPROM has run out of room to write new data
     * and the data trying to be set has been lost. This error code is the
     * result of ignoring the ::SIM_EEPROM_ERASE_PAGE_RED error code.
     *
     * The application must call the function ::halSimEepromErasePage() to make room for
     * any further calls to set a token.
     */
    EMBER_SIM_EEPROM_FULL = 0x45,

    //  Errors 46 and 47 are now defined below in the
    //    flash error block  (was attempting to prevent renumbering).

    /**
     * @brief Attempt 1 to initialize the Simulated EEPROM has failed.
     *
     * This failure means the information already stored in the Flash (or a lack
     * thereof), is fatally incompatible with the token information compiled
     * into the code image being run.
     */
    EMBER_SIM_EEPROM_INIT_1_FAILED = 0x48,

    /**
     * @brief Attempt 2 to initialize the Simulated EEPROM has failed.
     *
     * This failure means Attempt 1 failed, and the token system failed to
     * properly reload default tokens and reset the Simulated EEPROM.
     */
    EMBER_SIM_EEPROM_INIT_2_FAILED = 0x49,

    /**
     * @brief Attempt 3 to initialize the Simulated EEPROM has failed.
     *
     * This failure means one or both of the tokens ::TOKEN_MFG_NVDATA_VERSION or
     * ::TOKEN_STACK_NVDATA_VERSION were incorrect and the token system failed to
     * properly reload default tokens and reset the Simulated EEPROM.
     */
    EMBER_SIM_EEPROM_INIT_3_FAILED = 0x4A,

    /**
     * @brief The Simulated EEPROM is repairing itself.
     *
     * While there's nothing for an app to do when the SimEE is going to
     * repair itself (SimEE has to be fully functional for the rest of the
     * system to work), alert the application to the fact that repair
     * is occurring.  There are debugging scenarios where an app might want
     * to know that repair is happening, such as monitoring frequency.
     * @note  Common situations will trigger an expected repair, such as
     *        using an erased chip or changing token definitions.
     */
    EMBER_SIM_EEPROM_REPAIRING = 0x4D,

    //@}

    /**
     * @name  Flash Errors
     */
    //@{

    /**
     * @brief A fatal error has occurred while trying to write data to the
     * Flash. The target memory attempting to be programmed is already programmed.
     * The flash write routines were asked to flip a bit from a 0 to 1, which is
     * physically impossible and the write was therefore inhibited. The data in
     * the Flash cannot be trusted after this error.
     */
    EMBER_ERR_FLASH_WRITE_INHIBITED = 0x46,

    /**
     * @brief A fatal error has occurred while trying to write data to the
     * Flash and the write verification has failed.  Data in the Flash
     * cannot be trusted after this error and it is possible this error is the
     * result of exceeding the life cycles of the Flash.
     */
    EMBER_ERR_FLASH_VERIFY_FAILED = 0x47,

    /**
     * @description A fatal error has occurred while trying to write data to the
     * Flash possibly due to write protection or an invalid address. Data in
     * the Flash cannot be trusted after this error and it is possible this error
     * is the result of exceeding the life cycles of the Flash.
     */
    EMBER_ERR_FLASH_PROG_FAIL = 0x4B,

    /**
     * @description A fatal error has occurred while trying to erase the Flash possibly
     * due to write protection. Data in the Flash cannot be trusted after
     * this error and it is possible this error is the result of exceeding the
     * life cycles of the Flash.
     */
    EMBER_ERR_FLASH_ERASE_FAIL = 0x4C,

    //@}

    /**
     * @name  Bootloader Errors
     */
    //@{

    /**
     * @brief The bootloader received an invalid message (failed attempt
     * to go into bootloader).
     */
    EMBER_ERR_BOOTLOADER_TRAP_TABLE_BAD = 0x58,

    /**
     * @brief The bootloader received an invalid message (failed attempt to go
     * into the bootloader).
     */
    EMBER_ERR_BOOTLOADER_TRAP_UNKNOWN = 0x59,

    /**
     * @brief The bootloader cannot complete the bootload operation because
     * either an image was not found or the image exceeded memory bounds.
     */
    EMBER_ERR_BOOTLOADER_NO_IMAGE = 0x05A,

    //@}

    /**
     * @name  Transport Errors
     */
    //@{

    /**
     * @brief The APS layer attempted to send or deliver a message
     * and failed.
     */
    EMBER_DELIVERY_FAILED = 0x66,

    /**
     * @brief This binding index is out of range for the current binding table.
     */
    EMBER_BINDING_INDEX_OUT_OF_RANGE = 0x69,

    /**
     * @brief This address table index is out of range for the current
     * address table.
     */
    EMBER_ADDRESS_TABLE_INDEX_OUT_OF_RANGE = 0x6A,

    /**
     * @brief An invalid binding table index was given to a function.
     */
    EMBER_INVALID_BINDING_INDEX = 0x6C,

    /**
     * @brief The API call is not allowed given the current state of the
     * stack.
     */
    EMBER_INVALID_CALL = 0x70,

    /**
     * @brief The link cost to a node is not known.
     */
    EMBER_COST_NOT_KNOWN = 0x71,

    /**
     * @brief The maximum number of in-flight messages  = i.e.,
     * ::EMBER_APS_UNICAST_MESSAGE_COUNT, has been reached.
     */
    EMBER_MAX_MESSAGE_LIMIT_REACHED = 0x72,

    /**
     * @brief The message to be transmitted is too big to fit into a
     * single over-the-air packet.
     */
    EMBER_MESSAGE_TOO_LONG = 0x74,

    /**
     * @brief The application is trying to delete or overwrite a binding
     * that is in use.
     */
    EMBER_BINDING_IS_ACTIVE = 0x75,

    /**
     * @brief The application is trying to overwrite an address table entry
     * that is in use.
     */
    EMBER_ADDRESS_TABLE_ENTRY_IS_ACTIVE = 0x76,

    /**
     * @brief An attempt was made to transmit during the suspend period.
     */
    EMBER_TRANSMISSION_SUSPENDED = 0x77,

    //@}
    //

    /**
     * @name  Green Power status codes
     */
    //@{

    /**
     * @brief Security match.
     */
    EMBER_MATCH = 0x78,
    /**
     * @brief Drop frame.
     */
    EMBER_DROP_FRAME       = 0x79,
    EMBER_PASS_UNPROCESSED = 0x7A,
    EMBER_TX_THEN_DROP     = 0x7B,
    EMBER_NO_SECURITY      = 0x7C,
    EMBER_COUNTER_FAILURE  = 0x7D,
    EMBER_AUTH_FAILURE     = 0x7E,
    EMBER_UNPROCESSED      = 0x7F,

    //@}
    //

    /**
     * @name  HAL Module Errors
     */
    //@{

    /**
     * @brief The conversion is complete.
     */
    EMBER_ADC_CONVERSION_DONE = 0x80,

    /**
     * @brief The conversion cannot be done because a request is being
     * processed.
     */
    EMBER_ADC_CONVERSION_BUSY = 0x81,

    /**
     * @brief The conversion is deferred until the current request has been
     * processed.
     */
    EMBER_ADC_CONVERSION_DEFERRED = 0x82,

    /**
     * @brief No results are pending.
     */
    EMBER_ADC_NO_CONVERSION_PENDING = 0x84,

    /**
     * @brief Sleeping (for a duration) has been abnormally interrupted
     * and exited prematurely.
     */
    EMBER_SLEEP_INTERRUPTED = 0x85,

    //@}

    /**
     * @name  PHY Errors
     */
    //@{

    /**
     * @brief The transmit attempt failed because the radio scheduler could not find
     * a slot to transmit this packet in or a higher priority event interrupted it.
     */
    EMBER_PHY_TX_SCHED_FAIL = 0x87,

    /**
     * @brief The transmit hardware buffer underflowed.
     */
    EMBER_PHY_TX_UNDERFLOW = 0x88,

    /**
     * @brief The transmit hardware did not finish transmitting a packet.
     */
    EMBER_PHY_TX_INCOMPLETE = 0x89,

    /**
     * @brief An unsupported channel setting was specified.
     */
    EMBER_PHY_INVALID_CHANNEL = 0x8A,

    /**
     * @brief An unsupported power setting was specified.
     */
    EMBER_PHY_INVALID_POWER = 0x8B,

    /**
     * @brief The requested operation cannot be completed because the radio
     * is currently busy, either transmitting a packet or performing calibration.
     */
    EMBER_PHY_TX_BUSY = 0x8C,

    /**
     * @brief The transmit attempt failed because all CCA attempts indicated that
     * the channel was busy.
     */
    EMBER_PHY_TX_CCA_FAIL = 0x8D,

    /**
     * @brief The transmit attempt was blocked from going over the air. Typically
     * this is due to the Radio Hold Off (RHO) or Coexistence plugins as they can
     * prevent transmits based on external signals.
     */
    EMBER_PHY_TX_BLOCKED = 0x8E,

    /**
     * @brief The expected ACK was received after the last transmission.
     */
    EMBER_PHY_ACK_RECEIVED = 0x8F,

    //@}

    /**
     * @name  Return Codes Passed to emberStackStatusHandler()
     * See also ::emberStackStatusHandler = ,.
     */
    //@{

    /**
     * @brief The stack software has completed initialization and is ready
     * to send and receive packets over the air.
     */
    EMBER_NETWORK_UP = 0x90,

    /**
     * @brief The network is not operating.
     */
    EMBER_NETWORK_DOWN = 0x91,

    /**
     * @brief An attempt to join a network failed.
     */
    EMBER_JOIN_FAILED = 0x94,

    /**
     * @brief After moving, a mobile node's attempt to re-establish contact
     * with the network failed.
     */
    EMBER_MOVE_FAILED = 0x96,

    /**
     * @brief An attempt to join as a router failed due to a Zigbee
     * versus Zigbee Pro incompatibility.  Zigbee devices joining Zigbee Pro networks
     * (or vice versa) must join as End Devices, not Routers.
     */
    EMBER_CANNOT_JOIN_AS_ROUTER = 0x98,

    /** @brief The local node ID has changed. The application can get the new
     *  node ID by calling ::emberGetNodeId().
     */
    EMBER_NODE_ID_CHANGED = 0x99,

    /** @brief The local PAN ID has changed. The application can get the new PAN
     *  ID by calling ::emberGetPanId().
     */
    EMBER_PAN_ID_CHANGED = 0x9A,

    /** @brief The channel has changed.
     */
    EMBER_CHANNEL_CHANGED = 0x9B,

    /** @brief An attempt to join or rejoin the network failed because
     *  no router beacons could be heard by the joining node.
     */
    EMBER_NO_BEACONS = 0xAB,

    /** @brief An attempt was made to join a Secured Network using a pre-configured
     *  key, but the Trust Center sent back a Network Key in-the-clear when
     *  an encrypted Network Key was required. (::EMBER_REQUIRE_ENCRYPTED_KEY).
     */
    EMBER_RECEIVED_KEY_IN_THE_CLEAR = 0xAC,

    /** @brief An attempt was made to join a Secured Network, but the device did
     *  not receive a Network Key.
     */
    EMBER_NO_NETWORK_KEY_RECEIVED = 0xAD,

    /** @brief After a device joined a Secured Network, a Link Key was requested
     *  (::EMBER_GET_LINK_KEY_WHEN_JOINING) but no response was ever received.
     */
    EMBER_NO_LINK_KEY_RECEIVED = 0xAE,

    /** @brief An attempt was made to join a Secured Network without a
     *  pre-configured key, but the Trust Center sent encrypted data using a
     *  pre-configured key.
     */
    EMBER_PRECONFIGURED_KEY_REQUIRED = 0xAF,

    //@}

    /**
     * @name  Security Errors
     */
    /**
     * @brief The passed key data is not valid. A key of all zeros or
     *   all F's are reserved values and cannot be used.
     */
    EMBER_KEY_INVALID = 0xB2,

    /**
     * @brief The chosen security level (the value of ::EMBER_SECURITY_LEVEL)
     *  is not supported by the stack.
     */
    EMBER_INVALID_SECURITY_LEVEL = 0x95,

    /**
     * @brief An error occurred when trying to encrypt at the APS Level.
     *
     * In order to APS encrypt an outgoing packet, the sender
     * needs to know the EUI64 of the destination. This error occurs because
     * the EUI64 of the destination can't be determined from
     * the short address (no entry in the neighbor, child, binding
     * or address tables).
     *
     * Everytime this error code is seen, note that the stack initiates an
     * IEEE address discovery request behind the scenes. Responses
     * to the request are stored in the trust center cache portion of the
     * address table. Note that you need at least 1 entry allocated for
     * TC cache in the address table plugin. Depending on the available rows in
     * the table, newly discovered addresses may replace old ones. The address
     * table plugin is enabled by default on the host. If you are using an SoC
     * platform, please be sure to add the address table plugin.
     *
     * When customers choose to send APS messages by using short addresses,
     * they should incorporate a retry mechanism and try again, no sooner than
     * 2 seconds later, to resend the APS message. If the app always
     * receives 0xBE (EMBER_IEEE_ADDR_DISCOVERY_IN_PROGRESS) after
     * multiple retries, that might indicate that:
     * a) destination node is not on the network
     * b) there are problems with the health of the network
     * c) there may not be any space set asidein the address table for
     *    the newly discovered address - this can be rectified by reserving
     *    more entries for the trust center cache in the address table plugin
     */
    EMBER_IEEE_ADDRESS_DISCOVERY_IN_PROGRESS = 0xBE,
    /**
     * @brief An error occurred when trying to encrypt at the APS Level.
     *
     * This error occurs either because the long address of the recipient can't be
     * determined from the short address (no entry in the binding table)
     * or there is no link key entry in the table associated with the destination,
     * or there was a failure to load the correct key into the encryption core.
     */
    EMBER_APS_ENCRYPTION_ERROR = 0xA6,

    /** @brief There was an attempt to form or join a network with security
     * without calling ::emberSetInitialSecurityState() first.
     */
    EMBER_SECURITY_STATE_NOT_SET = 0xA8,

    /** @brief There was an attempt to set an entry in the key table using
     *   an invalid long address. Invalid addresses include:
     *    - The local device's IEEE address
     *    - Trust Center's IEEE address
     *    - An existing table entry's IEEE address
     *    - An address consisting of all zeros or all F's
     */
    EMBER_KEY_TABLE_INVALID_ADDRESS = 0xB3,

    /** @brief There was an attempt to set a security configuration that
     *   is not valid given the other security settings.
     */
    EMBER_SECURITY_CONFIGURATION_INVALID = 0xB7,

    /** @brief There was an attempt to broadcast a key switch too quickly after
     *    broadcasting the next network key. The Trust Center must wait at
     *    least a period equal to the broadcast timeout so that all routers
     *    have a chance to receive the broadcast of the new network key.
     */
    EMBER_TOO_SOON_FOR_SWITCH_KEY = 0xB8,

    /** @brief The received signature corresponding to the message that was passed
        to the CBKE Library failed verification and is not valid.
     */
    EMBER_SIGNATURE_VERIFY_FAILURE = 0xB9,

    /** @brief The message could not be sent because the link key corresponding
        to the destination is not authorized for use in APS data messages.
        APS Commands (sent by the stack) are allowed. To use it for encryption
        of APS data messages it must be authorized using a key agreement protocol
        (such as CBKE).
     */
    EMBER_KEY_NOT_AUTHORIZED = 0xBB,

    /** @brief The security data provided was not valid, or an
     *    integrity check failed.
     */
    EMBER_SECURITY_DATA_INVALID = 0xBD,

    //@}

    /**
     * @name  Miscellaneous Network Errors
     */
    //@{

    /**
     * @brief The node has not joined a network.
     */
    EMBER_NOT_JOINED = 0x93,

    /**
     * @brief A message cannot be sent because the network is currently
     *  overloaded.
     */
    EMBER_NETWORK_BUSY = 0xA1,

    /**
     * @brief The application tried to send a message using an
     * endpoint that it has not defined.
     */
    EMBER_INVALID_ENDPOINT = 0xA3,

    /**
     * @brief The application tried to use a binding that has been remotely
     * modified and the change has not yet been reported to the application.
     */
    EMBER_BINDING_HAS_CHANGED = 0xA4,

    /**
     * @brief An attempt to generate random bytes failed because of
     * insufficient random data from the radio.
     */
    EMBER_INSUFFICIENT_RANDOM_DATA = 0xA5,

    /** A Zigbee route error command frame was received indicating
     * that a source routed message from this node failed en route.
     */
    EMBER_SOURCE_ROUTE_FAILURE = 0xA9,

    /** A Zigbee route error command frame was received indicating
     * that a message sent to this node along a many-to-one route
     * failed en route. The route error frame was delivered by
     * an ad-hoc search for a functioning route.
     */
    EMBER_MANY_TO_ONE_ROUTE_FAILURE = 0xAA,

    //@}

    /**
     * @name  Miscellaneous Utility Errors
     */
    //@{

    /**
     * @brief A critical and fatal error indicating that the version of the
     * stack trying to run does not match with the chip it's running on. The
     * software (stack) on the chip must be replaced with software
     * compatible with the chip.
     */
    EMBER_STACK_AND_HARDWARE_MISMATCH = 0xB0,

    /**
     * @brief An index was passed into the function that was larger
     * than the valid range.
     */
    EMBER_INDEX_OUT_OF_RANGE = 0xB1,

    /**
     * @brief There are no empty entries left in the table.
     */
    EMBER_TABLE_FULL = 0xB4,

    /**
     * @brief The requested table entry has been erased and contains
     *   no valid data.
     */
    EMBER_TABLE_ENTRY_ERASED = 0xB6,

    /**
     * @brief The requested function cannot be executed because
     *   the library that contains the necessary functionality is not present.
     */
    EMBER_LIBRARY_NOT_PRESENT = 0xB5,

    /**
     * @brief The stack accepted the command and is currently processing
     * the request.  The results will be returned via an appropriate handler.
     */
    EMBER_OPERATION_IN_PROGRESS = 0xBA,

    /**
     * @brief The EUI of the Trust center has changed due to a successful rejoin.
     *   The device may need to perform other authentication to verify the new TC
     *   is authorized to take over.
     */
    EMBER_TRUST_CENTER_EUI_HAS_CHANGED = 0xBC,

    //@}

    /**
     * @name  NVM3 Token Errors
     */
    //@{

    /**
     * @brief NVM3 is telling the application that the initialization was aborted
     *   as no valid NVM3 page was found.
     */
    EMBER_NVM3_TOKEN_NO_VALID_PAGES = 0xC0,

    /**
     * @brief NVM3 is telling the application that the initialization was aborted
     *   as the NVM3 instance was already opened with other parameters.
     */
    EMBER_NVM3_ERR_OPENED_WITH_OTHER_PARAMETERS = 0xC1,

    /**
     * @brief NVM3 is telling the application that the initialization was aborted
     *   as the NVM3 instance is not aligned properly in memory.
     */
    EMBER_NVM3_ERR_ALIGNMENT_INVALID = 0xC2,

    /**
     * @brief NVM3 is telling the application that the initialization was aborted
     *   as the size of the NVM3 instance is too small.
     */
    EMBER_NVM3_ERR_SIZE_TOO_SMALL = 0xC3,

    /**
     * @brief NVM3 is telling the application that the initialization was aborted
     *    as the NVM3 page size is not supported.
     */
    EMBER_NVM3_ERR_PAGE_SIZE_NOT_SUPPORTED = 0xC4,

    /**
     * @brief NVM3 is telling the application that there was an error initializing
     * some of the tokens.
     */
    EMBER_NVM3_ERR_TOKEN_INIT = 0xC5,

    /**
     * @brief NVM3 is telling the application there has been an error when
     * attempting to upgrade SimEE tokens.
     */
    EMBER_NVM3_ERR_UPGRADE = 0xC6,

    /**
     * @brief NVM3 is telling the application that there has been an unknown
     * error.
     */
    EMBER_NVM3_ERR_UNKNOWN = 0xC7,

    //@}

    /**
     * @name  Application Errors
     * These error codes are available for application use.
     */
    //@{

    /**
     * @brief This error is reserved for customer application use.
     *  This will never be returned from any portion of the network stack or HAL.
     */
    EMBER_APPLICATION_ERROR_0  = 0xF0,
    EMBER_APPLICATION_ERROR_1  = 0xF1,
    EMBER_APPLICATION_ERROR_2  = 0xF2,
    EMBER_APPLICATION_ERROR_3  = 0xF3,
    EMBER_APPLICATION_ERROR_4  = 0xF4,
    EMBER_APPLICATION_ERROR_5  = 0xF5,
    EMBER_APPLICATION_ERROR_6  = 0xF6,
    EMBER_APPLICATION_ERROR_7  = 0xF7,
    EMBER_APPLICATION_ERROR_8  = 0xF8,
    EMBER_APPLICATION_ERROR_9  = 0xF9,
    EMBER_APPLICATION_ERROR_10 = 0xFA,
    EMBER_APPLICATION_ERROR_11 = 0xFB,
    EMBER_APPLICATION_ERROR_12 = 0xFC,
    EMBER_APPLICATION_ERROR_13 = 0xFD,
    EMBER_APPLICATION_ERROR_14 = 0xFE,
    EMBER_APPLICATION_ERROR_15 = 0xFF,
};

/** @brief The control structure for events.
 *
 * This structure should not be accessed directly.
 * It holds the event status (one of the @e EMBER_EVENT_ values)
 * and the time left before the event fires.
 */
typedef struct
{
    /** The event's status, either inactive or the units for timeToExecute. */
    EmberEventUnits status;
    /** The ID of the task this event belongs to. */
    EmberTaskId taskid;
    /** Indicates how long before the event fires.
     *  Units are milliseconds.
     */
    uint32_t timeToExecute;
} EmberEventControl;

/**
 * @name ZigBee Broadcast Addresses
 *@{
 *  ZigBee specifies three different broadcast addresses that
 *  reach different collections of nodes.  Broadcasts are normally sent only
 *  to routers.  Broadcasts can also be forwarded to end devices, either
 *  all of them or only those that do not sleep.  Broadcasting to end
 *  devices is both significantly more resource-intensive and significantly
 *  less reliable than broadcasting to routers.
 */

/** Broadcast to all routers. */
#define EMBER_BROADCAST_ADDRESS 0xFFFC

/**
 * @description Useful macro for avoiding compiler warnings related to unused
 * function arguments or unused variables.
 */
#define UNUSED_VAR(x) (void) (x)

/**
 * @brief The maximum 802.15.4 channel number is 26.
 */
#define EMBER_MAX_802_15_4_CHANNEL_NUMBER 26

/**
 * @brief The minimum 2.4GHz 802.15.4 channel number is 11.
 */
#define EMBER_MIN_802_15_4_CHANNEL_NUMBER 11

/**
 * @brief The maximum SubGhz channel number on pages 28, 30, 31 is 26.
 */
#define EMBER_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGES_28_30_31 26

/**
 * @brief The maximum SubGhz channel number on page 29 is 8.
 */
#define EMBER_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGE_29 8

/**
 * @brief The broadcast endpoint, as defined in the ZigBee spec.
 */
#define EMBER_BROADCAST_ENDPOINT 0xFF

/**
 * @brief Useful to reference a single bit of a byte.
 */
#define EMBER_BIT(nr) (1UL << (nr)) // Unsigned avoids compiler warnings re EMBER_BIT(15)

/**
 * @brief Returns the low byte of the 16-bit value \c n as an \c uint8_t.
 */
#define EMBER_LOW_BYTE(n) ((uint8_t)((n) &0xFF))

/**
 * @brief Returns the high byte of the 16-bit value \c n as an \c uint8_t.
 */
#define EMBER_HIGH_BYTE(n) ((uint8_t)(EMBER_LOW_BYTE((n) >> 8)))
/**
 * @brief Returns the low byte of the 32-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_0(n) ((uint8_t)((n) &0xFF))

/**
 * @brief Returns the second byte of the 32-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_1(n) EMBER_BYTE_0((n) >> 8)

/**
 * @brief Returns the third byte of the 32-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_2(n) EMBER_BYTE_0((n) >> 16)

/**
 * @brief Returns the high byte of the 32-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_3(n) EMBER_BYTE_0((n) >> 24)

/**
 * @brief Returns the fifth byte of the 64-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_4(n) EMBER_BYTE_0((n) >> 32)

/**
 * @brief Returns the sixth byte of the 64-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_5(n) EMBER_BYTE_0((n) >> 40)

/**
 * @brief Returns the seventh byte of the 64-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_6(n) EMBER_BYTE_0((n) >> 48)

/**
 * @brief Returns the high byte of the 64-bit value \c n as an \c uint8_t.
 */
#define EMBER_BYTE_7(n) EMBER_BYTE_0((n) >> 56)

/**
 * @brief Returns the value built from the two \c uint8_t
 * values \c high and \c low.
 */
#define EMBER_HIGH_LOW_TO_INT(high, low) (((uint16_t)(((uint16_t)(high)) << 8)) + ((uint16_t)((low) &0xFF)))

/**
 * @brief The kind of arguments the main function takes
 */
#define MAIN_FUNCTION_PARAMETERS void
#define MAIN_FUNCTION_ARGUMENTS

// Stubs to just silence some compile errors

#define emberAfPrintEnabled(...) false

#define emberAfPrintActiveArea EMBER_AF_PRINT_CORE

/**
 * @brief Returns the value of \c bit within the register or byte \c reg.
 */
#define READBIT(reg, bit) ((reg) & (EMBER_BIT(bit)))

/**
 * @brief Returns the value of the bitmask \c bits within
 * the register or byte \c reg.
 */
#define READBITS(reg, bits) ((reg) & (bits))

#define MILLISECOND_TICKS_PER_SECOND 1000
#define MILLISECOND_TICKS_PER_DECISECOND (MILLISECOND_TICKS_PER_SECOND / 10)

/**
 * @brief Macro that copies the token value from non-volatile storage into a RAM
 * location.  This macro can only be used with tokens that are defined using
 * DEFINE_INDEXED_TOKEN.
 *
 * @note To better understand the parameters of this macro, refer to the
 *           example of token usage above.
 *
 * @param data   A pointer to where the token data should be placed.
 *
 * @param token  The token name used in <code>DEFINE_*_TOKEN</code>,
 *               prepended with <code>TOKEN_</code>.
 * @param index  The index to access in the indexed token.
 */
#define halCommonGetIndexedToken(data, token, index)

/**
 * @brief Macro that sets the value of a token in non-volatile storage.  This
 * macro can only be used with tokens that are defined using
 * DEFINE_INDEXED_TOKEN.
 *
 * @note  To better understand the parameters of this macro, refer to the
 *           example of token usage above.
 *
 * @param token  The token name used in <code>DEFINE_*_TOKEN</code>,
 * prepended with <code>TOKEN_</code>.
 *
 * @param index  The index to access in the indexed token.
 *
 * @param data   A pointer to where the token data should be placed.
 */
#define halCommonSetIndexedToken(token, index, data)
