/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include <stdio.h>
#include CHIP_AF_API_ZCL_CORE

// Copies "src," including the NUL terminator, to "dst."  The number of
// characters written, not including the NUL, is returned.
static size_t writeString(uint8_t *dst, const char *src)
{
  uint8_t *finger = dst;
  while ((*finger = *src) != '\0') {
    finger++;
    src++;
  }
  return finger - dst;
}

// Any character not in [0-9a-fA-F] returns a value the range 0x00--0x0F.
static uint8_t hexCharToNibble(char c)
{
  return ('0' <= c && c <= '9'
          ? c - '0'
          : ('a' <= c && c <= 'f'
             ? c - ('a' - 10)
             : ('A' <= c && c <= 'F'
                ? c - ('A' - 10)
                : (uint8_t)-1)));
}

// Writes the integer "value" as a hexadecimal string to "result."  "size" is
// the number of bytes in "value."  Leading zeros are omitted from the string
// and lowercase letters are used for nibbles with values from 0xA to 0xF.  The
// string is NUL terminated.  The number of characters written, not including
// the NUL, is returned.  "result" is assumed to be adequately sized.
//   0x0000: returns 1 and sets "result" to "0"
//   0x0001: returns 1 and sets "result" to "1"
//   0xDEAD: returns 4 and sets "result" to "dead"
uint8_t chZclIntToHexString(uint32_t value, size_t size, uint8_t *result)
{
  assert(size <= sizeof(value));
  // Make sure value is not larger than what fits in 'size'.  If
  // 'size' is sizeof(uintmax_t) this is automatically true but can't
  // be tested because the arithmetic overflows.
  assert(size == sizeof(uintmax_t)
         || value <= (((uintmax_t ) 1) << (8 * size)) - 1);

  return sprintf((char *) result, "%x", value);
}

bool emHexStringToInt(const uint8_t *chars, size_t length, uintmax_t *result)
{
  // Converts the hexadecimal string "chars" to the integer "result."
  // "length" is the number of characters to read.
  // Leading zero characters are not allowed.
  // Strings with any characters not in range [0-9,a-f,A-F] are
  // rejected as invalid.

  if ((length == 0)
      || ((length > 1) && (*chars == '0')) // leading zero found.
      || (sizeof(*result) < (length + 1) / 2)) {
    return false;
  }

  uintmax_t tmp = 0;
  for (; length; chars++, length--) {
    uint8_t nibble = hexCharToNibble(*chars);
    if (0x0F < nibble) {
      return false;
    } else {
      tmp = (tmp << 4) + nibble;
    }
  }

  *result = tmp;

  return true;
}

bool chZclHexStringToInt(const uint8_t *chars, size_t length, uintmax_t *result)
{
  // Used in parsing URIs-
  // If a leading zero character is found we return true but with an invalid
  // result value (this enables the the caller to signal "not found" as per
  // test spec).
  // If the string contains any non hex-digit character, the character is
  // dropped (along with any subsequent characters) before conversion.
  // Otherwise behaves as emHexStringToInt().

  if ((length > 1) && (*chars == '0')) {
    // Found a leading zero on input string, return true with invalid value.
    *result = (uintmax_t)-1;
    return true;
  }

  // Drop any trailing characters after first found non hex-digit character.
  size_t actualLength = 0;
  uint8_t *p = (uint8_t *)chars;
  for (; actualLength < length; p++) {
    if (0x0F < hexCharToNibble(*p)) {
      break;
    } else {
      ++actualLength;
    }
  }

  return emHexStringToInt(chars, actualLength, result);
}

size_t chZclClusterToString(const ChipZclClusterSpec_t *clusterSpec,
                            uint8_t *result)
{
  uint8_t *finger = result;
  *finger++ = (clusterSpec->role == CHIP_ZCL_ROLE_CLIENT ? 'c' : 's');
  if (clusterSpec->manufacturerCode != CHIP_ZCL_MANUFACTURER_CODE_NULL) {
    finger += chZclIntToHexString(clusterSpec->manufacturerCode,
                                  sizeof(clusterSpec->manufacturerCode),
                                  finger);
    *finger++ = CHIP_ZCL_URI_PATH_MANUFACTURER_CODE_CLUSTER_ID_SEPARATOR;
  }
  finger += chZclIntToHexString(clusterSpec->id,
                                sizeof(clusterSpec->id),
                                finger);
  // chZclIntToHexString adds a null terminator, so we don't need to add one
  // here.
  return finger - result;
}

