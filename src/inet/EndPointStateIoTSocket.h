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

#include <stdint.h>

namespace chip {
namespace Inet {

/**
 * Definitions shared by all IoT Socket based EndPoint classes.
 */
class EndPointStateIoTSocket
{
public:
    virtual void SelectCallback(void * readMask, void * writeMask, void * exceptionMask){};
    int32_t GetSocketId() { return mSocket; };

    static constexpr int32_t kInvalidSocketFd = -1;

protected:
    EndPointStateIoTSocket() : mSocket(kInvalidSocketFd){};
    virtual ~EndPointStateIoTSocket(){};

    int32_t mSocket;       /**< Encapsulated socket descriptor. */
    int32_t mSocketFamily; /**< Protocol family, i.e. IPv4 or IPv6. */
    uint16_t mBoundPort;
};

} // namespace Inet
} // namespace chip
