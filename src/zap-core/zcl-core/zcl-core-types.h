/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief ZCL Core Types
 ******************************************************************************/

#ifndef ZCL_CORE_TYPES_H
#define ZCL_CORE_TYPES_H

/**
 * @addtogroup ZCLIP
 * @{
 */

// -----------------------------------------------------------------------------
// Utilities.

/**
 * @addtogroup ZCLIP_utilities Utilities
 *
 * @{
 */

/**
 * The longest ZCL/IP URI is:
 *   coaps://nih:sha-256;<uid>:PPPPP/zcl/g/GGGG/RMMMM_CCCC/a/AAAA
 * where <uid> is a 256-bit UID represented as 64 hexadecimal characters, PPPPP
 * is a 16-bit UDP port in decimal, GGGG is the 16-bit group ID in hexadecimal,
 * R is c or s for client or server, MMMM is the 16-bit manufacturer code in
 * hexadecimal, CCCC is the 16-bit cluster ID in hexadecimal, and AAAA is the
 * 16-bit attribute ID in hexadecimal.  An extra byte is reserved for a null
 * terminator. */
#define EMBER_ZCL_URI_MAX_LENGTH 120

/**
 * The longest ZCL/IP URI path is a manufacturer-specific attribute request
 * sent to a group:
 *   zcl/g/GGGG/RMMMM_CCCC/a/AAAA
 * where GGGG is the 16-bit group ID, R is c or s for client or server, MMMM is
 * the 16-bit manufacturer code, CCCC is the 16-bit cluster ID, and AAAA is the
 * 16-bit attribute ID.  An extra byte is reserved for a null terminator. */
#define EMBER_ZCL_URI_PATH_MAX_LENGTH 29

/**
 * The longest cluster ID in a ZCL/IP URI path is manufacturer-specific:
 *   RMMMM_CCCC
 * where R is c or s for client or server, MMMM is the 16-bit manufacturer
 * code, and CCCC is the 16-bit cluster ID.  An extra byte is reserved for a
 * null terminator. */
#define EMBER_ZCL_URI_PATH_CLUSTER_ID_MAX_LENGTH 11

/**
 * Manufacturer codes, if present, are separated from the cluster ID by an
 * underscore. */
#define EMBER_ZCL_URI_PATH_MANUFACTURER_CODE_CLUSTER_ID_SEPARATOR '_'

/** String overhead. */
#define EMBER_ZCL_STRING_OVERHEAD            1
/** Maximum string length. */
#define EMBER_ZCL_STRING_LENGTH_MAX          0xFE
/** Invalid string length. */
#define EMBER_ZCL_STRING_LENGTH_INVALID      0xFF
/** Long string overhead. */
#define EMBER_ZCL_LONG_STRING_OVERHEAD       2
/** Maximum long string length. */
#define EMBER_ZCL_LONG_STRING_LENGTH_MAX     0xFFFE
/** Invalid long string length. */
#define EMBER_ZCL_LONG_STRING_LENGTH_INVALID 0xFFFF

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Types.

/**
 * @addtogroup ZCLIP_zcl_types ZCL Types
 *
 * See zcl-core-types.h for source code.
 * @{
 */

