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
 * @brief System on chip specific code for fragmentation.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "fragmentation.h"

//-----------------------------------------------------------------------------
// Globals

//-----------------------------------------------------------------------------
// Functions

void emAfPluginFragmentationPlatformInitCallback(void)
{
  emberFragmentWindowSize = EMBER_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE;
}

EmberStatus emAfPluginFragmentationSend(txFragmentedPacket* txPacket,
                                        uint8_t fragmentNumber,
                                        uint16_t fragmentLen,
                                        uint16_t offset)
{
  EmberStatus status;
  EmberMessageBuffer message;
  message = emberFillLinkedBuffers(txPacket->bufferPtr + offset,
                                   fragmentLen);
  if (message == EMBER_NULL_MESSAGE_BUFFER) {
    return EMBER_NO_BUFFERS;
  }
  status = emberSendUnicast(txPacket->messageType,
                            txPacket->indexOrDestination,
                            &txPacket->apsFrame,
                            message);

  emberReleaseMessageBuffer(message);
  return status;
}

void emAfPluginFragmentationHandleSourceRoute(txFragmentedPacket* txPacket,
                                              uint16_t indexOrDestination)
{
  // Nothing to do on SOC.  This will be handled later by the NWK code
  // calling into the application to ask for a source route.
}

void emAfPluginFragmentationSendReply(EmberNodeId sender,
                                      EmberApsFrame* apsFrame,
                                      rxFragmentedPacket* rxPacket)
{
  emberSetReplyFragmentData(HIGH_LOW_TO_INT(rxPacket->fragmentMask,
                                            rxPacket->fragmentBase));
  emberSendReply(apsFrame->clusterId, EMBER_NULL_MESSAGE_BUFFER);
}
