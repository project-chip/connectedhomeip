#include <net_if.h>
#include <sys/socket.h>

int socket(int family, int type, int proto)
{
    return 0;
}

int socketpair(int family, int type, int proto, int sv[2])
{
    return 0;
}

int shutdown(int sock, int how)
{
    return 0;
}

int bind(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return 0;
}

int connect(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return 0;
}

int listen(int sock, int backlog)
{
    return 0;
}

int accept(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
}

ssize_t send(int sock, const void * buf, size_t len, int flags)
{
    return 0;
}

ssize_t recv(int sock, void * buf, size_t max_len, int flags)
{
    return 0;
}

ssize_t sendto(int sock, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen)
{
    return 0;
}

ssize_t sendmsg(int sock, const struct msghdr * message, int flags)
{
    return 0;
}

ssize_t recvfrom(int sock, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    return 0;
}

int getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen)
{
    return 0;
}

int setsockopt(int sock, int level, int optname, const void * optval, socklen_t optlen)
{
    return 0;
}

int getsockname(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
}

int getpeername(int sockfd, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
}

ssize_t recvmsg(int socket, struct msghdr * message, int flags)
{
    return 0;
}