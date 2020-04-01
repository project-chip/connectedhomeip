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
#include <stdio.h>
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
#include CHIP_AF_API_ZCL_CORE_RESOURCE_DIRECTORY

static uint16_t appendResourceTypeCoreRd(char *finger,
                                         char *endOfBuffer);
static uint16_t appendResourceTypeClusterId(char *finger,
                                            char *endOfBuffer,
                                            const ChipZclClusterSpec_t *spec);
static uint16_t appendEndpointId(char *finger,
                                 char *endOfBuffer,
                                 ChipZclEndpointId_t endpointId,
                                 ChipZclDeviceId_t deviceId);
static uint16_t appendRevision(char *finger,
                               char *endOfBuffer,
                               const uint16_t version,
                               bool isProtocolRevision);
static bool startsWith(const uint8_t *string, const uint8_t *prefix);

uint16_t appendUriDelimiter(char *finger);

#define CH_ZCL_URI_PATH_APPEND_MAX_LEN (17)
#define CH_ZCL_URI_ENDPOINT_APPEND_MAX_LEN (17)
#define CH_ZCL_URI_CLUSTER_REVISION_APPEND_MAX_LEN (20)
#define CH_ZCL_URI_CLUSTER_ID_APPEND_MAX_LEN (20)
#define CH_ZCL_URI_DEVICE_ID_AND_ENDPOINT_APPEND_MAX_LEN (16)

// max wildcard string length = type_size * 2 + nul
#define CH_ZCL_WILDCARD_COMPARE_STRING_MAX_LEN                     \
  ((sizeof(ChipZclClusterId_t) > sizeof(ChipZclDeviceId_t))      \
   ? (sizeof(ChipZclClusterId_t) * 2 + CHIP_ZCL_STRING_OVERHEAD) \
   : (sizeof(ChipZclDeviceId_t) * 2 + CHIP_ZCL_STRING_OVERHEAD))

// Enums for helping with CLI commands.
typedef enum {
  CH_ZCL_CLI_DISCOVERY_REQUEST_FOR_CORE_RD,
  CH_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_ID,
  CH_ZCL_CLI_DISCOVERY_REQUEST_BY_DEVICE_ID,
  CH_ZCL_CLI_DISCOVERY_REQUEST_BY_ENDPOINT_AND_DEVICE_ID,
  CH_ZCL_CLI_DISCOVERY_REQUEST_BY_UID_STRING,
  CH_ZCL_CLI_DISCOVERY_REQUEST_BY_RESOURCE_VERSION, // if=urn:zcl:v0
  CH_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_REVISION, // if=urn:zcl:c.v0
} ChZclCliDiscoveryRequestType;

typedef struct {
  ChZclCliDiscoveryRequestType type;
  union {
    const ChipZclClusterSpec_t *clusterSpec;
    ChipZclEndpointId_t endpointId;
    const uint8_t *uidString;
    ChipZclClusterRevision_t version;
  } data;
  ChipZclDeviceId_t deviceId;
} ChZclDiscoveryRequest;

// URI handlers
static void wellKnownUriHandler(ChZclContext_t *context);
static void wellKnownUriQueryHandler(ChZclContext_t *context);
static bool wellKnownUriQueryDeviceTypeAndEndpointParse(ChZclContext_t *context,
                                                        void *castString,
                                                        uint8_t depth);
static bool wellKnownUriQueryVersionParse(ChZclContext_t *context,
                                          void *castString,
                                          uint8_t depth);
static bool wellKnownUriQueryClusterIdParse(ChZclContext_t *context,
                                            void *castString,
                                            uint8_t depth);
static bool wellKnownUriQueryUidParse(ChZclContext_t *context,
                                      void *data,
                                      uint8_t depth);

static ChZclUriQuery wellKnownUriQueries[] = {
  // target attributes / relation type
  { chZclUriQueryStringPrefixMatch, CH_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CLUSTER_ID, wellKnownUriQueryClusterIdParse              },
  { chZclUriQueryStringPrefixMatch, CH_ZCL_URI_QUERY_PREFIX_VERSION, wellKnownUriQueryVersionParse                },
  { chZclUriQueryStringPrefixMatch, CH_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT, wellKnownUriQueryDeviceTypeAndEndpointParse  },
  { chZclUriQueryStringPrefixMatch, CH_ZCL_URI_QUERY_UID, wellKnownUriQueryUidParse                    },

  // terminator
  { NULL, NULL, NULL                                  },
};

ChZclUriPath chZclWellKnownUriPaths[] = {
  // .well-known/core
  {   1, 255, CH_ZCL_URI_FLAG_METHOD_GET | CH_ZCL_URI_FLAG_FORMAT_LINK, chZclUriPathStringMatch, CH_ZCL_URI_WELL_KNOWN, NULL, NULL },
  { 255, 255, CH_ZCL_URI_FLAG_METHOD_GET | CH_ZCL_URI_FLAG_FORMAT_LINK, chZclUriPathStringMatch, CH_ZCL_URI_CORE, wellKnownUriQueries, wellKnownUriHandler },
};

//----------------------------------------------------------------
// URI segment matching functions

