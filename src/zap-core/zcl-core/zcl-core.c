/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include "stack/ip/ip-address.h"
#include "stack/ip/tls/tls.h"
#include "stack/ip/tls/tls-sha256.h"
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include EMBER_AF_API_ZCL_CORE_RESOURCE_DIRECTORY
#include EMBER_AF_API_ZCL_CORE_DTLS_MANAGER

void emberZclGetPublicKeyCallback(const uint8_t **publicKey,
                                  uint16_t *publicKeySize);
EmberZclUid_t emZclUid = { { 0 } };

// -----------------------------------------------------------------------------
// Constants.

#define RFC3306_NETWORK_PREFIX_OFFSET 4

static uint16_t fetchCoapOptions(EmberCoapReadOptions *options,
                                 EmberCoapOptionType wantType,
                                 const uint8_t **valuePointers,
                                 uint16_t *valueLengths,
                                 uint16_t maxOptions);
static bool uriLookup(EmZclContext_t *context, EmZclUriPath *paths);
static bool processQueries(EmZclContext_t *context, EmZclUriQuery *queries);
static bool isValidFormatOption(EmZclContext_t *context,
                                EmberCoapOptionType optionType,
                                uint8_t flags);

#define isValidAcceptOption(context, flags) \
  isValidFormatOption((context), EMBER_COAP_OPTION_ACCEPT, (flags))

// Coap content-format option check result is always true for GET or DELETE
// methods because there is no associated request payload to check.
#define isValidContentFormatOption(context, flags) \
  ((context->code == EMBER_COAP_CODE_GET)          \
   || (context->code == EMBER_COAP_CODE_DELETE)    \
   || isValidFormatOption((context), EMBER_COAP_OPTION_CONTENT_FORMAT, (flags)))

// URI handlers
static void zclHandler(EmZclContext_t *context);
static void clusterHandler(EmZclContext_t *context);
static bool hexSegmentToInt(const EmZclContext_t *context,
                            uint8_t depth,
                            size_t size,
                            uintmax_t *result);

static bool zclParseUriLinkFormat(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, EmZclUriContext_t *context);
static bool zclParseUriLinkFormatPlusCbor(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, EmZclUriContext_t *context);

static EmberZclStatus_t isAccessAllowed(EmZclContext_t *context, void *data);
static bool dropIncomingMulticast(const EmZclContext_t *context, const uint8_t *uri);

typedef struct {
  EmberZclStatus_t status;
} DefaultResponseError;

static const ZclipStructSpec defaultResponseErrorSpec[] = {
  #define EMBER_ZCLIP_STRUCT DefaultResponseError
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT), 1, NULL),
  EMBER_ZCLIP_FIELD_NAMED(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, status, "s"),
  #undef EMBER_ZCLIP_STRUCT
};

//----------------------------------------------------------------
// URI path data
//
// First two numbers are:
//
//  - Entries to skip if the match succeeds but there are more segments.
//       0 -> handler will parse additional segments on its own
//       1 -> continue with next entry to parse next segment
//     255 -> no additional segments are allowed (e.g. .../n)
//    Any other value is used to jump to a shared sub-URI, in this
//    case the cluster ID matching, which has two different possible
//    prefixes.
//
//  - Entries to skip if the match fails.
//    Jump forward this many entries to get to the next URI at the same
//    depth (e.g. from zcl/e to zcl/g).  Putting simpler URIs earlier
//    makes it easier to keep track of these.
//
// GET means that only a GET request is allowed.
//
// Configure CBOR or LINK format flag for URI for automated coarse checking
// of Accept and Content-Format option values in CoAP request; or, leave
// format unconfigured if more complicated than this coarse check, and
// instead perform option value checking in URI handler.

#define M(x) EM_ZCL_URI_FLAG_METHOD_ ## x
#define GXXX (M(GET))
#define XPXX (M(POST))
#define GPXX (M(GET) | M(POST))
#define GXPX (M(GET) | M(PUT))
#define GXPD (M(GET) | M(PUT) | M(DELETE))

#define F(x) EM_ZCL_URI_FLAG_FORMAT_ ## x
#define FCBOR (F(CBOR))
#define FLINK (F(LINK))

static bool uriManagementCommandIdMatch(EmZclContext_t *context, void *data, uint8_t depth);
static bool uriEndpointIdMatch         (EmZclContext_t *context, void *data, uint8_t depth);
static bool uriGroupIdMatch            (EmZclContext_t *context, void *data, uint8_t depth);
static bool uriClusterIdMatch          (EmZclContext_t *context, void *data, uint8_t depth);
static bool uriAttributeIdMatch        (EmZclContext_t *context, void *data, uint8_t depth);
static bool uriBindingIdMatch          (EmZclContext_t *context, void *data, uint8_t depth);
static bool uriClusterCommandIdMatch   (EmZclContext_t *context, void *data, uint8_t depth);
static bool uriReportingIdMatch        (EmZclContext_t *context, void *data, uint8_t depth);

static EmZclUriQuery attributeUriQueries[] = {
  // zcl/[eg]/XX/<cluster>/a?f=1,2+3,4-5,...
  { emZclUriQueryStringPrefixMatch, "f=", emZclAttributeUriQueryFilterParse },
  // zcl/[eg]/XX/<cluster>/a?u
  { emZclUriQueryStringPrefixMatch, "u", emZclAttributeUriQueryUndividedParse },
  // zcl/[eg]/XX/<cluster>/a?meta=  (==metadata query)
  { emZclUriQueryStringPrefixMatch, "meta=", emZclAttributeUriMetadataQueryParse },

  // terminator
  { NULL, NULL, NULL },
};

