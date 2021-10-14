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

static const uint8_t chiptest_key1[]                          = { 0xf2, 0xe3, 0x4b, 0xd9, 0x50, 0xe9, 0x1c, 0xf3, 0x7d, 0x22,
                                         0xe1, 0x13, 0x5a, 0x39, 0x9b, 0x02, 0xa1, 0x7c, 0xb1, 0x93 };
static const struct pbkdf2_test_vector chiptest_test_vector_1 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen     = 16,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key1,
                                                                  .tcId     = 1,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key2[]                          = { 0x2b, 0x77, 0x27, 0x5c, 0xc3, 0x12, 0x0b, 0x15, 0x13, 0xf6,
                                         0xf3, 0xe0, 0x36, 0x49, 0xfd, 0x49, 0x33, 0x76, 0x52, 0x60 };
static const struct pbkdf2_test_vector chiptest_test_vector_2 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen     = 16,
                                                                  .iter     = 2,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key2,
                                                                  .tcId     = 2,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key3[]                          = { 0x82, 0xff, 0xaa, 0xfc, 0x0b, 0x04, 0x91, 0x80, 0xee, 0xa7,
                                         0x9a, 0x04, 0x10, 0x31, 0x58, 0x87, 0xb6, 0x60, 0xac, 0x7e };
static const struct pbkdf2_test_vector chiptest_test_vector_3 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen     = 16,
                                                                  .iter     = 5,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key3,
                                                                  .tcId     = 3,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key4[]                          = { 0xf8, 0x8a, 0xfb, 0xb7, 0x9d, 0xda, 0x3f, 0x28, 0x2e, 0x21,
                                         0xad, 0xf2, 0x53, 0xd0, 0xe9, 0xf1, 0x70, 0x82, 0x3a, 0x9f };
static const struct pbkdf2_test_vector chiptest_test_vector_4 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen     = 16,
                                                                  .iter     = 3,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 4,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key5[] = { 0x0d, 0xbf, 0x87, 0x38, 0xd2, 0x30, 0xbf, 0x28, 0xba, 0xe0, 0xfb, 0x4d, 0x8f,
                                         0x07, 0x34, 0x98, 0x24, 0xb5, 0xe0, 0xb1, 0xa7, 0x0b, 0xa2, 0x19, 0x3b };
static const struct pbkdf2_test_vector chiptest_test_vector_5 = {
    .password = chip::Uint8::from_const_char("passwordPASSWORDpassword"),
    .plen     = 24,
    .salt     = chip::Uint8::from_const_char("saltSALTsaltSALTsaltSALTsaltSALT"),
    .slen     = 32,
    .iter     = 10,
    .key_len  = 25,
    .key      = chiptest_key5,
    .tcId     = 5,
    .result   = CHIP_NO_ERROR
};

static const struct pbkdf2_test_vector chiptest_test_vector_6 = { .password = nullptr,
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen     = 16,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 6,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_7 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 0,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen     = 16,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 7,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_8 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = nullptr,
                                                                  .slen     = 16,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 8,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_9 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 0,
                                                                  .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                  .slen     = 16,
                                                                  .iter     = 1,
                                                                  .key_len  = 0,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 9,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_10 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 0,
                                                                   .salt     = chip::Uint8::from_const_char("saltSALTsaltSALT"),
                                                                   .slen     = 16,
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
                                                                   .salt     = chip::Uint8::from_const_char("saltSALTsaltSAL"),
                                                                   .slen     = 15,
                                                                   .iter     = 1,
                                                                   .key_len  = 20,
                                                                   .key      = chiptest_key4,
                                                                   .tcId     = 12,
                                                                   .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_13 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 8,
                                                                   .salt     = chip::Uint8::from_const_char(
                                                                       "saltSALTsaltSALTsaltSALTsaltSALTs"),
                                                                   .slen    = 33,
                                                                   .iter    = 1,
                                                                   .key_len = 20,
                                                                   .key     = chiptest_key4,
                                                                   .tcId    = 13,
                                                                   .result  = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector * pbkdf2_sha256_test_vectors[] = {
    &chiptest_test_vector_1, &chiptest_test_vector_2, &chiptest_test_vector_3,
#if !CHIP_TARGET_STYLE_EMBEDDED
    // The following test vector takes excessive time to run on an embedded target
    &chiptest_test_vector_4,
#endif
    &chiptest_test_vector_5, &chiptest_test_vector_6, &chiptest_test_vector_7, &chiptest_test_vector_8, &chiptest_test_vector_9,
    &chiptest_test_vector_10, &chiptest_test_vector_11, &chiptest_test_vector_12, &chiptest_test_vector_13
};