bool chZclStringToCluster(const uint8_t *chars,
                          size_t length,
                          ChipZclClusterSpec_t *clusterSpec)
{
  ChipZclRole_t role;
  uintmax_t manufacturerCode;
  uintmax_t clusterId;

  const uint8_t *finger = chars;

  // Cluster ids start with c for client or s for server.  No other values are
  // valid.
  if (*finger == 'c') {
    role = CHIP_ZCL_ROLE_CLIENT;
  } else if (*finger == 's') {
    role = CHIP_ZCL_ROLE_SERVER;
  } else {
    return false;
  }
  finger++;
  length--;

  // Next is either the manufacturer code and cluster id joined together or
  // just the cluster id by itself.  Both are two-byte values.
  const uint8_t *separator
    = memchr(finger,
             CHIP_ZCL_URI_PATH_MANUFACTURER_CODE_CLUSTER_ID_SEPARATOR,
             length);
  if (separator == NULL) {
    manufacturerCode = CHIP_ZCL_MANUFACTURER_CODE_NULL;
  } else {
    uint16_t sublength = separator - finger;
    if (!(sublength <= sizeof(clusterSpec->manufacturerCode) * 2  // bytes to nibbles
          && chZclHexStringToInt(finger, sublength, &manufacturerCode)
          && manufacturerCode != CHIP_ZCL_MANUFACTURER_CODE_NULL)) {
      return false;
    }
    finger = separator + 1;
    length = length - sublength - 1;
  }
  if (!(length <= sizeof(clusterSpec->id) * 2  // bytes to nibbles
        && chZclHexStringToInt(finger, length, &clusterId))) {
    return false;
  }

  clusterSpec->role = role;
  clusterSpec->manufacturerCode = manufacturerCode;
  clusterSpec->id = clusterId;
  return true;
}
// It converts the endpoint or group to equivalent null terminated uri string
// destination  : input that has the endpoint or group value and type
// result       : output collecter - caller ensures the memeory is allocated
// returns      : size of the output
static size_t applicationDestinationToUriPath(const ChipZclApplicationDestination_t *destination,
                                              uint8_t *result)
{
  uint8_t *finger = result;
  if (destination->type == CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT) {
    finger += writeString(finger, "zcl/e/");
    finger += chZclIntToHexString(destination->data.endpointId,
                                  sizeof(destination->data.endpointId),
                                  finger);
  } else {
    finger += writeString(finger, "zcl/g/");
    finger += chZclIntToHexString(destination->data.groupId,
                                  sizeof(destination->data.groupId),
                                  finger);
  }

  // chZclIntToHexString adds a null terminator, so we don't need to add one
  // here.
  return finger - result;
}

static size_t clusterToUriPath(const ChipZclApplicationDestination_t *destination,
                               const ChipZclClusterSpec_t *clusterSpec,
                               uint8_t *result)
{
  uint8_t *finger = result;
  if (destination != NULL) {
    finger += applicationDestinationToUriPath(destination, finger);
    *finger++ = '/';
  }
  finger += chZclClusterToString(clusterSpec, finger);
  // chZclClusterToString adds a null terminator, so we don't need to add one
  // here.
  return finger - result;
}

size_t chZclThingToUriPath(const ChipZclApplicationDestination_t *destination,
                           const ChipZclClusterSpec_t *clusterSpec,
                           char thing,
                           uint8_t *result)
{
  uint8_t *finger = result;
  finger += clusterToUriPath(destination, clusterSpec, finger);
  if ((thing >= 'a')
      && (thing <= 'z')) {
    *finger++ = '/';
    *finger++ = thing;
  }
  *finger = '\0';
  return finger - result;
}

