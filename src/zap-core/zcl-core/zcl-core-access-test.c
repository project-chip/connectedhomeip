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
 * @brief
 ******************************************************************************/

#define EMBER_AF_API_STACK "stack/include/ember.h"

#include PLATFORM_HEADER
#include "core/ember-stack.h"
#include "zclip-struct.h"
#include "cbor.h"
#include "zcl-core-types.h"
#include "zcl-core.h"

#define USE_STUB_emberVerifyEcdsaSignature
#define USE_STUB_emSha256Hash
#include "stack/ip/stubs.c"

//----------------------------------------------------------------
// Stuff temporarily copied from zcl-core-access.c for testing.

// We save the UIDs of the incoming tokens and set the payload link
// to be the audience string received in the token
// No payload link implies that all clusters are allowed
typedef struct {
  EmberZclUid_t remoteUid; // the uid of the client that own the token.
} RemoteTokenData_t;

typedef struct {
  uint8_t index;             // only used for tokens that we send to others
  const uint8_t *bytes;      // the actual token
  uint16_t length;           // length of the token
  uint16_t audienceOffset;   // offset of the 'aud' string
  uint16_t audienceLength;   // length of the 'aud' string
  EmberZclUid_t confUid;     // the uid of the client that own the token.
} TokenData_t;

extern bool emDecodeAccessToken(TokenData_t *token);

extern int16_t checkUidAccess(const uint8_t *accessString,
                              uint16_t accessStringLength,
                              EmberZclUid_t *targetUid,
                              uint8_t *clusterBuf,
                              uint16_t clusterBufLength);

extern bool checkClusterAccess(const uint8_t *accessString,
                               uint16_t accessStringLength,
                               const EmberZclClusterSpec_t *clusterSpec,
                               EmberZclDeviceId_t deviceId,
                               uint8_t accessType);

extern bool emCacheConfiguredToken(const uint8_t *tokenBytes, uint16_t tokenLength);

extern int8_t checkResourceAccess(EmberZclUid_t *targetUid,
                                  const RemoteTokenData_t *tokenData,
                                  const EmberZclClusterSpec_t *clusterSpec,
                                  int8_t accessType,
                                  uint16_t mask);

//----------------------------------------------------------------

// From the ZCLIP test spec

// Access token X.509 1, Server X.509 Keys 1, Client X.509 Keys 2, scope “zcl(s0.c)”
//  base64url = 2GKEQEBYTqIDaXpjbChzMC5jKRgZoWNraWR4OW5pOi8vL3NoYS0yNTY7a09OeklXQk1tMjlvcnZaRm53VmJIYUVLOVlvRDI1cjJaX29uc01LRXFIa4GDQEBYQB8R-CMKK3Trp6B58AecOZyjOxpW0MOO0bwqgvGsz4WPa7_uFhvC4v6PC8mduQO_GUbPti9ACQDBRepIr_MgPyA
static const uint8_t tokenX509_1[] = {
  0xD8, 0x62, 0x84, 0x40, 0x40, 0x58, 0x4E, 0xA2, 0x03, 0x69, 0x7A, 0x63,
  0x6C, 0x28, 0x73, 0x30, 0x2E, 0x63, 0x29, 0x18, 0x19, 0xA1, 0x63, 0x6B,
  0x69, 0x64, 0x78, 0x39, 0x6E, 0x69, 0x3A, 0x2F, 0x2F, 0x2F, 0x73, 0x68,
  0x61, 0x2D, 0x32, 0x35, 0x36, 0x3B, 0x6B, 0x4F, 0x4E, 0x7A, 0x49, 0x57,
  0x42, 0x4D, 0x6D, 0x32, 0x39, 0x6F, 0x72, 0x76, 0x5A, 0x46, 0x6E, 0x77,
  0x56, 0x62, 0x48, 0x61, 0x45, 0x4B, 0x39, 0x59, 0x6F, 0x44, 0x32, 0x35,
  0x72, 0x32, 0x5A, 0x5F, 0x6F, 0x6E, 0x73, 0x4D, 0x4B, 0x45, 0x71, 0x48,
  0x6B, 0x81, 0x83, 0x40, 0x40, 0x58, 0x40, 0x1F, 0x11, 0xF8, 0x23, 0x0A,
  0x2B, 0x74, 0xEB, 0xA7, 0xA0, 0x79, 0xF0, 0x07, 0x9C, 0x39, 0x9C, 0xA3,
  0x3B, 0x1A, 0x56, 0xD0, 0xC3, 0x8E, 0xD1, 0xBC, 0x2A, 0x82, 0xF1, 0xAC,
  0xCF, 0x85, 0x8F, 0x6B, 0xBF, 0xEE, 0x16, 0x1B, 0xC2, 0xE2, 0xFE, 0x8F,
  0x0B, 0xC9, 0x9D, 0xB9, 0x03, 0xBF, 0x19, 0x46, 0xCF, 0xB6, 0x2F, 0x40,
  0x09, 0x00, 0xC1, 0x45, 0xEA, 0x48, 0xAF, 0xF3, 0x20, 0x3F, 0x20
};

