/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include "stack/ip/ip-address.h"
#include "stack/ip/tls/tls.h"
#include "stack/ip/tls/tls-sha256.h"
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
#include CHIP_AF_API_ZCL_CORE_RESOURCE_DIRECTORY
#include CHIP_AF_API_ZCL_CORE_DTLS_MANAGER

void chipZclGetPublicKeyCallback(const uint8_t **publicKey,
                                  uint16_t *publicKeySize);
ChipZclUid_t chZclUid = { { 0 } };

// -----------------------------------------------------------------------------
// Constants.

#define RFC3306_NETWORK_PREFIX_OFFSET 4

static uint16_t fetchCoapOptions(ChipCoapReadOptions *options,
                                 ChipCoapOptionType wantType,
                                 const uint8_t **valuePointers,
                                 uint16_t *valueLengths,
                                 uint16_t maxOptions);
static bool uriLookup(ChZclContext_t *context, ChZclUriPath *paths);
static bool processQueries(ChZclContext_t *context, ChZclUriQuery *queries);
static bool isValidFormatOption(ChZclContext_t *context,
                                ChipCoapOptionType optionType,
                                uint8_t flags);

#define isValidAcceptOption(context, flags) \
  isValidFormatOption((context), CHIP_COAP_OPTION_ACCEPT, (flags))

// Coap content-format option check result is always true for GET or DELETE
// methods because there is no associated request payload to check.
#define isValidContentFormatOption(context, flags) \
  ((context->code == CHIP_COAP_CODE_GET)          \
   || (context->code == CHIP_COAP_CODE_DELETE)    \
   || isValidFormatOption((context), CHIP_COAP_OPTION_CONTENT_FORMAT, (flags)))

// URI handlers
static void zclHandler(ChZclContext_t *context);
static void clusterHandler(ChZclContext_t *context);
static bool hexSegmentToInt(const ChZclContext_t *context,
                            uint8_t depth,
                            size_t size,
                            uintmax_t *result);

static bool zclParseUriLinkFormat(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, ChZclUriContext_t *context);
static bool zclParseUriLinkFormatPlusCbor(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, ChZclUriContext_t *context);

static ChipZclStatus_t isAccessAllowed(ChZclContext_t *context, void *data);
static bool dropIncomingMulticast(const ChZclContext_t *context, const uint8_t *uri);

typedef struct {
  ChipZclStatus_t status;
} DefaultResponseError;

