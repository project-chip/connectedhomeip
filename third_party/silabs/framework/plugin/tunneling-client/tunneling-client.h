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
 * @brief Definitions for the Tunneling Client plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX 0xFF

/**
 * @brief Requests a tunneling cluster tunnel with a server.
 *
 * This function can be used to request a tunnel with a server. The tunneling
 * client plugin will look up the long address of the server (using discovery,
 * if necessary), establish a link key with the server, and create an address
 * table entry for the server before sending the request. All future
 * communication using the tunnel will be sent using the address table entry.
 * The plugin will call ::emberAfPluginTunnelingClientTunnelOpenedCallback with
 * the status of the request.
 *
 * @param server The network address of the server to which the request will be
 * sent.
 * @param clientEndpoint The local endpoint from which the request will be
 * sent.
 * @param serverEndpoint The remote endpoint to which the request will be sent.
 * @param protocolId The protocol ID of the requested tunnel.
 * @param manufacturerCode The manufacturer code of the requested tunnel.
 * @param flowControlSupport true if flow control support is requested or false
 * if not.  Note: flow control is not currently supported by the Tunneling
 * client or server plugins.
 * @return ::EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS if the request is in
 * process or another ::EmberAfPluginTunnelingClientStatus otherwise.
 */
EmberAfPluginTunnelingClientStatus emberAfPluginTunnelingClientRequestTunnel(EmberNodeId server,
                                                                             uint8_t clientEndpoint,
                                                                             uint8_t serverEndpoint,
                                                                             uint8_t protocolId,
                                                                             uint16_t manufacturerCode,
                                                                             bool flowControlSupport);

/**
 * @brief Transfers data to a server through a tunneling cluster tunnel.
 *
 * This function can be used to transfer data to a server through a tunnel. The
 * tunneling client plugin will send data to the endpoint on the node that
 * is managing the given tunnel.
 *
 * @param tunnelIndex The index of the tunnel through which to send data.
 * @param data The buffer containing the raw octets of data.
 * @param dataLen The length in octets of the data.
 * @return ::EMBER_ZCL_STATUS_SUCCESS Indicates if data was sent,
 * ::EMBER_ZCL_STATUS_FAILURE if an error occurred, or
 * ::EMBER_ZCL_STATUS_NOT_FOUND if the tunnel does not exist.
 */
EmberAfStatus emberAfPluginTunnelingClientTransferData(uint8_t tunnelIndex,
                                                       uint8_t *data,
                                                       uint16_t dataLen);

/**
 * @brief Closes a tunneling cluster tunnel.
 *
 * This function can be used to close a tunnel. The tunneling client plugin
 * will send the close command to the endpoint on the node that is managing the
 * given tunnel.
 *
 * @param tunnelIndex The index of the tunnel to close.
 * @return ::EMBER_ZCL_STATUS_SUCCESS Indicates if the close request was sent,
 * ::EMBER_ZCL_STATUS_FAILURE if an error occurred, or
 * ::EMBER_ZCL_STATUS_NOT_FOUND if the tunnel does not exist.
 */
EmberAfStatus emberAfPluginTunnelingClientCloseTunnel(uint8_t tunnelIndex);

/**
 * @brief Cleans up a tunneling cluster tunnel.
 *
 * This function can be used to clean up all states associated with a tunnel.
 * The tunneling client plugin will not send the close command.
 *
 * @param tunnelIndex The index of the tunnel to clean up.
 */
void emberAfPluginTunnelingClientCleanup(uint8_t tunnelIndex);

void emAfPluginTunnelingClientPrint(void);
