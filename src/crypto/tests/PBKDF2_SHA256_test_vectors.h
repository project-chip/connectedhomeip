/*
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
 * @file - This file contains PBKDF2 SHA256 test vectors.
 */

#pragma once

#include <stddef.h>

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPSafeCasts.h>

struct pbkdf2_test_vector
{
    const uint8_t * password;
    size_t plen;
    const uint8_t * salt;
    size_t slen;
    unsigned int iter;
    uint32_t key_len;
    const uint8_t * key;
    unsigned tcId;
    CHIP_ERROR result;
};

static const uint8_t chiptest_key1[]                          = { 0x9a, 0x87, 0xd2, 0x5e, 0x37, 0x40, 0x8f, 0xd7, 0x2f, 0x42,
                                         0x3d, 0x22, 0x85, 0xc7, 0x08, 0x6d, 0x3a, 0x64, 0x2a, 0x26 };
static const struct pbkdf2_test_vector chiptest_test_vector_1 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                  .slen     = 8,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key1,
                                                                  .tcId     = 1,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key2[]                          = { 0xa0, 0xf3, 0x0e, 0x25, 0xe8, 0x65, 0x9c, 0x36, 0xa0, 0x95,
                                         0x89, 0x49, 0x0d, 0x64, 0xa6, 0xb1, 0x2e, 0x0c, 0x38, 0x3c };
static const struct pbkdf2_test_vector chiptest_test_vector_2 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                  .slen     = 8,
                                                                  .iter     = 2,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key2,
                                                                  .tcId     = 2,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key3[]                          = { 0x86, 0x16, 0x14, 0x28, 0x77, 0x14, 0xd4, 0x29, 0x75, 0x16,
                                         0x5e, 0xb6, 0x3e, 0xa7, 0x8c, 0xce, 0x85, 0x50, 0x71, 0x3c };
static const struct pbkdf2_test_vector chiptest_test_vector_3 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                  .slen     = 8,
                                                                  .iter     = 5,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key3,
                                                                  .tcId     = 3,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key4[]                          = { 0x3f, 0x12, 0x78, 0x37, 0x88, 0x9c, 0x4a, 0x62, 0x3d, 0x6e,
                                         0x8d, 0xc8, 0x8c, 0xa3, 0x3c, 0x01, 0x23, 0xa2, 0x9f, 0x07 };
static const struct pbkdf2_test_vector chiptest_test_vector_4 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                  .slen     = 8,
                                                                  .iter     = 3,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 4,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key5[] = { 0x28, 0x1a, 0xef, 0x26, 0xfa, 0x2d, 0xdf, 0x5b, 0x76, 0x82, 0x01, 0xff, 0x3a,
                                         0xdd, 0xfc, 0x68, 0x0a, 0xda, 0x07, 0x87, 0xc9, 0xb4, 0x76, 0x55, 0x42 };
static const struct pbkdf2_test_vector chiptest_test_vector_5 = { .password =
                                                                      chip::Uint8::from_const_char("passwordPASSWORDpassword"),
                                                                  .plen    = 24,
                                                                  .salt    = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen    = 16,
                                                                  .iter    = 10,
                                                                  .key_len = 25,
                                                                  .key     = chiptest_key5,
                                                                  .tcId    = 5,
                                                                  .result  = CHIP_NO_ERROR };

static const struct pbkdf2_test_vector chiptest_test_vector_6 = { .password = nullptr,
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                  .slen     = 8,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 6,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_7 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 0,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                  .slen     = 8,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 7,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_8 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = nullptr,
                                                                  .slen     = 8,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 8,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_9 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 0,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                  .slen     = 8,
                                                                  .iter     = 1,
                                                                  .key_len  = 0,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 9,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_10 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 0,
                                                                   .salt     = chip::Uint8::from_const_char("saltSALT"),
                                                                   .slen     = 8,
                                                                   .iter     = 1,
                                                                   .key_len  = 20,
                                                                   .key      = nullptr,
                                                                   .tcId     = 10,
                                                                   .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_11 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 8,
                                                                   .salt     = nullptr,
                                                                   .slen     = 0,
                                                                   .iter     = 1,
                                                                   .key_len  = 20,
                                                                   .key      = chiptest_key4,
                                                                   .tcId     = 11,
                                                                   .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_12 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 8,
                                                                   .salt     = chip::Uint8::from_const_char("saltSAL"),
                                                                   .slen     = 7,
                                                                   .iter     = 1,
                                                                   .key_len  = 20,
                                                                   .key      = chiptest_key4,
                                                                   .tcId     = 12,
                                                                   .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_13 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 8,
                                                                   .salt     = chip::Uint8::from_const_char("saltSALTsaltSALTs"),
                                                                   .slen     = 17,
                                                                   .iter     = 1,
                                                                   .key_len  = 20,
                                                                   .key      = chiptest_key4,
                                                                   .tcId     = 13,
                                                                   .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector * pbkdf2_sha256_test_vectors[] = {
    &chiptest_test_vector_1, &chiptest_test_vector_2, &chiptest_test_vector_3,
#if !CHIP_TARGET_STYLE_EMBEDDED
    // The following test vector takes excessive time to run on an embedded target
    &chiptest_test_vector_4,
#endif
    &chiptest_test_vector_5, &chiptest_test_vector_6, &chiptest_test_vector_7, &chiptest_test_vector_8, &chiptest_test_vector_9,
    &chiptest_test_vector_10, &chiptest_test_vector_11, &chiptest_test_vector_12, &chiptest_test_vector_13
};
