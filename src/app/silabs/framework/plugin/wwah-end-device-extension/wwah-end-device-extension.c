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
 * @brief Routines for the WWAH End Device Extension plugin, which sets up
 *        additional configuration needed by end devices operating on a WWAH
 *        network.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "wwah-end-device-extension.h"
#include "../end-device-support/end-device-support.h"

extern EmberEventControl emberAfPluginWwahEndDeviceExtensionUnenforceFastPollNetworkEventControls[];

void emberAfPluginWwahEndDeviceExtensionStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_UP) {
    emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);

    emberAfNetworkEventControlSetDelayMS(emberAfPluginWwahEndDeviceExtensionUnenforceFastPollNetworkEventControls,
                                         EMBER_AF_PLUGIN_WWAH_END_DEVICE_EXTENSION_FAST_POLL_ON_NETWORK_UP * 1000);
  } else if (status == EMBER_NETWORK_DOWN) {
    emberAfNetworkEventControlSetInactive(emberAfPluginWwahEndDeviceExtensionUnenforceFastPollNetworkEventControls);
  }
}

void emberAfPluginWwahEndDeviceExtensionUnenforceFastPollNetworkEventHandler(void)
{
  emberAfRemoveFromCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);
}
