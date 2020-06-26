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
 * @brief SOC-specific code related to the reception and processing of interpan
 * messages.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "interpan.h"
#include "app/framework/util/af-main.h"

static const EmberMacFilterMatchData filters[] = {
  EMBER_AF_PLUGIN_INTERPAN_FILTER_LIST
    (EMBER_MAC_FILTER_MATCH_END), // terminator
};

//------------------------------------------------------------------------------

void emberMacFilterMatchMessageHandler(const EmberMacFilterMatchStruct *macFilterMatchStruct)
{
  uint8_t data[EMBER_AF_MAXIMUM_INTERPAN_LENGTH];
  uint8_t length;

  length = emAfCopyMessageIntoRamBuffer(macFilterMatchStruct->message,
                                        data,
                                        EMBER_AF_MAXIMUM_INTERPAN_LENGTH);
  if (length == 0) {
    return;
  }

  emAfPluginInterpanProcessMessage(length,
                                   data);
}

EmberStatus emAfPluginInterpanSendRawMessage(uint8_t length, uint8_t* message)
{
  EmberStatus status;
  EmberMessageBuffer buffer = emberFillLinkedBuffers(message, length);
  if (buffer == EMBER_NULL_MESSAGE_BUFFER) {
    return EMBER_NO_BUFFERS;
  }

  status = emberSendRawMessage(buffer);
  emberReleaseMessageBuffer(buffer);
  return status;
}

void emberAfPluginInterpanInitCallback(void)
{
  interpanPluginInit();

  interpanPluginSetMacMatchFilterEnable(true);
}

void interpanPluginSetMacMatchFilterEnable(bool enable)
{
  const EmberMacFilterMatchData* matchData = enable ? filters : NULL;

  EmberStatus status = emberSetMacFilterMatchList(matchData);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("ERR: failed %s inter-PAN MAC filter (0x%x)",
                      enable ? "enabling" : "disabling",
                      status);
  }
}

// Because the stack only handles message buffers we must convert
// the message into buffers before passing it to the stack.  Then
// we must copy the message back into the flat array afterwards.

// NOTE:  It is expected that when encrypting, the message buffer
// pointed to by *apsFrame is big enough to hold additional
// space for the Auxiliary security header and the MIC.

EmberStatus emAfInterpanApsCryptMessage(bool encrypt,
                                        uint8_t* apsFrame,
                                        uint8_t* messageLength,
                                        uint8_t apsHeaderLength,
                                        EmberEUI64 remoteEui64)
{
  EmberStatus status = EMBER_LIBRARY_NOT_PRESENT;

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES)

  EmberMessageBuffer buffer = emberFillLinkedBuffers(apsFrame,
                                                     *messageLength);
  if (buffer == EMBER_NULL_MESSAGE_BUFFER) {
    return EMBER_NO_BUFFERS;
  }

  status = emberApsCryptMessage(encrypt,
                                buffer,
                                apsHeaderLength,
                                remoteEui64);
  if (status == EMBER_SUCCESS) {
    // It is expected that when encrypting, the message is big enough to hold
    // the additional data (AUX header and MIC)
    // Decrypting will shrink the message, removing the AUX header and MIC.
    emberCopyFromLinkedBuffers(buffer,
                               0,
                               apsFrame,
                               emberMessageBufferLength(buffer));
    *messageLength = emberMessageBufferLength(buffer);
  }
  emberReleaseMessageBuffer(buffer);

#endif

  return status;
}
