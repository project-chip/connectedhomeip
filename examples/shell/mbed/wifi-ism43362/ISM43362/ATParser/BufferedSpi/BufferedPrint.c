/*
 * Copyright (c) 2014-2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mbed_error.h"

size_t BufferedSpiThunk(void *buf_serial, const void *s, size_t length);

int BufferedPrintfC(void *stream, int size, const char *format, va_list arg)
{
    int r;
    char buffer[512];
    if (size >= 512) {
        return -1;
    }
    memset(buffer, 0, size);
    r = vsprintf(buffer, format, arg);
    // this may not hit the heap but should alert the user anyways
    if (r > (int32_t) size) {
        error("%s %d buffer overwrite (max_buf_size: %d exceeded: %d)!\r\n", __FILE__, __LINE__, size, r);
        return 0;
    }
    if (r > 0) {
        BufferedSpiThunk(stream, buffer, r);
    }
    return r;
}
