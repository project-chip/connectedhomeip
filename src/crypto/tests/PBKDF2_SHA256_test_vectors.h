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

#ifndef _PBKDF2_SHA256_TEST_VECTORS_H_
#define _PBKDF2_SHA256_TEST_VECTORS_H_

#include <stddef.h>

#include <core/CHIPError.h>
#include <core/CHIPSafeCasts.h>

struct pbkdf2_test_vector
{
    const uint8_t * password;
    size_t plen;
    const uint8_t * salt;
    size_t slen;
    unsigned int iter;
    size_t key_len;
    const uint8_t * key;
    unsigned tcId;
    int result;
};

static const uint8_t chiptest_key1[]                          = { 0x12, 0x0f, 0xb6, 0xcf, 0xfc, 0xf8, 0xb3, 0x2c, 0x43, 0xe7,
                                         0x22, 0x52, 0x56, 0xc4, 0xf8, 0x37, 0xa8, 0x65, 0x48, 0xc9 };
static const struct pbkdf2_test_vector chiptest_test_vector_1 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("salt"),
                                                                  .slen     = 4,
                                                                  .iter     = 1,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key1,
                                                                  .tcId     = 1,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key2[]                          = { 0xae, 0x4d, 0x0c, 0x95, 0xaf, 0x6b, 0x46, 0xd3, 0x2d, 0x0a,
                                         0xdf, 0xf9, 0x28, 0xf0, 0x6d, 0xd0, 0x2a, 0x30, 0x3f, 0x8e };
static const struct pbkdf2_test_vector chiptest_test_vector_2 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("salt"),
                                                                  .slen     = 4,
                                                                  .iter     = 2,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key2,
                                                                  .tcId     = 2,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key3[]                          = { 0xc5, 0xe4, 0x78, 0xd5, 0x92, 0x88, 0xc8, 0x41, 0xaa, 0x53,
                                         0x0d, 0xb6, 0x84, 0x5c, 0x4c, 0x8d, 0x96, 0x28, 0x93, 0xa0 };
static const struct pbkdf2_test_vector chiptest_test_vector_3 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("salt"),
                                                                  .slen     = 4,
                                                                  .iter     = 4096,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key3,
                                                                  .tcId     = 3,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key4[]                          = { 0xad, 0x35, 0x24, 0x0a, 0xc6, 0x83, 0xfe, 0xbf, 0xaf, 0x3c,
                                         0xd4, 0x9d, 0x84, 0x54, 0x73, 0xfb, 0xbb, 0xaa, 0x24, 0x37 };
static const struct pbkdf2_test_vector chiptest_test_vector_4 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("salt"),
                                                                  .slen     = 4,
                                                                  .iter     = 3,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 4,
                                                                  .result   = CHIP_NO_ERROR };

static const uint8_t chiptest_key5[] = { 0x34, 0x8c, 0x89, 0xdb, 0xcb, 0xd3, 0x2b, 0x2f, 0x32, 0xd8, 0x14, 0xb8, 0x11,
                                         0x6e, 0x84, 0xcf, 0x2b, 0x17, 0x34, 0x7e, 0xbc, 0x18, 0x00, 0x18, 0x1c };
static const struct pbkdf2_test_vector chiptest_test_vector_5 = {
    .password = chip::Uint8::from_const_char("passwordPASSWORDpassword"),
    .plen     = 24,
    .salt     = chip::Uint8::from_const_char("saltSALTsaltSALTsaltSALTsaltSALTsalt"),
    .slen     = 36,
    .iter     = 4096,
    .key_len  = 25,
    .key      = chiptest_key5,
    .tcId     = 5,
    .result   = CHIP_NO_ERROR
};

static const struct pbkdf2_test_vector chiptest_test_vector_6 = { .password = nullptr,
                                                                  .plen     = 8,
                                                                  .salt     = chip::Uint8::from_const_char("salt"),
                                                                  .slen     = 4,
                                                                  .iter     = 16777216,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 6,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_7 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 0,
                                                                  .salt     = chip::Uint8::from_const_char("salt"),
                                                                  .slen     = 4,
                                                                  .iter     = 16777216,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 7,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_8 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 8,
                                                                  .salt     = nullptr,
                                                                  .slen     = 4,
                                                                  .iter     = 16777216,
                                                                  .key_len  = 20,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 8,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_9 = { .password = chip::Uint8::from_const_char("password"),
                                                                  .plen     = 0,
                                                                  .salt     = chip::Uint8::from_const_char("salt"),
                                                                  .slen     = 4,
                                                                  .iter     = 16777216,
                                                                  .key_len  = 0,
                                                                  .key      = chiptest_key4,
                                                                  .tcId     = 9,
                                                                  .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_10 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 0,
                                                                   .salt     = chip::Uint8::from_const_char("salt"),
                                                                   .slen     = 4,
                                                                   .iter     = 16777216,
                                                                   .key_len  = 20,
                                                                   .key      = NULL,
                                                                   .tcId     = 10,
                                                                   .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector chiptest_test_vector_11 = { .password = chip::Uint8::from_const_char("password"),
                                                                   .plen     = 8,
                                                                   .salt     = nullptr,
                                                                   .slen     = 0,
                                                                   .iter     = 16777216,
                                                                   .key_len  = 20,
                                                                   .key      = chiptest_key4,
                                                                   .tcId     = 8,
                                                                   .result   = CHIP_ERROR_INVALID_ARGUMENT };

static const struct pbkdf2_test_vector * pbkdf2_sha256_test_vectors[] = {
    &chiptest_test_vector_1, &chiptest_test_vector_2, &chiptest_test_vector_3,
#if !CHIP_TARGET_STYLE_EMBEDDED
    // The following test vector takes excessive time to run on an embedded target
    &chiptest_test_vector_4,
#endif
    &chiptest_test_vector_5, &chiptest_test_vector_6, &chiptest_test_vector_7, &chiptest_test_vector_8, &chiptest_test_vector_9,
    &chiptest_test_vector_10, &chiptest_test_vector_11
};

#endif // _PBKDF2_SHA256_TEST_VECTORS_H_