static const ZclipStructSpec defaultResponseErrorSpec[] = {
  #define CHIP_ZCLIP_STRUCT DefaultResponseError
  CHIP_ZCLIP_OBJECT(sizeof(CHIP_ZCLIP_STRUCT), 1, NULL),
  CHIP_ZCLIP_FIELD_NAMED(CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, status, "s"),
  #undef CHIP_ZCLIP_STRUCT
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

#define M(x) CH_ZCL_URI_FLAG_METHOD_ ## x
#define GXXX (M(GET))
#define XPXX (M(POST))
#define GPXX (M(GET) | M(POST))
#define GXPX (M(GET) | M(PUT))
#define GXPD (M(GET) | M(PUT) | M(DELETE))

#define F(x) CH_ZCL_URI_FLAG_FORMAT_ ## x
#define FCBOR (F(CBOR))
#define FLINK (F(LINK))

static bool uriManagementCommandIdMatch(ChZclContext_t *context, void *data, uint8_t depth);
static bool uriEndpointIdMatch         (ChZclContext_t *context, void *data, uint8_t depth);
static bool uriGroupIdMatch            (ChZclContext_t *context, void *data, uint8_t depth);
static bool uriClusterIdMatch          (ChZclContext_t *context, void *data, uint8_t depth);
static bool uriAttributeIdMatch        (ChZclContext_t *context, void *data, uint8_t depth);
static bool uriBindingIdMatch          (ChZclContext_t *context, void *data, uint8_t depth);
static bool uriClusterCommandIdMatch   (ChZclContext_t *context, void *data, uint8_t depth);
static bool uriReportingIdMatch        (ChZclContext_t *context, void *data, uint8_t depth);

static ChZclUriQuery attributeUriQueries[] = {
  // zcl/[eg]/XX/<cluster>/a?f=1,2+3,4-5,...
  { chZclUriQueryStringPrefixMatch, "f=", chZclAttributeUriQueryFilterParse },
  // zcl/[eg]/XX/<cluster>/a?u
  { chZclUriQueryStringPrefixMatch, "u", chZclAttributeUriQueryUndividedParse },
  // zcl/[eg]/XX/<cluster>/a?meta=  (==metadata query)
  { chZclUriQueryStringPrefixMatch, "meta=", chZclAttributeUriMetadataQueryParse },

  // terminator
  { NULL, NULL, NULL },
};

static ChZclUriPath zclUriPaths[] = {
  // zcl
  {   1, 255, GXXX | FCBOR, chZclUriPathStringMatch, "zcl", NULL, zclHandler },

  // zcl/t
  { 255, 1, XPXX | FCBOR, chZclUriPathStringMatch, "t", NULL, chZclAccessTokenHandler },

  // zcl/m
  {   1, 3, GXXX | FCBOR, chZclUriPathStringMatch, "m", NULL, chZclManagementHandler },
  {   1, 255, GXXX | FCBOR, chZclUriPathStringMatch, "c", NULL, chZclManagementCommandHandler },
  { 255, 255, XPXX | FLINK, uriManagementCommandIdMatch, NULL, NULL, chZclManagementCommandIdHandler },

  // zcl/e
  {   1, 2, GXXX | FCBOR, chZclUriPathStringMatch, "e", NULL, chZclUriEndpointHandler },
  // zcl/e/XX
  {   3, 255, GXXX | FCBOR, uriEndpointIdMatch, NULL, NULL, chZclUriEndpointIdHandler },

  // zcl/g
  {   1, 255, GXXX | FCBOR, chZclUriPathStringMatch, "g", NULL, chZclUriGroupHandler },
  // zcl/g/XXXX
  {   1, 255, GXXX | FCBOR, uriGroupIdMatch, NULL, NULL, chZclUriGroupIdHandler },

  // cluster ID, after either zcl/e/XX or zcl/g/XXXX
  {   1, 255, GXXX | FCBOR, uriClusterIdMatch, NULL, NULL, clusterHandler },

  // zcl/[eg]/XX/<cluster>/a:
  {   1, 3, GPXX | FCBOR, chZclUriPathStringMatch, "a", attributeUriQueries, chZclUriClusterAttributeHandler },
  // zcl/[eg]/XX/<cluster>/a/XXXX:
  {  255, 1, GXPX | FCBOR, uriAttributeIdMatch, NULL, NULL, chZclUriClusterAttributeIdHandler },
  // zcl/[eg]/XX/<cluster>/a/XXXX/$xxxx:  ($xxxx==attribute metadata spec)
  { 255, 255, GXXX | FCBOR, uriAttributeIdMatch, "$", NULL, chZclUriClusterAttributeIdHandler },

  // zcl/[eg]/XX/<cluster>/b:
  {   1, 2, GPXX | FCBOR, chZclUriPathStringMatch, "b", NULL, chZclUriClusterBindingHandler },
  // zcl/[eg]/XX/<cluster>/b/XX:
  { 255, 255, GXPD | FCBOR, uriBindingIdMatch, NULL, NULL, chZclUriClusterBindingIdHandler },

  // zcl/[eg]/XX/<cluster>/c:
  {   1, 2, GXXX | FCBOR, chZclUriPathStringMatch, "c", NULL, chZclUriClusterCommandHandler },
  // zcl/[eg]/XX/<cluster>/c/XX:
  { 255, 255, XPXX | FCBOR, uriClusterCommandIdMatch, NULL, NULL, chZclUriClusterCommandIdHandler },

  // zcl/[eg]/XX/<cluster>/n:
  {   0, 1, XPXX | FCBOR, chZclUriPathStringMatch, "n", NULL, chZclUriClusterNotificationHandler },

  // zcl/[eg]/XX/<cluster>/r:
  {   1, 255, GPXX | FCBOR, chZclUriPathStringMatch, "r", NULL, chZclUriClusterReportingConfigurationHandler },
  // zcl/[eg]/XX/<cluster>/r/XX:
  { 255, 255, GXPD | FCBOR, uriReportingIdMatch, NULL, NULL, chZclUriClusterReportingConfigurationIdHandler },
};

void chZclInitHandler(void)
{
  const uint8_t *publicKey = NULL;
  uint16_t publicKeySize = 0;
  chipZclGetPublicKeyCallback(&publicKey, &publicKeySize);

  // TODO: Every device should have a UID, so we should assert if we don't have
  // the public key to generate it.  Most applications don't have public keys
  // yet though, so this isn't doable right now.
  //assert(publicKey != NULL && publicKeySize != 0);

  Sha256State state;
  emSha256Start(&state);
  emSha256HashBytes(&state, (const uint8_t *)"zcl.uid", 7);
  emSha256HashBytes(&state, publicKey, publicKeySize);
  emSha256Finish(&state, chZclUid.bytes);

  // Re-register to listen to all group multicast IPv6 addresses
  chZclReregisterAllMcastAddresses();

  chipUdpListen(CHIP_COAP_PORT, emFf05AllCoapNodesMulticastAddress.contents); // listen on ff05::fd
}

void chZclHandler(ChipCoapCode code,
                  uint8_t *uri,
                  ChipCoapReadOptions *options,
                  const uint8_t *payload,
                  uint16_t payloadLength,
                  const ChipCoapRequestInfo *info)
{
  ChZclContext_t context;
  MEMSET(&context, 0, sizeof(ChZclContext_t));
  context.code = code;
  context.options = options;
  context.payload = payload;
  context.payloadLength = payloadLength;
  context.info = info;
  context.groupId = CHIP_ZCL_GROUP_NULL;
  context.uriPathSegments = fetchCoapOptions(context.options,
                                             CHIP_COAP_OPTION_URI_PATH,
                                             context.uriPath,
                                             context.uriPathLength,
                                             MAX_URI_PATH_SEGMENTS);

  // EZ-mode command: set cluster ID to CHIP_ZCL_CLUSTER_NULL so that chZclIsProtectedResource()
  // does not disallow this as CHIP_ZCL_CLUSTER_BASIC (id 0)
  // TODO: Rework the access control implementation so that it does not require such workarounds
  if ((uri != NULL) && (strncmp((char *)uri, "zcl/m/c/", strlen("zcl/m/c/")) == 0)) {
    context.clusterSpec.id = CHIP_ZCL_CLUSTER_NULL;
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
           || uriLookup(&context, chZclWellKnownUriPaths)
           || uriLookup(&context, chZclRdUriPaths)
           )
      ) {
    chZclRespond404NotFound(info);
  }
}

