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
#include <stdio.h>
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include EMBER_AF_API_ZCL_CORE_RESOURCE_DIRECTORY

static uint16_t appendResourceTypeCoreRd(char *finger,
                                         char *endOfBuffer);
static uint16_t appendResourceTypeClusterId(char *finger,
                                            char *endOfBuffer,
                                            const EmberZclClusterSpec_t *spec);
static uint16_t appendEndpointId(char *finger,
                                 char *endOfBuffer,
                                 EmberZclEndpointId_t endpointId,
                                 EmberZclDeviceId_t deviceId);
static uint16_t appendRevision(char *finger,
                               char *endOfBuffer,
                               const uint16_t version,
                               bool isProtocolRevision);
static bool startsWith(const uint8_t *string, const uint8_t *prefix);

uint16_t appendUriDelimiter(char *finger);

#define EM_ZCL_URI_PATH_APPEND_MAX_LEN (17)
#define EM_ZCL_URI_ENDPOINT_APPEND_MAX_LEN (17)
#define EM_ZCL_URI_CLUSTER_REVISION_APPEND_MAX_LEN (20)
#define EM_ZCL_URI_CLUSTER_ID_APPEND_MAX_LEN (20)
#define EM_ZCL_URI_DEVICE_ID_AND_ENDPOINT_APPEND_MAX_LEN (16)

// max wildcard string length = type_size * 2 + nul
#define EM_ZCL_WILDCARD_COMPARE_STRING_MAX_LEN                     \
  ((sizeof(EmberZclClusterId_t) > sizeof(EmberZclDeviceId_t))      \
   ? (sizeof(EmberZclClusterId_t) * 2 + EMBER_ZCL_STRING_OVERHEAD) \
   : (sizeof(EmberZclDeviceId_t) * 2 + EMBER_ZCL_STRING_OVERHEAD))

// Enums for helping with CLI commands.
typedef enum {
  EM_ZCL_CLI_DISCOVERY_REQUEST_FOR_CORE_RD,
  EM_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_ID,
  EM_ZCL_CLI_DISCOVERY_REQUEST_BY_DEVICE_ID,
  EM_ZCL_CLI_DISCOVERY_REQUEST_BY_ENDPOINT_AND_DEVICE_ID,
  EM_ZCL_CLI_DISCOVERY_REQUEST_BY_UID_STRING,
  EM_ZCL_CLI_DISCOVERY_REQUEST_BY_RESOURCE_VERSION, // if=urn:zcl:v0
  EM_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_REVISION, // if=urn:zcl:c.v0
} EmZclCliDiscoveryRequestType;

typedef struct {
  EmZclCliDiscoveryRequestType type;
  union {
    const EmberZclClusterSpec_t *clusterSpec;
    EmberZclEndpointId_t endpointId;
    const uint8_t *uidString;
    EmberZclClusterRevision_t version;
  } data;
  EmberZclDeviceId_t deviceId;
} EmZclDiscoveryRequest;

// URI handlers
static void wellKnownUriHandler(EmZclContext_t *context);
static void wellKnownUriQueryHandler(EmZclContext_t *context);
static bool wellKnownUriQueryDeviceTypeAndEndpointParse(EmZclContext_t *context,
                                                        void *castString,
                                                        uint8_t depth);
static bool wellKnownUriQueryVersionParse(EmZclContext_t *context,
                                          void *castString,
                                          uint8_t depth);
static bool wellKnownUriQueryClusterIdParse(EmZclContext_t *context,
                                            void *castString,
                                            uint8_t depth);
static bool wellKnownUriQueryUidParse(EmZclContext_t *context,
                                      void *data,
                                      uint8_t depth);

static EmZclUriQuery wellKnownUriQueries[] = {
  // target attributes / relation type
  { emZclUriQueryStringPrefixMatch, EM_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CLUSTER_ID, wellKnownUriQueryClusterIdParse              },
  { emZclUriQueryStringPrefixMatch, EM_ZCL_URI_QUERY_PREFIX_VERSION, wellKnownUriQueryVersionParse                },
  { emZclUriQueryStringPrefixMatch, EM_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT, wellKnownUriQueryDeviceTypeAndEndpointParse  },
  { emZclUriQueryStringPrefixMatch, EM_ZCL_URI_QUERY_UID, wellKnownUriQueryUidParse                    },

  // terminator
  { NULL, NULL, NULL                                  },
};

EmZclUriPath emZclWellKnownUriPaths[] = {
  // .well-known/core
  {   1, 255, EM_ZCL_URI_FLAG_METHOD_GET | EM_ZCL_URI_FLAG_FORMAT_LINK, emZclUriPathStringMatch, EM_ZCL_URI_WELL_KNOWN, NULL, NULL },
  { 255, 255, EM_ZCL_URI_FLAG_METHOD_GET | EM_ZCL_URI_FLAG_FORMAT_LINK, emZclUriPathStringMatch, EM_ZCL_URI_CORE, wellKnownUriQueries, wellKnownUriHandler },
};

//----------------------------------------------------------------
// URI segment matching functions