// From 07-5123-05, section 2.5.3, table 2-10.
/** A success or failure status, used as a system-wide return type for functions. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclStatus_t
#else
typedef uint8_t EmberZclStatus_t;
enum
#endif
{
  /** The operation was successful. */
  EMBER_ZCL_STATUS_SUCCESS                     = 0x00,
  /** The operation was not successful. */
  EMBER_ZCL_STATUS_FAILURE                     = 0x01,
  /**
   * The sender is recognized but forbidden from carrying out this
   * command. */
  EMBER_ZCL_STATUS_FORBIDDEN                  = 0x7D,
  /**
   * The sender of the command does not have authorization to carry out this
   * command. */
  EMBER_ZCL_STATUS_NOT_AUTHORIZED              = 0x7E,
  /** A reserved field/subfield/bit contains a non-zero value. */
  EMBER_ZCL_STATUS_RESERVED_FIELD_NOT_ZERO     = 0x7F,
  /**
   * The command appears to contain the wrong fields, as detected either by the
   * presence of one or more invalid field entries or by missing
   * fields. Command not carried out. */
  EMBER_ZCL_STATUS_MALFORMED_COMMAND           = 0x80,
  /**
   * The specified cluster command is not supported on the device. The command is not
   * carried out. */
  EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND       = 0x81,
  /** The specified general ZCL command is not supported on the device. */
  EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND       = 0x82,
  /**
   * A manufacturer-specific unicast, cluster specific command was received with
   * an unknown manufacturer code, or the manufacturer code was recognized but
   * the command is not supported. */
  EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND = 0x83,
  /**
   * A manufacturer-specific unicast, ZCL specific command was received with an
   * unknown manufacturer code, or the manufacturer code was recognized but the
   * command is not supported. */
  EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND = 0x84,
  /**
   * At least one field of the command contains an incorrect value, according to
   * the specification the device is implemented to. */
  EMBER_ZCL_STATUS_INVALID_FIELD               = 0x85,
  /** The specified attribute does not exist on the device. */
  EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE       = 0x86,
  /**
   * An out of range error, or set to a reserved value. An attribute keeps its old
   * value. Note that an attribute value may be out of range if the attribute is
   * related to another, e.g., with minimum and maximum attributes. See the
   * individual attribute descriptions in ZCL specification for specific details. */
  EMBER_ZCL_STATUS_INVALID_VALUE               = 0x87,
  /** Attempt to write a read only attribute. */
  EMBER_ZCL_STATUS_READ_ONLY                   = 0x88,
  /**
   * An operation (e.g., an attempt to create an entry in a table) failed due to
   * an insufficient amount of free space available. */
  EMBER_ZCL_STATUS_INSUFFICIENT_SPACE          = 0x89,
  /**
   * An attempt to create an entry in a table failed due to a duplicate entry
   * already present in the table. */
  EMBER_ZCL_STATUS_DUPLICATE_EXISTS            = 0x8A,
  /** The requested information (e.g., table entry) could not be found. */
  EMBER_ZCL_STATUS_NOT_FOUND                   = 0x8B,
  /** Periodic reports cannot be issued for this attribute. */
  EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE      = 0x8C,
  /** The data type given for an attribute is incorrect. The command is not carried out. */
  EMBER_ZCL_STATUS_INVALID_DATA_TYPE           = 0x8D,
  /** The selector for an attribute is incorrect. */
  EMBER_ZCL_STATUS_INVALID_SELECTOR            = 0x8E,
  /**
   * A request has been made to read an attribute that the requestor is not
   * authorized to read. No action taken. */
  EMBER_ZCL_STATUS_WRITE_ONLY                  = 0x8F,
  /**
   * Setting the requested values puts the device in an inconsistent state
   * on startup. No action taken.*/
  EMBER_ZCL_STATUS_INCONSISTENT_STARTUP_STATE  = 0x90,
  /**
   * An attempt has been made to write an attribute that is present but is
   * defined using an out-of-band method and not over the air. */
  EMBER_ZCL_STATUS_DEFINED_OUT_OF_BAND         = 0x91,
  /** The supplied values (e.g., contents of table cells) are inconsistent. */
  EMBER_ZCL_STATUS_INCONSISTENT                = 0x92,
  /**
   * The credentials presented by the device sending the command are not
   * sufficient to perform this action. */
  EMBER_ZCL_STATUS_ACTION_DENIED               = 0x93,
  /** The exchange was aborted due to excessive response time. */
  EMBER_ZCL_STATUS_TIMEOUT                     = 0x94,
  /** Failed case when a client or a server decides to abort the upgrade process. */
  EMBER_ZCL_STATUS_ABORT                       = 0x95,
  /**
   * Invalid OTA upgrade image (ex. failed signature validation or signer
   * information check or CRC check). */
  EMBER_ZCL_STATUS_INVALID_IMAGE               = 0x96,
  /** Server does not have the data block available yet. */
  EMBER_ZCL_STATUS_WAIT_FOR_DATA               = 0x97,
  /** No OTA upgrade image available for a particular client. */
  EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE          = 0x98,
  /**
   * The client still requires more OTA upgrade image files to
   * successfully upgrade. */
  EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE          = 0x99,
  /** The command has been received and is being processed. */
  EMBER_ZCL_STATUS_NOTIFICATION_PENDING        = 0x9A,
  /** An operation was unsuccessful due to a hardware failure. */
  EMBER_ZCL_STATUS_HARDWARE_FAILURE            = 0xC0,
  /** An operation was unsuccessful due to a software failure. */
  EMBER_ZCL_STATUS_SOFTWARE_FAILURE            = 0xC1,
  /** An error occurred during calibration. */
  EMBER_ZCL_STATUS_CALIBRATION_ERROR           = 0xC2,
  /** Distinguished value that represents a null (invalid) status. */
  EMBER_ZCL_STATUS_NULL                        = 0xFF,
};

/** Representation (length and pointer) of a text or binary string value. */
typedef struct {
  /** Length of the string. */
  uint32_t length;
  /** Pointer to the string. */
  uint8_t *ptr;
} EmberZclStringType_t;

// From 07-5123-05, section 2.5.2, table 2-9.
typedef uint8_t  data8_t;
typedef uint16_t data16_t;
//typedef uint24_t data24_t;
typedef uint32_t data32_t;
//typedef uint40_t data40_t;
//typedef uint48_t data48_t;
//typedef uint56_t data56_t;
typedef uint64_t data64_t;
typedef uint8_t  bitmap8_t;
typedef uint16_t bitmap16_t;
//typedef uint24_t bitmap24_t;
typedef uint32_t bitmap32_t;
//typedef uint40_t bitmap40_t;
//typedef uint48_t bitmap48_t;
//typedef uint56_t bitmap56_t;
typedef uint64_t bitmap64_t;
typedef uint8_t  enum8_t;
typedef uint16_t enum16_t;
typedef uint32_t utc_time_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Discovery.