static EmZclUriPath zclUriPaths[] = {
  // zcl
  {   1, 255, GXXX | FCBOR, emZclUriPathStringMatch, "zcl", NULL, zclHandler },

  // zcl/t
  { 255, 1, XPXX | FCBOR, emZclUriPathStringMatch, "t", NULL, emZclAccessTokenHandler },

  // zcl/m
  {   1, 3, GXXX | FCBOR, emZclUriPathStringMatch, "m", NULL, emZclManagementHandler },
  {   1, 255, GXXX | FCBOR, emZclUriPathStringMatch, "c", NULL, emZclManagementCommandHandler },
  { 255, 255, XPXX | FLINK, uriManagementCommandIdMatch, NULL, NULL, emZclManagementCommandIdHandler },

  // zcl/e
  {   1, 2, GXXX | FCBOR, emZclUriPathStringMatch, "e", NULL, emZclUriEndpointHandler },
  // zcl/e/XX
  {   3, 255, GXXX | FCBOR, uriEndpointIdMatch, NULL, NULL, emZclUriEndpointIdHandler },

  // zcl/g
  {   1, 255, GXXX | FCBOR, emZclUriPathStringMatch, "g", NULL, emZclUriGroupHandler },
  // zcl/g/XXXX
  {   1, 255, GXXX | FCBOR, uriGroupIdMatch, NULL, NULL, emZclUriGroupIdHandler },

  // cluster ID, after either zcl/e/XX or zcl/g/XXXX
  {   1, 255, GXXX | FCBOR, uriClusterIdMatch, NULL, NULL, clusterHandler },

  // zcl/[eg]/XX/<cluster>/a:
  {   1, 3, GPXX | FCBOR, emZclUriPathStringMatch, "a", attributeUriQueries, emZclUriClusterAttributeHandler },
  // zcl/[eg]/XX/<cluster>/a/XXXX:
  {  255, 1, GXPX | FCBOR, uriAttributeIdMatch, NULL, NULL, emZclUriClusterAttributeIdHandler },
  // zcl/[eg]/XX/<cluster>/a/XXXX/$xxxx:  ($xxxx==attribute metadata spec)
  { 255, 255, GXXX | FCBOR, uriAttributeIdMatch, "$", NULL, emZclUriClusterAttributeIdHandler },

  // zcl/[eg]/XX/<cluster>/b:
  {   1, 2, GPXX | FCBOR, emZclUriPathStringMatch, "b", NULL, emZclUriClusterBindingHandler },
  // zcl/[eg]/XX/<cluster>/b/XX:
  { 255, 255, GXPD | FCBOR, uriBindingIdMatch, NULL, NULL, emZclUriClusterBindingIdHandler },

  // zcl/[eg]/XX/<cluster>/c:
  {   1, 2, GXXX | FCBOR, emZclUriPathStringMatch, "c", NULL, emZclUriClusterCommandHandler },
  // zcl/[eg]/XX/<cluster>/c/XX:
  { 255, 255, XPXX | FCBOR, uriClusterCommandIdMatch, NULL, NULL, emZclUriClusterCommandIdHandler },

  // zcl/[eg]/XX/<cluster>/n:
  {   0, 1, XPXX | FCBOR, emZclUriPathStringMatch, "n", NULL, emZclUriClusterNotificationHandler },

  // zcl/[eg]/XX/<cluster>/r:
  {   1, 255, GPXX | FCBOR, emZclUriPathStringMatch, "r", NULL, emZclUriClusterReportingConfigurationHandler },
  // zcl/[eg]/XX/<cluster>/r/XX:
  { 255, 255, GXPD | FCBOR, uriReportingIdMatch, NULL, NULL, emZclUriClusterReportingConfigurationIdHandler },
};

void emZclInitHandler(void)
{
  const uint8_t *publicKey = NULL;
  uint16_t publicKeySize = 0;
  emberZclGetPublicKeyCallback(&publicKey, &publicKeySize);

  // TODO: Every device should have a UID, so we should assert if we don't have
  // the public key to generate it.  Most applications don't have public keys
  // yet though, so this isn't doable right now.
  //assert(publicKey != NULL && publicKeySize != 0);

  Sha256State state;
  emSha256Start(&state);
  emSha256HashBytes(&state, (const uint8_t *)"zcl.uid", 7);
  emSha256HashBytes(&state, publicKey, publicKeySize);
  emSha256Finish(&state, emZclUid.bytes);

  // Re-register to listen to all group multicast IPv6 addresses
  emZclReregisterAllMcastAddresses();

  emberUdpListen(EMBER_COAP_PORT, emFf05AllCoapNodesMulticastAddress.contents); // listen on ff05::fd
}

