/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_BUFFER_QUEUE
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_DTLS_MANAGER
#include "stack/ip/tls/tls.h"
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#else
  #define emberAfPluginZclCorePrint(...)
  #define emberAfPluginZclCorePrintln(...)
  #define emberAfPluginZclCoreFlush()
  #define emberAfPluginZclCoreDebugExec(x)
  #define emberAfPluginZclCorePrintBuffer(buffer, len, withSpace)
  #define emberAfPluginZclCorePrintString(buffer)
#endif

extern void emSha256Hash(const uint8_t *input, int count, uint8_t *output);

extern bool emberVerifyEcdsaSignature(const uint8_t *hash, EccPublicKey *key,
                                      const uint8_t *r, uint16_t rLength, const uint8_t *s,
                                      uint16_t sLength);

// global variable allows test apps to disable access control via cli.
// (otherwise test apps need to provide all necessary access tokens).
// ZCL_ACCESS_CONTROL_ON_TEST enables access control and access token
// signature verification with hardcoded public key
// TODO: Implement an API for setting Commissioning Tool public key
emZclAccessControlMode_t emZclUseAccessControl = ZCL_ACCESS_CONTROL_OFF;

// This file has several related chunks of functionality:
// - Maintaining information about the current set of DTLS sessions.
// - Parsing access tokens.
// - Verifying that an access token allows a particular action.
// - Caching information about access tokens that have been configured
//   on this device.
// - Caching information about access tokens received over DTLS connections.

// Two needed functions that don't currently exist.  The local UID
// value will be obtained from the node's operational certificate,
// once we have such.
extern EmberZclUid_t emZclUid;
bool emIsOurUid(const EmberZclUid_t *uid)
{
  return (memcmp(uid, &emZclUid, sizeof(EmberZclUid_t)) == 0);
}

// We save the UIDs of the incoming tokens and set the payload link
// to be the audience string received in the token
// No payload link implies that all clusters are allowed
typedef struct {
  EmberZclUid_t remoteUid; // the uid of the client that own the token.
} RemoteTokenData_t;

Buffer remoteTokens = NULL_BUFFER;

// Struct holding information about an access token that installed on
// the local node.  Once these are in NVM3 we may need to store an NVM
// token identifier in place of a pointer to the bytes.  This is also
// why we store an offset to the audience rather than a pointer.
//
// 'bytes' and 'length' are used when we send a token to another
// device.  The 'aud' string is used to check what permissions the
// token gives us.
typedef struct {
  uint8_t index;             // only used for tokens that we send to others
  const uint8_t *bytes;      // the actual token
  uint16_t length;           // length of the token
  uint16_t audienceOffset;   // offset of the 'aud' string
  uint16_t audienceLength;   // length of the 'aud' string
  EmberZclUid_t confUid;     // the uid of the client that own the token.
} TokenData_t;

// This contains a set of TokenData_t records for tokens that have our
// UID and are sent to remote nodes.  We cache the data to avoid
// having to reparse the tokens whenever we need to check permissions.
static Buffer localTokens = NULL_BUFFER;

// Mask of the token indexes that are currently in use.
static uint16_t localTokenIndexes;

static int8_t checkLocalAccess(EmberZclUid_t *targetUid,
                               const RemoteTokenData_t *tokenData,
                               const EmberZclClusterSpec_t *clusterSpec,
                               uint8_t accessType,
                               uint16_t mask);
static int8_t checkRemoteAccess(EmberZclUid_t *targetUid,
                                const RemoteTokenData_t *tokenData,
                                const EmberZclClusterSpec_t *clusterSpec,
                                uint8_t accessType,
                                uint16_t mask);
bool checkClusterAccess(const uint8_t *accessString,
                        uint16_t accessStringLength,
                        const EmberZclClusterSpec_t *clusterSpec,
                        EmberZclDeviceId_t deviceId,
                        uint8_t accessType);

//----------------------------------------------------------------
// Private functions

#ifndef EM_ZCL_MAX_REMOTE_ACCESS_TOKENS
  #define EM_ZCL_MAX_REMOTE_ACCESS_TOKENS 3 // TODO: pluginize this?
#endif

static void printString(const uint8_t *buffer,
                        uint8_t length,
                        bool newline)
{
  for (uint8_t i = 0; i < length; i++) {
    emberAfPluginZclCorePrint("%c", buffer[i]);
  }
  if (newline) {
    emberAfPluginZclCorePrintln("");
  }
}

void emZclPrintAccessTokens(void)
{
  emberAfPluginZclCorePrintln("Access Control: Remote Tokens");
  uint8_t index = 0;
  for (Buffer buffer = emBufferQueueHead(&remoteTokens);
       buffer != NULL_BUFFER;
       buffer = emBufferQueueNext(&remoteTokens, buffer)) {
    const RemoteTokenData_t *entry
      = (const RemoteTokenData_t *)emGetBufferPointer(buffer);

    uint8_t result[EMBER_ZCL_UID_STRING_SIZE];
    emZclUidToBase64Url(&entry->remoteUid, EMBER_ZCL_UID_BITS, result);
    emberAfPluginZclCorePrint("index: %d | uid: %s | ", index, result);
    index++;

    Buffer accessBuffer = emGetPayloadLink(buffer);
    uint16_t accessBufferLength = emGetBufferLength(accessBuffer);
    if (accessBufferLength == 0) {
      emberAfPluginZclCorePrint("access: all");
    } else {
      uint8_t* accessBufferPtr = emGetBufferPointer(accessBuffer);
      emberAfPluginZclCorePrint("access: ");
      printString(accessBufferPtr, accessBufferLength, false);
    }
    emberAfPluginZclCorePrintln("");
  }
  emberAfPluginZclCorePrintln("");
}

