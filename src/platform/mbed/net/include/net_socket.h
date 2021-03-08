#ifndef MBED_NET_NET_SOCKET_H
#define MBED_NET_NET_SOCKET_H

#include <netsocket/Socket.h>
#include <netsocket/TCPSocket.h>
#include <netsocket/UDPSocket.h>

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_SOCKET 5

#define TCP_SOCKET SOCK_STREAM
#define UDP_SOCKET SOCK_DGRAM
#define SOCKET_NOT_INITIALIZED 0
#define ERR_NO_MEMORY -1
#define ERR_OPEN -2
struct BSDSocket
{
    BSDSocket() {}
    ~BSDSocket() {}

    union
    {

        TCPSocket tcpSocket;
        UDPSocket udpSocket;
    };
    int type;
};

int mbed_socket(int family, int type, int proto);

int mbed_socketpair(int family, int type, int proto, int sv[2]);

int mbed_close(int sock);

int mbed_shutdown(int sock, int how);

int mbed_bind(int sock, const struct sockaddr * addr, socklen_t addrlen);

int mbed_connect(int sock, const struct sockaddr * addr, socklen_t addrlen);

int mbed_listen(int sock, int backlog);

int mbed_accept(int sock, struct sockaddr * addr, socklen_t * addrlen);

ssize_t mbed_send(int sock, const void * buf, size_t len, int flags);

ssize_t mbed_recv(int sock, void * buf, size_t max_len, int flags);

ssize_t mbed_sendto(int sock, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen);

ssize_t mbed_sendmsg(int sock, const struct msghdr * message, int flags);

ssize_t mbed_recvfrom(int sock, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen);

int mbed_getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen);

int mbed_setsockopt(int sock, int level, int optname, const void * optval, socklen_t optlen);

int mbed_getsockname(int sock, struct sockaddr * addr, socklen_t * addrlen);

int mbed_getpeername(int sockfd, struct sockaddr * addr, socklen_t * addrlen);

ssize_t mbed_recvmsg(int socket, struct msghdr * message, int flags);

#ifdef __cplusplus
}
#endif

#endif /* MBED_NET_NET_SOCKET_H */