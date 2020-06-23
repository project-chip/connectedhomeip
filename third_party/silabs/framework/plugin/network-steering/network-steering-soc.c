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
 * @brief SoC routines for the Network Steering plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/network-steering/network-steering.h"
#include "app/framework/plugin/network-steering/network-steering-internal.h"

//============================================================================
// Globals

static EmberMessageBuffer storedNetworks = EMBER_NULL_MESSAGE_BUFFER;

#define MAX_NETWORKS (PACKET_BUFFER_SIZE >> 1)  // 16

#define NULL_PAN_ID 0xFFFF

#define PLUGIN_NAME emAfNetworkSteeringPluginName

//============================================================================
// Forward Declarations

//============================================================================

uint8_t emAfPluginNetworkSteeringGetMaxPossiblePanIds(void)
{
  return MAX_NETWORKS;
}

void emAfPluginNetworkSteeringClearStoredPanIds(void)
{
  if (storedNetworks != EMBER_NULL_MESSAGE_BUFFER) {
    emberReleaseMessageBuffer(storedNetworks);
    storedNetworks = EMBER_NULL_MESSAGE_BUFFER;
  }
}

uint16_t* emAfPluginNetworkSteeringGetStoredPanIdPointer(uint8_t index)
{
  if (index >= MAX_NETWORKS) {
    return NULL;
  }

  if (storedNetworks == EMBER_NULL_MESSAGE_BUFFER) {
    storedNetworks = emberAllocateStackBuffer();
    if (storedNetworks == EMBER_NULL_MESSAGE_BUFFER) {
      emberAfCorePrintln("Error: %p failed to allocate stack buffer.", PLUGIN_NAME);
      return NULL;
    }
    MEMSET(emberMessageBufferContents(storedNetworks), 0xFF, PACKET_BUFFER_SIZE);
  }

  return (uint16_t*)(emberMessageBufferContents(storedNetworks) + (index * sizeof(uint16_t)));
}