// true if token removed, false otherwise
static bool removeRemoteToken(const EmberZclUid_t *targetUid,
                              uint8_t* targetContents,
                              uint16_t targetLength)
{
  for (Buffer buffer = emBufferQueueHead(&remoteTokens);
       buffer != NULL_BUFFER;
       buffer = emBufferQueueNext(&remoteTokens, buffer)) {
    const RemoteTokenData_t *entry
      = (const RemoteTokenData_t *)emGetBufferPointer(buffer);

    if (MEMCOMPARE(&entry->remoteUid,
                   targetUid,
                   sizeof(EmberZclUid_t)) == 0) {
      Buffer accessBuffer = emGetPayloadLink(buffer);
      uint16_t accessBufferLength = emGetBufferLength(accessBuffer);
      uint8_t* accessBufferContents = emGetBufferPointer(accessBuffer);
      if (accessBufferLength == targetLength
          && MEMCOMPARE(accessBufferContents,
                        targetContents,
                        targetLength) == 0) {
        emberAfPluginZclCorePrintln("Access Control: Removed duplicate token");
        emBufferQueueRemove(&remoteTokens, buffer);
        return true; // should only have one duplicate
      }
    }
  }
  return false;
}

// adds given buffer to queue, after removing duplicates
// if queue is full, removes the oldest item
static void addRemoteTokenBuffer(Buffer tokenBuffer)
{
  // remove duplicates
  const RemoteTokenData_t *entry
    = (const RemoteTokenData_t *)emGetBufferPointer(tokenBuffer);
  Buffer accessBuffer = emGetPayloadLink(tokenBuffer);
  uint16_t accessBufferLength = emGetBufferLength(accessBuffer);
  uint8_t* accessBufferContents = emGetBufferPointer(accessBuffer);
  removeRemoteToken(&entry->remoteUid,
                    accessBufferContents,
                    accessBufferLength);

  // add to queue
  emBufferQueueAddToHead(&remoteTokens, tokenBuffer);

  // remove oldest item, if queue too big
  if (emBufferQueueLength(&remoteTokens) > EM_ZCL_MAX_REMOTE_ACCESS_TOKENS) {
    emBufferQueueRemove(&remoteTokens, remoteTokens);
  }

  emZclPrintAccessTokens();
}

// returns true if protected clusterSpec with given accessType, false otherwise
bool emZclIsProtectedResource(const EmberZclClusterSpec_t *clusterSpec,
                              uint8_t accessType)
{
  if (emZclUseAccessControl == ZCL_ACCESS_CONTROL_OFF) {
    return false;
  }

  emberAfPluginZclCorePrintln("Access Control: test access type: %c (r: %d, m: %d, c: %d)", accessType, clusterSpec->role, clusterSpec->manufacturerCode, clusterSpec->id);
  // Check if this is an unprotected zcl access (direction is not checked).
  // (Spec v047: 6.5 - Cluster Security Requirements)
  switch (accessType) {
    //TODO- should be able to access the EMBER_ZCL_CLUSTER_IDs enums from here!
    case 'a': // attribute
      if ((clusterSpec->id == 0x0000)         // EMBER_ZCL_CLUSTER_BASIC
          || (clusterSpec->id == 0x0101)      // EMBER_ZCL_CLUSTER_DOOR_LOCK
          || (clusterSpec->id == 0x0500)      // EMBER_ZCL_CLUSTER_IAS_ZONE
          || (clusterSpec->id == 0x0501)      // EMBER_ZCL_CLUSTER_IAS_ACE
          || (clusterSpec->id == 0x0502)      // EMBER_ZCL_CLUSTER_IAS_WD
          || (clusterSpec->id == 0x2000)      // EMBER_ZCL_CLUSTER_OTA_BOOTLOAD
          ) {
        return true;
      }
      break;
    case 'c': // command
    case 'r': // report
    case 'b': // binding
      if ((clusterSpec->id == 0x0000)         // EMBER_ZCL_CLUSTER_BASIC
          || (clusterSpec->id == 0x0004)      // EMBER_ZCL_CLUSTER_GROUPS
          || (clusterSpec->id == 0x0005)      // EMBER_ZCL_CLUSTER_SCENES
          || (clusterSpec->id == 0x0006)      // EMBER_ZCL_CLUSTER_ON_OFF
          || (clusterSpec->id == 0x0008)      // EMBER_ZCL_CLUSTER_LEVEL_CONTROL
          || (clusterSpec->id == 0x0009)      // EMBER_ZCL_CLUSTER_ALARM
          || (clusterSpec->id == 0x0101)      // EMBER_ZCL_CLUSTER_DOOR_LOCK
          || (clusterSpec->id == 0x0102)      // EMBER_ZCL_CLUSTER_WINDOW_COVERING
          || (clusterSpec->id == 0x0300)      // EMBER_ZCL_CLUSTER_COLOR_CONTROL
          || (clusterSpec->id == 0x0500)      // EMBER_ZCL_CLUSTER_IAS_ZONE
          || (clusterSpec->id == 0x0501)      // EMBER_ZCL_CLUSTER_IAS_ACE
          || (clusterSpec->id == 0x0502)      // EMBER_ZCL_CLUSTER_IAS_WD
          || (clusterSpec->id == 0x2000)      // EMBER_ZCL_CLUSTER_OTA_BOOTLOAD
          ) {
        return true;
      }
      break;
    default:
      emberAfPluginZclCorePrintln("Access Control: Unknown access type: %c", accessType);
      break;
  }
  emberAfPluginZclCorePrintln("Access Control: Unrestricted");
  return false;
}

