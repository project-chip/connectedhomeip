/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <inet/InetConfig.h>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>

namespace chip {
namespace Inet {

/**
 *  @class IPPacketInfo
 *
 *  @brief
 *     Information about an incoming/outgoing message/connection.
 *
 *   @warning
 *     Do not alter the contents of this class without first reading and understanding
 *     the code/comments in UDPEndPoint::GetPacketInfo().
 */
class IPPacketInfo
{
public:
    IPAddress SrcAddress;  /**< The source IPAddress in the packet. */
    IPAddress DestAddress; /**< The destination IPAddress in the packet. */
    InterfaceId Interface; /**< The interface identifier for the connection. */
    uint16_t SrcPort;      /**< The source port in the packet. */
    uint16_t DestPort;     /**< The destination port in the packet. */

    void Clear();
};

static_assert(std::is_standard_layout<IPPacketInfo>::value, "IPPacketInfo must be standard layout, because its bytes represents is used in LwIP");

} // namespace Inet
} // namespace chip
