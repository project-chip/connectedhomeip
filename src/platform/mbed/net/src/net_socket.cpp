#include "BSDSocket.h"
#include "EventFileHandle.h"
#include "FdControlBlock.h"
#include "OpenFileHandleAsFileDescriptor.h"
#include <net_socket.h>
#include <rtos/EventFlags.h>

#include "common.h"

#include "mbed-trace/mbed_trace.h"

using namespace mbed;
using namespace rtos;

#define TRACE_GROUP "NETS"

#define NO_FREE_SOCKET_SLOT (-1)

static BSDSocket sockets[MBED_NET_SOCKET_MAX_NUMBER];

static BSDSocket * getBSDSocket(int fd)
{
    BSDSocket * socket = static_cast<BSDSocket *>(mbed_file_handle(fd));

    if (socket == nullptr)
    {
        return nullptr;
    }

    return socket;
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

struct mbed_socket_option_t
{
    int level;
    int optname;
};

static mbed_socket_option_t convert_socket_option(int level, int optname)
{
    switch (optname)
    {
    case SO_REUSEADDR:
        return { level, NSAPI_REUSEADDR };
    case SO_KEEPALIVE:
        return { level, NSAPI_KEEPALIVE };
    case SO_BROADCAST:
        return { level, NSAPI_BROADCAST };
    default:
        tr_warning("Passing unknown option %d to socket", optname);
        return { level, optname };
    }
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
    tr_info("Create socket family %d type %d proto %d", family, type, proto);
    int index = getFreeSocketSlotIndex();
    if (index == NO_FREE_SOCKET_SLOT)
    {
        tr_err("No free socket slot");
        set_errno(ENOBUFS);
        return -1;
    }

    BSDSocket * socket = &sockets[index];
    return socket->open(family, type);
}

int mbed_socketpair(int family, int type, int proto, int sv[2])
{
    set_errno(EAFNOSUPPORT);
    return -1;
}

int mbed_shutdown(int fd, int how)
{
    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (how != SHUT_RD && how != SHUT_WR && how != SHUT_RDWR)
    {
        set_errno(EINVAL);
        return -1;
    }

    tr_info("Shutdown fd %d how %d", fd, how);

    switch (how)
    {
    case SHUT_RD:
        socket->enable_input(false);
        break;
    case SHUT_WR:
        socket->enable_output(false);
        break;
    case SHUT_RDWR:
        socket->enable_input(false);
        socket->enable_output(false);
        break;
    }

    return 0;
}

int mbed_bind(int fd, const struct sockaddr * addr, socklen_t addrlen)
{
    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (addr == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (socket->socketName)
    {
        set_errno(EINVAL);
        return -1;
    }

    SocketAddress sockAddr;
    if (convert_bsd_addr_to_mbed(&sockAddr, (struct sockaddr *) addr))
    {
        set_errno(EINVAL);
        return -1;
    }

    tr_info("Bind fd %d address %s port %d", fd, sockAddr.get_ip_address(), sockAddr.get_port());

    auto ret = socket->getNetSocket()->bind(sockAddr);
    if ((ret != NSAPI_ERROR_OK) && (ret != NSAPI_ERROR_UNSUPPORTED))
    {
        tr_err("Bind failed [%d]", ret);
        set_errno(EIO);
        return -1;
    }

    socket->socketName = sockAddr;

    return 0;
}

int mbed_connect(int fd, const struct sockaddr * addr, socklen_t addrlen)
{
    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (addr == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    SocketAddress sockAddr;
    if (convert_bsd_addr_to_mbed(&sockAddr, (struct sockaddr *) addr))
    {
        set_errno(EINVAL);
        return -1;
    }

    tr_info("Connect fd %d address %s", fd, sockAddr.get_ip_address());
    auto ret = socket->getNetSocket()->connect(sockAddr);
    if ((ret != NSAPI_ERROR_OK) && (ret != NSAPI_ERROR_UNSUPPORTED))
    {
        tr_err("Connect failed [%d]", ret);
        switch (ret)
        {
        case NSAPI_ERROR_IN_PROGRESS:
            set_errno(EALREADY);
            break;
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(EBADF);
            break;
        case NSAPI_ERROR_IS_CONNECTED:
            set_errno(EISCONN);
            break;
        default:
            set_errno(EIO);
        }

        return -1;
    }

    if (!socket->is_blocking())
    {
        tr_debug("Connect not blocking\n");
        socket->write(NULL, 0);
        set_errno(EINPROGRESS);
        return -1;
    }

    return 0;
}

int mbed_listen(int fd, int backlog)
{
    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (backlog < 0)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (socket->getSocketType() != BSDSocket::MBED_TCP_SOCKET)
    {
        set_errno(EOPNOTSUPP);
        return -1;
    }

    tr_info("Listen fd %d backlog %d", fd, backlog);
    auto ret = socket->getNetSocket()->listen(backlog);
    if ((ret != NSAPI_ERROR_OK) && (ret != NSAPI_ERROR_UNSUPPORTED))
    {
        tr_err("Listen failed [%d]", ret);
        set_errno(EIO);
        return -1;
    }

    return 0;
}

int mbed_accept(int fd, struct sockaddr * addr, socklen_t * addrlen)
{
    nsapi_error_t error;
    Socket * newSocket = nullptr;
    int index;
    SocketAddress sockAddr;

    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    socket->read(NULL, 0);

    if (addr == nullptr || addrlen == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (socket->getSocketType() != BSDSocket::MBED_TCP_SOCKET)
    {
        set_errno(EOPNOTSUPP);
        return -1;
    }

    tr_info("Connection accept for fd %d socket", fd);
    newSocket = socket->getNetSocket()->accept(&error);
    if ((error != NSAPI_ERROR_OK) && (error != NSAPI_ERROR_UNSUPPORTED))
    {
        tr_err("Accept failed [%d]", error);
        set_errno(ENOBUFS);
        return -1;
    }

    error = newSocket->getpeername(&sockAddr);
    if (error != NSAPI_ERROR_OK)
    {
        tr_err("Get peer name failed [%d]", error);
        delete newSocket;
        switch (error)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_NO_CONNECTION:
            set_errno(ECONNABORTED);
            break;
        default:
            set_errno(ENOBUFS);
        }
        return -1;
    }

    if (convert_mbed_addr_to_bsd(addr, &sockAddr))
    {
        delete newSocket;
        set_errno(EINVAL);
        return -1;
    }

    index = getFreeSocketSlotIndex();
    if (index == NO_FREE_SOCKET_SLOT)
    {
        tr_err("No free socket slot");
        delete newSocket;
        set_errno(ENOBUFS);
        return -1;
    }

    return sockets[index].open(addr->sa_family, BSDSocket::MBED_TCP_SOCKET, (InternetSocket *) newSocket);
}

ssize_t mbed_send(int fd, const void * buf, size_t len, int flags)
{
    ssize_t ret;
    bool blockingState;
    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (buf == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (!socket->is_output_enable())
    {
        return 0;
    }

    if (flags & MSG_DONTWAIT)
    {
        blockingState = socket->is_blocking();
        socket->set_blocking(false);
    }

    tr_info("Socket fd %d send %d bytes", fd, len);
    ret = socket->getNetSocket()->send(buf, len);
    if (ret < 0)
    {
        tr_err("Send failed [%d]", ret);
        switch (ret)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_WOULD_BLOCK:
            set_errno(EWOULDBLOCK);
            break;
        case NSAPI_ERROR_NO_ADDRESS:
            set_errno(ENOTCONN);
            break;
        default:
            set_errno(ENOBUFS);
        }
        ret = -1;
    }
    else
    {
        socket->write(NULL, 0);
    }

    if (flags & MSG_DONTWAIT)
    {
        socket->set_blocking(blockingState);
    }

    return ret;
}

ssize_t mbed_sendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
    ssize_t ret;
    bool blockingState;
    SocketAddress sockAddr;

    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (buf == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (!socket->is_output_enable())
    {
        return 0;
    }

    if (flags & MSG_DONTWAIT)
    {
        blockingState = socket->is_blocking();
        socket->set_blocking(false);
    }

    if (dest_addr != nullptr)
    {
        if (convert_bsd_addr_to_mbed(&sockAddr, (struct sockaddr *) dest_addr) < 0)
        {
            set_errno(EINVAL);
            return -1;
        }
    }

    tr_info("Socket fd %d send %d bytes to %s", fd, len, sockAddr.get_ip_address());
    ret = socket->getNetSocket()->sendto(sockAddr, buf, len);
    if (ret < 0)
    {
        tr_err("Send to failed [%d]", ret);
        switch (ret)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_WOULD_BLOCK:
            set_errno(EWOULDBLOCK);
            break;
        default:
            set_errno(ENOBUFS);
        }
        ret = -1;
    }
    else
    {
        socket->write(NULL, 0);
    }

    if (flags & MSG_DONTWAIT)
    {
        socket->set_blocking(blockingState);
    }

    return ret;
}

ssize_t mbed_sendmsg(int fd, const struct msghdr * message, int flags)
{
    ssize_t ret;
    bool blockingState;
    SocketAddress sockAddr;
    ssize_t total = 0;

    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (message == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (!socket->is_output_enable())
    {
        return 0;
    }

    if (flags & MSG_DONTWAIT)
    {
        blockingState = socket->is_blocking();
        socket->set_blocking(false);
    }

    if (convert_bsd_addr_to_mbed(&sockAddr, (struct sockaddr *) message->msg_name) < 0)
    {
        set_errno(EINVAL);
        return -1;
    }

    tr_info("Socket fd %d send message to %s", fd, sockAddr.get_ip_address());
    for (size_t i = 0; i < message->msg_iovlen; i++)
    {
        ret = socket->getNetSocket()->sendto(sockAddr, (void *) message->msg_iov[i].iov_base, message->msg_iov[i].iov_len);
        if (ret < 0)
        {
            switch (ret)
            {
            case NSAPI_ERROR_NO_SOCKET:
                set_errno(ENOTSOCK);
                break;
            case NSAPI_ERROR_WOULD_BLOCK:
                set_errno(EWOULDBLOCK);
                break;
            default:
                set_errno(ENOBUFS);
            }
            total = -1;
            break;
        }
        total += ret;
    }

    socket->write(NULL, 0);

    if (flags & MSG_DONTWAIT)
    {
        socket->set_blocking(blockingState);
    }

    return total;
}

ssize_t mbed_recv(int fd, void * buf, size_t max_len, int flags)
{
    ssize_t ret;
    bool blockingState;
    SocketAddress peerAddr;

    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (buf == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (!socket->is_input_enable())
    {
        return 0;
    }

    if (flags & MSG_DONTWAIT)
    {
        blockingState = socket->is_blocking();
        socket->set_blocking(false);
    }

    ret = socket->getNetSocket()->recv(buf, max_len);
    if (ret < 0)
    {
        tr_err("Receive failed [%d]", ret);
        switch (ret)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_WOULD_BLOCK:
            set_errno(EWOULDBLOCK);
            break;
        default:
            set_errno(ENOBUFS);
        }
        ret = -1;
    }
    else
    {
        socket->getNetSocket()->getpeername(&peerAddr);
        tr_info("Socket fd %d recevied %d bytes from %s", fd, ret, peerAddr.get_ip_address());
        socket->read(NULL, 0);
    }

    if (flags & MSG_DONTWAIT)
    {
        socket->set_blocking(blockingState);
    }

    return ret;
}

ssize_t mbed_recvfrom(int fd, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    ssize_t ret;
    bool blockingState;
    SocketAddress sockAddr;

    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (buf == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (!socket->is_input_enable())
    {
        return 0;
    }

    if (flags & MSG_DONTWAIT)
    {
        blockingState = socket->is_blocking();
        socket->set_blocking(false);
    }

    ret = socket->getNetSocket()->recvfrom(&sockAddr, buf, max_len);
    if (ret < 0)
    {
        tr_err("Receive from failed [%d]", ret);
        switch (ret)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_WOULD_BLOCK:
            set_errno(EWOULDBLOCK);
            break;
        default:
            set_errno(ENOBUFS);
        }
        ret = -1;
    }
    else
    {
        tr_info("Socket fd %d recevied %d bytes from %s", fd, ret, sockAddr.get_ip_address());
        socket->read(NULL, 0);
        if (src_addr != nullptr)
        {
            if (convert_mbed_addr_to_bsd(src_addr, &sockAddr))
            {
                set_errno(EINVAL);
                ret = -1;
            }
        }
    }

    if (flags & MSG_DONTWAIT)
    {
        socket->set_blocking(blockingState);
    }

    return ret;
}

ssize_t mbed_recvmsg(int fd, struct msghdr * message, int flags)
{
    bool blockingState;
    SocketAddress sockAddr;
    ssize_t total = 0;

    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (message == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (!socket->is_input_enable())
    {
        return 0;
    }

    if (flags & MSG_DONTWAIT)
    {
        blockingState = socket->is_blocking();
        socket->set_blocking(false);
    }

    for (size_t i = 0; i < message->msg_iovlen; i++)
    {
        auto ret = socket->getNetSocket()->recvfrom(&sockAddr, (void *) message->msg_iov[i].iov_base, message->msg_iov[i].iov_len);
        if (ret < 0)
        {
            tr_err("Receive from failed [%d]", ret);
            switch (ret)
            {
            case NSAPI_ERROR_NO_SOCKET:
                set_errno(ENOTSOCK);
                break;
            case NSAPI_ERROR_WOULD_BLOCK:
                set_errno(EWOULDBLOCK);
                break;
            default:
                set_errno(ENOBUFS);
            }
            total = -1;
            break;
        }
        total += ret;
        tr_info("Socket fd %d received %d bytes message from %s", fd, ret, sockAddr.get_ip_address());
    }

    socket->read(NULL, 0);

    if (total != -1)
    {
        if (message->msg_name != nullptr)
        {
            if (convert_mbed_addr_to_bsd((sockaddr *) message->msg_name, &sockAddr))
            {
                set_errno(EINVAL);
                total = -1;
            }
        }
    }

    if (flags & MSG_DONTWAIT)
    {
        socket->set_blocking(blockingState);
    }

    return total;
}

int mbed_getsockopt(int fd, int level, int optname, void * optval, socklen_t * optlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    // Use NSAPI options instead of POSIX one
    auto opt = convert_socket_option(level, optname);
    auto ret = socket->getsockopt(opt.level, opt.optname, optval, optlen);
    if (ret < 0)
    {
        tr_err("Get socket option %s [%d]", ret == NSAPI_ERROR_UNSUPPORTED ? "unsupported" : "failed", ret);
        switch (ret)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_UNSUPPORTED:
            if ((optname == SO_ERROR) || (optval != nullptr))
            {
                *(int *) optval = 0;
                return 0;
            }
            set_errno(ENOPROTOOPT);
            break;
        default:
            set_errno(ENOBUFS);
        }
        return -1;
    }
    tr_info("Get socket fd %d option level %d optname %d", fd, level, optname);
    return 0;
}

int mbed_setsockopt(int fd, int level, int optname, const void * optval, socklen_t optlen)
{
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    // Convert the option to NSAPI option alias
    auto opt = convert_socket_option(level, optname);
    auto ret = socket->setsockopt(opt.level, opt.optname, optval, optlen);
    if (ret < 0)
    {
        tr_err("Set socket option %s: level = %d, optname=%d, val=%p, len=%d => [%d]",
               ret == NSAPI_ERROR_UNSUPPORTED ? "unsupported" : "failed", level, optname, optval, optlen, ret);
        switch (ret)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_UNSUPPORTED:
            set_errno(ENOPROTOOPT);
            break;
        default:
            set_errno(ENOBUFS);
        }
        return -1;
    }
    tr_info("Set socket fd %d option level %d optname %d", fd, level, optname);
    return 0;
}

int mbed_getsockname(int fd, struct sockaddr * addr, socklen_t * addrlen)
{
    auto * socket = getBSDSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (addr == nullptr || addrlen == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    if (socket->socketName)
    {
        set_errno(EINVAL);
        return -1;
    }

    tr_info("Get socket fd %d name", fd);
    if (socket->socketName.get_ip_version() == NSAPI_IPv4)
    {
        if (*addrlen < sizeof(sockaddr_in))
        {
            *addrlen = sizeof(sockaddr_in);
            set_errno(ENOBUFS);
            return -1;
        }
    }
    else if (socket->socketName.get_ip_version() == NSAPI_IPv6)
    {
        if (*addrlen < sizeof(sockaddr_in6))
        {
            *addrlen = sizeof(sockaddr_in6);
            set_errno(ENOBUFS);
            return -1;
        }
    }

    if (convert_mbed_addr_to_bsd(addr, &socket->socketName) < 0)
    {
        set_errno(ENOBUFS);
        return -1;
    }

    return 0;
}

int mbed_getpeername(int fd, struct sockaddr * addr, socklen_t * addrlen)
{
    SocketAddress sockAddr;
    auto * socket = getSocket(fd);
    if (socket == nullptr)
    {
        set_errno(EBADF);
        return -1;
    }

    if (addr == nullptr || addrlen == nullptr)
    {
        set_errno(EINVAL);
        return -1;
    }

    auto ret = socket->getpeername(&sockAddr);
    if (ret < 0)
    {
        tr_err("Get peer name failed [%d]", ret);
        switch (ret)
        {
        case NSAPI_ERROR_NO_SOCKET:
            set_errno(ENOTSOCK);
            break;
        case NSAPI_ERROR_NO_CONNECTION:
            set_errno(ENOTCONN);
            break;
        default:
            set_errno(ENOBUFS);
        }

        return -1;
    }
    tr_info("Get socket fd %d peer name", fd);
    convert_mbed_addr_to_bsd(addr, &sockAddr);
    return 0;
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
        if (cb.err && (events & POLLERR))
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