bool emUriQueryZclStringtoCluster(ChZclContext_t *context,
                                  uint8_t depth,
                                  const uint16_t offset)
{
  char *finger = (char *)context->uriQuery[depth] + offset;
  char *end = (char *)(context->uriQuery[depth] + context->uriQueryLength[depth]);

  if (*finger != ':') { // verify ':' in zcl:c
    return false;
  }
  finger += 2;
  if (*finger != CH_ZCL_URI_QUERY_DOT) { // verify '.' in "zcl:c."
    return false;
  }
  ++finger;

  char *roleOptionDot = memchr(finger,
                               CH_ZCL_URI_QUERY_DOT,
                               end - finger);
  char *wildcard = memchr(finger,
                          CH_ZCL_URI_QUERY_WILDCARD,
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
    ChipZclClusterSpec_t *clusterSpec = &context->clusterSpec;
    clusterSpec->manufacturerCode = CHIP_ZCL_MANUFACTURER_CODE_NULL;
    context->mask |= CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUSTER_AND_ROLE;
    size_t len = endOfClsId - finger;

    if (len == 0) {
      if (wildcard) { // zcl:c.*
        context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD_ALL;
        return true;
      } else {
        return false;  // clusterId length must be > 0.
      }
    }

    // parse clusterId.
    uintmax_t clusterId;
    if (!chZclHexStringToInt((const uint8_t *)finger,
                             len,
                             &clusterId)) {
      return false;
    }
    context->clusterSpec.id = clusterId;

    if (roleOptionDot && wildcard) {
      if (roleOptionDot < wildcard) { // zcl:c.<cluster_id>.*
        context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ID;
        return true;
      } else { // zcl:c.<cluster_id>*.
        assert(0); // returned false earlier for this, so should not get here.
      }
    } else if (roleOptionDot && !wildcard) { // zcl:c.<cluster_id>.c/s
      context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ID;
      finger = roleOptionDot + 1;
      if (finger[0] == 'c') {
        clusterSpec->role = CHIP_ZCL_ROLE_CLIENT;
        context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ROLE;
        return true;
      } else if (finger[0] == 's') {
        clusterSpec->role = CHIP_ZCL_ROLE_SERVER;
        context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ROLE;
        return true;
      } else {
        return false;
      }
    } else if (!roleOptionDot && wildcard) { // zcl:c.<cluster_id>*
      context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD;
      return true;
    } else { // zcl:c.<cluster_id>
      assert(0); // returned false earlier for this, so should not get here.
    }
  }

  return false;
}

static bool wellKnownUriQueryDeviceTypeAndEndpointParse(ChZclContext_t *context,
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
                 CH_ZCL_URI_QUERY_POSTFIX_DEVICE_ID,
                 strlen(CH_ZCL_URI_QUERY_POSTFIX_DEVICE_ID)) == 0) {
    uintmax_t deviceId;
    uintmax_t endpointId;
    char *dot = strchr(finger, CH_ZCL_URI_QUERY_DOT);
    if (dot == NULL) {
      return false; // must have at least "zcl:d."
    }
    finger = dot + 1;

    char *zeOptionDot = memchr(finger, CH_ZCL_URI_QUERY_DOT, end - finger);
    char *wildcard = memchr(finger, CH_ZCL_URI_QUERY_WILDCARD, end - finger);
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
      context->mask |= CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_DEVICE_AND_ENDPOINT;
      size_t len = endOfDeviceId - finger;

      if (len == 0) {
        if (wildcard) { // zcl:d.*
          return true; // no mask here so that we return everything
        } else {
          return false;  // deviceId length must be > 0.
        }
      }

      // parse deviceId.
      if (!(chZclHexStringToInt((const uint8_t *)finger,
                                len,
                                &deviceId))) {
        return false;
      }

      context->deviceId = deviceId;

      if (zeOptionDot && wildcard) {
        if (zeOptionDot < wildcard) { // zcl:d.<device_id>.*
          context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ID;
          return true;
        } else { // zcl:d.<device_id>*.
          assert(0); // returned false earlier for this, so should not get here.
        }
      } else if (zeOptionDot && !wildcard) { // zcl:d.<device_id>.<endpoint_id>
        context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ID;
        finger = zeOptionDot + 1;
        if (chZclHexStringToInt((const uint8_t *)finger,
                                end - finger,
                                &endpointId)) {
          const ChZclEndpointEntry_t *ep = chZclFindEndpoint(endpointId);
          context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ENDPOINT;
          context->endpoint = ep; // (ep may be NULL here).
          return true;
        }
      } else if (!zeOptionDot && wildcard) { // zcl:d.<device_id>*
        context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WILDCARD;
        return true;
      } else { // zcl:d.<device_id>
        assert(0); // returned false earlier for this, so should not get here.
      }
    }
  }

  return false;
}

static bool wellKnownUriQueryVersionParse(ChZclContext_t *context,
                                          void *castString,
                                          uint8_t depth)
{
  char *string = (char *)castString;
  const uint16_t length = strlen(string);
  uintmax_t clusterRevision;

  // is it the weird c.v# or just v#?
  char *start = (char *)context->uriQuery[depth];
  char *dot = strstr(start + length, CH_ZCL_URI_QUERY_VERSION_KEY);
  char *end = start + context->uriQueryLength[depth];
  bool status = false;

  if (context->uriQueryLength[depth] <= length) {
    return false;
  }

  if (dot != NULL) { // c.v#
    // (note- "...zcl:c.v*" wildcard is rejected).
    status = chZclHexStringToInt((const uint8_t *)dot + strlen(CH_ZCL_URI_QUERY_VERSION_KEY),
                                 (size_t)(end - dot - strlen(CH_ZCL_URI_QUERY_VERSION_KEY)),
                                 &clusterRevision);
    if (status) {
      context->clusterRevision = clusterRevision;
      context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUSTER_REVISION;
      return true;
    }
  } else { // v#
    // ".well-known/core?if=urn:zcl:vX"
    // wildcard "zcl:*" should be allowed.
    uint8_t *tmp = (uint8_t *)start + length;
    if (tmp[0] == CH_ZCL_URI_QUERY_WILDCARD) {
      context->mask |= CH_ZCL_DISCOVERY_CONTEXT_FILTER_RESOURCE_WILDCARD;
      context->clusterRevision = 0;  // We only support v0.
      return true;
    } else if (tmp[0] == 'v') {
      tmp++;
      status = chZclHexStringToInt(tmp,
                                   (context->uriQueryLength[depth] - length - 1),
                                   &clusterRevision);

      if (status) {
        context->mask |= CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_RESOURCE_VERSION;
        context->clusterRevision = clusterRevision;
        return true;
      }
    }
  }

  return false;
}