bool emUriQueryZclStringtoCluster(EmZclContext_t *context,
                                  uint8_t depth,
                                  const uint16_t offset)
{
  char *finger = (char *)context->uriQuery[depth] + offset;
  char *end = (char *)(context->uriQuery[depth] + context->uriQueryLength[depth]);

  if (*finger != ':') { // verify ':' in zcl:c
    return false;
  }
  finger += 2;
  if (*finger != EM_ZCL_URI_QUERY_DOT) { // verify '.' in "zcl:c."
    return false;
  }
  ++finger;

  char *roleOptionDot = memchr(finger,
                               EM_ZCL_URI_QUERY_DOT,
                               end - finger);
  char *wildcard = memchr(finger,
                          EM_ZCL_URI_QUERY_WILDCARD,
                          end - finger);
  char *endOfClsId = NULL;

  if (roleOptionDot && wildcard) {
    if (roleOptionDot < wildcard) { // zcl:c.<cluster_id>.*
      endOfClsId = roleOptionDot;
    } else { // zcl:c.<cluster_id>*.
      return false; // currently not supporting wildcard followed by option dot.
    }
  } else if ((roleOptionDot && !wildcard) // zcl:c.<cluster_id>.c/s
             && (end - (roleOptionDot + 1) == 1)) {  // should only be one character
    endOfClsId = roleOptionDot;
  } else if (!roleOptionDot && wildcard) { // zcl:c.<cluster_id>*
    endOfClsId = wildcard;
  } else { // zcl:c.<cluster_id>
    return false; // This is potentially the same as zcl:c.<cluster_id>.*
  }

  if (endOfClsId) {
    EmberZclClusterSpec_t *clusterSpec = &context->clusterSpec;
    clusterSpec->manufacturerCode = EMBER_ZCL_MANUFACTURER_CODE_NULL;
    context->mask |= EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUSTER_AND_ROLE;
    size_t len = endOfClsId - finger;

    if (len == 0) {
      if (wildcard) { // zcl:c.*
        context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD_ALL;
        return true;
      } else {
        return false;  // clusterId length must be > 0.
      }
    }

    // parse clusterId.
    uintmax_t clusterId;
    if (!emZclHexStringToInt((const uint8_t *)finger,
                             len,
                             &clusterId)) {
      return false;
    }
    context->clusterSpec.id = clusterId;

    if (roleOptionDot && wildcard) {
      if (roleOptionDot < wildcard) { // zcl:c.<cluster_id>.*
        context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ID;
        return true;
      } else { // zcl:c.<cluster_id>*.
        assert(0); // returned false earlier for this, so should not get here.
      }
    } else if (roleOptionDot && !wildcard) { // zcl:c.<cluster_id>.c/s
      context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ID;
      finger = roleOptionDot + 1;
      if (finger[0] == 'c') {
        clusterSpec->role = EMBER_ZCL_ROLE_CLIENT;
        context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ROLE;
        return true;
      } else if (finger[0] == 's') {
        clusterSpec->role = EMBER_ZCL_ROLE_SERVER;
        context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ROLE;
        return true;
      } else {
        return false;
      }
    } else if (!roleOptionDot && wildcard) { // zcl:c.<cluster_id>*
      context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD;
      return true;
    } else { // zcl:c.<cluster_id>
      assert(0); // returned false earlier for this, so should not get here.
    }
  }

  return false;
}

static bool wellKnownUriQueryDeviceTypeAndEndpointParse(EmZclContext_t *context,
                                                        void *castString,
                                                        uint8_t depth)
{
  const uint16_t length = strlen((char *) castString);
  char *start = (char *)(context->uriQuery[depth] + length);
  char *finger = start;
  char *end = (char *)(context->uriQuery[depth] + context->uriQueryLength[depth]);

  if (context->uriQueryLength[depth] <= length) {
    return false;
  }

  if (MEMCOMPARE(finger,
                 EM_ZCL_URI_QUERY_POSTFIX_DEVICE_ID,
                 strlen(EM_ZCL_URI_QUERY_POSTFIX_DEVICE_ID)) == 0) {
    uintmax_t deviceId;
    uintmax_t endpointId;
    char *dot = strchr(finger, EM_ZCL_URI_QUERY_DOT);
    if (dot == NULL) {
      return false; // must have at least "zcl:d."
    }
    finger = dot + 1;

    char *zeOptionDot = memchr(finger, EM_ZCL_URI_QUERY_DOT, end - finger);
    char *wildcard = memchr(finger, EM_ZCL_URI_QUERY_WILDCARD, end - finger);
    char *endOfDeviceId = NULL;

    if (zeOptionDot && wildcard) {
      if (zeOptionDot < wildcard) { // zcl:d.<device_id>.*
        endOfDeviceId = zeOptionDot;
      } else { // zcl:d.<device_id>*.
        return false; // currently not supporting wildcard followed by option dot.
      }
    } else if (zeOptionDot && !wildcard) { // zcl:d.<device_id>.<endpoint_id>
      endOfDeviceId = zeOptionDot;
    } else if (!zeOptionDot && wildcard) { // zcl:d.<device_id>*
      endOfDeviceId = wildcard;
    } else { // zcl:d.<device_id>
      return false; // This is potentially the same as zcl:d.<device_id>.*
    }

    if (endOfDeviceId) {
      context->mask |= EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_DEVICE_AND_ENDPOINT;
      size_t len = endOfDeviceId - finger;

      if (len == 0) {
        if (wildcard) { // zcl:d.*
          return true; // no mask here so that we return everything
        } else {
          return false;  // deviceId length must be > 0.
        }
      }

      // parse deviceId.
      if (!(emZclHexStringToInt((const uint8_t *)finger,
                                len,
                                &deviceId))) {
        return false;
      }

      context->deviceId = deviceId;

      if (zeOptionDot && wildcard) {
        if (zeOptionDot < wildcard) { // zcl:d.<device_id>.*
          context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ID;
          return true;
        } else { // zcl:d.<device_id>*.
          assert(0); // returned false earlier for this, so should not get here.
        }
      } else if (zeOptionDot && !wildcard) { // zcl:d.<device_id>.<endpoint_id>
        context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ID;
        finger = zeOptionDot + 1;
        if (emZclHexStringToInt((const uint8_t *)finger,
                                end - finger,
                                &endpointId)) {
          const EmZclEndpointEntry_t *ep = emZclFindEndpoint(endpointId);
          context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ENDPOINT;
          context->endpoint = ep; // (ep may be NULL here).
          return true;
        }
      } else if (!zeOptionDot && wildcard) { // zcl:d.<device_id>*
        context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WILDCARD;
        return true;
      } else { // zcl:d.<device_id>
        assert(0); // returned false earlier for this, so should not get here.
      }
    }
  }

  return false;
}

static bool wellKnownUriQueryVersionParse(EmZclContext_t *context,
                                          void *castString,
                                          uint8_t depth)
{
  char *string = (char *)castString;
  const uint16_t length = strlen(string);
  uintmax_t clusterRevision;

  // is it the weird c.v# or just v#?
  char *start = (char *)context->uriQuery[depth];
  char *dot = strstr(start + length, EM_ZCL_URI_QUERY_VERSION_KEY);
  char *end = start + context->uriQueryLength[depth];
  bool status = false;

  if (context->uriQueryLength[depth] <= length) {
    return false;
  }

  if (dot != NULL) { // c.v#
    // (note- "...zcl:c.v*" wildcard is rejected).
    status = emZclHexStringToInt((const uint8_t *)dot + strlen(EM_ZCL_URI_QUERY_VERSION_KEY),
                                 (size_t)(end - dot - strlen(EM_ZCL_URI_QUERY_VERSION_KEY)),
                                 &clusterRevision);
    if (status) {
      context->clusterRevision = clusterRevision;
      context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUSTER_REVISION;
      return true;
    }
  } else { // v#
    // ".well-known/core?if=urn:zcl:vX"
    // wildcard "zcl:*" should be allowed.
    uint8_t *tmp = (uint8_t *)start + length;
    if (tmp[0] == EM_ZCL_URI_QUERY_WILDCARD) {
      context->mask |= EM_ZCL_DISCOVERY_CONTEXT_FILTER_RESOURCE_WILDCARD;
      context->clusterRevision = 0;  // We only support v0.
      return true;
    } else if (tmp[0] == 'v') {
      tmp++;
      status = emZclHexStringToInt(tmp,
                                   (context->uriQueryLength[depth] - length - 1),
                                   &clusterRevision);

      if (status) {
        context->mask |= EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_RESOURCE_VERSION;
        context->clusterRevision = clusterRevision;
        return true;
      }
    }
  }

  return false;
}

