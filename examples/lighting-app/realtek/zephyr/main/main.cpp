/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTask.h"
#include <system/SystemError.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;

#include "soc.h"
#include <openthread-system.h>
#include <openthread/instance.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/time.h>

extern "C" {
// replace memcpy, memset
#define CHECK_STR_UNALIGNED(X, Y) (((uint32_t) (X) & (sizeof(uint32_t) - 1)) | ((uint32_t) (Y) & (sizeof(uint32_t) - 1)))

#define STR_OPT_BIGBLOCKSIZE (sizeof(uint32_t) << 2)

#define STR_OPT_LITTLEBLOCKSIZE (sizeof(uint32_t))

void * __wrap_memcpy(void * s1, const void * s2, size_t n)
{
    char * dst       = (char *) s1;
    const char * src = (const char *) s2;

    uint32_t * aligned_dst;
    const uint32_t * aligned_src;

    /* If the size is small, or either SRC or DST is unaligned,
     * then punt into the byte copy loop.  This should be rare.
     */
    if (n < sizeof(uint32_t) || CHECK_STR_UNALIGNED(src, dst))
    {
        while (n--)
        {
            *dst++ = *src++;
        }

        return s1;
    } /* if */

    aligned_dst = (uint32_t *) dst;
    aligned_src = (const uint32_t *) src;

    /* Copy 4X long words at a time if possible.  */
    while (n >= STR_OPT_BIGBLOCKSIZE)
    {
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        n -= STR_OPT_BIGBLOCKSIZE;
    } /* while */

    /* Copy one long word at a time if possible.  */
    while (n >= STR_OPT_LITTLEBLOCKSIZE)
    {
        *aligned_dst++ = *aligned_src++;
        n -= STR_OPT_LITTLEBLOCKSIZE;
    } /* while */

    /* Pick up any residual with a byte copier.  */
    dst = (char *) aligned_dst;
    src = (const char *) aligned_src;
    while (n--)
    {
        *dst++ = *src++;
    }

    return s1;
} /* _memcpy() */

#define wsize sizeof(uint32_t)
#define wmask (wsize - 1)

void * __wrap_memset(void * dst0, int Val, size_t length)
{
    size_t t;
    uint32_t Wideval;
    uint8_t * dst;

    dst = (uint8_t *) dst0;
    /*
     * If not enough words, just fill bytes.  A length >= 2 words
     * guarantees that at least one of them is `complete' after
     * any necessary alignment.  For instance:
     *
     *  |-----------|-----------|-----------|
     *  |00|01|02|03|04|05|06|07|08|09|0A|00|
     *            ^---------------------^
     *       dst         dst+length-1
     *
     * but we use a minimum of 3 here since the overhead of the code
     * to do word writes is substantial.
     */
    if (length < 3 * wsize)
    {
        while (length != 0)
        {
            *dst++ = Val;
            --length;
        }
        return (dst0);
    }

    if ((Wideval = (uint32_t) Val) != 0) /* Fill the word. */
    {
        Wideval = ((Wideval << 24) | (Wideval << 16) | (Wideval << 8) | Wideval); /* u_int is 32 bits. */
    }

    /* Align destination by filling in bytes. */
    if ((t = (uint32_t) dst & wmask) != 0)
    {
        t = wsize - t;
        length -= t;
        do
        {
            *dst++ = Val;
        } while (--t != 0);
    }

    /* Fill words.  Length was >= 2*words so we know t >= 1 here. */
    t = length / wsize;
    do
    {
        *(uint32_t *) dst = Wideval;
        dst += wsize;
    } while (--t != 0);

    /* Mop up trailing bytes, if any. */
    t = length & wmask;
    if (t != 0)
        do
        {
            *dst++ = Val;
        } while (--t != 0);

    return (dst0);
}
}

int main()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef CONFIG_CHIP_PW_RPC
    rpc::Init();
#endif

    if (err == CHIP_NO_ERROR)
    {
        err = AppTask::Instance().StartApp();
    }

    LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
