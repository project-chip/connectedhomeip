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
 * @brief Code to manage the mirrors via the CLI.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "meter-mirror.h"

uint16_t emberAfPluginSimpleMeteringClientRemoveMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress);

//-----------------------------------------------------------------------------
// Globals

void emAfPluginMeterMirrorStatusCommand(void)
{
  uint8_t meteringDeviceTypeAttribute = 0xFF;
  uint16_t endpoint;
  uint8_t mirrorEui[EUI64_SIZE];

  emberAfSimpleMeteringClusterPrintln("Known meters");
  // Go over our endpoints and find an unallocated meter mirror
  for (endpoint = EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
       endpoint < EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START + EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS;
       endpoint++) {
    bool validEndpoint;

    // Reset these every run
    MEMSET(mirrorEui, 0xFF, EUI64_SIZE);
    meteringDeviceTypeAttribute = 0xFF;
    validEndpoint = emberAfPluginMeterMirrorGetEui64ByEndpoint(endpoint,
                                                               mirrorEui);

    if (!validEndpoint) {
      continue;
    }
    if (emberAfPluginMeterMirrorIsMirrorUsed(endpoint)) {
      // Retrieve the meter device type, this will tell us what
      // type of meter we are looking at on this endpoint
      emberAfReadServerAttribute(endpoint,
                                 ZCL_SIMPLE_METERING_CLUSTER_ID,
                                 ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                                 &meteringDeviceTypeAttribute,
                                 sizeof(meteringDeviceTypeAttribute));
    }

    emberAfSimpleMeteringClusterPrint("ep: %d  ieee:", endpoint);
    emberAfPrintBigEndianEui64(mirrorEui);
    emberAfSimpleMeteringClusterPrintln("  type: 0x%X", meteringDeviceTypeAttribute);
  }
  // Explicitly usea  function call to find the number of mirrors allocated instead
  // of just calculating it.  In case there is ever a discrepancy,
  // this will help track down any bugs.

  emberAfSimpleMeteringClusterPrintln("%d of %d mirrors used.",
                                      emAfPluginMeterMirrorGetMirrorsAllocated(),
                                      EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS);
}

void emAfPluginMeterMirrorRemoveCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberEUI64 mirrorEui64;
  bool success = emberAfPluginMeterMirrorGetEui64ByEndpoint(endpoint, mirrorEui64);
  if (!success) {
    emberAfSimpleMeteringClusterPrintln("Error: Failed to obtain EUI64 for endpoint %d", endpoint);
    return;
  }
  emberAfPluginSimpleMeteringClientRemoveMirrorCallback(mirrorEui64);
}