static bool wellKnownUriQueryClusterIdParse(EmZclContext_t *context,
                                            void *castString,
                                            uint8_t depth)
{
  char *string = (char *) castString;
  const uint16_t length = strlen(string);

  if (context->uriQueryLength[depth] == length) {
    context->mask |= EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUS;
    return true;
  } else {
    bool matched;
    matched = emUriQueryZclStringtoCluster(context,
                                           depth,
                                           length);
    if (matched) {
      return true;
    }
  }

  return false;
}

static bool wellKnownUriQueryUidParse(EmZclContext_t *context,
                                      void *data,
                                      uint8_t depth)
{
  context->mask |= EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID;
  uint16_t length = context->uriQueryLength[depth];

  // Save reference to UID filter string.
  context->uidFilterString = (context->uriQuery[depth] + strlen((const char *)data));
  context->uidFilterLength = length - strlen((const char *)data);

  // if only one char, it should be wildcard
  if (context->uidFilterLength == 1) {
    if (context->uriQuery[depth][length - 1] == EM_ZCL_URI_QUERY_WILDCARD) {
      context->uidFilterLength--;
      return true;
    } else {
      return false;
    }
  }

  // if <= "ep=ni:///sha-256;" chars, it should end with wildcard
  if (context->uidFilterLength <= strlen(EM_ZCL_URI_QUERY_UID_SHA_256)) {
    if (context->uriQuery[depth][length - 1] == EM_ZCL_URI_QUERY_WILDCARD
        && (strncmp(EM_ZCL_URI_QUERY_UID_SHA_256,
                    (const char *) context->uidFilterString,
                    context->uidFilterLength - 1) == 0)) { // get rid of ending * from length
      context->uidFilterLength--;
      return true;
    } else {
      return false;
    }
  }

  // if > "ep=ni:///sha-256;" chars, then remove the "ep=ni:///sha-256;"
  context->uidFilterString += strlen(EM_ZCL_URI_QUERY_UID_SHA_256);
  context->uidFilterLength -= strlen(EM_ZCL_URI_QUERY_UID_SHA_256);

  // if ends with wildcard, remove the wildcard char from length
  if (context->uriQuery[depth][length - 1] == EM_ZCL_URI_QUERY_WILDCARD) {
    context->mask |= EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID_PREFIX;
    context->uidFilterLength--;
  } else if (context->uidFilterLength != EMBER_ZCL_UID_BASE64URL_LENGTH) {
    return false; // if no wildcard, then we should have the whole UID
  }

  // Validate that UID filter string contains only base64url characters.
  return emZclConvertBase64UrlToCode(context->uidFilterString,
                                     context->uidFilterLength,
                                     NULL);
}

static void wellKnownUriHandler(EmZclContext_t *context)
{
  if (context->mask == 0) {
    if (emIsMulticastAddress(context->info->localAddress.bytes)) {
      emberAfAppPrintln("Suppressing response to multicast discovery request - invalid request.");
    } else if ((context->uriPathSegments >= 2)
               && (MEMCOMPARE(context->uriPath[0], EM_ZCL_URI_WELL_KNOWN, strlen(EM_ZCL_URI_WELL_KNOWN)) == 0)
               && (MEMCOMPARE(context->uriPath[1], EM_ZCL_URI_CORE, strlen(EM_ZCL_URI_CORE)) == 0)) {
      wellKnownUriQueryHandler(context);       // If context is link-format+cbor this will send an empty Array response, i.e. [].
    } else {
      emZclRespond405MethodNotAllowed(context->info);
    }
  } else {
    wellKnownUriQueryHandler(context);
  }
}

