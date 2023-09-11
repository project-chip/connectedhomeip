/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *  Shared state for socket implementations of TCPEndPoint and UDPEndPoint.
 */

#pragma once

#include <inet/EndPointBasis.h>

#include <inet/IPAddress.h>
#include <system/SocketEvents.h>

namespace chip {
namespace Inet {

/**
 * Definitions shared by all sockets-based EndPoint classes.
 */
class DLL_EXPORT EndPointStateSockets
{
protected:
    EndPointStateSockets() : mSocket(kInvalidSocketFd) {}

    static constexpr int kInvalidSocketFd = -1;
    int mSocket;                     /**< Encapsulated socket descriptor. */
    IPAddressType mAddrType;         /**< Protocol family, i.e. IPv4 or IPv6. */
    System::SocketWatchToken mWatch; /**< Socket event watcher */
};

} // namespace Inet
} // namespace chip
