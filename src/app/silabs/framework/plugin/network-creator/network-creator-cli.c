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
 * @brief CLI for the Network Creator plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "network-creator.h"

#if defined(EMBER_AF_GENERATE_CLI) || defined(EMBER_AF_API_COMMAND_INTERPRETER2)

// -----------------------------------------------------------------------------
// CLI Command Definitions

// plugin network-creator start <centralized:1>
void emberAfPluginNetworkCreatorStartCommand(void)
{
  bool centralizedSecurity = (bool)emberUnsignedCommandArgument(0);
  EmberStatus status = emberAfPluginNetworkCreatorStart(centralizedSecurity);

  emberAfCorePrintln("%p: %p: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                     "Form",
                     status);
}

// plugin network-creator form <centralized:1> <panId:1> <radioTxPower:1> <channel:1>
void emberAfPluginNetworkCreatorFormCommand(void)
{
  bool centralizedSecurity = (bool)emberUnsignedCommandArgument(0);
  EmberPanId panId = (EmberPanId)emberUnsignedCommandArgument(1);
  int8_t radioTxPower = (int8_t)emberSignedCommandArgument(2);
  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(3);

  EmberStatus status = emberAfPluginNetworkCreatorNetworkForm(centralizedSecurity,
                                                              panId,
                                                              radioTxPower,
                                                              channel);

  emberAfCorePrintln("%p: %p: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                     "Form",
                     status);
}

// plugin network-creator mask add <mask:1> <channel:1>
// plugin network-creator mask subtract <mask:1> <channel:1>
// plugin network-creator mask set <mask:1> <new:1>
void emberAfPluginNetworkCreatorChannelMaskCommand(void)
{
  bool channelMaskIsPrimary = ((uint8_t)emberUnsignedCommandArgument(0) == 1);
  uint32_t channelOrNewMask = (uint32_t)emberUnsignedCommandArgument(1);
  uint32_t *channelMask = (channelMaskIsPrimary
                           ? &emAfPluginNetworkCreatorPrimaryChannelMask
                           : &emAfPluginNetworkCreatorSecondaryChannelMask);

  // Check if operation is add or subtract first.
  if (emberStringCommandArgument(-1, NULL)[1] != 'e') {
    if (channelOrNewMask < EMBER_MIN_802_15_4_CHANNEL_NUMBER
        || channelOrNewMask > EMBER_MAX_802_15_4_CHANNEL_NUMBER) {
      emberAfCorePrintln("%p: %p: 0x%X",
                         EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                         "Illegal 802.15.4 channel",
                         channelOrNewMask);
    } else if (emberStringCommandArgument(-1, NULL)[0] == 'a') {
      *channelMask |= (1 << channelOrNewMask);
    } else {
      *channelMask &= ~(1 << channelOrNewMask);
    }
  } else {
    *channelMask = channelOrNewMask;
  }

  emberAfCorePrint("%p channel mask now: 0x%4X [",
                   (channelMaskIsPrimary ? "Primary" : "Secondary"),
                   *channelMask);
  emberAfPrintChannelListFromMask(*channelMask);
  emberAfCorePrintln("]");
}

// plugin network-creator status
void emberAfPluginNetworkCreatorStatusCommand(void)
{
  uint8_t i;
  uint32_t masks[] = { 0,
                       emAfPluginNetworkCreatorPrimaryChannelMask,
                       emAfPluginNetworkCreatorSecondaryChannelMask };

  emberAfCorePrintln("%p: %p:",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME,
                     "Channel mask");
  for (i = 1; i <= 2; i++) {
    emberAfCorePrint("    (%d) 0x%4X [",
                     i,
                     masks[i]);
    emberAfPrintChannelListFromMask(masks[i]);
    emberAfCorePrintln("]");
  }
}

#endif /*
          defined(EMBER_AF_GENERATE_CLI)
        || defined(EMBER_AF_API_COMMAND_INTERPRETER2)
        */