// get well-known/core?a=A&b=B:
static void wellKnownUriQueryHandler(EmZclContext_t *context)
{
  // code to reply to discovery by both endpoint and class type.

  bool clusterFound = true;

  uint8_t wildcardComparePrefix[EM_ZCL_WILDCARD_COMPARE_STRING_MAX_LEN];
  uint8_t wildcardCompareString[EM_ZCL_WILDCARD_COMPARE_STRING_MAX_LEN];

  if (emberAfPluginResourceDirectoryHaveRegistered()) {
    // Do not respond to queries when we have already registered with an RD server
    emZclRespond404NotFound(context->info);
    return;
  }

  if (context->mask == EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_NONE) {
    return;
  }

  // Check for Accept option, assume default link-format+cbor if it is absent.
  EmberCoapContentFormatType accept = EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT;
  uint32_t optionValue;
  if (emberReadIntegerOption(context->options, EMBER_COAP_OPTION_ACCEPT, &optionValue)) {
    accept = (EmberCoapContentFormatType)optionValue;
  }

  char payload[EM_ZCL_MAX_WELL_KNOWN_REPLY_PAYLOAD];
  MEMSET(payload, 0, COUNTOF(payload));

  EmZclDiscPayloadContext_t dpc;
  if (!emZclInitDiscPayloadContext(&dpc, accept, (uint8_t *)payload, EM_ZCL_MAX_WELL_KNOWN_REPLY_PAYLOAD)) {
    // No payload implementation to support the format specified by Accept option.
    emZclRespond406NotAcceptable(context->info);
    return;
  }

  dpc.startPayload(&dpc);
  if (context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_RESOURCE_WILDCARD) {
    // GET .well-known/core?if=urn:zcl:* -- return all resources under /zcl/
    dpc.appendUid(&dpc);

    size_t i;
    for (i = 0; i < emZclEndpointCount; i++) {
      const EmZclEndpointEntry_t *epEntry = &emZclEndpointTable[i];
      const EmberZclClusterSpec_t **clusterSpecs = epEntry->clusterSpecs;
      while (*clusterSpecs != NULL) {
        dpc.startLink(&dpc);
        dpc.addResourceUri(&dpc, epEntry->endpointId, *clusterSpecs);
        dpc.addRt(&dpc, *clusterSpecs, true);
        dpc.addIf(&dpc, epEntry->endpointId, *clusterSpecs);
        dpc.addZe(&dpc, epEntry->endpointId, epEntry->deviceId);
        dpc.endLink(&dpc);

        clusterSpecs++;
      }         // while
    }
  } else if ((context->mask & EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUS)
             || (context->mask & EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_RESOURCE_VERSION)) {
    // /.well-known/core?rt=urn:zcl response to signal ZCLIP is supported.
    if (context->clusterRevision == 0) {   // we only support resource==0.
      dpc.appendUid(&dpc);
    }
  } else if ((context->mask & EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_DEVICE_AND_ENDPOINT)
             || (context->mask & EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUSTER_AND_ROLE)
             || (context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUSTER_REVISION)) {
    clusterFound = false;
    size_t i;
    for (i = 0; i < emZclEndpointCount; i++) {
      const EmZclEndpointEntry_t *epEntry = &emZclEndpointTable[i];
      const EmberZclClusterSpec_t **clusterSpecs = epEntry->clusterSpecs;
      const EmberZclClusterSpec_t *spec = NULL;
      spec = *clusterSpecs;
      while (spec != NULL) {
        do {
          if (context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WILDCARD) {
            emZclIntToHexString(epEntry->deviceId, sizeof(EmberZclDeviceId_t), wildcardCompareString);
            emZclIntToHexString(context->deviceId, sizeof(EmberZclDeviceId_t), wildcardComparePrefix);
            if (!startsWith(wildcardCompareString, wildcardComparePrefix)) {
              break;
            }
          }
          if ((context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ID)
              && (context->deviceId != epEntry->deviceId)) {
            break;
          }
          if ((context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ENDPOINT)
              && ((context->endpoint == NULL)
                  || (epEntry->endpointId != context->endpoint->endpointId))) {
            break;
          }
          if ((spec->manufacturerCode != EMBER_ZCL_MANUFACTURER_CODE_NULL)
              && (spec->manufacturerCode != context->clusterSpec.manufacturerCode)) {
            break;
          }
          if (context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD) {
            emZclIntToHexString(spec->id, sizeof(EmberZclClusterId_t), wildcardCompareString);
            emZclIntToHexString(context->clusterSpec.id, sizeof(EmberZclClusterId_t), wildcardComparePrefix);
            if (!startsWith(wildcardCompareString, wildcardComparePrefix)) {
              break;
            }
          }
          if ((context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ID)
              && (spec->id != context->clusterSpec.id)) {
            break;
          }
          if ((context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ROLE)
              && (spec->role != context->clusterSpec.role)) {
            break;
          }
          if ((context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUSTER_REVISION)) {
            EmberZclClusterRevision_t clusterRevision;
            EmberZclStatus_t status;
            status = emberZclReadAttribute(epEntry->endpointId,
                                           spec,
                                           EMBER_ZCL_ATTRIBUTE_CLUSTER_REVISION,
                                           &clusterRevision,
                                           sizeof(clusterRevision));

            if ((status != EMBER_ZCL_STATUS_SUCCESS)
                || context->clusterRevision != clusterRevision) {
              break;
            }
          }

          // If we got here we have a match so do the actual appending.
          dpc.startLink(&dpc);
          dpc.addResourceUri(&dpc, epEntry->endpointId, spec);
          dpc.addRt(&dpc, spec, true);
          dpc.addIf(&dpc, epEntry->endpointId, spec);
          dpc.addZe(&dpc, epEntry->endpointId, epEntry->deviceId);
          dpc.endLink(&dpc);
        } while (0); //do

        if (context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD_ALL
            || spec->id == context->clusterSpec.id) {
          clusterFound = true;
        } else if (context->mask & EM_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD) {
          if (startsWith(wildcardCompareString, wildcardComparePrefix)) {
            clusterFound = true;
          }
        }
        clusterSpecs++;
        spec = *clusterSpecs;
      } // while
    } // for
  } else if (context->mask & EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID) {
    uint8_t uidb64u[EMBER_ZCL_UID_BASE64URL_SIZE];
    size_t uidb64uLen = emZclUidToBase64Url(&emZclUid, EMBER_ZCL_UID_BITS, uidb64u);
    if (uidb64uLen == EMBER_ZCL_UID_BASE64URL_LENGTH
        && context->uidFilterLength <= EMBER_ZCL_UID_BASE64URL_LENGTH) {
      if (context->mask & EM_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID_PREFIX // prefix wildcard
          || context->uidFilterLength == EMBER_ZCL_UID_BASE64URL_LENGTH) { // full match
        if (strncmp((const char *)uidb64u,
                    (const char *)context->uidFilterString,
                    context->uidFilterLength) == 0) {
          dpc.appendUid(&dpc);
        }
      } else { // full wildcard
        dpc.appendUid(&dpc);
      }
    }
  } else { // .well-known/core with no filters
    dpc.appendUid(&dpc);
  }

  dpc.endPayload(&dpc);

  if (dpc.status != DISCOVERY_PAYLOAD_CONTEXT_STATUS_SUCCESS) {
    // Response payload exceeds buffer size or a "construct" function failed.
    if (dpc.status == DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW) {
      emZclRespond413RequestEntityTooLarge(context->info);
    } else {
      emZclRespond500InternalServerError(context->info);
    }
    return;
  }

  if (emIsMulticastAddress(context->info->localAddress.bytes)
      && dpc.payloadIsEmpty(&dpc)) {
    emberAfAppPrintln("Suppressing response to multicast discovery request - no payload.");
  } else {
    if ((dpc.contentFormat != EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR)
        && (!clusterFound)) {
      emZclRespond404NotFound(context->info);
    } else {
      emZclRespond205ContentLinkFormat(context->info,
                                       dpc.payloadPointer(&dpc),
                                       dpc.payloadLength(&dpc),
                                       dpc.contentFormat);
    }
  }
}

//----------------------------------------------------------------
// URI segment matching functions

// append "</zcl/e/EE/[cs]CCCC>;" without quotes.
uint16_t emZclUriAppendUriPath(char *finger,
                               char *endOfBuffer,
                               EmberZclEndpointId_t endpointId,
                               const EmberZclClusterSpec_t *clusterSpec)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < EM_ZCL_URI_PATH_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;
  finger += sprintf(finger, "</zcl");

  if (endpointId != EMBER_ZCL_ENDPOINT_NULL) {
    finger += sprintf(finger, "/e/%d", endpointId);
    if (clusterSpec != NULL) {
      finger += sprintf(finger,
                        "/%c%x",
                        (clusterSpec->role == EMBER_ZCL_ROLE_CLIENT) ? 'c' : 's',
                        clusterSpec->id);
    }
  }

  finger += sprintf(finger, ">;");

  return finger - start;
}

