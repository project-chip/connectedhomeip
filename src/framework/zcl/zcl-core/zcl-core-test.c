/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE

#include "stack/ip/tls/tls.h"
#include "stack/ip/tls/tls-sha256.h"
#include "app/coap/coap.h"

#define USE_STUB_emIsDefaultDomainPrefix
#define USE_STUB_chipCoapSend
#define USE_STUB_chipGetNetworkParameters
#define USE_STUB_chipGetNodeId
#define USE_STUB_chipNetworkStatus
#define USE_STUB_chipReadIntegerOption
#define USE_STUB_emEventControlSetDelayMS
#define USE_STUB_emIsDefaultGlobalPrefix
#define USE_STUB_emIsLocalIpAddress
#define USE_STUB_emMacExtendedId
#define USE_STUB_emStoreDefaultGlobalPrefix
#define USE_STUB_chipUdpListen
#define USE_STUB_chipRemoveUdpListeners
#define USE_STUB_chipGetSecureDtlsSessionId
#define USE_STUB_chipGetDtlsConnectionPeerAddressBySessionId
#define USE_STUB_chipGetDtlsConnectionPeerPortBySessionId
#define USE_STUB_chipGetDtlsConnectionPeerPublicKeyBySessionId
#define USE_STUB_chipGetDtlsConnectionNextSessionId
#define USE_STUB_chipOpenDtlsConnection
#define USE_STUB_chipCloseDtlsConnection
#define USE_STUB_chipCoapRequestHandler
#define USE_STUB_emProcessCoapMessage
#define USE_STUB_chipReadLocationPath

#include "stack/ip/stubs.c"

void chZclHandler(ChipCoapCode code,
                  uint8_t *uri,
                  ChipCoapReadOptions *options,
                  const uint8_t *payload,
                  uint16_t payloadLength,
                  const ChipCoapRequestInfo *info);
void resetSystemTokens(void);

static const char *testUri;

const ChipZclClusterSpec_t chipZclCluster3456ClientSpec = {
  CHIP_ZCL_ROLE_CLIENT,
  CHIP_ZCL_MANUFACTURER_CODE_NULL,
  0x3456,
};
const ChipZclClusterSpec_t chipZclCluster3456ServerSpec = {
  CHIP_ZCL_ROLE_SERVER,
  CHIP_ZCL_MANUFACTURER_CODE_NULL,
  0x3456,
};
const ChipZclClusterSpec_t chipZclCluster34569abcClientSpec = {
  CHIP_ZCL_ROLE_CLIENT,
  0x3456,
  0x9abc,
};
const ChipZclClusterSpec_t chipZclCluster34569abcServerSpec = {
  CHIP_ZCL_ROLE_SERVER,
  0x3456,
  0x9abc,
};

// These are generated in sorted order by AppBuilder.
const ChipZclClusterSpec_t *chZclEndpoint1bSpec[] = {
  &chipZclCluster3456ClientSpec,
  &chipZclCluster34569abcClientSpec,
  &chipZclCluster3456ServerSpec,
  &chipZclCluster34569abcServerSpec,
  NULL,
};

const ChZclEndpointEntry_t chZclEndpointTable[] = {
  { 0x1b, 0xDEAD, chZclEndpoint1bSpec }
};
const size_t chZclEndpointCount = COUNTOF(chZclEndpointTable);

// These are generated in sorted order by AppBuilder.
const ChZclCommandEntry_t chZclCommandTable[] = {
  { &chipZclCluster3456ClientSpec, 0xf4, NULL, NULL },
  { &chipZclCluster34569abcClientSpec, 0xf4, NULL, NULL },
  { &chipZclCluster3456ServerSpec, 0xf4, NULL, NULL },
  { &chipZclCluster34569abcServerSpec, 0xf4, NULL, NULL },
};
const size_t chZclCommandCount = COUNTOF(chZclCommandTable);

// enum {
//   clustersOnEndpointHandler,
//   groupIdsOnDeviceHandler,
//   endpointsInGroupHandler,
//   endpointsOnDeviceHandler
// };

void chZclAccessTokenHandler(ChZclContext_t *context)
{
}

bool chZclAddIncomingToken(const ChipZclUid_t *remoteUid,
                           const uint8_t *tokenBytes,
                           uint16_t tokenLength)
{
  return false;
}

