/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE

#include "stack/ip/tls/tls.h"
#include "stack/ip/tls/tls-sha256.h"
#include "app/coap/coap.h"

#define USE_STUB_emIsDefaultDomainPrefix
#define USE_STUB_emberCoapSend
#define USE_STUB_emberGetNetworkParameters
#define USE_STUB_emberGetNodeId
#define USE_STUB_emberNetworkStatus
#define USE_STUB_emberReadIntegerOption
#define USE_STUB_emEventControlSetDelayMS
#define USE_STUB_emIsDefaultGlobalPrefix
#define USE_STUB_emIsLocalIpAddress
#define USE_STUB_emMacExtendedId
#define USE_STUB_emStoreDefaultGlobalPrefix
#define USE_STUB_emberUdpListen
#define USE_STUB_emberRemoveUdpListeners
#define USE_STUB_emberGetSecureDtlsSessionId
#define USE_STUB_emberGetDtlsConnectionPeerAddressBySessionId
#define USE_STUB_emberGetDtlsConnectionPeerPortBySessionId
#define USE_STUB_emberGetDtlsConnectionPeerPublicKeyBySessionId
#define USE_STUB_emberGetDtlsConnectionNextSessionId
#define USE_STUB_emberOpenDtlsConnection
#define USE_STUB_emberCloseDtlsConnection
#define USE_STUB_emberCoapRequestHandler
#define USE_STUB_emProcessCoapMessage
#define USE_STUB_emberReadLocationPath

#include "stack/ip/stubs.c"

void emZclHandler(EmberCoapCode code,
                  uint8_t *uri,
                  EmberCoapReadOptions *options,
                  const uint8_t *payload,
                  uint16_t payloadLength,
                  const EmberCoapRequestInfo *info);
void resetSystemTokens(void);

static const char *testUri;

const EmberZclClusterSpec_t emberZclCluster3456ClientSpec = {
  EMBER_ZCL_ROLE_CLIENT,
  EMBER_ZCL_MANUFACTURER_CODE_NULL,
  0x3456,
};
const EmberZclClusterSpec_t emberZclCluster3456ServerSpec = {
  EMBER_ZCL_ROLE_SERVER,
  EMBER_ZCL_MANUFACTURER_CODE_NULL,
  0x3456,
};
const EmberZclClusterSpec_t emberZclCluster34569abcClientSpec = {
  EMBER_ZCL_ROLE_CLIENT,
  0x3456,
  0x9abc,
};
const EmberZclClusterSpec_t emberZclCluster34569abcServerSpec = {
  EMBER_ZCL_ROLE_SERVER,
  0x3456,
  0x9abc,
};

// These are generated in sorted order by AppBuilder.
const EmberZclClusterSpec_t *emZclEndpoint1bSpec[] = {
  &emberZclCluster3456ClientSpec,
  &emberZclCluster34569abcClientSpec,
  &emberZclCluster3456ServerSpec,
  &emberZclCluster34569abcServerSpec,
  NULL,
};

const EmZclEndpointEntry_t emZclEndpointTable[] = {
  { 0x1b, 0xDEAD, emZclEndpoint1bSpec }
};
const size_t emZclEndpointCount = COUNTOF(emZclEndpointTable);

// These are generated in sorted order by AppBuilder.
const EmZclCommandEntry_t emZclCommandTable[] = {
  { &emberZclCluster3456ClientSpec, 0xf4, NULL, NULL },
  { &emberZclCluster34569abcClientSpec, 0xf4, NULL, NULL },
  { &emberZclCluster3456ServerSpec, 0xf4, NULL, NULL },
  { &emberZclCluster34569abcServerSpec, 0xf4, NULL, NULL },
};
const size_t emZclCommandCount = COUNTOF(emZclCommandTable);

// enum {
//   clustersOnEndpointHandler,
//   groupIdsOnDeviceHandler,
//   endpointsInGroupHandler,
//   endpointsOnDeviceHandler
// };

void emZclAccessTokenHandler(EmZclContext_t *context)
{
}

bool emZclAddIncomingToken(const EmberZclUid_t *remoteUid,
                           const uint8_t *tokenBytes,
                           uint16_t tokenLength)
{
  return false;
}

EmberZclStatus_t emZclAllowRemoteAccess(const uint8_t sessionId,
                                        const EmberZclClusterSpec_t *clusterSpec,
                                        EmberZclDeviceId_t endpointDeviceId,
                                        uint8_t accessType)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