static uint16_t appendEndpointId(char *finger,
                                 char *endOfBuffer,
                                 EmberZclEndpointId_t endpointId,
                                 EmberZclDeviceId_t deviceId)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < EM_ZCL_URI_ENDPOINT_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;

  if (endpointId != EMBER_ZCL_ENDPOINT_NULL) {
    finger += sprintf(finger,
                      "%s%s%x.%d",
                      EM_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT,
                      EM_ZCL_URI_QUERY_POSTFIX_DEVICE_ID,
                      deviceId,
                      endpointId);
    finger += appendUriDelimiter(finger);
  }

  return finger - start;
}

static uint16_t appendUidString(char *finger,
                                const char *endOfBuffer,
                                const uint8_t *uidString)
{
  size_t length = strlen((const char *)uidString);

  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < (strlen(EM_ZCL_URI_QUERY_UID_SHA_256_PREFIX) + length + 1))) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, EM_ZCL_URI_QUERY_UID_SHA_256_PREFIX);  // add uid prefix.
  MEMCOPY(finger, uidString, length);
  finger += length;
  finger += appendUriDelimiter(finger);

  return finger - start;
}

static uint16_t appendRevision(char *finger,
                               char *endOfBuffer,
                               const uint16_t revision,
                               bool isProtocolRevision)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < EM_ZCL_URI_CLUSTER_REVISION_APPEND_MAX_LEN)) {
    return 0;
  }

  char * start = finger;

  finger += sprintf(finger, isProtocolRevision ? EM_ZCL_URI_QUERY_PROTOCOL_REVISION_FORMAT : EM_ZCL_URI_QUERY_CLUSTER_REVISION_FORMAT, revision);
  finger += appendUriDelimiter(finger);

  return finger - start;
}

uint16_t appendUriDelimiter(char *finger)
{
  finger += sprintf(finger, "&");
  return 1;
}

static uint16_t appendResourceTypeCoreRd(char *finger,
                                         char *endOfBuffer)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < EM_ZCL_URI_CLUSTER_ID_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, EM_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CORE_RD);
  finger += appendUriDelimiter(finger);

  return finger - start;
}

static uint16_t appendResourceTypeClusterId(char *finger,
                                            char *endOfBuffer,
                                            const EmberZclClusterSpec_t *spec)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < EM_ZCL_URI_CLUSTER_ID_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, EM_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CLUSTER_ID);
  finger += sprintf(finger, ":c"); // include trailing cluster tag
  if (spec != NULL) {
    finger += sprintf(finger,
                      ".%x.%c",
                      spec->id,
                      (spec->role == EMBER_ZCL_ROLE_CLIENT) ? 'c' : 's');
  }
  finger += appendUriDelimiter(finger);

  return finger - start;
}

static uint16_t appendDeviceIdAndEndpoint(char *finger,
                                          char *endOfBuffer,
                                          EmberZclDeviceId_t deviceId,
                                          EmberZclEndpointId_t endpointId)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger < EM_ZCL_URI_DEVICE_ID_AND_ENDPOINT_APPEND_MAX_LEN)
          || ((deviceId == EMBER_ZCL_DEVICE_ID_NULL)
              && (endpointId == EMBER_ZCL_ENDPOINT_NULL)))) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, EM_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT);
  finger += sprintf(finger, EM_ZCL_URI_QUERY_POSTFIX_DEVICE_ID);
  finger += sprintf(finger, "%x", deviceId);

  if (endpointId == EMBER_ZCL_ENDPOINT_NULL) {
    finger += sprintf(finger, "%c", EM_ZCL_URI_QUERY_WILDCARD);
  } else {
    finger += sprintf(finger, ".%x", endpointId);
  }

  finger += appendUriDelimiter(finger);

  return finger - start;
}

bool emZclUriBreak(char *finger)
{
  if (finger == NULL) {
    return false;
  }

  finger--;

  if (*finger == ';') {
    *finger = ',';
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------
// Discovery CLI utilities functions

EmberCoapContentFormatType discRequestAccept = EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR;
EmberZclDiscoveryRequestMode discRequestMode = EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY;
#define EM_ZCL_DISC_CLI_MAX_BUFFER_LEN (512)
char cliDiscoveryRequestUri[EM_ZCL_DISC_CLI_MAX_BUFFER_LEN] = { 0 };
char * cliDiscoveryRequestUriFinger = cliDiscoveryRequestUri;

bool emZclDiscSetAccept(EmberCoapContentFormatType accept)
{
  if (accept != EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR
      && accept != EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT) {
    return false;
  }
  discRequestAccept = accept;
  return true;
}

bool emberZclDiscSetMode(EmberZclDiscoveryRequestMode mode)
{
  if (mode >= EMBER_ZCL_DISCOVERY_REQUEST_MODE_MAX) {
    return false;
  }

  if (discRequestMode != mode) {
    emberZclDiscInit();
  }

  discRequestMode = (EmberZclDiscoveryRequestMode) mode;
  return true;
}

void emberZclDiscInit(void)
{
  const uint16_t wellKnownCoreLen = strlen(EM_ZCL_URI_WELL_KNOWN_CORE);

  MEMSET(cliDiscoveryRequestUri,
         0x00,
         sizeof(char) * EM_ZCL_DISC_CLI_MAX_BUFFER_LEN);
  cliDiscoveryRequestUriFinger = cliDiscoveryRequestUri;

  MEMCOPY(cliDiscoveryRequestUriFinger,
          EM_ZCL_URI_WELL_KNOWN_CORE,
          wellKnownCoreLen);
  cliDiscoveryRequestUriFinger += wellKnownCoreLen;

  MEMCOPY(cliDiscoveryRequestUriFinger++, "?", 1);
}

bool emberZclDiscSend(EmberCoapResponseHandler responseHandler)
{
  extern const Bytes16 emFf05AllCoapNodesMulticastAddress;

  emberAfAppPrintln("Sent discovery command: %s", cliDiscoveryRequestUri);

  EmberCoapSendInfo info = { 0 }; // use defaults

  EmberCoapOption options[] = {
    { EMBER_COAP_OPTION_ACCEPT, NULL, 1, EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR, },
  };
  // Override default link-format+cbor w/ link-format.
  if (discRequestAccept == EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT) {
    options[0].intValue = discRequestAccept;
  }

  info.options = options;
  info.numberOfOptions = 1;

  EmberStatus status = emberCoapGet((const EmberIpv6Address *)&emFf05AllCoapNodesMulticastAddress,
                                    (const uint8_t *) cliDiscoveryRequestUri,
                                    responseHandler,
                                    &info);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("Failed to send discovery message with error (0x%x)", status);
  } else {
    emberAfAppPrintln("%p 0x%x", "get", status);
  }

  return status == EMBER_SUCCESS;
}

bool emberZclDiscAppendQuery(EmZclDiscoveryRequest request,
                             EmberCoapResponseHandler responseHandler)
{
  EmZclCliDiscoveryRequestType type = request.type;
  char **finger, *end;

  if (discRequestMode == EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY) {
    emberZclDiscInit();
  }

  finger = &cliDiscoveryRequestUriFinger;
  end = &cliDiscoveryRequestUri[EM_ZCL_DISC_CLI_MAX_BUFFER_LEN];

  switch (type) {
    case EM_ZCL_CLI_DISCOVERY_REQUEST_FOR_CORE_RD:
      *finger += appendResourceTypeCoreRd(*finger, end);
      break;
    case EM_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_ID:
      *finger += appendResourceTypeClusterId(*finger, end, request.data.clusterSpec);
      break;
    case EM_ZCL_CLI_DISCOVERY_REQUEST_BY_DEVICE_ID:
      *finger += appendDeviceIdAndEndpoint(*finger,
                                           end,
                                           request.deviceId,
                                           EMBER_ZCL_ENDPOINT_NULL);
      break;
    case EM_ZCL_CLI_DISCOVERY_REQUEST_BY_ENDPOINT_AND_DEVICE_ID:
      *finger += appendEndpointId(*finger,
                                  end,
                                  request.data.endpointId,
                                  request.deviceId);
      break;
    case EM_ZCL_CLI_DISCOVERY_REQUEST_BY_UID_STRING:
      *finger += appendUidString(*finger, end, request.data.uidString);
      break;
    case EM_ZCL_CLI_DISCOVERY_REQUEST_BY_RESOURCE_VERSION:
      *finger += appendRevision(*finger, end, request.data.version, true);
      break;
    case EM_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_REVISION:
      *finger += appendRevision(*finger, end, request.data.version, false);
      break;
    default:
      return false;
  }

  if (discRequestMode == EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY) {
    return emberZclDiscSend(responseHandler);
  }

  return true;
}

bool emberZclDiscCoreRd(EmberCoapResponseHandler responseHandler)
{
  EmZclDiscoveryRequest request = { .type = EM_ZCL_CLI_DISCOVERY_REQUEST_FOR_CORE_RD };
  return emberZclDiscAppendQuery(request, responseHandler);
}

bool emberZclDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec,
                             EmberCoapResponseHandler responseHandler)
{
  EmZclDiscoveryRequest request = { .type = EM_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_ID,
                                    .data.clusterSpec = clusterSpec };
  return emberZclDiscAppendQuery(request, responseHandler);
}