/**
 * @addtogroup ZCLIP_discovery Discovery
 *
 * See zcl-core-types.h for source code.
 * @{
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef uint32_t EmZclDiscoveryContextMask_t;
enum {
  /** No filter */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_NONE                                 = 0x00000001,
  /** Filter by Cluster (wildcard) e.g. zcl:c.<cluster_id_start>* */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD                     = 0x00000002,
  /** Filter by Cluster (wildcard) to return all e.g. zcl:c.* */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD_ALL                 = 0x00000004,
  /** Filter by Cluster identifier e.g. zcl:c.<cluster_id>.<role> or zcl:c.<cluster_id>.* */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ID                      = 0x00000008,
  /** Filter by Cluster role e.g. zcl:c.<cluster_id>.<role> */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ROLE                    = 0x00000010,
  /** Filter by Device (wildcard) e.g. zcl:d.<device_id_start>* */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WILDCARD                   = 0x00000020,
  /** Filter by Device identifier e.g. zcl:d.<device_id>.<endpoint_id> or zcl:d.<device_id>.* */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ID                    = 0x00000040,
  /** Filter by Endpoint e.g. zcl:d.<device_id>.<endpoint_id> */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ENDPOINT              = 0x00000080,
  /** Filter by Cluster revision. */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUSTER_REVISION                     = 0x00000100,
  /** Query to check if ZCLIP is supported by Cluster. */
  EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUS                = 0x00000200,
  /** Query to check if ZCLIP is supported by Resource version. */
  EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_RESOURCE_VERSION    = 0x00000400,
  /** Query to check if ZCLIP is supported by Cluster/Role. */
  EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUSTER_AND_ROLE    = 0x00000800,
  /** Query to check if ZCLIP is supported by Device/Endpoint. */
  EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_DEVICE_AND_ENDPOINT = 0x00001000,
  /** Query for Unique identifier. */
  EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID                                  = 0x00002000,
  /** Query for Unique identifier prefix. */
  EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID_PREFIX                           = 0x00004000,
  /** Filter by resource wildcard i.e. if=urn:zcl* */
  EM_ZCL_DISCOVERY_CONTEXT_FILTER_RESOURCE_WILDCARD                       = 0x00008000,
};
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Messages.

/**
 * @addtogroup ZCLIP_messages Messages
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** Defines possible message statuses. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclMessageStatus_t
#else
typedef uint8_t EmberZclMessageStatus_t;
enum
#endif
{
  /** CoAP ::EMBER_COAP_MESSAGE_TIMED_OUT status recevied. */
  EMBER_ZCL_MESSAGE_STATUS_COAP_TIMEOUT      = EMBER_COAP_MESSAGE_TIMED_OUT,
  /** CoAP ::EMBER_COAP_MESSAGE_ACKED status received. */
  EMBER_ZCL_MESSAGE_STATUS_COAP_ACK          = EMBER_COAP_MESSAGE_ACKED,
  /** CoAP ::EMBER_COAP_MESSAGE_RESET status received. */
  EMBER_ZCL_MESSAGE_STATUS_COAP_RESET        = EMBER_COAP_MESSAGE_RESET,
  /** CoAP ::EMBER_COAP_MESSAGE_RESPONSE status received. */
  // This must be the last of the EMBER_COAP_MESSAGE_... values so that
  // our addition does not collide with an existing value.
  EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE     = EMBER_COAP_MESSAGE_RESPONSE,
  /** Discovery timed out. */
  EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT,
  /** CoAP unknown status recevied. */
  EMBER_ZCL_MESSAGE_STATUS_NULL              = 0xFF,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef void (*EmZclMessageResponseHandler)(EmberZclMessageStatus_t status,
                                            EmberCoapCode code,
                                            EmberCoapReadOptions *options,
                                            uint8_t *payload,
                                            uint16_t payloadLength,
                                            EmberCoapResponseInfo *info);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// UIDs.

/**
 * @addtogroup ZCLIP_addresses Addresses
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** UID size in bits. */
#define EMBER_ZCL_UID_BITS          256
/** UID size in bytes. */
#define EMBER_ZCL_UID_SIZE          EMBER_BITS_TO_BYTES(EMBER_ZCL_UID_BITS)
/** Text string length to represent a UID (hexadecimal characters). */
#define EMBER_ZCL_UID_STRING_LENGTH (EMBER_ZCL_UID_BITS / 4) // bits to nibbles
/** Text string length to represent a UID (hexadecimal characters), plus trailing NUL. */
#define EMBER_ZCL_UID_STRING_SIZE   (EMBER_ZCL_UID_STRING_LENGTH + 1) // NUL
/** Text string length to represent UID length (base64url characters). */
#define EMBER_ZCL_UID_BASE64URL_LENGTH (((EMBER_ZCL_UID_SIZE * 8) + 5) / 6)
/** Text string length to represent UID length (base64url characters), plus trailing NUL. */
#define EMBER_ZCL_UID_BASE64URL_SIZE (EMBER_ZCL_UID_BASE64URL_LENGTH + 1) // NUL

/** UID (Unique Identifier). */
typedef struct {
  uint8_t bytes[EMBER_ZCL_UID_SIZE];
} EmberZclUid_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Endpoints.

/**
 * @addtogroup ZCLIP_endpoints Endpoints
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** An endpoint identifier. */
typedef uint8_t EmberZclEndpointId_t;
/** A minimum endpoint identifer value. */
#define EMBER_ZCL_ENDPOINT_MIN  0x01
/** A maximum endpoint identifer value. */
#define EMBER_ZCL_ENDPOINT_MAX  0xF0
/** A distinguished value that represents a null (invalid) endpoint identifer. */
#define EMBER_ZCL_ENDPOINT_NULL ((EmberZclEndpointId_t)-1)

