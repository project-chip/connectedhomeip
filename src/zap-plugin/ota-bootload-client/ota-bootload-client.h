/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief ZCL OTA Bootload Client API
 ******************************************************************************/

#ifndef __OTA_BOOTLOAD_CLIENT_H__
#define __OTA_BOOTLOAD_CLIENT_H__

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE

/**
 * @addtogroup ZCLIP
 *
 * @{
 */

/**
 * @addtogroup OTA_Bootload
 * @{
 */

/**
 * @addtogroup OTA_Bootload_Types
 * @{
 */

// -----------------------------------------------------------------------------
// Types

/** This structure holds information about an OTA Server device. */
typedef struct {
  /** Protocol used to communicate with the server. */
  EmberZclScheme_t scheme;
  uint8_t const *name;
  uint8_t nameLength;
  /** IPv6 address of the server. */
  EmberIpv6Address address;
  /** UDP port of the server. */
  uint16_t port;
  /** UID of the server. */
  EmberZclUid_t uid;
  /** Endpoint ID of the server. */
  EmberZclEndpointId_t endpointId;
} EmberZclOtaBootloadClientServerInfo_t;

/** @} end addtogroup OTA_Bootload_Types */

/**
 * @addtogroup OTA_Bootload_API
 *
 * See @ref ota-bootload-client-callbacks for the application callbacks
 * associated with the OTA Bootload Client plugin.
 * @{
 */

// -----------------------------------------------------------------------------
// API

/** @} end addtogroup OTA_Bootload_API */

/** @} end addtogroup OTA_Bootload */

/** @} end addtogroup ZCLIP */

#endif // __OTA_BOOTLOAD_CLIENT_H__
