/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#define _CHIP_BLE_BLE_H
#include "BleError.h"

#include <lib/core/ErrorStr.h>

namespace chip {
namespace Ble {

/**
 * Register a text error formatter for BLE Layer errors.
 */
void RegisterLayerErrorFormatter()
{
    static ErrorFormatter sBleLayerErrorFormatter = { FormatLayerError, nullptr };

    RegisterErrorFormatter(&sBleLayerErrorFormatter);
}

bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const char * desc = nullptr;

    if (!err.IsPart(ChipError::SdkPart::kBLE))
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (err.AsInteger())
    {
    case BLE_ERROR_ADAPTER_UNAVAILABLE.AsInteger():
        desc = "BLE adapter unavailable";
        break;
    case BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK.AsInteger():
        desc = "No chip over BLE connection received callback set";
        break;
    case BLE_ERROR_CENTRAL_UNSUBSCRIBED.AsInteger():
        desc = "BLE central unsubscribed";
        break;
    case BLE_ERROR_GATT_SUBSCRIBE_FAILED.AsInteger():
        desc = "GATT subscribe operation failed";
        break;
    case BLE_ERROR_GATT_UNSUBSCRIBE_FAILED.AsInteger():
        desc = "GATT unsubscribe operation failed";
        break;
    case BLE_ERROR_GATT_WRITE_FAILED.AsInteger():
        desc = "GATT write characteristic operation failed";
        break;
    case BLE_ERROR_GATT_INDICATE_FAILED.AsInteger():
        desc = "GATT indicate characteristic operation failed";
        break;
    case BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT.AsInteger():
        desc = "BLE transport protocol fired abort";
        break;
    case BLE_ERROR_REMOTE_DEVICE_DISCONNECTED.AsInteger():
        desc = "Remote device closed BLE connection";
        break;
    case BLE_ERROR_APP_CLOSED_CONNECTION.AsInteger():
        desc = "Application closed BLE connection";
        break;
    case BLE_ERROR_NOT_CHIP_DEVICE.AsInteger():
        desc = "BLE device doesn't seem to support chip";
        break;
    case BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS.AsInteger():
        desc = "Incompatible BLE transport protocol versions";
        break;
    case BLE_ERROR_INVALID_FRAGMENT_SIZE.AsInteger():
        desc = "Invalid fragment size";
        break;
    case BLE_ERROR_START_TIMER_FAILED.AsInteger():
        desc = "Start timer failed";
        break;
    case BLE_ERROR_CONNECT_TIMED_OUT.AsInteger():
        desc = "Connect handshake timed out";
        break;
    case BLE_ERROR_RECEIVE_TIMED_OUT.AsInteger():
        desc = "Receive handshake timed out";
        break;
    case BLE_ERROR_INVALID_MESSAGE.AsInteger():
        desc = "Invalid message";
        break;
    case BLE_ERROR_FRAGMENT_ACK_TIMED_OUT.AsInteger():
        desc = "Message fragment acknowledgement timed out";
        break;
    case BLE_ERROR_KEEP_ALIVE_TIMED_OUT.AsInteger():
        desc = "Keep-alive receipt timed out";
        break;
    case BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK.AsInteger():
        desc = "Missing required callback";
        break;
    case BLE_ERROR_INVALID_ACK.AsInteger():
        desc = "Received invalid BLE transport protocol fragment acknowledgement";
        break;
    case BLE_ERROR_REASSEMBLER_MISSING_DATA.AsInteger():
        desc = "BLE message reassembler did not receive enough data";
        break;
    case BLE_ERROR_INVALID_BTP_HEADER_FLAGS.AsInteger():
        desc = "Received invalid BLE transport protocol header flags";
        break;
    case BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER.AsInteger():
        desc = "Received invalid BLE transport protocol sequence number";
        break;
    case BLE_ERROR_REASSEMBLER_INCORRECT_STATE.AsInteger():
        desc = "BLE message reassembler received packet in incorrect state";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "Ble", err, desc);

    return true;
}

} /* namespace Ble */
} /* namespace chip */