void emZclHandler(EmberCoapCode code,
                  uint8_t *uri,
                  EmberCoapReadOptions *options,
                  const uint8_t *payload,
                  uint16_t payloadLength,
                  const EmberCoapRequestInfo *info)
{
  EmZclContext_t context;
  MEMSET(&context, 0, sizeof(EmZclContext_t));
  context.code = code;
  context.options = options;
  context.payload = payload;
  context.payloadLength = payloadLength;
  context.info = info;
  context.groupId = EMBER_ZCL_GROUP_NULL;
  context.uriPathSegments = fetchCoapOptions(context.options,
                                             EMBER_COAP_OPTION_URI_PATH,
                                             context.uriPath,
                                             context.uriPathLength,
                                             MAX_URI_PATH_SEGMENTS);

  // EZ-mode command: set cluster ID to EMBER_ZCL_CLUSTER_NULL so that emZclIsProtectedResource()
  // does not disallow this as EMBER_ZCL_CLUSTER_BASIC (id 0)
  // TODO: Rework the access control implementation so that it does not require such workarounds
  if ((uri != NULL) && (strncmp((char *)uri, "zcl/m/c/", strlen("zcl/m/c/")) == 0)) {
    context.clusterSpec.id = EMBER_ZCL_CLUSTER_NULL;
  }

  // BDB Spec 2.8.6.2: Some multicast requests must be dropped
  if (dropIncomingMulticast(&context, uri)) {
    return;
  }

  // simPrint("received %d segments", context.uriPathSegments);
  // All ZCL/IP URIs start with zcl or .well-known, and
  // that is all we handle.
  if (context.uriPathSegments == 0
      || MAX_URI_PATH_SEGMENTS < context.uriPathSegments
      || !(uriLookup(&context, zclUriPaths)
           || uriLookup(&context, emZclWellKnownUriPaths)
           || uriLookup(&context, emZclRdUriPaths)
           )
      ) {
    emZclRespond404NotFound(info);
  }
}

static uint16_t fetchCoapOptions(EmberCoapReadOptions *options,
                                 EmberCoapOptionType wantType,
                                 const uint8_t **valuePointers,
                                 uint16_t *valueLengths,
                                 uint16_t maxOptions)
{
  uint16_t count = 0;
  emberResetReadOptionPointer(options);
  while (true) {
    const uint8_t *valuePointer;
    uint16_t valueLength;
    EmberCoapOptionType type = emberReadNextOption(options,
                                                   &valuePointer,
                                                   &valueLength);
    if (type == EMBER_COAP_NO_OPTION) {
      break;
    } else if (wantType == type) {
      *valuePointers++ = valuePointer;
      *valueLengths++ = valueLength;
      count++;
      if (maxOptions <= count) {
        break;
      }
    }
  }
  return count;
}

static bool uriLookup(EmZclContext_t *context, EmZclUriPath *paths)
{
  uint8_t segmentsMatched = 0;

  while (true) {
    uint8_t skip = paths->failSkip;

    if (paths->match(context, paths->data, segmentsMatched)) {
      EmberZclStatus_t status = isAccessAllowed(context, paths->data);
      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS: {
          segmentsMatched += 1;
          if (segmentsMatched == context->uriPathSegments) {
            if (context->code < EMBER_COAP_CODE_GET
                || EMBER_COAP_CODE_DELETE < context->code
                || (context->code == EMBER_COAP_CODE_GET
                    && !READBITS(paths->flags, EM_ZCL_URI_FLAG_METHOD_GET))
                || (context->code == EMBER_COAP_CODE_POST
                    && !READBITS(paths->flags, EM_ZCL_URI_FLAG_METHOD_POST))
                || (context->code == EMBER_COAP_CODE_PUT
                    && !READBITS(paths->flags, EM_ZCL_URI_FLAG_METHOD_PUT))
                || (context->code == EMBER_COAP_CODE_DELETE
                    && !READBITS(paths->flags, EM_ZCL_URI_FLAG_METHOD_DELETE))) {
              emZclRespond405MethodNotAllowed(context->info);
            } else if (paths->action != NULL) {
              if (paths->queries != NULL
                  && !processQueries(context, paths->queries)) {
                if (context->code != EMBER_COAP_CODE_GET) {
                  emZclRespond405MethodNotAllowed(context->info); // only GET access allowed for queries.
                } else {
                  emZclRespond400BadRequest(context->info); // bad query format.
                }
              } else if (!isValidAcceptOption(context, paths->flags)) {
                emZclRespond406NotAcceptable(context->info);
              } else if (!isValidContentFormatOption(context, paths->flags)) {
                emZclRespond415UnsupportedContentFormat(context->info);
              } else {
                paths->action(context);
              }
            }
            return true;
          } else {
            skip = paths->matchSkip;
          }
          break;
        }
        case EMBER_ZCL_STATUS_NOT_AUTHORIZED:
          emZclRespond401Unauthorized(context->info);
          return true;
        case EMBER_ZCL_STATUS_FORBIDDEN:
          emberZclRespondWithCode(context->info, EMBER_COAP_CODE_403_FORBIDDEN);
          return true;
        case EMBER_ZCL_STATUS_FAILURE:
          emZclRespond400BadRequest(context->info);
          return true;
        default:
          emZclRespond500InternalServerError(context->info);
          return true;
      } // end switch
    } // end if

    if (skip == 255) {
      break;         // ran out of URIs to try
    } else {
      paths += skip; // try next URI
    }
  }

  return false;
}

