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
 * @brief Routines for handling endpoints the Comms Hub Function plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "comms-hub-tunnel-endpoints.h"

#define MAX_NUMBER_DEVICES            EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE
#define MAX_DEVICE_TUNNEL_ENDPOINTS   3

typedef struct {
  uint16_t nodeId;
  uint8_t endpoint;
} EmAfTunnelEndpointMap;

static EmAfTunnelEndpointMap endpointMap[MAX_NUMBER_DEVICES];

#define DEFAULT_ADDRESS  0xFFFE

void emberAfPluginTunnelingEndpointInit(void)
{
  uint8_t i;
  for ( i = 0; i < MAX_NUMBER_DEVICES; i++ ) {
    endpointMap[i].nodeId = DEFAULT_ADDRESS;
    endpointMap[i].endpoint = INVALID_TUNNELING_ENDPOINT;
  }
}

// NOTE:  This function and file assume only 1 tunneling endpoint exists on each address.
// If devices will support more than 1 tunneling endpoint, an array of endpoints should
// be stored.
void emberAfPluginAddTunnelingEndpoint(uint16_t nodeId, uint8_t *endpointList, uint8_t numEndpoints)
{
  uint8_t i;
  uint8_t firstUnused = MAX_NUMBER_DEVICES;

  if ( numEndpoints > 0 ) {
    for ( i = 0; i < MAX_NUMBER_DEVICES; i++ ) {
      if ( endpointMap[i].nodeId == nodeId ) {
        return;
      } else if ( (endpointMap[i].nodeId == DEFAULT_ADDRESS) && (firstUnused >= MAX_NUMBER_DEVICES) ) {
        firstUnused = i;
      }
    }
    if ( firstUnused < MAX_NUMBER_DEVICES ) {
      i = firstUnused;
      endpointMap[i].nodeId = nodeId;
      endpointMap[i].endpoint = endpointList[0];
    }
  }
}

uint8_t emberAfPluginGetDeviceTunnelingEndpoint(uint16_t nodeId)
{
  uint8_t i;
  uint8_t endpoint = INVALID_TUNNELING_ENDPOINT;
  for ( i = 0; i < MAX_NUMBER_DEVICES; i++ ) {
    if ( endpointMap[i].nodeId == nodeId ) {
      endpoint = endpointMap[i].endpoint;
      break;
    }
  }
  return endpoint;
}
