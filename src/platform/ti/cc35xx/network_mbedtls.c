/*
 * Copyright (c) 2024, Texas Instruments Incorporated
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
 */
/* C runtime includes */

#ifndef MBEDTLS_CONFIG_FILE
#define MBEDTLS_CONFIG_FILE "config-hsm.h" // note ! need to be located before the includes of mbedtls
#endif

#include <mbedtls/threading.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <threading_alt.h>
#include <time.h>

/* The SDK's prebuilt mbedtls.a (platform_util.obj) references gmtime() when
 * compiled with MBEDTLS_HAVE_TIME_DATE. newlib-nano for bare-metal does not
 * provide gmtime, so supply a stub that returns a zeroed-out struct tm. */
struct tm * gmtime(const time_t * timer)
{
    (void) timer;
    static struct tm s_tm = { 0 };
    return &s_tm;
}

#ifdef MBEDTLS_THREADING_ALT

// Function to call in your main application to set the hooks
void initialize_mbedtls_threading()
{
    mbedtls_threading_set_alt(threading_mutex_init_pthread, threading_mutex_free_pthread, threading_mutex_lock_pthread,
                              threading_mutex_unlock_pthread);
}

#else
void initialize_mbedtls_threading() {}; // empty function
#endif