ChipZclStatus_t chZclAllowRemoteAccess(const uint8_t sessionId,
                                        const ChipZclClusterSpec_t *clusterSpec,
                                        ChipZclDeviceId_t endpointDeviceId,
                                        uint8_t accessType)
{
  return CHIP_ZCL_STATUS_SUCCESS;
}

bool chZclIsProtectedResource(const ChipZclClusterSpec_t *clusterSpec,
                              uint8_t accessType)
{
  return false;
}

bool chZclExtractBinaryAccessToken(CborState *state,
                                   uint8_t *output,
                                   uint16_t *outputSize)
{
  return false;
}

bool chipDtlsTransmitHandler(const uint8_t *payload,
                              uint16_t payloadLength,
                              const ChipIpv6Address *localAddress,
                              uint16_t localPort,
                              const ChipIpv6Address *remoteAddress,
                              uint16_t remotePort,
                              void *transmitHandlerData)
{
  return 0;
}

void chipZclGetPublicKeyCallback(const uint8_t **publicKey,
                                  uint16_t *publicKeySize)
{
}

bool chZclAttributeUriMetadataQueryParse(ChZclContext_t *context,
                                         void *data,
                                         uint8_t depth)
{
  assert(0);
  return false;
}

bool chZclAttributeUriQueryFilterParse(ChZclContext_t *context,
                                       void *data,
                                       uint8_t depth)
{
  assert(0);
  return false;
}

bool chZclAttributeUriQueryUndividedParse(ChZclContext_t *context,
                                          void *data,
                                          uint8_t depth)
{
  assert(0);
  return false;
}

void chZclClusterCommandHandler(ChZclContext_t *context)
{
  chZclRespond205ContentCbor(context->info, NULL, 0);
}
void chZclUriClusterAttributeHandler(ChZclContext_t *context)
{
  chZclRespond205ContentCbor(context->info, NULL, 0);
}
void chZclUriClusterAttributeIdHandler(ChZclContext_t *context)
{
  chZclRespond205ContentCbor(context->info, NULL, 0);
}

bool chZclHasReportingConfiguration(ChipZclEndpointId_t endpointId,
                                    const ChipZclClusterSpec_t *clusterSpec,
                                    ChipZclReportingConfigurationId_t reportingConfigurationId)
{
  return true;
}
void chZclUriClusterNotificationHandler(ChZclContext_t *context)
{
}
void chZclUriClusterReportingConfigurationHandler(ChZclContext_t *context)
{
}
void chZclUriClusterReportingConfigurationIdHandler(ChZclContext_t *context)
{
}

ChipZclStatus_t chipZclReadAttribute(ChipZclEndpointId_t endpointId,
                                       const ChipZclClusterSpec_t *clusterSpec,
                                       ChipZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength)
{
  return CHIP_ZCL_STATUS_SUCCESS;
}

const ChZclAttributeEntry_t *chZclFindAttribute(const ChipZclClusterSpec_t *clusterSpec,
                                                ChipZclAttributeId_t attributeId,
                                                bool includeRemote)
{
  return (ChZclAttributeEntry_t *) 1;  // any non-NULL value works
}

ChipStatus chZclSend(const ChipZclCoapEndpoint_t *destination,
                      ChipCoapCode code,
                      const uint8_t *uri,
                      const uint8_t *payload,
                      uint16_t payloadLength,
                      ChipCoapResponseHandler handler,
                      void *applicationData,
                      uint16_t applicationDataLength,
                      bool skipRetryEvent)
{
  return CHIP_SUCCESS;
}

ChipStatus chZclSendWithOptions(const ChipZclCoapEndpoint_t *destination,
                                 ChipCoapCode code,
                                 const uint8_t *uriPath,
                                 const ChipCoapOption options[],
                                 uint16_t optionsLength,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 ChipCoapResponseHandler handler,
                                 void *applicationData,
                                 uint16_t applicationDataLength,
                                 bool skipRetryEvent)
{
  return CHIP_SUCCESS;
}

void chipParseBlockOptionValue(uint32_t value,
                                ChipCoapBlockOption *option)
{
  option->more = (value & 0x08) != 0;
  option->logSize = (value & 0x07) + BLOCK_OPTION_VALUE_LOG_SIZE_MIN;
  option->number = value >> 4;
}

