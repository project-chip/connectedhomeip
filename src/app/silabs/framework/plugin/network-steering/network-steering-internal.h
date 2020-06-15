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
 * @brief Internal APIs and defines for the Network Steering plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/plugin/network-steering/network-steering.h"

extern const char * emAfPluginNetworkSteeringStateNames[];
extern uint8_t emAfPluginNetworkSteeringTotalBeacons;
extern uint8_t emAfPluginNetworkSteeringJoinAttempts;

#ifndef EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS
extern uint8_t emAfPluginNetworkSteeringPanIdIndex;
#endif // #ifndef EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS

uint8_t emAfPluginNetworkSteeringGetMaxPossiblePanIds(void);
void emAfPluginNetworkSteeringClearStoredPanIds(void);
uint16_t* emAfPluginNetworkSteeringGetStoredPanIdPointer(uint8_t index);

void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState);
uint8_t emAfPluginNetworkSteeringGetCurrentChannel();

extern EmberAfPluginNetworkSteeringJoiningState emAfPluginNetworkSteeringState;

#ifdef EMBER_AF_PLUGIN_NETWORK_STEERING_TRY_ALL_KEYS
  #define TRYING_ALL_KEYS                                                \
  (((emAfPluginNetworkSteeringState)                                     \
    == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS) \
   || ((emAfPluginNetworkSteeringState)                                  \
       == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS))
#else // EMBER_AF_PLUGIN_NETWORK_STEERING_TRY_ALL_KEYS
    #define TRYING_ALL_KEYS         false
#endif // EMBER_AF_PLUGIN_NETWORK_STEERING_TRY_ALL_KEYS

#define emAfPluginNetworkSteeringStateUsesInstallCodes()                      \
  (((emAfPluginNetworkSteeringState)                                          \
    == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE)      \
   || ((emAfPluginNetworkSteeringState)                                       \
       == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE) \
   || TRYING_ALL_KEYS)

#define emAfPluginNetworkSteeringStateUsesCentralizedKey()              \
  (((emAfPluginNetworkSteeringState)                                    \
    == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED) \
   || ((emAfPluginNetworkSteeringState)                                 \
       == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED))

#define emAfPluginNetworkSteeringStateUsesDistributedKey()              \
  (((emAfPluginNetworkSteeringState)                                    \
    == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED) \
   || ((emAfPluginNetworkSteeringState)                                 \
       == EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED))

#define emAfPluginNetworkSteeringStateSetUpdateTclk() \
  ((emAfPluginNetworkSteeringState)                   \
     |= EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_UPDATE_TCLK)
#define emAfPluginNetworkSteeringStateUpdateTclk() \
  ((emAfPluginNetworkSteeringState)                \
   & EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_UPDATE_TCLK)
#define emAfPluginNetworkSteeringStateClearUpdateTclk() \
  ((emAfPluginNetworkSteeringState)                     \
     &= ~EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_UPDATE_TCLK)

#define emAfPluginNetworkSteeringStateSetVerifyTclk() \
  ((emAfPluginNetworkSteeringState)                   \
     |= EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_VERIFY_TCLK)
#define emAfPluginNetworkSteeringStateVerifyTclk() \
  ((emAfPluginNetworkSteeringState)                \
   & EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_VERIFY_TCLK)
#define emAfPluginNetworkSteeringStateClearVerifyTclk() \
  ((emAfPluginNetworkSteeringState)                     \
     &= ~EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_VERIFY_TCLK)
