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
 * @brief Tokens for the WWAH Connectivity Manager plugin.
 *******************************************************************************
   ******************************************************************************/

#include "wwah-connectivity-manager.h"

/**
 * Custom Application Tokens
 */

#define CREATOR_PLUGIN_CONNECTION_MANAGER_STATE                       (0x0015)
#define NVM3KEY_PLUGIN_CONNECTION_MANAGER_STATE                       (NVM3KEY_DOMAIN_USER | 0x0015)

#define CREATOR_PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_INTERVAL  (0x0016)
#define NVM3KEY_PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_INTERVAL  (NVM3KEY_DOMAIN_USER | 0x0016)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
typedef struct {
  // bit 0 = connection manager enabled state
  // bit 1 = connection manager bad parent recovery
  // bit 2 = connection manager trust center connectivity
  // bit 3 = wwah periodic router checkins enabled state
  // Remaining bits are reserved
  uint32_t connectionManagerFlags;

  uint16_t fastRejoinTimeoutSec;
  uint16_t durationBetweenEachRejoinSec;
  uint16_t fastRejoinFirstBackoffTimeSec;
  uint16_t maxBackoffTimeSec;
  uint16_t maxBackoffIter;
} EmberAfConnectionManagerTokenStruct;

#define PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE                          (0x00000001)
#define PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY                    (0x00000002)
#define PLUGIN_WWAH_CONNECTION_MANAGER_TRUST_CENTER_CONNECTIVITY              (0x00000004)
#define PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE  (0x00000008)

#endif  // DEFINETYPES

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(PLUGIN_CONNECTION_MANAGER_STATE,
                   EmberAfConnectionManagerTokenStruct,
                   { 0x00000000, EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_FAST_REJOIN_TIMEOUT_SEC, EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_DURATION_BETWEEN_EACH_REJOIN_SEC, EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_FAST_REJOIN_FIRST_BACKOFF_TIME_SEC, EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_MAX_BACKOFF_TIME_SEC, EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_MAX_BACKOFF_ITERATIONS })
DEFINE_BASIC_TOKEN(PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_INTERVAL,
                   uint16_t,
                   0xFFFF)

#endif  // DEFINETOKENS
