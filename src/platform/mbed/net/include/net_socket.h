#ifndef MBED_NET_NET_SOCKET_H
#define MBED_NET_NET_SOCKET_H

#include <sys/eventfd.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBED_NET_SOCKET_MAX_NUMBER 5

int mbed_socket(int family, int type, int proto);

int mbed_socketpair(int family, int type, int proto, int sv[2]);

int mbed_shutdown(int fd, int how);

int mbed_bind(int fd, const struct sockaddr * addr, socklen_t addrlen);

int mbed_connect(int fd, const struct sockaddr * addr, socklen_t addrlen);

int mbed_listen(int fd, int backlog);

int mbed_accept(int fd, struct sockaddr * addr, socklen_t * addrlen);

ssize_t mbed_send(int fd, const void * buf, size_t len, int flags);

ssize_t mbed_sendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr, socklen_t addrlen);

ssize_t mbed_sendmsg(int fd, const struct msghdr * message, int flags);

ssize_t mbed_recv(int fd, void * buf, size_t max_len, int flags);

ssize_t mbed_recvfrom(int fd, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen);

ssize_t mbed_recvmsg(int fd, struct msghdr * message, int flags);

int mbed_getsockopt(int fd, int level, int optname, void * optval, socklen_t * optlen);

int mbed_setsockopt(int fd, int level, int optname, const void * optval, socklen_t optlen);

int mbed_getsockname(int fd, struct sockaddr * addr, socklen_t * addrlen);

int mbed_getpeername(int fd, struct sockaddr * addr, socklen_t * addrlen);

int mbed_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout);

int mbed_eventfd(unsigned int initval, int flags);

int mbed_eventfd_read(int fd, eventfd_t * value);

int mbed_eventfd_write(int fd, eventfd_t value);

#ifdef __cplusplus
}
#endif

#endif /* MBED_NET_NET_SOCKET_H */
