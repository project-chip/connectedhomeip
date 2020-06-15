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
 * @brief User interface for delayd join.
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"
#include "app/framework/plugin/delayed-join/delayed-join.h"

#include "app/framework/util/af-main.h"
#include "stack/include/trust-center.h"

// *****************************************************************************
// Functions

void emAfDelayedJoinActivateCommand(void)
{
  #if defined(EZSP_HOST)
  EzspStatus status = emberAfSetEzspPolicy(EZSP_TRUST_CENTER_POLICY,
                                           (EZSP_DECISION_ALLOW_JOINS
                                            | EZSP_DECISION_ALLOW_UNSECURED_REJOINS
                                            | EZSP_DECISION_DEFER_JOINS),
                                           "Trust Center Policy",
                                           "Delayed joins");

  if (EZSP_SUCCESS != status) {
    emberAfCorePrintln("%p: %p: 0x%X",
                       EMBER_AF_PLUGIN_DELAYED_JOIN_PLUGIN_NAME,
                       "failed to configure delayed joining",
                       status);
  }
  #endif // EZSP_HOST
}

void emAfDelayedJoinSendNetworkKeyCommand(void)
{
  EmberNodeId targetShort = (EmberNodeId)emberUnsignedCommandArgument(0);
  EmberEUI64  targetLong;
  emberCopyBigEndianEui64Argument(1, targetLong);
  EmberNodeId parentShort = (EmberNodeId)emberUnsignedCommandArgument(2);

  if (EMBER_SUCCESS != emberUnicastCurrentNetworkKey(targetShort,
                                                     targetLong,
                                                     parentShort)) {
    emberAfCorePrintln("%p: %p 0x%X",
                       EMBER_AF_PLUGIN_DELAYED_JOIN_PLUGIN_NAME,
                       "failed to send the encrypted network key to ",
                       targetShort);
  }
}

void emAfDelayedJoinSetNetworkKeyTimeoutCommand(void)
{
  uint8_t seconds = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginDelayedJoinSetNetworkKeyTimeout(seconds);
}