static bool wellKnownUriQueryClusterIdParse(ChZclContext_t *context,
                                            void *castString,
                                            uint8_t depth)
{
  char *string = (char *) castString;
  const uint16_t length = strlen(string);

  if (context->uriQueryLength[depth] == length) {
    context->mask |= CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUS;
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

static bool wellKnownUriQueryUidParse(ChZclContext_t *context,
                                      void *data,
                                      uint8_t depth)
{
  context->mask |= CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID;
  uint16_t length = context->uriQueryLength[depth];

  // Save reference to UID filter string.
  context->uidFilterString = (context->uriQuery[depth] + strlen((const char *)data));
  context->uidFilterLength = length - strlen((const char *)data);

  // if only one char, it should be wildcard
  if (context->uidFilterLength == 1) {
    if (context->uriQuery[depth][length - 1] == CH_ZCL_URI_QUERY_WILDCARD) {
      context->uidFilterLength--;
      return true;
    } else {
      return false;
    }
  }

  // if <= "ep=ni:///sha-256;" chars, it should end with wildcard
  if (context->uidFilterLength <= strlen(CH_ZCL_URI_QUERY_UID_SHA_256)) {
    if (context->uriQuery[depth][length - 1] == CH_ZCL_URI_QUERY_WILDCARD
        && (strncmp(CH_ZCL_URI_QUERY_UID_SHA_256,
                    (const char *) context->uidFilterString,
                    context->uidFilterLength - 1) == 0)) { // get rid of ending * from length
      context->uidFilterLength--;
      return true;
    } else {
      return false;
    }
  }

  // if > "ep=ni:///sha-256;" chars, then remove the "ep=ni:///sha-256;"
  context->uidFilterString += strlen(CH_ZCL_URI_QUERY_UID_SHA_256);
  context->uidFilterLength -= strlen(CH_ZCL_URI_QUERY_UID_SHA_256);

  // if ends with wildcard, remove the wildcard char from length
  if (context->uriQuery[depth][length - 1] == CH_ZCL_URI_QUERY_WILDCARD) {
    context->mask |= CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID_PREFIX;
    context->uidFilterLength--;
  } else if (context->uidFilterLength != CHIP_ZCL_UID_BASE64URL_LENGTH) {
    return false; // if no wildcard, then we should have the whole UID
  }

  // Validate that UID filter string contains only base64url characters.
  return chZclConvertBase64UrlToCode(context->uidFilterString,
                                     context->uidFilterLength,
                                     NULL);
}

static void wellKnownUriHandler(ChZclContext_t *context)
{
  if (context->mask == 0) {
    if (emIsMulticastAddress(context->info->localAddress.bytes)) {
      chipAfAppPrintln("Suppressing response to multicast discovery request - invalid request.");
    } else if ((context->uriPathSegments >= 2)
               && (MEMCOMPARE(context->uriPath[0], CH_ZCL_URI_WELL_KNOWN, strlen(CH_ZCL_URI_WELL_KNOWN)) == 0)
               && (MEMCOMPARE(context->uriPath[1], CH_ZCL_URI_CORE, strlen(CH_ZCL_URI_CORE)) == 0)) {
      wellKnownUriQueryHandler(context);       // If context is link-format+cbor this will send an empty Array response, i.e. [].
    } else {
      chZclRespond405MethodNotAllowed(context->info);
    }
  } else {
    wellKnownUriQueryHandler(context);
  }
}

// get well-known/core?a=A&b=B:
static void wellKnownUriQueryHandler(ChZclContext_t *context)
{
  // code to reply to discovery by both endpoint and class type.

  bool clusterFound = true;

  uint8_t wildcardComparePrefix[CH_ZCL_WILDCARD_COMPARE_STRING_MAX_LEN];
  uint8_t wildcardCompareString[CH_ZCL_WILDCARD_COMPARE_STRING_MAX_LEN];

  if (chipAfPluginResourceDirectoryHaveRegistered()) {
    // Do not respond to queries when we have already registered with an RD server
    chZclRespond404NotFound(context->info);
    return;
  }

  if (context->mask == CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_NONE) {
    return;
  }

  // Check for Accept option, assume default link-format+cbor if it is absent.
  ChipCoapContentFormatType accept = CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT;
  uint32_t optionValue;
  if (chipReadIntegerOption(context->options, CHIP_COAP_OPTION_ACCEPT, &optionValue)) {
    accept = (ChipCoapContentFormatType)optionValue;
  }

  char payload[CH_ZCL_MAX_WELL_KNOWN_REPLY_PAYLOAD];
  MEMSET(payload, 0, COUNTOF(payload));

  ChZclDiscPayloadContext_t dpc;
  if (!chZclInitDiscPayloadContext(&dpc, accept, (uint8_t *)payload, CH_ZCL_MAX_WELL_KNOWN_REPLY_PAYLOAD)) {
    // No payload implementation to support the format specified by Accept option.
    chZclRespond406NotAcceptable(context->info);
    return;
  }

  dpc.startPayload(&dpc);
  if (context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_RESOURCE_WILDCARD) {
    // GET .well-known/core?if=urn:zcl:* -- return all resources under /zcl/
    dpc.appendUid(&dpc);

    size_t i;
    for (i = 0; i < chZclEndpointCount; i++) {
      const ChZclEndpointEntry_t *epEntry = &chZclEndpointTable[i];
      const ChipZclClusterSpec_t **clusterSpecs = epEntry->clusterSpecs;
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
  } else if ((context->mask & CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUS)
             || (context->mask & CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_RESOURCE_VERSION)) {
    // /.well-known/core?rt=urn:zcl response to signal ZCLIP is supported.
    if (context->clusterRevision == 0) {   // we only support resource==0.
      dpc.appendUid(&dpc);
    }
  } else if ((context->mask & CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_DEVICE_AND_ENDPOINT)
             || (context->mask & CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_ZCLIP_SUPPORT_BY_CLUSTER_AND_ROLE)
             || (context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUSTER_REVISION)) {
    clusterFound = false;
    size_t i;
    for (i = 0; i < chZclEndpointCount; i++) {
      const ChZclEndpointEntry_t *epEntry = &chZclEndpointTable[i];
      const ChipZclClusterSpec_t **clusterSpecs = epEntry->clusterSpecs;
      const ChipZclClusterSpec_t *spec = NULL;
      spec = *clusterSpecs;
      while (spec != NULL) {
        do {
          if (context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WILDCARD) {
            chZclIntToHexString(epEntry->deviceId, sizeof(ChipZclDeviceId_t), wildcardCompareString);
            chZclIntToHexString(context->deviceId, sizeof(ChipZclDeviceId_t), wildcardComparePrefix);
            if (!startsWith(wildcardCompareString, wildcardComparePrefix)) {
              break;
            }
          }
          if ((context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ID)
              && (context->deviceId != epEntry->deviceId)) {
            break;
          }
          if ((context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_DEVICE_ID_WITH_ENDPOINT)
              && ((context->endpoint == NULL)
                  || (epEntry->endpointId != context->endpoint->endpointId))) {
            break;
          }
          if ((spec->manufacturerCode != CHIP_ZCL_MANUFACTURER_CODE_NULL)
              && (spec->manufacturerCode != context->clusterSpec.manufacturerCode)) {
            break;
          }
          if (context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD) {
            chZclIntToHexString(spec->id, sizeof(ChipZclClusterId_t), wildcardCompareString);
            chZclIntToHexString(context->clusterSpec.id, sizeof(ChipZclClusterId_t), wildcardComparePrefix);
            if (!startsWith(wildcardCompareString, wildcardComparePrefix)) {
              break;
            }
          }
          if ((context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ID)
              && (spec->id != context->clusterSpec.id)) {
            break;
          }
          if ((context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WITH_ROLE)
              && (spec->role != context->clusterSpec.role)) {
            break;
          }
          if ((context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUSTER_REVISION)) {
            ChipZclClusterRevision_t clusterRevision;
            ChipZclStatus_t status;
            status = chipZclReadAttribute(epEntry->endpointId,
                                           spec,
                                           CHIP_ZCL_ATTRIBUTE_CLUSTER_REVISION,
                                           &clusterRevision,
                                           sizeof(clusterRevision));

            if ((status != CHIP_ZCL_STATUS_SUCCESS)
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

        if (context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD_ALL
            || spec->id == context->clusterSpec.id) {
          clusterFound = true;
        } else if (context->mask & CH_ZCL_DISCOVERY_CONTEXT_FILTER_BY_CLUS_ID_WILDCARD) {
          if (startsWith(wildcardCompareString, wildcardComparePrefix)) {
            clusterFound = true;
          }
        }
        clusterSpecs++;
        spec = *clusterSpecs;
      } // while
    } // for
  } else if (context->mask & CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID) {
    uint8_t uidb64u[CHIP_ZCL_UID_BASE64URL_SIZE];
    size_t uidb64uLen = chZclUidToBase64Url(&chZclUid, CHIP_ZCL_UID_BITS, uidb64u);
    if (uidb64uLen == CHIP_ZCL_UID_BASE64URL_LENGTH
        && context->uidFilterLength <= CHIP_ZCL_UID_BASE64URL_LENGTH) {
      if (context->mask & CH_ZCL_DISCOVERY_CONTEXT_QUERY_FOR_UID_PREFIX // prefix wildcard
          || context->uidFilterLength == CHIP_ZCL_UID_BASE64URL_LENGTH) { // full match
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
      chZclRespond413RequestEntityTooLarge(context->info);
    } else {
      chZclRespond500InternalServerError(context->info);
    }
    return;
  }

  if (emIsMulticastAddress(context->info->localAddress.bytes)
      && dpc.payloadIsEmpty(&dpc)) {
    chipAfAppPrintln("Suppressing response to multicast discovery request - no payload.");
  } else {
    if ((dpc.contentFormat != CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR)
        && (!clusterFound)) {
      chZclRespond404NotFound(context->info);
    } else {
      chZclRespond205ContentLinkFormat(context->info,
                                       dpc.payloadPointer(&dpc),
                                       dpc.payloadLength(&dpc),
                                       dpc.contentFormat);
    }
  }
}

//----------------------------------------------------------------
// URI segment matching functions

// append "</zcl/e/EE/[cs]CCCC>;" without quotes.
uint16_t chZclUriAppendUriPath(char *finger,
                               char *endOfBuffer,
                               ChipZclEndpointId_t endpointId,
                               const ChipZclClusterSpec_t *clusterSpec)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < CH_ZCL_URI_PATH_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;
  finger += sprintf(finger, "</zcl");

  if (endpointId != CHIP_ZCL_ENDPOINT_NULL) {
    finger += sprintf(finger, "/e/%d", endpointId);
    if (clusterSpec != NULL) {
      finger += sprintf(finger,
                        "/%c%x",
                        (clusterSpec->role == CHIP_ZCL_ROLE_CLIENT) ? 'c' : 's',
                        clusterSpec->id);
    }
  }

  finger += sprintf(finger, ">;");

  return finger - start;
}

static uint16_t appendEndpointId(char *finger,
                                 char *endOfBuffer,
                                 ChipZclEndpointId_t endpointId,
                                 ChipZclDeviceId_t deviceId)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < CH_ZCL_URI_ENDPOINT_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;

  if (endpointId != CHIP_ZCL_ENDPOINT_NULL) {
    finger += sprintf(finger,
                      "%s%s%x.%d",
                      CH_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT,
                      CH_ZCL_URI_QUERY_POSTFIX_DEVICE_ID,
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
      && ((endOfBuffer - finger) < (strlen(CH_ZCL_URI_QUERY_UID_SHA_256_PREFIX) + length + 1))) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, CH_ZCL_URI_QUERY_UID_SHA_256_PREFIX);  // add uid prefix.
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
      && ((endOfBuffer - finger) < CH_ZCL_URI_CLUSTER_REVISION_APPEND_MAX_LEN)) {
    return 0;
  }

  char * start = finger;

  finger += sprintf(finger, isProtocolRevision ? CH_ZCL_URI_QUERY_PROTOCOL_REVISION_FORMAT : CH_ZCL_URI_QUERY_CLUSTER_REVISION_FORMAT, revision);
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
      && ((endOfBuffer - finger) < CH_ZCL_URI_CLUSTER_ID_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, CH_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CORE_RD);
  finger += appendUriDelimiter(finger);

  return finger - start;
}

static uint16_t appendResourceTypeClusterId(char *finger,
                                            char *endOfBuffer,
                                            const ChipZclClusterSpec_t *spec)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger) < CH_ZCL_URI_CLUSTER_ID_APPEND_MAX_LEN)) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, CH_ZCL_URI_QUERY_PREFIX_RESOURCE_TYPE_CLUSTER_ID);
  finger += sprintf(finger, ":c"); // include trailing cluster tag
  if (spec != NULL) {
    finger += sprintf(finger,
                      ".%x.%c",
                      spec->id,
                      (spec->role == CHIP_ZCL_ROLE_CLIENT) ? 'c' : 's');
  }
  finger += appendUriDelimiter(finger);

  return finger - start;
}

