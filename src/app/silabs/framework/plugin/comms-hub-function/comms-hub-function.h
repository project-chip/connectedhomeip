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
 * @brief APIs for the Comms Hub Function plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef COMMS_HUB_FUNCTION_H_INCLUDED
#define COMMS_HUB_FUNCTION_H_INCLUDED

// Printing macros for plugin: Comms Hub Function
#define emberAfPluginCommsHubFunctionPrint(...)    emberAfAppPrint(__VA_ARGS__)
#define emberAfPluginCommsHubFunctionPrintln(...)  emberAfAppPrintln(__VA_ARGS__)
#define emberAfPluginCommsHubFunctionDebugExec(x)  emberAfAppDebugExec(x)
#define emberAfPluginCommsHubFunctionPrintBuffer(buffer, len, withSpace) emberAfAppPrintBuffer(buffer, len, withSpace)

typedef enum {
  EMBER_AF_CHF_STATUS_SUCCESS                 = 0x00,
  EMBER_AF_CHF_STATUS_TOO_MANY_PEND_MESSAGES  = 0xFA,
  EMBER_AF_CHF_STATUS_FNF_ATTR_FAILURE        = 0xFB,
  EMBER_AF_CHF_STATUS_NO_MIRROR               = 0xFC,
  EMBER_AF_CHF_STATUS_TUNNEL_FAILURE          = 0xFD,
  EMBER_AF_CHF_STATUS_NO_ACCESS               = 0xFE,
  EMBER_AF_CHF_STATUS_SEND_TIMEOUT            = 0xFF,
} EmberAfPluginCommsHubFunctionStatus;

/**
 * @brief Passes a message to be tunneled over the HAN using either
 * a sleepy buffer system (GSME) or direct to the device
 * by initiating a tunnel (ESME, HCALCS, PPMID, TYPE2).
 *
 * This function is used to transfer data to a device on the HAN.
 *
 * @param destinationDeviceId The EUI64 of the destination device.
 * @param length The length in octets of the data.
 * @param payload The buffer (memory location at WAN Message Handler) containing the
 *  raw octets of the message (GBCS Message)
 * @param messageCode The GBCS Message Code for the data that is being sent.
 * @return
 * ::EMBER_AF_CHF_STATUS_SUCCESS data was sent or has been queue to be sent.
 * ::EMBER_AF_CHF_STATUS_NO_ACCESS No entry in the GBCS Device Log for the specified device.
 * ::EMBER_AF_CHF_STATUS_NO_MIRROR Mirror endpoint for given device has not been configured.
 * ::EMBER_AF_CHF_STATUS_FNF_ATTR_FAILURE Unable to read or write the functional notification flags attribute.
 * ::EMBER_AF_CHF_STATUS_TOO_MANY_PEND_MESSAGES There are too many messages currently pending to be delivered.
 * ::EMBER_AF_CHF_STATUS_TUNNEL_FAILURE tunnel cannot be created to non sleepy devices.
 */
EmberAfPluginCommsHubFunctionStatus emberAfPluginCommsHubFunctionSend(EmberEUI64 destinationDeviceId,
                                                                      uint16_t length,
                                                                      uint8_t *payload,
                                                                      uint16_t messageCode);

/**
 * @brief Accepts a tunnel.
 *
 * This callback is called by the tunnel manager when a tunnel is requested. The
 * given device identifier should be checked against the Device Log to verify
 * whether tunnels from the device should be accepted or not.
 *
 * @param deviceId An identifier of the device from which a tunnel is requested
 * @return True if the tunnel should be allowed, false otherwise.
 */
bool emAfPluginCommsHubFunctionTunnelAcceptCallback(EmberEUI64 deviceId);

/**
 * @brief Notifies tunnel data was received.
 *
 * This callback is called by the tunnel manager when data is received over a tunnel.
 * It is responsible for the implementation of the GET, GET_RESPONSE, PUT
 * protocol used when communicating with a sleepy device.
 *
 * @param senderDeviceId The identifier of the device from which the data was received.
 * @param length The length of the data received
 * @param payload Data received.
 */
void emAfPluginCommsHubFunctionTunnelDataReceivedCallback(EmberEUI64 senderDeviceId,
                                                          uint16_t length,
                                                          uint8_t *payload);

/**
 * @brief Sets the default remote part message timeout.
 *
 * This function is used to set the default timeout for all messages
 * destined for a sleepy device. If the device does not retrieve the message
 * before this time, it will be discarded and a ::EMBER_AF_CHF_STATUS_SEND_TIMEOUT
 * error will be return in emberAfPluginCommsHubFunctionSendCallback().
 *
 * @param timeout Timeout in seconds.
 */
void emAfPluginCommsHubFunctionSetDefaultTimeout(uint32_t timeout);

/**
 * @brief Update Functional Notification Flags for the given device
 *
 * @param deviceId Identifier of the device for which the notification flags are
 *  to be updated
 * @param resetMask Each flag to be reset has the corresponding bit set to 0,
 *  all other flags have the corresponding bit set to 1
 * @param setMask Each flag to be set has the corresponding bit set to 1,
 *  all other flags have the corresponding bit set to 0
 */
EmberAfPluginCommsHubFunctionStatus emAfUpdateFunctionalNotificationFlagsByEui64(EmberEUI64 deviceId,
                                                                                 uint32_t resetMask,
                                                                                 uint32_t setMask);

/**
 * @brief Updates Functional Notification Flags on the given endpoint.
 *
 * @param endpoint An endpoint on which the notification flags are to be updated.
 * @param resetMask Each flag to be reset has the corresponding bit set to 0,
 *  all other flags have the corresponding bit set to 1.
 * @param setMask Each flag to be set has the corresponding bit set to 1,
 *  all other flags have the corresponding bit set to 0.
 */
EmberAfPluginCommsHubFunctionStatus emAfUpdateFunctionalNotificationFlagsByEndpoint(uint8_t endpoint,
                                                                                    uint32_t resetMask,
                                                                                    uint32_t setMask);
#endif // COMMS_HUB_FUNCTION_SUB_GHZ_H_INCLUDED
