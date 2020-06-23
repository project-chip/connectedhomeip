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
 * @brief CLI for the Network Find plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "network-find.h"

//------------------------------------------------------------------------------
// Definitions for legacy CLI

#if !defined(EMBER_AF_GENERATE_CLI)

void findMaskCommand(void);
void findModeCommand(void);
void findPrintCommand(void);

const char * const emberCommandTableNetworkFindMaskCommandArguments[] = {
  "channel page (0 and, if sub-GHz support included, 28-31)",
  "channel mask",
  NULL
};

const char * const emberCommandTableNetworkFindModeCommandArguments[] = {
  "Bit 0 = enable 2.4GHz, Bit 1 = enable sub-GHz",
  NULL
};

EmberCommandEntry emberAfPluginNetworkFindCommands[] = {
  emberCommandEntryActionWithDetails("mask", findMaskCommand, "uw", "Set a channel mask for a given channel page for 'find joinable' and 'find unused'.", emberCommandTableNetworkFindMaskCommandArguments),
  emberCommandEntryActionWithDetails("mode", findModeCommand, "u", "Set a search mode for 'find joinable' and 'find unused'.", emberCommandTableNetworkFindModeCommandArguments),
  emberCommandEntryActionWithDetails("print", findPrintCommand, "", "Print network search parameters (mode and channel masks).", NULL),
  emberCommandEntryTerminator(),
};

#endif // EMBER_AF_GENERATE_CLI

//------------------------------------------------------------------------------

void findMaskCommand(void)
{
  const uint8_t page = (uint8_t)emberUnsignedCommandArgument(0);
  const uint32_t mask = (uint32_t)emberUnsignedCommandArgument(1);

  const EmberStatus status = emberAfSetFormAndJoinChannelMask(page, mask);

  switch (status) {
    case EMBER_SUCCESS:
      break;
    case EMBER_PHY_INVALID_CHANNEL:
      emberAfCorePrintln("Invalid channel page %d", page);
      break;
    case EMBER_MAC_INVALID_CHANNEL_MASK:
      emberAfCorePrintln("Invalid mask for channel page %d", page);
      break;
    default:
      emberAfCorePrintln("Error 0x%X", status);
      break;
  }
}

void findModeCommand(void)
{
  const uint8_t mode = (uint8_t)emberUnsignedCommandArgument(0);

  const EmberStatus status = emberAfSetFormAndJoinSearchMode(mode);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error 0x%X", status);
  }
}

void findPrintCommand(void)
{
  uint8_t page;
  uint8_t mode;
  bool allowAllChannels;

  // Only page 0 (and 28-31 if sub-GHz support is included) are valid.
  // For simplicity, go from 0 to 31 and simply skip any invalid page.
  for (page = 0; page <= 31; ++page) {
    const uint32_t mask = emberAfGetFormAndJoinChannelMask(page);
    if (mask != 0xFFFFFFFFU) {
      emberAfCorePrint("Page %d mask 0x%4X, %p", page, mask, "channels: ");
      emberAfCoreDebugExec(emberAfPrintChannelListFromMask(mask));
      emberAfCorePrintln("");
    }
  }

  mode = emberAfGetFormAndJoinSearchMode();
  emberAfCorePrintln("%s%s scan is %sABLED",
                     "2.4 ",
                     "GHz",
                     (mode & FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ) ? "EN" : "DIS");
  emberAfCorePrintln("%s%s scan is %sABLED",
                     "Sub-",
                     "GHz",
                     (mode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ) ? "EN" : "DIS");

  allowAllChannels = emberAfPluginNetworkFindGetEnableScanningAllChannelsCallback();
  emberAfCorePrintln("%s%s scan is %sABLED",
                     "Fallback on ",
                     "all channels",
                     allowAllChannels ? "EN" : "DIS");
}
