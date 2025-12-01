/*
 * Copyright (c) 2016-2022 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 *  ======== memory.c ========
 */

#if defined(__ti__) && !defined(__clang__)

#pragma FUNC_EXT_CALLED(malloc);
#pragma FUNC_EXT_CALLED(memalign);
#pragma FUNC_EXT_CALLED(free);
#pragma FUNC_EXT_CALLED(calloc);
#pragma FUNC_EXT_CALLED(realloc);
#pragma FUNC_EXT_CALLED(aligned_alloc);

#define ATTRIBUTE

#elif defined(__IAR_SYSTEMS_ICC__)

#define ATTRIBUTE

#else

#define ATTRIBUTE __attribute__((used))

#endif

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <portmacro.h>

#if defined(__GNUC__) && !defined(__ti__)

#include <reent.h>

#endif

extern void * pvPortRealloc(void * pv, size_t size);

/*
 * This Header is only needed to support advanced memory services - namely
 * realloc() and memalign().  If the user doesn't require those features, they
 * can remove the overhead and save both code and data.
 */
#if defined(TI_POSIX_FREERTOS_MEMORY_ENABLEADV)
/*
 * Header is a union to make sure that the size is a power of 2.
 *
 * On the MSP430 small model (MSP430X), size_t is 2 bytes, which makes
 * the size of this struct 6 bytes.
 */
typedef union Header
{
    struct
    {
        void * actualBuf;
        size_t size;
    } header;
    int pad[2]; /* 4 words on 28L, 8 bytes on most others */
} Header;
#endif

/*
 *  ======== malloc ========
 */
void ATTRIBUTE * malloc(size_t size)
{
#if defined(TI_POSIX_FREERTOS_MEMORY_ENABLEADV)
    Header * packet;
    size_t allocSize;

    allocSize = size + sizeof(Header);

    /*
     * If size is very large and allocSize overflows, the result will be
     * smaller than size. In this case, don't try to allocate.
     */
    if ((size == 0) || (allocSize < size))
    {
        errno = EINVAL;
        return (NULL);
    }

    packet = (Header *) pvPortMalloc(allocSize);

    if (packet == NULL)
    {
        errno = ENOMEM;
        return (NULL);
    }

    packet->header.actualBuf = (void *) packet;
    packet->header.size      = allocSize;

    return (packet + 1);
#else
    void * packet;

    if (size == 0)
    {
        errno = EINVAL;
        return (NULL);
    }

    packet = pvPortMalloc(size);

    if (packet == NULL)
    {
        errno = ENOMEM;
        return (NULL);
    }

    return (packet);
#endif
}

/*
 *  ======== calloc ========
 */
void ATTRIBUTE * calloc(size_t nmemb, size_t size)
{
    size_t nbytes;
    void * retval;

    /* guard against divide by zero exception below */
    if (nmemb == 0)
    {
        errno = EINVAL;
        return (NULL);
    }

    nbytes = nmemb * size;

    /* return NULL if there's an overflow */
    if (nmemb && size != (nbytes / nmemb))
    {
        errno = EOVERFLOW;
        return (NULL);
    }

    retval = malloc(nbytes);
    if (retval != NULL)
    {
        (void) memset(retval, (int) '\0', nbytes);
    }

    return (retval);
}
#ifndef DeviceFamily_CC3220
/*
 *  ======== realloc ========
 */
void ATTRIBUTE * realloc(void * ptr, size_t size)
{
#if defined(TI_POSIX_FREERTOS_MEMORY_ENABLEADV)
    void * retval;
    Header * packet;
    size_t oldSize;

    if (ptr == NULL)
    {
        retval = malloc(size);
    }
    else if (size == 0)
    {
        errno  = EINVAL;
        retval = NULL;
    }
    else
    {
        packet = (Header *) ptr - 1;
        retval = malloc(size);
        if (retval != NULL)
        {
            oldSize = packet->header.size - sizeof(Header);
            (void) memcpy(retval, ptr, (size < oldSize) ? size : oldSize);
            free(ptr);
        }
    }

    return (retval);
#else
    void * packet;

    if (size == 0)
    {
        errno = EINVAL;
        return (NULL);
    }

    packet = pvPortRealloc(ptr, size);

    if (packet == NULL)
    {
        errno = ENOMEM;
        return (NULL);
    }

    return (packet);
#endif
}

