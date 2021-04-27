#ifndef MBED_NET_NET_DEF_H
#define MBED_NET_NET_DEF_H

#include <byteorder.h>
#include <errno.h>
#include <netsocket/nsapi_types.h>
#include <platform/mbed_retarget.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IF_NAMESIZE NSAPI_INTERFACE_NAME_MAX_SIZE

/** @brief Convert 16-bit value from network to host byte order.
 *
 * @param x The network byte order value to convert.
 *
 * @return Host byte order value.
 */
#define ntohs(x) sys_be16_to_cpu(x)

/** @brief Convert 32-bit value from network to host byte order.
 *
 * @param x The network byte order value to convert.
 *
 * @return Host byte order value.
 */
#define ntohl(x) sys_be32_to_cpu(x)

/** @brief Convert 64-bit value from network to host byte order.
 *
 * @param x The network byte order value to convert.
 *
 * @return Host byte order value.
 */
#define ntohll(x) sys_be64_to_cpu(x)

/** @brief Convert 16-bit value from host to network byte order.
 *
 * @param x The host byte order value to convert.
 *
 * @return Network byte order value.
 */
#define htons(x) sys_cpu_to_be16(x)

/** @brief Convert 32-bit value from host to network byte order.
 *
 * @param x The host byte order value to convert.
 *
 * @return Network byte order value.
 */
#define htonl(x) sys_cpu_to_be32(x)

/** @brief Convert 64-bit value from host to network byte order.
 *
 * @param x The host byte order value to convert.
 *
 * @return Network byte order value.
 */
#define htonll(x) sys_cpu_to_be64(x)

#ifdef __cplusplus
}
#endif

#endif /* MBED_NET_NET_DEF_H */
