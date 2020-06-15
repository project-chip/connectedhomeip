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
 * @brief APIs and defines for the Network Steering plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_NETWORK_STEERING_H
#define SILABS_NETWORK_STEERING_H

// -----------------------------------------------------------------------------
// Constants

extern const uint8_t emAfNetworkSteeringPluginName[];

// -----------------------------------------------------------------------------
// Types

enum {
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE                         = 0x00,
  // The Use Configured Key states are only run if explicitly configured to do
  // so. See emAfPluginNetworkSteeringSetConfiguredKey()
#ifndef EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CONFIGURED      = 0x01,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CONFIGURED    = 0x02,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE    = 0x03,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE  = 0x04,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED     = 0x05,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED   = 0x06,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED     = 0x07,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED   = 0x08,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED                = 0x09,
#else // EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_INSTALL_CODE    = 0x01,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_CENTRALIZED     = 0x02,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_DISTRIBUTED     = 0x03,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_INSTALL_CODE  = 0x04,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_CENTRALIZED   = 0x05,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_DISTRIBUTED   = 0x06,

  // Either the USE_ALL_KEY states are run or the non USE_ALL_KEY states are
  // run, but never both
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_PRIMARY_USE_ALL_KEYS    = 0x07,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_SECONDARY_USE_ALL_KEYS  = 0x08,

  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_SCAN_FINISHED                = 0x09,
#endif // EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIMIZE_SCANS
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_UPDATE_TCLK                  = 0x10,
  EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_VERIFY_TCLK                  = 0x20,
};
typedef uint8_t EmberAfPluginNetworkSteeringJoiningState;

enum {
  EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NONE                       = 0x00,
  EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE             = 0x01,
};
typedef uint8_t EmberAfPluginNetworkSteeringOptions;

typedef struct {
  uint32_t currentChannelMask;
  EmberBeaconIterator beaconIterator;
} NetworkSteeringState;

extern EmberAfPluginNetworkSteeringOptions emAfPluginNetworkSteeringOptionsMask;

// -----------------------------------------------------------------------------
// Globals

/** @brief The first set of channels on which to search for joinable networks. */
extern uint32_t emAfPluginNetworkSteeringPrimaryChannelMask;
/** @brief The second set of channels on which to search for joinable networks. */
extern uint32_t emAfPluginNetworkSteeringSecondaryChannelMask;

// -----------------------------------------------------------------------------
// API

/** @brief Initiates a network-steering procedure.
 *
 *
 * If the node is currently on a network, it will perform network steering,
 * in which it opens up the network with a broadcast
 * permit join message.
 *
 * If the node is not on a network, it will scan a series of primary channels
 * (see ::emAfPluginNetworkSteeringPrimaryChannelMask) to find possible
 * networks to join. If it is unable to join any of those networks, it will
 * try scanning on a set of secondary channels
 * (see ::emAfPluginNetworkSteeringSecondaryChannelMask). Upon completion of
 * this process, the plugin will call
 * ::emberAfPluginNetworkSteeringCompleteCallback with information regarding
 * the success or failure of the procedure.
 *
 * This procedure will try to join networks using install codes, the centralized
 * default key, and the distributed default key.
 *
 * @return An ::EmberStatus value that indicates the success or failure of
 * the initiating of the network steering process.
 *
 * @note Do not call this API from a stack status callback, as this plugin acts
 * when its own stack status callback is invoked.
 */
EmberStatus emberAfPluginNetworkSteeringStart(void);

/** @brief Stops the network steering procedure.
 *
 *
 * @return An ::EmberStatus value that indicates the success or failure of
 * the initiating of the network steering process.
 */
EmberStatus emberAfPluginNetworkSteeringStop(void);

/** @brief Overrides the channel mask. */
void emAfPluginNetworkSteeringSetChannelMask(uint32_t mask, bool secondaryMask);

/** @brief Sets extended PAN ID to search for. */
void emAfPluginNetworkSteeringSetExtendedPanIdFilter(uint8_t* extendedPanId,
                                                     bool turnFilterOn);

/** @brief Set a different key to use when joining. */
void emAfPluginNetworkSteeringSetConfiguredKey(uint8_t *key,
                                               bool useConfiguredKey);

/** @brief Cleans up the network steering process which took place */
void emAfPluginNetworkSteeringCleanup(EmberStatus status);

#endif /* __NETWORK_STEERING_H__ */