static uint16_t appendDeviceIdAndEndpoint(char *finger,
                                          char *endOfBuffer,
                                          ChipZclDeviceId_t deviceId,
                                          ChipZclEndpointId_t endpointId)
{
  if ((endOfBuffer != NULL)
      && ((endOfBuffer - finger < CH_ZCL_URI_DEVICE_ID_AND_ENDPOINT_APPEND_MAX_LEN)
          || ((deviceId == CHIP_ZCL_DEVICE_ID_NULL)
              && (endpointId == CHIP_ZCL_ENDPOINT_NULL)))) {
    return 0;
  }

  char *start = finger;

  finger += sprintf(finger, CH_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT);
  finger += sprintf(finger, CH_ZCL_URI_QUERY_POSTFIX_DEVICE_ID);
  finger += sprintf(finger, "%x", deviceId);

  if (endpointId == CHIP_ZCL_ENDPOINT_NULL) {
    finger += sprintf(finger, "%c", CH_ZCL_URI_QUERY_WILDCARD);
  } else {
    finger += sprintf(finger, ".%x", endpointId);
  }

  finger += appendUriDelimiter(finger);

  return finger - start;
}

bool chZclUriBreak(char *finger)
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

ChipCoapContentFormatType discRequestAccept = CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR;
ChipZclDiscoveryRequestMode discRequestMode = CHIP_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY;
#define CH_ZCL_DISC_CLI_MAX_BUFFER_LEN (512)
char cliDiscoveryRequestUri[CH_ZCL_DISC_CLI_MAX_BUFFER_LEN] = { 0 };
char * cliDiscoveryRequestUriFinger = cliDiscoveryRequestUri;

