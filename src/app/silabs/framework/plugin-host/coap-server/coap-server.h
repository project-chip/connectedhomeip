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
 * @brief Definitions for the COAP Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_COAP_SERVER_H
#define SILABS_COAP_SERVER_H

#include "include/coap/coap.h"

/** @brief Sends a message using COAP.
 *
 * Sends an unsolicited messages to the server.
 *
 * @param *uri A pointer to the URI string.
 *
 * @param *payload A pointer to the payload (if any).  NULL indicates no payload.
 *
 * @param length A number of bytes in the payload.  0 indicates no payload.
 *
 * @param device A number of the device from which the message was received.
 *
 * @param method.  A COAP method to use to send the message.
 *
 */
void emberAfPluginCoapServerSendMessage(uint8_t *uri,
                                        uint8_t *payload,
                                        uint16_t length,
                                        uint16_t device,
                                        uint8_t method);

/** @brief Creates a listener port based on the device table index.
 *
 * When a new device joins the gateway, it will be placed into the device table
 * at a unique index point. This method will open up the appropriate COAP
 * port on which to listen for traffic.
 *
 * @param deviceIndex The index of the device to create.
 *
 */
void emberAfPluginCoapServerCreateDevice(uint16_t deviceIndex);

/** @brief Removes a listener port based on the device table index.
 *
 * When a device is removed from the device table, it's necessary to close the
 * corresponding COAP port.
 *
 * @param deviceIndex The index of the device to remove.
 *
 */
void emberAfPluginCoapServerRemoveDevice(uint16_t deviceIndex);

/** @brief Copies the response string into the data in pointer.
 *
 * Gateway relay COAP plugin may generate a response string. If so, the COAP
 * server will need to obtain it using this API.
 *
 * @param *serverString A string of the IP address of the server.
 *
 * @param serverPort A port of the server to which responses will be sent.
 *
 */
void emberAfPluginCoapServerSetServerNameAndPort(uint8_t *serverString,
                                                 uint16_t serverPort);

/** @brief Defines the maximum number of devices that can connect. Note:
 * this will be specific to the implementation hardware.
 */
 #define PLUGIN_COAP_SERVER_MAX_PORTS 20

/** @brief COAP response code for message received and understood.
 */
#define COAP_RESPONSE_OK                  200

/** @brief COAP response code for resource created.
 */
#define COAP_RESPONSE_CREATED             201

/** @brief COAP response code for message was valid.
 */
#define COAP_RESPONSE_VALID               203

/** @brief COAP response code stating response message has data.
 */
#define COAP_RESPONSE_DATA                205

/** @brief COAP response code for a bad request.
 */
#define COAP_RESPONSE_BAD_REQUEST         400

/** @brief COAP response code stating the resource was not found.
 */
#define COAP_RESPONSE_NOT_FOUND           404

/** @brief COAP response code stating resource can not be accessed with this
 *  method
 */
#define COAP_RESPONSE_METHOD_NOT_ALLOWED  405

/** @brief COAP response code for an internal server error.
 */
#define COAP_RESPONSE_INTERNAL_ERROR      500

/** @brief COAP response code stating the resource was not yet implemented.
 */
#define COAP_RESPONSE_NOT_IMPLEMENTED     501

/** @brief COAP response code for unavailable service.
 */
#define COAP_RESPONSE_UNAVAILABLE         503

/** @brief COAP response code for gateway timeout error.
 */
#define COAP_RESPONSE_TIMEOUT             504

#endif
