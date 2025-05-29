/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This header file includes Zephyr socket header and implements POSIX wrappers for APIs with
 * 	`zsock_` prefix if necessary. It also defines BSD socket API functions which for various
 *  reasons have not been implemented or exposed in older Zephyr version.
 */

#pragma once

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS
#include <zephyr/net/socket.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS

#if CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#include <sys/select.h>
#endif

static inline ssize_t recvmsg(int sock, struct msghdr * msg, int flags)
{
    // Older Zephyr version doesn't implement recvmsg at all, but if the message vector size is > 0 we can simply
    // translate recvmsg to recvfrom which fills only the first of the provided buffers (although
    // we don't get control messages in such a case).

    if (msg->msg_iovlen < 1)
    {
        errno = EMSGSIZE;
        return -1;
    }

    ssize_t ret = recvfrom(sock, msg->msg_iov[0].iov_base, msg->msg_iov[0].iov_len, flags, static_cast<sockaddr *>(msg->msg_name),
                           &msg->msg_namelen);

    if (ret >= 0)
        msg->msg_controllen = 0;

    return ret;
}

#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_NAMES
extern "C" {

/** POSIX wrapper for @ref zsock_fd_set */
#define fd_set zsock_fd_set
/** POSIX wrapper for @ref ZSOCK_FD_SETSIZE */
#define FD_SETSIZE ZSOCK_FD_SETSIZE

/** POSIX wrapper for @ref ZSOCK_FD_ZERO */
static inline void FD_ZERO(zsock_fd_set * set)
{
    ZSOCK_FD_ZERO(set);
}

/** POSIX wrapper for @ref ZSOCK_FD_ISSET */
static inline int FD_ISSET(int fd, zsock_fd_set * set)
{
    return ZSOCK_FD_ISSET(fd, set);
}

/** POSIX wrapper for @ref ZSOCK_FD_CLR */
static inline void FD_CLR(int fd, zsock_fd_set * set)
{
    ZSOCK_FD_CLR(fd, set);
}

/** POSIX wrapper for @ref ZSOCK_FD_SET */
static inline void FD_SET(int fd, zsock_fd_set * set)
{
    ZSOCK_FD_SET(fd, set);
}

/** POSIX wrapper for @ref zsock_select */
static inline int select(int nfds, zsock_fd_set * readfds, zsock_fd_set * writefds, zsock_fd_set * exceptfds,
                         struct timeval * timeout)
{
    return zsock_select(nfds, readfds, writefds, exceptfds, timeout);
}

/** POSIX wrapper for @ref zsock_pollfd */
#define pollfd zsock_pollfd

/** POSIX wrapper for @ref zsock_socket */
static inline int socket(int family, int type, int proto)
{
    return zsock_socket(family, type, proto);
}

/** POSIX wrapper for @ref zsock_socketpair */
static inline int socketpair(int family, int type, int proto, int sv[2])
{
    return zsock_socketpair(family, type, proto, sv);
}

/** POSIX wrapper for @ref zsock_close */
static inline int close(int sock)
{
    return zsock_close(sock);
}

/** POSIX wrapper for @ref zsock_shutdown */
static inline int shutdown(int sock, int how)
{
    return zsock_shutdown(sock, how);
}

/** POSIX wrapper for @ref zsock_bind */
static inline int bind(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return zsock_bind(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_connect */
static inline int connect(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return zsock_connect(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_listen */
static inline int listen(int sock, int backlog)
{
    return zsock_listen(sock, backlog);
}

/** POSIX wrapper for @ref zsock_accept */
static inline int accept(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return zsock_accept(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_send */
static inline ssize_t send(int sock, const void * buf, size_t len, int flags)
{
    return zsock_send(sock, buf, len, flags);
}

/** POSIX wrapper for @ref zsock_recv */
static inline ssize_t recv(int sock, void * buf, size_t max_len, int flags)
{
    return zsock_recv(sock, buf, max_len, flags);
}

/** POSIX wrapper for @ref zsock_sendto */
static inline ssize_t sendto(int sock, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr,
                             socklen_t addrlen)
{
    return zsock_sendto(sock, buf, len, flags, dest_addr, addrlen);
}

/** POSIX wrapper for @ref zsock_sendmsg */
static inline ssize_t sendmsg(int sock, const struct msghdr * message, int flags)
{
    return zsock_sendmsg(sock, message, flags);
}

/** POSIX wrapper for @ref zsock_recvfrom */
static inline ssize_t recvfrom(int sock, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    return zsock_recvfrom(sock, buf, max_len, flags, src_addr, addrlen);
}

/** POSIX wrapper for @ref zsock_recvmsg */
static inline ssize_t recvmsg(int sock, struct msghdr * msg, int flags)
{
    return zsock_recvmsg(sock, msg, flags);
}

/** POSIX wrapper for @ref zsock_poll */
static inline int poll(struct zsock_pollfd * fds, int nfds, int timeout)
{
    return zsock_poll(fds, nfds, timeout);
}

/** POSIX wrapper for @ref zsock_getsockopt */
static inline int getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen)
{
    return zsock_getsockopt(sock, level, optname, optval, optlen);
}

/** POSIX wrapper for @ref zsock_setsockopt */
static inline int setsockopt(int sock, int level, int optname, const void * optval, socklen_t optlen)
{
    return zsock_setsockopt(sock, level, optname, optval, optlen);
}

/** POSIX wrapper for @ref zsock_getpeername */
static inline int getpeername(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return zsock_getpeername(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_getsockname */
static inline int getsockname(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return zsock_getsockname(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_getaddrinfo */
static inline int getaddrinfo(const char * host, const char * service, const struct zsock_addrinfo * hints,
                              struct zsock_addrinfo ** res)
{
    return zsock_getaddrinfo(host, service, hints, res);
}

/** POSIX wrapper for @ref zsock_freeaddrinfo */
static inline void freeaddrinfo(struct zsock_addrinfo * ai)
{
    zsock_freeaddrinfo(ai);
}

/** POSIX wrapper for @ref zsock_gai_strerror */
static inline const char * gai_strerror(int errcode)
{
    return zsock_gai_strerror(errcode);
}

/** POSIX wrapper for @ref zsock_getnameinfo */
static inline int getnameinfo(const struct sockaddr * addr, socklen_t addrlen, char * host, socklen_t hostlen, char * serv,
                              socklen_t servlen, int flags)
{
    return zsock_getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
}

/** POSIX wrapper for @ref zsock_addrinfo */
#define addrinfo zsock_addrinfo

/** POSIX wrapper for @ref zsock_gethostname */
static inline int gethostname(char * buf, size_t len)
{
    return zsock_gethostname(buf, len);
}

/** POSIX wrapper for @ref zsock_inet_pton */
static inline int inet_pton(sa_family_t family, const char * src, void * dst)
{
    return zsock_inet_pton(family, src, dst);
}

/** POSIX wrapper for @ref zsock_inet_ntop */
static inline char * inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    return zsock_inet_ntop(family, src, dst, size);
}

/** POSIX wrapper for @ref ZSOCK_POLLIN */
#define POLLIN ZSOCK_POLLIN
/** POSIX wrapper for @ref ZSOCK_POLLOUT */
#define POLLOUT ZSOCK_POLLOUT
/** POSIX wrapper for @ref ZSOCK_POLLERR */
#define POLLERR ZSOCK_POLLERR
/** POSIX wrapper for @ref ZSOCK_POLLHUP */
#define POLLHUP ZSOCK_POLLHUP
/** POSIX wrapper for @ref ZSOCK_POLLNVAL */
#define POLLNVAL ZSOCK_POLLNVAL

/** POSIX wrapper for @ref ZSOCK_MSG_PEEK */
#define MSG_PEEK ZSOCK_MSG_PEEK
/** POSIX wrapper for @ref ZSOCK_MSG_CTRUNC */
#define MSG_CTRUNC ZSOCK_MSG_CTRUNC
/** POSIX wrapper for @ref ZSOCK_MSG_TRUNC */
#define MSG_TRUNC ZSOCK_MSG_TRUNC
/** POSIX wrapper for @ref ZSOCK_MSG_DONTWAIT */
#define MSG_DONTWAIT ZSOCK_MSG_DONTWAIT
/** POSIX wrapper for @ref ZSOCK_MSG_WAITALL */
#define MSG_WAITALL ZSOCK_MSG_WAITALL

/** POSIX wrapper for @ref ZSOCK_SHUT_RD */
#define SHUT_RD ZSOCK_SHUT_RD
/** POSIX wrapper for @ref ZSOCK_SHUT_WR */
#define SHUT_WR ZSOCK_SHUT_WR
/** POSIX wrapper for @ref ZSOCK_SHUT_RDWR */
#define SHUT_RDWR ZSOCK_SHUT_RDWR

} // extern "C"
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_NAMES
