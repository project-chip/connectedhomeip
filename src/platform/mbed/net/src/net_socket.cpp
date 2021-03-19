#include "BSDSocket.h"
#include "EventFileHandle.h"
#include "FdControlBlock.h"
#include "OpenFileHandleAsFileDescriptor.h"
#include "common.h"
#include <mbed_retarget.h>
#include <net_socket.h>
#include <rtos/EventFlags.h>

using namespace mbed;
using namespace rtos;

#define NO_FREE_SOCKET_SLOT (-1)

static BSDSocket sockets[MBED_NET_SOCKET_MAX_NUMBER];

nsapi_version_t Inet2Nsapi(int family)
{
    switch (family)
    {
    case AF_INET:
        return NSAPI_IPv4;
        break;
    case AF_INET6:
        return NSAPI_IPv6;
        break;
    default:
        return NSAPI_UNSPEC;
    }
}

void msghdr2Netsocket(SocketAddress * dst, struct sockaddr_in * src)
{
    dst->set_ip_bytes(src->sin_addr.s4_addr, Inet2Nsapi(src->sin_family));
}

void Sockaddr2Netsocket(SocketAddress * dst, struct sockaddr * src)
{
    sockaddr_in * addr = reinterpret_cast<sockaddr_in *>(src);
    dst->set_ip_bytes(addr->sin_addr.s4_addr, Inet2Nsapi(src->sa_family));
}

static Socket * getSocket(int fd)
{
    BSDSocket * socket = static_cast<BSDSocket *>(mbed_file_handle(fd));

    if (socket == nullptr)
    {
        return nullptr;
    }

    return socket->getNetSocket();
}

int getFreeSocketSlotIndex()
{
    int index = NO_FREE_SOCKET_SLOT;
    for (int i = 0; i < MBED_NET_SOCKET_MAX_NUMBER; i++)
    {
        if (!sockets[i].isSocketOpen())
        {
            index = i;
            break;
        }
    }
    return index;
}

int mbed_socket(int family, int type, int proto)
{
    int index = getFreeSocketSlotIndex();
    if (index == NO_FREE_SOCKET_SLOT)
    {
        set_errno(ENOBUFS);
        return -1;
    }

    BSDSocket * socket = &sockets[index];
    return socket->open(type);
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

    retFd = mbed_socket(AF_INET, BSDSocket::MBED_TCP_SOCKET, 0);
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
        set_errno(EBADF);
        return -1;
    }

    auto ret = socket->send(buf, len);
    if (ret < 0)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    ::write(fd, NULL, 0);
    return ret;
}

ssize_t mbed_sendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }
    SocketAddress sockAddr;
    Sockaddr2Netsocket(&sockAddr, (struct sockaddr *) dest_addr);

    auto ret = socket->sendto(sockAddr, buf, len);
    if (ret < 0)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    ::write(fd, NULL, 0);
    return ret;
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

    auto ret = socket->sendto(sockAddr, (void *) message, sizeof(msghdr));
    if (ret < 0)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    ::write(fd, NULL, 0);
    return ret;
}

ssize_t mbed_recv(int fd, void * buf, size_t max_len, int flags)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    auto ret = socket->recv(buf, max_len);
    if (ret < 0)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    ::read(fd, NULL, 0);
    return ret;
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

    auto ret = socket->recvfrom(&sockAddr, buf, max_len);
    if (ret < 0)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    ::read(fd, NULL, 0);
    return ret;
}

ssize_t mbed_recvmsg(int fd, struct msghdr * message, int flags)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    SocketAddress sockAddr;
    msghdr2Netsocket(&sockAddr, (struct sockaddr_in *) message->msg_name);

    auto ret = socket->recvfrom(&sockAddr, (void *) message, sizeof(msghdr));
    if (ret < 0)
    {
        set_errno(ENOBUFS);
        return -1;
    }
    ::read(fd, NULL, 0);
    return ret;
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

static int get_max_select_fd(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds)
{
    int max;

    max = nfds;

    for (int fd = max; fd < FD_SETSIZE; ++fd)
    {
        if (FD_ISSET(fd, readfds) || FD_ISSET(fd, writefds) || FD_ISSET(fd, exceptfds))
        {
            max++;
        }
    }

    return max;
}

int mbed_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout)
{
    nfds = get_max_select_fd(nfds, readfds, writefds, exceptfds);

    auto control_blocks = std::unique_ptr<FdControlBlock[]>{ new (std::nothrow) FdControlBlock[nfds] };
    if (!control_blocks)
    {
        errno = ENOMEM;
        return -1;
    }
    size_t fd_count = 0;
    rtos::EventFlags flag;
    const uint32_t event_flag = 1;
    int fd_processed          = 0;

    // Convert input into FdControlBlock which are more manageable.
    for (int i = 0; i < nfds; ++i)
    {
        auto cb = FdControlBlock(i, readfds, writefds, exceptfds);
        if (cb.handle)
        {
            control_blocks[fd_count] = cb;
            ++fd_count;
        }
    }

    // Install handler
    bool must_wait = true;
    for (size_t i = 0; i < fd_count; ++i)
    {
        auto & cb = control_blocks[i];
        if (cb.poll())
        {
            // One event is set, we don't need to wait to process the FD
            must_wait = false;
            break;
        }
        else
        {
            cb.handle->sigio([&cb, &flag]() {
                if (cb.poll())
                {
                    flag.set(event_flag);
                }
            });
        }
    }

    // Wait operation
    if (fd_count && must_wait)
    {
        if (!timeout)
        {
            // Wait forever
            flag.wait_any(event_flag);
        }
        else if (timeout->tv_sec || timeout->tv_usec)
        {
            // wait for the expected
            rtos::Kernel::Clock::duration_u32 duration{ timeout->tv_sec * 1000 + timeout->tv_usec / 1000 };
            flag.wait_any_for(event_flag, duration);
        }
        else
        {
            // No timeout value set and no file descriptor ready, return
            // immediately, no fd processed
            return fd_processed;
        }
    }

    // Update output file descriptors
    for (auto & fds : { readfds, writefds, exceptfds })
    {
        if (fds)
        {
            FD_ZERO(fds);
        }
    }

    // Update fds watch and watch list
    for (size_t i = 0; i < fd_count; ++i)
    {
        auto & cb   = control_blocks[i];
        auto events = cb.poll();
        if (cb.read && (events & POLLIN))
        {
            FD_SET(cb.fd, readfds);
            ++fd_processed;
        }
        if (cb.write && (events & POLLOUT))
        {
            FD_SET(cb.fd, writefds);
            ++fd_processed;
        }
        if (cb.err && (events & POLLOUT))
        {
            FD_SET(cb.fd, exceptfds);
            ++fd_processed;
        }
        // remove temporary sigio
        cb.handle->sigio(nullptr);
    }

    return fd_processed;
}

int mbed_eventfd(unsigned int initval, int flags)
{
    if (initval || flags)
    {
        return -1;
    }
    return open_fh_as_fd<EventFileHandle>();
}

int mbed_eventfd_read(int fd, eventfd_t * value)
{
    return read(fd, value, sizeof(*value));
}

int mbed_eventfd_write(int fd, eventfd_t value)
{
    return write(fd, &value, sizeof(value));
}