static EmberZclStatus_t isAccessAllowed(EmZclContext_t *context, void *data)
{
  // we only care about single characters "a", "c", "n", "r" etc.
  if (data == NULL || ((uint8_t *) data)[1] != '\0') {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  uint8_t accessType = ((uint8_t *) data)[0]; // fetch character

  if (!emZclIsProtectedResource(&context->clusterSpec, accessType)) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  EmberZclDeviceId_t deviceId = (context->endpoint == NULL
                                 ? EMBER_ZCL_DEVICE_ID_NULL
                                 : context->endpoint->deviceId);
  uint8_t sessionId = (uint8_t)(unsigned long) context->info->transmitHandlerData;

  // check if we already have access or if we are using an incorrect DTLS mode
  EmberZclStatus_t status
    = emZclAllowRemoteAccess(sessionId,
                             &context->clusterSpec,
                             deviceId,
                             accessType);

  // continue if unauthorized, because the payload could contain the token providing access
  if (status != EMBER_ZCL_STATUS_NOT_AUTHORIZED && status != EMBER_ZCL_STATUS_FORBIDDEN) {
    return status;
  }

  // look for {"at": "access_token"} in payload
  if (context->payloadLength) {
    CborState state;
    emCborDecodeStart(&state, context->payload, context->payloadLength);
    if (emCborDecodeMap(&state)) {
      uint8_t *keyString;
      uint32_t keyStringLength;
      while (!emCborPeekSequenceEnd(&state)) { // iterate through all pairs
        if (!emCborDecodeItem(&state, CBOR_TEXT, &keyString, &keyStringLength)) {
          emCborDecodeSkipValue(&state); // go to next pair if key is not a string
          continue;
        }

        if ((keyStringLength == 2)
            && strncmp("at", (const char *) keyString, 2) == 0) {
          EmberZclUid_t uid;
          if (emberZclDtlsManagerGetUidBySessionId(sessionId,
                                                   &uid) != EMBER_SUCCESS) {
            return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
          }

          uint16_t binaryTokenSize = EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN;
          uint8_t binaryToken[EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN];
          if (!emZclExtractBinaryAccessToken(&state,
                                             binaryToken,
                                             &binaryTokenSize)) {
            return EMBER_ZCL_STATUS_FAILURE;
          }

          if (emZclAddIncomingToken(&uid,
                                    binaryToken,
                                    binaryTokenSize)) {
            // new access token added successfully, so check access again
            return emZclAllowRemoteAccess(sessionId,
                                          &context->clusterSpec,
                                          deviceId,
                                          accessType);
          } else {
            return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
          }
        } else {
          emCborDecodeSkipValue(&state); // unknown key, so ignore the value
        } // "at"
      } // while
    } // if
  }

  return status;
}

static bool processQueries(EmZclContext_t *context, EmZclUriQuery *queries)
{
  context->uriQuerySegments = fetchCoapOptions(context->options,
                                               EMBER_COAP_OPTION_URI_QUERY,
                                               context->uriQuery,
                                               context->uriQueryLength,
                                               MAX_URI_PATH_SEGMENTS);

  // If we see a query that we don't know about, then just ignore it for the
  // sake of future compatibility.
  for (size_t segment = 0; segment < context->uriQuerySegments; segment++) {
    for (EmZclUriQuery *query = queries; query->match != NULL; query++) {
      if (query->match(context, query->data, segment)
          && !query->parse(context, query->data, segment)) {
        return false;
      }
    }
  }

  // If there is an active query filter after query has been parsed then
  // we should check that access type is GET.
  if ((context->attributeQuery.filterCount != 0)
      && (context->code != EMBER_COAP_CODE_GET)) {
    return false;
  }

  return true;
}

static bool isValidFormatOption(EmZclContext_t *context,
                                EmberCoapOptionType optionType,
                                uint8_t flags)
{
  const uint8_t *valuePointer;
  uint16_t valueLength;
  uint16_t count = fetchCoapOptions(context->options,
                                    optionType,
                                    &valuePointer,
                                    &valueLength,
                                    1);
  if (count != 0 && READBITS(flags, EM_ZCL_URI_FLAG_FORMAT_MASK)) {
    uint16_t value = valuePointer[0];
    if (valueLength == 2) {
      value = (value << 8) | valuePointer[1];
    }
    EmberCoapContentFormatType cf = (EmberCoapContentFormatType)(value);
    if (READBITS(flags, EM_ZCL_URI_FLAG_FORMAT_CBOR)) {
      return (cf == EMBER_COAP_CONTENT_FORMAT_CBOR);
    } else if (READBITS(flags, EM_ZCL_URI_FLAG_FORMAT_LINK)) {
      return (cf == EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR
              || cf == EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT);
    } else {
      return false;
    }
  }
  // Option absent, or no format flag bits configured.
  return true;
}

//----------------------------------------------------------------
// URI segment matching functions

static bool stringMatch(const uint8_t *bytes,
                        uint8_t bytesLength,
                        void *castString,
                        bool matchBytesLength)
{
  char *string = (char *) castString;
  const uint16_t length = strlen(string);

  return ((bytesLength == length || !matchBytesLength)
          && MEMCOMPARE(bytes, string, length) == 0);
}

bool emZclUriPathStringMatch(EmZclContext_t *context,
                             void *castString,
                             uint8_t depth)
{
  // char temp[100], *string = (char *) castString;
  // MEMCOPY(temp, context->uriPath[depth], context->uriPathLength[depth]);
  // temp[context->uriPathLength[depth]] = 0;
  // fprintf(stderr, "[want %s have %s]\n", string, temp);
  return stringMatch(context->uriPath[depth],
                     context->uriPathLength[depth],
                     castString,
                     true); // match bytes length
}

bool emZclUriQueryStringPrefixMatch(EmZclContext_t *context,
                                    void *castString,
                                    uint8_t depth)
{
  return stringMatch(context->uriQuery[depth],
                     context->uriQueryLength[depth],
                     castString,
                     false); // match string length
}

static bool uriManagementCommandIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t commandId;
  if (hexSegmentToInt(context, depth, sizeof(context->command->commandId), &commandId)) {
    context->command = emZclManagementFindCommand((EmberZclCommandId_t)commandId);
    return context->command != NULL;
  } else {
    return false;
  }
}

