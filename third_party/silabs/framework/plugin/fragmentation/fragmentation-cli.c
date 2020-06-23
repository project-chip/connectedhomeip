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
 * @brief CLI commands related to the fragmentation code.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "fragmentation.h"

//------------------------------------------------------------------------------
// CLI stuff

void emAfPluginFragmentationSetWindowSizeCommand(void);

#ifdef EZSP_HOST
void emAfResetAndInitNCP(void);
#endif //EZSP_HOST

//------------------------------------------------------------------------------
// Globals

#if !defined(EMBER_AF_GENERATE_CLI)

EmberCommandEntry emberAfPluginFragmentationCommands[] = {
  // This is commented out for everything but simulation due to the fact
  // it is non-standard.  Smart Energy defines the window size of 1 as
  // the only supported configuration.
#ifdef EMBER_TEST
  emberCommandEntryAction("set-rx-window-size",
                          emAfPluginFragmentationSetWindowSizeCommand,
                          "u",
                          "Sets the window size for fragmented transmissions."),
  emberCommandEntryAction("artificial-block-drop",
                          emAfPluginFragmentationArtificialBlockDropCommand,
                          "Sets up the plugin to artificially drop a block"),

#endif //EMBER_TEST
  emberCommandEntryTerminator(),
};

#endif

//------------------------------------------------------------------------------
// Functions

void emAfPluginFragmentationSetWindowSizeCommand(void)
{
  emberFragmentWindowSize = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfAppPrintln("Fragmentation RX window size set to %d",
                    emberFragmentWindowSize);

#ifdef EZSP_HOST
  emAfResetAndInitNCP();
#endif //EZSP_HOST
}

void emAfPluginFragmentationArtificialBlockDropCommand(void)
{
#if defined(EMBER_TEST)
  emAfPluginFragmentationArtificiallyDropBlockNumber = emberUnsignedCommandArgument(0);
#endif
}
