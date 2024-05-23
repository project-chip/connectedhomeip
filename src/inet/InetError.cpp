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
 *      This file contains functions for working with Inet Layer errors.
 */

#include <stddef.h>

#include <inet/Inet.h>
#include <inet/InetError.h>

extern void FormatError(char * buf, uint16_t bufSize, const char * subsys, int32_t err, const char * desc);

namespace chip {
namespace Inet {

/**
 * Register a text error formatter for Inet Layer errors.
 */
void RegisterLayerErrorFormatter()
{
    static chip::ErrorFormatter sInetLayerErrorFormatter = { FormatLayerError, nullptr };

    RegisterErrorFormatter(&sInetLayerErrorFormatter);
}

/**
 * Given an Inet Layer error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not an Inet Layer error.
 *
 */
bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const char * desc = nullptr;

    if (!err.IsPart(ChipError::SdkPart::kInet))
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (err.AsInteger())
    {
    case INET_ERROR_WRONG_ADDRESS_TYPE.AsInteger():
        desc = "Wrong address type";
        break;
    case CHIP_ERROR_CONNECTION_ABORTED.AsInteger():
        desc = "TCP connection aborted";
        break;
    case INET_ERROR_PEER_DISCONNECTED.AsInteger():
        desc = "Peer disconnected";
        break;
    case CHIP_ERROR_INCORRECT_STATE.AsInteger():
        desc = "Incorrect state";
        break;
    case CHIP_ERROR_MESSAGE_TOO_LONG.AsInteger():
        desc = "Message too long";
        break;
    case CHIP_ERROR_NO_CONNECTION_HANDLER.AsInteger():
        desc = "No TCP connection handler";
        break;
    case CHIP_ERROR_NO_MEMORY.AsInteger():
        desc = "No memory";
        break;
    case CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG.AsInteger():
        desc = "Outbound message truncated";
        break;
    case CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG.AsInteger():
        desc = "Inbound message too big";
        break;
    case INET_ERROR_HOST_NOT_FOUND.AsInteger():
        desc = "Host not found";
        break;
    case INET_ERROR_DNS_TRY_AGAIN.AsInteger():
        desc = "DNS try again";
        break;
    case INET_ERROR_DNS_NO_RECOVERY.AsInteger():
        desc = "DNS no recovery";
        break;
    case CHIP_ERROR_INVALID_ARGUMENT.AsInteger():
        desc = "Bad arguments";
        break;
    case INET_ERROR_WRONG_PROTOCOL_TYPE.AsInteger():
        desc = "Wrong protocol type";
        break;
    case INET_ERROR_UNKNOWN_INTERFACE.AsInteger():
        desc = "Unknown interface";
        break;
    case CHIP_ERROR_NOT_IMPLEMENTED.AsInteger():
        desc = "Not implemented";
        break;
    case INET_ERROR_ADDRESS_NOT_FOUND.AsInteger():
        desc = "Address not found";
        break;
    case INET_ERROR_HOST_NAME_TOO_LONG.AsInteger():
        desc = "Host name too long";
        break;
    case INET_ERROR_INVALID_HOST_NAME.AsInteger():
        desc = "Invalid host name";
        break;
    case CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.AsInteger():
        desc = "Not supported";
        break;
    case CHIP_ERROR_ENDPOINT_POOL_FULL.AsInteger():
        desc = "No more TCP endpoints";
        break;
    case INET_ERROR_IDLE_TIMEOUT.AsInteger():
        desc = "Idle timeout";
        break;
    case CHIP_ERROR_UNEXPECTED_EVENT.AsInteger():
        desc = "Unexpected event";
        break;
    case INET_ERROR_INVALID_IPV6_PKT.AsInteger():
        desc = "Invalid IPv6 Packet";
        break;
    case INET_ERROR_INTERFACE_INIT_FAILURE.AsInteger():
        desc = "Failure to initialize interface";
        break;
    case INET_ERROR_TCP_USER_TIMEOUT.AsInteger():
        desc = "TCP User Timeout";
        break;
    case INET_ERROR_TCP_CONNECT_TIMEOUT.AsInteger():
        desc = "TCP Connect Timeout";
        break;
    case INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE.AsInteger():
        desc = "Incompatible IP address type";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "Inet", err, desc);

    return true;
}

} // namespace Inet
} // namespace chip
