/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>

namespace chip {
namespace Inet {

/**
 * Information about an incoming/outgoing message/connection.
 *
 * @warning
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

    /**
     *  Reset the members of the IPPacketInfo object.
     */
    void Clear();
};

} // namespace Inet
} // namespace chip
