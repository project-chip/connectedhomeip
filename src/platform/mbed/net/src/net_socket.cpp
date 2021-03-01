#include <net_socket.h>

int mbed_socket(int family, int type, int proto)
{
    return 0;
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
    return 0;
}

int mbed_connect(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return 0;
}

int mbed_listen(int sock, int backlog)
{
    return 0;
}

int mbed_accept(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
}

ssize_t mbed_send(int sock, const void * buf, size_t len, int flags)
{
    return 0;
}

ssize_t mbed_recv(int sock, void * buf, size_t max_len, int flags)
{
    return 0;
}

ssize_t mbed_sendto(int sock, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
    return 0;
}

ssize_t mbed_sendmsg(int sock, const struct msghdr * message, int flags)
{
    return 0;
}

ssize_t mbed_recvfrom(int sock, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    return 0;
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