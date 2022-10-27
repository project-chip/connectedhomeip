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

/* Include some files for defining library routines */
#include <errno.h>
#include <stdio.h>  /* printf, fflush, FILE */
#include <stdlib.h> /* abort */
#include <string.h>
#if (!defined(__CC_ARM)) && (!defined(__ICCARM__))
#include <sys/time.h>
#endif
#include "syslog.h"

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#define LWIP_PLATFORM_BYTESWAP 0

/** @todo fix some warnings: don't use #pragma if compiling with cygwin gcc */
//#ifndef __GNUC__
#if (!defined(__ICCARM__)) && (!defined(__GNUC__)) && (!defined(__CC_ARM))
#include <limits.h>
#pragma warning(disable : 4244) /* disable conversion warning (implicit integer promotion!) */
#pragma warning(disable : 4127) /* conditional expression is constant */
#pragma warning(disable : 4996) /* 'strncpy' was declared deprecated */
#pragma warning(disable : 4103) /* structure packing changed by including file */
#endif

//#define LWIP_PROVIDE_ERRNO

#if (!defined(__CC_ARM)) && (!defined(__ICCARM__))
#define LWIP_TIMEVAL_PRIVATE 0
#endif

/* Define generic types used in lwIP */
typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned long u32_t;
typedef signed long s32_t;

typedef size_t mem_ptr_t;
typedef u32_t sys_prot_t;

/* Define (sn)printf formatters for these lwIP types */
#define X8_F "02x"
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"
#define SZT_F U32_F

/* Compiler hints for packing structures */
#if defined(__ICCARM__)
#define PACK_STRUCT_STRUCT __packed
#else
#define PACK_STRUCT_STRUCT __attribute__((packed))
#endif

//#define LWIP_DEBUG_USE_PRINTF

#ifdef LWIP_DEBUG_USE_PRINTF
/* Plaform specific diagnostic output */
#define LWIP_PLATFORM_DIAG(x)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        printf x;                                                                                                                  \
    } while (0)
#else
//#define LWIP_PLATFORM_DIAG(x)   do { LWIP_LOGI x; } while(0)
void filogic_log_print(const char * fmt, ...);
#define LWIP_PLATFORM_DIAG(x)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        filogic_log_print x;                                                                                                       \
    } while (0)
#endif

#if 0
#define LWIP_PLATFORM_ASSERT(x)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__);                                               \
        fflush(NULL);                                                                                                              \
        abort();                                                                                                                   \
    } while (0)
#else

#ifdef MTK_DEBUG_LEVEL_NONE
#define LWIP_NOASSERT 1
#endif

#ifndef MTK_DEBUG_LEVEL_NONE
#define LWIP_PLATFORM_ASSERT(x)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__);                                               \
    } while (0)
#else
#define LWIP_PLATFORM_ASSERT(x)
#endif
#endif

#if 0
#define LWIP_ERROR(message, expression, handler)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expression))                                                                                                         \
        {                                                                                                                          \
            printf("Assertion \"%s\" failed at line %d in %s\n", message, __LINE__, __FILE__);                                     \
            fflush(NULL);                                                                                                          \
            handler;                                                                                                               \
        }                                                                                                                          \
    } while (0)
#else
#ifndef MTK_DEBUG_LEVEL_NONE
#define LWIP_ERROR(message, expression, handler)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expression))                                                                                                         \
        {                                                                                                                          \
            printf("Assertion \"%s\" failed at line %d in %s\n", message, __LINE__, __FILE__);                                     \
            handler;                                                                                                               \
        }                                                                                                                          \
    } while (0)
#else
#define LWIP_ERROR(message, expression, handler)
#endif
#endif

/* C runtime functions redefined */
//#define snprintf _snprintf //2015-07-22 Cheng Liu @132663

u32_t dns_lookup_external_hosts_file(const char * name);

#define LWIP_RAND() ((u32_t) rand())

#endif /* __ARCH_CC_H__ */
