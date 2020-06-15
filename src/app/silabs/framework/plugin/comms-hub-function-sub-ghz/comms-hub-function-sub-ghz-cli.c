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
 * @brief CLI for the Comms Hub Function Sub Ghz plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "comms-hub-function-sub-ghz.h"
#include "../comms-hub-function/comms-hub-function.h"   // the print macros

// External Functions

void emAfPluginCommsHubFunctionSubGhzCliChannelChangeStart(void)
{
  const EmberAfPluginCommsHubFunctionStatus status = emberAfCommsHubFunctionSubGhzStartChannelChangeSequence();
  if (status == EMBER_AF_CHF_STATUS_SUCCESS) {
    emberAfPluginCommsHubFunctionPrintln("OK");
  } else {
    emberAfPluginCommsHubFunctionPrintln("Error: 0x%x", status);
  }
}

void emAfPluginCommsHubFunctionSubGhzCliChannelChangeComplete(void)
{
  const uint8_t page = emberUnsignedCommandArgument(0);
  const uint8_t channel = emberUnsignedCommandArgument(1);
  const uint32_t pageChannelMask = (page << 27 | BIT32(channel)); // note replace as soon as possible with EMBER_PAGE_CHANNEL_MASK_FROM_CHANNEL_NUMBER(page, channel);
  const EmberAfPluginCommsHubFunctionStatus status = emberAfCommsHubFunctionSubGhzCompleteChannelChangeSequence(pageChannelMask);
  if (status == EMBER_AF_CHF_STATUS_SUCCESS) {
    emberAfPluginCommsHubFunctionPrintln("OK");
  } else {
    emberAfPluginCommsHubFunctionPrintln("Error: 0x%x", status);
  }
}
