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

#include <netsocket/TCPSocket.h>
#include <netsocket/UDPSocket.h>
// FIXME
//#include <mstd_atomic>
#include "OpenFileHandleAsFileDescriptor.h"
#include "common.h"
#include <atomic>
#include <platform/FileHandle.h>
#include <sys/socket.h>

namespace mbed {

struct BSDSocket : public FileHandle
{
    using flags_type = uint32_t;
    enum
    {
        MBED_TCP_SOCKET = SOCK_STREAM,
        MBED_UDP_SOCKET = SOCK_DGRAM
    };
    BSDSocket(){};
    ~BSDSocket(){};

    int open(int type)
    {
        InternetSocket * socket;
        switch (type)
        {
        case MBED_TCP_SOCKET: {
            socket = new (&tcpSocket) TCPSocket();
        }
        break;
        case MBED_UDP_SOCKET: {
            socket = new (&udpSocket) UDPSocket();
        }
        break;
        default:
            set_errno(ESOCKTNOSUPPORT);
            return -1;
        };

        if (socket->open(NetworkInterface::get_default_instance()) != NSAPI_ERROR_OK)
        {
            close();
            set_errno(ENOBUFS);
            return -1;
        }

        _type = type;

        _flags.store(0);
        socket->sigio([&]() {
            auto current = _flags.load();
            if (current & POLLOUT)
            {
                current &= ~POLLOUT;
            }

            if (!(current & POLLIN))
            {
                current |= POLLIN;
            }
            _flags.store(current);
            if (_callback)
            {
                _callback();
            }
        });

        socket->set_blocking(false);

        _fd = bind_to_fd(this);
        if (_fd < 0)
        {
            close();
            set_errno(ENFILE);
            return -1;
        }

        return _fd;
    }

    int close() override
    {
        switch (_type)
        {
        case MBED_TCP_SOCKET:
            tcpSocket.~TCPSocket();
            break;
        case MBED_UDP_SOCKET:
            udpSocket.~UDPSocket();
            break;
        }

        _fd       = -1;
        _callback = nullptr;
        _flags.store(0);
        if (socketName)
        {
            socketName.set_ip_bytes(nullptr, NSAPI_UNSPEC);
        }
    }

    ssize_t read(void * buffer, size_t size) override
    {
        while (true)
        {
            auto current = _flags.load();
            auto success = _flags.compare_exchange_weak(current, (current & ~POLLIN));
            if (success)
            {
                break;
            }
        }

        return 0;
    }

    ssize_t write(const void * buffer, size_t size) override
    {
        while (true)
        {
            auto current = _flags.load();
            auto success = _flags.compare_exchange_weak(current, (current | POLLOUT));
            if (success)
            {
                break;
            }
        }

        return 0;
    }

    off_t seek(off_t offset, int whence = SEEK_SET) override { return -ESPIPE; }

    int set_blocking(bool blocking) override
    {
        if (blocking)
        {
            return -EINVAL;
        }
        return 0;
    }

    bool is_blocking() const override { return false; }

    short poll(short events) const override
    {
        auto state = _flags.load();

        return (state & events);
    }

    void sigio(Callback<void()> func) override
    {
        _callback = func;
        if (_callback && poll(POLLIN))
        {
            _callback();
        }
    }

    bool isSocketOpen() { return _fd >= 0; }

    Socket * getNetSocket()
    {
        Socket * ret = nullptr;
        switch (_type)
        {
        case MBED_TCP_SOCKET:
            ret = &tcpSocket;
            break;
        case MBED_UDP_SOCKET:
            ret = &udpSocket;
            break;
        }
        return ret;
    }

    SocketAddress socketName;

private:
    union
    {

        TCPSocket tcpSocket;
        UDPSocket udpSocket;
    };
    int _fd = -1;
    int _type;
    Callback<void()> _callback = nullptr;
    // FIXME
    // mstd::atomic<counter_type> _flags = { 0 };
    std::atomic<flags_type> _flags = { 0 };
};

} // namespace mbed

#endif // MBED_NET_BSD_SOCKET_H