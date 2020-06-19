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
 * @brief CLI for the EZ-Mode plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "ez-mode.h"

#ifdef EMBER_AF_LEGACY_CLI
  #error The EZ-Mode Commissioning plugin is not compatible with the legacy CLI.
#endif

#define MAX_CLUSTER_IDS 3
static EmberAfClusterId clusterIds[MAX_CLUSTER_IDS];

// plugin ezmode-commissioning client <endpoint:1> <direction:1> <cluster ids:n>
void emberAfPluginEzModeCommissioningClientCommand(void)
{
  EmberStatus status;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfEzModeCommissioningDirection direction = (EmberAfEzModeCommissioningDirection)emberUnsignedCommandArgument(1);
  uint8_t clusterIdLength;
  uint8_t count = emberCommandArgumentCount() - 2;
  if (MAX_CLUSTER_IDS < count) {
    count = MAX_CLUSTER_IDS;
    emberAfAppPrintln("Cluster ids truncated to %d", count);
  }
  for (clusterIdLength = 0; clusterIdLength < count; clusterIdLength++) {
    clusterIds[clusterIdLength]
      = (EmberAfClusterId)emberUnsignedCommandArgument(clusterIdLength + 2);
  }
  status = emberAfEzmodeClientCommission(endpoint,
                                         direction,
                                         clusterIds,
                                         clusterIdLength);
  UNUSED_VAR(status);
  emberAfAppPrintln("%p 0x%x", "client", status);
}

// plugin ezmode-commissioning server <endpoint:1> <identify time:1>
void emberAfPluginEzModeCommissioningServerCommand(void)
{
  EmberStatus status;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t identifyTimeS = (uint16_t)emberUnsignedCommandArgument(1);
  if (identifyTimeS == 0) {
    identifyTimeS = EMBER_AF_PLUGIN_EZMODE_COMMISSIONING_IDENTIFY_TIMEOUT;
  }
  status = emberAfEzmodeServerCommissionWithTimeout(endpoint, identifyTimeS);
  UNUSED_VAR(status);
  emberAfAppPrintln("%p 0x%x", "server", status);
}