bool chZclDiscSetAccept(ChipCoapContentFormatType accept)
{
  if (accept != CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR
      && accept != CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT) {
    return false;
  }
  discRequestAccept = accept;
  return true;
}

bool chipZclDiscSetMode(ChipZclDiscoveryRequestMode mode)
{
  if (mode >= CHIP_ZCL_DISCOVERY_REQUEST_MODE_MAX) {
    return false;
  }

  if (discRequestMode != mode) {
    chipZclDiscInit();
  }

  discRequestMode = (ChipZclDiscoveryRequestMode) mode;
  return true;
}

void chipZclDiscInit(void)
{
  const uint16_t wellKnownCoreLen = strlen(CH_ZCL_URI_WELL_KNOWN_CORE);

  MEMSET(cliDiscoveryRequestUri,
         0x00,
         sizeof(char) * CH_ZCL_DISC_CLI_MAX_BUFFER_LEN);
  cliDiscoveryRequestUriFinger = cliDiscoveryRequestUri;

  MEMCOPY(cliDiscoveryRequestUriFinger,
          CH_ZCL_URI_WELL_KNOWN_CORE,
          wellKnownCoreLen);
  cliDiscoveryRequestUriFinger += wellKnownCoreLen;

  MEMCOPY(cliDiscoveryRequestUriFinger++, "?", 1);
}

