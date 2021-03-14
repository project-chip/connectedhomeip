#include "common.h"
#include <mbed_retarget.h>
#include <net_socket.h>
#include <rtos/EventFlags.h>

using namespace mbed;
using namespace rtos;

#define TCP_SOCKET SOCK_STREAM
#define UDP_SOCKET SOCK_DGRAM
#define SOCKET_NOT_INITIALIZED (0)
#define NO_FREE_SOCKET_SLOT (-1)

static BSDSocket sockets[MBED_NET_SOCKET_MAX_NUMBER];

ssize_t BSDSocket::read(void *, size_t)
{
    return 1;
}

ssize_t BSDSocket::write(const void *, size_t)
{
    return 1;
}

off_t BSDSocket::seek(off_t offset, int whence)
{
    return -1;
}

int BSDSocket::close()
{
    switch (type)
    {
    case TCP_SOCKET:
        tcpSocket.~TCPSocket();
    case UDP_SOCKET:
        udpSocket.~UDPSocket();
    }

    type = SOCKET_NOT_INITIALIZED;
    _cb  = nullptr;
    fd   = SOCKET_NOT_INITIALIZED;
}

int BSDSocket::set_blocking(bool blocking)
{
    if (blocking)
    {
        return -EINVAL;
    }
    return 0;
}

bool BSDSocket::is_blocking() const
{
    return false;
}

short BSDSocket::poll(short events) const
{
    return POLLIN | POLLOUT;
}

void BSDSocket::sigio(Callback<void()> func)
{
    _cb = func;
}

nsapi_version_t Inet2Nsapi(int family, int & size)
{
    switch (family)
    {
    case AF_INET:
        size = 4;
        return NSAPI_IPv4;
        break;
    case AF_INET6:
        size = 16;
        return NSAPI_IPv6;
        break;
    default:
        size = 0;
        return NSAPI_UNSPEC;
    }
}

void msghdr2Netsocket(SocketAddress * dst, struct sockaddr_in * src)
{
    uint8_t adr[16];
    uint8_t byte;
    int size;

    nsapi_version_t addr_type = Inet2Nsapi(src->sin_family, size);

    for (int i = 0; i < size; i++)
    {
        byte = ((src->sin_addr.s_addr >> (8 * i)) & 0XFF);
#ifdef LITTLE_ENDIAN
        adr[i] = byte;
#else
        adr[(size - 1) - i] = byte;
#endif
    }
    dst->set_ip_bytes(adr, addr_type);
}

void Sockaddr2Netsocket(SocketAddress * dst, struct sockaddr * src)
{
    uint8_t adr[16];
    uint8_t byte;
    int size;
    sockaddr_in * addr        = reinterpret_cast<sockaddr_in *>(src);
    nsapi_version_t addr_type = Inet2Nsapi(src->sa_family, size);

    for (int i = 0; i < size; i++)
    {
        byte = ((addr->sin_addr.s_addr >> (8 * i)) & 0XFF);
#ifdef LITTLE_ENDIAN
        adr[i] = byte;
#else
        adr[(size - 1) - i] = byte;
#endif
    }
    dst->set_ip_bytes(adr, addr_type);
}

static Socket * getSocket(int fd)
{
    Socket * ret       = nullptr;
    BSDSocket * socket = static_cast<BSDSocket *>(mbed_file_handle(fd));

    if (socket != nullptr)
    {
        switch (socket->type)
        {
        case TCP_SOCKET:
            ret = &socket->tcpSocket;
        case UDP_SOCKET:
            ret = &sockets->udpSocket;
        }
    }
    return ret;
}

int getFreeSocketSlotIndex()
{
    int index = NO_FREE_SOCKET_SLOT;
    for (int i = 0; i < MBED_NET_SOCKET_MAX_NUMBER; i++)
    {
        if (sockets[i].type == SOCKET_NOT_INITIALIZED)
        {
            index = i;
            break;
        }
    }
    return index;
}

int mbed_socket(int family, int type, int proto)
{
    int fd    = -1;
    int index = getFreeSocketSlotIndex();
    if (index == NO_FREE_SOCKET_SLOT)
    {
        set_errno(ENOMEM);
        return -1;
    }

    BSDSocket * socket = &sockets[index];

    switch (type)
    {
    case TCP_SOCKET: {
        TCPSocket * tcpSocket = new (&sockets->tcpSocket) TCPSocket();
        if (tcpSocket->open(NetworkInterface::get_default_instance()) != NSAPI_ERROR_OK)
        {
            socket->close();
            set_errno(EPROTO);
            return fd;
        }
        socket->type = TCP_SOCKET;
    }
    break;
    case UDP_SOCKET: {
        UDPSocket * udpSocket = new (&socket->udpSocket) UDPSocket();
        if (udpSocket->open(NetworkInterface::get_default_instance()) != NSAPI_ERROR_OK)
        {
            socket->close();
            set_errno(EPROTO);
            return fd;
        }
        socket->type = UDP_SOCKET;
    }
    break;
    default:
        break;
    };

    if (socket->type != SOCKET_NOT_INITIALIZED)
    {
        socket->fd = bind_to_fd(socket);
        if (socket->fd < 0)
        {
            socket->close();
            set_errno(EBADFD);
            return fd;
        }
        fd = socket->fd;
    }
    return fd;
}