// Access token X.509 2, Server X.509 Keys 1, Client X.509 Keys 2, scope “zcl(ni:///sha-256;qm3mOb8SuoN1E5ASWPgmAyIcTbBinJH5IYe4xMDdfDE)”
//  base64url = 2GKEQEBYhKIDeD56Y2wobmk6Ly8vc2hhLTI1NjtxbTNtT2I4U3VvTjFFNUFTV1BnbUF5SWNUYkJpbkpINUlZZTR4TURkZkRFKRgZoWNraWR4OW5pOi8vL3NoYS0yNTY7a09OeklXQk1tMjlvcnZaRm53VmJIYUVLOVlvRDI1cjJaX29uc01LRXFIa4GDQEBYQBfI5mHMOk3FnwT09J9XZRcJi_cZD5-0WmzRzgHdsJr0PUcFabRQV7igfhJ1wgD1RlB5k6whynU2ZyYLyYcYfH0
static const uint8_t tokenX509_2[] = {
  0xD8, 0x62, 0x84, 0x40, 0x40, 0x58, 0x84, 0xA2, 0x03, 0x78, 0x3E, 0x7A,
  0x63, 0x6C, 0x28, 0x6E, 0x69, 0x3A, 0x2F, 0x2F, 0x2F, 0x73, 0x68, 0x61,
  0x2D, 0x32, 0x35, 0x36, 0x3B, 0x71, 0x6D, 0x33, 0x6D, 0x4F, 0x62, 0x38,
  0x53, 0x75, 0x6F, 0x4E, 0x31, 0x45, 0x35, 0x41, 0x53, 0x57, 0x50, 0x67,
  0x6D, 0x41, 0x79, 0x49, 0x63, 0x54, 0x62, 0x42, 0x69, 0x6E, 0x4A, 0x48,
  0x35, 0x49, 0x59, 0x65, 0x34, 0x78, 0x4D, 0x44, 0x64, 0x66, 0x44, 0x45,
  0x29, 0x18, 0x19, 0xA1, 0x63, 0x6B, 0x69, 0x64, 0x78, 0x39, 0x6E, 0x69,
  0x3A, 0x2F, 0x2F, 0x2F, 0x73, 0x68, 0x61, 0x2D, 0x32, 0x35, 0x36, 0x3B,
  0x6B, 0x4F, 0x4E, 0x7A, 0x49, 0x57, 0x42, 0x4D, 0x6D, 0x32, 0x39, 0x6F,
  0x72, 0x76, 0x5A, 0x46, 0x6E, 0x77, 0x56, 0x62, 0x48, 0x61, 0x45, 0x4B,
  0x39, 0x59, 0x6F, 0x44, 0x32, 0x35, 0x72, 0x32, 0x5A, 0x5F, 0x6F, 0x6E,
  0x73, 0x4D, 0x4B, 0x45, 0x71, 0x48, 0x6B, 0x81, 0x83, 0x40, 0x40, 0x58,
  0x40, 0x17, 0xC8, 0xE6, 0x61, 0xCC, 0x3A, 0x4D, 0xC5, 0x9F, 0x04, 0xF4,
  0xF4, 0x9F, 0x57, 0x65, 0x17, 0x09, 0x8B, 0xF7, 0x19, 0x0F, 0x9F, 0xB4,
  0x5A, 0x6C, 0xD1, 0xCE, 0x01, 0xDD, 0xB0, 0x9A, 0xF4, 0x3D, 0x47, 0x05,
  0x69, 0xB4, 0x50, 0x57, 0xB8, 0xA0, 0x7E, 0x12, 0x75, 0xC2, 0x00, 0xF5,
  0x46, 0x50, 0x79, 0x93, 0xAC, 0x21, 0xCA, 0x75, 0x36, 0x67, 0x26, 0x0B,
  0xC9, 0x87, 0x18, 0x7C, 0x7D
};