bool emberZclDiscByEndpoint(EmberZclEndpointId_t endpointId,
                            EmberZclDeviceId_t deviceId,
                            EmberCoapResponseHandler responseHandler)
{
  EmZclDiscoveryRequest request = { .type = EM_ZCL_CLI_DISCOVERY_REQUEST_BY_ENDPOINT_AND_DEVICE_ID,
                                    .data.endpointId = endpointId,
                                    .deviceId = deviceId };
  return emberZclDiscAppendQuery(request, responseHandler);
}

bool emberZclDiscByUid(const EmberZclUid_t *uid,
                       uint16_t uidBits,
                       EmberCoapResponseHandler responseHandler)
{
  if (uidBits > EMBER_ZCL_UID_SIZE * 8) {
    return false;
  }

  uint8_t string[EMBER_ZCL_UID_BASE64URL_SIZE];
  uint8_t *finger = string;
  finger += emZclUidToBase64Url(uid, uidBits, finger);

  // A partially-specified UID becomes a prefix match.  Everything is NUL
  // terminated.
  if (finger != NULL) { // add a bounds check for MISRA compliance
    if (uidBits != EMBER_ZCL_UID_BITS) {
      *finger++ = EM_ZCL_URI_QUERY_WILDCARD;
    }
    *finger = '\0';
  }

  return emZclDiscByUidString(string, responseHandler);
}

bool emZclDiscByUidString(const uint8_t *uidString,
                          EmberCoapResponseHandler responseHandler)
{
  EmZclDiscoveryRequest request = {
    .type = EM_ZCL_CLI_DISCOVERY_REQUEST_BY_UID_STRING,
    .data.uidString = uidString,
  };
  return emberZclDiscAppendQuery(request, responseHandler);
}

bool emberZclDiscByResourceVersion(EmberZclClusterRevision_t version,
                                   EmberCoapResponseHandler responseHandler)
{
  EmZclDiscoveryRequest request = { .type = EM_ZCL_CLI_DISCOVERY_REQUEST_BY_RESOURCE_VERSION,
                                    .data.version = version };
  return emberZclDiscAppendQuery(request, responseHandler);
}

bool emberZclDiscByClusterRev(EmberZclClusterRevision_t version,
                              EmberCoapResponseHandler responseHandler)
{
  EmZclDiscoveryRequest request = { .type = EM_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_REVISION,
                                    .data.version = version };
  return emberZclDiscAppendQuery(request, responseHandler);
}

bool emberZclDiscByDeviceId(EmberZclDeviceId_t deviceId,
                            EmberCoapResponseHandler responseHandler)
{
  EmZclDiscoveryRequest request = { .type = EM_ZCL_CLI_DISCOVERY_REQUEST_BY_DEVICE_ID,
                                    .deviceId = deviceId };
  return emberZclDiscAppendQuery(request, responseHandler);
}

// ----------------------------------------------------------------------------
// Helper functions

static bool startsWith(const uint8_t *string, const uint8_t *prefix)
{
  while (*prefix != '\0') {
    if (*prefix != *string) {
      return false;
    }
    prefix++;
    string++;
  }
  return true;
}

// ----------------------------------------------------------------------------
// Discovery Payload Context: uri and attribute value string constructors
//

static int16_t constructResourceUriValueString(char *buffer,
                                               EmberZclEndpointId_t endpointId,
                                               const EmberZclClusterSpec_t *clusterSpec)
{
  if (buffer != NULL) {
    char *finger = buffer;
    finger += sprintf(finger, "/zcl");

    if (endpointId != EMBER_ZCL_ENDPOINT_NULL) {
      finger += sprintf(finger, "/e/%x", endpointId);
      if (clusterSpec != NULL) {
        finger += sprintf(finger,
                          "/%c%x",
                          (clusterSpec->role == EMBER_ZCL_ROLE_CLIENT) ? 'c' : 's',
                          clusterSpec->id);
      }
    }
    return finger - buffer;
  }
  return -1;
}