bool chipZclDiscSend(ChipCoapResponseHandler responseHandler)
{
  extern const Bytes16 emFf05AllCoapNodesMulticastAddress;

  chipAfAppPrintln("Sent discovery command: %s", cliDiscoveryRequestUri);

  ChipCoapSendInfo info = { 0 }; // use defaults

  ChipCoapOption options[] = {
    { CHIP_COAP_OPTION_ACCEPT, NULL, 1, CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR, },
  };
  // Override default link-format+cbor w/ link-format.
  if (discRequestAccept == CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT) {
    options[0].intValue = discRequestAccept;
  }

  info.options = options;
  info.numberOfOptions = 1;

  ChipStatus status = chipCoapGet((const ChipIpv6Address *)&emFf05AllCoapNodesMulticastAddress,
                                    (const uint8_t *) cliDiscoveryRequestUri,
                                    responseHandler,
                                    &info);
  if (status != CHIP_SUCCESS) {
    chipAfAppPrintln("Failed to send discovery message with error (0x%x)", status);
  } else {
    chipAfAppPrintln("%p 0x%x", "get", status);
  }

  return status == CHIP_SUCCESS;
}

bool chipZclDiscAppendQuery(ChZclDiscoveryRequest request,
                             ChipCoapResponseHandler responseHandler)
{
  ChZclCliDiscoveryRequestType type = request.type;
  char **finger, *end;

  if (discRequestMode == CHIP_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY) {
    chipZclDiscInit();
  }

  finger = &cliDiscoveryRequestUriFinger;
  end = &cliDiscoveryRequestUri[CH_ZCL_DISC_CLI_MAX_BUFFER_LEN];

  switch (type) {
    case CH_ZCL_CLI_DISCOVERY_REQUEST_FOR_CORE_RD:
      *finger += appendResourceTypeCoreRd(*finger, end);
      break;
    case CH_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_ID:
      *finger += appendResourceTypeClusterId(*finger, end, request.data.clusterSpec);
      break;
    case CH_ZCL_CLI_DISCOVERY_REQUEST_BY_DEVICE_ID:
      *finger += appendDeviceIdAndEndpoint(*finger,
                                           end,
                                           request.deviceId,
                                           CHIP_ZCL_ENDPOINT_NULL);
      break;
    case CH_ZCL_CLI_DISCOVERY_REQUEST_BY_ENDPOINT_AND_DEVICE_ID:
      *finger += appendEndpointId(*finger,
                                  end,
                                  request.data.endpointId,
                                  request.deviceId);
      break;
    case CH_ZCL_CLI_DISCOVERY_REQUEST_BY_UID_STRING:
      *finger += appendUidString(*finger, end, request.data.uidString);
      break;
    case CH_ZCL_CLI_DISCOVERY_REQUEST_BY_RESOURCE_VERSION:
      *finger += appendRevision(*finger, end, request.data.version, true);
      break;
    case CH_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_REVISION:
      *finger += appendRevision(*finger, end, request.data.version, false);
      break;
    default:
      return false;
  }

  if (discRequestMode == CHIP_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY) {
    return chipZclDiscSend(responseHandler);
  }

  return true;
}

bool chipZclDiscCoreRd(ChipCoapResponseHandler responseHandler)
{
  ChZclDiscoveryRequest request = { .type = CH_ZCL_CLI_DISCOVERY_REQUEST_FOR_CORE_RD };
  return chipZclDiscAppendQuery(request, responseHandler);
}

bool chipZclDiscByClusterId(const ChipZclClusterSpec_t *clusterSpec,
                             ChipCoapResponseHandler responseHandler)
{
  ChZclDiscoveryRequest request = { .type = CH_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_ID,
                                    .data.clusterSpec = clusterSpec };
  return chipZclDiscAppendQuery(request, responseHandler);
}

bool chipZclDiscByEndpoint(ChipZclEndpointId_t endpointId,
                            ChipZclDeviceId_t deviceId,
                            ChipCoapResponseHandler responseHandler)
{
  ChZclDiscoveryRequest request = { .type = CH_ZCL_CLI_DISCOVERY_REQUEST_BY_ENDPOINT_AND_DEVICE_ID,
                                    .data.endpointId = endpointId,
                                    .deviceId = deviceId };
  return chipZclDiscAppendQuery(request, responseHandler);
}

bool chipZclDiscByUid(const ChipZclUid_t *uid,
                       uint16_t uidBits,
                       ChipCoapResponseHandler responseHandler)
{
  if (uidBits > CHIP_ZCL_UID_SIZE * 8) {
    return false;
  }

  uint8_t string[CHIP_ZCL_UID_BASE64URL_SIZE];
  uint8_t *finger = string;
  finger += chZclUidToBase64Url(uid, uidBits, finger);

  // A partially-specified UID becomes a prefix match.  Everything is NUL
  // terminated.
  if (finger != NULL) { // add a bounds check for MISRA compliance
    if (uidBits != CHIP_ZCL_UID_BITS) {
      *finger++ = CH_ZCL_URI_QUERY_WILDCARD;
    }
    *finger = '\0';
  }

  return chZclDiscByUidString(string, responseHandler);
}