// Extracts binary access token from CBOR input e.g.
// input="{19:\"2GKEQEBYTqIDaXpjbChzMC5jKRgZoWNraWR4OW5pOi8v
//              L3NoYS0yNTY7a09OeklXQk1tMjlvcnZaRm53VmJIYUVL
//              OVlvRDI1cjJaX29uc01LRXFIa4GDQEBYQB8R525CMKK3
//              Trp6B58AecOZyjOxpW0MOO0bwqgvGsz4WPa7_uFhvC4v
//              6PC8mduQO_GUbPti9ACQDBRepIr_MgPyA\"}"
//
//        hex = A11378D132474B4551454259547149446158706A6243
//        687A4D43356A4B52675A6F574E72615752344F5735704F6938
//        764C334E6F595330794E5459376130394F656B6C58516B3174
//        4D6A6C76636E5A61526D3533566D4A495955564C4F566C7652
//        444931636A4A61583239756330314C52584649613447445145
//        425951423852353235434D4B4B33547270364235384165634F
//        5A796A4F787057304D4F4F30627771677647737A3457506137
//        5F75466876433476365043386D6475514F5F47556250746939
//        414351444252657049725F4D67507941
//
// output (hex) = d862844040584ea23697a636c2873302e63291819a
//                1636b696478396e693a2f2f2f7368612d3235363b6
//                b4f4e7a4957424d6d32396f72765a466e775662486
//                1454b39596f44323572325a5f6f6e734d4b4571486
//                b8183404058401f11e76e4230a2b74eba7a79f079c
//                399ca33b1a56dc38ed1bc2a82f1accf858f6bbfee1
//                61bc2e2fe8f0bc99db903bf1946cfb62f4090c145e
//                a48aff323f2
//
// note: output is still a CBOR map but we have filtered out
//       the access token key and converted it to binary.
//       outputSize should initially be the size of the output
//       array. If successful, it will signify number of bytes
//       used.
bool emZclExtractBinaryAccessToken(CborState *state,
                                   uint8_t *output,
                                   uint16_t *outputSize)
{
  uint16_t key = 0;
  uint8_t value[EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN];

  if (!emCborDecodeMap(state)
      || (key = emCborDecodeKey(state) != EM_ZCL_ACCESS_TOKEN_KEY)
      || !emCborDecodeValue(state,
                            EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                            sizeof(value),
                            value)) {
    emberAfPluginZclCorePrintln("Access Control: failed token key:%d", key);
    return false;
  }

  // emberAfPluginZclCorePrintln("Access Control: token value:%s", value);
  int decodeResult = emDecodeBase64Url(value,
                                       strlen((const char *)value),
                                       output,
                                       *outputSize);

  if (decodeResult <= 0
      || (*outputSize < decodeResult)) {
    return false;
  } else {
    // emberAfPluginZclCoreDebugExec(
    //   emberAfPluginZclCorePrintln("Access Control: binary token { ");
    //   for (uint16_t i = 0; i < decodeResult; i++) {
    //   emberAfPluginZclCorePrint("%x ", output[i]);
    // }
    //   emberAfPluginZclCorePrintln("}");
    //   );
    *outputSize = decodeResult;
    return true;
  }
}

//----------------------------------------------------------------
// Internal functions

void emZclAccessMarkApplicationBuffersHandler(void)
{
  emMarkBuffer(&localTokens);
  emMarkBuffer(&remoteTokens);
}

// Token resource: /zcl/t
void emZclAccessTokenHandler(EmZclContext_t *context)
{
  if (emIsMulticastAddress(context->info->localAddress.bytes)) {
    // do not respond if request over multicast
    return;
  }

  // access token should only be set over a DTLS session
  if (context->info->transmitHandler != &emberDtlsTransmitHandler) {
    emZclRespond400BadRequest(context->info);
    return;
  }

  EmberZclUid_t uid;
  if (emberZclDtlsManagerGetUidBySessionId((uint8_t)(unsigned long) context->info->transmitHandlerData,
                                           &uid) != EMBER_SUCCESS) {
    emZclRespond401Unauthorized(context->info);
    return;
  }

  uint16_t binaryTokenSize = EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN;
  uint8_t binaryToken[EM_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN];
  CborState state;
  emCborDecodeStart(&state, context->payload, context->payloadLength);
  if (!emZclExtractBinaryAccessToken(&state,
                                     binaryToken,
                                     &binaryTokenSize)) {
    emZclRespond400BadRequest(context->info);
    return;
  }

  if (emZclAddIncomingToken(NULL,
                            binaryToken,
                            binaryTokenSize)) {
    emZclRespond201Created(context->info, NULL);
  } else {
    emZclRespond401Unauthorized(context->info);
  }
}

