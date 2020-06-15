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
 * @brief Tokens for the WWAH Server Silabs plugin.
 *******************************************************************************
   ******************************************************************************/

#include "wwah-server-silabs.h"

/**
 * Custom Application Tokens
 */
#define CREATOR_PLUGIN_WWAH_CLIENT_ENDPOINT         (0x0010)
#define CREATOR_WWAH_MAC_POLL_FAILURE_WAIT_TIME     (0x0011)
// The following is a simee mask for ZLL state, Configuration Mode, and Parent
// Classification
#define CREATOR_PLUGIN_WWAH_CONFIGURATION_MASK      (0x0012)
#define CREATOR_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_CHANNEL  (0x0013)
#define CREATOR_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_PANID  (0x0014)
// 0x0015 taken by wwah-connection-manager-tokens.h
// 0x0016 taken by wwah-connection-manager-tokens.h
#define CREATOR_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER  (0x0017)

// 0x001B taken by wwah-app-event-retry-tokens.h
// 0x001C taken by wwah-app-event-retry-tokens.h
// 0x001D taken by wwah-app-event-retry-tokens.h
// 0x001E taken by wwah-app-event-retry-tokens.h
// 0x001F taken by wwah-app-event-retry-tokens.h
// 0x0020 taken by ias-zone-server-tokens.h

#define NVM3KEY_PLUGIN_WWAH_CLIENT_ENDPOINT       (NVM3KEY_DOMAIN_USER | 0x0010)
#define NVM3KEY_WWAH_MAC_POLL_FAILURE_WAIT_TIME   (NVM3KEY_DOMAIN_USER | 0x0011)
// The following is a nvm3 mask for ZLL state, Configuration Mode, and Parent
// Classification
#define NVM3KEY_PLUGIN_WWAH_CONFIGURATION_MASK    (NVM3KEY_DOMAIN_USER | 0x0012)
#define NVM3KEY_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_CHANNEL  (NVM3KEY_DOMAIN_USER | 0x0013)
#define NVM3KEY_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_PANID  (NVM3KEY_DOMAIN_USER | 0x0014)
// (NVM3KEY_DOMAIN_USER | 0x0015) taken by wwah-connection-manager-tokens.h
// (NVM3KEY_DOMAIN_USER | 0x0016) taken by wwah-connection-manager-tokens.h
#define NVM3KEY_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER  (NVM3KEY_DOMAIN_USER | 0x0017)

// (NVM3KEY_DOMAIN_USER | 0x001B) taken by wwah-app-event-retry-tokens.h
// (NVM3KEY_DOMAIN_USER | 0x001C) taken by wwah-app-event-retry-tokens.h
// (NVM3KEY_DOMAIN_USER | 0x001D) taken by wwah-app-event-retry-tokens.h
// (NVM3KEY_DOMAIN_USER | 0x001E) taken by wwah-app-event-retry-tokens.h
// (NVM3KEY_DOMAIN_USER | 0x001F) taken by wwah-app-event-retry-tokens.h
// (NVM3KEY_DOMAIN_USER | 0x0020) taken by ias-zone-server-tokens.h

#define MAC_POLL_FAILURE_WAIT_TIME_DEFAULT  0x03
#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#endif  // DEFINETYPES

#ifdef DEFINETOKENS
// Define the actual token storage information here

DEFINE_BASIC_TOKEN(PLUGIN_WWAH_CLIENT_ENDPOINT, uint8_t, 0x00)
DEFINE_BASIC_TOKEN(WWAH_MAC_POLL_FAILURE_WAIT_TIME, uint8_t, MAC_POLL_FAILURE_WAIT_TIME_DEFAULT)
DEFINE_BASIC_TOKEN(PLUGIN_WWAH_CONFIGURATION_MASK,
                   uint8_t,
                   PLUGIN_WWAH_CONFIGURATION_MASK_DEFAULT)
DEFINE_BASIC_TOKEN(PLUGIN_WWAH_PENDING_NETWORK_UPDATE_CHANNEL,
                   uint8_t,
                   0xFF)
DEFINE_BASIC_TOKEN(PLUGIN_WWAH_PENDING_NETWORK_UPDATE_PANID,
                   uint16_t,
                   0xFFFF)
DEFINE_INDEXED_TOKEN(PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER,
                     uint16_t,
                     MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_TOKEN,
                     { PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER_NULL_VALUE })

#endif  // DEFINETOKENS
