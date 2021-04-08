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
#include "mbed-trace/mbed_trace.h"
#include <atomic>
#include <mstd_atomic>
#include <netsocket/TCPSocket.h>
#include <netsocket/UDPSocket.h>

#define TRACE_GROUP "BSDS"
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

    int open(int type, InternetSocket * socket = nullptr)
    {
        if (socket != nullptr)
        {
            _socket = socket;
        }
        else
        {
            switch (type)
            {
            case MBED_TCP_SOCKET: {
                _socket = new TCPSocket();
            }
            break;
            case MBED_UDP_SOCKET: {
                _socket = new UDPSocket();
            }
            break;
            default:
                tr_err("Socket type not supported");
                set_errno(ESOCKTNOSUPPORT);
                return -1;
            };

            if (_socket->open(NetworkInterface::get_default_instance()) != NSAPI_ERROR_OK)
            {
                close();
                tr_err("Open socket failed");
                set_errno(ENOBUFS);
                return -1;
            }
        }

        _type = type;

        _flags.store(0);
        _socket->sigio([&]() {
            tr_debug("Socket %d event", _fd);
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

        _socket->set_blocking(true);

        _fd = bind_to_fd(this);
        if (_fd < 0)
        {
            close();
            tr_err("Bind socket to fd failed");
            set_errno(ENFILE);
            return -1;
        }

        tr_info("Open %s socket with fd %d", type == MBED_TCP_SOCKET ? "TCP" : "UDP", _fd);

        return _fd;
    }

    int close() override
    {
        delete _socket;
        _socket = nullptr;

        tr_info("Close %s socket fd %d", _type == MBED_TCP_SOCKET ? "TCP" : "UDP", _fd);

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
        tr_info("Read from socket fd %d", _fd);
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
        tr_info("Write to socket fd %d", _fd);
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
        tr_info("Set socket fd %d blocking: %s", _fd, blocking ? "true" : "false");
        _blocking = blocking;
        if (_socket != nullptr)
        {
            _socket->set_blocking(_blocking);
        }

        return 0;
    }

    bool is_blocking() const override { return _blocking; }

    int enable_input(bool enabled) override
    {
        tr_info("Set socket fd %d input enable: %s\n", _fd, enabled ? "true" : "false");
        _inputEnable = enabled;
        return 0;
    }

    bool is_input_enable() { return _inputEnable; }

    int enable_output(bool enabled) override
    {
        tr_info("Set socket fd %d output enable: %s", _fd, enabled ? "true" : "false");
        _outputEnable = enabled;
        return 0;
    }

    bool is_output_enable() { return _outputEnable; }

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

    InternetSocket * getNetSocket() { return _socket; }

    int getSocketType() { return _type; }

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
};

} // namespace mbed

#endif // MBED_NET_BSD_SOCKET_H