static bool uriEndpointIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t endpointId;
  if (hexSegmentToInt(context, depth, sizeof(context->endpoint->endpointId), &endpointId)) {
    context->endpoint = emZclFindEndpoint((EmberZclEndpointId_t)endpointId);
    return (context->endpoint != NULL);
  } else {
    return false;
  }
}

static bool uriGroupIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t groupId;
  if (hexSegmentToInt(context, depth, sizeof(context->groupId), &groupId)
      && emZclHasGroup((EmberZclGroupId_t)groupId)) {
    context->groupId = groupId;
    return true;
  } else {
    return false;
  }
}

static bool uriClusterIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  if (!emZclStringToCluster(context->uriPath[depth],
                            context->uriPathLength[depth],
                            &context->clusterSpec)) {
    return false;
  }

  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    size_t i;
    for (i = 0; i < emZclEndpointCount; i++) {
      if (emberZclIsEndpointInGroup(emZclEndpointTable[i].endpointId,
                                    context->groupId)
          && emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                     &context->clusterSpec)) {
        return true;
      }
    }
    return false;
  } else {
    return emZclEndpointHasCluster(context->endpoint->endpointId,
                                   &context->clusterSpec);
  }
}

static bool uriAttributeIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t attributeId;
  if (hexSegmentToInt(context, depth, sizeof(context->attribute->attributeId), &attributeId)) {
    context->attribute = emZclFindAttribute(&context->clusterSpec,
                                            (EmberZclAttributeId_t)attributeId,
                                            false); // exclude remote

    if (context->attribute != NULL) {
      // Found attribute, now check if we also need to match on uri
      // metadata item string: a/XXXX/$yyyy:
      // where $yyyy = the metadata item string, e.g. $base, $acc
      context->attributeQuery.metadata = EM_ZCL_METADATA_NONE;
      char *finger = (char *)context->uriPath[depth] + context->uriPathLength[depth] + 1;
      char *metaUriStr = (finger[0] == '$') ? finger : NULL;
      bool hasMetaUri = (metaUriStr != NULL);
      bool wantMetaUri = (data != NULL);
      if (hasMetaUri != wantMetaUri) {
        return false;
      }
      if (!wantMetaUri) {
        return true;
      }

      // Check for one of the supported metadata types.
      // Note- metadata wildcard "*" is not allowed here.
      if (MEMCOMPARE(metaUriStr, EM_ZCL_URI_METADATA_BASE, strlen(EM_ZCL_URI_METADATA_BASE)) == 0) {
        context->attributeQuery.metadata = EM_ZCL_METADATA_BASE;
        return true;
      } else if (MEMCOMPARE(metaUriStr, EM_ZCL_URI_METADATA_ACCESS, strlen(EM_ZCL_URI_METADATA_ACCESS)) == 0) {
        context->attributeQuery.metadata = EM_ZCL_METADATA_ACCESS;
        return true;
      } else {
        context->attributeQuery.metadata = EM_ZCL_METADATA_NOT_SUPPORTED;
      }
      //TODO- Add support for other metadata types here.
    }
  }

  return false;
}

static bool uriBindingIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t bindingId;
  if (hexSegmentToInt(context, depth, sizeof(context->bindingId), &bindingId)
      && emZclHasBinding(context, (EmberZclBindingId_t)bindingId)) {
    context->bindingId = bindingId;
    return true;
  } else {
    return false;
  }
}

static bool uriClusterCommandIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t commandId;
  if (hexSegmentToInt(context, depth, sizeof(context->command->commandId), &commandId)) {
    context->command = emZclFindCommand(&context->clusterSpec, (EmberZclCommandId_t)commandId);
    return (context->command != NULL);
  } else {
    return false;
  }
}

static bool uriReportingIdMatch(EmZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t reportingConfigurationId;
  if (hexSegmentToInt(context, depth, sizeof(context->reportingConfigurationId), &reportingConfigurationId)
      && emZclHasReportingConfiguration(context->endpoint->endpointId,
                                        &context->clusterSpec,
                                        (EmberZclReportingConfigurationId_t)reportingConfigurationId)) {
    context->reportingConfigurationId = reportingConfigurationId;
    return true;
  } else {
    return false;
  }
}

