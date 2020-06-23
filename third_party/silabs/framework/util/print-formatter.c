/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 */

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
 */
/***************************************************************************//**
 * @file
 * @brief Utilities for printing in common formats: buffers, EUI64s, keys
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

//------------------------------------------------------------------------------

static void emAfPrintBuffer(uint16_t area,
                            const uint8_t *buffer,
                            uint16_t bufferLen,
                            const char * formatString)
{
  if (emberAfPrintEnabled(area)) {
    uint16_t index = 0;
    for (; index < bufferLen; index++) {
      emberAfPrint(area, formatString, buffer[index]);
    }
  }
}

void emberAfPrintBuffer(uint16_t area,
                        const uint8_t *buffer,
                        uint16_t bufferLen,
                        bool withSpace)
{
  emAfPrintBuffer(area, buffer, bufferLen, (withSpace ? "%x " : "%x"));
}

void emberAfPrintString(uint16_t area, const uint8_t *buffer)
{
  emAfPrintBuffer(area, buffer + 1, emberAfStringLength(buffer), "%c");
}

void emberAfPrintLongString(uint16_t area, const uint8_t *buffer)
{
  emAfPrintBuffer(area, buffer + 2, emberAfLongStringLength(buffer), "%c");
}

void emberAfPrintLittleEndianEui64(const EmberEUI64 eui64)
{
  emberAfPrint(emberAfPrintActiveArea,
               "(%c)%X%X%X%X%X%X%X%X",
               '<',
               eui64[0],
               eui64[1],
               eui64[2],
               eui64[3],
               eui64[4],
               eui64[5],
               eui64[6],
               eui64[7]);
}

void emberAfPrintBigEndianEui64(const EmberEUI64 eui64)
{
  emberAfPrint(emberAfPrintActiveArea,
               "(%c)%X%X%X%X%X%X%X%X",
               '>',
               eui64[7],
               eui64[6],
               eui64[5],
               eui64[4],
               eui64[3],
               eui64[2],
               eui64[1],
               eui64[0]);
}

void emberAfPrintZigbeeKey(const uint8_t *key)
{
  // Zigbee Keys are 16 bytes long
  emberAfPrint8ByteBlocks(2, key, false);
}

void emberAfPrintCert(const uint8_t *cert)
{
  // Certificates are 48 bytes long
  emberAfPrint8ByteBlocks(6, cert, true);
}

void emberAfPrintKey(bool publicKey, const uint8_t *key)
{
  // ECC Public Keys are 22 bytes
  // ECC Private Keys are 21 bytes

  emberAfPrintZigbeeKey(key);
  emberAfPrintBuffer(emberAfPrintActiveArea, key + 16, 5, true);
  emberAfPrintln(emberAfPrintActiveArea, (publicKey ? "%X" : ""), key[21]);
}

void emberAfPrintKey283k1(bool publicKey, const uint8_t *key)
{
  // ECC Public 283k1 Keys are 37 bytes
  // ECC Private 283k1 Keys are 36 bytes
  emberAfPrint8ByteBlocks(4, key, true);
  emberAfPrintBuffer(emberAfPrintActiveArea, key + 32, 4, true);
  emberAfPrintln(emberAfPrintActiveArea, (publicKey ? "%X" : ""), key[36]);
}

void emberAfPrintCert283k1(const uint8_t *cert)
{
  // ECC 283k1 certificates are 74 bytes long
  emberAfPrint8ByteBlocks(9, cert, true);
  emberAfPrintln(emberAfPrintActiveArea, "%X %X", cert[72], cert[73]);
}

void emberAfPrintIeeeLine(const EmberEUI64 ieee)
{
  emberAfPrintBigEndianEui64(ieee);
  emberAfPrintln(emberAfPrintActiveArea, "");
}

void emberAfPrintTextLine(const char * text)
{
  emberAfPrintln(emberAfPrintActiveArea, "%p", text);
}

void emberAfPrint8ByteBlocks(uint8_t numBlocks,
                             const uint8_t *block,
                             bool crBetweenBlocks)
{
  uint8_t i;
  for (i = 0; i < numBlocks; i++) {
    emberAfPrintBuffer(emberAfPrintActiveArea, block + 8 * i, 8, true);
    // By moving the '%p' to a separate function call, we can
    // save CONST space.  The above string is duplicated elsewhere in the
    // code and therefore will be deadstripped.
    emberAfPrint(emberAfPrintActiveArea,
                 " %p",
                 (crBetweenBlocks || ((i + 1) == numBlocks) ? "\r\n" : ""));
  }
}

void emberAfPrintIssuer(const uint8_t* issuer)
{
  // The issuer field in the certificate is in big-endian form.
  emberAfPrint(emberAfPrintActiveArea, "(>) ");
  emberAfPrint8ByteBlocks(1, issuer, true);
}

void emberAfPrintChannelListFromMask(uint32_t channelMask)
{
  if (emberAfPrintEnabled(emberAfPrintActiveArea)) {
    uint8_t i;
    bool firstPrint = true;
    for (i = 0;
         i < EMBER_MAX_CHANNELS_PER_PAGE;
         i++) {
      if ((channelMask & 0x01UL) != 0U) {
        if (!firstPrint) {
          emberAfPrint(emberAfPrintActiveArea, ", ");
        }
        emberAfPrint(emberAfPrintActiveArea, "%d", i);
        firstPrint = false;
      }
      channelMask >>= 1;
    }
  }
}
