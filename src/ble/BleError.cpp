/*
 *
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
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
 *    @file
 *      This file contains functions for working with BLE Layer errors.
 */

#include <stddef.h>

#include <BleLayer/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include <BleLayer/BleLayer.h>
#include <BleLayer/BleError.h>

#include <Weave/Support/ErrorStr.h>

namespace nl {
namespace Ble {

/**
 * Register a text error formatter for BLE Layer errors.
 */
void RegisterBleLayerErrorFormatter(void)
{
    static ErrorFormatter sBleLayerErrorFormatter =
    {
        FormatBleLayerError,
        NULL
    };

    RegisterErrorFormatter(&sBleLayerErrorFormatter);
}

bool FormatBleLayerError(char * buf, uint16_t bufSize, int32_t err)
{
    const char * desc = NULL;

    if (err < BLE_ERROR_MIN || err > BLE_ERROR_MAX)
    {
        return false;
    }

#if !WEAVE_CONFIG_SHORT_ERROR_STR
    switch (err)
    {
    case BLE_ERROR_BAD_ARGS                                     : desc = "Bad arguments"; break;
    case BLE_ERROR_INCORRECT_STATE                              : desc = "Incorrect state"; break;
    case BLE_ERROR_NO_ENDPOINTS                                 : desc = "No more BLE endpoints"; break;
    case BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK              : desc = "No Weave over BLE connection received callback set"; break;
    case BLE_ERROR_CENTRAL_UNSUBSCRIBED                         : desc = "BLE central unsubscribed"; break;
    case BLE_ERROR_GATT_SUBSCRIBE_FAILED                        : desc = "GATT subscribe operation failed"; break;
    case BLE_ERROR_GATT_UNSUBSCRIBE_FAILED                      : desc = "GATT unsubscribe operation failed"; break;
    case BLE_ERROR_GATT_WRITE_FAILED                            : desc = "GATT write characteristic operation failed"; break;
    case BLE_ERROR_GATT_INDICATE_FAILED                         : desc = "GATT indicate characteristic operation failed"; break;
    case BLE_ERROR_NOT_IMPLEMENTED                              : desc = "Not implemented"; break;
    case BLE_ERROR_WOBLE_PROTOCOL_ABORT                         : desc = "BLE transport protocol fired abort"; break;
    case BLE_ERROR_REMOTE_DEVICE_DISCONNECTED                   : desc = "Remote device closed BLE connection"; break;
    case BLE_ERROR_APP_CLOSED_CONNECTION                        : desc = "Application closed BLE connection"; break;
    case BLE_ERROR_OUTBOUND_MESSAGE_TOO_BIG                     : desc = "Outbound message too big"; break;
    case BLE_ERROR_NOT_WEAVE_DEVICE                             : desc = "BLE device doesn't seem to support Weave"; break;
    case BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS               : desc = "Incompatible BLE transport protocol versions"; break;
    case BLE_ERROR_NO_MEMORY                                    : desc = "No memory"; break;
    case BLE_ERROR_MESSAGE_INCOMPLETE                           : desc = "Message incomplete"; break;
    case BLE_ERROR_INVALID_FRAGMENT_SIZE                        : desc = "Invalid fragment size"; break;
    case BLE_ERROR_START_TIMER_FAILED                           : desc = "Start timer failed"; break;
    case BLE_ERROR_CONNECT_TIMED_OUT                            : desc = "Connect handshake timed out"; break;
    case BLE_ERROR_RECEIVE_TIMED_OUT                            : desc = "Receive handshake timed out"; break;
    case BLE_ERROR_INVALID_MESSAGE                              : desc = "Invalid message"; break;
    case BLE_ERROR_FRAGMENT_ACK_TIMED_OUT                       : desc = "Message fragment acknowledgement timed out"; break;
    case BLE_ERROR_KEEP_ALIVE_TIMED_OUT                         : desc = "Keep-alive receipt timed out"; break;
    case BLE_ERRROR_NO_CONNECT_COMPLETE_CALLBACK                : desc = "Missing required callback"; break;
    case BLE_ERROR_INVALID_ACK                                  : desc = "Received invalid BLE transport protocol fragment acknowledgement"; break;
    case BLE_ERROR_REASSEMBLER_MISSING_DATA                     : desc = "BLE message reassembler did not receive enough data"; break;
    case BLE_ERROR_INVALID_BTP_HEADER_FLAGS                     : desc = "Received invalid BLE transport protocol header flags"; break;
    case BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER                  : desc = "Received invalid BLE transport protocol sequence number"; break;
    case BLE_ERROR_REASSEMBLER_INCORRECT_STATE                  : desc = "BLE message reassembler received packet in incorrect state"; break;
    case BLE_ERROR_RECEIVED_MESSAGE_TOO_BIG                     : desc = "Message received by BLE message reassembler was too large"; break;
    }
#endif // !WEAVE_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "Ble", err, desc);

    return true;
}

} /* namespace Ble */
} /* namespace nl */

#endif // CONFIG_NETWORK_LAYER_BLE