static int16_t constructRtValueString(char *buffer,
                                      const EmberZclClusterSpec_t *spec,
                                      bool includeTrailingClusterTag)
{
  if (buffer != NULL) {
    char *finger = buffer;
    finger += sprintf(finger, "urn:zcl");
    if (includeTrailingClusterTag) {
      finger += sprintf(finger, ":c");
    }
    if (spec != NULL) {
      finger += sprintf(finger,
                        ".%x.%c",
                        spec->id,
                        (spec->role == EMBER_ZCL_ROLE_CLIENT) ? 'c' : 's');
    }
    return finger - buffer;
  }
  return -1;
}

static int16_t constructIfValueString(char *buffer,
                                      EmberZclEndpointId_t endpointId,
                                      const EmberZclClusterSpec_t *clusterSpec)
{
  if (buffer != NULL) {
    char *finger = (char *)buffer;
    if (endpointId != EMBER_ZCL_ENDPOINT_NULL && clusterSpec != NULL) {
      EmberZclClusterRevision_t clusterRevision;
      EmberZclStatus_t status;

      status = emberZclReadAttribute(endpointId,
                                     clusterSpec,
                                     EMBER_ZCL_ATTRIBUTE_CLUSTER_REVISION,
                                     &clusterRevision,
                                     sizeof(clusterRevision));

      if (status != EMBER_ZCL_STATUS_SUCCESS) {
        return -1;
      }
      finger += sprintf(finger, "urn:zcl:c.v%x", clusterRevision);
    } else {
      finger += sprintf(finger, "urn:zcl:v0");
    }
    return finger - buffer;
  }
  return -1;
}

static int16_t constructEpValueString(char *buffer)
{
  if (buffer != NULL) {
    char *finger = buffer;
    finger += sprintf(finger, "ni:///sha-256;");
    finger += emZclUidToBase64Url(&emZclUid, EMBER_ZCL_UID_BITS, (uint8_t *)finger);
    return finger - buffer;
  }
  return -1;
}

static int16_t constructZeValueString(char *buffer,
                                      EmberZclEndpointId_t endpointId,
                                      EmberZclDeviceId_t deviceId)
{
  if (buffer != NULL) {
    char *finger = (char *)buffer;
    if (endpointId != EMBER_ZCL_ENDPOINT_NULL) {
      finger += sprintf(finger, "urn:zcl:d.%x.%x", deviceId, endpointId);
    }
    return finger - buffer;
  }
  return -1;
}

// ----------------------------------------------------------------------------
// Discovery Payload Context: link-format implementation
//
// Payload structure is a list of links in link-format structured text.

static bool appendPayloadLinkFormat(EmZclDiscPayloadContext_t *dpc, uint8_t *data, uint16_t dataLength)
{
  if (dpc->status == DISCOVERY_PAYLOAD_CONTEXT_STATUS_SUCCESS) {
    const uint8_t *end = dpc->cborState.end;
    uint8_t *finger = dpc->cborState.finger;
    if (dataLength <= end - finger) {
      MEMCOPY(finger, data, dataLength);
      dpc->cborState.finger += dataLength;
      return true;
    } else {
      dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
      return false;
    }
  }
  return false;
}

static void separateLinkLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  if (dpc->linkCount != 0) {
    appendPayloadLinkFormat(dpc, (uint8_t *)",", 1);
  }
}

static void separateAttrLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  appendPayloadLinkFormat(dpc, (uint8_t *)";", 1);
}

static bool payloadIsEmptyLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  // Zero length.
  return (dpc->payloadLength(dpc) == 0);
}

static void startPayloadLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  // Nothing to do.
}

static void startLinkLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  separateLinkLinkFormat(dpc);
  dpc->linkCount++;
}

