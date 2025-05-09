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

extern "C" {

/** POSIX wrapper for @ref zsock_socket */
static inline int socket(int family, int type, int proto)
{
    return 0;
    // return zsock_socket(family, type, proto);
}

/** POSIX wrapper for @ref zsock_socketpair */
static inline int socketpair(int family, int type, int proto, int sv[2])
{
    return 0;
    // return zsock_socketpair(family, type, proto, sv);
}

/** POSIX wrapper for @ref zsock_close */
static inline int close(int sock)
{
    return 0;
    // return zsock_close(sock);
}

/** POSIX wrapper for @ref zsock_shutdown */
static inline int shutdown(int sock, int how)
{
    return 0;
    // return zsock_shutdown(sock, how);
}

/** POSIX wrapper for @ref zsock_bind */
static inline int bind(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return 0;
    // return zsock_bind(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_connect */
static inline int connect(int sock, const struct sockaddr * addr, socklen_t addrlen)
{
    return 0;
    // return zsock_connect(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_listen */
static inline int listen(int sock, int backlog)
{
    return 0;
    // return zsock_listen(sock, backlog);
}

/** POSIX wrapper for @ref zsock_accept */
static inline int accept(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
    // return zsock_accept(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_send */
static inline ssize_t send(int sock, const void * buf, size_t len, int flags)
{
    return 0;
    // return zsock_send(sock, buf, len, flags);
}

/** POSIX wrapper for @ref zsock_recv */
static inline ssize_t recv(int sock, void * buf, size_t max_len, int flags)
{
    return 0;
    // return zsock_recv(sock, buf, max_len, flags);
}

/** POSIX wrapper for @ref zsock_sendto */
static inline ssize_t sendto(int sock, const void * buf, size_t len, int flags, const struct sockaddr * dest_addr,
                             socklen_t addrlen)
{
    return 0;
    // return zsock_sendto(sock, buf, len, flags, dest_addr, addrlen);
}

/** POSIX wrapper for @ref zsock_sendmsg */
static inline ssize_t sendmsg(int sock, const struct msghdr * message, int flags)
{
    return 0;
    // return zsock_sendmsg(sock, message, flags);
}

/** POSIX wrapper for @ref zsock_recvfrom */
static inline ssize_t recvfrom(int sock, void * buf, size_t max_len, int flags, struct sockaddr * src_addr, socklen_t * addrlen)
{
    return 0;
    // return zsock_recvfrom(sock, buf, max_len, flags, src_addr, addrlen);
}

/** POSIX wrapper for @ref zsock_recvmsg */
static inline ssize_t recvmsg(int sock, struct msghdr * msg, int flags)
{
    return 0;
    // return zsock_recvmsg(sock, msg, flags);
}

/** POSIX wrapper for @ref zsock_poll */
static inline int poll(struct zsock_pollfd * fds, int nfds, int timeout)
{
    return 0;
    // return zsock_poll(fds, nfds, timeout);
}

/** POSIX wrapper for @ref zsock_getsockopt */
static inline int getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen)
{
    return 0;
    // return zsock_getsockopt(sock, level, optname, optval, optlen);
}

/** POSIX wrapper for @ref zsock_setsockopt */
static inline int setsockopt(int sock, int level, int optname, const void * optval, socklen_t optlen)
{
    return 0;
    // return zsock_setsockopt(sock, level, optname, optval, optlen);
}

/** POSIX wrapper for @ref zsock_getpeername */
static inline int getpeername(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
    // return zsock_getpeername(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_getsockname */
static inline int getsockname(int sock, struct sockaddr * addr, socklen_t * addrlen)
{
    return 0;
    // return zsock_getsockname(sock, addr, addrlen);
}

/** POSIX wrapper for @ref zsock_getaddrinfo */
static inline int getaddrinfo(const char * host, const char * service, const struct zsock_addrinfo * hints,
                              struct zsock_addrinfo ** res)
{
    return 0;
    // return zsock_getaddrinfo(host, service, hints, res);
}

/** POSIX wrapper for @ref zsock_freeaddrinfo */
static inline void freeaddrinfo(struct zsock_addrinfo * ai)
{
    return 0;
    zsock_freeaddrinfo(ai);
}

/** POSIX wrapper for @ref zsock_gai_strerror */
static inline const char * gai_strerror(int errcode)
{
    return 0;
    // return zsock_gai_strerror(errcode);
}

/** POSIX wrapper for @ref zsock_getnameinfo */
static inline int getnameinfo(const struct sockaddr * addr, socklen_t addrlen, char * host, socklen_t hostlen, char * serv,
                              socklen_t servlen, int flags)
{
    return 0;
    // return zsock_getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
}

/** POSIX wrapper for @ref zsock_gethostname */
static inline int gethostname(char * buf, size_t len)
{
    return 0;
    // return zsock_gethostname(buf, len);
}

/** POSIX wrapper for @ref zsock_inet_pton */
static inline int inet_pton(sa_family_t family, const char * src, void * dst)
{
    return 0;
    // return zsock_inet_pton(family, src, dst);
}

/** POSIX wrapper for @ref zsock_inet_ntop */
static inline char * inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    return 0;
    // return zsock_inet_ntop(family, src, dst, size);
}

} // extern "C"
