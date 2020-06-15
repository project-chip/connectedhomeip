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
 * @brief APIs for handling endpoints the Comms Hub Function plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_COMMS_HUB_TUNNEL_ENDPOINTS_H
#define SILABS_COMMS_HUB_TUNNEL_ENDPOINTS_H

#define INVALID_TUNNELING_ENDPOINT  0xFF

/**
 * @brief Initializes tunneling endpoints.
 *
 * This function initializes the table of device tunnel endpoints.
 *
 */
void emberAfPluginTunnelingEndpointInit(void);

/**
 * @brief Adds a tunneling endpoint.
 *
 * This function adds an address and tunnel endpoint into the table.
 *
 * @param nodeId The address of the device that should be added to the table.
 * @param endpointList The list of tunneling endpoints on the device.
 * @param numEndpoints The number of tunneling endpoints on the device.
 *
 */
void emberAfPluginAddTunnelingEndpoint(uint16_t nodeId, uint8_t *endpointList, uint8_t numEndpoints);

/**
 * @brief Gets a tunneling endpoint.
 *
 * This function returns the tunneling endpoint for the specified nodeId. If an endpoint
 * for the nodeId cannot be found, it returns INVALID_TUNNELING_ENDPOINT.
 *
 * @return The tunneling endpoint for the nodeId, or INVALID_TUNNELING_ENDPOINT.
 *
 */
uint8_t emberAfPluginGetDeviceTunnelingEndpoint(uint16_t nodeId);

#endif  // #ifndef _COMMS_HUB_TUNNEL_ENDPOINTS_H_
