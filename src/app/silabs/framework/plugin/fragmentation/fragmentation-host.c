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
 * @brief Host specific code for fragmentation.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "fragmentation.h"

//-----------------------------------------------------------------------------
// Globals

uint16_t emberMacIndirectTimeout = 0;
uint16_t emberApsAckTimeoutMs    = 0;
uint8_t  emberFragmentWindowSize = EMBER_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE;

//-----------------------------------------------------------------------------
// Functions

void emberAfPluginFragmentationNcpInitCallback(void)
{
  ezspGetConfigurationValue(EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT,
                            &emberMacIndirectTimeout);
  ezspGetConfigurationValue(EZSP_CONFIG_APS_ACK_TIMEOUT, &emberApsAckTimeoutMs);
  emberAfSetEzspConfigValue(EZSP_CONFIG_FRAGMENT_WINDOW_SIZE,
                            emberFragmentWindowSize,
                            "Fragmentation RX window size");
}

void emAfPluginFragmentationPlatformInitCallback(void)
{
}

EmberStatus emAfPluginFragmentationSend(txFragmentedPacket* txPacket,
                                        uint8_t fragmentNumber,
                                        uint16_t fragmentLen,
                                        uint16_t offset)
{
  EmberStatus status;

  status = ezspSendUnicast(txPacket->messageType,
                           txPacket->indexOrDestination,
                           &(txPacket->apsFrame),
                           fragmentNumber,
                           fragmentLen,
                           txPacket->bufferPtr + offset,
                           &(txPacket->apsFrame.sequence));
  return status;
}

void emAfPluginFragmentationHandleSourceRoute(txFragmentedPacket* txPacket,
                                              uint16_t indexOrDestination)
{
//now this is unnneccessary and similar to SoC
}

void emAfPluginFragmentationSendReply(EmberNodeId sender,
                                      EmberApsFrame* apsFrame,
                                      rxFragmentedPacket* rxPacket)
{
  apsFrame->groupId = HIGH_LOW_TO_INT(rxPacket->fragmentMask, rxPacket->fragmentBase);
  ezspSendReply(sender, apsFrame, 0, NULL);
}
