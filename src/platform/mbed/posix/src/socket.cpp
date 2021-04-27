#include <net_socket.h>
#include <sys/socket.h>

int socket(int family, int type, int proto)
{
    return mbed_socket(family, type, proto);
}

int socketpair(int family, int type, int proto, int sv[2])
{
    return mbed_socketpair(family, type, proto, sv);
}

int shutdown(int sock, int how)
{
    return mbed_shutdown(sock, how);
}

int bind(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return mbed_bind(sock, addr, addrlen);
}

int connect(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return mbed_connect(sock, addr, addrlen);
}

int listen(int sock, int backlog)
{
    return mbed_listen(sock, backlog);
}

int accept(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return mbed_accept(sock, addr, addrlen);
}

ssize_t send(int sock, const void * buf, size_t len, int flags)
{
    return mbed_send(sock, buf, len, flags);
}

ssize_t recv(int sock, void * buf, size_t max_len, int flags)
{
    return mbed_recv(sock, buf, max_len, flags);
}

ssize_t sendto(int sock, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
    return mbed_sendto(sock, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sock, const struct msghdr * message, int flags)
{
    return mbed_sendmsg(sock, message, flags);
}

ssize_t recvfrom(int sock, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    return mbed_recvfrom(sock, buf, max_len, flags, src_addr, addrlen);
}

int getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen)
{
    return mbed_getsockopt(sock, level, optname, optval, optlen);
}

int setsockopt(int sock, int level, int optname, const void * optval, socklen_t optlen)
{
    return mbed_setsockopt(sock, level, optname, optval, optlen);
}

int getsockname(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return mbed_getsockname(sock, addr, addrlen);
}

int getpeername(int sockfd, struct sockaddr * addr, socklen_t * addrlen)
{
    return mbed_getpeername(sockfd, addr, addrlen);
}

ssize_t recvmsg(int socket, struct msghdr * message, int flags)
{
    return mbed_recvmsg(socket, message, flags);
}