/** An endpoint index. */
typedef uint8_t EmberZclEndpointIndex_t;
/** A distinguished value that represents a null (invalid) endpoint index. */
#define EMBER_ZCL_ENDPOINT_INDEX_NULL ((EmberZclEndpointIndex_t)-1)

/** A device identifier. */
typedef uint16_t EmberZclDeviceId_t;
/** A distinguished value that represents a null (invalid) device identifer. */
#define EMBER_ZCL_DEVICE_ID_NULL ((EmberZclDeviceId_t)-1)

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Groups.

/**
 * @addtogroup ZCLIP_groups Groups
 *
 * See zcl-core-types.h for source code.
 * @{
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef uint8_t EmZclGroupNameSupportMask_t;
enum {
  /** Group names are supported. */
  EM_ZCL_GROUP_NAME_SUPPORT_ENABLED  = 0x01,
};
#endif

/** A group identifier. */
typedef uint16_t EmberZclGroupId_t;
/**
 * A group identifier for the all-endpoints (endpoint broadcast) group.
 *
 * All endpoints are always members of this group. This group cannot be
 * removed and no endpoint can be removed from it.
 */
#define EMBER_ZCL_GROUP_ALL_ENDPOINTS 0xFFFF
/** A minimum group identifier value. */
#define EMBER_ZCL_GROUP_MIN           0x0001
/** A maximum group identifier value. */
#define EMBER_ZCL_GROUP_MAX           0xFFF7
/** A distinguished value that represents a null (invalid) group identifier. */
#define EMBER_ZCL_GROUP_NULL          0x0000

/** Lower range for the automatic address assignment mode  */
#define EMBER_ZCL_GROUP_ASSIGNMENT_MODE_AUTO_MIN     0x00
/** Upper range for the automatic address assignment mode  */
#define EMBER_ZCL_GROUP_ASSIGNMENT_MODE_AUTO_MAX    0x10
/** Manual address assignment mode */
#define EMBER_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL       0xFE
/** Value representing empty address parameters passed, using defaults */
#define EMBER_ZCL_GROUP_ASSIGNMENT_MODE_NULL         0xFF

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Roles.

/**
 * @addtogroup ZCLIP_clusters Clusters
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** Defines possible roles of a cluster. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclRole_t
#else
typedef uint8_t EmberZclRole_t;
enum
#endif
{
  /** Cluster is a client. */
  EMBER_ZCL_ROLE_CLIENT = 0,
  /** Cluster is a server. */
  EMBER_ZCL_ROLE_SERVER = 1,
};

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Manufacturer codes.

/**
 * @addtogroup ZCLIP_clusters Clusters
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** A manufacturer code. */
typedef uint16_t EmberZclManufacturerCode_t;
/** A distinguished value that represents a null (invalid) manufacturer code. */
#define EMBER_ZCL_MANUFACTURER_CODE_NULL 0x0000

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Clusters.

/**
 * @addtogroup ZCLIP_clusters Clusters
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** A cluster identifier. */
typedef uint16_t EmberZclClusterId_t;
/** A distinguished value that represents a null (invalid) cluster identifier. */
#define EMBER_ZCL_CLUSTER_NULL ((EmberZclClusterId_t)-1)

/** This structure holds a cluster specification. */
typedef struct {
  /** Role of a cluster. */
  EmberZclRole_t role;
  /** Manufacturer code of a cluster. */
  EmberZclManufacturerCode_t manufacturerCode;
  /** Identifier of a cluster. */
  EmberZclClusterId_t id;
} EmberZclClusterSpec_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Attributes.

/**
 * @addtogroup ZCLIP_attributes Attributes
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** An attribute identifier. */
typedef uint16_t EmberZclAttributeId_t;
/** An attribute identifier for the Cluster revision. */
#define EMBER_ZCL_ATTRIBUTE_CLUSTER_REVISION 0xFFFD
/** An attribute identifier for a Reporting status. */
#define EMBER_ZCL_ATTRIBUTE_REPORTING_STATUS 0xFFFE
/** A distinguished value that represents a null (invalid) attribute identifier. */
#define EMBER_ZCL_ATTRIBUTE_NULL             ((EmberZclAttributeId_t)-1)

/** A cluster revision. */
typedef uint16_t EmberZclClusterRevision_t;
/** A cluster revision for Pre-ZCL 6 specification. */
#define EMBER_ZCL_CLUSTER_REVISION_PRE_ZCL6 0
/** A cluster revision for ZCL 6 specification. */
#define EMBER_ZCL_CLUSTER_REVISION_ZCL6     1
/** A distinguished value that represents a null (invalid) cluster revision. */
#define EMBER_ZCL_CLUSTER_REVISION_NULL     ((EmberZclClusterRevision_t)-1)

/** This structure holds an attribute specification. */
typedef struct {
  /** CoAP code of an attribute. */
  EmberCoapCode code;
  /** A group identifier of an attribute. */
  EmberZclGroupId_t groupId;
  /** An endpoint identifier of an attribute. */
  EmberZclEndpointId_t endpointId;
  /** A cluster specification of an attribute. */
  const EmberZclClusterSpec_t *clusterSpec;
  /** An attribute identifier. */
  EmberZclAttributeId_t attributeId;
  /** A status of an attribute used when reading and writing. */
  EmberZclStatus_t status;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  CborState *state;
#endif
} EmberZclAttributeContext_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Bindings.

