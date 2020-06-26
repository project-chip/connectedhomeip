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
 * @brief Config for Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading device specific file.
 *
 * This file defines the interface for the customer's application to
 * control the behavior of the OTA client.
 *******************************************************************************
   ******************************************************************************/

// Note: EMBER_AF_MANUFACTURER_CODE defined in client's config

#if !defined(EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_IMAGE_TYPE_ID)
  #define EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_IMAGE_TYPE_ID    0x5678
#endif

#if !defined(EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_FIRMWARE_VERSION)
  #define EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_FIRMWARE_VERSION 0x00000005L
#endif

#define SECONDS_IN_MS (1000L)
#define MINUTES_IN_MS (60 * SECONDS_IN_MS)
#define HOURS_IN_MS (60 * MINUTES_IN_MS)

// By default if hardware version is not defined, it is not used.
// Most products do not limit upgrade images based on the version.
// Instead they have different images for different hardware.  However
// this can provide support for an image that only supports certain hardware
// revision numbers.
#define EMBER_AF_INVALID_HARDWARE_VERSION 0xFFFF
#if !defined(EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_HARDWARE_VERSION)
  #define EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_HARDWARE_VERSION EMBER_AF_INVALID_HARDWARE_VERSION
#endif