// check to see if the remote device with given UID has access to the requested
// resource with given clusterSpec and accessType
EmberZclStatus_t emZclAllowRemoteAccess(const uint8_t sessionId,
                                        const EmberZclClusterSpec_t *clusterSpec,
                                        EmberZclDeviceId_t endpointDeviceId,
                                        uint8_t accessType)
{
  EmberZclStatus_t status = EMBER_ZCL_STATUS_NOT_AUTHORIZED;  // Returned if UID/kid not found
  if (!emZclIsProtectedResource(clusterSpec, accessType)) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  EmberZclUid_t targetUid = { { 0 } };
  if (emberZclDtlsManagerGetUidBySessionId(sessionId, &targetUid)
      != EMBER_SUCCESS) { // PSK/RPK mode?
    return EMBER_ZCL_STATUS_FAILURE;
  }

  // iterate through remoteTokens
  // if UID is a match, check if it has required cluster access
  Buffer buffer = NULL_BUFFER;
  for (buffer = emBufferQueueHead(&remoteTokens);
       buffer != NULL_BUFFER;
       buffer = emBufferQueueNext(&remoteTokens, buffer)) {
    const RemoteTokenData_t *entry
      = (const RemoteTokenData_t *)emGetBufferPointer(buffer);
    if (MEMCOMPARE(&entry->remoteUid,
                   &targetUid,
                   sizeof(EmberZclUid_t)) == 0) {
      Buffer accessBuffer = emGetPayloadLink(buffer);
      uint16_t accessBufferLength = emGetBufferLength(accessBuffer);
      if (accessBufferLength == 0) {
        return EMBER_ZCL_STATUS_SUCCESS; // allow all
      } else if (checkClusterAccess(emGetBufferPointer(accessBuffer),
                                    accessBufferLength,
                                    clusterSpec,
                                    endpointDeviceId,
                                    accessType)) {
        return EMBER_ZCL_STATUS_SUCCESS;
      } else {
        status = EMBER_ZCL_STATUS_FORBIDDEN;
        continue; // could have another token which provides access
      }
    }
  }

  emberAfPluginZclCoreDebugExec(                                                                      \
    uint8_t uidString[EMBER_ZCL_UID_STRING_SIZE];                                                     \
    emZclUidToBase64Url(&targetUid, EMBER_ZCL_UID_BITS, uidString);                                   \
    emberAfPluginZclCorePrintln("Access Control: Access denied for cluster:0x%2x access:%c | uid:%s", \
                                clusterSpec->id,                                                      \
                                accessType,                                                           \
                                uidString);                                                           \
    );

  return status;
}

//----------------------------------------------------------------
// Managing tokens

// 1: initial 's' or 'c',
// 5: optional manufacturer ID + '_'
// 4: cluster ID
// 2: '.' + char showing access type
#define MAX_CLUSTER_STRING_LENGTH (1 + 5 + 4 + 2)

int16_t checkUidAccess(const uint8_t *accessString,
                       uint16_t accessStringLength,
                       EmberZclUid_t *targetUid,
                       uint8_t *accessBuf,
                       uint16_t accessBufLength)
{
  // Checks that 'targetUid' is allowed access
  //
  // For outgoing tokens we don't know the type of remote devices so the
  // caller can pass in EMBER_ZCL_DEVICE_ID_NULL and we just have to
  // assume that the remote node is the right type for whatever token we
  // have.
  //
  // Any cluster/device specs found are copied to 'accessBuf' for later use.
  //
  // Returns the total length of the specs, or -1 if access is
  // not allowed.
  //
  // The "audience" value has the permissions encoded as a string:
  //  "zcl([<RS UID> (..)] [d.<device id> (..)] [<cl>.<access group> (..)])"
  // It isn't clear if the order is mandatory.  From the examples the
  // different bits are separated by spaces.  The choices are:
  //   <RS UID> = "ni///sha-256;..."   - the server UID
  //   d.X where X is a device type ID - any server of this type
  //   [cs]X.[acrb]                    - the relevent clusters

  bool uidMatch = false;
  bool uidSeen = false;
  bool deviceOrClusterIdSeen = false;
  uint8_t *writeFinger = accessBuf;
  const uint8_t *writeEnd = accessBuf + accessBufLength;
  uint16_t clusterLength = 0;

  const uint8_t *end = accessString + accessStringLength;
  while (accessString < end) {
    if (*accessString == ' ') {
      accessString += 1;
    } else {
      const uint8_t *start = accessString;
      while ((accessString < end)
             && (*accessString != ' ')) {
        accessString += 1;
      }
      uint16_t length = accessString - start;
      switch (*start) {
        case 'n': {
          uidSeen = true;
          if (targetUid) {
            EmberZclUid_t uid;
            if (emZclNiUriToUid(start, length, &uid)
                && memcmp(&uid, targetUid, sizeof(EmberZclUid_t)) == 0) {
              uidMatch = true;
            }
          } else {
            uidMatch = true; // targetUid not supplied so always match.
          }
          break;
        }
        case 'd':
        case 's':
        case 'c':
          deviceOrClusterIdSeen = true;
          if (writeFinger) {
            if (writeFinger + length < writeEnd) {
              memcpy(writeFinger, start, length);
              // Always adding a space at the end makes comparisons
              // easier because you don't have to worry about one
              // cluster+space being a prefix of another.
              writeFinger[length] = ' ';
            }
            writeFinger += length + 1;
          }
          clusterLength += length + 1;
          break;

          // for future compatibility we ignore anything we don't understand
      }
    }
  }

  // The spec doesn't actually say so, but the rule seems to be that the
  // absence of a spec means that all values are allowed.  There must be
  // at least one spec of some kind.
  if ((uidSeen || deviceOrClusterIdSeen)
      && !(uidSeen && !uidMatch)) {
    return clusterLength;
  }

  return -1;
}

