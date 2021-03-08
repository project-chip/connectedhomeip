#include <net_socket.h>

BSDSocket sockets[MAX_SOCKET];

Socket * getSocket(int id)
{
    int type = 1;
    switch (type)
    {
    case TCP_SOCKET:
        return &sockets[id].tcpSocket;
    case UDP_SOCKET:
        return &sockets[id].udpSocket;
    default:
        return nullptr;
    }
}

int close(int id)
{
    auto * sock = getSocket(id);
    sock->~Socket();
    sockets[id].type = SOCKET_NOT_INITIALIZED;
}

int mbed_socket(int family, int type, int proto)
{
    int id = ERR_NO_MEMORY;
    for (int i = 0; i < MAX_SOCKET; i++)
    {
        if (sockets[i].type == SOCKET_NOT_INITIALIZED)
        {
            id = i;
            break;
        }
    }
    if (id != ERR_NO_MEMORY)
    {
        switch (type)
        {
        case TCP_SOCKET: {
            TCPSocket * tcpSocket = new (&sockets[id].tcpSocket) TCPSocket();
            if (tcpSocket->open(NetworkInterface::get_default_instance()) != NSAPI_ERROR_OK)
            {
                tcpSocket->~TCPSocket();
                sockets[id].type = SOCKET_NOT_INITIALIZED;
                return ERR_OPEN;
            }
            sockets[id].type = TCP_SOCKET;
        }
        break;
        case UDP_SOCKET: {
            UDPSocket * udpSocket = new (&sockets[id].udpSocket) UDPSocket();
            if (udpSocket->open(NetworkInterface::get_default_instance()) != NSAPI_ERROR_OK)
            {
                udpSocket->~UDPSocket();
                sockets[id].type = SOCKET_NOT_INITIALIZED;
                return ERR_OPEN;
            }
            sockets[id].type = UDP_SOCKET;
        }
        break;
        default:
            break;
        };
    }
    return id;
}

int mbed_socketpair(int family, int type, int proto, int sv[2])
{
    return 0;
}

int mbed_shutdown(int sock, int how)
{
    return 0;
}

int mbed_bind(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }
    SocketAddress sockAddr;
    return socket->bind(sockAddr);
}

int mbed_connect(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }
    SocketAddress sockAddr;
    return socket->connect(sockAddr);
}

int mbed_listen(int sock, int backlog)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }

    return socket->listen(backlog);
}

int mbed_accept(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }
    nsapi_error_t error;
    socket->accept(&error);//must return socket
    return 0;
}

ssize_t mbed_send(int sock, const void * buf, size_t len, int flags)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }
    return socket->send(buf, len);
}

ssize_t mbed_recv(int sock, void * buf, size_t max_len, int flags)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }
    return socket->recv(buf, max_len);
}

ssize_t mbed_sendto(int sock, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }
    SocketAddress sockAddr;
    return socket->sendto(sockAddr, buf, len);
}

ssize_t mbed_sendmsg(int sock, const struct msghdr * message, int flags)
{
    return 0;
}

ssize_t mbed_recvfrom(int sock, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    auto * socket = getSocket(sock);
    if (socket == nullptr)
    {
        return -1;
    }
    SocketAddress sockAddr;
    return socket->recvfrom(&sockAddr, buf, max_len);
}

int mbed_getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen)
{
    return 0;
}

int mbed_setsockopt(int sock, int level, int optname, const void * optval, socklen_t optlen)
{
    return 0;
}

int mbed_getsockname(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
}

int mbed_getpeername(int sockfd, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
}

ssize_t mbed_recvmsg(int socket, struct msghdr * message, int flags)
{
    return 0;
}
