/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file enumerates and defines the different types of events
 *      generated at the Inet layer.
 *
 */

#ifndef INETLAYEREVENTS_H
#define INETLAYEREVENTS_H

#include <inet/InetConfig.h>

#include <system/SystemEvent.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace Inet {

/**
 *   The Inet layer event type definitions.
 *
 */
enum
{
    kInetEvent_TCPConnectComplete    = _INET_CONFIG_EVENT(0), /**< The event for TCP connection completion */
    kInetEvent_TCPConnectionReceived = _INET_CONFIG_EVENT(1), /**< The event for TCP connection reception */
    kInetEvent_TCPDataReceived       = _INET_CONFIG_EVENT(2), /**< The event for data reception over a TCP connection */
    kInetEvent_TCPDataSent           = _INET_CONFIG_EVENT(3), /**< The event for data transmission over a TCP connection */
    kInetEvent_TCPError              = _INET_CONFIG_EVENT(4), /**< The event for an error on a TCP connection */
    kInetEvent_UDPDataReceived       = _INET_CONFIG_EVENT(5), /**< The event for data reception over UDP */
    kInetEvent_DNSResolveComplete    = _INET_CONFIG_EVENT(6), /**< The event for DNS name resolution completion */
    kInetEvent_RawDataReceived       = _INET_CONFIG_EVENT(7)  /**< The event for data reception over an InetLayer raw endpoint */
};

/**
 *  Check to verify if a System::EventType is a valid Inet layer event type.
 *
 *  @param[in]  aType  A chip System Layer event type.
 *
 *  @return true if it falls within the enumerated range; otherwise, false.
 *
 */
static inline bool INET_IsInetEvent(chip::System::EventType aType)
{
    return (aType >= kInetEvent_TCPConnectComplete && aType <= kInetEvent_RawDataReceived);
}

} // namespace Inet
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
#endif // !defined(INETLAYEREVENTS_H)