/**
 * @addtogroup ZCLIP_bindings Bindings
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** A binding identifier. */
typedef uint8_t EmberZclBindingId_t;
/** A distinguished value that represents a null (invalid) binding identifier. */
#define EMBER_ZCL_BINDING_NULL ((EmberZclBindingId_t)-1)

/** This structure holds a binding context. */
typedef struct {
  /** CoAP code of binding. */
  EmberCoapCode code;
  /** A group identifier of binding. */
  EmberZclGroupId_t groupId;
  /** An endpoint identifier of binding. */
  EmberZclEndpointId_t endpointId;
  /** A cluster specification of binding. */
  const EmberZclClusterSpec_t *clusterSpec;
  /** A binding identifier. */
  EmberZclBindingId_t bindingId;
} EmberZclBindingContext_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Commands.

/**
 * @addtogroup ZCLIP_commands Commands
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** A command identifier. */
typedef uint8_t EmberZclCommandId_t;
/** A distinguished value that represents a null (invalid) command identifier. */
#define EMBER_ZCL_COMMAND_NULL ((EmberZclCommandId_t)-1)

/** This structure holds a command context. */
typedef struct {
  /** A remote address of a command. */
  EmberIpv6Address remoteAddress;
  /** CoAP code of a command. */
  EmberCoapCode code;
  /** EZ-Mode needs access to the request info structure */
  const EmberCoapRequestInfo *info;
  /** Payload of a command. */
  const uint8_t *payload;
  /** Payload length of a command. */
  uint16_t payloadLength;
  /** A group identifier of a command. */
  EmberZclGroupId_t groupId;
  /** An endpoint identifier of a command. */
  EmberZclEndpointId_t endpointId;
  /** A cluster specification of a command. */
  const EmberZclClusterSpec_t *clusterSpec;
  /** A command identifier. */
  EmberZclCommandId_t commandId;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  CborState *state;
  uint8_t *buffer;
  EmberStatus result;
#endif
} EmberZclCommandContext_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Reporting.

/**
 * @addtogroup ZCLIP_reporting Reporting
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** A reporting configuration identifier. */
typedef uint8_t EmberZclReportingConfigurationId_t;
/**
 * A distinguished value that represents a default reporting configuration
 * identifier. */
#define EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT 0
/**
 * A distinguished value that represents a null (invalid) reporting configuration
 * identifier. */
#define EMBER_ZCL_REPORTING_CONFIGURATION_NULL    ((EmberZclReportingConfigurationId_t)-1)

/** This structure holds a notification context. */
typedef struct {
  /** A remote address of a notification. */
  EmberIpv6Address remoteAddress;
  /** A source endpoint identifier of a notification. */
  EmberZclEndpointId_t sourceEndpointId;
  /** A source reporting configuration identifier of a notification. */
  EmberZclReportingConfigurationId_t sourceReportingConfigurationId;
  /** A source timestamp of a notification. */
  uint32_t sourceTimestamp;
  /** A group identifier of a notification. */
  EmberZclGroupId_t groupId;
  /** An endpoint identifier of a notification. */
  EmberZclEndpointId_t endpointId;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  const EmberZclClusterSpec_t *clusterSpec;
  EmberZclAttributeId_t attributeId;
  uint8_t *buffer;
  size_t bufferLength;
#endif
} EmberZclNotificationContext_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Addresses.

/**
 * @addtogroup ZCLIP_addresses Addresses
 *
 * See zcl-core-types.h for source code.
 * @{
 */

enum {
  EMBER_ZCL_NO_FLAGS               = 0x00,
  EMBER_ZCL_USE_COAPS_FLAG         = 0x01,
  EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG = 0x02,
  EMBER_ZCL_HAVE_UID_FLAG          = 0x04,
  EMBER_ZCL_AVOID_NEW_DTLS_SESSION = 0x08, // Directive to emZclSend()
};

typedef struct {
  uint16_t flags;
  EmberIpv6Address address;
  EmberZclUid_t uid;
  uint16_t port;
} EmberZclCoapEndpoint_t;

/** Defines possible types for an application destination. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclApplicationDestinationType_t
#else
typedef uint8_t EmberZclApplicationDestinationType_t;
enum
#endif
{
  /** An application destination uses an endpoint type. */
  EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT = 0x00,
  /** An application destination uses a group type. */
  EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP    = 0x01,
};

/** This structure holds an application destination. */
typedef struct {
  /**
   * Data holds an endpoint identifier or a group identifier for an application
   * destination. */
  union {
    /** An endpoint identifier of an application destination. */
    EmberZclEndpointId_t endpointId;
    /** A group identifier of an application destination. */
    EmberZclGroupId_t groupId;
  } data;
  /** Type of an application destination. */
  EmberZclApplicationDestinationType_t type;
} EmberZclApplicationDestination_t;

/** This structure holds a destination. */
typedef struct {
  /** A destination of a network. */
  EmberZclCoapEndpoint_t network;
  /** A destination of an application. */
  EmberZclApplicationDestination_t application;
} EmberZclDestination_t;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef uint16_t EmZclCacheIndex_t;
typedef struct {
  EmberZclUid_t key;
  EmberIpv6Address value;
  EmZclCacheIndex_t index;      // for commands only
} EmZclCacheEntry_t;

