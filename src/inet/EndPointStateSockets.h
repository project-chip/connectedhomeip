/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
