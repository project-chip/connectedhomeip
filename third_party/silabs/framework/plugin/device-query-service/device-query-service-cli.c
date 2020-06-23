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
 * @brief CLI for the Device Query Service plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/device-query-service/device-query-service.h"
#include "app/framework/plugin/device-database/device-database.h"

//============================================================================
// Globals

//============================================================================
// Forward declarations

//============================================================================

void emAfPluginDeviceQueryServiceEnableDisableCommand(void)
{
  emberAfPluginDeviceQueryServiceEnableDisable(emberCommandName()[0] == 'e');
}

void emAfPluginDeviceQueryServiceStatusCommand(void)
{
  EmberEUI64 currentEui64;
  bool enabled = emberAfPluginDeviceQueryServiceGetEnabledState();
  emberAfPluginDeviceQueryServiceGetCurrentDiscoveryTargetEui64(currentEui64);
  emberAfCorePrintln("Enabled: %p", enabled ? "yes" : "no");
  emberAfCorePrint("Current Discovery Target: ");
  emberAfPrintBigEndianEui64(currentEui64);
  emberAfCorePrintln("");
  const EmberAfDeviceInfo* device = emberAfPluginDeviceDatabaseFindDeviceByEui64(currentEui64);
  emberAfCorePrintln("Status: %p",
                     (device == NULL
                      ? ""
                      : emberAfPluginDeviceDatabaseGetStatusString(device->status)));
}