size_t chZclThingIdToUriPath(const ChipZclApplicationDestination_t *destination,
                             const ChipZclClusterSpec_t *clusterSpec,
                             char thing,
                             uintmax_t thingId,
                             size_t size,
                             uint8_t *result)
{
  uint8_t *finger = result;
  finger += chZclThingToUriPath(destination, clusterSpec, thing, finger);
  *finger++ = '/';
  finger += chZclIntToHexString(thingId, size, finger);
  // chZclIntToHexString adds a null terminator, so we don't need to add one
  // here.
  return finger - result;
}

size_t chZclDestinationToUri(const ChipZclDestination_t *destination,
                             uint8_t *result)
{
  uint8_t *finger = result;

  if (destination->network.flags & CHIP_ZCL_USE_COAPS_FLAG) {
    finger += writeString(finger, "coaps://");
  } else {
    finger += writeString(finger, "coap://");
  }

  if (destination->network.flags & CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    *finger++ = '[';
    if (!chipIpv6AddressToString(&destination->network.address,
                                  finger,
                                  CHIP_IPV6_ADDRESS_STRING_SIZE)) {
      return 0;
    }
    finger += strlen((const char *)finger);
    *finger++ = ']';
  } else if (destination->network.flags & CHIP_ZCL_HAVE_UID_FLAG) {
    finger += writeString(finger, "sha-256;");
    finger += chZclUidToBase64Url(&destination->network.uid,
                                  CHIP_ZCL_UID_BITS,
                                  finger);
  } else {
    assert(false);
    return 0;
  }

  if (destination->network.port
      != ((destination->network.flags & CHIP_ZCL_USE_COAPS_FLAG)
          ? CHIP_COAP_SECURE_PORT
          : CHIP_COAP_PORT)) {
    finger += sprintf((char *)finger, ":%u", destination->network.port);
  }

  // applicationDestinationToUriPath adds a null terminator, so we don't need
  // to add one here.
  *finger++ = '/';
  finger += applicationDestinationToUriPath(&destination->application, finger);
  return finger - result;
}

bool chZclUriToBindingEntry(const uint8_t *uri,
                            ChipZclBindingEntry_t *result,
                            bool includeCluster)
{
  const uint8_t *finger = uri;
  if (strncmp((const char *)finger, "coap://", 7) == 0) {
    finger += 7;
    result->destination.network.scheme = CHIP_ZCL_SCHEME_COAP;
    result->destination.network.port = CHIP_COAP_PORT;
  } else if (strncmp((const char *)finger, "coaps://", 8) == 0) {
    finger += 8;
    result->destination.network.scheme = CHIP_ZCL_SCHEME_COAPS;
    result->destination.network.port = CHIP_COAP_SECURE_PORT;
  } else {
    return false;
  }

  if (*finger == '[') {
    finger++;
    const uint8_t *end = (const uint8_t *)strchr((const char *)finger, ']');
    if (end == NULL || CHIP_IPV6_ADDRESS_STRING_SIZE <= end - finger) {
      return false;
    }
    uint8_t address[CHIP_IPV6_ADDRESS_STRING_SIZE] = { 0 };
    MEMCOPY(address, finger, end - finger);
    if (!chipIpv6StringToAddress(address,
                                  &result->destination.network.data.address)) {
      return false;
    }
    finger = end + 1;
    result->destination.network.type = CHIP_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS;
  } else if (strncmp((const char *)finger, "sha-256;", 8) == 0) {
    finger += 8;
    uint16_t uidBits;
    if (!chZclBase64UrlToUid(finger,
                             CHIP_ZCL_UID_BASE64URL_LENGTH,
                             &result->destination.network.data.uid,
                             &uidBits)) {
      return false;
    }
    if (uidBits != CHIP_ZCL_UID_BITS) {
      return false;
    }
    finger += CHIP_ZCL_UID_BASE64URL_LENGTH;
    result->destination.network.type = CHIP_ZCL_NETWORK_DESTINATION_TYPE_UID;
  } else {
    return false;
  }

  if (*finger == ':') {
    finger++;
    result->destination.network.port = 0;
    do {
      uint8_t value = chipHexToInt(*finger);
      if (9 < value) {
        return false;
      }
      uintmax_t port = 10 * result->destination.network.port + value;
      if (UINT16_MAX < port) {
        return false;
      }
      result->destination.network.port = (uint16_t)port;
      finger++;
    } while (*finger != '/');
  }

  if (*finger != '/') {
    return false;
  }
  finger++;

  if (strncmp((const char *)finger, "zcl/e/", 6) == 0
      || strncmp((const char *)finger, "zcl/g/", 6) == 0) {
    finger += 6;
    const uint8_t *end = (const uint8_t *)strchr((const char *)finger, '/');
    if (end == NULL) {
      end = finger + strlen((const char *)finger);
    }
    size_t length = end - finger;
    uintmax_t id;
    if (!chZclHexStringToInt(finger, length, &id)) {
      return false;
    }
    if (finger[-2] == 'e') {
      result->destination.application.data.endpointId = (ChipZclEndpointId_t)id;
      result->destination.application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
    } else {
      result->destination.application.data.groupId = (ChipZclGroupId_t)id;
      result->destination.application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP;
    }
    finger = end;
  } else {
    return false;
  }

  if (!includeCluster) {
    return (*finger == '\0');
  }

  if (*finger != '/') {
    return false;
  }
  finger++;

  const uint8_t *end = (const uint8_t *)strchr((const char *)finger, '/');
  if (end == NULL) {
    end = finger + strlen((const char *)finger);
  }
  size_t length = end - finger;

  return chZclStringToCluster(finger, length, &result->clusterSpec);
  return true;
}

