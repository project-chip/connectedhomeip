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

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

// Prints an ipv6 address associated with a domain name to the console
static void dnsResponseCallback(EmberDnsLookupStatus status,
                                const uint8_t *domainName,
                                uint8_t domainNameLength,
                                const EmberDnsResponse *response,
                                void *applicationData,
                                uint16_t applicationDataLength);

// The address of the DNS server
static EmberIpv6Address dnsServerAddress = { { 0 } };

// Refer to RFC1034
#define DNS_NAME_MAX_LENGTH 253

// define a placeholder byte containing a leading character to contain the first
// encoded domain length.  Use this character to filter out the leading byte later.
// ASCII NULL was selected as it cannot appear in a valid domain name, and will
// be obvious if unreplaced.
#define LEADING_BYTE 0

// returns EMBER_SUCCESS on success, or !EMBER_SUCCESS on failure.
// this function destructively overwrites the input dnsAddressHumanReadable
static uint8_t convertDomainNameToLengthEncodedFormat(uint8_t * dnsAddressHumanReadable)
{
  // A length-encoded name needs characters describing the length of each
  // subdomain of a domain name. This requires an additional leading byte to
  // hold the leading length.
  // For example  silabs.com -> '\6''s''i''l''a''b''s''\3''c''o''m'
  // skip the LEADING_BYTE
  uint8_t *endPointer = (dnsAddressHumanReadable + 1);
  uint8_t lengthToEncode = 0;

  // be clear to the user of this function what is going on...
  if (dnsAddressHumanReadable[0] != LEADING_BYTE) {
    emberAfAppPrintln("%p: %p", "ERR", "convertDomainNameToLengthEncodedFormat requires a LEADING_BYTE");
    return !EMBER_SUCCESS;
  }
  // Walk through the input string, placing the encoded length at the head of each field,
  // overwriting the '.' and LEADING_BYTE contained in the input
  // example:
  // X 's' 'i' 'l' 'a' 'b' 's' '.' 'c' 'o' 'm'
  // |                          |
  // 6 's' 'i' 'l' 'a' 'b' 's'  3  'c' 'o' 'm'
  while (*endPointer != '\0' ) {
    //If the endPointer hits the boundary of the next field...
    if (*endPointer == '.') {
      // advance to the previous field delimiter and overwrite it
      // with lengthToEncode
      *(endPointer - (lengthToEncode + 1)) = lengthToEncode;

      // reset the length count and consume the '.'
      lengthToEncode = 0;
      endPointer++;
    } else {
      // for all non-'.' bytes, advance the length count
      lengthToEncode++;
      endPointer++;
    }
  }
  // The last field is not encoded by the loop above because it is not '.' terminated, so
  // encode the last field
  *(endPointer - (lengthToEncode + 1)) = lengthToEncode;
  return EMBER_SUCCESS;
}

// dns query <domain name:1-253>
void emberDnsCliQueryCommand(void)
{
  // a length encoded domain name will be EMBER_MAX_DNS_NAME_LENGTH+2 to account for the leading
  // and trailing bytes surrounding the domain name
  uint8_t dnsAddress[EMBER_MAX_DNS_NAME_LENGTH + 2] = { 0 };
  uint8_t dnsLength = 0;
  uint8_t *dnsInput = emberStringCommandArgument(0, &dnsLength);
  EmberStatus returnCode;

  if (dnsLength == 0) {
    emberAfAppPrintln("%p: %p", "ERR", "no domain name specified");
    return;
  } else if (dnsLength > EMBER_MAX_DNS_NAME_LENGTH) {
    emberAfAppPrintln("%p: %p", "ERR", "domain name too long");
    return;
  }

  // insert an invalid, nonprintable leading byte at the beginning of the domain name to accommodate length-encoding
  dnsAddress[0] = LEADING_BYTE;
  MEMCOPY((dnsAddress + 1), dnsInput, dnsLength);

  if (convertDomainNameToLengthEncodedFormat(dnsAddress) == EMBER_SUCCESS) {
    if (emberIsIpv6UnspecifiedAddress(&dnsServerAddress)) {
      // If no server is set, a NULL value will perform a lookup in all available domains.
      returnCode = emberDnsLookup(dnsAddress, dnsLength + 2, NULL, dnsResponseCallback, NULL, 0);
    } else {
      // Perform a DNS query targeted at dnsServerAddress's domain.
      returnCode = emberDnsLookup(dnsAddress, dnsLength + 2, (const uint8_t*)dnsServerAddress.bytes, dnsResponseCallback, NULL, 0);
    }

    if (returnCode == EMBER_SUCCESS) {
      emberAfAppPrintln("Performing lookup on: \"%p\"", dnsInput);
    } else {
      emberAfAppPrintln("emberDnsLookup failed for \"%p\": (%d)", dnsInput, returnCode);
      emberAfAppPrint("Length Encoded buffer: [");
      emberAfAppPrintBuffer(dnsAddress, dnsLength + 2, 1);
      emberAfAppPrintln("]");
    }
  }
}

// dns server <address:0-40>
void emberDnsCliSetServerIp(void)
{
  if (!emberGetIpv6AddressArgument(0, &dnsServerAddress)) {
    emberAfAppPrintln("%p: %p", "ERR", "invalid DNS server address");
    return;
  }

  if (emberIsIpv6UnspecifiedAddress(&dnsServerAddress)) {
    emberAfCorePrintln("DNS server unconfigured.");
    MEMSET(&dnsServerAddress, 0, sizeof(dnsServerAddress));
  } else {
    emberAfCorePrint("DNS server set to: ");
    emberAfCoreDebugExec(emberAfPrintIpv6Address(&dnsServerAddress));
    emberAfCorePrintln("");
  }
}

static void dnsResponseCallback(EmberDnsLookupStatus status,
                                const uint8_t *domainName,
                                uint8_t domainNameLength,
                                const EmberDnsResponse *response,
                                void *applicationData,
                                uint16_t applicationDataLength)
{
  EmberIpv6Address res = { { 0 } };
  switch (status) {
    case EMBER_DNS_LOOKUP_SUCCESS:
      MEMCOPY(&res, &response->ipAddress, sizeof(res));
      emberAfCorePrint("%s is located at: ", domainName);
      emberAfCoreDebugExec(emberAfPrintIpv6Address(&res));
      emberAfCorePrintln("");
      break;

    case EMBER_DNS_LOOKUP_NO_BORDER_ROUTER:
      emberAfCorePrintln("Failed to lookup %s, NO_BORDER_ROUTER", domainName);
      break;

    case EMBER_DNS_LOOKUP_NO_DNS_SERVER:
    case EMBER_DNS_LOOKUP_NO_DNS_RESPONSE:
    case EMBER_DNS_LOOKUP_NO_DNS_RESPONSE_ERROR:
      emberAfCorePrintln("Failed to lookup %s, DNS server error (%d)", domainName, status);
      break;

    case EMBER_DNS_LOOKUP_NO_ENTRY_FOR_NAME:
      emberAfCorePrintln("Failed to lookup %s, NO_ENTRY_FOR_NAME", domainName, status);
      break;

    case EMBER_DNS_LOOKUP_NO_BUFFERS:
      emberAfCorePrintln("Failed to lookup %s, NO_BUFFERS", domainName);
      break;

    default:
      emberAfCorePrintln("Failed to lookup %s, unspecified error (%d)", domainName, status);
      break;
  }
}
