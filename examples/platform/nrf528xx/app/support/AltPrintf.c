/*
 *
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *          Substitute newlib printf functions with an alternate, minimal implementation.
 *
 *          This code is intended to work with the tiny printf implementation published by
 *          Marco Paland (https://github.com/mpaland/printf).
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Include header file from mpaland printf.
#include "printf.h"

// Disable the mapping #defines from the printf.h header so the following functions don't get renamed.
#undef printf
#undef sprintf
#undef snprintf
#undef vsnprintf
#undef vprintf

int printf(const char * format, ...)
{
    /* Stdio not supported */
    return -1;
}

int sprintf(char * buffer, const char * format, ...)
{
    /* Dangerous; should not be called */
    buffer[0] = 0;
    return -1;
}

int snprintf(char * buffer, size_t count, const char * format, ...)
{
    int res;
    va_list va;
    va_start(va, format);
    res = vsnprintf_(buffer, count, format, va);
    va_end(va);
    return res;
}

int vprintf(const char * format, va_list va)
{
    /* Stdio not supported */
    return -1;
}

int vsprintf(char * buffer, const char * format, va_list va)
{
    /* Dangerous; should not be called */
    buffer[0] = 0;
    return -1;
}

int vsnprintf(char * buffer, size_t count, const char * format, va_list va)
{
    return vsnprintf_(buffer, count, format, va);
}

int fprintf(FILE * stream, const char * format, ...)
{
    /* Stdio not supported */
    return -1;
}

int vfprintf(FILE * stream, const char * format, va_list ap)
{
    /* Stdio not supported */
    return -1;
}

/* iprintf variants */

int iprintf(const char * format, ...)
{
    /* Stdio not supported */
    return -1;
}

int siprintf(char * buffer, const char * format, ...)
{
    /* Dangerous; should not be called */
    buffer[0] = 0;
    return -1;
}

int sniprintf(char * buffer, size_t count, const char * format, ...)
{
    int res;
    va_list va;
    va_start(va, format);
    res = vsnprintf_(buffer, count, format, va);
    va_end(va);
    return res;
}

int viprintf(const char * format, va_list va)
{
    /* Stdio not supported */
    return -1;
}

int vsiprintf(char * buffer, const char * format, va_list va)
{
    /* Dangerous; should not be called */
    buffer[0] = 0;
    return -1;
}

int vsniprintf(char * buffer, size_t count, const char * format, va_list va)
{
    return vsnprintf_(buffer, count, format, va);
}

int fiprintf(FILE * stream, const char * format, ...)
{
    /* Stdio not supported */
    return -1;
}

int vfiprintf(FILE * stream, const char * format, va_list ap)
{
    /* Stdio not supported */
    return -1;
}