// Access token X.509 3, Server X.509 Keys 1, Client X.509 Keys 2, scope “zcl(d.102)”
//  base64url = 2GKEQEBYT6IDanpjbChkLjEwMikYGaFja2lkeDluaTovLy9zaGEtMjU2O2tPTnpJV0JNbTI5b3J2WkZud1ZiSGFFSzlZb0QyNXIyWl9vbnNNS0VxSGuBg0BAWEBvsDj5k_4xP4YhqC5E0blx7l1slnbWAJQ1q3VFnc7cqGXhzbC-3f4PVEVwVLUKQ_BKfzFBHJ5HmMd1FJJgtKm2
static const uint8_t tokenX509_3[] = {
  0xD8, 0x62, 0x84, 0x40, 0x40, 0x58, 0x4F, 0xA2, 0x03, 0x6A, 0x7A, 0x63,
  0x6C, 0x28, 0x64, 0x2E, 0x31, 0x30, 0x32, 0x29, 0x18, 0x19, 0xA1, 0x63,
  0x6B, 0x69, 0x64, 0x78, 0x39, 0x6E, 0x69, 0x3A, 0x2F, 0x2F, 0x2F, 0x73,
  0x68, 0x61, 0x2D, 0x32, 0x35, 0x36, 0x3B, 0x6B, 0x4F, 0x4E, 0x7A, 0x49,
  0x57, 0x42, 0x4D, 0x6D, 0x32, 0x39, 0x6F, 0x72, 0x76, 0x5A, 0x46, 0x6E,
  0x77, 0x56, 0x62, 0x48, 0x61, 0x45, 0x4B, 0x39, 0x59, 0x6F, 0x44, 0x32,
  0x35, 0x72, 0x32, 0x5A, 0x5F, 0x6F, 0x6E, 0x73, 0x4D, 0x4B, 0x45, 0x71,
  0x48, 0x6B, 0x81, 0x83, 0x40, 0x40, 0x58, 0x40, 0x6F, 0xB0, 0x38, 0xF9,
  0x93, 0xFE, 0x31, 0x3F, 0x86, 0x21, 0xA8, 0x2E, 0x44, 0xD1, 0xB9, 0x71,
  0xEE, 0x5D, 0x6C, 0x96, 0x76, 0xD6, 0x00, 0x94, 0x35, 0xAB, 0x75, 0x45,
  0x9D, 0xCE, 0xDC, 0xA8, 0x65, 0xE1, 0xCD, 0xB0, 0xBE, 0xDD, 0xFE, 0x0F,
  0x54, 0x45, 0x70, 0x54, 0xB5, 0x0A, 0x43, 0xF0, 0x4A, 0x7F, 0x31, 0x41,
  0x1C, 0x9E, 0x47, 0x98, 0xC7, 0x75, 0x14, 0x92, 0x60, 0xB4, 0xA9, 0xB6
};