bool emZclIsProtectedResource(const EmberZclClusterSpec_t *clusterSpec,
                              uint8_t accessType)
{
  return false;
}

bool emZclExtractBinaryAccessToken(CborState *state,
                                   uint8_t *output,
                                   uint16_t *outputSize)
{
  return false;
}

bool emberDtlsTransmitHandler(const uint8_t *payload,
                              uint16_t payloadLength,
                              const EmberIpv6Address *localAddress,
                              uint16_t localPort,
                              const EmberIpv6Address *remoteAddress,
                              uint16_t remotePort,
                              void *transmitHandlerData)
{
  return 0;
}

void emberZclGetPublicKeyCallback(const uint8_t **publicKey,
                                  uint16_t *publicKeySize)
{
}

bool emZclAttributeUriMetadataQueryParse(EmZclContext_t *context,
                                         void *data,
                                         uint8_t depth)
{
  assert(0);
  return false;
}

bool emZclAttributeUriQueryFilterParse(EmZclContext_t *context,
                                       void *data,
                                       uint8_t depth)
{
  assert(0);
  return false;
}

bool emZclAttributeUriQueryUndividedParse(EmZclContext_t *context,
                                          void *data,
                                          uint8_t depth)
{
  assert(0);
  return false;
}

void emZclClusterCommandHandler(EmZclContext_t *context)
{
  emZclRespond205ContentCbor(context->info, NULL, 0);
}
void emZclUriClusterAttributeHandler(EmZclContext_t *context)
{
  emZclRespond205ContentCbor(context->info, NULL, 0);
}
void emZclUriClusterAttributeIdHandler(EmZclContext_t *context)
{
  emZclRespond205ContentCbor(context->info, NULL, 0);
}

bool emZclHasReportingConfiguration(EmberZclEndpointId_t endpointId,
                                    const EmberZclClusterSpec_t *clusterSpec,
                                    EmberZclReportingConfigurationId_t reportingConfigurationId)
{
  return true;
}
void emZclUriClusterNotificationHandler(EmZclContext_t *context)
{
}
void emZclUriClusterReportingConfigurationHandler(EmZclContext_t *context)
{
}
void emZclUriClusterReportingConfigurationIdHandler(EmZclContext_t *context)
{
}

