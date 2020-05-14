/**
 *
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

/**
 *    @file
 *      This file provides unit test code for the simple codec
 *
 */

#include <stdlib.h>
#include <memory.h>
#include "utest.h" /* TODO pass all that stuff in -D */
#include "../../api/chip-zcl.h"

int main()
{
    // Let's assume we are encoding and decoding a 100 byte buffer that contains
    // the following data:
    //   unsigned int, 1 byte, value = 0x13
    //   unsigned int, 2 bytes, value = 0x4231
    //   unsigned int, 4 bytes, value = 0xABCD1234
    //   unsigned length-prefixed string, 1 byte length, value of string "This is an example of the string."
    //
    ChipZclRawBuffer_t * buffer = chipZclBufferAlloc(1000);

    printf("Success \n");

    chipZclBufferFree(buffer);

    return 0;
}