bool checkClusterAccess(const uint8_t *accessString,
                        uint16_t accessStringLength,
                        const EmberZclClusterSpec_t *clusterSpec,
                        EmberZclDeviceId_t endpointDeviceId,
                        uint8_t accessType)
{
  // Returns true if the accessString allows 'clusterSpec' with 'accessType'.
  // Access type is a character as defined in the spec.
  //  'a' attributes
  //  'c' commands
  //  'r' reports
  //  'b' bindings
  // If accessString contains d.<id>, then one of them needs to be
  // equal to endpointDeviceId, unless it is EMBER_ZCL_DEVICE_ID_NULL

  bool deviceIdSeen = false;
  bool deviceIdMatch = false;
  bool clusterIdSeen = false;
  bool clusterIdMatch = false;

  uint8_t cluster[MAX_CLUSTER_STRING_LENGTH];
  size_t clusterLen = emZclClusterToString(clusterSpec, cluster);
  cluster[clusterLen++] = '.';
  cluster[clusterLen++] = accessType;

  const uint8_t *end = accessString + accessStringLength;
  while (accessString < end) {
    if (*accessString == ' ') {
      accessString += 1;
    } else {
      const uint8_t *start = accessString;
      while (accessString < end
             && *accessString != ' ') {
        accessString += 1;
      }
      uint16_t length = accessString - start;

      switch (*start) {
        case 'd': {
          deviceIdSeen = true;
          uintmax_t deviceId;
          if (!deviceIdMatch // already found a match
              && ((endpointDeviceId == EMBER_ZCL_DEVICE_ID_NULL) // match on any deviceId.
                  || ((length >= 3)
                      && (start[1] == '.')
                      && emHexStringToInt(start + 2, length - 2, &deviceId)
                      && (deviceId == endpointDeviceId)))) {
            emberAfPluginZclCorePrintln("Access Control: checkClusterAccess() deviceId found: 0x%2x",
                                        endpointDeviceId);
            deviceIdMatch = true;
          }
          break;
        }
        case 's':
        case 'c':
          clusterIdSeen = true;
          if (!clusterIdMatch // already found a match
              && (length == clusterLen
                  && memcmp(cluster, start, clusterLen) == 0)) {
            emberAfPluginZclCorePrintln("Access Control: checkClusterAccess() clusterId found");
            clusterIdMatch = true;
          }
          break;

          // for future compatibility we ignore anything we don't understand
      }

      if (clusterIdMatch && deviceIdMatch) {
        return true;
      }
    }
  }

  if (clusterIdSeen && deviceIdSeen) {
    return clusterIdMatch && deviceIdMatch;
  } else if (clusterIdSeen) {
    return clusterIdMatch;
  } else if (deviceIdSeen) {
    return deviceIdMatch;
  } else {
    return false; // nothing seen, something's wrong
  }
}

// TODO: do we need this now that we have emZclAllowRemoteAccess?
int8_t checkResourceAccess(EmberZclUid_t *targetUid,
                           const RemoteTokenData_t *tokenData,
                           const EmberZclClusterSpec_t *clusterSpec,
                           uint8_t accessType,
                           uint16_t mask)
{
  // Checks to see if requested resource access is protected.
  //
  // 'mask' is a bitmask showing which tokens are of interest.
  //
  // Returns the index of the first token that matches, or -1 (FF) if none do.
  // Returns -2 for an unprotected resource (no token is required).

  // Check if this is an unprotected zcl access (direction is not checked).
  // (Spec v040: 6.5 - Cluster Security Requirements)
  if (!emZclIsProtectedResource(clusterSpec, accessType)) {
    return -2;
  }

  // Resource is protected so we need to sort out token access...

  if ((tokenData == NULL) || (targetUid == NULL)) {
    return -1;
  }

  if (!memcmp(targetUid, &emZclUid, sizeof(EmberZclUid_t))) {
    // Incoming resource request (we are RS).
    return checkLocalAccess(targetUid,
                            tokenData,
                            clusterSpec,
                            accessType,
                            mask);
  } else {
    // Outgoing resource request (we are resource client).
    return checkRemoteAccess(targetUid,
                             tokenData,
                             clusterSpec,
                             accessType,
                             mask);
  }
}

