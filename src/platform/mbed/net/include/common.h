#ifndef MBED_NET_COMMON_H
#define MBED_NET_COMMON_H

#include <SocketAddress.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef set_errno
#define set_errno(err)                                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        if (err)                                                                                                                   \
        {                                                                                                                          \
            errno = (err);                                                                                                         \
        }                                                                                                                          \
    } while (0)
#endif

int convert_bsd_addr_to_mbed(SocketAddress * out, struct sockaddr * in);
int convert_mbed_addr_to_bsd(struct sockaddr * out, const SocketAddress * in);

#ifdef __cplusplus
}
#endif

#endif /* MBED_NET_COMMON_H */
