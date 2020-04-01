/***************************************************************************//**
 * @file
 * @brief ZCL Core Well Known API
 ******************************************************************************/

#ifndef ZCL_CORE_WELL_KNOWN_H
#define ZCL_CORE_WELL_KNOWN_H

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE

/**
 * @addtogroup ZCLIP
 *
 * @{
 */

/**
 * @addtogroup ZCLIP_discovery Discovery
 *
 * See zcl-core-well-known.h for source code.
 * @{
 */

/** Defines possible request modes. */
typedef enum {
  /** Discovery request is allowed a single query. */
  EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY = 0,
  /** Discovery request is allowed multiple queries. */
  EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY = 1,
  /** Maximum discovery request mode. */
  EMBER_ZCL_DISCOVERY_REQUEST_MODE_MAX = 2
} EmberZclDiscoveryRequestMode;

/**************************************************************************//**
 * Initialization for sending Discovery command.
 *****************************************************************************/
void emberZclDiscInit(void);

/**************************************************************************//**
 * This function sets mode to create a query.
 *
 * @param      mode  EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY - single query
 *                   EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY - multiple queries
 *
 * @return     True if mode was set or false otherwise.
 *
 * Under EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode, appending one query string
 * automatically triggers the Discovery command to be broadcast.
 * Under EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode, appended query strings
 * is accumulated. The accumulated query string will not be broadcast until
 * emberZclDiscSend() is called.
 *****************************************************************************/
bool emberZclDiscSetMode(EmberZclDiscoveryRequestMode mode);

/**************************************************************************//**
 * This function broadcasts a GET using the Discovery request string.
 *
 * @param      responseHandler  The response handler
 *
 * @return     True if the message was sent or false otherwise.
 *
 *****************************************************************************/
