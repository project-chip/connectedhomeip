/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
 *    Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 *    All rights reserved.
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
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

/* Include some files for defining library routines */
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>

#define LWIP_TIMEVAL_PRIVATE 0

/* Define platform endianness */
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif /* BYTE_ORDER */

typedef uintptr_t mem_ptr_t;

/* Define (sn)printf formatters for these lwIP types */
#define X8_F "02x"
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

/* If only we could use C99 and get %zu */
#if defined(__x86_64__)
#define SZT_F "lu"
#else
#define SZT_F "u"
#endif

/* Compiler hints for packing structures */
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

/* prototypes for printf() and abort() */
#include <stdio.h>
#include <stdlib.h>
/* Plaform specific diagnostic output */
#define LWIP_PLATFORM_DIAG(x)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        printf x;                                                                                                                  \
    } while (0)

#define LWIP_PLATFORM_ASSERT(x)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__);                                               \
        fflush(NULL);                                                                                                              \
        abort();                                                                                                                   \
    } while (0)

#define LWIP_RAND() ((u32_t) rand())

#endif /* __ARCH_CC_H__ */
