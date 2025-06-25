/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    Copyright (c) 2025 NXP LLC.
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
 *      This file contains functions for working with WiFiPAF Layer errors.
 */

#include "WiFiPAFError.h"

#include <lib/core/ErrorStr.h>

namespace chip {
namespace WiFiPAF {

/**
 * Register a text error formatter for WiFiPAF Layer errors.
 */
void RegisterLayerErrorFormatter()
{
    static ErrorFormatter sWiFiPAFLayerErrorFormatter = { FormatLayerError, nullptr };

    RegisterErrorFormatter(&sWiFiPAFLayerErrorFormatter);
}

bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const char * desc = nullptr;

    if (!err.IsPart(ChipError::SdkPart::kBLE))
    {
        return false;
    }

    switch (err.AsInteger())
    {
    case WIFIPAF_ERROR_NO_CONNECTION_RECEIVED_CALLBACK.AsInteger():
        desc = "No chip over WiFiPAF connection received callback set";
        break;
    case WIFIPAF_ERROR_CHIPPAF_PROTOCOL_ABORT.AsInteger():
        desc = "WiFiPAF transport protocol fired abort";
        break;
    case WIFIPAF_ERROR_REMOTE_DEVICE_DISCONNECTED.AsInteger():
        desc = "Remote device closed WiFiPAF connection";
        break;
    case WIFIPAF_ERROR_APP_CLOSED_CONNECTION.AsInteger():
        desc = "Application closed WiFiPAF connection";
        break;
    case WIFIPAF_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS.AsInteger():
        desc = "Incompatible WiFiPAF transport protocol versions";
        break;
    case WIFIPAF_ERROR_INVALID_FRAGMENT_SIZE.AsInteger():
        desc = "Invalid fragment size";
        break;
    case WIFIPAF_ERROR_START_TIMER_FAILED.AsInteger():
        desc = "Start timer failed";
        break;
    case WIFIPAF_ERROR_CONNECT_TIMED_OUT.AsInteger():
        desc = "Connect handshake timed out";
        break;
    case WIFIPAF_ERROR_RECEIVE_TIMED_OUT.AsInteger():
        desc = "Receive handshake timed out";
        break;
    case WIFIPAF_ERROR_INVALID_MESSAGE.AsInteger():
        desc = "Invalid message";
        break;
    case WIFIPAF_ERROR_FRAGMENT_ACK_TIMED_OUT.AsInteger():
        desc = "Message fragment acknowledgement timed out";
        break;
    case WIFIPAF_ERROR_KEEP_ALIVE_TIMED_OUT.AsInteger():
        desc = "Keep-alive receipt timed out";
        break;
    case WIFIPAF_ERROR_NO_CONNECT_COMPLETE_CALLBACK.AsInteger():
        desc = "Missing required callback";
        break;
    case WIFIPAF_ERROR_INVALID_ACK.AsInteger():
        desc = "Received invalid WiFiPAF transport protocol fragment acknowledgement";
        break;
    case WIFIPAF_ERROR_REASSEMBLER_MISSING_DATA.AsInteger():
        desc = "WiFiPAF message reassembler did not receive enough data";
        break;
    case WIFIPAF_ERROR_INVALID_PAFTP_HEADER_FLAGS.AsInteger():
        desc = "Received invalid WiFiPAF transport protocol header flags";
        break;
    case WIFIPAF_ERROR_INVALID_PAFTP_SEQUENCE_NUMBER.AsInteger():
        desc = "Received invalid WiFiPAF transport protocol sequence number";
        break;
    default:
        return false;
    }

    FormatError(buf, bufSize, "WiFiPAF", err, desc);

    return true;
}

} /* namespace WiFiPAF */
} /* namespace chip */