#else
/*
 *  ======== realloc ========
 */
void ATTRIBUTE * realloc(void * ptr, size_t size)
{
#if defined(TI_POSIX_FREERTOS_MEMORY_ENABLEADV)
    void * retval;
    Header * packet;
    size_t oldSize;

    if (ptr == NULL)
    {
        retval = malloc(size);
    }
    else if (size == 0)
    {
        errno  = EINVAL;
        retval = NULL;
    }
    else
    {
        packet = (Header *) ptr - 1;
        retval = malloc(size);
        if (retval != NULL)
        {
            oldSize = packet->header.size - sizeof(Header);
            (void) memcpy(retval, ptr, (size < oldSize) ? size : oldSize);
            free(ptr);
        }
    }

    return (retval);
#else
    /* Unsupported implementation */
    return (NULL);
#endif
}
#endif

/*
 *  ======== free ========
 */
void ATTRIBUTE free(void * ptr)
{
#if defined(TI_POSIX_FREERTOS_MEMORY_ENABLEADV)
    Header * packet;

    if (ptr != NULL)
    {
        packet = ((Header *) ptr) - 1;
        vPortFree(packet->header.actualBuf);
    }
#else
    vPortFree(ptr);
#endif
}

/*
 *  ======== memalign ========
 */
void ATTRIBUTE * memalign(size_t boundary, size_t size)
{
#if defined(TI_POSIX_FREERTOS_MEMORY_ENABLEADV)
    Header * packet;
    void * tmp;

    /* return NULL if size is 0, or alignment is not a power-of-2 */
    if (size == 0 || (boundary & (boundary - 1)))
    {
        errno = EINVAL;
        return (NULL);
    }

    if (boundary < sizeof(Header))
    {
        boundary = sizeof(Header);
    }

    /*
     * Allocate 'align + size + sizeof(Header)' so that we have room for
     * the 'packet' and can return an aligned buffer.
     */
    tmp = pvPortMalloc(boundary + size + sizeof(Header));

    if (tmp == NULL)
    {
        errno = ENOMEM;
        return (NULL);
    }

    if ((unsigned int) tmp & (boundary - 1))
    {
        /* tmp is not already aligned */
        packet = (Header *) (((unsigned int) tmp + boundary) & ~(boundary - 1)) - 1;
        if (packet < (Header *) tmp)
        {
            /* don't have room for Header before aligned address */
            packet = (Header *) ((unsigned int) packet + boundary);
        }
    }
    else
    {
        /* tmp is already aligned to boundary (by chance) */
        packet = ((Header *) (((unsigned int) tmp + boundary))) - 1;
    }

    packet->header.actualBuf = tmp;
    packet->header.size      = size + sizeof(Header);

    return (packet + 1);
#else
    /* user called an unsupported function */
    return (NULL);
#endif
}

/*
 *  ======== aligned_alloc ========
 */
void ATTRIBUTE * aligned_alloc(size_t alignment, size_t size)
{
    /* use existing implementation to allocate buffer */
    return (memalign(alignment, size));
}

#if defined(__GNUC__) && !defined(__ti__)

/*
 *  ======== _malloc_r ========
 */
void ATTRIBUTE * _malloc_r(struct _reent * rptr, size_t size)
{
    return malloc(size);
}

/*
 *  ======== _calloc_r ========
 */
void ATTRIBUTE * _calloc_r(struct _reent * rptr, size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

/*
 *  ======== _free_r ========
 */
void ATTRIBUTE _free_r(struct _reent * rptr, void * ptr)
{
    free(ptr);
}

/*
 *  ======== _realloc_r ========
 */
void ATTRIBUTE * _realloc_r(struct _reent * rptr, void * ptr, size_t size)
{
    return realloc(ptr, size);
}

#endif