bool chipReadBlockOption(ChipCoapReadOptions *options,
                          ChipCoapOptionType type,
                          ChipCoapBlockOption *option)
{
  uint32_t value;
  if (chipReadIntegerOption(options, type, &value)) {
    chipParseBlockOptionValue(value, option);
    return true;
  } else {
    return false;
  }
}

uint32_t chipBlockOptionValue(bool more,
                               uint8_t logSize,
                               uint32_t number)
{
  assert(BLOCK_OPTION_VALUE_LOG_SIZE_MIN <= logSize && logSize <= BLOCK_OPTION_VALUE_LOG_SIZE_MAX);
  return ((logSize - BLOCK_OPTION_VALUE_LOG_SIZE_MIN)
          | (more ? 0x08 : 0)
          | (number << 4));
}

void chipInitCoapOption(ChipCoapOption *option,
                         ChipCoapOptionType type,
                         uint32_t value)
{
  option->type = type;
  option->value = NULL;
  option->valueLength = 0;
  option->intValue = value;
}

void chZclCoapStatusHandler(ChipCoapStatus status, ChipCoapResponseInfo *info)
{
}

static ChipCoapCode expectedResponseCode;

ChipStatus chipCoapRespondWithPath(const ChipCoapRequestInfo *requestInfo,
                                     ChipCoapCode code,
                                     const uint8_t *path,
                                     const ChipCoapOption *options,
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
  return CHIP_SUCCESS;
}

// chipResetReadOptionPointer and chipReadNextOption only work for URI paths,
// which must be stored as a NUL-terminated string in testUri.
const char *testUriFinger = NULL;
void chipResetReadOptionPointer(ChipCoapReadOptions *options)
{
  testUriFinger = testUri;
}
ChipCoapOptionType chipReadNextOption(ChipCoapReadOptions *options,
                                        const uint8_t **valuePointerLoc,
                                        uint16_t *valueLengthLoc)
{
  if (testUriFinger == NULL) {
    return CHIP_COAP_NO_OPTION;
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
  return CHIP_COAP_OPTION_URI_PATH;
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
                        ChipCoapCode requestCode,
                        const uint8_t *payload,
                        uint16_t payloadLength,
                        ChipCoapCode responseCode)
{
  // TODO: Set up info.
  ChipCoapRequestInfo info;
  memset(&info, 0, sizeof(info));
  testUri = uri;
  expectedResponseCode = responseCode;
  chZclHandler(requestCode,
               (uint8_t *)uri,
               NULL, // TODO: options
               payload,
               payloadLength,
               &info);
  if (expectedResponseCode != (ChipCoapCode) - 1) {
    fprintf(stderr, " URI %s got no reponse, want %d.%d\n",
            testUri,
            GET_COAP_CLASS(expectedResponseCode),
            GET_COAP_DETAIL(expectedResponseCode));
  }
  assert(expectedResponseCode == (ChipCoapCode) - 1);
}

#define uriTest(u, req, res) (requestTest((u), (req), NULL, 0, (res)))
#define uriGetTest(u, rc) (uriTest((u), CHIP_COAP_CODE_GET, (rc)))

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
      assert(chZclHexStringToInt(buffer, 1, &r1));
      assert(r0 == r1);
    } else {
      assert(!chZclHexStringToInt(buffer, 1, &r1));
    }

    // Read back what snprintf() writes.
    snprintf((char *)buffer, sizeof(buffer), "%x", i);
    assert(chZclHexStringToInt(buffer, strlen((char *)buffer), &r1));
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
      chZclIntToHexString(i, 1, buffer2);
      assert(strcmp((const char *)buffer1, (const char *)buffer2) == 0);
    }
    sprintf((char *)buffer1, "%x", i);
    chZclIntToHexString(i, 2, buffer2);
    assert(strcmp((const char *)buffer1, (const char *)buffer2) == 0);
  }
  uint32_t x = -1;
  uint8_t buffer3[2 * sizeof(x) + 1];
  chZclIntToHexString(x, sizeof(x), buffer3);
}

// CBOR-encoded binding for POSTing.
// {"u":"coap://[::]:4660/zcl/e/2","r":0}
static const int8u aBinding[] = {
  0xa2, 0x61, 0x75, 0x78, 0x18, 0x63, 0x6f, 0x61,
  0x70, 0x3a, 0x2f, 0x2f, 0x5b, 0x3a, 0x3a, 0x5d,
  0x3a, 0x34, 0x36, 0x36, 0x30, 0x2f, 0x7a, 0x63,
  0x6c, 0x2f, 0x65, 0x2f, 0x32, 0x61, 0x72, 0x00
};