size_t chZclCoapEndpointToUri(const ChipZclCoapEndpoint_t *network,
                              uint8_t *result)
{
  uint8_t *finger = result;

  if (network->flags & CHIP_ZCL_USE_COAPS_FLAG) {
    finger += writeString(finger, "coaps://");
  } else {
    finger += writeString(finger, "coap://");
  }

  if (network->flags & CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    *finger++ = '[';
    if (!chipIpv6AddressToString(&network->address,
                                  finger,
                                  CHIP_IPV6_ADDRESS_STRING_SIZE)) {
      return 0;
    }
    finger += strlen((const char *)finger);
    *finger++ = ']';
  } else if (network->flags & CHIP_ZCL_HAVE_UID_FLAG) {
    finger += writeString(finger, "sha-256;");
    finger += chZclUidToBase64Url(&network->uid,
                                  CHIP_ZCL_UID_BITS,
                                  finger);
  } else {
    assert(false);
    return 0;
  }

  if (network->port
      != ((network->flags & CHIP_ZCL_USE_COAPS_FLAG)
          ? CHIP_COAP_SECURE_PORT
          : CHIP_COAP_PORT)) {
    finger += sprintf((char *)finger, ":%u", network->port);
  }
  return finger - result;
}

size_t chZclUriToCoapEndpoint(const uint8_t *uri, ChipZclCoapEndpoint_t *network)
{
  const uint8_t *finger = uri;
  if (strncmp((const char *)finger, "coap://", 7) == 0) {
    finger += 7;
    network->flags = CHIP_ZCL_NO_FLAGS;
    network->port = CHIP_COAP_PORT;
  } else if (strncmp((const char *)finger, "coaps://", 8) == 0) {
    finger += 8;
    network->flags = CHIP_ZCL_USE_COAPS_FLAG;
    network->port = CHIP_COAP_SECURE_PORT;
  } else {
    return 0;
  }

  if (*finger == '[') {
    finger++;
    const uint8_t *end = (const uint8_t *)strchr((const char *)finger, ']');
    if (end == NULL || CHIP_IPV6_ADDRESS_STRING_SIZE <= end - finger) {
      return 0;
    }
    uint8_t address[CHIP_IPV6_ADDRESS_STRING_SIZE] = { 0 };
    MEMCOPY(address, finger, end - finger);
    if (!chipIpv6StringToAddress(address,
                                  &network->address)) {
      return 0;
    }
    finger = end + 1;
    network->flags |= CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG;
  } else if (strncmp((const char *)finger, "sha-256;", 8) == 0) {
    finger += 8;
    uint16_t uidBits;
    if (!chZclBase64UrlToUid(finger,
                             CHIP_ZCL_UID_BASE64URL_LENGTH,
                             &network->uid,
                             &uidBits)) {
      return 0;
    }
    if (uidBits != CHIP_ZCL_UID_BITS) {
      return 0;
    }
    finger += CHIP_ZCL_UID_BASE64URL_LENGTH;
    network->flags |= CHIP_ZCL_HAVE_UID_FLAG;
  } else {
    return 0;
  }

  if (*finger == ':') {
    finger++;
    network->port = 0;
    do {
      uint8_t value = chipHexToInt(*finger);
      if (9 < value) {
        return 0;
      }
      uintmax_t port = 10 * network->port + value;
      if (UINT16_MAX < port) {
        return 0;
      }
      network->port = (uint16_t)port;
      finger++;
    } while ((*finger != '/') && (*finger != 0));
  }
  return finger - uri;
}

