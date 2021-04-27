/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_NET_BSD_SOCKET_H
#define MBED_NET_BSD_SOCKET_H

#include "OpenFileHandleAsFileDescriptor.h"
#include "common.h"
#include <mstd_atomic>
#include <netsocket/TCPSocket.h>
#include <netsocket/UDPSocket.h>

namespace mbed {

struct BSDSocket : public FileHandle
{
    using flags_type = uint32_t;
    enum
    {
        MBED_TCP_SOCKET = SOCK_STREAM,
        MBED_UDP_SOCKET = SOCK_DGRAM
    };

    enum
    {
        MBED_IPV4_SOCKET = AF_INET,
        MBED_IPV6_SOCKET = AF_INET6
    };
    BSDSocket(){};
    ~BSDSocket(){};

    int open(int family, int type, InternetSocket * socket = nullptr);

    int close() override;
    ssize_t read(void * buffer, size_t size) override;
    ssize_t write(const void * buffer, size_t size) override;
    off_t seek(off_t offset, int whence = SEEK_SET) override;
    int set_blocking(bool blocking) override;
    bool is_blocking() const override;
    int enable_input(bool enabled) override;
    bool is_input_enable();
    int enable_output(bool enabled) override;
    bool is_output_enable();
    short poll(short events) const override;
    void sigio(Callback<void()> func) override;
    bool isSocketOpen();
    InternetSocket * getNetSocket();
    int getSocketType();

    SocketAddress socketName;

private:
    InternetSocket * _socket = nullptr;
    int _fd                  = -1;
    int _type;
    Callback<void()> _callback      = nullptr;
    mstd::atomic<flags_type> _flags = { 0 };
    bool _blocking                  = true;
    bool _inputEnable               = true;
    bool _outputEnable              = true;
    bool _factory_allocated         = false;
};

} // namespace mbed

#endif // MBED_NET_BSD_SOCKET_H
