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
  EMBER_APS_OPTION_NONE                     = 0x0000,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  EMBER_APS_OPTION_ENCRYPT_WITH_TRANSIENT_KEY = 0x0001,
  EMBER_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER = 0x0002,
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
  EMBER_APS_OPTION_DSA_SIGN                 = 0x0010,
  /** Send the message using APS Encryption using the Link Key shared
      with the destination node to encrypt the data at the APS Level. */
  EMBER_APS_OPTION_ENCRYPTION               = 0x0020,
  /** Resend the message using the APS retry mechanism.
      This option and the enable route discovery option must be enabled for
      an existing route to be repaired automatically. */
  EMBER_APS_OPTION_RETRY                    = 0x0040,
  /** Send the message with the NWK 'enable route discovery' flag, which
      causes a route discovery to be initiated if no route to the destination
      is known.  Note that in the mesh stack, this option and the APS retry
      option must be enabled an existing route to be repaired
      automatically. */
  EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY   = 0x0100,
  /** Send the message with the NWK 'force route discovery' flag, which causes
      a route discovery to be initiated even if one is known. */
  EMBER_APS_OPTION_FORCE_ROUTE_DISCOVERY    = 0x0200,
  /** Include the source EUI64 in the network frame. */
  EMBER_APS_OPTION_SOURCE_EUI64             = 0x0400,
  /** Include the destination EUI64 in the network frame. */
  EMBER_APS_OPTION_DESTINATION_EUI64        = 0x0800,
  /** Send a ZDO request to discover the node ID of the destination if it is
      not already known. */
  EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY = 0x1000,
  /** This message is being sent in response to a call to
      ::emberPollHandler().  It causes the message to be sent
      immediately instead of being queued up until the next poll from the
      (end device) destination. */
  EMBER_APS_OPTION_POLL_RESPONSE            = 0x2000,
  /** This incoming message is a valid ZDO request and the application
   *   is responsible for sending a ZDO response. This flag is used only
   *   within emberIncomingMessageHandler() when
   *   EMBER_APPLICATION_RECEIVES_UNSUPPORTED_ZDO_REQUESTS is defined. */
  EMBER_APS_OPTION_ZDO_RESPONSE_REQUIRED    = 0x4000,
  /** This message is part of a fragmented message.  This option may only
      be set for unicasts.  The groupId field gives the index of this
      fragment in the low-order byte.  If the low-order byte is zero this
      is the first fragment and the high-order byte contains the number
      of fragments in the message. */
  EMBER_APS_OPTION_FRAGMENT                 = SIGNED_ENUM 0x8000
};


/** @brief An in-memory representation of a ZigBee APS frame
 * of an incoming or outgoing message.
 */
typedef struct {
  /** The application profile ID that describes the format of the message. */
  uint16_t profileId;
  /** The cluster ID for this message. */
  uint16_t clusterId;
  /** The source endpoint. */
  uint8_t sourceEndpoint;
  /** The destination endpoint. */
  uint8_t destinationEndpoint;
  /** A bitmask of options from the enumeration above. */
  EmberApsOption options;
  /** The group ID for this message, if it is multicast mode. */
  uint16_t groupId;
  /** The sequence number. */
  uint8_t sequence;
  uint8_t radius;
} EmberApsFrame;

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

/** @brief 16-bit ZigBee multicast group identifier. */
typedef uint16_t EmberMulticastId;

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
  EMBER_DENY_TC_LINK_KEY_REQUESTS  = 0x00,
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
  EMBER_DENY_APP_LINK_KEY_REQUESTS  = 0x00,
  EMBER_ALLOW_APP_LINK_KEY_REQUEST = 0x01
};


/** @brief This is the Extended Security Bitmask that controls the use
 *  of various extended security features.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberExtendedSecurityBitmask
#else
typedef uint16_t EmberExtendedSecurityBitmask;
enum
#endif
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // If this bit is set, the 'key token data' field is set in the Initial
  // Security Bitmask to 0 (No Preconfig Key token). Otherwise, the
  // field is left as is.
  EMBER_PRECONFIG_KEY_NOT_VALID       = 0x0001,
#endif

  // bits 2-3 are unused.
  /** This denotes that the network key update can only happen if the network
      key update request is unicast and encrypted i.e. broadcast network key update
      requests will not be processed if bit 1 is set*/
  EMBER_SECURE_NETWORK_KEY_ROTATION   = 0x0002,

  /** This denotes whether a joiner node (router or end-device) uses a Global
      Link Key or a Unique Link Key. */
  EMBER_JOINER_GLOBAL_LINK_KEY        = 0x0010,

  /** This denotes whether the device's outgoing frame counter is allowed to
      be reset during forming or joining. If the flag is set, the outgoing frame
      counter is not allowed to be reset. If the flag is not set, the frame
      counter is allowed to be reset. */

  EMBER_EXT_NO_FRAME_COUNTER_RESET    = 0x0020,

  /** This denotes whether a device should discard or accept network leave
      without rejoin commands. */
  EMBER_NWK_LEAVE_WITHOUT_REJOIN_NOT_ALLOWED = 0x0040,

  // Bit 7 reserved for future use (stored in TOKEN).

  /** This denotes whether a router node should discard or accept network Leave
      Commands. */
  EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED = 0x0100,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  /** This denotes whether a node is running the latest stack specification or
      is emulating R18 specs behavior. If this flag is enabled, a router
      node should only send encrypted Update Device messages while the TC should
      only accept encrypted Updated Device messages.*/
  EMBER_R18_STACK_BEHAVIOR            = 0x0200,
#endif

  // Bit 10 is reserved for future use (stored in TOKEN).
  // Bit 11 is reserved for future use(stored in RAM).
  // Bits 12-15 are unused.
};

/** @brief This data structure contains the key data that is passed
 *   into various other functions. */
typedef struct {
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
