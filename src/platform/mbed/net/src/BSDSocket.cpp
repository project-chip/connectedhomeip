#include "BSDSocket.h"
#include "mbed-trace/mbed_trace.h"

#define TRACE_GROUP "BSDS"

namespace mbed {

int BSDSocket::open(int family, int type, InternetSocket * socket)
{
    nsapi_error_t err;
    SocketAddress addr;

    // switch (family)
    // {
    // case MBED_IPV4_SOCKET: {
    //     err = NetworkInterface::get_default_instance()->get_ip_address(&addr);
    //     if ((err != NSAPI_ERROR_OK) || (addr.get_ip_version() != NSAPI_IPv4))
    //     {
    //         tr_err("IPv4 not supported");
    //         set_errno(ESOCKTNOSUPPORT);
    //         return -1;
    //     }
    // }
    // break;
    // case MBED_IPV6_SOCKET: {
    //     err = NetworkInterface::get_default_instance()->get_ipv6_link_local_address(&addr);
    //     if ((err != NSAPI_ERROR_OK) || (addr.get_ip_version() != NSAPI_IPv6))
    //     {
    //         tr_err("IPv6 not supported");
    //         set_errno(ESOCKTNOSUPPORT);
    //         return -1;
    //     }
    // }
    // break;
    // default:
    //     tr_err("Socket family not supported");
    //     set_errno(ESOCKTNOSUPPORT);
    //     return -1;
    // };

    if (socket != nullptr)
    {
        _socket            = socket;
        _factory_allocated = true;
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

int BSDSocket::close()
{
    if (_factory_allocated)
    {
        _socket->close();
    }
    else
    {
        delete _socket;
    }

    _socket = nullptr;

    tr_info("Close %s socket fd %d", _type == MBED_TCP_SOCKET ? "TCP" : "UDP", _fd);

    _fd                = -1;
    _callback          = nullptr;
    _factory_allocated = false;
    _flags.store(0);
    if (socketName)
    {
        socketName.set_ip_bytes(nullptr, NSAPI_UNSPEC);
    }
    return 0;
}

ssize_t BSDSocket::read(void * buffer, size_t size)
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

ssize_t BSDSocket::write(const void * buffer, size_t size)
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

off_t BSDSocket::seek(off_t offset, int whence)
{
    return -ESPIPE;
}

int BSDSocket::set_blocking(bool blocking)
{
    tr_info("Set socket fd %d blocking: %s", _fd, blocking ? "true" : "false");
    _blocking = blocking;
    if (_socket != nullptr)
    {
        _socket->set_blocking(_blocking);
    }

    return 0;
}

bool BSDSocket::is_blocking() const
{
    return _blocking;
}

int BSDSocket::enable_input(bool enabled)
{
    tr_info("Set socket fd %d input enable: %s\n", _fd, enabled ? "true" : "false");
    _inputEnable = enabled;
    return 0;
}

bool BSDSocket::is_input_enable()
{
    return _inputEnable;
}

int BSDSocket::enable_output(bool enabled)
{
    tr_info("Set socket fd %d output enable: %s", _fd, enabled ? "true" : "false");
    _outputEnable = enabled;
    return 0;
}

bool BSDSocket::is_output_enable()
{
    return _outputEnable;
}

short BSDSocket::poll(short events) const
{
    auto state = _flags.load();
    return (state & events);
}

void BSDSocket::sigio(Callback<void()> func)
{
    _callback = func;
    if (_callback && poll(POLLIN))
    {
        _callback();
    }
}

bool BSDSocket::isSocketOpen()
{
    return _fd >= 0;
}

InternetSocket * BSDSocket::getNetSocket()
{
    return _socket;
}

int BSDSocket::getSocketType()
{
    return _type;
}

} // namespace mbed