EmberZclStatus_t emberZclReadAttribute(EmberZclEndpointId_t endpointId,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

const EmZclAttributeEntry_t *emZclFindAttribute(const EmberZclClusterSpec_t *clusterSpec,
                                                EmberZclAttributeId_t attributeId,
                                                bool includeRemote)
{
  return (EmZclAttributeEntry_t *) 1;  // any non-NULL value works
}

EmberStatus emZclSend(const EmberZclCoapEndpoint_t *destination,
                      EmberCoapCode code,
                      const uint8_t *uri,
                      const uint8_t *payload,
                      uint16_t payloadLength,
                      EmberCoapResponseHandler handler,
                      void *applicationData,
                      uint16_t applicationDataLength,
                      bool skipRetryEvent)
{
  return EMBER_SUCCESS;
}

EmberStatus emZclSendWithOptions(const EmberZclCoapEndpoint_t *destination,
                                 EmberCoapCode code,
                                 const uint8_t *uriPath,
                                 const EmberCoapOption options[],
                                 uint16_t optionsLength,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseHandler handler,
                                 void *applicationData,
                                 uint16_t applicationDataLength,
                                 bool skipRetryEvent)
{
  return EMBER_SUCCESS;
}

void emberParseBlockOptionValue(uint32_t value,
                                EmberCoapBlockOption *option)
{
  option->more = (value & 0x08) != 0;
  option->logSize = (value & 0x07) + BLOCK_OPTION_VALUE_LOG_SIZE_MIN;
  option->number = value >> 4;
}

bool emberReadBlockOption(EmberCoapReadOptions *options,
                          EmberCoapOptionType type,
                          EmberCoapBlockOption *option)
{
  uint32_t value;
  if (emberReadIntegerOption(options, type, &value)) {
    emberParseBlockOptionValue(value, option);
    return true;
  } else {
    return false;
  }
}

uint32_t emberBlockOptionValue(bool more,
                               uint8_t logSize,
                               uint32_t number)
{
  assert(BLOCK_OPTION_VALUE_LOG_SIZE_MIN <= logSize && logSize <= BLOCK_OPTION_VALUE_LOG_SIZE_MAX);
  return ((logSize - BLOCK_OPTION_VALUE_LOG_SIZE_MIN)
          | (more ? 0x08 : 0)
          | (number << 4));
}

void emberInitCoapOption(EmberCoapOption *option,
                         EmberCoapOptionType type,
                         uint32_t value)
{
  option->type = type;
  option->value = NULL;
  option->valueLength = 0;
  option->intValue = value;
}

void emZclCoapStatusHandler(EmberCoapStatus status, EmberCoapResponseInfo *info)
{
}

static EmberCoapCode expectedResponseCode;

EmberStatus emberCoapRespondWithPath(const EmberCoapRequestInfo *requestInfo,
                                     EmberCoapCode code,
                                     const uint8_t *path,
                                     const EmberCoapOption *options,
                                     uint8_t numberOfOptions,
                                     const uint8_t *payload,
                                     uint16_t payloadLength)
{
  if (code != expectedResponseCode) {
    fprintf(stderr, " URI %s got %d.%d want %d.%d\n",
            testUri,
            GET_COAP_CLASS(code),
            GET_COAP_DETAIL(code),
            GET_COAP_CLASS(expectedResponseCode),
            GET_COAP_DETAIL(expectedResponseCode));
  }
  assert(code == expectedResponseCode);
  expectedResponseCode = -1;
  // int i;
  // fprintf(stderr, "[payload");
  // for (i = 0; i < payloadLength; i++) {
  //   fprintf(stderr, " %02X", payload[i]);
  // }
  // fprintf(stderr, "]\n");
  return EMBER_SUCCESS;
}

// emberResetReadOptionPointer and emberReadNextOption only work for URI paths,
// which must be stored as a NUL-terminated string in testUri.
const char *testUriFinger = NULL;
void emberResetReadOptionPointer(EmberCoapReadOptions *options)
{
  testUriFinger = testUri;
}
EmberCoapOptionType emberReadNextOption(EmberCoapReadOptions *options,
                                        const uint8_t **valuePointerLoc,
                                        uint16_t *valueLengthLoc)
{
  if (testUriFinger == NULL) {
    return EMBER_COAP_NO_OPTION;
  }

  *valuePointerLoc = (const uint8_t *)testUriFinger;
  const char *finger = strchr(testUriFinger, '/');
  if (finger == NULL) {
    *valueLengthLoc = strlen(testUriFinger);
    testUriFinger = NULL;
  } else {
    *valueLengthLoc = (finger - testUriFinger);
    testUriFinger += *valueLengthLoc + 1;
    if (*testUriFinger == '\0') {
      testUriFinger = NULL;
    }
  }
  return EMBER_COAP_OPTION_URI_PATH;
}

static const char *notFound[] = {
  "foo/",
  "zcl/x/",
  "zcl/ex/",
  "zcl/e/1/",
  "zcl/e/1/",
  "zcl/e/1g/",
  "zcl/e/123/",
  NULL
};

static const char *found[] = {
  ".well-known/core/",
  "zcl/",
  "zcl/e/",
  "zcl/e/1b/",
  "zcl/g/",
  "zcl/g/1b2c/",
  NULL
};

// n and r are not yet implemented
static const char *clusterSuffixes[] = {
  "a",
  "a/f4e3",
  "c",
  "c/f4",
  NULL
};

static void requestTest(const char *uri,
                        EmberCoapCode requestCode,
                        const uint8_t *payload,
                        uint16_t payloadLength,
                        EmberCoapCode responseCode)
{
  // TODO: Set up info.
  EmberCoapRequestInfo info;
  memset(&info, 0, sizeof(info));
  testUri = uri;
  expectedResponseCode = responseCode;
  emZclHandler(requestCode,
               (uint8_t *)uri,
               NULL, // TODO: options
               payload,
               payloadLength,
               &info);
  if (expectedResponseCode != (EmberCoapCode) - 1) {
    fprintf(stderr, " URI %s got no reponse, want %d.%d\n",
            testUri,
            GET_COAP_CLASS(expectedResponseCode),
            GET_COAP_DETAIL(expectedResponseCode));
  }
  assert(expectedResponseCode == (EmberCoapCode) - 1);
}

#define uriTest(u, req, res) (requestTest((u), (req), NULL, 0, (res)))
#define uriGetTest(u, rc) (uriTest((u), EMBER_COAP_CODE_GET, (rc)))

static void hexToIntTest(void)
{
  int i;
  uint8_t buffer[3];
  for (i = 0; i < 256; i++) {
    unsigned int r0;
    buffer[0] = i;
    buffer[1] = 0;
    uintmax_t r1;

    // We should accept it iff sscanf() does and get the same answer.
    if (sscanf((const char *)buffer, "%1x", &r0) == 1) {
      assert(emZclHexStringToInt(buffer, 1, &r1));
      assert(r0 == r1);
    } else {
      assert(!emZclHexStringToInt(buffer, 1, &r1));
    }

    // Read back what snprintf() writes.
    snprintf((char *)buffer, sizeof(buffer), "%x", i);
    assert(emZclHexStringToInt(buffer, strlen((char *)buffer), &r1));
    assert(r1 == i);
  }
}

static void intToHexTest(void)
{
  int i;
  uint8_t buffer1[5] = { 0 }, buffer2[5] = { 0 };
  for (i = 0; i < 65536; i++) {
    if (i < 256) {
      sprintf((char *)buffer1, "%x", i);
      emZclIntToHexString(i, 1, buffer2);
      assert(strcmp((const char *)buffer1, (const char *)buffer2) == 0);
    }
    sprintf((char *)buffer1, "%x", i);
    emZclIntToHexString(i, 2, buffer2);
    assert(strcmp((const char *)buffer1, (const char *)buffer2) == 0);
  }
  uint32_t x = -1;
  uint8_t buffer3[2 * sizeof(x) + 1];
  emZclIntToHexString(x, sizeof(x), buffer3);
}

// CBOR-encoded binding for POSTing.
// {"u":"coap://[::]:4660/zcl/e/2","r":0}
static const int8u aBinding[] = {
  0xa2, 0x61, 0x75, 0x78, 0x18, 0x63, 0x6f, 0x61,
  0x70, 0x3a, 0x2f, 0x2f, 0x5b, 0x3a, 0x3a, 0x5d,
  0x3a, 0x34, 0x36, 0x36, 0x30, 0x2f, 0x7a, 0x63,
  0x6c, 0x2f, 0x65, 0x2f, 0x32, 0x61, 0x72, 0x00
};

static bool cacheAddEntry(const EmberZclUid_t *uid, const EmberIpv6Address *address)
{
  return emZclCacheAdd(uid, address, NULL);
}

static EmberZclUid_t cacheExpectedUidInstance;

static EmberZclUid_t *cacheExpectedUid(uint8_t uidNumber)
{
  MEMSET(&cacheExpectedUidInstance.bytes,
         uidNumber,
         sizeof(cacheExpectedUidInstance.bytes));
  return &cacheExpectedUidInstance;
}

const EmberIpv6Address ipv6Prefix[] = {
  // Last byte of first row of each distinguishes 62-bit prefixes.
  { {
      0x20, 0x01, 0x0d, 0xb8, 0x85, 0x69, 0xb2, 0xb1, // 0xb1
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    } },
  { {
      0x20, 0x01, 0x0d, 0xb8, 0x85, 0x69, 0xb2, 0xb5, // 0xb5
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    } },
};

static EmberIpv6Address cacheExpectedAddrInstance;

static EmberIpv6Address *cacheExpectedAddrPrefix(uint8_t addrNumber, uint8_t prefixIndex)
{
  assert(prefixIndex <= sizeof(ipv6Prefix) / sizeof(ipv6Prefix[0]));
  MEMCOPY(&cacheExpectedAddrInstance,
          &ipv6Prefix[prefixIndex],
          sizeof(EmberIpv6Address));
  MEMSET(&cacheExpectedAddrInstance.bytes[12], addrNumber, 4);
  return &cacheExpectedAddrInstance;
}

static EmberIpv6Address *cacheExpectedAddr(uint8_t addrNumber)
{
  return cacheExpectedAddrPrefix(addrNumber, 0);
}

static void cachePopulate(uint8_t count)
{
  EmberZclUid_t returnedUid;
  EmberIpv6Address returnedAddress;
  uint8_t i;
  for (i = 1; i <= count; i++) {
    assert(cacheAddEntry(cacheExpectedUid(i), cacheExpectedAddr(i)));
    assert(emZclCacheGet(cacheExpectedUid(i), &returnedAddress));
    assert(MEMCOMPARE(cacheExpectedAddr(i), &returnedAddress, sizeof(EmberIpv6Address)) == 0);
    assert(emZclCacheGetFirstKeyForValue(cacheExpectedAddr(i), &returnedUid));
    assert(MEMCOMPARE(cacheExpectedUid(i), &returnedUid, sizeof(EmberZclUid_t)) == 0);
  }
}

//static void printIUA(const EmZclCacheIndex_t *index,
//                     const EmberZclUid_t *uid,
//                     const EmberIpv6Address *addr) {
//  if (index != NULL) {
//    fprintf(stderr, "\nIndx: %04x", *index);
//  }
//  if (uid != NULL) {
//    fprintf(stderr, "\nUID:  ");
//    for (int i = 0; i < sizeof(EmberZclUid_t); ++i) {
//      fprintf(stderr, "%02x ", uid->bytes[i]);
//    }
//  }
//  if (addr != NULL) {
//    fprintf(stderr, "\nIPv6: ");
//    for (int i = 0; i < sizeof(EmberIpv6Address); ++i) {
//      fprintf(stderr, "%02x ", addr->bytes[i]);
//    }
//  }
//  fprintf(stderr, "\n");
//}

// ScanPredicate
//static bool cachePrintEntry(const void *criteria, const EmZclCacheEntry_t *entry)
//{
//  printIUA(&entry->index, &entry->key.uid, &entry->value.address);
//  return false; // continue through all entries
//}

// Print cache entries in queue order
//static void cachePrint(void)
//{
//  emZclCacheScan(NULL, cachePrintEntry);
//}

int main(int argc, char *argv[])
{
  fprintf(stderr, "[%s ", argv[0]);

  resetSystemTokens();

  fputc('.', stderr);

  EmberZclBindingEntry_t entry;
  entry.destination.network.port = 0x1234;
  MEMCOPY(&entry.destination.network.data.address, "abcdefghijlmnopq", 16);
  entry.endpointId = 2;
  emberZclSetBinding(1, &entry);

  hexToIntTest();

  fputc('.', stderr);

  intToHexTest();

  fputc('.', stderr);

  const char **uris;

  for (uris = notFound; *uris != NULL; uris++) {
    uriGetTest(*uris, EMBER_COAP_CODE_404_NOT_FOUND);
  }

  fputc('.', stderr);

  uriTest("zcl/", EMBER_COAP_CODE_POST, EMBER_COAP_CODE_405_METHOD_NOT_ALLOWED);

  fputc('.', stderr);

  assert(emberZclAddEndpointToGroup(0x1b, 0x1b2c, NULL, 0, 0xFF, NULL, 0) == EMBER_ZCL_STATUS_SUCCESS);

  fputc('.', stderr);

  for (uris = found; *uris != NULL; uris++) {
    uriGetTest(*uris, EMBER_COAP_CODE_205_CONTENT);
  }

  fputc('.', stderr);

  // Testing the various cluster ID combinations.
  int i;
  char uri[100];
  for (i = 0; i < 8; i++) {
    int count = sprintf(uri, "zcl/%c/1b%s/%c3456%s/",
                        "eg"[i & 0x01],
                        (i & 0x01) ? "2c" : "",
                        "c s"[i & 0x02],
                        (i & 0x04) ? "" : "_9abc");
    uriGetTest(uri, ((i & 0x01)
                     ? EMBER_COAP_CODE_404_NOT_FOUND  // "g"
                     : EMBER_COAP_CODE_205_CONTENT)); // "e"
    const char **suffixes;
    for (suffixes = clusterSuffixes; *suffixes != 0; suffixes++) {
      sprintf(uri + count, "%s/", *suffixes);
      uriGetTest(uri,
                 (((*suffixes)[0] == 'c'
                   && 1 < strlen(*suffixes))
                  ? EMBER_COAP_CODE_405_METHOD_NOT_ALLOWED
                  : EMBER_COAP_CODE_205_CONTENT));
    }
  }

  fputc('.', stderr);

  uriGetTest("zcl/e/1b/c3456_9abc/b/0/", EMBER_COAP_CODE_404_NOT_FOUND);
  requestTest("zcl/e/1b/c3456_9abc/b/",
              EMBER_COAP_CODE_POST,
              aBinding,
              sizeof(aBinding),
              EMBER_COAP_CODE_201_CREATED);
  uriGetTest("zcl/e/1b/c3456_9abc/b/0/", EMBER_COAP_CODE_205_CONTENT);

  fputc('.', stderr);

  // From "16-07043-003-UIDTextForZCLIPSpecExplanation minor update 2016-11-09"
  // sent by Sander Raaijmakers to mesh_ip_tsc@mail.zigbee.org on November 9,
  // 2016.
  //
  // Updated by "16-07008-030-zcl-over-ip-base-device-specification",
  // Section 2.7, May 15, 2017 re base64url encoding of binary UID.
  const uint8_t input[] = {
    'z', 'c', 'l', '.', 'u', 'i', 'd', 0x04,
    0x6f, 0xef, 0xe2, 0xb4, 0xb1, 0x59, 0x5a, 0xdb,
    0x73, 0xa9, 0x57, 0x83, 0xf7, 0x70, 0x80, 0x8e,
    0x2f, 0xa9, 0x72, 0x6c, 0x18, 0x2e, 0x24, 0x96,
    0x93, 0x8e, 0x77, 0x96, 0x2d, 0x1e, 0x16, 0x05,
    0xfa, 0x41, 0x11, 0x56, 0x95, 0x63, 0x09, 0xa2,
    0x3a, 0x92, 0xf6, 0x64, 0xf7, 0x7c, 0x16, 0x94,
    0x47, 0xba, 0xf0, 0x5b, 0x82, 0xc9, 0x1e, 0x75,
    0x5c, 0xf4, 0x55, 0x84, 0xa4, 0x66, 0xeb, 0x95,
  };
  const EmberZclUid_t expectedUid = {
    .bytes = {
      0x36, 0xf6, 0xe4, 0x52, 0xb1, 0xca, 0xc0, 0x2a,
      0x4f, 0x6a, 0x6b, 0x4e, 0xf3, 0x41, 0xaf, 0x8e,
      0x92, 0x28, 0x1f, 0x25, 0x01, 0x5b, 0x08, 0x1f,
      0x86, 0x83, 0xa5, 0x12, 0x94, 0xb5, 0x6d, 0x46,
    },
  };
  const uint8_t expectedBase64Url[] = "NvbkUrHKwCpPamtO80GvjpIoHyUBWwgfhoOlEpS1bUY";

  EmberZclUid_t actualUid;
  emSha256Hash(input, sizeof(input), actualUid.bytes);
  assert(MEMCOMPARE(&expectedUid, &actualUid, sizeof(EmberZclUid_t)) == 0);

  uint8_t actualBase64Url[EMBER_ZCL_UID_BASE64URL_SIZE];
  MEMSET(actualBase64Url, 0, EMBER_ZCL_UID_BASE64URL_SIZE);
  assert(emZclUidToBase64Url(&expectedUid, EMBER_ZCL_UID_BITS, actualBase64Url) == EMBER_ZCL_UID_BASE64URL_LENGTH);
  assert(MEMCOMPARE(expectedBase64Url, actualBase64Url, EMBER_ZCL_UID_BASE64URL_LENGTH) == 0);
  MEMSET(&actualUid, 0, sizeof(EmberZclUid_t));
  uint16_t resultBits = 0;
  assert(emZclBase64UrlToUid(actualBase64Url, EMBER_ZCL_UID_BASE64URL_LENGTH, &actualUid, &resultBits));
  assert(MEMCOMPARE(&expectedUid, &actualUid, sizeof(EmberZclUid_t)) == 0);
  assert(resultBits == EMBER_ZCL_UID_BITS);

  fputc('.', stderr);

// ----------------------------------------------------------------------------
// UID cache tests
  EmberIpv6Address actualAddress;

  // Clear cache.
  emZclCacheRemoveAll();
  assert(emZclCacheGetEntryCount() == 0);
  assert(!emZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), NULL));
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddrPrefix(1, 1), NULL));
  assert(!emZclCacheRemove(&expectedUid));

  fputc('.', stderr);

  // Add an entry.
  assert(cacheAddEntry(cacheExpectedUid(1), cacheExpectedAddr(1)));
  assert(emZclCacheGetEntryCount() == 1);
  assert(emZclCacheGet(cacheExpectedUid(1), &actualAddress));
  assert(MEMCOMPARE(cacheExpectedAddr(1), &actualAddress, sizeof(EmberIpv6Address)) == 0);
  assert(emZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), &actualUid));
  assert(MEMCOMPARE(cacheExpectedUid(1), &actualUid, sizeof(EmberZclUid_t)) == 0);

  fputc('.', stderr);

  // Remove that entry.
  assert(emZclCacheRemove(cacheExpectedUid(1)));
  assert(emZclCacheGetEntryCount() == 0);
  assert(!emZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), NULL));

  fputc('.', stderr);

  // Test remove head (which is last one in: 3).
  emZclCacheRemoveAll();
  cachePopulate(3);
  assert(emZclCacheGetEntryCount() == 3);
  assert(emZclCacheRemove(cacheExpectedUid(3)));
  assert(emZclCacheGetEntryCount() == 2);
  assert(!emZclCacheGet(cacheExpectedUid(3), NULL));
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddr(3), NULL));
  assert(emZclCacheGet(cacheExpectedUid(1), NULL));
  assert(emZclCacheGet(cacheExpectedUid(2), NULL));

  fputc('.', stderr);

  // Test remove tail (which is first one in: 1).
  emZclCacheRemoveAll();
  cachePopulate(3);
  assert(emZclCacheGetEntryCount() == 3);
  assert(emZclCacheRemove(cacheExpectedUid(1)));
  assert(emZclCacheGetEntryCount() == 2);
  assert(!emZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), NULL));
  assert(emZclCacheGet(cacheExpectedUid(2), NULL));
  assert(emZclCacheGet(cacheExpectedUid(3), NULL));

  fputc('.', stderr);

  // Test remove interior entry (2).
  emZclCacheRemoveAll();
  cachePopulate(3);
  assert(emZclCacheGetEntryCount() == 3);
  assert(emZclCacheRemove(cacheExpectedUid(2)));
  assert(emZclCacheGetEntryCount() == 2);
  assert(!emZclCacheGet(cacheExpectedUid(2), NULL));
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddr(2), NULL));
  assert(emZclCacheGet(cacheExpectedUid(1), NULL));
  assert(emZclCacheGet(cacheExpectedUid(3), NULL));

  // Test remove all by address (entries positioned at head, tail, interior).
  emZclCacheRemoveAll();
  assert(cacheAddEntry(cacheExpectedUid(7), cacheExpectedAddr(7))); // tail
  cachePopulate(3); // 1-3
  assert(cacheAddEntry(cacheExpectedUid(4), cacheExpectedAddr(7)));
  assert(cacheAddEntry(cacheExpectedUid(5), cacheExpectedAddr(5)));
  assert(cacheAddEntry(cacheExpectedUid(6), cacheExpectedAddr(7))); // head
  assert(emZclCacheGetEntryCount() == 7);
  assert(emZclCacheRemoveAllByValue(cacheExpectedAddr(7)));
  assert(emZclCacheGetEntryCount() == 4);
  assert(emZclCacheGet(cacheExpectedUid(1), NULL));
  assert(emZclCacheGet(cacheExpectedUid(2), NULL));
  assert(emZclCacheGet(cacheExpectedUid(3), NULL));
  assert(!emZclCacheGet(cacheExpectedUid(4), NULL));
  assert(emZclCacheGet(cacheExpectedUid(5), NULL));
  assert(!emZclCacheGet(cacheExpectedUid(6), NULL));
  assert(!emZclCacheGet(cacheExpectedUid(7), NULL));
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddr(7), NULL));

  fputc('.', stderr);

  // Test add entry for same UID updates existing entry with new address.
  emZclCacheRemoveAll();
  cachePopulate(3);
  assert(emZclCacheGetEntryCount() == 3);
  assert(emZclCacheGet(cacheExpectedUid(2), &actualAddress));
  assert(MEMCOMPARE(cacheExpectedAddr(2), &actualAddress, sizeof(EmberIpv6Address)) == 0);
  assert(emZclCacheGetFirstKeyForValue(cacheExpectedAddr(2), &actualUid));
  assert(MEMCOMPARE(cacheExpectedUid(2), &actualUid, sizeof(EmberZclUid_t)) == 0);
  assert(cacheAddEntry(cacheExpectedUid(2), cacheExpectedAddr(7)));
  assert(emZclCacheGetEntryCount() == 3);
  assert(emZclCacheGet(cacheExpectedUid(2), &actualAddress));
  assert(MEMCOMPARE(cacheExpectedAddr(2), &actualAddress, sizeof(EmberIpv6Address)) != 0);
  assert(MEMCOMPARE(cacheExpectedAddr(7), &actualAddress, sizeof(EmberIpv6Address)) == 0);
  assert(!emZclCacheGetFirstKeyForValue(cacheExpectedAddr(2), &actualUid));

  fputc('.', stderr);

  // Add max entries and test that next add reuses LRU entry (1).
  emZclCacheRemoveAll();
  cachePopulate(EMBER_ZCL_CACHE_TABLE_SIZE);
  uint8_t plusOne = EMBER_ZCL_CACHE_TABLE_SIZE + 1;
  assert(cacheAddEntry(cacheExpectedUid(plusOne), cacheExpectedAddr(plusOne)));
  assert(emZclCacheGetEntryCount() == EMBER_ZCL_CACHE_TABLE_SIZE);
  assert(!emZclCacheGet(cacheExpectedUid(1), &actualAddress));
  assert(emZclCacheGet(cacheExpectedUid(plusOne), &actualAddress));

  fputc('.', stderr);

  // Test remove all for Ipv6 prefix.
  const EmberIpv6Address prefix = { {
                                      0x20, 0x01, 0x0d, 0xb8, 0x85, 0x69, 0xb2, 0xb4,
                                    } };
  const uint8_t prefixLengthInBits = (7 * 8) + 6; // through upper 6 bits of 8th byte
  emZclCacheRemoveAll();
  // Populate with 1,4 and 2,3 with different prefixes.
  assert(cacheAddEntry(cacheExpectedUid(1), cacheExpectedAddrPrefix(1, 0)));
  assert(emZclCacheGetEntryCount() == 1);
  assert(cacheAddEntry(cacheExpectedUid(2), cacheExpectedAddrPrefix(2, 1)));
  assert(emZclCacheGetEntryCount() == 2);
  assert(cacheAddEntry(cacheExpectedUid(3), cacheExpectedAddrPrefix(3, 1)));
  assert(emZclCacheGetEntryCount() == 3);
  assert(cacheAddEntry(cacheExpectedUid(4), cacheExpectedAddrPrefix(4, 0)));
  assert(emZclCacheGetEntryCount() == 4);
  // Remove all with specified prefix, verify resulting absence/presence.
  assert(emZclCacheRemoveAllByIpv6Prefix(&prefix, prefixLengthInBits) == 2);
  assert(emZclCacheGetEntryCount() == 2);
  assert(emZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!emZclCacheGet(cacheExpectedUid(2), NULL));
  assert(!emZclCacheGet(cacheExpectedUid(3), NULL));
  assert(emZclCacheGet(cacheExpectedUid(4), NULL));

  fputc('.', stderr);

  // Test cache operations with cache index.
  emZclCacheRemoveAll();
  const int ccount = EMBER_ZCL_CACHE_TABLE_SIZE;
  cachePopulate(ccount); // note, index i will have uid/addr i+1
  // cachePrint();
  for (int i = 0; i < ccount; ++i) {
    EmZclCacheIndex_t index, otherIndex;
    index = i;
    MEMSET(actualUid.bytes, 0xAA, sizeof(EmberZclUid_t));
    MEMSET(actualAddress.bytes, 0xBB, sizeof(EmberIpv6Address));
    // printIUA(&index, &actualUid, &actualAddress);
    assert(emZclCacheGetByIndex(index, &actualUid, &actualAddress));
    // printIUA(&index, &actualUid, &actualAddress);
    assert(MEMCOMPARE(cacheExpectedUid(i + 1), &actualUid, sizeof(EmberZclUid_t)) == 0);
    assert(MEMCOMPARE(cacheExpectedAddr(i + 1), &actualAddress, sizeof(EmberIpv6Address)) == 0);
    assert(emZclCacheGetIndex(cacheExpectedUid(i + 1), &otherIndex));
    assert(index == otherIndex);
    //printIUA(NULL, &actualUid, NULL);
  }
  // cachePrint();
  fputc('.', stderr);

  fprintf(stderr, " done]\n");
  return 0;
}