typedef bool (*EmZclCacheScanPredicate)(const void *criteria,
                                        const EmZclCacheEntry_t *entry);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Endpoints.

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct {
  EmberZclEndpointId_t endpointId;
  EmberZclDeviceId_t deviceId;
  const EmberZclClusterSpec_t **clusterSpecs;
} EmZclEndpointEntry_t;
#endif

// -----------------------------------------------------------------------------
// Attributes.

/**
 * @addtogroup ZCLIP_attributes Attributes
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/**************************************************************************//**
 * A handler fired when reading attributes.
 *
 * @param status A message status
 * @param context A context of a read attribute
 * @param buffer A content of a read attribute
 * @param bufferLength Content length
 *
 * @note `context->status` shows whether attribute was read successfully or
 *       if an error occured. If successful, a buffer contains an attribute value.
 *       If unsuccessful, the buffer is irrelevant.
 *
 * @sa emberZclSendAttributeRead()
 *****************************************************************************/
typedef void (*EmberZclReadAttributeResponseHandler)(EmberZclMessageStatus_t status,
                                                     const EmberZclAttributeContext_t *context,
                                                     const void *buffer,
                                                     size_t bufferLength);

/**************************************************************************//**
 * A handler fired when writing attributes.
 *
 * @param status A message status
 * @param context A context of a written attribute
 *
 * @note `context->status` shows whether attribute was written successfully
 *       or if an error occured.
 *
 * @sa emberZclSendAttributeWrite()
 *****************************************************************************/
typedef void (*EmberZclWriteAttributeResponseHandler)(EmberZclMessageStatus_t status,
                                                      const EmberZclAttributeContext_t *context);
/** This structure holds write data for an attribute. */
typedef struct {
  /** An attribute identifier to write to. */
  EmberZclAttributeId_t attributeId;
  /** A buffer containing data to be written. */
  const void *buffer;
  /** Length of data to be written. */
  size_t bufferLength;
} EmberZclAttributeWriteData_t;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef uint16_t EmZclAttributeMask_t;
enum {
  EM_ZCL_ATTRIBUTE_STORAGE_NONE           = 0x0000, // b0000_0000 b0000_0000
  EM_ZCL_ATTRIBUTE_STORAGE_TYPE_EXTERNAL  = 0x0001, // b0000_0000 b0000_0001
  EM_ZCL_ATTRIBUTE_STORAGE_TYPE_RAM       = 0x0003, // b0000_0000 b0000_0011
  EM_ZCL_ATTRIBUTE_STORAGE_TYPE_MASK      = 0x0003, // b0000_0000 b0000_0011
  EM_ZCL_ATTRIBUTE_STORAGE_SINGLETON_MASK = 0x0004, // b0000_0000 b0000_0100
  EM_ZCL_ATTRIBUTE_STORAGE_MASK           = 0x0007, // b0000_0000 b0000_0111

  EM_ZCL_ATTRIBUTE_ACCESS_READABLE        = 0x0010, // b0000_0000 b0001_0000
  EM_ZCL_ATTRIBUTE_ACCESS_WRITABLE        = 0x0020, // b0000_0000 b0010_0000
  EM_ZCL_ATTRIBUTE_ACCESS_REPORTABLE      = 0x0040, // b0000_0000 b0100_0000
  EM_ZCL_ATTRIBUTE_ACCESS_MASK            = 0x0070, // b0000_0000 b0111_0000

  EM_ZCL_ATTRIBUTE_DATA_DEFAULT           = 0x0100, // b0000_0001 b0000_0000
  EM_ZCL_ATTRIBUTE_DATA_MINIMUM           = 0x0200, // b0000_0010 b0000_0000
  EM_ZCL_ATTRIBUTE_DATA_MAXIMUM           = 0x0400, // b0000_0100 b0000_0000
  EM_ZCL_ATTRIBUTE_DATA_MASK              = 0x0700, // b0000_0111 b0000_0000
  EM_ZCL_ATTRIBUTE_DATA_BOUNDED           = 0x0800, // b0000_1000 b0000_0000
  EM_ZCL_ATTRIBUTE_DATA_ANALOG            = 0x1000, // b0001_0000 b0000_0000
};

typedef struct {
  const EmberZclClusterSpec_t *clusterSpec;
  EmberZclAttributeId_t attributeId;
  EmZclAttributeMask_t mask;
  size_t dataOffset;
  size_t defaultMinMaxLookupOffset;
  size_t size;
  uint8_t type;
} EmZclAttributeEntry_t;

typedef uint8_t EmZclAttributeQueryFilterType_t;
enum {
  EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_ID,
  EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_COUNT,
  EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_RANGE,
  EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_WILDCARD,
};

typedef struct {
  EmberZclAttributeId_t start;
  uint16_t count;
} EmZclAttributeQueryFilterCountData_t;

typedef struct {
  EmberZclAttributeId_t start;
  EmberZclAttributeId_t end;
} EmZclAttributeQueryFilterRangeData_t;

typedef struct {
  EmZclAttributeQueryFilterType_t type;
  union {
    EmberZclAttributeId_t attributeId;
    EmZclAttributeQueryFilterCountData_t countData;
    EmZclAttributeQueryFilterRangeData_t rangeData;
  } data;
} EmZclAttributeQueryFilter_t;

#define EM_ZCL_ATTRIBUTE_QUERY_FILTER_COUNT_MAX 10