static bool cacheAddEntry(const ChipZclUid_t *uid, const ChipIpv6Address *address)
{
  return chZclCacheAdd(uid, address, NULL);
}

static ChipZclUid_t cacheExpectedUidInstance;

static ChipZclUid_t *cacheExpectedUid(uint8_t uidNumber)
{
  MEMSET(&cacheExpectedUidInstance.bytes,
         uidNumber,
         sizeof(cacheExpectedUidInstance.bytes));
  return &cacheExpectedUidInstance;
}

const ChipIpv6Address ipv6Prefix[] = {
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

static ChipIpv6Address cacheExpectedAddrInstance;

static ChipIpv6Address *cacheExpectedAddrPrefix(uint8_t addrNumber, uint8_t prefixIndex)
{
  assert(prefixIndex <= sizeof(ipv6Prefix) / sizeof(ipv6Prefix[0]));
  MEMCOPY(&cacheExpectedAddrInstance,
          &ipv6Prefix[prefixIndex],
          sizeof(ChipIpv6Address));
  MEMSET(&cacheExpectedAddrInstance.bytes[12], addrNumber, 4);
  return &cacheExpectedAddrInstance;
}

static ChipIpv6Address *cacheExpectedAddr(uint8_t addrNumber)
{
  return cacheExpectedAddrPrefix(addrNumber, 0);
}

static void cachePopulate(uint8_t count)
{
  ChipZclUid_t returnedUid;
  ChipIpv6Address returnedAddress;
  uint8_t i;
  for (i = 1; i <= count; i++) {
    assert(cacheAddEntry(cacheExpectedUid(i), cacheExpectedAddr(i)));
    assert(chZclCacheGet(cacheExpectedUid(i), &returnedAddress));
    assert(MEMCOMPARE(cacheExpectedAddr(i), &returnedAddress, sizeof(ChipIpv6Address)) == 0);
    assert(chZclCacheGetFirstKeyForValue(cacheExpectedAddr(i), &returnedUid));
    assert(MEMCOMPARE(cacheExpectedUid(i), &returnedUid, sizeof(ChipZclUid_t)) == 0);
  }
}

//static void printIUA(const ChZclCacheIndex_t *index,
//                     const ChipZclUid_t *uid,
//                     const ChipIpv6Address *addr) {
//  if (index != NULL) {
//    fprintf(stderr, "\nIndx: %04x", *index);
//  }
//  if (uid != NULL) {
//    fprintf(stderr, "\nUID:  ");
//    for (int i = 0; i < sizeof(ChipZclUid_t); ++i) {
//      fprintf(stderr, "%02x ", uid->bytes[i]);
//    }
//  }
//  if (addr != NULL) {
//    fprintf(stderr, "\nIPv6: ");
//    for (int i = 0; i < sizeof(ChipIpv6Address); ++i) {
//      fprintf(stderr, "%02x ", addr->bytes[i]);
//    }
//  }
//  fprintf(stderr, "\n");
//}

// ScanPredicate
//static bool cachePrintEntry(const void *criteria, const ChZclCacheEntry_t *entry)
//{
//  printIUA(&entry->index, &entry->key.uid, &entry->value.address);
//  return false; // continue through all entries
//}

// Print cache entries in queue order
//static void cachePrint(void)
//{
//  chZclCacheScan(NULL, cachePrintEntry);
//}

int main(int argc, char *argv[])
{
  fprintf(stderr, "[%s ", argv[0]);

  resetSystemTokens();

  fputc('.', stderr);

  ChipZclBindingEntry_t entry;
  entry.destination.network.port = 0x1234;
  MEMCOPY(&entry.destination.network.data.address, "abcdefghijlmnopq", 16);
  entry.endpointId = 2;
  chipZclSetBinding(1, &entry);

  hexToIntTest();

  fputc('.', stderr);

  intToHexTest();

  fputc('.', stderr);

  const char **uris;

  for (uris = notFound; *uris != NULL; uris++) {
    uriGetTest(*uris, CHIP_COAP_CODE_404_NOT_FOUND);
  }

  fputc('.', stderr);

  uriTest("zcl/", CHIP_COAP_CODE_POST, CHIP_COAP_CODE_405_METHOD_NOT_ALLOWED);

  fputc('.', stderr);

  assert(chipZclAddEndpointToGroup(0x1b, 0x1b2c, NULL, 0, 0xFF, NULL, 0) == CHIP_ZCL_STATUS_SUCCESS);

  fputc('.', stderr);

  for (uris = found; *uris != NULL; uris++) {
    uriGetTest(*uris, CHIP_COAP_CODE_205_CONTENT);
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
                     ? CHIP_COAP_CODE_404_NOT_FOUND  // "g"
                     : CHIP_COAP_CODE_205_CONTENT)); // "e"
    const char **suffixes;
    for (suffixes = clusterSuffixes; *suffixes != 0; suffixes++) {
      sprintf(uri + count, "%s/", *suffixes);
      uriGetTest(uri,
                 (((*suffixes)[0] == 'c'
                   && 1 < strlen(*suffixes))
                  ? CHIP_COAP_CODE_405_METHOD_NOT_ALLOWED
                  : CHIP_COAP_CODE_205_CONTENT));
    }
  }

  fputc('.', stderr);

  uriGetTest("zcl/e/1b/c3456_9abc/b/0/", CHIP_COAP_CODE_404_NOT_FOUND);
  requestTest("zcl/e/1b/c3456_9abc/b/",
              CHIP_COAP_CODE_POST,
              aBinding,
              sizeof(aBinding),
              CHIP_COAP_CODE_201_CREATED);
  uriGetTest("zcl/e/1b/c3456_9abc/b/0/", CHIP_COAP_CODE_205_CONTENT);

  fputc('.', stderr);

  // From "16-07043-003-UIDTextForZCLIPSpecExplanation minor update 2016-11-09"
  // sent by Sander Raaijmakers to mesh_ip_tsc@mail.zigbee.org on Novchip 9,
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
  const ChipZclUid_t expectedUid = {
    .bytes = {
      0x36, 0xf6, 0xe4, 0x52, 0xb1, 0xca, 0xc0, 0x2a,
      0x4f, 0x6a, 0x6b, 0x4e, 0xf3, 0x41, 0xaf, 0x8e,
      0x92, 0x28, 0x1f, 0x25, 0x01, 0x5b, 0x08, 0x1f,
      0x86, 0x83, 0xa5, 0x12, 0x94, 0xb5, 0x6d, 0x46,
    },
  };
  const uint8_t expectedBase64Url[] = "NvbkUrHKwCpPamtO80GvjpIoHyUBWwgfhoOlEpS1bUY";

  ChipZclUid_t actualUid;
  emSha256Hash(input, sizeof(input), actualUid.bytes);
  assert(MEMCOMPARE(&expectedUid, &actualUid, sizeof(ChipZclUid_t)) == 0);

  uint8_t actualBase64Url[CHIP_ZCL_UID_BASE64URL_SIZE];
  MEMSET(actualBase64Url, 0, CHIP_ZCL_UID_BASE64URL_SIZE);
  assert(chZclUidToBase64Url(&expectedUid, CHIP_ZCL_UID_BITS, actualBase64Url) == CHIP_ZCL_UID_BASE64URL_LENGTH);
  assert(MEMCOMPARE(expectedBase64Url, actualBase64Url, CHIP_ZCL_UID_BASE64URL_LENGTH) == 0);
  MEMSET(&actualUid, 0, sizeof(ChipZclUid_t));
  uint16_t resultBits = 0;
  assert(chZclBase64UrlToUid(actualBase64Url, CHIP_ZCL_UID_BASE64URL_LENGTH, &actualUid, &resultBits));
  assert(MEMCOMPARE(&expectedUid, &actualUid, sizeof(ChipZclUid_t)) == 0);
  assert(resultBits == CHIP_ZCL_UID_BITS);

  fputc('.', stderr);