static bool zclParseUriLinkFormat(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, EmZclUriContext_t *context)
{
  // Parses link-format discovery response "</zcl/e/EE/[cs]CCCC>"
  // into EmZclUriContext_t struct.
  // returns- true : zcl/e/ tag was found, 'incoming' ptr is updated.
  //        - false: zcl/e/ tag was not found.

  char zclTag[] = "</zcl/e/";
  uint8_t zclTagLen = strlen(zclTag);
  char *finger = (char *)*incoming;
  char *tagStart = NULL;

  // Decode from the present payload position until we find a zcl tag.
  for (char *p = finger; p < ((char *)payload + payloadLength - zclTagLen); ) {
    if (MEMCOMPARE(p, zclTag, zclTagLen) == 0) {
      tagStart = p;
      break;
    } else {
      ++p;
    }
  }

  if (tagStart != NULL) {
    finger = tagStart + zclTagLen;

    // parsing endpoint number.
    uintmax_t endpointId;
    char *delimiter;
    if (((delimiter = strchr(finger, '/')) != NULL)
        && (emZclHexStringToInt((const uint8_t *)finger,
                                delimiter - finger,
                                &endpointId))) {
      finger = delimiter;
      finger++;

      // parsing cluster number.
      EmberZclClusterSpec_t clusterSpec;
      char *endBracket;
      if (((endBracket = strchr(finger, '>')) != NULL)
          && (emZclStringToCluster((const uint8_t *)finger,
                                   endBracket - finger,
                                   &clusterSpec))) {
        context->endpointId = endpointId;
        MEMCOPY(context->clusterSpec, &clusterSpec, sizeof(EmberZclClusterSpec_t));
        *incoming = (uint8_t *)endBracket + 1; // update ptr for next call.
        return true;
      }
    }
  }

  return false;
}

static bool zclParseUriLinkFormatPlusCbor(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, EmZclUriContext_t *context)
{
  // Parses link-format+cbor discovery response "[{1:"/zcl/e/EE/[cs]CCCC",}, {1:"/zcl/e/EE/[cs]CCCC",}]"
  // into EmZclUriContext_t struct. The payload may contain a number of maps
  // (one for each cluster), we always decode from the start of the payload in
  // order to verify that the initial cbor Array is correct and then we jump to
  // the last decoded payload position (as indicated by 'incoming' ptr).
  // returns- true : zcl/e/ tag was found, 'incoming' ptr is updated.
  //        - false: zcl/e/ tag was not found.

  char zclTag[] = "/zcl/e/";
  size_t zclTagLen = strlen(zclTag);
  char zclUri[20] = { 0 };
  bool foundZclTag = false;
  CborState state;

  emCborDecodeStart(&state, payload, payloadLength); // Always start decode from start of payload.
  if (emCborDecodeArray(&state)) { // The response payload must start with a cbor Array.
    if (*incoming > payload) {
      state.finger = *incoming; // Jump forward to last decoded position.
    }

    // Decode until we find a map containing a zcl tag.
    while (!foundZclTag) {
      // Loop for all maps in the array.
      uint8_t type = emCborDecodePeek(&state, NULL);
      if (type == CBOR_MAP) {
        if (!emCborDecodeMap(&state)) {
          break;
        }
        // Loop until the map is completely decoded.
        while (1) {
          uint8_t type = emCborDecodePeek(&state, NULL);
          if (type == CBOR_UNSIGNED) {
            uint16_t keyValue;
            keyValue = emCborDecodeKey(&state);
            if (keyValue == 1) {
              if (!emCborDecodeValue(&state,
                                     EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                                     sizeof(zclUri),
                                     (uint8_t *)zclUri)) {
                break;
              }
              if (MEMCOMPARE(zclUri, zclTag, zclTagLen) == 0) {
                foundZclTag = true;  //Note: Don't break here, continue to decode the end of the map.
              }
            } else if (!emCborDecodeSkipValue(&state)) { // Skip value.
              break;
            }
          } else if (!emCborDecodeSkipValue(&state)
                     || !emCborDecodeSkipValue(&state)) { // Skip key and value.
            break;
          }
        } //while (inside map)
      } else if (!emCborDecodeSkipValue(&state)) {
        break;
      }
    } // while

    if (foundZclTag) {
      char *finger = zclUri;
      uint16_t zclUriLen = strlen(zclUri);
      finger += zclTagLen;

      // parsing endpoint number.
      uintmax_t endpointId;
      char *delimiter;
      if (((delimiter = strchr(finger, '/')) != NULL)
          && (emZclHexStringToInt((const uint8_t *)finger,
                                  delimiter - finger,
                                  &endpointId))) {
        finger = delimiter;
        finger++;

        // parsing cluster number.
        EmberZclClusterSpec_t clusterSpec;
        if (emZclStringToCluster((const uint8_t *)finger,
                                 zclUriLen - (finger - zclUri), // cluster string length.
                                 &clusterSpec)) {
          context->endpointId = endpointId;
          MEMCOPY(context->clusterSpec, &clusterSpec, sizeof(EmberZclClusterSpec_t));
          *incoming = state.finger;  // update ptr for next call.
          return true;
        }
      }
    }
  }

  return false;
}

//----------------------------------------------------------------
// URI handlers

// get zcl:
static const uint8_t egmtPayload[] = { // ["e", "g", "m", "t"]
  0x84, 0x61, 0x65, 0x61, 0x67, 0x61, 0x6D, 0x61, 0x74
};
static void zclHandler(EmZclContext_t *context)
{
  emZclRespond205ContentCbor(context->info, egmtPayload, sizeof(egmtPayload));
}

// GET zcl/e/XX/[cs]XXXX:
// GET zcl/e/XX/[cs]XXXXXXXX:
// GET zcl/g/XXXX/[cs]XXXX:
// GET zcl/g/XXXX/[cs]XXXXXXXX:
static const uint8_t abcnrPayload[] = { // ["a", "b", "c", "n", "r"]
  0x85, 0x61, 0x61, 0x61, 0x62, 0x61, 0x63, 0x61, 0x6e, 0x61, 0x72,
};
static void clusterHandler(EmZclContext_t *context)
{
  if (context->groupId == EMBER_ZCL_GROUP_NULL) {
    // Only supported for single endpoint URI...
    emZclRespond205ContentCbor(context->info, abcnrPayload, sizeof(abcnrPayload));
  } else {
    // ...not for group URI.
    emZclRespond404NotFound(context->info);
  }
}

