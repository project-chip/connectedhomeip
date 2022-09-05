/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2004, Apple Computer, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dnssd_ipc.h"
#include "os.h"


#if defined(_WIN32)

char *win32_strerror(int inErrorCode)
{
    static char buffer[1024];
    DWORD n;
    os_memset(buffer, 0, sizeof(buffer));
    n = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD) inErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        sizeof(buffer),
        NULL);
    if (n > 0)
    {
        // Remove any trailing CR's or LF's since some messages have them.
        while ((n > 0) && isspace(((unsigned char *) buffer)[n - 1]))
            buffer[--n] = '\0';
    }
    return buffer;
}

#endif

void put_uint32(const uint32_t l, char **ptr)
{
    (*ptr)[0] = (char)((l >> 24) &  0xFF);
    (*ptr)[1] = (char)((l >> 16) &  0xFF);
    (*ptr)[2] = (char)((l >>  8) &  0xFF);
    (*ptr)[3] = (char)((l      ) &  0xFF);
    *ptr += sizeof(uint32_t);
}

uint32_t get_uint32(const char **ptr, const char *end)
{
    if (!*ptr || *ptr + sizeof(uint32_t) > end)
    {
        *ptr = NULL;
        return(0);
    }
    else
    {
        uint8_t *p = (uint8_t*) *ptr;
        *ptr += sizeof(uint32_t);
        return((uint32_t) ((uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]));
    }
}

void put_uint16(uint16_t s, char **ptr)
{
    (*ptr)[0] = (char)((s >>  8) &  0xFF);
    (*ptr)[1] = (char)((s      ) &  0xFF);
    *ptr += sizeof(uint16_t);
}

uint16_t get_uint16(const char **ptr, const char *end)
{
    if (!*ptr || *ptr + sizeof(uint16_t) > end)
    {
        *ptr = NULL;
        return(0);
    }
    else
    {
        uint8_t *p = (uint8_t*) *ptr;
        *ptr += sizeof(uint16_t);
        return((uint16_t) ((uint16_t)p[0] << 8 | p[1]));
    }
}

int put_string(const char *str, char **ptr)
{
    if (!str) str = "";
    //strcpy(*ptr, str);
    os_strncpy(*ptr, str, os_strlen(str));
    *ptr += os_strlen(str) + 1;
    return 0;
}

int get_string(const char **ptr, const char *const end, char *buffer, int buflen)
{
    if (!*ptr)
    {
        *buffer = 0;
        return(-1);
    }
    else
    {
        char *lim = buffer + buflen;    // Calculate limit
        while (*ptr < end && buffer < lim)
        {
            char c = *buffer++ = *(*ptr)++;
            if (c == 0) return(0);      // Success
        }
        if (buffer == lim) buffer--;
        *buffer = 0;                    // Failed, so terminate string,
        *ptr = NULL;                    // clear pointer,
        return(-1);                     // and return failure indication
    }
}

void put_rdata(const int rdlen, const unsigned char *rdata, char **ptr)
{
    os_memcpy(*ptr, rdata, rdlen);
    *ptr += rdlen;
}

const char *get_rdata(const char **ptr, const char *end, int rdlen)
{
    if (!*ptr || *ptr + rdlen > end)
    {
        *ptr = NULL;
        return(0);
    }
    else
    {
        const char *rd = *ptr;
        *ptr += rdlen;
        return rd;
    }
}

void ConvertHeaderBytes(ipc_msg_hdr *hdr)
{
    hdr->version   = htonl(hdr->version);
    hdr->datalen   = htonl(hdr->datalen);
    hdr->ipc_flags = htonl(hdr->ipc_flags);
    hdr->op        = htonl(hdr->op );
    hdr->reg_index = htonl(hdr->reg_index);
}