bool chZclDiscByUidString(const uint8_t *uidString,
                          ChipCoapResponseHandler responseHandler)
{
  ChZclDiscoveryRequest request = {
    .type = CH_ZCL_CLI_DISCOVERY_REQUEST_BY_UID_STRING,
    .data.uidString = uidString,
  };
  return chipZclDiscAppendQuery(request, responseHandler);
}

bool chipZclDiscByResourceVersion(ChipZclClusterRevision_t version,
                                   ChipCoapResponseHandler responseHandler)
{
  ChZclDiscoveryRequest request = { .type = CH_ZCL_CLI_DISCOVERY_REQUEST_BY_RESOURCE_VERSION,
                                    .data.version = version };
  return chipZclDiscAppendQuery(request, responseHandler);
}

bool chipZclDiscByClusterRev(ChipZclClusterRevision_t version,
                              ChipCoapResponseHandler responseHandler)
{
  ChZclDiscoveryRequest request = { .type = CH_ZCL_CLI_DISCOVERY_REQUEST_BY_CLUSTER_REVISION,
                                    .data.version = version };
  return chipZclDiscAppendQuery(request, responseHandler);
}

bool chipZclDiscByDeviceId(ChipZclDeviceId_t deviceId,
                            ChipCoapResponseHandler responseHandler)
{
  ChZclDiscoveryRequest request = { .type = CH_ZCL_CLI_DISCOVERY_REQUEST_BY_DEVICE_ID,
                                    .deviceId = deviceId };
  return chipZclDiscAppendQuery(request, responseHandler);
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
                                               ChipZclEndpointId_t endpointId,
                                               const ChipZclClusterSpec_t *clusterSpec)
{
  if (buffer != NULL) {
    char *finger = buffer;
    finger += sprintf(finger, "/zcl");

    if (endpointId != CHIP_ZCL_ENDPOINT_NULL) {
      finger += sprintf(finger, "/e/%x", endpointId);
      if (clusterSpec != NULL) {
        finger += sprintf(finger,
                          "/%c%x",
                          (clusterSpec->role == CHIP_ZCL_ROLE_CLIENT) ? 'c' : 's',
                          clusterSpec->id);
      }
    }
    return finger - buffer;
  }
  return -1;
}

static int16_t constructRtValueString(char *buffer,
                                      const ChipZclClusterSpec_t *spec,
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
                        (spec->role == CHIP_ZCL_ROLE_CLIENT) ? 'c' : 's');
    }
    return finger - buffer;
  }
  return -1;
}

static int16_t constructIfValueString(char *buffer,
                                      ChipZclEndpointId_t endpointId,
                                      const ChipZclClusterSpec_t *clusterSpec)
{
  if (buffer != NULL) {
    char *finger = (char *)buffer;
    if (endpointId != CHIP_ZCL_ENDPOINT_NULL && clusterSpec != NULL) {
      ChipZclClusterRevision_t clusterRevision;
      ChipZclStatus_t status;

      status = chipZclReadAttribute(endpointId,
                                     clusterSpec,
                                     CHIP_ZCL_ATTRIBUTE_CLUSTER_REVISION,
                                     &clusterRevision,
                                     sizeof(clusterRevision));

      if (status != CHIP_ZCL_STATUS_SUCCESS) {
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
    finger += chZclUidToBase64Url(&chZclUid, CHIP_ZCL_UID_BITS, (uint8_t *)finger);
    return finger - buffer;
  }
  return -1;
}

static int16_t constructZeValueString(char *buffer,
                                      ChipZclEndpointId_t endpointId,
                                      ChipZclDeviceId_t deviceId)
{
  if (buffer != NULL) {
    char *finger = (char *)buffer;
    if (endpointId != CHIP_ZCL_ENDPOINT_NULL) {
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

static bool appendPayloadLinkFormat(ChZclDiscPayloadContext_t *dpc, uint8_t *data, uint16_t dataLength)
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

static void separateLinkLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  if (dpc->linkCount != 0) {
    appendPayloadLinkFormat(dpc, (uint8_t *)",", 1);
  }
}

static void separateAttrLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  appendPayloadLinkFormat(dpc, (uint8_t *)";", 1);
}

static bool payloadIsEmptyLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  // Zero length.
  return (dpc->payloadLength(dpc) == 0);
}

static void startPayloadLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  // Nothing to do.
}

static void startLinkLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  separateLinkLinkFormat(dpc);
  dpc->linkCount++;
}

static void addResourceUriLinkFormat(ChZclDiscPayloadContext_t *dpc,
                                     ChipZclEndpointId_t endpointId,
                                     const ChipZclClusterSpec_t *clusterSpec)
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

static void addRtLinkFormat(ChZclDiscPayloadContext_t *dpc,
                            const ChipZclClusterSpec_t *spec,
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

static void addIfLinkFormat(ChZclDiscPayloadContext_t *dpc,
                            ChipZclEndpointId_t endpointId,
                            const ChipZclClusterSpec_t *clusterSpec)
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

static void addEpLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  separateAttrLinkFormat(dpc);

  // Construct value string.
  char buffer[CH_ZCL_URI_DEVICE_UID_APPEND_MAX_LEN] = { 0 };
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

static void addZeLinkFormat(ChZclDiscPayloadContext_t *dpc,
                            ChipZclEndpointId_t endpointId,
                            ChipZclDeviceId_t deviceId)
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

static void endLinkLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  dpc->linkCount++;
}