// The owning UID in tokenX509_1, tokenX509_2, tokenX509_3.
static const EmberZclUid_t uidX509 = {
  {
    0x90, 0xE3, 0x73, 0x21, 0x60, 0x4C, 0x9B, 0x6F,
    0x68, 0xAE, 0xF6, 0x45, 0x9F, 0x05, 0x5B, 0x1D,
    0xA1, 0x0A, 0xF5, 0x8A, 0x03, 0xDB, 0x9A, 0xF6,
    0x67, 0xFA, 0x27, 0xB0, 0xC2, 0x84, 0xA8, 0x79
  }
};

// Access token RPK 1, Server RPK Keys 1, Client RPK Keys 2, aud “zcl(s0.c)”
static const uint8_t tokenRpk1[] = {
  0xD8, 0x62, 0x84, 0x40, 0x40, 0x58, 0x4E, 0xA2, 0x03, 0x69, 0x7A, 0x63,
  0x6C, 0x28, 0x73, 0x30, 0x2E, 0x63, 0x29, 0x18, 0x19, 0xA1, 0x63, 0x6B,
  0x69, 0x64, 0x78, 0x39, 0x6E, 0x69, 0x3A, 0x2F, 0x2F, 0x2F, 0x73, 0x68,
  0x61, 0x2D, 0x32, 0x35, 0x36, 0x3B, 0x31, 0x4A, 0x59, 0x58, 0x71, 0x56,
  0x53, 0x68, 0x63, 0x43, 0x4B, 0x74, 0x65, 0x4A, 0x5F, 0x31, 0x61, 0x56,
  0x4D, 0x77, 0x78, 0x6D, 0x46, 0x6D, 0x58, 0x38, 0x6A, 0x2D, 0x36, 0x50,
  0x50, 0x54, 0x70, 0x61, 0x38, 0x64, 0x50, 0x42, 0x61, 0x38, 0x6D, 0x77,
  0x67, 0x81, 0x83, 0x40, 0x40, 0x58, 0x40, 0x90, 0xC4, 0x3F, 0xCB, 0x64,
  0xAF, 0xB2, 0x25, 0xB7, 0x01, 0xC0, 0xE4, 0xF4, 0x22, 0x5C, 0xAC, 0x57,
  0x49, 0xC3, 0xA1, 0x91, 0x98, 0x2C, 0x75, 0x20, 0x16, 0xB5, 0xDE, 0x9C,
  0x3B, 0xFD, 0xCB, 0xD7, 0x1D, 0x7D, 0x71, 0xE7, 0x56, 0xF1, 0xBF, 0x41,
  0xAB, 0xB6, 0x37, 0xB0, 0xA0, 0x9E, 0x0D, 0xE4, 0x99, 0x66, 0x06, 0xB8,
  0x9A, 0x27, 0xDB, 0xAF, 0x24, 0x95, 0xC1, 0x9D, 0xCD, 0x83, 0x23
};

// The owning UID in tokenRpk1.
static const EmberZclUid_t rpk1Uid = {
  {
    0xD4, 0x96, 0x17, 0xA9, 0x54, 0xA1, 0x70, 0x22,
    0xAD, 0x78, 0x9F, 0xF5, 0x69, 0x53, 0x30, 0xC6,
    0x61, 0x66, 0x5F, 0xC8, 0xFE, 0xE8, 0xF3, 0xD3,
    0xA5, 0xAF, 0x1D, 0x3C, 0x16, 0xBC, 0x9B, 0x08
  }
};