static bool hexSegmentToInt(const EmZclContext_t *context,
                            uint8_t depth,
                            size_t size,
                            uintmax_t *result)
{
  return (context->uriPathLength[depth] <= size * 2 // bytes to nibbles
          && emZclHexStringToInt(context->uriPath[depth],
                                 context->uriPathLength[depth],
                                 result));
}

int32_t emberZclCompareClusterSpec(const EmberZclClusterSpec_t *s1,
                                   const EmberZclClusterSpec_t *s2)
{
  if (s1->role != s2->role) {
    return (s1->role == EMBER_ZCL_ROLE_CLIENT ? -1 : 1);
  }

  int32_t manufacturerCodeDiff = s1->manufacturerCode - s2->manufacturerCode;
  if (manufacturerCodeDiff != 0) {
    return manufacturerCodeDiff;
  }

  int32_t idDiff = s1->id - s2->id;
  if (idDiff != 0) {
    return idDiff;
  }

  return 0;
}

bool emberZclAreClusterSpecsEqual(const EmberZclClusterSpec_t *s1,
                                  const EmberZclClusterSpec_t *s2)
{
  return (emberZclCompareClusterSpec(s1, s2) == 0);
}

void emberZclReverseClusterSpec(const EmberZclClusterSpec_t *s1,
                                EmberZclClusterSpec_t *s2)
{
  s2->role = (s1->role == EMBER_ZCL_ROLE_CLIENT
              ? EMBER_ZCL_ROLE_SERVER
              : EMBER_ZCL_ROLE_CLIENT);
  s2->manufacturerCode = s1->manufacturerCode;
  s2->id = s1->id;
}

// returns true if we should suppress response, false otherwise
// based on spec 16-07008-047: 2.8.6.2 Handling of Multicast Request Message
static bool suppressResponse(const EmberCoapRequestInfo *info,
                             EmberCoapCode code,
                             const uint8_t *payload,
                             uint16_t payloadLength)
{
  // do not suppress response for unicast
  if (!emIsMulticastAddress(info->localAddress.bytes)) {
    return false;
  }

  // suppress multicast response for 4.xx or 5.xx error status
  if (emberCoapIsClientErrorResponse(code)
      || emberCoapIsServerErrorResponse(code)) {
    return true;
  }

  assert(emberCoapIsSuccessResponse(code)); // should not get here unless 2.xx

  if (payloadLength == 0
      || payload == NULL
      || payload[0] == 0x80 // empty array
      || payload[0] == 0xA0) { // empty map
    // TODO: we assume above that if payload present, it is CBOR format.
    //       do we support other payload formats as multicast responses?
    // TODO: if the payload is a map of one or more entries, and the value of
    //       every entry either is empty or is the Default Response?
    return true;
  }

  return false;
}

EmberStatus emZclRespondWithCode(const EmberCoapRequestInfo *info,
                                 EmberCoapCode code)
{
  if (suppressResponse(info, code, NULL, 0)) {
    return EMBER_ZCL_STATUS_ACTION_DENIED;
  }
  return emberCoapRespondWithCode(info, code);
}

EmberStatus emberZclRespondWithPath(const EmberCoapRequestInfo *requestInfo,
                                    EmberCoapCode code,
                                    const uint8_t *path,
                                    const EmberCoapOption *options,
                                    uint8_t numberOfOptions,
                                    const uint8_t *payload,
                                    uint16_t payloadLength)
{
  if (suppressResponse(requestInfo, code, payload, payloadLength)) {
    return EMBER_ZCL_STATUS_ACTION_DENIED;
  }

  return emberCoapRespondWithPath(requestInfo,
                                  code,
                                  path,
                                  options,
                                  numberOfOptions,
                                  payload,
                                  payloadLength);
}

EmberStatus emberZclRespondCborPayload(const EmberCoapRequestInfo *info,
                                       EmberCoapCode code,
                                       const uint8_t *locationPath,
                                       const uint8_t *payload,
                                       uint16_t payloadLength)
{
  EmberCoapOption options[] = {
    { EMBER_COAP_OPTION_CONTENT_FORMAT, NULL, 1, EMBER_COAP_CONTENT_FORMAT_CBOR, },
  };
  return emberZclRespondWithPath(info,
                                 code,
                                 locationPath,
                                 options,
                                 COUNTOF(options),
                                 payload,
                                 payloadLength);
}

EmberStatus emberZclRespondLinkFormatPayload(const EmberCoapRequestInfo *info,
                                             EmberCoapCode code,
                                             const uint8_t *payload,
                                             uint16_t payloadLength,
                                             EmberCoapContentFormatType contentFormat)
{
  EmberCoapOption options[] = {
    { EMBER_COAP_OPTION_CONTENT_FORMAT, NULL, 1, EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR, },
  };
  // Override default link-format+cbor w/ link-format.
  if (contentFormat == EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT) {
    options[0].intValue = contentFormat;
  }
  return emberZclRespondWithPath(info,
                                 code,
                                 NULL,                // location path
                                 options,
                                 COUNTOF(options),
                                 payload,
                                 payloadLength);
}