static void addResourceUriLinkFormat(EmZclDiscPayloadContext_t *dpc,
                                     EmberZclEndpointId_t endpointId,
                                     const EmberZclClusterSpec_t *clusterSpec)
{
  // Construct value string.
  char buffer[256] = { 0 };
  char *finger = (char *)buffer;
  *finger++ = '<';

  int16_t ret;
  if ((ret = constructResourceUriValueString(finger, endpointId, clusterSpec)) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  finger += ret;
  *finger++ = '>';
  *finger = '\0';

  appendPayloadLinkFormat(dpc, (uint8_t *)buffer, finger - buffer);
}

static void addRtLinkFormat(EmZclDiscPayloadContext_t *dpc,
                            const EmberZclClusterSpec_t *spec,
                            bool includeTrailingClusterTag)
{
  separateAttrLinkFormat(dpc);

  // Construct value string.
  char buffer[50] = { 0 };
  char *finger = (char *)buffer;
  *finger++ = 'r';
  *finger++ = 't';
  *finger++ = '=';

  int16_t ret;
  if ((ret = constructRtValueString(finger, spec, includeTrailingClusterTag)) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  finger += ret;

  appendPayloadLinkFormat(dpc, (uint8_t *)buffer, finger - buffer);
  dpc->attrCount++;
}

static void addIfLinkFormat(EmZclDiscPayloadContext_t *dpc,
                            EmberZclEndpointId_t endpointId,
                            const EmberZclClusterSpec_t *clusterSpec)
{
  separateAttrLinkFormat(dpc);

  char buffer[50] = { 0 };
  char *finger = (char *)buffer;
  *finger++ = 'i';
  *finger++ = 'f';
  *finger++ = '=';

  int16_t ret;
  if ((ret = constructIfValueString(finger, endpointId, clusterSpec)) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  finger += ret;

  appendPayloadLinkFormat(dpc, (uint8_t *)buffer, finger - buffer);
  dpc->attrCount++;
}

static void addEpLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  separateAttrLinkFormat(dpc);

  // Construct value string.
  char buffer[EM_ZCL_URI_DEVICE_UID_APPEND_MAX_LEN] = { 0 };
  char *finger = (char *)buffer;

  *finger++ = 'e';
  *finger++ = 'p';
  *finger++ = '=';
  *finger++ = '"';

  int16_t ret;
  if ((ret = constructEpValueString(finger)) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  finger += ret;
  *finger++ = '"';

  appendPayloadLinkFormat(dpc, (uint8_t *)buffer, finger - buffer);
  dpc->attrCount++;
}

static void addZeLinkFormat(EmZclDiscPayloadContext_t *dpc,
                            EmberZclEndpointId_t endpointId,
                            EmberZclDeviceId_t deviceId)
{
  separateAttrLinkFormat(dpc);

  // Construct value string.
  char buffer[50] = { 0 };
  char *finger = (char *)buffer;
  *finger++ = 'z';
  *finger++ = 'e';
  *finger++ = '=';

  int16_t ret;
  if ((ret = constructZeValueString(finger, endpointId, deviceId)) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  finger += ret;

  appendPayloadLinkFormat(dpc, (uint8_t *)buffer, finger - buffer);
  dpc->attrCount++;
}

static void endLinkLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  dpc->linkCount++;
}

static void endPayloadLinkFormat(EmZclDiscPayloadContext_t *dpc)
{
  // Nothing to do.
}

// ----------------------------------------------------------------------------
// Discovery Payload Context: link-format+cbor implemementation
//
// Payload structure is a CBOR array containing zero or more CBOR maps,
// each map representing a link.

static bool payloadIsEmptyLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc)
{
  // Either zero length, or first byte signifies empty CBOR array.
  return (dpc->payloadLength(dpc) == 0 || *(dpc->payloadPointer(dpc)) == 0x80);
}

static void startPayloadLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc)
{
  // Open CBOR array.
  if (!emCborEncodeIndefiniteArray(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

static void startLinkLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc)
{
  // Open CBOR map.
  if (!emCborEncodeIndefiniteMap(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

static void addResourceUriLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc,
                                             EmberZclEndpointId_t endpointId,
                                             const EmberZclClusterSpec_t *clusterSpec)
{
  // Construct value string.
  char buffer[256] = { 0 };

  if (constructResourceUriValueString(buffer, endpointId, clusterSpec) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Resource URI (href) is numeric map key 1.
  if (!emCborEncodeMapEntry(&dpc->cborState, 1, EMBER_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

static void addRtLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc,
                                    const EmberZclClusterSpec_t *spec,
                                    bool includeTrailingClusterTag)
{
  // Construct value string.
  char buffer[50] = { 0 };

  if (constructRtValueString(buffer, spec, includeTrailingClusterTag) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Resource Type (rt) is numeric map key 9.
  if (!emCborEncodeMapEntry(&dpc->cborState, 9, EMBER_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void addIfLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc,
                                    EmberZclEndpointId_t endpointId,
                                    const EmberZclClusterSpec_t *clusterSpec)
{
  // Construct value string.
  char buffer[50] = { 0 };

  if (constructIfValueString(buffer, endpointId, clusterSpec) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Interface (if) is numeric key map 10.
  if (!emCborEncodeMapEntry(&dpc->cborState, 10, EMBER_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void addEpLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc)
{
  // Construct value string.
  char buffer[EM_ZCL_URI_DEVICE_UID_APPEND_MAX_LEN] = { 0 };

  if (constructEpValueString(buffer) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Endpoint (ep) is text key map "ep".
  if (!emCborEncodeValue(&dpc->cborState, EMBER_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)"ep")
      || !emCborEncodeValue(&dpc->cborState, EMBER_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void addZeLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc,
                                    EmberZclEndpointId_t endpointId,
                                    EmberZclDeviceId_t deviceId)
{
  // Construct value string.
  char buffer[50] = { 0 };

  if (constructZeValueString(buffer, endpointId, deviceId) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Zigbee Endpoint (ze) is text key map "ze".
  if (!emCborEncodeValue(&dpc->cborState, EMBER_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)"ze")
      || !emCborEncodeValue(&dpc->cborState, EMBER_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void endLinkLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc)
{
  // Close CBOR map.
  if (!emCborEncodeBreak(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->linkCount++;
}

static void endPayloadLinkFormatPlusCbor(EmZclDiscPayloadContext_t *dpc)
{
  // Close CBOR array.
  if (!emCborEncodeBreak(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

// ----------------------------------------------------------------------------
// Discovery Payload Context: common

static const uint8_t *payloadPointerCommon(EmZclDiscPayloadContext_t *dpc)
{
  return dpc->cborState.start;
}

static uint16_t payloadLengthCommon(EmZclDiscPayloadContext_t *dpc)
{
  return emCborEncodeSize(&dpc->cborState);
}

static void appendUidCommon(EmZclDiscPayloadContext_t *dpc)
{
  dpc->startLink(dpc);
  dpc->addResourceUri(dpc, EMBER_ZCL_ENDPOINT_NULL, NULL);
  dpc->addRt(dpc, NULL, false);
  dpc->addIf(dpc, EMBER_ZCL_ENDPOINT_NULL, NULL);
  dpc->addEp(dpc);
  dpc->endLink(dpc);
}

bool emZclInitDiscPayloadContext(EmZclDiscPayloadContext_t *dpc,
                                 EmberCoapContentFormatType contentFormat,
                                 uint8_t *buffer,
                                 uint16_t bufferLength)
{
  if (dpc == NULL || buffer == NULL || bufferLength == 0) {
    return false;
  }

  MEMSET(dpc, 0, sizeof(EmZclDiscPayloadContext_t));
  dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_SUCCESS;
  dpc->contentFormat = contentFormat;

  // Leverage the start/finger/end pointers in CborState for payload buffer management
  // of other content-format types.
  emCborEncodeStart(&dpc->cborState, buffer, bufferLength);

  switch (contentFormat) {
    case EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT:
      dpc->payloadIsEmpty = payloadIsEmptyLinkFormat;
      dpc->payloadPointer = payloadPointerCommon;
      dpc->payloadLength = payloadLengthCommon;
      dpc->startPayload = startPayloadLinkFormat;
      dpc->startLink = startLinkLinkFormat;
      dpc->addResourceUri = addResourceUriLinkFormat;
      dpc->addRt = addRtLinkFormat;
      dpc->addIf = addIfLinkFormat;
      dpc->addEp = addEpLinkFormat;
      dpc->addZe = addZeLinkFormat;
      dpc->endLink = endLinkLinkFormat;
      dpc->endPayload = endPayloadLinkFormat;
      dpc->appendUid = appendUidCommon;
      return true;

    case EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR:
      dpc->payloadIsEmpty = payloadIsEmptyLinkFormatPlusCbor;
      dpc->payloadPointer = payloadPointerCommon;
      dpc->payloadLength = payloadLengthCommon;
      dpc->startPayload = startPayloadLinkFormatPlusCbor;
      dpc->startLink = startLinkLinkFormatPlusCbor;
      dpc->addResourceUri = addResourceUriLinkFormatPlusCbor;
      dpc->addRt = addRtLinkFormatPlusCbor;
      dpc->addIf = addIfLinkFormatPlusCbor;
      dpc->addEp = addEpLinkFormatPlusCbor;
      dpc->addZe = addZeLinkFormatPlusCbor;
      dpc->endLink = endLinkLinkFormatPlusCbor;
      dpc->endPayload = endPayloadLinkFormatPlusCbor;
      dpc->appendUid = appendUidCommon;
      return true;

    default:
      // Unsupported content-format.
      dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
      return false;
  }
}