typedef uint16_t EmZclMetadata_t;
enum {
  EM_ZCL_METADATA_NONE,
  EM_ZCL_METADATA_ID,
  EM_ZCL_METADATA_TYPE,
  EM_ZCL_METADATA_BASE,
  EM_ZCL_METADATA_DISPLAY_NAME,
  EM_ZCL_METADATA_DESCRIPTION,
  EM_ZCL_METADATA_UNIT,
  EM_ZCL_METADATA_MIN_VALUE,
  EM_ZCL_METADATA_MAX_VALUE,
  EM_ZCL_METADATA_ACCESS,
  EM_ZCL_METADATA_VARIABILITY,
  EM_ZCL_METADATA_VOLATILITY,
  EM_ZCL_METADATA_LIMITED_WRITES,
  EM_ZCL_METADATA_LINKS,
  EM_ZCL_METADATA_TAGS,
  EM_ZCL_METADATA_VALUE_TAGS,
  EM_ZCL_METADATA_HYPERTEXT_REF,
  EM_ZCL_METADATA_WILDCARD,
  EM_ZCL_METADATA_NOT_SUPPORTED,
};

typedef struct {
  // f=
  EmZclAttributeQueryFilter_t filters[EM_ZCL_ATTRIBUTE_QUERY_FILTER_COUNT_MAX];
  uint8_t filterCount;

  // u
  bool undivided;

  // meta=
  EmZclMetadata_t metadata;
} EmZclAttributeQuery_t;
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Bindings.

/**
 * @addtogroup ZCLIP_bindings Bindings
 *
 * See zcl-core-types.h for source code.
 * @{
 */

// These are the actual values stored in tokens.  Any changes to these
// types needs to be done very carefully to avoid breaking bindings in
// existing devices.

/** Defines possible schemes for a network destination. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclScheme_t
#else
typedef uint8_t EmberZclScheme_t;
enum
#endif
{
  /** Network destination uses standard CoAP scheme. */
  EMBER_ZCL_SCHEME_COAP  = 0x00,
  /** Network destination uses secure CoAP scheme. */
  EMBER_ZCL_SCHEME_COAPS = 0x01,
};

/** Defines possible types for a network destination. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclNetworkDestinationType_t
#else
typedef uint8_t EmberZclNetworkDestinationType_t;
enum
#endif
{
  /** A network destination uses an address type. */
  EMBER_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS  = 0x00,
  /** A network destination uses a unique identifier type. */
  EMBER_ZCL_NETWORK_DESTINATION_TYPE_UID      = 0x01,
  //EMBER_ZCL_NETWORK_DESTINATION_TYPE_HOSTNAME = 0x02,
};

/** This structure holds a binding entry. */
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  // From URI.
  /** An endpoint identifier of binding. */
  EmberZclEndpointId_t endpointId;
  /** A cluster specification of binding. */
  EmberZclClusterSpec_t clusterSpec;

  struct {
    struct {
      EmberZclScheme_t scheme;
      union {
        EmberIpv6Address address;
        EmberZclUid_t uid;
      } data;
      EmberZclNetworkDestinationType_t type;
      uint16_t port;
    } network;
    EmberZclApplicationDestination_t application;
  } destination;

  /** A reporting configuration of binding. */
  EmberZclReportingConfigurationId_t reportingConfigurationId;
} EmberZclBindingEntry_t;

/**************************************************************************//**
 * A handler fired when adding, updating, or removing a binding.
 *
 * @param status A message status
 * @param context A context of binding to add, update, or remove
 * @param entry An entry of binding to add, update, or remove
 *
 * @sa emberZclSendAddBinding()
 * @sa emberZclSendUpdateBinding()
 * @sa emberZclSendRemoveBinding()
 *****************************************************************************/
typedef void (*EmberZclBindingResponseHandler)(EmberZclMessageStatus_t status,
                                               const EmberZclBindingContext_t *context,
                                               const EmberZclBindingEntry_t *entry);

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Commands.

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef void (*EmZclRequestHandler)(const EmberZclCommandContext_t *context,
                                    const void *request);
typedef void (*EmZclResponseHandler)(EmberZclMessageStatus_t status,
                                     const EmberZclCommandContext_t *context,
                                     const void *response);
typedef struct {
  const EmberZclClusterSpec_t *clusterSpec;
  EmberZclCommandId_t commandId;
  const ZclipStructSpec *spec;
  EmZclRequestHandler handler;
} EmZclCommandEntry_t;
#endif

// -----------------------------------------------------------------------------
// Groups.

/**
 * @addtogroup ZCLIP_groups Groups
 *
 * See zcl-core-types.h for source code.
 * @{
 */

#ifndef EMBER_ZCL_MAX_GROUP_NAME_LENGTH
  #define EMBER_ZCL_MAX_GROUP_NAME_LENGTH 0
#endif

/**
 * This structure holds a group entry that represents membership of an endpoint
 * in a group. */
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  /** A group identifier of a group entry. */
  EmberZclGroupId_t groupId;
  /** An endpoint identifier of a group entry. */
  EmberZclEndpointId_t endpointId;
  /** Length of group name. */
  uint8_t groupNameLength;
  /** An array containing group name. */
#if EMBER_ZCL_MAX_GROUP_NAME_LENGTH == 0
  /** Save space **/
#else
  uint8_t groupName[EMBER_ZCL_MAX_GROUP_NAME_LENGTH];