// The UID in scope of tokenX509_2.
EmberZclUid_t emZclUid = {
  {
    0xAA, 0x6D, 0xE6, 0x39, 0xBF, 0x12, 0xBA, 0x83,
    0x75, 0x13, 0x90, 0x12, 0x58, 0xF8, 0x26, 0x03,
    0x22, 0x1C, 0x4D, 0xB0, 0x62, 0x9C, 0x91, 0xF9,
    0x21, 0x87, 0xB8, 0xC4, 0xC0, 0xDD, 0x7C, 0x31
  }
};

EmberZclUid_t emRemoteUid;

extern Buffer remoteTokens;
extern bool emZclUseAccessControl;

//----------------------------------------------------------------
// Stubs for zcl-core-access.c: TODO: clean up

EmberStatus emberZclRespondWithPath(const EmberCoapRequestInfo *requestInfo,
                                    EmberCoapCode code,
                                    const uint8_t *path,
                                    const EmberCoapOption *options,
                                    uint8_t numberOfOptions,
                                    const uint8_t *payload,
                                    uint16_t payloadLength)
{
  return 0;
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

EmberStatus emberZclDtlsManagerGetUidBySessionId(const uint8_t sessionId,
                                                 EmberZclUid_t *remoteUid)
{
  // assume we have session 1 open with uidX509
  if (sessionId == 1) {
    *remoteUid = uidX509;
    return EMBER_SUCCESS;
  }
  return EMBER_ERR_FATAL;
}

//----------------------------------------------------------------
// Stubs for zcl-core-uri.c:

bool emberIpv6StringToAddress(const uint8_t *src, EmberIpv6Address *dst)
{
  return false;
}
bool emberIpv6AddressToString(const EmberIpv6Address *src,
                              uint8_t *dst,
                              size_t dstSize)
{
  return false;
}

extern bool emHexStringToInt(const uint8_t *chars,
                             size_t length,
                             uintmax_t *result);

//----------------------------------------------------------------

// static void printBuffer(uint8_t *buffer, uint16_t length)
// {
//   uint16_t i;
//   fprintf(stderr, "\n%d bytes\n", length);
//   for (i = 0; i < length; i++) {
//     fprintf(stderr, " %02X", buffer[i]);
//   }
//   fprintf(stderr, "\n");
// }

static void protectedResourceTest(void)
{
  EmberZclClusterSpec_t clusterSpec;
  clusterSpec.role = EMBER_ZCL_ROLE_SERVER;
  clusterSpec.manufacturerCode = EMBER_ZCL_MANUFACTURER_CODE_NULL;

  clusterSpec.id = 0x0000; // EMBER_ZCL_CLUSTER_BASIC
  assert(emZclIsProtectedResource(&clusterSpec, 'c'));
  assert(emZclIsProtectedResource(&clusterSpec, 'r'));
  assert(emZclIsProtectedResource(&clusterSpec, 'b'));
  assert(emZclIsProtectedResource(&clusterSpec, 'a'));

  clusterSpec.id = 0x2000; // EMBER_ZCL_CLUSTER_OTA_BOOTLOAD
  assert(emZclIsProtectedResource(&clusterSpec, 'c'));
  assert(emZclIsProtectedResource(&clusterSpec, 'r'));
  assert(emZclIsProtectedResource(&clusterSpec, 'b'));
  assert(emZclIsProtectedResource(&clusterSpec, 'a'));

  clusterSpec.id = 0x0003; // EMBER_ZCL_CLUSTER_IDENTIFY
  assert(!emZclIsProtectedResource(&clusterSpec, 'c'));
  assert(!emZclIsProtectedResource(&clusterSpec, 'r'));
  assert(!emZclIsProtectedResource(&clusterSpec, 'b'));
  assert(!emZclIsProtectedResource(&clusterSpec, 'a'));
}

static void extractBinaryTokenTest(void)
{
  char *tokenX509_1_base64url
    = "2GKEQEBYTqIDaXpjbChzMC5jKRgZoWNraWR4OW5pOi8vL3NoYS0yNTY7a09Oekl"
      "XQk1tMjlvcnZaRm53VmJIYUVLOVlvRDI1cjJaX29uc01LRXFIa4GDQEBYQB8R-C"
      "MKK3Trp6B58AecOZyjOxpW0MOO0bwqgvGsz4WPa7_uFhvC4v6PC8mduQO_GUbPt"
      "i9ACQDBRepIr_MgPyA";
  char payload[EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN];
  uint16_t outputSize = 0;
  uint8_t output[EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN];
  CborState state;

  // encode CBOR map e.g. {19:"tokenX509_1_base64url"}
  emCborEncodeMapStart(&state, payload, EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN, 1);
  assert(emCborEncodeMapEntry(&state,
                              EM_ZCL_ACCESS_TOKEN_KEY,
                              EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                              sizeof(tokenX509_1_base64url),
                              (uint8_t *)tokenX509_1_base64url));
  // printBuffer(payload, state.finger - state.start);

  // decode CBOR map - check that outputSize can hold the decoded payload
  emCborDecodeStart(&state, payload, emCborEncodeSize(&state));
  assert(!emZclExtractBinaryAccessToken(&state, output, &outputSize)); // fail as outputSize=0

  // decode CBOR map - after fixing outputSize to the size of output array
  emCborDecodeStart(&state, payload, emCborEncodeSize(&state));
  outputSize = EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN;
  assert(emZclExtractBinaryAccessToken(&state, output, &outputSize)); // success as outputSize can hold output
  assert(memcmp(output, tokenX509_1, outputSize) == 0); // extracted actual binary token

  // encode CBOR map with wrong key e.g. {2:"tokenX509_1_base64url"}
  emCborEncodeMapStart(&state, payload, EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN, 1);
  assert(emCborEncodeMapEntry(&state,
                              2,
                              EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                              sizeof(tokenX509_1_base64url),
                              (uint8_t *)tokenX509_1_base64url));
  // decode CBOR map - using wrong access token key (2)
  emCborDecodeStart(&state, payload, emCborEncodeSize(&state));
  outputSize = EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN;
  assert(!emZclExtractBinaryAccessToken(&state, output, &outputSize)); // fail as wrong key (2)
}

static void parseTest(void)
{
  TokenData_t token;

  memset(&token, 0, sizeof(token));
  token.bytes = tokenRpk1;
  token.length = sizeof(tokenRpk1);

  assert(emDecodeAccessToken(&token));
  uint8_t foo[100];
  memcpy(foo, token.bytes + token.audienceOffset, token.audienceLength);
  foo[token.audienceLength] = 0;
  // fprintf(stderr, "[aud: '%s']\n", foo);
  EmberZclClusterSpec_t clusterSpec;
  clusterSpec.role = EMBER_ZCL_ROLE_SERVER;
  clusterSpec.manufacturerCode = EMBER_ZCL_MANUFACTURER_CODE_NULL;
  clusterSpec.id = 0;
  assert(checkUidAccess(token.bytes + token.audienceOffset,
                        token.audienceLength,
                        &emZclUid,
                        NULL,
                        0)
         == 5); // the length of the 'aud' string in the token
  assert(checkClusterAccess(token.bytes + token.audienceOffset,
                            token.audienceLength,
                            &clusterSpec,
                            EMBER_ZCL_DEVICE_ID_NULL,
                            'c'));
  clusterSpec.id = 1;
  assert(!checkClusterAccess(token.bytes + token.audienceOffset,
                             token.audienceLength,
                             &clusterSpec,
                             EMBER_ZCL_DEVICE_ID_NULL,
                             'c'));
}

static void localTokenTest(void)
{
  memcpy(&emRemoteUid, &rpk1Uid, sizeof(EmberZclUid_t));

  Buffer tokenBuffer = emAllocateBuffer(sizeof(RemoteTokenData_t));
  RemoteTokenData_t *token = (RemoteTokenData_t *)emGetBufferPointer(tokenBuffer);
  memset(token, 0, sizeof(RemoteTokenData_t));
  memcpy(&token->remoteUid, &emRemoteUid, sizeof(EmberZclUid_t));

  EmberZclClusterSpec_t clusterSpec;
  clusterSpec.role = EMBER_ZCL_ROLE_SERVER;
  clusterSpec.manufacturerCode = EMBER_ZCL_MANUFACTURER_CODE_NULL;

  clusterSpec.id = 1;
  assert(checkResourceAccess(&emRemoteUid, NULL, &clusterSpec, 'c', 0x1) == -2);  // -2 == unprotected access (dtls session not required)

  clusterSpec.id = 0;
  assert(checkResourceAccess(&emZclUid, token, &clusterSpec, 'c', 0xFFFF) == -1);
  assert(emCacheConfiguredToken(tokenRpk1, sizeof(tokenRpk1)));
  assert(checkResourceAccess(&emZclUid, token, &clusterSpec, 'c', 0x0) == -1);
  assert(checkResourceAccess(&emZclUid, token, &clusterSpec, 'c', 0x2) == -1);
  assert(checkResourceAccess(&emZclUid, token, &clusterSpec, 'c', 0x1) == 0);
  assert(checkResourceAccess(&emRemoteUid, token, &clusterSpec, 'c', 0x1) == -1);
}

static void receivedTokenTest(void)
{
  // Processing the same token twice should have no additional effect.
  int i;
  Buffer queueBuffer;
  Buffer clusterBuffer;
  for (i = 0; i < 2; i++) {
    assert(emZclAddIncomingToken(&uidX509,
                                 tokenX509_1,
                                 sizeof(tokenX509_1)));
    assert(emBufferQueueLength(&remoteTokens) == 1);
    queueBuffer = emBufferQueueHead(&remoteTokens);
    assert(queueBuffer != NULL_BUFFER);
    assert(memcmp(emGetBufferPointer(queueBuffer), &uidX509, sizeof(EmberZclUid_t)) == 0);
    clusterBuffer = emGetPayloadLink(queueBuffer);
    assert(clusterBuffer != NULL_BUFFER);
    assert(emGetBufferLength(clusterBuffer) == 5);
    assert(memcmp(emGetBufferPointer(clusterBuffer), "s0.c ", 5) == 0);
  }

  uint8_t sessionId = 1; // we assume this is open with uidX509
  EmberZclClusterSpec_t clusterSpec;
  clusterSpec.role = EMBER_ZCL_ROLE_SERVER;
  clusterSpec.manufacturerCode = EMBER_ZCL_MANUFACTURER_CODE_NULL;
  clusterSpec.id = 0x0000;

  // restricted resource - access token grants access
  assert(emZclAllowRemoteAccess(sessionId,
                                &clusterSpec,
                                EMBER_ZCL_DEVICE_ID_NULL,
                                'c') == EMBER_ZCL_STATUS_SUCCESS);

  // restricted resource - access token does not grant access
  assert(emZclAllowRemoteAccess(sessionId,
                                &clusterSpec,
                                EMBER_ZCL_DEVICE_ID_NULL,
                                'a') == EMBER_ZCL_STATUS_FORBIDDEN);

  // restricted resource - access token does not grant access
  clusterSpec.id = 0x2000;
  assert(emZclAllowRemoteAccess(sessionId,
                                &clusterSpec,
                                EMBER_ZCL_DEVICE_ID_NULL,
                                'a') == EMBER_ZCL_STATUS_FORBIDDEN);

  // restricted resource - access token grants access but wrong DTLS session used
  assert(emZclAllowRemoteAccess(0,
                                &clusterSpec,
                                EMBER_ZCL_DEVICE_ID_NULL,
                                'c') == EMBER_ZCL_STATUS_FAILURE);

  // add token - check that it was added to the head of the queue
  assert(emZclAddIncomingToken(&uidX509,
                               tokenX509_2,
                               sizeof(tokenX509_2)));
  assert(emBufferQueueLength(&remoteTokens) == 2);
  queueBuffer = emBufferQueueHead(&remoteTokens);
  assert(memcmp(emGetBufferPointer(queueBuffer), &uidX509, sizeof(EmberZclUid_t)) == 0);
  assert(queueBuffer != NULL_BUFFER);
  clusterBuffer = emGetPayloadLink(queueBuffer);
  assert(clusterBuffer == NULL_BUFFER); // grants access to everything
  assert(emGetBufferLength(clusterBuffer) == 0);
  assert(emGetBufferPointer(clusterBuffer) == NULL);

  // add token - check that it was added to the head of the queue
  //             check that oldest item is removed from queue, as max capacity is 2
  assert(emZclAddIncomingToken(&uidX509,
                               tokenX509_3,
                               sizeof(tokenX509_3)));
  assert(emBufferQueueLength(&remoteTokens) == 2);
  queueBuffer = emBufferQueueHead(&remoteTokens);
  assert(memcmp(emGetBufferPointer(queueBuffer), &uidX509, sizeof(EmberZclUid_t)) == 0);
  assert(queueBuffer != NULL_BUFFER);
  clusterBuffer = emGetPayloadLink(queueBuffer);
  assert(clusterBuffer != NULL_BUFFER);
  assert(emGetBufferLength(clusterBuffer) == 6);
  assert(memcmp(emGetBufferPointer(clusterBuffer), "d.102 ", 6) == 0);
  // move pointer to second item in queue and check that it was the previously added one
  queueBuffer = emBufferQueueNext(&remoteTokens, queueBuffer);
  assert(memcmp(emGetBufferPointer(queueBuffer), &uidX509, sizeof(EmberZclUid_t)) == 0);
  assert(queueBuffer != NULL_BUFFER);
  clusterBuffer = emGetPayloadLink(queueBuffer);
  assert(clusterBuffer == NULL_BUFFER);
  assert(emGetBufferLength(clusterBuffer) == 0);
  assert(emGetBufferPointer(clusterBuffer) == NULL);
  // move pointer to next item, which should be NULL as it is end of queue
  queueBuffer = emBufferQueueNext(&remoteTokens, queueBuffer);
  assert(queueBuffer == NULL_BUFFER);

  // adding token with incorrect UID should fail
  assert(!emZclAddIncomingToken(&emZclUid,
                                tokenX509_1,
                                sizeof(tokenX509_1)));
  assert(!emZclAddIncomingToken(&emZclUid,
                                tokenX509_2,
                                sizeof(tokenX509_2)));
  assert(!emZclAddIncomingToken(&emZclUid,
                                tokenX509_3,
                                sizeof(tokenX509_3)));
  assert(emBufferQueueLength(&remoteTokens) == 2);

  // restricted resources - tokenX509_2 grants access to everything
  clusterSpec.id = 0x0000;
  assert(emZclAllowRemoteAccess(sessionId,
                                &clusterSpec,
                                EMBER_ZCL_DEVICE_ID_NULL,
                                'a') == EMBER_ZCL_STATUS_SUCCESS);
  clusterSpec.id = 0x2000;
  assert(emZclAllowRemoteAccess(sessionId,
                                &clusterSpec,
                                EMBER_ZCL_DEVICE_ID_NULL,
                                'c') == EMBER_ZCL_STATUS_SUCCESS);
}

const EmZclEndpointEntry_t emZclEndpointTable[] = {
  { 0, 0, NULL }
};

const size_t emZclEndpointCount = 1;

int main(int argc, char *argv[])
{
  fprintf(stderr, "[%s ", argv[0]);

  emInitializeBuffers();

  emZclUseAccessControl = ZCL_ACCESS_CONTROL_ON;
  protectedResourceTest();
  extractBinaryTokenTest();
  parseTest();
  localTokenTest();
  receivedTokenTest();

  fprintf(stderr, " done]\n");
  return 0;
}