//----------------------------------------------------------------
// These two functions are used for debugging only.  ZCLIP does not
// use hexidecimal for sending UIDs over the air.

size_t chZclUidToString(const ChipZclUid_t *uid,
                        uint16_t uidBits,
                        uint8_t *result)
{
  assert(uidBits <= CHIP_ZCL_UID_BITS);

  // We can only send whole nibbles over the air, so the number of bits
  // requested by the user has to be massaged into a multiple of four.
  uidBits &= 0xFFFC;

  uint8_t *finger = result;
  uint8_t index = 0;
  for (size_t i = 0; i < uidBits; i += 4) { // bytes to bits
    index = i / 8;
    if ( index < CHIP_ZCL_UID_SIZE) {
      uint8_t byte = uid->bytes[index];
      uint8_t nibble = (byte >> (4 - (i % 8))) & 0x0F;
      // TODO: Should we print uppercase hexits in the UID?
      *finger++ = (uint8_t) ("0123456789abcdef"[nibble]);
    }
  }
  *finger = '\0';
  return finger - result;
}

bool chZclStringToUid(const uint8_t *uid,
                      size_t length,
                      ChipZclUid_t *result,
                      uint16_t *resultBits)
{
  if (CHIP_ZCL_UID_STRING_LENGTH < length) {
    return false;
  }

  MEMSET(result, 0, sizeof(ChipZclUid_t));
  *resultBits = 0;

  const uint8_t *finger = uid;
  for (size_t i = 0; i < length; i++) {
    // TODO: Should we accept uppercase hexits in the UID?
    uint8_t nibble = hexCharToNibble(*finger++);
    if (0x0F < nibble) {
      return false;
    }
    result->bytes[*resultBits / 8] += (nibble << ((*resultBits + 4) % 8));
    *resultBits += 4;
  }
  return true;
}

//----------------------------------------------------------------
// Test/convert base64url character to its 6-bit code value (range 0-63).
// Described in RFC4648.
//  Char  : Code
// 'A'-'Z': 0-25
// 'a'-'z': 26-51
// '0'-'9': 52-61
// '-'    : 62          // ordinary base64 uses '+' for 62
// '_'    : 63          // ordinary base64 uses '/' for 63
//

static uint8_t decodeBase64UrlChar(uint8_t c)
{
  if ('A' <= c && c <= 'Z') {
    return c - 'A';
  } else if ('a' <= c && c <= 'z') {
    return c - 'a' + 26;
  } else if ('0' <= c && c <= '9') {
    return c - '0' + 52;
  } else if (c == '-') {
    return 62;
  } else if (c == '_') {
    return 63;
  } else {
    return 0xFF;
  }
}

// If code is NULL, just validates that input string contains only
// base64url characters.
bool chZclConvertBase64UrlToCode(const uint8_t *base64Url,
                                 uint16_t length,
                                 uint8_t *code)
{
  uint8_t i;
  for (i = 0; i < length; ++i) {
    uint8_t val = decodeBase64UrlChar(base64Url[i]);
    if (val == 0xFF) {
      return false;
    }
    if (code != NULL) {
      code[i] = val;
    }
  }
  return true;
}