// ----------------------------------------------------------------------------
// UID cache tests
  ChipIpv6Address actualAddress;

  // Clear cache.
  chZclCacheRemoveAll();
  assert(chZclCacheGetEntryCount() == 0);
  assert(!chZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), NULL));
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddrPrefix(1, 1), NULL));
  assert(!chZclCacheRemove(&expectedUid));

  fputc('.', stderr);

  // Add an entry.
  assert(cacheAddEntry(cacheExpectedUid(1), cacheExpectedAddr(1)));
  assert(chZclCacheGetEntryCount() == 1);
  assert(chZclCacheGet(cacheExpectedUid(1), &actualAddress));
  assert(MEMCOMPARE(cacheExpectedAddr(1), &actualAddress, sizeof(ChipIpv6Address)) == 0);
  assert(chZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), &actualUid));
  assert(MEMCOMPARE(cacheExpectedUid(1), &actualUid, sizeof(ChipZclUid_t)) == 0);

  fputc('.', stderr);

  // Remove that entry.
  assert(chZclCacheRemove(cacheExpectedUid(1)));
  assert(chZclCacheGetEntryCount() == 0);
  assert(!chZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), NULL));

  fputc('.', stderr);

  // Test remove head (which is last one in: 3).
  chZclCacheRemoveAll();
  cachePopulate(3);
  assert(chZclCacheGetEntryCount() == 3);
  assert(chZclCacheRemove(cacheExpectedUid(3)));
  assert(chZclCacheGetEntryCount() == 2);
  assert(!chZclCacheGet(cacheExpectedUid(3), NULL));
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddr(3), NULL));
  assert(chZclCacheGet(cacheExpectedUid(1), NULL));
  assert(chZclCacheGet(cacheExpectedUid(2), NULL));

  fputc('.', stderr);

  // Test remove tail (which is first one in: 1).
  chZclCacheRemoveAll();
  cachePopulate(3);
  assert(chZclCacheGetEntryCount() == 3);
  assert(chZclCacheRemove(cacheExpectedUid(1)));
  assert(chZclCacheGetEntryCount() == 2);
  assert(!chZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddr(1), NULL));
  assert(chZclCacheGet(cacheExpectedUid(2), NULL));
  assert(chZclCacheGet(cacheExpectedUid(3), NULL));

  fputc('.', stderr);

  // Test remove interior entry (2).
  chZclCacheRemoveAll();
  cachePopulate(3);
  assert(chZclCacheGetEntryCount() == 3);
  assert(chZclCacheRemove(cacheExpectedUid(2)));
  assert(chZclCacheGetEntryCount() == 2);
  assert(!chZclCacheGet(cacheExpectedUid(2), NULL));
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddr(2), NULL));
  assert(chZclCacheGet(cacheExpectedUid(1), NULL));
  assert(chZclCacheGet(cacheExpectedUid(3), NULL));

  // Test remove all by address (entries positioned at head, tail, interior).
  chZclCacheRemoveAll();
  assert(cacheAddEntry(cacheExpectedUid(7), cacheExpectedAddr(7))); // tail
  cachePopulate(3); // 1-3
  assert(cacheAddEntry(cacheExpectedUid(4), cacheExpectedAddr(7)));
  assert(cacheAddEntry(cacheExpectedUid(5), cacheExpectedAddr(5)));
  assert(cacheAddEntry(cacheExpectedUid(6), cacheExpectedAddr(7))); // head
  assert(chZclCacheGetEntryCount() == 7);
  assert(chZclCacheRemoveAllByValue(cacheExpectedAddr(7)));
  assert(chZclCacheGetEntryCount() == 4);
  assert(chZclCacheGet(cacheExpectedUid(1), NULL));
  assert(chZclCacheGet(cacheExpectedUid(2), NULL));
  assert(chZclCacheGet(cacheExpectedUid(3), NULL));
  assert(!chZclCacheGet(cacheExpectedUid(4), NULL));
  assert(chZclCacheGet(cacheExpectedUid(5), NULL));
  assert(!chZclCacheGet(cacheExpectedUid(6), NULL));
  assert(!chZclCacheGet(cacheExpectedUid(7), NULL));
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddr(7), NULL));

  fputc('.', stderr);

  // Test add entry for same UID updates existing entry with new address.
  chZclCacheRemoveAll();
  cachePopulate(3);
  assert(chZclCacheGetEntryCount() == 3);
  assert(chZclCacheGet(cacheExpectedUid(2), &actualAddress));
  assert(MEMCOMPARE(cacheExpectedAddr(2), &actualAddress, sizeof(ChipIpv6Address)) == 0);
  assert(chZclCacheGetFirstKeyForValue(cacheExpectedAddr(2), &actualUid));
  assert(MEMCOMPARE(cacheExpectedUid(2), &actualUid, sizeof(ChipZclUid_t)) == 0);
  assert(cacheAddEntry(cacheExpectedUid(2), cacheExpectedAddr(7)));
  assert(chZclCacheGetEntryCount() == 3);
  assert(chZclCacheGet(cacheExpectedUid(2), &actualAddress));
  assert(MEMCOMPARE(cacheExpectedAddr(2), &actualAddress, sizeof(ChipIpv6Address)) != 0);
  assert(MEMCOMPARE(cacheExpectedAddr(7), &actualAddress, sizeof(ChipIpv6Address)) == 0);
  assert(!chZclCacheGetFirstKeyForValue(cacheExpectedAddr(2), &actualUid));

  fputc('.', stderr);

  // Add max entries and test that next add reuses LRU entry (1).
  chZclCacheRemoveAll();
  cachePopulate(CHIP_ZCL_CACHE_TABLE_SIZE);
  uint8_t plusOne = CHIP_ZCL_CACHE_TABLE_SIZE + 1;
  assert(cacheAddEntry(cacheExpectedUid(plusOne), cacheExpectedAddr(plusOne)));
  assert(chZclCacheGetEntryCount() == CHIP_ZCL_CACHE_TABLE_SIZE);
  assert(!chZclCacheGet(cacheExpectedUid(1), &actualAddress));
  assert(chZclCacheGet(cacheExpectedUid(plusOne), &actualAddress));

  fputc('.', stderr);

  // Test remove all for Ipv6 prefix.
  const ChipIpv6Address prefix = { {
                                      0x20, 0x01, 0x0d, 0xb8, 0x85, 0x69, 0xb2, 0xb4,
                                    } };
  const uint8_t prefixLengthInBits = (7 * 8) + 6; // through upper 6 bits of 8th byte
  chZclCacheRemoveAll();
  // Populate with 1,4 and 2,3 with different prefixes.
  assert(cacheAddEntry(cacheExpectedUid(1), cacheExpectedAddrPrefix(1, 0)));
  assert(chZclCacheGetEntryCount() == 1);
  assert(cacheAddEntry(cacheExpectedUid(2), cacheExpectedAddrPrefix(2, 1)));
  assert(chZclCacheGetEntryCount() == 2);
  assert(cacheAddEntry(cacheExpectedUid(3), cacheExpectedAddrPrefix(3, 1)));
  assert(chZclCacheGetEntryCount() == 3);
  assert(cacheAddEntry(cacheExpectedUid(4), cacheExpectedAddrPrefix(4, 0)));
  assert(chZclCacheGetEntryCount() == 4);
  // Remove all with specified prefix, verify resulting absence/presence.
  assert(chZclCacheRemoveAllByIpv6Prefix(&prefix, prefixLengthInBits) == 2);
  assert(chZclCacheGetEntryCount() == 2);
  assert(chZclCacheGet(cacheExpectedUid(1), NULL));
  assert(!chZclCacheGet(cacheExpectedUid(2), NULL));
  assert(!chZclCacheGet(cacheExpectedUid(3), NULL));
  assert(chZclCacheGet(cacheExpectedUid(4), NULL));

  fputc('.', stderr);

  // Test cache operations with cache index.
  chZclCacheRemoveAll();
  const int ccount = CHIP_ZCL_CACHE_TABLE_SIZE;
  cachePopulate(ccount); // note, index i will have uid/addr i+1
  // cachePrint();
  for (int i = 0; i < ccount; ++i) {
    ChZclCacheIndex_t index, otherIndex;
    index = i;
    MEMSET(actualUid.bytes, 0xAA, sizeof(ChipZclUid_t));
    MEMSET(actualAddress.bytes, 0xBB, sizeof(ChipIpv6Address));
    // printIUA(&index, &actualUid, &actualAddress);
    assert(chZclCacheGetByIndex(index, &actualUid, &actualAddress));
    // printIUA(&index, &actualUid, &actualAddress);
    assert(MEMCOMPARE(cacheExpectedUid(i + 1), &actualUid, sizeof(ChipZclUid_t)) == 0);
    assert(MEMCOMPARE(cacheExpectedAddr(i + 1), &actualAddress, sizeof(ChipIpv6Address)) == 0);
    assert(chZclCacheGetIndex(cacheExpectedUid(i + 1), &otherIndex));
    assert(index == otherIndex);
    //printIUA(NULL, &actualUid, NULL);
  }
  // cachePrint();
  fputc('.', stderr);

  fprintf(stderr, " done]\n");
  return 0;
}
