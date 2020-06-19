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
 * @brief APIs and defines for the Meter Mirror plugin, which handles mirroring
 *        attributes of a sleepy end device.
 *******************************************************************************
   ******************************************************************************/

// A bit confusing, the EMBER_AF_MANUFACTURER_CODE is actually the manufacturer
// code defined in AppBuilder.  This is usually the specific vendor of
// the local application.  It does not have to be "Ember's" (Silabs) manufacturer
// code, but that is the default.
#define EM_AF_APPLICATION_MANUFACTURER_CODE EMBER_AF_MANUFACTURER_CODE

uint8_t emAfPluginMeterMirrorGetMirrorsAllocated(void);

extern EmberEUI64 nullEui64;

#define EM_AF_MIRROR_ENDPOINT_END              \
  (EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START \
   + EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS)

bool emberAfPluginMeterMirrorGetEui64ByEndpoint(uint8_t endpoint,
                                                EmberEUI64 returnEui64);
bool emberAfPluginMeterMirrorIsMirrorUsed(uint8_t endpoint);
bool emberAfPluginMeterMirrorGetEndpointByEui64(EmberEUI64 eui64,
                                                uint8_t *returnEndpoint);
uint16_t emberAfPluginMeterMirrorRequestMirror(EmberEUI64 requestingDeviceIeeeAddress);
uint16_t emberAfPluginMeterMirrorRemoveMirror(EmberEUI64 requestingDeviceIeeeAddress);