static uint16_t fetchCoapOptions(ChipCoapReadOptions *options,
                                 ChipCoapOptionType wantType,
                                 const uint8_t **valuePointers,
                                 uint16_t *valueLengths,
                                 uint16_t maxOptions)
{
  uint16_t count = 0;
  chipResetReadOptionPointer(options);
  while (true) {
    const uint8_t *valuePointer;
    uint16_t valueLength;
    ChipCoapOptionType type = chipReadNextOption(options,
                                                   &valuePointer,
                                                   &valueLength);
    if (type == CHIP_COAP_NO_OPTION) {
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

static bool uriLookup(ChZclContext_t *context, ChZclUriPath *paths)
{
  uint8_t segmentsMatched = 0;

  while (true) {
    uint8_t skip = paths->failSkip;

    if (paths->match(context, paths->data, segmentsMatched)) {
      ChipZclStatus_t status = isAccessAllowed(context, paths->data);
      switch (status) {
        case CHIP_ZCL_STATUS_SUCCESS: {
          segmentsMatched += 1;
          if (segmentsMatched == context->uriPathSegments) {
            if (context->code < CHIP_COAP_CODE_GET
                || CHIP_COAP_CODE_DELETE < context->code
                || (context->code == CHIP_COAP_CODE_GET
                    && !READBITS(paths->flags, CH_ZCL_URI_FLAG_METHOD_GET))
                || (context->code == CHIP_COAP_CODE_POST
                    && !READBITS(paths->flags, CH_ZCL_URI_FLAG_METHOD_POST))
                || (context->code == CHIP_COAP_CODE_PUT
                    && !READBITS(paths->flags, CH_ZCL_URI_FLAG_METHOD_PUT))
                || (context->code == CHIP_COAP_CODE_DELETE
                    && !READBITS(paths->flags, CH_ZCL_URI_FLAG_METHOD_DELETE))) {
              chZclRespond405MethodNotAllowed(context->info);
            } else if (paths->action != NULL) {
              if (paths->queries != NULL
                  && !processQueries(context, paths->queries)) {
                if (context->code != CHIP_COAP_CODE_GET) {
                  chZclRespond405MethodNotAllowed(context->info); // only GET access allowed for queries.
                } else {
                  chZclRespond400BadRequest(context->info); // bad query format.
                }
              } else if (!isValidAcceptOption(context, paths->flags)) {
                chZclRespond406NotAcceptable(context->info);
              } else if (!isValidContentFormatOption(context, paths->flags)) {
                chZclRespond415UnsupportedContentFormat(context->info);
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
        case CHIP_ZCL_STATUS_NOT_AUTHORIZED:
          chZclRespond401Unauthorized(context->info);
          return true;
        case CHIP_ZCL_STATUS_FORBIDDEN:
          chipZclRespondWithCode(context->info, CHIP_COAP_CODE_403_FORBIDDEN);
          return true;
        case CHIP_ZCL_STATUS_FAILURE:
          chZclRespond400BadRequest(context->info);
          return true;
        default:
          chZclRespond500InternalServerError(context->info);
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

static ChipZclStatus_t isAccessAllowed(ChZclContext_t *context, void *data)
{
  // we only care about single characters "a", "c", "n", "r" etc.
  if (data == NULL || ((uint8_t *) data)[1] != '\0') {
    return CHIP_ZCL_STATUS_SUCCESS;
  }

  uint8_t accessType = ((uint8_t *) data)[0]; // fetch character

  if (!chZclIsProtectedResource(&context->clusterSpec, accessType)) {
    return CHIP_ZCL_STATUS_SUCCESS;
  }

  ChipZclDeviceId_t deviceId = (context->endpoint == NULL
                                 ? CHIP_ZCL_DEVICE_ID_NULL
                                 : context->endpoint->deviceId);
  uint8_t sessionId = (uint8_t)(unsigned long) context->info->transmitHandlerData;

  // check if we already have access or if we are using an incorrect DTLS mode
  ChipZclStatus_t status
    = chZclAllowRemoteAccess(sessionId,
                             &context->clusterSpec,
                             deviceId,
                             accessType);

  // continue if unauthorized, because the payload could contain the token providing access
  if (status != CHIP_ZCL_STATUS_NOT_AUTHORIZED && status != CHIP_ZCL_STATUS_FORBIDDEN) {
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
          ChipZclUid_t uid;
          if (chipZclDtlsManagerGetUidBySessionId(sessionId,
                                                   &uid) != CHIP_SUCCESS) {
            return CHIP_ZCL_STATUS_NOT_AUTHORIZED;
          }

          uint16_t binaryTokenSize = CH_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN;
          uint8_t binaryToken[CH_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN];
          if (!chZclExtractBinaryAccessToken(&state,
                                             binaryToken,
                                             &binaryTokenSize)) {
            return CHIP_ZCL_STATUS_FAILURE;
          }

          if (chZclAddIncomingToken(&uid,
                                    binaryToken,
                                    binaryTokenSize)) {
            // new access token added successfully, so check access again
            return chZclAllowRemoteAccess(sessionId,
                                          &context->clusterSpec,
                                          deviceId,
                                          accessType);
          } else {
            return CHIP_ZCL_STATUS_NOT_AUTHORIZED;
          }
        } else {
          emCborDecodeSkipValue(&state); // unknown key, so ignore the value
        } // "at"
      } // while
    } // if
  }

  return status;
}

static bool processQueries(ChZclContext_t *context, ChZclUriQuery *queries)
{
  context->uriQuerySegments = fetchCoapOptions(context->options,
                                               CHIP_COAP_OPTION_URI_QUERY,
                                               context->uriQuery,
                                               context->uriQueryLength,
                                               MAX_URI_PATH_SEGMENTS);

  // If we see a query that we don't know about, then just ignore it for the
  // sake of future compatibility.
  for (size_t segment = 0; segment < context->uriQuerySegments; segment++) {
    for (ChZclUriQuery *query = queries; query->match != NULL; query++) {
      if (query->match(context, query->data, segment)
          && !query->parse(context, query->data, segment)) {
        return false;
      }
    }
  }

  // If there is an active query filter after query has been parsed then
  // we should check that access type is GET.
  if ((context->attributeQuery.filterCount != 0)
      && (context->code != CHIP_COAP_CODE_GET)) {
    return false;
  }

  return true;
}

static bool isValidFormatOption(ChZclContext_t *context,
                                ChipCoapOptionType optionType,
                                uint8_t flags)
{
  const uint8_t *valuePointer;
  uint16_t valueLength;
  uint16_t count = fetchCoapOptions(context->options,
                                    optionType,
                                    &valuePointer,
                                    &valueLength,
                                    1);
  if (count != 0 && READBITS(flags, CH_ZCL_URI_FLAG_FORMAT_MASK)) {
    uint16_t value = valuePointer[0];
    if (valueLength == 2) {
      value = (value << 8) | valuePointer[1];
    }
    ChipCoapContentFormatType cf = (ChipCoapContentFormatType)(value);
    if (READBITS(flags, CH_ZCL_URI_FLAG_FORMAT_CBOR)) {
      return (cf == CHIP_COAP_CONTENT_FORMAT_CBOR);
    } else if (READBITS(flags, CH_ZCL_URI_FLAG_FORMAT_LINK)) {
      return (cf == CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR
              || cf == CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT);
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

bool chZclUriPathStringMatch(ChZclContext_t *context,
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

bool chZclUriQueryStringPrefixMatch(ChZclContext_t *context,
                                    void *castString,
                                    uint8_t depth)
{
  return stringMatch(context->uriQuery[depth],
                     context->uriQueryLength[depth],
                     castString,
                     false); // match string length
}

static bool uriManagementCommandIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t commandId;
  if (hexSegmentToInt(context, depth, sizeof(context->command->commandId), &commandId)) {
    context->command = chZclManagementFindCommand((ChipZclCommandId_t)commandId);
    return context->command != NULL;
  } else {
    return false;
  }
}

static bool uriEndpointIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t endpointId;
  if (hexSegmentToInt(context, depth, sizeof(context->endpoint->endpointId), &endpointId)) {
    context->endpoint = chZclFindEndpoint((ChipZclEndpointId_t)endpointId);
    return (context->endpoint != NULL);
  } else {
    return false;
  }
}

static bool uriGroupIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t groupId;
  if (hexSegmentToInt(context, depth, sizeof(context->groupId), &groupId)
      && chZclHasGroup((ChipZclGroupId_t)groupId)) {
    context->groupId = groupId;
    return true;
  } else {
    return false;
  }
}

static bool uriClusterIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  if (!chZclStringToCluster(context->uriPath[depth],
                            context->uriPathLength[depth],
                            &context->clusterSpec)) {
    return false;
  }

  if (context->groupId != CHIP_ZCL_GROUP_NULL) {
    size_t i;
    for (i = 0; i < chZclEndpointCount; i++) {
      if (chipZclIsEndpointInGroup(chZclEndpointTable[i].endpointId,
                                    context->groupId)
          && chZclEndpointHasCluster(chZclEndpointTable[i].endpointId,
                                     &context->clusterSpec)) {
        return true;
      }
    }
    return false;
  } else {
    return chZclEndpointHasCluster(context->endpoint->endpointId,
                                   &context->clusterSpec);
  }
}

static bool uriAttributeIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t attributeId;
  if (hexSegmentToInt(context, depth, sizeof(context->attribute->attributeId), &attributeId)) {
    context->attribute = chZclFindAttribute(&context->clusterSpec,
                                            (ChipZclAttributeId_t)attributeId,
                                            false); // exclude remote

    if (context->attribute != NULL) {
      // Found attribute, now check if we also need to match on uri
      // metadata item string: a/XXXX/$yyyy:
      // where $yyyy = the metadata item string, e.g. $base, $acc
      context->attributeQuery.metadata = CH_ZCL_METADATA_NONE;
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
      if (MEMCOMPARE(metaUriStr, CH_ZCL_URI_METADATA_BASE, strlen(CH_ZCL_URI_METADATA_BASE)) == 0) {
        context->attributeQuery.metadata = CH_ZCL_METADATA_BASE;
        return true;
      } else if (MEMCOMPARE(metaUriStr, CH_ZCL_URI_METADATA_ACCESS, strlen(CH_ZCL_URI_METADATA_ACCESS)) == 0) {
        context->attributeQuery.metadata = CH_ZCL_METADATA_ACCESS;
        return true;
      } else {
        context->attributeQuery.metadata = CH_ZCL_METADATA_NOT_SUPPORTED;
      }
      //TODO- Add support for other metadata types here.
    }
  }

  return false;
}

static bool uriBindingIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t bindingId;
  if (hexSegmentToInt(context, depth, sizeof(context->bindingId), &bindingId)
      && chZclHasBinding(context, (ChipZclBindingId_t)bindingId)) {
    context->bindingId = bindingId;
    return true;
  } else {
    return false;
  }
}

static bool uriClusterCommandIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t commandId;
  if (hexSegmentToInt(context, depth, sizeof(context->command->commandId), &commandId)) {
    context->command = chZclFindCommand(&context->clusterSpec, (ChipZclCommandId_t)commandId);
    return (context->command != NULL);
  } else {
    return false;
  }
}

static bool uriReportingIdMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t reportingConfigurationId;
  if (hexSegmentToInt(context, depth, sizeof(context->reportingConfigurationId), &reportingConfigurationId)
      && chZclHasReportingConfiguration(context->endpoint->endpointId,
                                        &context->clusterSpec,
                                        (ChipZclReportingConfigurationId_t)reportingConfigurationId)) {
    context->reportingConfigurationId = reportingConfigurationId;
    return true;
  } else {
    return false;
  }
}

static bool zclParseUriLinkFormat(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, ChZclUriContext_t *context)
{
  // Parses link-format discovery response "</zcl/e/EE/[cs]CCCC>"
  // into ChZclUriContext_t struct.
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
        && (chZclHexStringToInt((const uint8_t *)finger,
                                delimiter - finger,
                                &endpointId))) {
      finger = delimiter;
      finger++;

      // parsing cluster number.
      ChipZclClusterSpec_t clusterSpec;
      char *endBracket;
      if (((endBracket = strchr(finger, '>')) != NULL)
          && (chZclStringToCluster((const uint8_t *)finger,
                                   endBracket - finger,
                                   &clusterSpec))) {
        context->endpointId = endpointId;
        MEMCOPY(context->clusterSpec, &clusterSpec, sizeof(ChipZclClusterSpec_t));
        *incoming = (uint8_t *)endBracket + 1; // update ptr for next call.
        return true;
      }
    }
  }

  return false;
}

static bool zclParseUriLinkFormatPlusCbor(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, ChZclUriContext_t *context)
{
  // Parses link-format+cbor discovery response "[{1:"/zcl/e/EE/[cs]CCCC",}, {1:"/zcl/e/EE/[cs]CCCC",}]"
  // into ChZclUriContext_t struct. The payload may contain a number of maps
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
                                     CHIP_ZCLIP_TYPE_MAX_LENGTH_STRING,
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
          && (chZclHexStringToInt((const uint8_t *)finger,
                                  delimiter - finger,
                                  &endpointId))) {
        finger = delimiter;
        finger++;

        // parsing cluster number.
        ChipZclClusterSpec_t clusterSpec;
        if (chZclStringToCluster((const uint8_t *)finger,
                                 zclUriLen - (finger - zclUri), // cluster string length.
                                 &clusterSpec)) {
          context->endpointId = endpointId;
          MEMCOPY(context->clusterSpec, &clusterSpec, sizeof(ChipZclClusterSpec_t));
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
static void zclHandler(ChZclContext_t *context)
{
  chZclRespond205ContentCbor(context->info, egmtPayload, sizeof(egmtPayload));
}

// GET zcl/e/XX/[cs]XXXX:
// GET zcl/e/XX/[cs]XXXXXXXX:
// GET zcl/g/XXXX/[cs]XXXX:
// GET zcl/g/XXXX/[cs]XXXXXXXX:
static const uint8_t abcnrPayload[] = { // ["a", "b", "c", "n", "r"]
  0x85, 0x61, 0x61, 0x61, 0x62, 0x61, 0x63, 0x61, 0x6e, 0x61, 0x72,
};
static void clusterHandler(ChZclContext_t *context)
{
  if (context->groupId == CHIP_ZCL_GROUP_NULL) {
    // Only supported for single endpoint URI...
    chZclRespond205ContentCbor(context->info, abcnrPayload, sizeof(abcnrPayload));
  } else {
    // ...not for group URI.
    chZclRespond404NotFound(context->info);
  }
}

static bool hexSegmentToInt(const ChZclContext_t *context,
                            uint8_t depth,
                            size_t size,
                            uintmax_t *result)
{
  return (context->uriPathLength[depth] <= size * 2 // bytes to nibbles
          && chZclHexStringToInt(context->uriPath[depth],
                                 context->uriPathLength[depth],
                                 result));
}

int32_t chipZclCompareClusterSpec(const ChipZclClusterSpec_t *s1,
                                   const ChipZclClusterSpec_t *s2)
{
  if (s1->role != s2->role) {
    return (s1->role == CHIP_ZCL_ROLE_CLIENT ? -1 : 1);
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

bool chipZclAreClusterSpecsEqual(const ChipZclClusterSpec_t *s1,
                                  const ChipZclClusterSpec_t *s2)
{
  return (chipZclCompareClusterSpec(s1, s2) == 0);
}

void chipZclReverseClusterSpec(const ChipZclClusterSpec_t *s1,
                                ChipZclClusterSpec_t *s2)
{
  s2->role = (s1->role == CHIP_ZCL_ROLE_CLIENT
              ? CHIP_ZCL_ROLE_SERVER
              : CHIP_ZCL_ROLE_CLIENT);
  s2->manufacturerCode = s1->manufacturerCode;
  s2->id = s1->id;
}

// returns true if we should suppress response, false otherwise
// based on spec 16-07008-047: 2.8.6.2 Handling of Multicast Request Message
static bool suppressResponse(const ChipCoapRequestInfo *info,
                             ChipCoapCode code,
                             const uint8_t *payload,
                             uint16_t payloadLength)
{
  // do not suppress response for unicast
  if (!emIsMulticastAddress(info->localAddress.bytes)) {
    return false;
  }

  // suppress multicast response for 4.xx or 5.xx error status
  if (chipCoapIsClientErrorResponse(code)
      || chipCoapIsServerErrorResponse(code)) {
    return true;
  }

  assert(chipCoapIsSuccessResponse(code)); // should not get here unless 2.xx

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

ChipStatus chZclRespondWithCode(const ChipCoapRequestInfo *info,
                                 ChipCoapCode code)
{
  if (suppressResponse(info, code, NULL, 0)) {
    return CHIP_ZCL_STATUS_ACTION_DENIED;
  }
  return chipCoapRespondWithCode(info, code);
}

ChipStatus chipZclRespondWithPath(const ChipCoapRequestInfo *requestInfo,
                                    ChipCoapCode code,
                                    const uint8_t *path,
                                    const ChipCoapOption *options,
                                    uint8_t numberOfOptions,
                                    const uint8_t *payload,
                                    uint16_t payloadLength)
{
  if (suppressResponse(requestInfo, code, payload, payloadLength)) {
    return CHIP_ZCL_STATUS_ACTION_DENIED;
  }

  return chipCoapRespondWithPath(requestInfo,
                                  code,
                                  path,
                                  options,
                                  numberOfOptions,
                                  payload,
                                  payloadLength);
}

ChipStatus chipZclRespondCborPayload(const ChipCoapRequestInfo *info,
                                       ChipCoapCode code,
                                       const uint8_t *locationPath,
                                       const uint8_t *payload,
                                       uint16_t payloadLength)
{
  ChipCoapOption options[] = {
    { CHIP_COAP_OPTION_CONTENT_FORMAT, NULL, 1, CHIP_COAP_CONTENT_FORMAT_CBOR, },
  };
  return chipZclRespondWithPath(info,
                                 code,
                                 locationPath,
                                 options,
                                 COUNTOF(options),
                                 payload,
                                 payloadLength);
}

ChipStatus chipZclRespondLinkFormatPayload(const ChipCoapRequestInfo *info,
                                             ChipCoapCode code,
                                             const uint8_t *payload,
                                             uint16_t payloadLength,
                                             ChipCoapContentFormatType contentFormat)
{
  ChipCoapOption options[] = {
    { CHIP_COAP_OPTION_CONTENT_FORMAT, NULL, 1, CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR, },
  };
  // Override default link-format+cbor w/ link-format.
  if (contentFormat == CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT) {
    options[0].intValue = contentFormat;
  }
  return chipZclRespondWithPath(info,
                                 code,
                                 NULL,                // location path
                                 options,
                                 COUNTOF(options),
                                 payload,
                                 payloadLength);
}

ChipStatus chipZclRespondWithStatus(const ChipCoapRequestInfo *info,
                                      ChipCoapCode code,
                                      ChipZclStatus_t status)
{
  if (suppressResponse(info, code, NULL, 0)) {
    return CHIP_ZCL_STATUS_ACTION_DENIED;
  }

  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  CborState state;
  emCborEncodeStart(&state, buffer, sizeof(buffer));
  return (chZclEncodeDefaultResponse(&state, status)
          ? chipZclRespondCborState(info, code, NULL, &state)
          : CHIP_ERR_FATAL);
}

bool chZclEncodeDefaultResponse(CborState *state,
                                ChipZclStatus_t status)
{
  if (status == CHIP_ZCL_STATUS_SUCCESS) { // No payload for success (Section 2.8.2.5)
    return TRUE;
  } else { // {"s":<error>} for failure
    DefaultResponseError errorResponse = { .status = status };
    return emCborEncodeStruct(state,
                              defaultResponseErrorSpec,
                              &errorResponse);
  }
}

bool chZclGetMulticastAddress(ChipIpv6Address *dst)
{
  ChipNetworkStatus status = chipNetworkStatus();
  if (status == CHIP_NO_NETWORK || status == CHIP_SAVED_NETWORK) {
    return false;
  }

  ChipNetworkParameters parameters = { { 0 } };
// The All Thread Nodes multicast address is filled in once the ULA prefix is
// known.  It is an RFC3306 address with the format ff33:40:<ula prefix>::1.
  MEMCOPY(dst,
          &emFf33AllThreadNodesMulticastAddress,
          sizeof(ChipIpv6Address));
  chipGetNetworkParameters(&parameters);
  MEMCOPY(dst->bytes + RFC3306_NETWORK_PREFIX_OFFSET,
          &parameters.ulaPrefix,
          sizeof(ChipIpv6Prefix));

  return true;
}

bool chZclParseUri(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, ChipCoapContentFormatType contentFormat, ChZclUriContext_t *context)
{
  // Parses the zcl discovery response "/zcl/e/EE/[cs]CCCC>"  into the
  // ChZclUriContext_t struct.
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
      case CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR:
        return zclParseUriLinkFormatPlusCbor(payload, payloadLength, incoming, context);
      default:
        return zclParseUriLinkFormat(payload, payloadLength, incoming, context);
    }
  }

  return false;
}

ChipZclStatus_t chZclCborValueReadStatusToChipStatus(ChZclCoreCborValueReadStatus_t cborValueReadStatus)
{
  switch (cborValueReadStatus) {
    case CH_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE:
      return CHIP_ZCL_STATUS_INVALID_DATA_TYPE;
    case CH_ZCL_CORE_CBOR_VALUE_READ_INVALID_BOOLEAN_VALUE:
    case CH_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE:
    case CH_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL:
      return CHIP_ZCL_STATUS_INVALID_VALUE;
    default:
      return CHIP_ZCL_STATUS_MALFORMED_COMMAND;
  }
}

static bool dropIncomingMulticast(const ChZclContext_t *context, const uint8_t *uri)
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
  } else if (chipZclIsAddressGroupMulticast(&(context->info->localAddress))
             && (strncmp((char *)uri, "zcl/g", strlen("zcl/g")) == 0)) {
    return false;
  } else if (strncmp((char *)uri, "zcl/m/c/", strlen("zcl/m/c/")) == 0) { // ez-mode
    return false;
  }

  // If OTA client is enabled allow zcl/e/XXX/c<OTAClusterId>/
#ifdef CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT
  char uriNullTerm[32] = "";
  char clStr[32] = "";       // Store c<cluster id>, e.g. c2000

  strncpy(uriNullTerm, (char *)uri, strlen("zcl/e/XXX/cXXXXXX/"));
  uriNullTerm[strlen("zcl/e/XXX/cXXXXXX/")] = '\0'; // Make sure it's NULL-terminated
  snprintf(clStr, 31, "/c%x/", CHIP_ZCL_CLUSTER_OTA_BOOTLOAD);

  if ((strncmp(uriNullTerm, "zcl/e/", strlen("zcl/e/")) == 0) && (strstr(uriNullTerm, clStr) != NULL)) {
    return false;
  }
#endif

  // Didn't match any of the allowed cases
  return true;
}