static void endPayloadLinkFormat(ChZclDiscPayloadContext_t *dpc)
{
  // Nothing to do.
}

// ----------------------------------------------------------------------------
// Discovery Payload Context: link-format+cbor implemementation
//
// Payload structure is a CBOR array containing zero or more CBOR maps,
// each map representing a link.

static bool payloadIsEmptyLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc)
{
  // Either zero length, or first byte signifies empty CBOR array.
  return (dpc->payloadLength(dpc) == 0 || *(dpc->payloadPointer(dpc)) == 0x80);
}

static void startPayloadLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc)
{
  // Open CBOR array.
  if (!emCborEncodeIndefiniteArray(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

static void startLinkLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc)
{
  // Open CBOR map.
  if (!emCborEncodeIndefiniteMap(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

static void addResourceUriLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc,
                                             ChipZclEndpointId_t endpointId,
                                             const ChipZclClusterSpec_t *clusterSpec)
{
  // Construct value string.
  char buffer[256] = { 0 };

  if (constructResourceUriValueString(buffer, endpointId, clusterSpec) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Resource URI (href) is numeric map key 1.
  if (!emCborEncodeMapEntry(&dpc->cborState, 1, CHIP_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

static void addRtLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc,
                                    const ChipZclClusterSpec_t *spec,
                                    bool includeTrailingClusterTag)
{
  // Construct value string.
  char buffer[50] = { 0 };

  if (constructRtValueString(buffer, spec, includeTrailingClusterTag) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Resource Type (rt) is numeric map key 9.
  if (!emCborEncodeMapEntry(&dpc->cborState, 9, CHIP_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void addIfLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc,
                                    ChipZclEndpointId_t endpointId,
                                    const ChipZclClusterSpec_t *clusterSpec)
{
  // Construct value string.
  char buffer[50] = { 0 };

  if (constructIfValueString(buffer, endpointId, clusterSpec) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Interface (if) is numeric key map 10.
  if (!emCborEncodeMapEntry(&dpc->cborState, 10, CHIP_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void addEpLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc)
{
  // Construct value string.
  char buffer[CH_ZCL_URI_DEVICE_UID_APPEND_MAX_LEN] = { 0 };

  if (constructEpValueString(buffer) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Endpoint (ep) is text key map "ep".
  if (!emCborEncodeValue(&dpc->cborState, CHIP_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)"ep")
      || !emCborEncodeValue(&dpc->cborState, CHIP_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void addZeLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc,
                                    ChipZclEndpointId_t endpointId,
                                    ChipZclDeviceId_t deviceId)
{
  // Construct value string.
  char buffer[50] = { 0 };

  if (constructZeValueString(buffer, endpointId, deviceId) == -1) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_FAIL;
    return;
  }

  // Zigbee Endpoint (ze) is text key map "ze".
  if (!emCborEncodeValue(&dpc->cborState, CHIP_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)"ze")
      || !emCborEncodeValue(&dpc->cborState, CHIP_ZCLIP_TYPE_STRING, 0xFF, (uint8_t *)buffer)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->attrCount++;
}

static void endLinkLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc)
{
  // Close CBOR map.
  if (!emCborEncodeBreak(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
  dpc->linkCount++;
}

static void endPayloadLinkFormatPlusCbor(ChZclDiscPayloadContext_t *dpc)
{
  // Close CBOR array.
  if (!emCborEncodeBreak(&dpc->cborState)) {
    dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_BUFFER_OVERFLOW;
  }
}

// ----------------------------------------------------------------------------
// Discovery Payload Context: common

static const uint8_t *payloadPointerCommon(ChZclDiscPayloadContext_t *dpc)
{
  return dpc->cborState.start;
}

static uint16_t payloadLengthCommon(ChZclDiscPayloadContext_t *dpc)
{
  return emCborEncodeSize(&dpc->cborState);
}

static void appendUidCommon(ChZclDiscPayloadContext_t *dpc)
{
  dpc->startLink(dpc);
  dpc->addResourceUri(dpc, CHIP_ZCL_ENDPOINT_NULL, NULL);
  dpc->addRt(dpc, NULL, false);
  dpc->addIf(dpc, CHIP_ZCL_ENDPOINT_NULL, NULL);
  dpc->addEp(dpc);
  dpc->endLink(dpc);
}

bool chZclInitDiscPayloadContext(ChZclDiscPayloadContext_t *dpc,
                                 ChipCoapContentFormatType contentFormat,
                                 uint8_t *buffer,
                                 uint16_t bufferLength)
{
  if (dpc == NULL || buffer == NULL || bufferLength == 0) {
    return false;
  }

  MEMSET(dpc, 0, sizeof(ChZclDiscPayloadContext_t));
  dpc->status = DISCOVERY_PAYLOAD_CONTEXT_STATUS_SUCCESS;
  dpc->contentFormat = contentFormat;

  // Leverage the start/finger/end pointers in CborState for payload buffer management
  // of other content-format types.
  emCborEncodeStart(&dpc->cborState, buffer, bufferLength);

  switch (contentFormat) {
    case CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT:
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

    case CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR:
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