EmberStatus emberZclRespondWithStatus(const EmberCoapRequestInfo *info,
                                      EmberCoapCode code,
                                      EmberZclStatus_t status)
{
  if (suppressResponse(info, code, NULL, 0)) {
    return EMBER_ZCL_STATUS_ACTION_DENIED;
  }

  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  CborState state;
  emCborEncodeStart(&state, buffer, sizeof(buffer));
  return (emZclEncodeDefaultResponse(&state, status)
          ? emberZclRespondCborState(info, code, NULL, &state)
          : EMBER_ERR_FATAL);
}

bool emZclEncodeDefaultResponse(CborState *state,
                                EmberZclStatus_t status)
{
  if (status == EMBER_ZCL_STATUS_SUCCESS) { // No payload for success (Section 2.8.2.5)
    return TRUE;
  } else { // {"s":<error>} for failure
    DefaultResponseError errorResponse = { .status = status };
    return emCborEncodeStruct(state,
                              defaultResponseErrorSpec,
                              &errorResponse);
  }
}

bool emZclGetMulticastAddress(EmberIpv6Address *dst)
{
  EmberNetworkStatus status = emberNetworkStatus();
  if (status == EMBER_NO_NETWORK || status == EMBER_SAVED_NETWORK) {
    return false;
  }

  EmberNetworkParameters parameters = { { 0 } };
// The All Thread Nodes multicast address is filled in once the ULA prefix is
// known.  It is an RFC3306 address with the format ff33:40:<ula prefix>::1.
  MEMCOPY(dst,
          &emFf33AllThreadNodesMulticastAddress,
          sizeof(EmberIpv6Address));
  emberGetNetworkParameters(&parameters);
  MEMCOPY(dst->bytes + RFC3306_NETWORK_PREFIX_OFFSET,
          &parameters.ulaPrefix,
          sizeof(EmberIpv6Prefix));

  return true;
}

bool emZclParseUri(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, EmberCoapContentFormatType contentFormat, EmZclUriContext_t *context)
{
  // Parses the zcl discovery response "/zcl/e/EE/[cs]CCCC>"  into the
  // EmZclUriContext_t struct.
  // The response payload may be link-format or link-format+cbor type so we
  // should handle both.
  // Note: caller must setup 'incoming' ptr to point to payload before first
  // call to this function.
  // returns- true : zcl/e/ tag was found, 'incoming' ptr is updated.
  //        - false: zcl/e/ invalid inputs or tag was not found.

  if ((payload != NULL)
      && (payloadLength != 0)
      && (*incoming != NULL)
      && (*incoming < payload + payloadLength)) {
    switch (contentFormat) {
      case EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR:
        return zclParseUriLinkFormatPlusCbor(payload, payloadLength, incoming, context);
      default:
        return zclParseUriLinkFormat(payload, payloadLength, incoming, context);
    }
  }

  return false;
}

EmberZclStatus_t emZclCborValueReadStatusToEmberStatus(EmZclCoreCborValueReadStatus_t cborValueReadStatus)
{
  switch (cborValueReadStatus) {
    case EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE:
      return EMBER_ZCL_STATUS_INVALID_DATA_TYPE;
    case EM_ZCL_CORE_CBOR_VALUE_READ_INVALID_BOOLEAN_VALUE:
    case EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE:
    case EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL:
      return EMBER_ZCL_STATUS_INVALID_VALUE;
    default:
      return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
  }
}

static bool dropIncomingMulticast(const EmZclContext_t *context, const uint8_t *uri)
{
  // Return "true" if this request must be dropped, "false" otherwise
  // BDB Spec 16-07008-071 2.8.6.2: Drop multicast requests
  //   UNLESS
  //        they are for discovery
  //     OR sent to a group URI and group multicast address
  //     OR are a part of EZ-mode exchange
  //     OR sent to OTA cluster client
  if (!isMulticastAddress(context->info->localAddress.bytes)) {
    return false;
  } else if (uri == NULL) {  // can't even check URI
    return true;
  } else if (strncmp((char *)uri, ".well-known/core", strlen(".well-known/core")) == 0) {
    return false;
  } else if (emberZclIsAddressGroupMulticast(&(context->info->localAddress))
             && (strncmp((char *)uri, "zcl/g", strlen("zcl/g")) == 0)) {
    return false;
  } else if (strncmp((char *)uri, "zcl/m/c/", strlen("zcl/m/c/")) == 0) { // ez-mode
    return false;
  }

  // If OTA client is enabled allow zcl/e/XXX/c<OTAClusterId>/
#ifdef EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT
  char uriNullTerm[32] = "";
  char clStr[32] = "";       // Store c<cluster id>, e.g. c2000

  strncpy(uriNullTerm, (char *)uri, strlen("zcl/e/XXX/cXXXXXX/"));
  uriNullTerm[strlen("zcl/e/XXX/cXXXXXX/")] = '\0'; // Make sure it's NULL-terminated
  snprintf(clStr, 31, "/c%x/", EMBER_ZCL_CLUSTER_OTA_BOOTLOAD);

  if ((strncmp(uriNullTerm, "zcl/e/", strlen("zcl/e/")) == 0) && (strstr(uriNullTerm, clStr) != NULL)) {
    return false;
  }
#endif

  // Didn't match any of the allowed cases
  return true;
}