static int8_t checkLocalAccess(EmberZclUid_t *targetUid,
                               const RemoteTokenData_t *tokenData,
                               const EmberZclClusterSpec_t *clusterSpec,
                               uint8_t accessType,
                               uint16_t mask)
{
  // Handles protected resource request from remote client
  // (i.e. we are the responding Resource Server).
  // Checks to see if any existing local tokens give the desired access.
  //
  // 'mask' is a bitmask showing which tokens are of interest.
  //
  // Returns the index of the first token that matches, or -1 (FF) if none do.

  const TokenData_t *token =
    (const TokenData_t *)emGetBufferPointer(localTokens);
  const TokenData_t *end =
    (const TokenData_t *)emGetBufferPointer(localTokens)
    + emGetBufferLength(localTokens);
  uint8_t index;

  for (index = 0; token < end; token++, index++) {
    if ((mask & BIT(index))
        && (!memcmp(&token->confUid, &tokenData->remoteUid, sizeof(EmberZclUid_t))) // Check token is owned by remote device.
        // no need to check deviceAccess here (device access is checked before token is saved).
        && checkClusterAccess(token->bytes + token->audienceOffset,
                              token->audienceLength,
                              clusterSpec,
                              EMBER_ZCL_DEVICE_ID_NULL,
                              accessType)) {
      return index;
    }
  }

  return -1;
}

static int8_t checkRemoteAccess(EmberZclUid_t *targetUid,
                                const RemoteTokenData_t *tokenData,
                                const EmberZclClusterSpec_t *clusterSpec,
                                uint8_t accessType,
                                uint16_t mask)
{
  // Handles protected resource request to a remote Resourse Server
  // (i.e. we are the requesting resource client).
  // Checks to see if any existing local tokens give the desired access.
  //
  // 'mask' is a bitmask showing which tokens are of interest.
  // TODO- check if token has been sent already.
  //
  // TODO- For an outgoing message we first check against the tokens that
  // have already been sent, and then if none match we check against those that
  // have not been sent.  If an unsent token matches we send it and then send
  // the message.  If no token matches there is no point in sending the
  // message.
  //
  // TODO-  remote RS may already have a token that allow us access- should we
  // send the coap request msg anayway?  If we get a Security Error response
  // we know we have to send the msg again but this time append/POST the token
  // (if we have one that allows us access.
  //
  // Returns the index of the first token that matches, or -1 (FF) if none do.

  const TokenData_t *token =
    (const TokenData_t *)emGetBufferPointer(localTokens);
  const TokenData_t *end =
    (const TokenData_t *)emGetBufferPointer(localTokens)
    + emGetBufferLength(localTokens);
  uint8_t index;

  for (index = 0; token < end; token++, index++) {
    if ((mask & BIT(index))
        && (!memcmp(&token->confUid, &emZclUid, sizeof(EmberZclUid_t))) // Check we own the token.
        // no need to check deviceAccess here (device access is checked before token is saved).
        && checkClusterAccess(token->bytes + token->audienceOffset,
                              token->audienceLength,
                              clusterSpec,
                              EMBER_ZCL_DEVICE_ID_NULL,
                              accessType)) {
      return index;
    }
  }

  return -1;
}

// Verify acess token signature using ECDSA-with-SHA256, return TRUE if valid or if Access Control is
// diabled.
bool verifyAccessTokenSignature(const uint8_t *signature, const uint8_t *token, uint32_t tokenLength)
{
  uint8_t hash[SHA256_HASH_LENGTH];
  CborState state;
  uint8_t buffer[1024];

  // For now only verify signature if we are explicitly configured to be in a test mode
  if (emZclUseAccessControl != ZCL_ACCESS_CONTROL_ON_TEST) {
    return true;
  }

  // Commissioning Tool's Public key. This should be provided out-of-band. For now use the CT key
  // provided in the ZCLIP Test Specification 16-07023-072, Test Group 15.
  // TODO: Key must be added/activated by a CLI command or API
  // Added 0x04 at the beginning of the key to indicate the right format for EccPublicKey
  EccPublicKey eccPublicKey = { 65, { 0x04, 0x21, 0x98, 0xa4, 0x5a, 0xc8, 0x5b, 0x48, 0xfd, 0xbd, 0x07, 0x58, 0xf6, 0x94, 0x9e, 0xbf, 0x69, 0x19, 0xc3, 0x8a, 0x6e, 0x12, 0x6b, 0x81, 0x07, 0x94, 0x9f, 0xbe, 0xcd, 0x81, 0xaa, 0xbd, 0xc0, 0xa0, 0x0c, 0x50, 0x9f, 0xe5, 0xc2, 0x4f, 0xe5, 0x47, 0x07, 0xb6, 0x89, 0x97, 0xa3, 0x94, 0xa6, 0x2e, 0x01, 0x22, 0x00, 0x0d, 0x64, 0xca, 0x74, 0x7a, 0xbc, 0xde, 0x73, 0xea, 0xdf, 0xa0, 0x9e } };

  // Access token signing is defined in 16-07008-071, Section 6.6.2.2 and RFCs 8152 and 8392.
  // The signature is constructed as follows:
  //  Build the access token, convert it to a byte string
  //  Form the SIG_Structure CBOR array ["Signature", <empty bstr>, <empty bstr>, <empty bstr>, token bstr]
  //  Calculate its hash using SHA-256
  //  Sign the hash using the private key
  emCborEncodeArrayStart(&state, buffer, sizeof(buffer), 5);
  emCborEncodeValue(&state, EMBER_ZCLIP_TYPE_STRING, sizeof("Signature"), (const uint8_t *)"Signature");
  emCborEncodeValue(&state, EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, 0, NULL);
  emCborEncodeValue(&state, EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, 0, NULL);
  emCborEncodeValue(&state, EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, 0, NULL);
  emCborEncodeValue(&state, EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, tokenLength, token);

  emSha256Hash(buffer, emCborEncodeSize(&state), hash);

  // Verify the signature using the public key
  return emberVerifyEcdsaSignature(hash, &eccPublicKey, signature, 32, signature + 32, 32);
}