int mbed_socketpair(int family, int type, int proto, int sv[2])
{
    return 0;
}

int mbed_shutdown(int fd, int how)
{
    return 0;
}

int mbed_bind(int fd, const struct sockaddr * addr, socklen_t addrlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;
    Sockaddr2Netsocket(&sockAddr, (struct sockaddr *) addr);

    return socket->bind(sockAddr);
}

int mbed_connect(int fd, const struct sockaddr * addr, socklen_t addrlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;
    Sockaddr2Netsocket(&sockAddr, (struct sockaddr *) addr);

    return socket->connect(sockAddr);
}

int mbed_listen(int fd, int backlog)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }

    return socket->listen(backlog);
}

int mbed_accept(int fd, struct sockaddr * addr, socklen_t * addrlen)
{
    int retFd     = -1;
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }

    retFd = mbed_socket(AF_INET, TCP_SOCKET, 0);
    if (retFd < 0)
    {
        return retFd;
    }

    return retFd;
}

ssize_t mbed_send(int fd, const void * buf, size_t len, int flags)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    return socket->send(buf, len);
}

ssize_t mbed_recv(int fd, void * buf, size_t max_len, int flags)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    return socket->recv(buf, max_len);
}

ssize_t mbed_sendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;
    Sockaddr2Netsocket(&sockAddr, (struct sockaddr *) dest_addr);

    return socket->sendto(sockAddr, buf, len);
}

ssize_t mbed_sendmsg(int fd, const struct msghdr * message, int flags)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;

    msghdr2Netsocket(&sockAddr, (struct sockaddr_in *) message->msg_name);

    return socket->sendto(sockAddr, (void *) message, sizeof(msghdr));
}

ssize_t mbed_recvfrom(int fd, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;
    Sockaddr2Netsocket(&sockAddr, src_addr);

    return socket->recvfrom(&sockAddr, buf, max_len);
}

int mbed_getsockopt(int fd, int level, int optname, void * optval, socklen_t * optlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    return socket->getsockopt(level, optname, optval, optlen);
}

int mbed_setsockopt(int fd, int level, int optname, const void * optval, socklen_t optlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    return socket->setsockopt(level, optname, optval, optlen);
}

int mbed_getsockname(int fd, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
}

int mbed_getpeername(int sockfd, struct sockaddr * addr, socklen_t * addrlen)
{
    auto * socket = getSocket(sockfd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;
    Sockaddr2Netsocket(&sockAddr, addr);
    return socket->getpeername(&sockAddr);
}

ssize_t mbed_recvmsg(int socket, struct msghdr * message, int flags)
{
    auto * fd = getSocket(socket);
    if (fd == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;
    msghdr2Netsocket(&sockAddr, (struct sockaddr_in *) message->msg_name);

    return fd->sendto(sockAddr, (void *) message, sizeof(msghdr));
}

int mbed_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout)
{
    int totalReady = 0;
    FileHandle * fh;
    short fdEvents    = POLLIN | POLLOUT | POLLERR;
    uint32_t waitTime = osWaitForever;
    uint32_t ret;

    EventFlags event;
    uint32_t eventFlag = 1;

    if (nfds < 0)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (timeout)
    {
        waitTime = (timeout->tv_sec * (uint32_t) 1000) + (timeout->tv_usec / (uint32_t) 1000);
    }

    for (int fd = 0; fd < nfds; ++fd)
    {
        fh = mbed_file_handle(fd);
        if (fh)
        {
            if (fh->poll(fdEvents))
            {
                event.set(eventFlag);
                break;
            }
            else
            {
                fh->sigio([&event, eventFlag]() { event.set(eventFlag); });
            }
        }
    }

    ret = event.wait_any(eventFlag, waitTime);
    if (ret & osFlagsError)
    {
        set_errno(EINTR);
        return -1;
    }

    for (int fd = 0; fd < nfds; ++fd)
    {
        fh = mbed_file_handle(fd);
        if (fh)
        {
            fdEvents = fh->poll(POLLIN | POLLOUT | POLLERR);

            if (readfds)
            {
                if (fdEvents & POLLIN)
                {
                    FD_SET(fd, readfds);
                    totalReady++;
                }
                else
                {
                    FD_CLR(fd, readfds);
                }
            }

            if (writefds)
            {
                if (fdEvents & POLLOUT)
                {
                    FD_SET(fd, writefds);
                    totalReady++;
                }
                else
                {
                    FD_CLR(fd, writefds);
                }
            }

            if (exceptfds)
            {
                if (fdEvents & POLLERR)
                {
                    FD_SET(fd, exceptfds);
                    totalReady++;
                }
                else
                {
                    FD_CLR(fd, exceptfds);
                }
            }
        }
    }

    for (int fd = 0; fd < nfds; ++fd)
    {
        fh = mbed_file_handle(fd);
        if (fh)
        {
            fh->sigio(nullptr);
        }
    }

    return totalReady;
}
