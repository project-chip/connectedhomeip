/*
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

/*
 *
 *    Description:
 *      This file defines processor-architecture- and toolchain-
 *      specific constants and types required for building
 *      LwIP against FreeRTOS.
 *
 */
#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

/* std.h functions required */
#include <stdint.h>

/* Disable lwIP's private definition of 'struct timeval' */
// #define LWIP_TIMEVAL_PRIVATE 0
#include <sys/select.h>

/* Define byte order of the system */
// #define BYTE_ORDER LITTLE_ENDIAN

/* Use lwip provided errors as ti compiler is too granular*/
#define LWIP_PROVIDE_ERRNO 1

#define LWIP_RAND() ((u32_t) rand())

/* Setup Packing Macros */
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

/* Different handling for unit test, normally not needed */
#ifdef LWIP_NOASSERT_ON_ERROR
#define LWIP_ERROR(message, expression, handler)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expression))                                                                                                         \
        {                                                                                                                          \
            handler;                                                                                                               \
        }                                                                                                                          \
    } while (0)
#endif

struct sio_status_s;
typedef struct sio_status_s sio_status_t;
#define sio_fd_t sio_status_t *
#define __sio_fd_t_defined

typedef uint32_t sys_prot_t;

#endif /* LWIP_ARCH_CC_H */
