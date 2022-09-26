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
 * @file - This file contains AES-CCM test vectors.
 */

#pragma once

#include <stddef.h>

#include <lib/core/CHIPError.h>

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif // CHIP_HAVE_CONFIG_H

struct ccm_128_test_vector
{
    const uint8_t * pt;
    size_t pt_len;
    const uint8_t * aad;
    size_t aad_len;
    const uint8_t * key;
    size_t key_len;
    const uint8_t * nonce;
    size_t nonce_len;
    const uint8_t * ct;
    size_t ct_len;
    const uint8_t * tag;
    size_t tag_len;
    unsigned tcId;
    CHIP_ERROR result;
};

static const uint8_t chiptest_dac9e1195a0d_pt_1[]                           = { 0xee };
static const uint8_t chiptest_dac9e1195a0d_aad_2[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_3[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                       0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_4[]                        = { 0x75, 0xc7, 0xc1, 0x23, 0xb3, 0x9b, 0x8a, 0x7e };
static const uint8_t chiptest_dac9e1195a0d_ct_5[]                           = { 0x74 };
static const uint8_t chiptest_dac9e1195a0d_tag_6[]                          = { 0x32, 0xf0, 0xc8, 0xf7, 0xdc, 0x07, 0x7c, 0xb8 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_7 = { .pt        = chiptest_dac9e1195a0d_pt_1,
                                                                                .pt_len    = 1,
                                                                                .aad       = chiptest_dac9e1195a0d_aad_2,
                                                                                .aad_len   = 0,
                                                                                .key       = chiptest_dac9e1195a0d_key_3,
                                                                                .key_len   = 16,
                                                                                .nonce     = chiptest_dac9e1195a0d_nonce_4,
                                                                                .nonce_len = 8,
                                                                                .ct        = chiptest_dac9e1195a0d_ct_5,
                                                                                .ct_len    = 1,
                                                                                .tag       = chiptest_dac9e1195a0d_tag_6,
                                                                                .tag_len   = 8,
                                                                                .tcId      = 1,
                                                                                .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_8[]                           = { 0xee };
static const uint8_t chiptest_dac9e1195a0d_aad_9[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_10[]                         = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_11[]                       = { 0x75, 0xc7, 0xc1, 0x23, 0xb3, 0x9b, 0x8a, 0x7e };
static const uint8_t chiptest_dac9e1195a0d_ct_12[]                          = { 0x74 };
static const uint8_t chiptest_dac9e1195a0d_tag_13[] = { 0xc0, 0x1c, 0x51, 0x81, 0x8d, 0x2a, 0x93, 0x57, 0xb4, 0x69, 0x2b, 0x24 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_14 = { .pt        = chiptest_dac9e1195a0d_pt_8,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_9,
                                                                                 .aad_len   = 0,
                                                                                 .key       = chiptest_dac9e1195a0d_key_10,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_11,
                                                                                 .nonce_len = 8,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_12,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_13,
                                                                                 .tag_len   = 12,
                                                                                 .tcId      = 2,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_15[]                           = { 0xee };
static const uint8_t chiptest_dac9e1195a0d_aad_16[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_17[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_18[]                        = { 0x75, 0xc7, 0xc1, 0x23, 0xb3, 0x9b, 0x8a, 0x7e };
static const uint8_t chiptest_dac9e1195a0d_ct_19[]                           = { 0x74 };
static const uint8_t chiptest_dac9e1195a0d_tag_20[]                          = { 0x35, 0x5e, 0x27, 0x43, 0xb6, 0x79, 0x36, 0x94,
                                                        0xca, 0x78, 0xd6, 0xd9, 0xc8, 0xdc, 0x14, 0x6c };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_21 = { .pt        = chiptest_dac9e1195a0d_pt_15,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_16,
                                                                                 .aad_len   = 0,
                                                                                 .key       = chiptest_dac9e1195a0d_key_17,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_18,
                                                                                 .nonce_len = 8,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_19,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_20,
                                                                                 .tag_len   = 16,
                                                                                 .tcId      = 3,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_22[]                           = { 0xcd };
static const uint8_t chiptest_dac9e1195a0d_aad_23[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_24[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_25[] = { 0x56, 0x59, 0x39, 0x7e, 0x06, 0x02, 0x8b, 0x14, 0x9d, 0x0b, 0x2b, 0x2b };
static const uint8_t chiptest_dac9e1195a0d_ct_26[]    = { 0xf5 };
static const uint8_t chiptest_dac9e1195a0d_tag_27[]   = { 0x24, 0x70, 0x9e, 0xaf, 0x87, 0x95, 0x80, 0xe8 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_28 = { .pt        = chiptest_dac9e1195a0d_pt_22,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_23,
                                                                                 .aad_len   = 0,
                                                                                 .key       = chiptest_dac9e1195a0d_key_24,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_25,
                                                                                 .nonce_len = 12,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_26,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_27,
                                                                                 .tag_len   = 8,
                                                                                 .tcId      = 4,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_29[]                           = { 0xcd };
static const uint8_t chiptest_dac9e1195a0d_aad_30[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_31[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_32[] = { 0x56, 0x59, 0x39, 0x7e, 0x06, 0x02, 0x8b, 0x14, 0x9d, 0x0b, 0x2b, 0x2b };
static const uint8_t chiptest_dac9e1195a0d_ct_33[]    = { 0xf5 };
static const uint8_t chiptest_dac9e1195a0d_tag_34[]   = { 0x9a, 0x23, 0xec, 0x3e, 0xac, 0xef, 0x72, 0xc2, 0x2c, 0x6a, 0x37, 0x08 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_35 = { .pt        = chiptest_dac9e1195a0d_pt_29,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_30,
                                                                                 .aad_len   = 0,
                                                                                 .key       = chiptest_dac9e1195a0d_key_31,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_32,
                                                                                 .nonce_len = 12,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_33,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_34,
                                                                                 .tag_len   = 12,
                                                                                 .tcId      = 5,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_36[]                           = { 0xcd };
static const uint8_t chiptest_dac9e1195a0d_aad_37[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_38[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_39[] = { 0x56, 0x59, 0x39, 0x7e, 0x06, 0x02, 0x8b, 0x14, 0x9d, 0x0b, 0x2b, 0x2b };
static const uint8_t chiptest_dac9e1195a0d_ct_40[]    = { 0xf5 };
static const uint8_t chiptest_dac9e1195a0d_tag_41[]   = { 0x9f, 0x5c, 0xd7, 0x4a, 0x83, 0x60, 0xc7, 0x76,
                                                        0xe7, 0x16, 0x7d, 0xe2, 0xdb, 0xef, 0xac, 0x98 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_42 = { .pt        = chiptest_dac9e1195a0d_pt_36,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_37,
                                                                                 .aad_len   = 0,
                                                                                 .key       = chiptest_dac9e1195a0d_key_38,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_39,
                                                                                 .nonce_len = 12,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_40,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_41,
                                                                                 .tag_len   = 16,
                                                                                 .tcId      = 6,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_43[]                           = { 0xf4 };
static const uint8_t chiptest_dac9e1195a0d_aad_44[]                          = { 0xb3, 0x3f, 0xa8, 0xae, 0x4b, 0xc6, 0xea, 0xe4,
                                                        0x5f, 0x28, 0x61, 0x9a, 0xc8, 0xd3, 0xae, 0x79 };
static const uint8_t chiptest_dac9e1195a0d_key_45[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_46[]                        = { 0x6d, 0x4c, 0x91, 0x17, 0x0d, 0xc6, 0x21, 0x7e };
static const uint8_t chiptest_dac9e1195a0d_ct_47[]                           = { 0x7f };
static const uint8_t chiptest_dac9e1195a0d_tag_48[]                          = { 0xc6, 0x3e, 0x43, 0xfc, 0x80, 0x03, 0x60, 0xf5 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_49 = { .pt        = chiptest_dac9e1195a0d_pt_43,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_44,
                                                                                 .aad_len   = 16,
                                                                                 .key       = chiptest_dac9e1195a0d_key_45,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_46,
                                                                                 .nonce_len = 8,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_47,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_48,
                                                                                 .tag_len   = 8,
                                                                                 .tcId      = 7,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_50[]                           = { 0xf4 };
static const uint8_t chiptest_dac9e1195a0d_aad_51[]                          = { 0xb3, 0x3f, 0xa8, 0xae, 0x4b, 0xc6, 0xea, 0xe4,
                                                        0x5f, 0x28, 0x61, 0x9a, 0xc8, 0xd3, 0xae, 0x79 };
static const uint8_t chiptest_dac9e1195a0d_key_52[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_53[]                        = { 0x6d, 0x4c, 0x91, 0x17, 0x0d, 0xc6, 0x21, 0x7e };
static const uint8_t chiptest_dac9e1195a0d_ct_54[]                           = { 0x7f };
static const uint8_t chiptest_dac9e1195a0d_tag_55[] = { 0xcf, 0x3d, 0x2c, 0xe6, 0x3c, 0xf3, 0x38, 0x07, 0x67, 0x36, 0xd1, 0x1c };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_56 = { .pt        = chiptest_dac9e1195a0d_pt_50,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_51,
                                                                                 .aad_len   = 16,
                                                                                 .key       = chiptest_dac9e1195a0d_key_52,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_53,
                                                                                 .nonce_len = 8,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_54,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_55,
                                                                                 .tag_len   = 12,
                                                                                 .tcId      = 8,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_57[]                           = { 0xf4 };
static const uint8_t chiptest_dac9e1195a0d_aad_58[]                          = { 0xb3, 0x3f, 0xa8, 0xae, 0x4b, 0xc6, 0xea, 0xe4,
                                                        0x5f, 0x28, 0x61, 0x9a, 0xc8, 0xd3, 0xae, 0x79 };
static const uint8_t chiptest_dac9e1195a0d_key_59[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_60[]                        = { 0x6d, 0x4c, 0x91, 0x17, 0x0d, 0xc6, 0x21, 0x7e };
static const uint8_t chiptest_dac9e1195a0d_ct_61[]                           = { 0x7f };
static const uint8_t chiptest_dac9e1195a0d_tag_62[]                          = { 0x08, 0x85, 0x6b, 0x4a, 0x5d, 0x73, 0x56, 0xc4,
                                                        0xf0, 0x46, 0x3e, 0xa9, 0x0e, 0x3c, 0x36, 0x77 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_63 = { .pt        = chiptest_dac9e1195a0d_pt_57,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_58,
                                                                                 .aad_len   = 16,
                                                                                 .key       = chiptest_dac9e1195a0d_key_59,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_60,
                                                                                 .nonce_len = 8,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_61,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_62,
                                                                                 .tag_len   = 16,
                                                                                 .tcId      = 9,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_64[]                           = { 0x38 };
static const uint8_t chiptest_dac9e1195a0d_aad_65[]                          = { 0xf4, 0xc7, 0xa8, 0x88, 0x22, 0x1e, 0xef, 0xf4,
                                                        0xc8, 0x15, 0x51, 0xdc, 0x15, 0xd4, 0x10, 0xfb };
static const uint8_t chiptest_dac9e1195a0d_key_66[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_67[] = { 0x7f, 0xf3, 0x44, 0xc6, 0xb3, 0x64, 0xc4, 0x4e, 0x4f, 0xe0, 0xaa, 0xa7 };
static const uint8_t chiptest_dac9e1195a0d_ct_68[]    = { 0x55 };
static const uint8_t chiptest_dac9e1195a0d_tag_69[]   = { 0x4b, 0xae, 0xdd, 0xe7, 0xe8, 0xc1, 0x48, 0xed };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_70 = { .pt        = chiptest_dac9e1195a0d_pt_64,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_65,
                                                                                 .aad_len   = 16,
                                                                                 .key       = chiptest_dac9e1195a0d_key_66,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_67,
                                                                                 .nonce_len = 12,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_68,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_69,
                                                                                 .tag_len   = 8,
                                                                                 .tcId      = 10,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_71[]                           = { 0x38 };
static const uint8_t chiptest_dac9e1195a0d_aad_72[]                          = { 0xf4, 0xc7, 0xa8, 0x88, 0x22, 0x1e, 0xef, 0xf4,
                                                        0xc8, 0x15, 0x51, 0xdc, 0x15, 0xd4, 0x10, 0xfb };
static const uint8_t chiptest_dac9e1195a0d_key_73[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_74[] = { 0x7f, 0xf3, 0x44, 0xc6, 0xb3, 0x64, 0xc4, 0x4e, 0x4f, 0xe0, 0xaa, 0xa7 };
static const uint8_t chiptest_dac9e1195a0d_ct_75[]    = { 0x55 };
static const uint8_t chiptest_dac9e1195a0d_tag_76[]   = { 0xdd, 0x3a, 0x04, 0xbc, 0xe4, 0x50, 0xa7, 0xe9, 0x84, 0x26, 0xd2, 0x5c };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_77 = { .pt        = chiptest_dac9e1195a0d_pt_71,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_72,
                                                                                 .aad_len   = 16,
                                                                                 .key       = chiptest_dac9e1195a0d_key_73,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_74,
                                                                                 .nonce_len = 12,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_75,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_76,
                                                                                 .tag_len   = 12,
                                                                                 .tcId      = 11,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_78[]                           = { 0x38 };
static const uint8_t chiptest_dac9e1195a0d_aad_79[]                          = { 0xf4, 0xc7, 0xa8, 0x88, 0x22, 0x1e, 0xef, 0xf4,
                                                        0xc8, 0x15, 0x51, 0xdc, 0x15, 0xd4, 0x10, 0xfb };
static const uint8_t chiptest_dac9e1195a0d_key_80[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_81[] = { 0x7f, 0xf3, 0x44, 0xc6, 0xb3, 0x64, 0xc4, 0x4e, 0x4f, 0xe0, 0xaa, 0xa7 };
static const uint8_t chiptest_dac9e1195a0d_ct_82[]    = { 0x55 };
static const uint8_t chiptest_dac9e1195a0d_tag_83[]   = { 0x20, 0xc7, 0xb9, 0x71, 0xfb, 0xf3, 0x6f, 0x9c,
                                                        0x44, 0x24, 0x78, 0x12, 0xd3, 0x99, 0xb1, 0x45 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_84 = { .pt        = chiptest_dac9e1195a0d_pt_78,
                                                                                 .pt_len    = 1,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_79,
                                                                                 .aad_len   = 16,
                                                                                 .key       = chiptest_dac9e1195a0d_key_80,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_81,
                                                                                 .nonce_len = 12,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_82,
                                                                                 .ct_len    = 1,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_83,
                                                                                 .tag_len   = 16,
                                                                                 .tcId      = 12,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_85[]                           = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90,
                                                       0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
static const uint8_t chiptest_dac9e1195a0d_aad_86[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_87[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_88[]                        = { 0xf2, 0xf0, 0x11, 0x4f, 0xe7, 0x9a, 0x24, 0xfb };
static const uint8_t chiptest_dac9e1195a0d_ct_89[]                           = { 0x17, 0x8d, 0xfc, 0xe4, 0xe8, 0x7b, 0xeb, 0x87,
                                                       0x63, 0xe3, 0xdd, 0xc8, 0x68, 0xfa, 0x73, 0x88 };
static const uint8_t chiptest_dac9e1195a0d_tag_90[]                          = { 0x7f, 0x10, 0xcf, 0x6c, 0x9b, 0x22, 0xb3, 0x96 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_91 = { .pt        = chiptest_dac9e1195a0d_pt_85,
                                                                                 .pt_len    = 16,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_86,
                                                                                 .aad_len   = 0,
                                                                                 .key       = chiptest_dac9e1195a0d_key_87,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_88,
                                                                                 .nonce_len = 8,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_89,
                                                                                 .ct_len    = 16,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_90,
                                                                                 .tag_len   = 8,
                                                                                 .tcId      = 13,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_92[]                           = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90,
                                                       0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
static const uint8_t chiptest_dac9e1195a0d_aad_93[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_94[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                        0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_95[]                        = { 0xf2, 0xf0, 0x11, 0x4f, 0xe7, 0x9a, 0x24, 0xfb };
static const uint8_t chiptest_dac9e1195a0d_ct_96[]                           = { 0x17, 0x8d, 0xfc, 0xe4, 0xe8, 0x7b, 0xeb, 0x87,
                                                       0x63, 0xe3, 0xdd, 0xc8, 0x68, 0xfa, 0x73, 0x88 };
static const uint8_t chiptest_dac9e1195a0d_tag_97[] = { 0x6f, 0x4c, 0x9a, 0x68, 0x32, 0x37, 0x33, 0x1d, 0x6a, 0x98, 0x74, 0xbc };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_98  = { .pt        = chiptest_dac9e1195a0d_pt_92,
                                                                                 .pt_len    = 16,
                                                                                 .aad       = chiptest_dac9e1195a0d_aad_93,
                                                                                 .aad_len   = 0,
                                                                                 .key       = chiptest_dac9e1195a0d_key_94,
                                                                                 .key_len   = 16,
                                                                                 .nonce     = chiptest_dac9e1195a0d_nonce_95,
                                                                                 .nonce_len = 8,
                                                                                 .ct        = chiptest_dac9e1195a0d_ct_96,
                                                                                 .ct_len    = 16,
                                                                                 .tag       = chiptest_dac9e1195a0d_tag_97,
                                                                                 .tag_len   = 12,
                                                                                 .tcId      = 14,
                                                                                 .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_99[]                            = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90,
                                                       0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
static const uint8_t chiptest_dac9e1195a0d_aad_100[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_101[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_102[]                        = { 0xf2, 0xf0, 0x11, 0x4f, 0xe7, 0x9a, 0x24, 0xfb };
static const uint8_t chiptest_dac9e1195a0d_ct_103[]                           = { 0x17, 0x8d, 0xfc, 0xe4, 0xe8, 0x7b, 0xeb, 0x87,
                                                        0x63, 0xe3, 0xdd, 0xc8, 0x68, 0xfa, 0x73, 0x88 };
static const uint8_t chiptest_dac9e1195a0d_tag_104[]                          = { 0x7e, 0x04, 0x2a, 0x69, 0xd0, 0x1b, 0x87, 0x26,
                                                         0xac, 0xe1, 0x31, 0xde, 0x34, 0x22, 0xc6, 0xae };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_105 = { .pt        = chiptest_dac9e1195a0d_pt_99,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_100,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_101,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_102,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_103,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_104,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 14,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_106[]                           = { 0xab, 0xf9, 0x8a, 0x73, 0x5c, 0xd5, 0x47, 0x8b,
                                                        0x9d, 0x6d, 0x3d, 0xdf, 0x02, 0x56, 0x5a, 0xe0 };
static const uint8_t chiptest_dac9e1195a0d_aad_107[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_108[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_109[] = { 0xc7, 0xcf, 0x09, 0xa8, 0xe2, 0x3b, 0xaa, 0xc1, 0xb6, 0x95, 0xc0, 0x42 };
static const uint8_t chiptest_dac9e1195a0d_ct_110[]    = { 0xf8, 0x86, 0xc0, 0x74, 0xae, 0x03, 0xe9, 0x02,
                                                        0xf3, 0x3d, 0x0c, 0x91, 0x9e, 0xd7, 0xb5, 0xee };
static const uint8_t chiptest_dac9e1195a0d_tag_111[]   = { 0x44, 0x45, 0x35, 0xc9, 0x73, 0xc9, 0x2b, 0xbb };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_112 = { .pt        = chiptest_dac9e1195a0d_pt_106,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_107,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_108,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_109,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_110,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_111,
                                                                                  .tag_len   = 8,
                                                                                  .tcId      = 15,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_113[]                           = { 0xab, 0xf9, 0x8a, 0x73, 0x5c, 0xd5, 0x47, 0x8b,
                                                        0x9d, 0x6d, 0x3d, 0xdf, 0x02, 0x56, 0x5a, 0xe0 };
static const uint8_t chiptest_dac9e1195a0d_aad_114[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_115[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_116[] = { 0xc7, 0xcf, 0x09, 0xa8, 0xe2, 0x3b, 0xaa, 0xc1, 0xb6, 0x95, 0xc0, 0x42 };
static const uint8_t chiptest_dac9e1195a0d_ct_117[]    = { 0xf8, 0x86, 0xc0, 0x74, 0xae, 0x03, 0xe9, 0x02,
                                                        0xf3, 0x3d, 0x0c, 0x91, 0x9e, 0xd7, 0xb5, 0xee };
static const uint8_t chiptest_dac9e1195a0d_tag_118[]   = { 0xc9, 0xa7, 0x79, 0xa1, 0xaa, 0x43, 0x0c, 0x7a, 0x4a, 0x43, 0x2f, 0x49 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_119 = { .pt        = chiptest_dac9e1195a0d_pt_113,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_114,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_115,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_116,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_117,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_118,
                                                                                  .tag_len   = 12,
                                                                                  .tcId      = 16,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_120[]                           = { 0xab, 0xf9, 0x8a, 0x73, 0x5c, 0xd5, 0x47, 0x8b,
                                                        0x9d, 0x6d, 0x3d, 0xdf, 0x02, 0x56, 0x5a, 0xe0 };
static const uint8_t chiptest_dac9e1195a0d_aad_121[]                          = {};
static const uint8_t chiptest_dac9e1195a0d_key_122[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_123[] = { 0xc7, 0xcf, 0x09, 0xa8, 0xe2, 0x3b, 0xaa, 0xc1, 0xb6, 0x95, 0xc0, 0x42 };
static const uint8_t chiptest_dac9e1195a0d_ct_124[]    = { 0xf8, 0x86, 0xc0, 0x74, 0xae, 0x03, 0xe9, 0x02,
                                                        0xf3, 0x3d, 0x0c, 0x91, 0x9e, 0xd7, 0xb5, 0xee };
static const uint8_t chiptest_dac9e1195a0d_tag_125[]   = { 0x1f, 0x5d, 0xdb, 0xdd, 0x5b, 0xcf, 0xc5, 0x4f,
                                                         0x0c, 0xbc, 0xf4, 0x7f, 0x46, 0x34, 0x63, 0x67 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_126 = { .pt        = chiptest_dac9e1195a0d_pt_120,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_121,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_122,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_123,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_124,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_125,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 17,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_127[]                           = { 0x0a, 0x61, 0xf2, 0xb4, 0xd6, 0x13, 0x5e, 0x2f,
                                                        0x63, 0xd3, 0xae, 0x42, 0xc3, 0x08, 0x08, 0x3e };
static const uint8_t chiptest_dac9e1195a0d_aad_128[]                          = { 0x2c, 0xa7, 0x1b, 0x23, 0x18, 0xdd, 0x96, 0xb2,
                                                         0x43, 0xc8, 0x70, 0xa3, 0xdd, 0xa9, 0xfa, 0x0d };
static const uint8_t chiptest_dac9e1195a0d_key_129[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_130[]                        = { 0x60, 0x66, 0x73, 0x03, 0x90, 0xc3, 0xae, 0x06 };
static const uint8_t chiptest_dac9e1195a0d_ct_131[]                           = { 0xc2, 0xaf, 0x35, 0x3b, 0x99, 0x82, 0xc3, 0x9f,
                                                        0x6d, 0x91, 0x4a, 0xef, 0x8c, 0xf4, 0x97, 0x19 };
static const uint8_t chiptest_dac9e1195a0d_tag_132[]                          = { 0xce, 0xb1, 0x1e, 0x3a, 0x99, 0xeb, 0x3b, 0xcd };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_133 = { .pt        = chiptest_dac9e1195a0d_pt_127,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_128,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_129,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_130,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_131,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_132,
                                                                                  .tag_len   = 8,
                                                                                  .tcId      = 18,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_134[]                           = { 0x0a, 0x61, 0xf2, 0xb4, 0xd6, 0x13, 0x5e, 0x2f,
                                                        0x63, 0xd3, 0xae, 0x42, 0xc3, 0x08, 0x08, 0x3e };
static const uint8_t chiptest_dac9e1195a0d_aad_135[]                          = { 0x2c, 0xa7, 0x1b, 0x23, 0x18, 0xdd, 0x96, 0xb2,
                                                         0x43, 0xc8, 0x70, 0xa3, 0xdd, 0xa9, 0xfa, 0x0d };
static const uint8_t chiptest_dac9e1195a0d_key_136[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_137[]                        = { 0x60, 0x66, 0x73, 0x03, 0x90, 0xc3, 0xae, 0x06 };
static const uint8_t chiptest_dac9e1195a0d_ct_138[]                           = { 0xc2, 0xaf, 0x35, 0x3b, 0x99, 0x82, 0xc3, 0x9f,
                                                        0x6d, 0x91, 0x4a, 0xef, 0x8c, 0xf4, 0x97, 0x19 };
static const uint8_t chiptest_dac9e1195a0d_tag_139[] = { 0x54, 0x26, 0x39, 0x94, 0xd2, 0x41, 0xe4, 0xff, 0x06, 0x0c, 0xcb, 0x0f };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_140 = { .pt        = chiptest_dac9e1195a0d_pt_134,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_135,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_136,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_137,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_138,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_139,
                                                                                  .tag_len   = 12,
                                                                                  .tcId      = 19,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_141[]                           = { 0x0a, 0x61, 0xf2, 0xb4, 0xd6, 0x13, 0x5e, 0x2f,
                                                        0x63, 0xd3, 0xae, 0x42, 0xc3, 0x08, 0x08, 0x3e };
static const uint8_t chiptest_dac9e1195a0d_aad_142[]                          = { 0x2c, 0xa7, 0x1b, 0x23, 0x18, 0xdd, 0x96, 0xb2,
                                                         0x43, 0xc8, 0x70, 0xa3, 0xdd, 0xa9, 0xfa, 0x0d };
static const uint8_t chiptest_dac9e1195a0d_key_143[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_144[]                        = { 0x60, 0x66, 0x73, 0x03, 0x90, 0xc3, 0xae, 0x06 };
static const uint8_t chiptest_dac9e1195a0d_ct_145[]                           = { 0xc2, 0xaf, 0x35, 0x3b, 0x99, 0x82, 0xc3, 0x9f,
                                                        0x6d, 0x91, 0x4a, 0xef, 0x8c, 0xf4, 0x97, 0x19 };
static const uint8_t chiptest_dac9e1195a0d_tag_146[]                          = { 0x08, 0x68, 0x46, 0xcc, 0x2f, 0x72, 0xa4, 0x90,
                                                         0x23, 0xc3, 0x0f, 0xc6, 0x9a, 0xac, 0x4b, 0x7f };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_147 = { .pt        = chiptest_dac9e1195a0d_pt_141,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_142,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_143,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_144,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_145,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_146,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 20,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_148[]                           = { 0x33, 0x23, 0x11, 0xb6, 0xae, 0xb2, 0x15, 0x2e,
                                                        0xb8, 0x44, 0x59, 0x4f, 0x41, 0xf8, 0xec, 0x69 };
static const uint8_t chiptest_dac9e1195a0d_aad_149[]                          = { 0xa4, 0x08, 0xf7, 0x5d, 0xdc, 0x1a, 0x13, 0x31,
                                                         0x3d, 0xfb, 0x35, 0x5f, 0x79, 0xcc, 0x36, 0x55 };
static const uint8_t chiptest_dac9e1195a0d_key_150[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_151[] = { 0x63, 0x90, 0x36, 0xf0, 0xad, 0x89, 0x82, 0x51, 0x58, 0xb0, 0x49, 0xca };
static const uint8_t chiptest_dac9e1195a0d_ct_152[]    = { 0x4f, 0x19, 0xc6, 0xa3, 0xbc, 0x09, 0x99, 0x34,
                                                        0xe3, 0x5b, 0x32, 0x9b, 0x89, 0xa8, 0x5e, 0x18 };
static const uint8_t chiptest_dac9e1195a0d_tag_153[]   = { 0x92, 0x62, 0xb1, 0x84, 0xe2, 0xec, 0xe1, 0xd4 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_154 = { .pt        = chiptest_dac9e1195a0d_pt_148,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_149,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_150,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_151,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_152,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_153,
                                                                                  .tag_len   = 8,
                                                                                  .tcId      = 21,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_155[]                           = { 0x33, 0x23, 0x11, 0xb6, 0xae, 0xb2, 0x15, 0x2e,
                                                        0xb8, 0x44, 0x59, 0x4f, 0x41, 0xf8, 0xec, 0x69 };
static const uint8_t chiptest_dac9e1195a0d_aad_156[]                          = { 0xa4, 0x08, 0xf7, 0x5d, 0xdc, 0x1a, 0x13, 0x31,
                                                         0x3d, 0xfb, 0x35, 0x5f, 0x79, 0xcc, 0x36, 0x55 };
static const uint8_t chiptest_dac9e1195a0d_key_157[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_158[] = { 0x63, 0x90, 0x36, 0xf0, 0xad, 0x89, 0x82, 0x51, 0x58, 0xb0, 0x49, 0xca };
static const uint8_t chiptest_dac9e1195a0d_ct_159[]    = { 0x4f, 0x19, 0xc6, 0xa3, 0xbc, 0x09, 0x99, 0x34,
                                                        0xe3, 0x5b, 0x32, 0x9b, 0x89, 0xa8, 0x5e, 0x18 };
static const uint8_t chiptest_dac9e1195a0d_tag_160[]   = { 0x0c, 0xc2, 0xc1, 0x5e, 0x0e, 0x93, 0x5a, 0x64, 0x9a, 0xfe, 0x4c, 0xce };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_161 = { .pt        = chiptest_dac9e1195a0d_pt_155,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_156,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_157,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_158,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_159,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_160,
                                                                                  .tag_len   = 12,
                                                                                  .tcId      = 22,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_162[]                           = { 0x33, 0x23, 0x11, 0xb6, 0xae, 0xb2, 0x15, 0x2e,
                                                        0xb8, 0x44, 0x59, 0x4f, 0x41, 0xf8, 0xec, 0x69 };
static const uint8_t chiptest_dac9e1195a0d_aad_163[]                          = { 0xa4, 0x08, 0xf7, 0x5d, 0xdc, 0x1a, 0x13, 0x31,
                                                         0x3d, 0xfb, 0x35, 0x5f, 0x79, 0xcc, 0x36, 0x55 };
static const uint8_t chiptest_dac9e1195a0d_key_164[]                          = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_165[] = { 0x63, 0x90, 0x36, 0xf0, 0xad, 0x89, 0x82, 0x51, 0x58, 0xb0, 0x49, 0xca };
static const uint8_t chiptest_dac9e1195a0d_ct_166[]    = { 0x4f, 0x19, 0xc6, 0xa3, 0xbc, 0x09, 0x99, 0x34,
                                                        0xe3, 0x5b, 0x32, 0x9b, 0x89, 0xa8, 0x5e, 0x18 };
static const uint8_t chiptest_dac9e1195a0d_tag_167[]   = { 0xbd, 0x90, 0x92, 0xa8, 0xdb, 0x2e, 0x78, 0x0b,
                                                         0x5a, 0x93, 0xa0, 0xfb, 0xe3, 0x3a, 0x5a, 0x38 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_168 = { .pt        = chiptest_dac9e1195a0d_pt_162,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_163,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_164,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_165,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_166,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_167,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 23,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_169[]    = { 0x3b, 0xc5, 0xc4, 0xda, 0x04, 0xd8, 0xa5, 0x15, 0x67, 0x9c, 0x5c,
                                                        0x40, 0xc0, 0xfd, 0x7d, 0x9e, 0x2b, 0x52, 0x22, 0x3f, 0xeb, 0x1f,
                                                        0x61, 0x99, 0x78, 0xe7, 0xce, 0x84, 0xdc, 0x3f, 0xbe, 0x85, 0xf2 };
static const uint8_t chiptest_dac9e1195a0d_aad_170[]   = {};
static const uint8_t chiptest_dac9e1195a0d_key_171[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_172[] = { 0x7b, 0x05, 0xbc, 0x8b, 0xcf, 0xc1, 0x6c, 0xc1 };
static const uint8_t chiptest_dac9e1195a0d_ct_173[]    = { 0xf1, 0x46, 0xa7, 0xb0, 0xf4, 0xff, 0x02, 0x3a, 0x23, 0xc3, 0xfe,
                                                        0xd5, 0x60, 0x4c, 0xdf, 0xe5, 0x09, 0x3e, 0x3c, 0x7b, 0x91, 0x4a,
                                                        0x68, 0xa5, 0xab, 0xff, 0x1a, 0x14, 0x96, 0x1e, 0x82, 0xb3, 0xb9 };
static const uint8_t chiptest_dac9e1195a0d_tag_174[]   = { 0x32, 0x6f, 0xff, 0xcc, 0xce, 0x78, 0x97, 0x57 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_175 = { .pt        = chiptest_dac9e1195a0d_pt_169,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_170,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_171,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_172,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_173,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_174,
                                                                                  .tag_len   = 8,
                                                                                  .tcId      = 24,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_176[]    = { 0x3b, 0xc5, 0xc4, 0xda, 0x04, 0xd8, 0xa5, 0x15, 0x67, 0x9c, 0x5c,
                                                        0x40, 0xc0, 0xfd, 0x7d, 0x9e, 0x2b, 0x52, 0x22, 0x3f, 0xeb, 0x1f,
                                                        0x61, 0x99, 0x78, 0xe7, 0xce, 0x84, 0xdc, 0x3f, 0xbe, 0x85, 0xf2 };
static const uint8_t chiptest_dac9e1195a0d_aad_177[]   = {};
static const uint8_t chiptest_dac9e1195a0d_key_178[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_179[] = { 0x7b, 0x05, 0xbc, 0x8b, 0xcf, 0xc1, 0x6c, 0xc1 };
static const uint8_t chiptest_dac9e1195a0d_ct_180[]    = { 0xf1, 0x46, 0xa7, 0xb0, 0xf4, 0xff, 0x02, 0x3a, 0x23, 0xc3, 0xfe,
                                                        0xd5, 0x60, 0x4c, 0xdf, 0xe5, 0x09, 0x3e, 0x3c, 0x7b, 0x91, 0x4a,
                                                        0x68, 0xa5, 0xab, 0xff, 0x1a, 0x14, 0x96, 0x1e, 0x82, 0xb3, 0xb9 };
static const uint8_t chiptest_dac9e1195a0d_tag_181[]   = { 0x5d, 0xca, 0x99, 0xed, 0x22, 0x49, 0x97, 0x05, 0x48, 0x57, 0x6e, 0x8b };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_182 = { .pt        = chiptest_dac9e1195a0d_pt_176,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_177,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_178,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_179,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_180,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_181,
                                                                                  .tag_len   = 12,
                                                                                  .tcId      = 25,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_183[]    = { 0x3b, 0xc5, 0xc4, 0xda, 0x04, 0xd8, 0xa5, 0x15, 0x67, 0x9c, 0x5c,
                                                        0x40, 0xc0, 0xfd, 0x7d, 0x9e, 0x2b, 0x52, 0x22, 0x3f, 0xeb, 0x1f,
                                                        0x61, 0x99, 0x78, 0xe7, 0xce, 0x84, 0xdc, 0x3f, 0xbe, 0x85, 0xf2 };
static const uint8_t chiptest_dac9e1195a0d_aad_184[]   = {};
static const uint8_t chiptest_dac9e1195a0d_key_185[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_186[] = { 0x7b, 0x05, 0xbc, 0x8b, 0xcf, 0xc1, 0x6c, 0xc1 };
static const uint8_t chiptest_dac9e1195a0d_ct_187[]    = { 0xf1, 0x46, 0xa7, 0xb0, 0xf4, 0xff, 0x02, 0x3a, 0x23, 0xc3, 0xfe,
                                                        0xd5, 0x60, 0x4c, 0xdf, 0xe5, 0x09, 0x3e, 0x3c, 0x7b, 0x91, 0x4a,
                                                        0x68, 0xa5, 0xab, 0xff, 0x1a, 0x14, 0x96, 0x1e, 0x82, 0xb3, 0xb9 };
static const uint8_t chiptest_dac9e1195a0d_tag_188[]   = { 0x31, 0x4c, 0xc7, 0xf3, 0x36, 0x41, 0x54, 0x55,
                                                         0xd8, 0xf5, 0xfb, 0x36, 0xea, 0x1b, 0x73, 0xd8 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_189 = { .pt        = chiptest_dac9e1195a0d_pt_183,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_184,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_185,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_186,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_187,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_188,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 26,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_190[]    = { 0x58, 0x07, 0xa2, 0x32, 0x31, 0x77, 0xa6, 0xba, 0xc6, 0x77, 0x3c,
                                                        0xd9, 0x28, 0x9b, 0xc1, 0x20, 0x14, 0xb8, 0x95, 0xa4, 0xbc, 0xa7,
                                                        0x8f, 0x93, 0x50, 0x86, 0xe1, 0x49, 0x7a, 0x38, 0xcc, 0x02, 0x42 };
static const uint8_t chiptest_dac9e1195a0d_aad_191[]   = {};
static const uint8_t chiptest_dac9e1195a0d_key_192[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_193[] = { 0xee, 0xbc, 0x7d, 0x0c, 0xce, 0x8d, 0x7f, 0xd6, 0xa5, 0x72, 0x6e, 0x49 };
static const uint8_t chiptest_dac9e1195a0d_ct_194[]    = { 0xb4, 0x48, 0x21, 0x1c, 0x9d, 0xa8, 0xc3, 0x31, 0x6c, 0x52, 0x69,
                                                        0xe5, 0xc4, 0xf6, 0x65, 0x78, 0x67, 0x3f, 0x1d, 0xfc, 0x03, 0x8a,
                                                        0x4f, 0x3f, 0x13, 0x2c, 0x85, 0x76, 0x2a, 0x89, 0x32, 0x06, 0xd4 };
static const uint8_t chiptest_dac9e1195a0d_tag_195[]   = { 0xae, 0x93, 0x77, 0x15, 0xa6, 0x5b, 0x80, 0xd5 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_196 = { .pt        = chiptest_dac9e1195a0d_pt_190,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_191,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_192,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_193,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_194,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_195,
                                                                                  .tag_len   = 8,
                                                                                  .tcId      = 27,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_197[]    = { 0x58, 0x07, 0xa2, 0x32, 0x31, 0x77, 0xa6, 0xba, 0xc6, 0x77, 0x3c,
                                                        0xd9, 0x28, 0x9b, 0xc1, 0x20, 0x14, 0xb8, 0x95, 0xa4, 0xbc, 0xa7,
                                                        0x8f, 0x93, 0x50, 0x86, 0xe1, 0x49, 0x7a, 0x38, 0xcc, 0x02, 0x42 };
static const uint8_t chiptest_dac9e1195a0d_aad_198[]   = {};
static const uint8_t chiptest_dac9e1195a0d_key_199[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_200[] = { 0xee, 0xbc, 0x7d, 0x0c, 0xce, 0x8d, 0x7f, 0xd6, 0xa5, 0x72, 0x6e, 0x49 };
static const uint8_t chiptest_dac9e1195a0d_ct_201[]    = { 0xb4, 0x48, 0x21, 0x1c, 0x9d, 0xa8, 0xc3, 0x31, 0x6c, 0x52, 0x69,
                                                        0xe5, 0xc4, 0xf6, 0x65, 0x78, 0x67, 0x3f, 0x1d, 0xfc, 0x03, 0x8a,
                                                        0x4f, 0x3f, 0x13, 0x2c, 0x85, 0x76, 0x2a, 0x89, 0x32, 0x06, 0xd4 };
static const uint8_t chiptest_dac9e1195a0d_tag_202[]   = { 0xc4, 0xa7, 0xd3, 0x61, 0x19, 0xeb, 0x20, 0x96, 0xaf, 0x5b, 0x39, 0x73 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_203 = { .pt        = chiptest_dac9e1195a0d_pt_197,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_198,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_199,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_200,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_201,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_202,
                                                                                  .tag_len   = 12,
                                                                                  .tcId      = 28,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_204[]    = { 0x58, 0x07, 0xa2, 0x32, 0x31, 0x77, 0xa6, 0xba, 0xc6, 0x77, 0x3c,
                                                        0xd9, 0x28, 0x9b, 0xc1, 0x20, 0x14, 0xb8, 0x95, 0xa4, 0xbc, 0xa7,
                                                        0x8f, 0x93, 0x50, 0x86, 0xe1, 0x49, 0x7a, 0x38, 0xcc, 0x02, 0x42 };
static const uint8_t chiptest_dac9e1195a0d_aad_205[]   = {};
static const uint8_t chiptest_dac9e1195a0d_key_206[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_207[] = { 0xee, 0xbc, 0x7d, 0x0c, 0xce, 0x8d, 0x7f, 0xd6, 0xa5, 0x72, 0x6e, 0x49 };
static const uint8_t chiptest_dac9e1195a0d_ct_208[]    = { 0xb4, 0x48, 0x21, 0x1c, 0x9d, 0xa8, 0xc3, 0x31, 0x6c, 0x52, 0x69,
                                                        0xe5, 0xc4, 0xf6, 0x65, 0x78, 0x67, 0x3f, 0x1d, 0xfc, 0x03, 0x8a,
                                                        0x4f, 0x3f, 0x13, 0x2c, 0x85, 0x76, 0x2a, 0x89, 0x32, 0x06, 0xd4 };
static const uint8_t chiptest_dac9e1195a0d_tag_209[]   = { 0x09, 0xbd, 0x22, 0x93, 0x58, 0xcb, 0x85, 0x4a,
                                                         0x72, 0x2d, 0xc1, 0x5d, 0x98, 0x32, 0x7c, 0xe6 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_210 = { .pt        = chiptest_dac9e1195a0d_pt_204,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_205,
                                                                                  .aad_len   = 0,
                                                                                  .key       = chiptest_dac9e1195a0d_key_206,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_207,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_208,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_209,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 29,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_211[]    = { 0x84, 0x4f, 0xf8, 0x35, 0xe3, 0xac, 0x66, 0x7b, 0x28, 0x84, 0xf3,
                                                        0x47, 0xf8, 0x88, 0xf5, 0x70, 0x91, 0x9d, 0xab, 0xc3, 0xd9, 0x75,
                                                        0x54, 0xe0, 0x87, 0x9c, 0xc5, 0x89, 0x43, 0x36, 0xcc, 0x19, 0x2d };
static const uint8_t chiptest_dac9e1195a0d_aad_212[]   = { 0x62, 0x55, 0xa0, 0x07, 0x9e, 0x72, 0xc9, 0x5c,
                                                         0x20, 0xf7, 0x0b, 0x6a, 0x09, 0xbb, 0x54, 0x13 };
static const uint8_t chiptest_dac9e1195a0d_key_213[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_214[] = { 0x2d, 0xbf, 0xc3, 0xa1, 0x2e, 0x4d, 0x2a, 0x86 };
static const uint8_t chiptest_dac9e1195a0d_ct_215[]    = { 0x2e, 0x6c, 0x47, 0x78, 0xf0, 0xcd, 0x09, 0x50, 0x95, 0xfa, 0xb4,
                                                        0xfb, 0x68, 0xb5, 0x59, 0xbf, 0xe6, 0xff, 0x2d, 0x09, 0xea, 0x7b,
                                                        0x66, 0x45, 0xfc, 0x1a, 0x25, 0x59, 0x5f, 0xd4, 0x48, 0x3e, 0xea };
static const uint8_t chiptest_dac9e1195a0d_tag_216[]   = { 0xe9, 0xb7, 0x04, 0x88, 0xaa, 0xf0, 0x23, 0x91 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_217 = { .pt        = chiptest_dac9e1195a0d_pt_211,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_212,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_213,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_214,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_215,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_216,
                                                                                  .tag_len   = 8,
                                                                                  .tcId      = 30,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_218[]    = { 0x84, 0x4f, 0xf8, 0x35, 0xe3, 0xac, 0x66, 0x7b, 0x28, 0x84, 0xf3,
                                                        0x47, 0xf8, 0x88, 0xf5, 0x70, 0x91, 0x9d, 0xab, 0xc3, 0xd9, 0x75,
                                                        0x54, 0xe0, 0x87, 0x9c, 0xc5, 0x89, 0x43, 0x36, 0xcc, 0x19, 0x2d };
static const uint8_t chiptest_dac9e1195a0d_aad_219[]   = { 0x62, 0x55, 0xa0, 0x07, 0x9e, 0x72, 0xc9, 0x5c,
                                                         0x20, 0xf7, 0x0b, 0x6a, 0x09, 0xbb, 0x54, 0x13 };
static const uint8_t chiptest_dac9e1195a0d_key_220[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_221[] = { 0x2d, 0xbf, 0xc3, 0xa1, 0x2e, 0x4d, 0x2a, 0x86 };
static const uint8_t chiptest_dac9e1195a0d_ct_222[]    = { 0x2e, 0x6c, 0x47, 0x78, 0xf0, 0xcd, 0x09, 0x50, 0x95, 0xfa, 0xb4,
                                                        0xfb, 0x68, 0xb5, 0x59, 0xbf, 0xe6, 0xff, 0x2d, 0x09, 0xea, 0x7b,
                                                        0x66, 0x45, 0xfc, 0x1a, 0x25, 0x59, 0x5f, 0xd4, 0x48, 0x3e, 0xea };
static const uint8_t chiptest_dac9e1195a0d_tag_223[]   = { 0x1e, 0xe1, 0xda, 0x23, 0xcf, 0x3d, 0x1d, 0xb0, 0xd2, 0x01, 0x0e, 0x3d };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_224 = { .pt        = chiptest_dac9e1195a0d_pt_218,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_219,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_220,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_221,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_222,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_223,
                                                                                  .tag_len   = 12,
                                                                                  .tcId      = 31,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_225[]    = { 0x84, 0x4f, 0xf8, 0x35, 0xe3, 0xac, 0x66, 0x7b, 0x28, 0x84, 0xf3,
                                                        0x47, 0xf8, 0x88, 0xf5, 0x70, 0x91, 0x9d, 0xab, 0xc3, 0xd9, 0x75,
                                                        0x54, 0xe0, 0x87, 0x9c, 0xc5, 0x89, 0x43, 0x36, 0xcc, 0x19, 0x2d };
static const uint8_t chiptest_dac9e1195a0d_aad_226[]   = { 0x62, 0x55, 0xa0, 0x07, 0x9e, 0x72, 0xc9, 0x5c,
                                                         0x20, 0xf7, 0x0b, 0x6a, 0x09, 0xbb, 0x54, 0x13 };
static const uint8_t chiptest_dac9e1195a0d_key_227[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_228[] = { 0x2d, 0xbf, 0xc3, 0xa1, 0x2e, 0x4d, 0x2a, 0x86 };
static const uint8_t chiptest_dac9e1195a0d_ct_229[]    = { 0x2e, 0x6c, 0x47, 0x78, 0xf0, 0xcd, 0x09, 0x50, 0x95, 0xfa, 0xb4,
                                                        0xfb, 0x68, 0xb5, 0x59, 0xbf, 0xe6, 0xff, 0x2d, 0x09, 0xea, 0x7b,
                                                        0x66, 0x45, 0xfc, 0x1a, 0x25, 0x59, 0x5f, 0xd4, 0x48, 0x3e, 0xea };
static const uint8_t chiptest_dac9e1195a0d_tag_230[]   = { 0xa5, 0x63, 0x5f, 0x07, 0x86, 0xe5, 0x08, 0x8c,
                                                         0xf9, 0x90, 0x07, 0x20, 0x59, 0x94, 0x5f, 0xe9 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_231 = { .pt        = chiptest_dac9e1195a0d_pt_225,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_226,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_227,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_228,
                                                                                  .nonce_len = 8,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_229,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_230,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 32,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_232[]    = { 0xd8, 0xc1, 0x36, 0xcc, 0x07, 0x01, 0x9d, 0x34, 0xcd, 0xca, 0xd0,
                                                        0xa0, 0x42, 0xfc, 0x3d, 0x75, 0x19, 0xe3, 0x0b, 0x5d, 0xdc, 0xb9,
                                                        0x10, 0xde, 0x53, 0xe6, 0x24, 0x12, 0x36, 0x36, 0xda, 0x52, 0x7b };
static const uint8_t chiptest_dac9e1195a0d_aad_233[]   = { 0x89, 0x3c, 0xd3, 0xe3, 0x53, 0x4d, 0xe7, 0x0d,
                                                         0x0f, 0x29, 0x39, 0xe7, 0xae, 0x69, 0x62, 0x7d };
static const uint8_t chiptest_dac9e1195a0d_key_234[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_235[] = { 0xb9, 0xb2, 0xeb, 0x2f, 0x55, 0x59, 0xba, 0xfd, 0x5d, 0xbb, 0xda, 0xf3 };
static const uint8_t chiptest_dac9e1195a0d_ct_236[]    = { 0xf9, 0x9f, 0x6a, 0x7c, 0x88, 0x86, 0x4d, 0x21, 0x2c, 0x12, 0x27,
                                                        0x91, 0x70, 0x23, 0xda, 0x36, 0xf1, 0xfa, 0x11, 0xf8, 0x26, 0x2d,
                                                        0x75, 0xb6, 0xc2, 0x0c, 0x4c, 0x19, 0xdb, 0x92, 0x14, 0xcc, 0x19 };
static const uint8_t chiptest_dac9e1195a0d_tag_237[]   = { 0x61, 0xaf, 0x0a, 0xf3, 0xe3, 0x2a, 0x8a, 0x82 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_238 = { .pt        = chiptest_dac9e1195a0d_pt_232,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_233,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_234,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_235,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_236,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_237,
                                                                                  .tag_len   = 8,
                                                                                  .tcId      = 33,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_239[]    = { 0xd8, 0xc1, 0x36, 0xcc, 0x07, 0x01, 0x9d, 0x34, 0xcd, 0xca, 0xd0,
                                                        0xa0, 0x42, 0xfc, 0x3d, 0x75, 0x19, 0xe3, 0x0b, 0x5d, 0xdc, 0xb9,
                                                        0x10, 0xde, 0x53, 0xe6, 0x24, 0x12, 0x36, 0x36, 0xda, 0x52, 0x7b };
static const uint8_t chiptest_dac9e1195a0d_aad_240[]   = { 0x89, 0x3c, 0xd3, 0xe3, 0x53, 0x4d, 0xe7, 0x0d,
                                                         0x0f, 0x29, 0x39, 0xe7, 0xae, 0x69, 0x62, 0x7d };
static const uint8_t chiptest_dac9e1195a0d_key_241[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_242[] = { 0xb9, 0xb2, 0xeb, 0x2f, 0x55, 0x59, 0xba, 0xfd, 0x5d, 0xbb, 0xda, 0xf3 };
static const uint8_t chiptest_dac9e1195a0d_ct_243[]    = { 0xf9, 0x9f, 0x6a, 0x7c, 0x88, 0x86, 0x4d, 0x21, 0x2c, 0x12, 0x27,
                                                        0x91, 0x70, 0x23, 0xda, 0x36, 0xf1, 0xfa, 0x11, 0xf8, 0x26, 0x2d,
                                                        0x75, 0xb6, 0xc2, 0x0c, 0x4c, 0x19, 0xdb, 0x92, 0x14, 0xcc, 0x19 };
static const uint8_t chiptest_dac9e1195a0d_tag_244[]   = { 0x97, 0x66, 0x01, 0x50, 0x02, 0xd8, 0x68, 0x6c, 0x12, 0xd0, 0x2c, 0x3d };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_245 = { .pt        = chiptest_dac9e1195a0d_pt_239,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_240,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_241,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_242,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_243,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_244,
                                                                                  .tag_len   = 12,
                                                                                  .tcId      = 34,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_246[]    = { 0xd8, 0xc1, 0x36, 0xcc, 0x07, 0x01, 0x9d, 0x34, 0xcd, 0xca, 0xd0,
                                                        0xa0, 0x42, 0xfc, 0x3d, 0x75, 0x19, 0xe3, 0x0b, 0x5d, 0xdc, 0xb9,
                                                        0x10, 0xde, 0x53, 0xe6, 0x24, 0x12, 0x36, 0x36, 0xda, 0x52, 0x7b };
static const uint8_t chiptest_dac9e1195a0d_aad_247[]   = { 0x89, 0x3c, 0xd3, 0xe3, 0x53, 0x4d, 0xe7, 0x0d,
                                                         0x0f, 0x29, 0x39, 0xe7, 0xae, 0x69, 0x62, 0x7d };
static const uint8_t chiptest_dac9e1195a0d_key_248[]   = { 0x04, 0xe1, 0xaf, 0x8f, 0x15, 0xe2, 0x16, 0x4e,
                                                         0xdb, 0x2d, 0xfd, 0xfe, 0xa2, 0xc4, 0x8b, 0xcf };
static const uint8_t chiptest_dac9e1195a0d_nonce_249[] = { 0xb9, 0xb2, 0xeb, 0x2f, 0x55, 0x59, 0xba, 0xfd, 0x5d, 0xbb, 0xda, 0xf3 };
static const uint8_t chiptest_dac9e1195a0d_ct_250[]    = { 0xf9, 0x9f, 0x6a, 0x7c, 0x88, 0x86, 0x4d, 0x21, 0x2c, 0x12, 0x27,
                                                        0x91, 0x70, 0x23, 0xda, 0x36, 0xf1, 0xfa, 0x11, 0xf8, 0x26, 0x2d,
                                                        0x75, 0xb6, 0xc2, 0x0c, 0x4c, 0x19, 0xdb, 0x92, 0x14, 0xcc, 0x19 };
static const uint8_t chiptest_dac9e1195a0d_tag_251[]   = { 0x9e, 0x7b, 0x72, 0x96, 0x6c, 0x9d, 0xbc, 0x93,
                                                         0xdb, 0x0e, 0xa9, 0x92, 0xa0, 0x8c, 0x9c, 0xad };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_252 = { .pt        = chiptest_dac9e1195a0d_pt_246,
                                                                                  .pt_len    = 33,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_247,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_248,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_249,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_250,
                                                                                  .ct_len    = 33,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_251,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 35,
                                                                                  .result    = CHIP_NO_ERROR };
static const uint8_t chiptest_dac9e1195a0d_pt_253[]                           = { 0x31, 0x5e, 0x88, 0xf4, 0x83, 0x2d, 0x0c, 0x1d,
                                                        0xb4, 0xd6, 0x22, 0xa7, 0x49, 0x97, 0x27, 0x6e };
static const uint8_t chiptest_dac9e1195a0d_aad_254[]                          = { 0x0f, 0x9d, 0xfc, 0x66, 0x19, 0x6e, 0xc5, 0x8b,
                                                         0xd9, 0x32, 0x8d, 0xc9, 0x4f, 0xea, 0x9f, 0xe2 };
static const uint8_t chiptest_dac9e1195a0d_key_255[]                          = { 0xb0, 0x40, 0x51, 0x6c, 0xe6, 0x32, 0x48, 0xb3,
                                                         0xfc, 0xf5, 0x00, 0x98, 0x48, 0xc0, 0xdb, 0xf3 };
static const uint8_t chiptest_dac9e1195a0d_nonce_256[] = { 0x30, 0x0d, 0xb6, 0x16, 0xc5, 0xb5, 0xe5, 0xde, 0xb4, 0xf8, 0x90, 0x79 };
static const uint8_t chiptest_dac9e1195a0d_ct_257[]    = { 0x84, 0x15, 0x9c, 0xfd, 0xb7, 0xfd, 0xab, 0x6a,
                                                        0x11, 0x3e, 0x66, 0x55, 0xfe, 0xf4, 0x17, 0x16 };
static const uint8_t chiptest_dac9e1195a0d_tag_258[]   = { 0xc8, 0x50, 0x01, 0xbe, 0xfc, 0x4d, 0xe3, 0x19,
                                                         0xea, 0x9f, 0x01, 0x6a, 0xde, 0xf5, 0x6f, 0xe3 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_259 = { .pt        = chiptest_dac9e1195a0d_pt_253,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_254,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_255,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_256,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_257,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_258,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 36,
                                                                                  .result    = CHIP_ERROR_INTERNAL };
static const uint8_t chiptest_dac9e1195a0d_pt_260[]                           = { 0x31, 0x5e, 0x88, 0xf4, 0x83, 0x2d, 0x0c, 0x1d,
                                                        0xb4, 0xd6, 0x22, 0xa7, 0x49, 0x97, 0x27, 0x6e };
static const uint8_t chiptest_dac9e1195a0d_aad_261[]                          = { 0x0f, 0x9d, 0xfc, 0x66, 0x19, 0x6e, 0xc5, 0x8b,
                                                         0xd9, 0x32, 0x8d, 0xc9, 0x4f, 0xea, 0x9f, 0xe2 };
static const uint8_t chiptest_dac9e1195a0d_key_262[]                          = { 0xb0, 0x40, 0x51, 0x6c, 0xe6, 0x32, 0x48, 0xb3,
                                                         0xfc, 0xf5, 0x00, 0x98, 0x48, 0xc0, 0xdb, 0xf3 };
static const uint8_t chiptest_dac9e1195a0d_nonce_263[] = { 0x30, 0x0d, 0xb6, 0x16, 0xc5, 0xb5, 0xe5, 0xde, 0xb4, 0xf8, 0x90, 0x79 };
static const uint8_t chiptest_dac9e1195a0d_ct_264[]    = { 0x85, 0x15, 0x9c, 0xfd, 0xb7, 0xfd, 0xab, 0x6a,
                                                        0x11, 0x3e, 0x66, 0x55, 0xfe, 0xf4, 0x17, 0x16 };
static const uint8_t chiptest_dac9e1195a0d_tag_265[]   = { 0xc7, 0x50, 0x01, 0xbe, 0xfc, 0x4d, 0xe3, 0x19,
                                                         0xea, 0x9f, 0x01, 0x6a, 0xde, 0xf5, 0x6f, 0xe3 };
static const struct ccm_128_test_vector chiptest_dac9e1195a0d_test_vector_266 = { .pt        = chiptest_dac9e1195a0d_pt_260,
                                                                                  .pt_len    = 16,
                                                                                  .aad       = chiptest_dac9e1195a0d_aad_261,
                                                                                  .aad_len   = 16,
                                                                                  .key       = chiptest_dac9e1195a0d_key_262,
                                                                                  .key_len   = 16,
                                                                                  .nonce     = chiptest_dac9e1195a0d_nonce_263,
                                                                                  .nonce_len = 12,
                                                                                  .ct        = chiptest_dac9e1195a0d_ct_264,
                                                                                  .ct_len    = 16,
                                                                                  .tag       = chiptest_dac9e1195a0d_tag_265,
                                                                                  .tag_len   = 16,
                                                                                  .tcId      = 37,
                                                                                  .result    = CHIP_ERROR_INTERNAL };

static const uint8_t test_vector_2ef53070ae20_key[16] = {
    0x09, 0x53, 0xfa, 0x93, 0xe7, 0xca, 0xac, 0x96, 0x38, 0xf5, 0x88, 0x20, 0x22, 0x0a, 0x39, 0x8e,
};

static const uint8_t test_vector_2ef53070ae20_nonce[13] = {
    0x00, 0x80, 0x00, 0x00, 0x01, 0x12, 0x01, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_2ef53070ae20_in[13] = {
    0xff, 0xfd, 0x03, 0x4b, 0x50, 0x05, 0x7e, 0x40, 0x00, 0x00, 0x01, 0x00, 0x00,
};

static const uint8_t test_vector_2ef53070ae20_ad[0] = {};

static const uint8_t test_vector_2ef53070ae20_ct[13] = {
    0xb5, 0xe5, 0xbf, 0xda, 0xcb, 0xaf, 0x6c, 0xb7, 0xfb, 0x6b, 0xff, 0x87, 0x1f,
};

static const uint8_t test_vector_2ef53070ae20_tag[16] = {
    0xb0, 0xd6, 0xdd, 0x82, 0x7d, 0x35, 0xbf, 0x37, 0x2f, 0xa6, 0x42, 0x5d, 0xcd, 0x17, 0xd3, 0x56,
};

static const struct ccm_128_test_vector aesccm128_matter_2ef53070ae20_test_vector_0 = {
    .pt        = test_vector_2ef53070ae20_in,
    .pt_len    = 13,
    .aad       = test_vector_2ef53070ae20_ad,
    .aad_len   = 0,
    .key       = test_vector_2ef53070ae20_key,
    .key_len   = 16,
    .nonce     = test_vector_2ef53070ae20_nonce,
    .nonce_len = 13,
    .ct        = test_vector_2ef53070ae20_ct,
    .ct_len    = 13,
    .tag       = test_vector_2ef53070ae20_tag,
    .tag_len   = 16,
    .tcId      = 0,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_91c8d337cf46_key[16] = {
    0x09, 0x53, 0xfa, 0x93, 0xe7, 0xca, 0xac, 0x96, 0x38, 0xf5, 0x88, 0x20, 0x22, 0x0a, 0x39, 0x8e,
};

static const uint8_t test_vector_91c8d337cf46_nonce[13] = {
    0x00, 0x80, 0x01, 0x48, 0x20, 0x23, 0x45, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_91c8d337cf46_in[9] = {
    0x12, 0x01, 0x04, 0x32, 0x03, 0x08, 0xba, 0x07, 0x2f,
};

static const uint8_t test_vector_91c8d337cf46_ad[0] = {};

static const uint8_t test_vector_91c8d337cf46_ct[9] = {
    0x79, 0xd7, 0xdb, 0xc0, 0xc9, 0xb4, 0xd4, 0x3e, 0xeb,
};

static const uint8_t test_vector_91c8d337cf46_tag[16] = {
    0x28, 0x15, 0x08, 0xe5, 0x0d, 0x58, 0xdb, 0xbd, 0x27, 0xc3, 0x95, 0x97, 0x80, 0x0f, 0x47, 0x33,
};

static const struct ccm_128_test_vector aesccm128_matter_91c8d337cf46_test_vector_1 = {
    .pt        = test_vector_91c8d337cf46_in,
    .pt_len    = 9,
    .aad       = test_vector_91c8d337cf46_ad,
    .aad_len   = 0,
    .key       = test_vector_91c8d337cf46_key,
    .key_len   = 16,
    .nonce     = test_vector_91c8d337cf46_nonce,
    .nonce_len = 13,
    .ct        = test_vector_91c8d337cf46_ct,
    .ct_len    = 9,
    .tag       = test_vector_91c8d337cf46_tag,
    .tag_len   = 16,
    .tcId      = 1,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_37fc295e3601_key[16] = {
    0x09, 0x53, 0xfa, 0x93, 0xe7, 0xca, 0xac, 0x96, 0x38, 0xf5, 0x88, 0x20, 0x22, 0x0a, 0x39, 0x8e,
};

static const uint8_t test_vector_37fc295e3601_nonce[13] = {
    0x00, 0x80, 0x2b, 0x38, 0x32, 0x2f, 0xe3, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_37fc295e3601_in[9] = {
    0x12, 0x01, 0x04, 0xfa, 0x02, 0x05, 0xa6, 0x00, 0x0a,
};

static const uint8_t test_vector_37fc295e3601_ad[0] = {};

static const uint8_t test_vector_37fc295e3601_ct[9] = {
    0x53, 0x27, 0x30, 0x86, 0xb8, 0xc5, 0xee, 0x00, 0xbd,
};

static const uint8_t test_vector_37fc295e3601_tag[16] = {
    0xd5, 0x2b, 0x87, 0xa8, 0xce, 0x62, 0x90, 0xa7, 0x72, 0xd4, 0x72, 0xb8, 0xc6, 0x2b, 0xdc, 0x13,
};

static const struct ccm_128_test_vector aesccm128_matter_37fc295e3601_test_vector_2 = {
    .pt        = test_vector_37fc295e3601_in,
    .pt_len    = 9,
    .aad       = test_vector_37fc295e3601_ad,
    .aad_len   = 0,
    .key       = test_vector_37fc295e3601_key,
    .key_len   = 16,
    .nonce     = test_vector_37fc295e3601_nonce,
    .nonce_len = 13,
    .ct        = test_vector_37fc295e3601_ct,
    .ct_len    = 9,
    .tag       = test_vector_37fc295e3601_tag,
    .tag_len   = 16,
    .tcId      = 2,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_8bab3993e4d1_key[16] = {
    0xbe, 0x63, 0x51, 0x05, 0x43, 0x48, 0x59, 0xf4, 0x84, 0xfc, 0x79, 0x8e, 0x04, 0x3c, 0xe4, 0x0e,
};

static const uint8_t test_vector_8bab3993e4d1_nonce[13] = {
    0x00, 0x80, 0x00, 0x00, 0x02, 0x12, 0x01, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_8bab3993e4d1_in[4] = {
    0x23,
    0x45,
    0x01,
    0x00,
};

static const uint8_t test_vector_8bab3993e4d1_ad[0] = {};

static const uint8_t test_vector_8bab3993e4d1_ct[4] = {
    0xb0,
    0xe5,
    0xd0,
    0xad,
};

static const uint8_t test_vector_8bab3993e4d1_tag[16] = {
    0x60, 0x78, 0xe0, 0xdd, 0xbb, 0x7c, 0xd4, 0x3f, 0xae, 0xa5, 0x7c, 0x70, 0x51, 0xe5, 0xb4, 0xae,
};

static const struct ccm_128_test_vector aesccm128_matter_8bab3993e4d1_test_vector_3 = {
    .pt        = test_vector_8bab3993e4d1_in,
    .pt_len    = 4,
    .aad       = test_vector_8bab3993e4d1_ad,
    .aad_len   = 0,
    .key       = test_vector_8bab3993e4d1_key,
    .key_len   = 16,
    .nonce     = test_vector_8bab3993e4d1_nonce,
    .nonce_len = 13,
    .ct        = test_vector_8bab3993e4d1_ct,
    .ct_len    = 4,
    .tag       = test_vector_8bab3993e4d1_tag,
    .tag_len   = 16,
    .tcId      = 3,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_76dbd8e42344_key[16] = {
    0xbe, 0x63, 0x51, 0x05, 0x43, 0x48, 0x59, 0xf4, 0x84, 0xfc, 0x79, 0x8e, 0x04, 0x3c, 0xe4, 0x0e,
};

static const uint8_t test_vector_76dbd8e42344_nonce[13] = {
    0x00, 0x80, 0x01, 0x48, 0x34, 0x23, 0x45, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_76dbd8e42344_in[9] = {
    0x12, 0x01, 0x02, 0x00, 0x12, 0x34, 0x56, 0x78, 0x00,
};

static const uint8_t test_vector_76dbd8e42344_ad[0] = {};

static const uint8_t test_vector_76dbd8e42344_ct[9] = {
    0x5c, 0x39, 0xda, 0x17, 0x92, 0xb1, 0xfe, 0xe9, 0xec,
};

static const uint8_t test_vector_76dbd8e42344_tag[16] = {
    0xa9, 0x23, 0x39, 0x58, 0xac, 0xed, 0x64, 0xf2, 0x34, 0x3b, 0x9d, 0x61, 0x0e, 0x87, 0x64, 0x40,
};

static const struct ccm_128_test_vector aesccm128_matter_76dbd8e42344_test_vector_4 = {
    .pt        = test_vector_76dbd8e42344_in,
    .pt_len    = 9,
    .aad       = test_vector_76dbd8e42344_ad,
    .aad_len   = 0,
    .key       = test_vector_76dbd8e42344_key,
    .key_len   = 16,
    .nonce     = test_vector_76dbd8e42344_nonce,
    .nonce_len = 13,
    .ct        = test_vector_76dbd8e42344_ct,
    .ct_len    = 9,
    .tag       = test_vector_76dbd8e42344_tag,
    .tag_len   = 16,
    .tcId      = 4,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_ea145e3af464_key[16] = {
    0x09, 0x53, 0xfa, 0x93, 0xe7, 0xca, 0xac, 0x96, 0x38, 0xf5, 0x88, 0x20, 0x22, 0x0a, 0x39, 0x8e,
};

static const uint8_t test_vector_ea145e3af464_nonce[13] = {
    0x00, 0x8b, 0x01, 0x48, 0x35, 0x23, 0x45, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_ea145e3af464_in[9] = {
    0x00, 0x03, 0x00, 0xa6, 0xac, 0x00, 0x00, 0x00, 0x02,
};

static const uint8_t test_vector_ea145e3af464_ad[0] = {};

static const uint8_t test_vector_ea145e3af464_ct[9] = {
    0x0d, 0x0d, 0x73, 0x0f, 0x94, 0xd7, 0xf3, 0x50, 0x9d,
};

static const uint8_t test_vector_ea145e3af464_tag[16] = {
    0xdd, 0xa1, 0x69, 0x4a, 0xdb, 0x79, 0x16, 0x52, 0xfb, 0x6a, 0xe0, 0x46, 0x82, 0xf1, 0x9b, 0x29,
};

static const struct ccm_128_test_vector aesccm128_matter_ea145e3af464_test_vector_5 = {
    .pt        = test_vector_ea145e3af464_in,
    .pt_len    = 9,
    .aad       = test_vector_ea145e3af464_ad,
    .aad_len   = 0,
    .key       = test_vector_ea145e3af464_key,
    .key_len   = 16,
    .nonce     = test_vector_ea145e3af464_nonce,
    .nonce_len = 13,
    .ct        = test_vector_ea145e3af464_ct,
    .ct_len    = 9,
    .tag       = test_vector_ea145e3af464_tag,
    .tag_len   = 16,
    .tcId      = 5,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_08f3cd0b35d1_key[16] = {
    0x09, 0x53, 0xfa, 0x93, 0xe7, 0xca, 0xac, 0x96, 0x38, 0xf5, 0x88, 0x20, 0x22, 0x0a, 0x39, 0x8e,
};

static const uint8_t test_vector_08f3cd0b35d1_nonce[13] = {
    0x00, 0x8b, 0x01, 0x48, 0x36, 0x23, 0x45, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_08f3cd0b35d1_in[9] = {
    0x00, 0x03, 0x00, 0xa6, 0xac, 0x00, 0x00, 0x00, 0x03,
};

static const uint8_t test_vector_08f3cd0b35d1_ad[0] = {};

static const uint8_t test_vector_08f3cd0b35d1_ct[9] = {
    0xd8, 0x5d, 0x80, 0x6b, 0xbe, 0xd2, 0x48, 0x61, 0x4f,
};

static const uint8_t test_vector_08f3cd0b35d1_tag[16] = {
    0xef, 0x7f, 0x4d, 0x55, 0xe4, 0x7d, 0x21, 0x52, 0x2e, 0xbe, 0x3d, 0x5b, 0xc7, 0x35, 0xa5, 0xc5,
};

static const struct ccm_128_test_vector aesccm128_matter_08f3cd0b35d1_test_vector_6 = {
    .pt        = test_vector_08f3cd0b35d1_in,
    .pt_len    = 9,
    .aad       = test_vector_08f3cd0b35d1_ad,
    .aad_len   = 0,
    .key       = test_vector_08f3cd0b35d1_key,
    .key_len   = 16,
    .nonce     = test_vector_08f3cd0b35d1_nonce,
    .nonce_len = 13,
    .ct        = test_vector_08f3cd0b35d1_ct,
    .ct_len    = 9,
    .tag       = test_vector_08f3cd0b35d1_tag,
    .tag_len   = 16,
    .tcId      = 6,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_70d7d4a1f8ea_key[16] = {
    0xbe, 0x63, 0x51, 0x05, 0x43, 0x48, 0x59, 0xf4, 0x84, 0xfc, 0x79, 0x8e, 0x04, 0x3c, 0xe4, 0x0e,
};

static const uint8_t test_vector_70d7d4a1f8ea_nonce[13] = {
    0x00, 0x80, 0x00, 0x00, 0x03, 0x12, 0x01, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_70d7d4a1f8ea_in[4] = {
    0x23,
    0x45,
    0x01,
    0x01,
};

static const uint8_t test_vector_70d7d4a1f8ea_ad[0] = {};

static const uint8_t test_vector_70d7d4a1f8ea_ct[4] = {
    0x77,
    0x77,
    0xed,
    0x35,
};

static const uint8_t test_vector_70d7d4a1f8ea_tag[16] = {
    0x35, 0xd8, 0x4e, 0x18, 0x78, 0x4c, 0x4b, 0xf3, 0xcb, 0x1b, 0x4c, 0x19, 0x1d, 0xc5, 0x55, 0xcc,
};

static const struct ccm_128_test_vector aesccm128_matter_70d7d4a1f8ea_test_vector_7 = {
    .pt        = test_vector_70d7d4a1f8ea_in,
    .pt_len    = 4,
    .aad       = test_vector_70d7d4a1f8ea_ad,
    .aad_len   = 0,
    .key       = test_vector_70d7d4a1f8ea_key,
    .key_len   = 16,
    .nonce     = test_vector_70d7d4a1f8ea_nonce,
    .nonce_len = 13,
    .ct        = test_vector_70d7d4a1f8ea_ct,
    .ct_len    = 4,
    .tag       = test_vector_70d7d4a1f8ea_tag,
    .tag_len   = 16,
    .tcId      = 7,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_56f7e4096627_key[16] = {
    0xbe, 0x63, 0x51, 0x05, 0x43, 0x48, 0x59, 0xf4, 0x84, 0xfc, 0x79, 0x8e, 0x04, 0x3c, 0xe4, 0x0e,
};

static const uint8_t test_vector_56f7e4096627_nonce[13] = {
    0x00, 0x80, 0x00, 0x00, 0x04, 0x12, 0x01, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_56f7e4096627_in[4] = {
    0x23,
    0x45,
    0x01,
    0x01,
};

static const uint8_t test_vector_56f7e4096627_ad[0] = {};

static const uint8_t test_vector_56f7e4096627_ct[4] = {
    0xae,
    0x21,
    0x46,
    0x60,
};

static const uint8_t test_vector_56f7e4096627_tag[16] = {
    0xd1, 0x46, 0xb2, 0x8b, 0xea, 0xfe, 0x7f, 0x98, 0x4f, 0x94, 0x30, 0x50, 0x2d, 0x07, 0xaa, 0xfe,
};

static const struct ccm_128_test_vector aesccm128_matter_56f7e4096627_test_vector_8 = {
    .pt        = test_vector_56f7e4096627_in,
    .pt_len    = 4,
    .aad       = test_vector_56f7e4096627_ad,
    .aad_len   = 0,
    .key       = test_vector_56f7e4096627_key,
    .key_len   = 16,
    .nonce     = test_vector_56f7e4096627_nonce,
    .nonce_len = 13,
    .ct        = test_vector_56f7e4096627_ct,
    .ct_len    = 4,
    .tag       = test_vector_56f7e4096627_tag,
    .tag_len   = 16,
    .tcId      = 8,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_02112b4a46d8_key[16] = {
    0xbe, 0x63, 0x51, 0x05, 0x43, 0x48, 0x59, 0xf4, 0x84, 0xfc, 0x79, 0x8e, 0x04, 0x3c, 0xe4, 0x0e,
};

static const uint8_t test_vector_02112b4a46d8_nonce[13] = {
    0x00, 0x80, 0x00, 0x00, 0x05, 0x12, 0x01, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
};

static const uint8_t test_vector_02112b4a46d8_in[4] = {
    0x23,
    0x45,
    0x01,
    0x00,
};

static const uint8_t test_vector_02112b4a46d8_ad[0] = {};

static const uint8_t test_vector_02112b4a46d8_ct[4] = {
    0x7d,
    0x3a,
    0xe6,
    0x2a,
};

static const uint8_t test_vector_02112b4a46d8_tag[16] = {
    0x52, 0xee, 0x03, 0xab, 0x84, 0xe1, 0xa3, 0x33, 0x65, 0xe8, 0xa6, 0x12, 0x75, 0x66, 0x5f, 0x71,
};

static const struct ccm_128_test_vector aesccm128_matter_02112b4a46d8_test_vector_9 = {
    .pt        = test_vector_02112b4a46d8_in,
    .pt_len    = 4,
    .aad       = test_vector_02112b4a46d8_ad,
    .aad_len   = 0,
    .key       = test_vector_02112b4a46d8_key,
    .key_len   = 16,
    .nonce     = test_vector_02112b4a46d8_nonce,
    .nonce_len = 13,
    .ct        = test_vector_02112b4a46d8_ct,
    .ct_len    = 4,
    .tag       = test_vector_02112b4a46d8_tag,
    .tag_len   = 16,
    .tcId      = 9,
    .result    = CHIP_NO_ERROR,
};
static const uint8_t test_vector_ac063659d220_key[16] = {
    0x63, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48,
};

static const uint8_t test_vector_ac063659d220_nonce[13] = {
    0x01, 0x00, 0x07, 0x08, 0x0d, 0x12, 0x34, 0x97, 0x36, 0x12, 0x34, 0x56, 0x77,
};

static const uint8_t test_vector_ac063659d220_in[8] = {
    0xea, 0x0a, 0x00, 0x57, 0x6f, 0x72, 0x6c, 0x64,
};

static const uint8_t test_vector_ac063659d220_ad[16] = {
    0xf4, 0xa0, 0x02, 0xc7, 0xfb, 0x1e, 0x4c, 0xa0, 0xa4, 0x69, 0xa0, 0x21, 0xde, 0x0d, 0xb8, 0x75,
};

static const uint8_t test_vector_ac063659d220_ct[8] = {
    0xde, 0x15, 0x47, 0x11, 0x84, 0x63, 0x12, 0x3e,
};

static const uint8_t test_vector_ac063659d220_tag[16] = {
    0x14, 0x60, 0x4c, 0x1d, 0xdb, 0x4f, 0x59, 0x87, 0x06, 0x4b, 0x17, 0x36, 0xf3, 0x92, 0x39, 0x62,
};

static const struct ccm_128_test_vector aesccm128_matter_ac063659d220_test_vector_10 = {
    .pt        = test_vector_ac063659d220_in,
    .pt_len    = 8,
    .aad       = test_vector_ac063659d220_ad,
    .aad_len   = 16,
    .key       = test_vector_ac063659d220_key,
    .key_len   = 16,
    .nonce     = test_vector_ac063659d220_nonce,
    .nonce_len = 13,
    .ct        = test_vector_ac063659d220_ct,
    .ct_len    = 8,
    .tag       = test_vector_ac063659d220_tag,
    .tag_len   = 16,
    .tcId      = 10,
    .result    = CHIP_NO_ERROR,
};

static const struct ccm_128_test_vector * ccm_128_test_vectors[] = {
// All test cases in the ifdef do NOT follow the crypto primitive constraints for
// Matter: they use different tag length and nonce length than the mandatory primitives.
// The set outside the #if is using mandatory lengths. BoringSSL *only* supports
// the exact mapping of the spec and the SDK only uses the exact mapping of the spec.
#if !CHIP_CRYPTO_BORINGSSL
    &chiptest_dac9e1195a0d_test_vector_7,          &chiptest_dac9e1195a0d_test_vector_14,
    &chiptest_dac9e1195a0d_test_vector_21,         &chiptest_dac9e1195a0d_test_vector_28,
    &chiptest_dac9e1195a0d_test_vector_35,         &chiptest_dac9e1195a0d_test_vector_42,
    &chiptest_dac9e1195a0d_test_vector_49,         &chiptest_dac9e1195a0d_test_vector_56,
    &chiptest_dac9e1195a0d_test_vector_63,         &chiptest_dac9e1195a0d_test_vector_70,
    &chiptest_dac9e1195a0d_test_vector_77,         &chiptest_dac9e1195a0d_test_vector_84,
    &chiptest_dac9e1195a0d_test_vector_91,         &chiptest_dac9e1195a0d_test_vector_98,
    &chiptest_dac9e1195a0d_test_vector_105,        &chiptest_dac9e1195a0d_test_vector_112,
    &chiptest_dac9e1195a0d_test_vector_119,        &chiptest_dac9e1195a0d_test_vector_126,
    &chiptest_dac9e1195a0d_test_vector_133,        &chiptest_dac9e1195a0d_test_vector_140,
    &chiptest_dac9e1195a0d_test_vector_147,        &chiptest_dac9e1195a0d_test_vector_154,
    &chiptest_dac9e1195a0d_test_vector_161,        &chiptest_dac9e1195a0d_test_vector_168,
    &chiptest_dac9e1195a0d_test_vector_175,        &chiptest_dac9e1195a0d_test_vector_182,
    &chiptest_dac9e1195a0d_test_vector_189,        &chiptest_dac9e1195a0d_test_vector_196,
    &chiptest_dac9e1195a0d_test_vector_203,        &chiptest_dac9e1195a0d_test_vector_210,
    &chiptest_dac9e1195a0d_test_vector_217,        &chiptest_dac9e1195a0d_test_vector_224,
    &chiptest_dac9e1195a0d_test_vector_231,        &chiptest_dac9e1195a0d_test_vector_238,
    &chiptest_dac9e1195a0d_test_vector_245,        &chiptest_dac9e1195a0d_test_vector_252,
#endif // !CHIP_CRYPTO_BORINGSSL
    &aesccm128_matter_2ef53070ae20_test_vector_0,  &aesccm128_matter_91c8d337cf46_test_vector_1,
    &aesccm128_matter_37fc295e3601_test_vector_2,  &aesccm128_matter_8bab3993e4d1_test_vector_3,
    &aesccm128_matter_76dbd8e42344_test_vector_4,  &aesccm128_matter_ea145e3af464_test_vector_5,
    &aesccm128_matter_08f3cd0b35d1_test_vector_6,  &aesccm128_matter_70d7d4a1f8ea_test_vector_7,
    &aesccm128_matter_56f7e4096627_test_vector_8,  &aesccm128_matter_02112b4a46d8_test_vector_9,
    &aesccm128_matter_ac063659d220_test_vector_10,
};
