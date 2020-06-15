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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include <stdlib.h>     // malloc, free
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully sent
 * by the concentrator plugin.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
{
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned in
 * the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Get Radio Power For Channel
 *
 * This callback is called by the framework when it is setting the radio power
 * during the discovery process. The framework will set the radio power
 * depending on what is returned by this callback.
 *
 * @param channel   Ver.: always
 */
int8_t emberAfPluginNetworkFindGetRadioPowerForChannelCallback(uint8_t channel)
{
  return EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER;
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been found.
 *  If the application returns true, the plugin will attempt to join the
 * network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork * networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Received
 *
 * This callback is called by the Comms Hub Function (CHF) plugin whenever a
 * message is received.
 *
 * @param senderDeviceId The EUI64 of the sending device  Ver.: always
 * @param dataLen The length in octets of the data  Ver.: always
 * @param data Buffer containing the raw octets of the data  Ver.: always
 */
void emberAfPluginCommsHubFunctionReceivedCallback(EmberEUI64 senderDeviceId,
                                                   uint16_t dataLen,
                                                   uint8_t * data)
{
  emberAfAppPrint("GBCS emberAfPluginCommsHubFunctionDataReceivedCallback: senderDeviceId=");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(senderDeviceId));
  emberAfAppPrint(", dataLen=0x%2x, data=0x", dataLen);
  emberAfAppPrintBuffer(data, dataLen, false);
  emberAfAppPrintln("");
}

/** @brief Send
 *
 * This callback is called by the Comms Hub Function (CHF) plugin to report the
 * status of a message that was previously sent.
 *
 * @param status The status of the message that was sent  Ver.: always
 * @param destinationDeviceId The EUI64 of the destination device to which the
 * data was sent  Ver.: always
 * @param dataLen The length in octets of the data that was sent  Ver.: always
 * @param data Buffer containing the raw octets of the data that was sent  Ver.:
 * always
 */
void emberAfPluginCommsHubFunctionSendCallback(uint8_t status,
                                               EmberEUI64 destinationDeviceId,
                                               uint16_t dataLen,
                                               uint8_t * data)
{
  emberAfAppPrint("GBCS emberAfPluginCommsHubFunctionSendCallback: status=0x%x, destinationDeviceId=", status);
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(destinationDeviceId));
  emberAfAppPrintln(" dataLen=0x%2x, data=0x%4x", dataLen, data);
}

/** @brief Encrypt Data
 *
 * This function is called by the Gbz Message Controller plugin to encrypt a ZCL
 * payload. If the encryption is successful, the decrypted data is expected to
 * overwrite the sources. The cipheredInfoLength will be overwritten with the
 * plaintext's length and the cipheredInfo will be overwritten with the
 * plaintext. If the decryption fails, the callback return false and should not
 * modify the source data.
 *
 * @param data   Ver.: always
 */
void emberAfPluginGbzMessageControllerEncryptDataCallback(EmberAfGbzMessageData *data)
{
  uint8_t * moo = (uint8_t *) malloc(sizeof(uint8_t) * data->plainPayloadLength + 2);
  MEMCOPY(moo, data->plainPayload, data->plainPayloadLength);
  moo[data->plainPayloadLength] = 0xDE;
  moo[data->plainPayloadLength + 1] = 0xAD;
  data->encryption = true;
  data->encryptedPayload = moo;
  data->encryptedPayloadLength = data->plainPayloadLength + 2;
}

/** @brief Log Data Updated
 *
 * This function is called by the Events server plugin when any modification to
 * the Event logs have been made. The argument will reflect the zcl command that
 * triggered the Event server's data change. If null, it means the logging data
 * have been updated through other means, e.g. CLI.
 *
 * @param cmd ZCL command  Ver.: always
 */
void emberAfPluginEventsServerLogDataUpdatedCallback(const EmberAfClusterCommand *cmd)
{
  emberAfAppPrintln("emberAfPluginEventsServerLogDataUpdatedCallback: cmdAddr=0x%4X", cmd);
}

/** @brief Data Log Access Request
 *
 * Query the application regarding the processing of the current GBCS specific
 * command.
      Depending on the vendor specific information, such as the
 * Tenancy attribute in the Device Management Cluster,
      the callback will
 * determine if the current command shall be processed or not. By returning a
 * value of true,
      the plugin will further process the message. By
 * returning a value of false, the plugin will reject the message and no
 * processing will be done.
 *
 * @param gpfMessage   Ver.: always
 * @param zclClusterCommand   Ver.: always
 */
bool emberAfPluginGasProxyFunctionDataLogAccessRequestCallback(const EmberAfGpfMessage *  gpfMessage,
                                                               const EmberAfClusterCommand *  zclClusterCommand)
{
  // allow access at at all times.
  return true;
}

/** @brief Ncp Init
 *
 * This function is called when the network coprocessor is being initialized,
 * either at startup or upon reset.  It provides applications on opportunity to
 * perform additional configuration of the NCP.  The function is always called
 * twice when the NCP is initialized.  In the first invocation, memoryAllocation
 * will be true and the application should only issue EZSP commands that affect
 * memory allocation on the NCP.  For example, tables on the NCP can be resized
 * in the first call.  In the second invocation, memoryAllocation will be false
 * and the application should only issue EZSP commands that do not affect memory
 * allocation.  For example, tables on the NCP can be populated in the second
 * call.  This callback is not called on SoCs.
 *
 * @param memoryAllocation   Ver.: always
 */
void emberAfNcpInitCallback(boolean memoryAllocation)
{
#ifdef EZSP_HOST
  if (memoryAllocation) {
    emberAfSetEzspConfigValue(EZSP_CONFIG_RETRY_QUEUE_SIZE,
                              EMBER_RETRY_QUEUE_SIZE,
                              "retry size");
  }
#endif  // EZSP_HOST
}