// Array sizer 64 causes initializer string's terminating NUL to be omitted,
// saving a byte of flash.
static const uint8_t base64UrlChars[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "abcdefghijklmnopqrstuvwxyz"
                                          "0123456789-_";

size_t chZclUidToBase64Url(const ChipZclUid_t *uid,
                           uint16_t uidBits,
                           uint8_t *base64Url)
{
  if (uid == NULL
      || base64Url == NULL
      || uidBits > CHIP_ZCL_UID_BITS
      || uidBits % 8 != 0) {
    return 0; // TODO: Is this the right return value for error?
  }

  uint8_t *finger = base64Url;
  uint16_t uidLen = uidBits / 8;
  uint16_t i;
  const uint8_t *b = uid->bytes;

  // Each six binary bits produce one base64url character.
  // Convert 3 binary bytes at a time (24 bits) -> 4 base64url chars.
  // Handle last 1 or 2 "remainder" bytes if not a multiple of 3.
  // Do NOT append padding characters ('=').
  for (i = 0; i < uidLen; i += 3) {
    // There's (always) a first binary byte
    uint8_t ucode = (b[i] & 0xfc) >> 2;
    *finger++ = base64UrlChars[ucode];
    ucode = (b[i] & 0x03) << 4;
    if (i + 1 < uidLen) {
      // There's a second binary byte
      ucode |= (b[i + 1] & 0xf0) >> 4;
      *finger++ = base64UrlChars[ucode];
      ucode = (b[i + 1] & 0x0f) << 2;
      if (i + 2 < uidLen) {
        // There's a third binary byte
        ucode |= (b[i + 2] & 0xc0) >> 6;
        *finger++ = base64UrlChars[ucode];
        ucode = (b[i + 2] & 0x3f);
        *finger++ = base64UrlChars[ucode];
      } else {
        // There was a 2-byte remainder -> 3 base64url chars
        *finger++ = base64UrlChars[ucode];
      }
    } else {
      // There was a 1-byte remainder -> 2 base64url chars
      *finger++ = base64UrlChars[ucode];
    }
  }
  *finger = '\0'; // terminating NUL
  return finger - base64Url;
}

// Returns the number of bytes of output, or -1 if an invalid character
// is found in the input.
int emDecodeBase64Url(const uint8_t *input,
                      size_t inputLength,
                      uint8_t *output,
                      size_t outputLength)
{
  uint8_t *finger = output;
  uint8_t *fingerLimit = output + outputLength;
  size_t i;
  uint16_t bits = 0;        // accumulates bits until we have a byte's worth
  uint8_t count = 0;        // how many bits we have

  for (i = 0; i < inputLength && finger < fingerLimit; i++) {
    uint8_t val = decodeBase64UrlChar(input[i]);
    if (val == 0xFF) {
      return -1;
    }
    bits = (bits << 6) | val;       // each input value gives us six more bits
    count += 6;
    if (8 <= count) {
      *finger++ = bits >> (count - 8);
      count -= 8;
    }
  }
  return finger - output;
}

bool chZclBase64UrlToUid(const uint8_t *base64Url,
                         size_t length,
                         ChipZclUid_t *result,
                         uint16_t *resultBits)
{
  if (base64Url == NULL
      || result == NULL) {
    return false;
  }

  int byteCount = emDecodeBase64Url(base64Url,
                                    length,
                                    result->bytes,
                                    sizeof(result->bytes));

  if (byteCount < 0) {
    return false;
  } else {
    if (resultBits != NULL) {
      *resultBits = byteCount * 8;
    }
    return true;
  }
}

#define niPrefix "ni:///sha-256;"
#define niPrefixLength (sizeof(niPrefix) - 1)   // don't count the nul

bool chZclNiUriToUid(const uint8_t *uri, uint16_t uriLength, ChipZclUid_t *uid)
{
  return (niPrefixLength + CHIP_ZCL_UID_BASE64URL_LENGTH <= uriLength
          && strncmp((const char *)uri, niPrefix, niPrefixLength) == 0
          && 0 < emDecodeBase64Url(uri + niPrefixLength,
                                   uriLength - niPrefixLength,
                                   uid->bytes,
                                   sizeof(uid->bytes)));
}