bool emDecodeAccessToken(TokenData_t *token)
{
  // Parses token cbor structure to get UID and audience string.
  //
  // A token looks like:
  //  tag for COSE Signed Data Object
  //  array of length four, containing:
  //    empty string - protected attributes
  //    empty string - unprotected attributes
  //    parameter string -
  //       cbor-encoded map:
  //       3 (audience): string of capabilities
  //       25 (confirmation): map
  //       "kid" : UID identity string (i.e. of client)
  //    array of signatures with one entry, containing
  //      array length three, containing
  //        empty string - protected attributes
  //        empty string - unprotected attributes
  //        signature string (ECDSA-with-SHA256, 64 bytes)
  //
  // Returns true if all went well.
  //TODO- Eventually this will need to check the signature.

  CborState state;
  emCborDecodeStart(&state, token->bytes, token->length);
  uint32_t tagType;
  uint8_t *contents;
  uint32_t length;
  uint8_t *payload;
  uint32_t payloadLength;

  if (!(emCborDecodeTag(&state, &tagType)
        && (tagType == CBOR_COSE_SIGN_TAG)
        && emCborDecodeArray(&state)
        // Empty string of protected attributes.
        && emCborDecodeItem(&state, CBOR_BYTES, &contents, &length)
        && (length == 0)
        // Empty string of unprotected attributes.
        && emCborDecodeItem(&state, CBOR_BYTES, &contents, &length)
        && (length == 0)
        // Parameter string.
        && emCborDecodeItem(&state, CBOR_BYTES, &payload, &payloadLength)
        && emCborDecodeArray(&state))) {
    return false;
  }

  // We are now at the array of signatures.  In theory there may be multiple
  // ones, so we pick out the one with no attributes.
  // How do we tell them apart?
  uint8_t *signature = NULL;

  while (!emCborPeekSequenceEnd(&state)) {
    uint32_t protectedLength;
    uint32_t unprotectedLength;
    uint8_t *temp;
    uint32_t tempLength;
    if (!(emCborDecodeArray(&state)
          && emCborDecodeItem(&state, CBOR_BYTES, &contents, &protectedLength)
          && emCborDecodeItem(&state, CBOR_BYTES, &contents, &unprotectedLength)
          && emCborDecodeItem(&state, CBOR_BYTES, &temp, &tempLength))) {
      return false;
    }
    if ((protectedLength == 0)
        && (unprotectedLength == 0)
        && (tempLength == 64)) {
      signature = temp; // for now assume sig is in the last array element.
    }
  }
  if (signature == NULL) {
    return false;       // no appropriate signature found
  }

  // For safety, check signature here, the earliest possible point.
  // Shouldn't even decode the payload until after checking the signature.
  if (verifyAccessTokenSignature(signature, payload, payloadLength) != true) {
    emberAfPluginZclCorePrintln("Access token signature not valid");
    return false;
  } else {
    emberAfPluginZclCorePrintln("Access token signature is valid");
  }

  // For testing we may want to check that there is nothing else.
  // For future-proofing we want to ignore anything extra.
  // for safety we check that there is nothing else.
  // assert(emCborDecodeSequenceEnd(&state));
  // assert(emCborDecodeSequenceEnd(&state));
  // assert(state.finger == state.end
  //        && state.nestDepth == 0);

  emCborDecodeStart(&state, payload, payloadLength); // Restart decode.
  assert(emCborDecodeMap(&state));
  bool haveUid = false;
  bool haveAudience = false;

  while (!emCborPeekSequenceEnd(&state)) {
    uint32_t key;
    if (!emCborDecodeValue(&state,
                           EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                           sizeof(key),
                           (uint8_t *) &key)) {
      return false;
    }
    if (key == 3) { // == "aud"
      uint8_t *audience;
      uint32_t audienceLength;
      if (!(emCborDecodeItem(&state,
                             CBOR_TEXT,
                             &audience,
                             &audienceLength)
            && (audienceLength > 5)
            && (strncmp((const char *)audience, "zcl(", 4) == 0)
            && audience[audienceLength - 1] == ')')) {
        return false;
      }
      haveAudience = true;
      token->audienceOffset = (audience + 4) - token->bytes;
      token->audienceLength = audienceLength - 5;
    } else if (key == 25) { // == "conf"
      uint8_t *keyString;
      uint32_t keyStringLength;
      if (!(emCborDecodeMap(&state))) {
        return false;
      }
      while (!emCborPeekSequenceEnd(&state)) {
        uint8_t *uidUri;
        uint32_t uidUriLength;
        if (!emCborDecodeItem(&state, CBOR_TEXT, &keyString, &keyStringLength)) {
          return false;
        }
        if ((keyStringLength == 3)
            && strncmp("kid", (const char *) keyString, 3) == 0) {
          if (!(emCborDecodeItem(&state, CBOR_TEXT, &uidUri, &uidUriLength)
                && emZclNiUriToUid(uidUri, uidUriLength, &token->confUid))) {
            return false;
          }
          haveUid = true;
        } else {
          emCborDecodeSkipValue(&state); // unknown key, so ignore the value
        }
      }
    } else {
      emCborDecodeSkipValue(&state); // unknown key, so ignore the value
    }
  }
  if (!(haveAudience && haveUid)) {
    return false;
  }

  // For testing we may want to check that there is nothing else.
  // For future-proofing we need to ignore anything extra.
  // assert(state.finger == state.end
  //        && state.nestDepth == 0);
  return true;
}