#endif
  /** Address assignment mode as passed in "add group" command or 0xFF if address is not assigned **/
  uint8_t addrAssignmentMode;
  /** Group IPv6 address or flag/scope bits from "add group" command **/
  EmberIpv6Address  groupMcastAddress;
  /** UDP port the group is listening on: EMBER_COAP_PORT or whatever was passed in "add group" **/
  uint16_t groupUdpPort;
} EmberZclGroupEntry_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Reporting.

/**
 * @addtogroup ZCLIP_reporting Reporting
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** This structure holds a reporting configuration. */
typedef struct {
  /** A minimum interval in seconds. */
  uint16_t minimumIntervalS;
  /** A maximum interval in seconds. */
  uint16_t maximumIntervalS;
} EmberZclReportingConfiguration_t;

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Internal.

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// This value is used when declaring buffers to hold ZCLIP payload data (with
// the exception of responses to /.well-known/core requests). This size has
// proven to be sufficient for the majority of ZCLIP messages, but can be
// updated to fit an application's needs. It is important to remember that
// fragmentation may not be supported on every transport layer sitting below
// a ZCLIP implementation.
#define EM_ZCL_MAX_PAYLOAD_SIZE 128

#define MAX_URI_PATH_SEGMENTS 6 // zcl/[eg]/XX/<cluster>/Y/ZZ
#define MAX_URI_QUERY_SEGMENTS 10 // actual value TBD

typedef struct {
  EmberCoapCode code;
  EmberCoapReadOptions *options;
  const uint8_t *payload;
  uint16_t payloadLength;
  const EmberCoapRequestInfo *info;
  const uint8_t *uriPath[MAX_URI_PATH_SEGMENTS];
  uint16_t uriPathLength[MAX_URI_PATH_SEGMENTS];
  uint8_t uriPathSegments;
  const uint8_t *uriQuery[MAX_URI_PATH_SEGMENTS];
  uint16_t uriQueryLength[MAX_URI_PATH_SEGMENTS];
  uint8_t uriQuerySegments;

  union {
    struct { // zcl values
      // Values parsed out of the URI Path
      const EmZclEndpointEntry_t *endpoint;
      EmberZclGroupId_t groupId;
      const EmZclAttributeEntry_t *attribute;
      EmberZclBindingId_t bindingId;
      const EmZclCommandEntry_t *command;
      EmberZclReportingConfigurationId_t reportingConfigurationId;

      // Values parsed out of the URI Queries
      EmberZclClusterSpec_t clusterSpec;
      EmberZclDeviceId_t deviceId;
      EmberZclClusterRevision_t clusterRevision;
      EmZclDiscoveryContextMask_t mask;
      EmZclAttributeQuery_t attributeQuery;
    };
    struct { // rd values
      uint8_t rdConfId;
    };
  };

  const uint8_t *uidFilterString;
  uint16_t uidFilterLength;

  EmberZclUid_t uid;
  uint16_t uidBits;
} EmZclContext_t;

typedef EmberStatus (*EmZclMultiEndpointHandler)(
  const EmZclContext_t *context,
  CborState *state,
  void *data);

typedef EmberStatus (*EmZclCliRequestCommandFunction)(
  const EmberZclDestination_t *destination,
  const void *payloadStruct,
  const EmZclResponseHandler responseHandler);

typedef uint8_t EmZclUriFlag;
enum {
  EM_ZCL_URI_FLAG_METHOD_MASK    = 0x0F,
  EM_ZCL_URI_FLAG_METHOD_GET     = 0x01,
  EM_ZCL_URI_FLAG_METHOD_POST    = 0x02,
  EM_ZCL_URI_FLAG_METHOD_PUT     = 0x04,
  EM_ZCL_URI_FLAG_METHOD_DELETE  = 0x08,
  EM_ZCL_URI_FLAG_FORMAT_MASK    = 0x30,
  EM_ZCL_URI_FLAG_FORMAT_CBOR    = 0x10,
  EM_ZCL_URI_FLAG_FORMAT_LINK    = 0x20,
};

typedef bool (EmZclSegmentMatch)(EmZclContext_t *context, void *data, uint8_t depth);
typedef void (EmZclUriAction)(EmZclContext_t *context);

typedef const struct {
  EmZclSegmentMatch *match;
  void *data;
  EmZclSegmentMatch *parse;
} EmZclUriQuery;

typedef const struct {
  uint8_t matchSkip;    // how many entries to skip if the match succeeds
  uint8_t failSkip;     // how many entries to skip if the match fails
  EmZclUriFlag flags;
  EmZclSegmentMatch *match;
  void *data;
  EmZclUriQuery *queries;
  EmZclUriAction *action;
} EmZclUriPath;

// data representation of a URI-reference / context of URI.
// e.g., </zc/e/EE/[sc]CCCC>
typedef struct {
  EmberZclEndpointId_t endpointId;
  EmberZclClusterSpec_t *clusterSpec;
} EmZclUriContext_t;
#endif

/** @} end addtogroup */

// Used by emZclUseAccessControl to enable/disable access control
typedef enum  {
  ZCL_ACCESS_CONTROL_OFF = 0,
  ZCL_ACCESS_CONTROL_ON = 1,
  ZCL_ACCESS_CONTROL_ON_TEST = 2
} emZclAccessControlMode_t;

#endif // ZCL_CORE_TYPES_H