bool emberZclDiscSend(EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a core.rd query to the discovery request string.
 *
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscCoreRd(EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a cluster ID query to the discovery request string.
 *
 * @param      clusterSpec      A structure for cluster ID / role / manufacture code
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec,
                             EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends an endpoint query to the Discovery request string.
 *
 * @param      endpointId       The endpoint identifier
 * @param      deviceId         The device identifier
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByEndpoint(EmberZclEndpointId_t endpointId,
                            EmberZclDeviceId_t deviceId,
                            EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a UID query to the discovery request string.
 *
 * @param      uid              The uid
 * @param      uidBits          The uid bits
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByUid(const EmberZclUid_t *uid,
                       uint16_t uidBits,
                       EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a cluster revision query to the discovery request string.
 *
 * @param      version          The version
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByClusterRev(EmberZclClusterRevision_t version,
                              EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a device ID query to the discovery request string.
 *
 * @param      deviceId         The device identifier
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByDeviceId(EmberZclDeviceId_t deviceId,
                            EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a resource version query to the discovery request string.
 *
 * @param      version          The version
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByResourceVersion(EmberZclClusterRevision_t version,
                                   EmberCoapResponseHandler responseHandler);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

extern EmZclUriPath emZclWellKnownUriPaths[];

#define EM_ZCL_URI_QUERY_UID                                "ep="
#define EM_ZCL_URI_QUERY_UID_SHA_256                        "ni:///sha-256;"
#define EM_ZCL_URI_QUERY_UID_SHA_256_PREFIX                 EM_ZCL_URI_QUERY_UID EM_ZCL_URI_QUERY_UID_SHA_256
#define EM_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CORE_RD       "rt=core.rd"
#define EM_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CLUSTER_ID    "rt=urn:zcl"
#define EM_ZCL_URI_QUERY_PREFIX_VERSION                     "if=urn:zcl:"
#define EM_ZCL_URI_QUERY_PROTOCOL_REVISION_FORMAT           "if=urn:zcl:v%x"
#define EM_ZCL_URI_QUERY_CLUSTER_REVISION_FORMAT            "if=urn:zcl:c.v%x"
#define EM_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT    "ze=urn:zcl:"
#define EM_ZCL_URI_QUERY_POSTFIX_DEVICE_ID                  "d."
#define EM_ZCL_URI_QUERY_DOT                                '.'
#define EM_ZCL_URI_QUERY_WILDCARD                           '*'
#define EM_ZCL_URI_QUERY_VERSION_KEY                        "c.v"
#define EM_ZCL_URI_WELL_KNOWN                               ".well-known"
#define EM_ZCL_URI_CORE                                     "core"
#define EM_ZCL_URI_WELL_KNOWN_CORE                          ".well-known/core"
#define EM_ZCL_URI_RESPONSE_DELIMITER                       ";"
#define EM_ZCL_URI_WELL_KNOWN_CORE_PAYLOAD                  "if=urn:zcl:v0;rt=urn:zcl"
#define EM_ZCL_URI_DEVICE_UID_APPEND_MAX_LEN                (104) // Includes prefix, uid and postfix chars.

#define EM_ZCL_URI_METADATA_QUERY                           "meta="
#define EM_ZCL_URI_METADATA_BASE                            "$base"
#define EM_ZCL_URI_METADATA_ACCESS                          "$acc"
#define EM_ZCL_URI_METADATA_WILDCARD                        "*"

#define EM_ZCL_MAX_WELL_KNOWN_REPLY_PAYLOAD (800)
//TODO- Buffer size increased (was 700) to pass zcl-test-group-2.bsh script- at
// the moment discovery responses (link-format+cbor) return an empty payload
// with an error status if the payload exceeds the buffer length- it would be
// better to Truncate the response at the buffer limit so the user gets a useful
// discovery reply (truncation should apply before limit is reached so that
// the response format is still valid).

#define isMulticastAddress(ipAddress) ((ipAddress)[0] == 0xFF)

uint16_t emZclUriAppendUriPath(char *finger,
                               char *endOfBuffer,
                               EmberZclEndpointId_t endpointId,
                               const EmberZclClusterSpec_t *clusterSpec);
bool emZclUriBreak(char *finger);
bool emZclDiscByUidString(const uint8_t *uidString, EmberCoapResponseHandler responseHandler);
bool emZclDiscSetAccept(EmberCoapContentFormatType accept);

//----------------------------------------------------------------
// Discovery Payload Context
//
// Supports generic generation CoRE link payloads with selectable
// Content-Format (link-format or link-format+cbor). Member function
// pointers are initialized with implementations for the selected
// format.

typedef enum {
  DISCOVERY_PAYLOAD_CONTEXT_STATUS_SUCCESS,
  DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL,
  DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW
} EmZclDiscPayloadContextStatus;

typedef struct EmZclDiscPayloadContext_s EmZclDiscPayloadContext_t;
struct EmZclDiscPayloadContext_s {
  EmberCoapContentFormatType contentFormat;
  EmZclDiscPayloadContextStatus status;
  CborState cborState; // reuse start/finger/end pointers for non-CBOR buffer implementation
  uint8_t linkCount;
  uint8_t attrCount;

  bool (*payloadIsEmpty)(EmZclDiscPayloadContext_t *dpc);
  const uint8_t *(*payloadPointer)(EmZclDiscPayloadContext_t *dpc);
  uint16_t (*payloadLength)(EmZclDiscPayloadContext_t *dpc);
  void (*startPayload)(EmZclDiscPayloadContext_t *dpc);
  void (*startLink)(EmZclDiscPayloadContext_t *dpc);
  void (*addResourceUri)(EmZclDiscPayloadContext_t *dpc,
                         EmberZclEndpointId_t endpointId,
                         const EmberZclClusterSpec_t *clusterSpec);
  void (*addRt)(EmZclDiscPayloadContext_t *dpc,
                const EmberZclClusterSpec_t *spec,
                bool includeTrailingClusterTag);
  void (*addIf)(EmZclDiscPayloadContext_t *dpc,
                EmberZclEndpointId_t endpointId,
                const EmberZclClusterSpec_t *clusterSpec);
  void (*addEp)(EmZclDiscPayloadContext_t *dpc);
  void (*addZe)(EmZclDiscPayloadContext_t *dpc,
                EmberZclEndpointId_t endpointId,
                EmberZclDeviceId_t deviceId);
  void (*endLink)(EmZclDiscPayloadContext_t *dpc);
  void (*endPayload)(EmZclDiscPayloadContext_t *dpc);
  void (*appendUid)(EmZclDiscPayloadContext_t *dpc);
};

bool emZclInitDiscPayloadContext(EmZclDiscPayloadContext_t *dpc,
                                 EmberCoapContentFormatType contentFormat,
                                 uint8_t *buffer,
                                 uint16_t bufferLength);

#endif // #ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @} end addtogroup ZCLIP_discovery */
/** @} end addtogroup ZCLIP */

#endif // #ifndef ZCL_CORE_WELL_KNOWN_H