bool emCacheConfiguredToken(const uint8_t *tokenBytes, uint16_t tokenLength)
{
  // Parse and save the needed information from the token, which is
  // presumably being saved somewhere else.

  TokenData_t token;

  memset(&token, 0, sizeof(token));
  token.bytes = tokenBytes;
  token.length = tokenLength;

  if (!emDecodeAccessToken(&token)) {
    return false;
  }

  uint8_t index;

  for (index = 0; index < 16; index++) {
    if (!((1 << index) & localTokenIndexes)) {
      break;
    }
  }

  if (index == 16) {
    return false;
  }
  token.index = index;

  uint16_t oldBufferLength = emGetBufferLength(localTokens);
  uint16_t newBufferLength = oldBufferLength + sizeof(token);
  Buffer newBuffer = emAllocateBuffer(newBufferLength);
  if (newBuffer == NULL_BUFFER) {
    return false;
  }

  uint8_t *finger = emGetBufferPointer(newBuffer);
  memcpy(finger, emGetBufferPointer(localTokens), oldBufferLength);
  finger += oldBufferLength;
  memcpy(finger, &token, sizeof(token));
  localTokens = newBuffer;
  localTokenIndexes |= 1 << index;
  return true;
}

// UID is the identifier of the device that added this token; could be
// NULL in which case don't compare it to the UID from the token
bool emZclAddIncomingToken(const EmberZclUid_t *remoteUid,
                           const uint8_t *tokenBytes,
                           uint16_t tokenLength)

{
  TokenData_t token;
  memset(&token, 0, sizeof(token));
  token.bytes = tokenBytes;
  token.length = tokenLength;

  // Decode the token
  if (!emDecodeAccessToken(&token)) {
    return false;
  }
  // Verify that the remote node is the owner of the token -- do so only if remoteUid is provided
  if (remoteUid != NULL) {
    if (memcmp(&token.confUid, remoteUid, sizeof(EmberZclUid_t)) != 0) {
      emberAfPluginZclCorePrintln("Access token UID mismatch");
      return false;
    }
  }

  RemoteTokenData_t incomingToken;
  memset(&incomingToken, 0, sizeof(incomingToken));
  memcpy(&incomingToken.remoteUid, &token.confUid, sizeof(EmberZclUid_t));
  Buffer incomingTokenBuffer = emFillBuffer((uint8_t *)&incomingToken, sizeof(incomingToken));
  if (incomingTokenBuffer == NULL_BUFFER) {
    return false;
  }

  // Save the token if it gives permission to access our UID.
  // We need to save cluster and device entries in the 'audience' string.
  // If there are no entries then all clusters and devices are allowed.
  // The entries are eventually stored in the payload link
  // of the buffer holding the RemoteTokenData_t struct.
  int16_t accessLength =
    checkUidAccess(token.bytes + token.audienceOffset,
                   token.audienceLength,
                   &emZclUid,
                   NULL,     // no access buffer on this check.
                   0);

  if (accessLength == -1) {
    // token has an 'audience' which does not allow our device access
    // => token is not saved.
    return false;
  } else if (accessLength == 0) {
    // allow access to everything
    emSetPayloadLink(incomingTokenBuffer, NULL_BUFFER);
    addRemoteTokenBuffer(incomingTokenBuffer);
    return true;
  }

  Buffer accessBuffer = emAllocateBuffer(accessLength);
  if (accessBuffer == NULL_BUFFER) {
    return false;  // unfortunate, but what can we do?
  }
  uint8_t *contents = emGetBufferPointer(accessBuffer);

  // Save token access spec entries to buffer.
  checkUidAccess(token.bytes + token.audienceOffset,
                 token.audienceLength,
                 &emZclUid,
                 contents,
                 accessLength);

  emSetPayloadLink(incomingTokenBuffer, accessBuffer);
  addRemoteTokenBuffer(incomingTokenBuffer);
  return true;
}
