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

#include <support/ErrorStr.h>

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

    if (err < INET_CONFIG_ERROR_MIN || err > INET_CONFIG_ERROR_MAX)
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (err)
    {
    case INET_ERROR_WRONG_ADDRESS_TYPE:
        desc = "Wrong address type";
        break;
    case CHIP_ERROR_CONNECTION_ABORTED:
        desc = "TCP connection aborted";
        break;
    case INET_ERROR_PEER_DISCONNECTED:
        desc = "Peer disconnected";
        break;
    case CHIP_ERROR_INCORRECT_STATE:
        desc = "Incorrect state";
        break;
    case CHIP_ERROR_MESSAGE_TOO_LONG:
        desc = "Message too long";
        break;
    case CHIP_ERROR_NO_CONNECTION_HANDLER:
        desc = "No TCP connection handler";
        break;
    case CHIP_ERROR_NO_MEMORY:
        desc = "No memory";
        break;
    case CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG:
        desc = "Outbound message truncated";
        break;
    case CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG:
        desc = "Inbound message too big";
        break;
    case INET_ERROR_HOST_NOT_FOUND:
        desc = "Host not found";
        break;
    case INET_ERROR_DNS_TRY_AGAIN:
        desc = "DNS try again";
        break;
    case INET_ERROR_DNS_NO_RECOVERY:
        desc = "DNS no recovery";
        break;
    case CHIP_ERROR_INVALID_ARGUMENT:
        desc = "Bad arguments";
        break;
    case INET_ERROR_WRONG_PROTOCOL_TYPE:
        desc = "Wrong protocol type";
        break;
    case INET_ERROR_UNKNOWN_INTERFACE:
        desc = "Unknown interface";
        break;
    case CHIP_ERROR_NOT_IMPLEMENTED:
        desc = "Not implemented";
        break;
    case INET_ERROR_ADDRESS_NOT_FOUND:
        desc = "Address not found";
        break;
    case INET_ERROR_HOST_NAME_TOO_LONG:
        desc = "Host name too long";
        break;
    case INET_ERROR_INVALID_HOST_NAME:
        desc = "Invalid host name";
        break;
    case CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE:
        desc = "Not supported";
        break;
    case CHIP_ERROR_ENDPOINT_POOL_FULL:
        desc = "No more TCP endpoints";
        break;
    case INET_ERROR_IDLE_TIMEOUT:
        desc = "Idle timeout";
        break;
    case CHIP_ERROR_UNEXPECTED_EVENT:
        desc = "Unexpected event";
        break;
    case INET_ERROR_INVALID_IPV6_PKT:
        desc = "Invalid IPv6 Packet";
        break;
    case INET_ERROR_INTERFACE_INIT_FAILURE:
        desc = "Failure to initialize interface";
        break;
    case INET_ERROR_TCP_USER_TIMEOUT:
        desc = "TCP User Timeout";
        break;
    case INET_ERROR_TCP_CONNECT_TIMEOUT:
        desc = "TCP Connect Timeout";
        break;
    case INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE:
        desc = "Incompatible IP address type";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "Inet", err, desc);

    return true;
}

} // namespace Inet
} // namespace chip
