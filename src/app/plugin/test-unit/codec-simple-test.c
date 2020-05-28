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

#include "ChipZclUnitTests.h"

#include <chip-zcl-codec.h>
#include <chip-zcl.h>

#include "gen.h"

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int testCodecSimple()
{
    // Let's assume we are encoding and decoding a 100 byte buffer that contains
    // the following data:
    //   unsigned int, 1 byte, value = 0x13
    //   unsigned int, 2 bytes, value = 0x4231
    //   unsigned int, 4 bytes, value = 0xABCD1234
    //   unsigned length-prefixed string, 1 byte length, value of string "This is an example of the string."
    //
    ChipZclBuffer_t * buffer = chipZclBufferAlloc(1000);
    uint8_t num1             = 0x13;
    uint16_t num2            = 0x4231;
    uint32_t num4            = 0xABCD1234;
    char * sIn               = "Test data is encoded and decoded back.";
    char * sOut              = malloc(100);
    uint16_t ret;
    ChipZclCodec_t codec;

    chipZclCodecEncodeStart(&codec, buffer);
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &num1, 1);
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &num2, 2);
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &num4, 4);
    chipZclCodecEncode(&codec, CHIP_ZCL_STRUCT_TYPE_STRING, sIn, strlen(sIn));
    chipZclCodecEncodeEnd(&codec);

    chipZclCodecDecodeStart(&codec, buffer);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &num1, 1, &ret);
    if (num1 != 0x13)
    {
        printf("Failure: num1=%d\n", num1);
        return 1;
    }
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &num2, 2, &ret);
    if (num2 != 0x4231)
    {
        printf("Failure: num2=%d\n", num2);
        return 1;
    }
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &num4, 4, &ret);
    if (num4 != 0xABCD1234)
    {
        printf("Failure: num4=%d\n", num4);
        return 1;
    }
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_STRING, sOut, 100, &ret);
    if (memcmp(sIn, sOut, ret))
    {
        printf("Failure: sOut=%s\n", sOut);
        return 1;
    }
    chipZclCodecDecodeEnd(&codec);

    chipZclBufferFree(buffer);

    printf("Success: %s\n", sOut);
    return 0;
}
