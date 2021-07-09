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

typedef struct ccm_test_vector
{
    const uint8_t * key;
    size_t key_len;
    const uint8_t * pt;
    size_t pt_len;
    const uint8_t * ct;
    size_t ct_len;
    const uint8_t * iv;
    size_t iv_len;
    const uint8_t * aad;
    size_t aad_len;
    const uint8_t * tag;
    size_t tag_len;
    unsigned tcId;
    uint64_t result;
} ccm_test_vector;

static const uint8_t chiptest_12cb0ed34854_key_1[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                       0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                       0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_2[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_3[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_4[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_5[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_6[] = { 0xfd, 0x9c, 0x65, 0x82, 0xe4, 0x1c, 0xfa, 0x32 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_7 = { .key     = chiptest_12cb0ed34854_key_1,
                                                                            .key_len = 32,
                                                                            .pt      = chiptest_12cb0ed34854_pt_2,
                                                                            .pt_len  = 0,
                                                                            .ct      = chiptest_12cb0ed34854_ct_3,
                                                                            .ct_len  = 0,
                                                                            .iv      = chiptest_12cb0ed34854_iv_4,
                                                                            .iv_len  = 7,
                                                                            .aad     = chiptest_12cb0ed34854_aad_5,
                                                                            .aad_len = 0,
                                                                            .tag     = chiptest_12cb0ed34854_tag_6,
                                                                            .tag_len = 8,
                                                                            .tcId    = 1,
                                                                            .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_8[]  = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                       0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                       0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_9[]   = {};
static const uint8_t chiptest_12cb0ed34854_ct_10[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_11[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_12[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_13[] = { 0x0d, 0xf6, 0xaa, 0x1e, 0xe8, 0x81, 0x20, 0x1f, 0x44, 0xd4, 0x84, 0x54 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_14 = { .key     = chiptest_12cb0ed34854_key_8,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_9,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_10,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_11,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_12,
                                                                             .aad_len = 0,
                                                                             .tag     = chiptest_12cb0ed34854_tag_13,
                                                                             .tag_len = 12,
                                                                             .tcId    = 2,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_15[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_16[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_17[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_18[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_19[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_20[] = { 0xe6, 0x85, 0x9c, 0x92, 0xca, 0x23, 0x66, 0xbe,
                                                        0x08, 0xb5, 0xaa, 0xbd, 0x0e, 0x21, 0x96, 0xc1 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_21 = { .key     = chiptest_12cb0ed34854_key_15,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_16,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_17,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_18,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_19,
                                                                             .aad_len = 0,
                                                                             .tag     = chiptest_12cb0ed34854_tag_20,
                                                                             .tag_len = 16,
                                                                             .tcId    = 3,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_22[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_23[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_24[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_25[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_26[] = { 0x7c };
static const uint8_t chiptest_12cb0ed34854_tag_27[] = { 0xe7, 0xbc, 0x88, 0x63, 0xab, 0x75, 0x31, 0x12 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_28 = { .key     = chiptest_12cb0ed34854_key_22,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_23,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_24,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_25,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_26,
                                                                             .aad_len = 1,
                                                                             .tag     = chiptest_12cb0ed34854_tag_27,
                                                                             .tag_len = 8,
                                                                             .tcId    = 4,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_29[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_30[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_31[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_32[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_33[] = { 0x7c };
static const uint8_t chiptest_12cb0ed34854_tag_34[] = { 0x96, 0xe8, 0x1c, 0x45, 0xea, 0xe6, 0x9a, 0xbe, 0x1e, 0x0c, 0x90, 0xe4 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_35 = { .key     = chiptest_12cb0ed34854_key_29,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_30,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_31,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_32,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_33,
                                                                             .aad_len = 1,
                                                                             .tag     = chiptest_12cb0ed34854_tag_34,
                                                                             .tag_len = 12,
                                                                             .tcId    = 5,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_36[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_37[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_38[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_39[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_40[] = { 0x7c };
static const uint8_t chiptest_12cb0ed34854_tag_41[] = { 0xfd, 0xe6, 0xa4, 0x32, 0x05, 0xb1, 0xe2, 0x74,
                                                        0x20, 0x4c, 0x6e, 0x3f, 0x66, 0xd0, 0x69, 0xbd };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_42 = { .key     = chiptest_12cb0ed34854_key_36,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_37,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_38,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_39,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_40,
                                                                             .aad_len = 1,
                                                                             .tag     = chiptest_12cb0ed34854_tag_41,
                                                                             .tag_len = 16,
                                                                             .tcId    = 6,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_43[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_44[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_45[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_46[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_47[] = { 0xa1, 0xd0, 0x44, 0x40, 0xaa, 0x76, 0x31, 0x6e, 0x97, 0xdd, 0x6c,
                                                        0xcb, 0x7f, 0x2e, 0xb2, 0x1f, 0x1f, 0x1d, 0x9d, 0x6b, 0x73, 0xde,
                                                        0xeb, 0x56, 0x04, 0xab, 0xb5, 0x6e, 0x45, 0x45, 0x54, 0x4d };
static const uint8_t chiptest_12cb0ed34854_tag_48[] = { 0xcd, 0x32, 0xdf, 0x9d, 0xa5, 0xb5, 0x53, 0x2c };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_49 = { .key     = chiptest_12cb0ed34854_key_43,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_44,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_45,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_46,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_47,
                                                                             .aad_len = 32,
                                                                             .tag     = chiptest_12cb0ed34854_tag_48,
                                                                             .tag_len = 8,
                                                                             .tcId    = 7,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_50[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_51[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_52[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_53[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_54[] = { 0xa1, 0xd0, 0x44, 0x40, 0xaa, 0x76, 0x31, 0x6e, 0x97, 0xdd, 0x6c,
                                                        0xcb, 0x7f, 0x2e, 0xb2, 0x1f, 0x1f, 0x1d, 0x9d, 0x6b, 0x73, 0xde,
                                                        0xeb, 0x56, 0x04, 0xab, 0xb5, 0x6e, 0x45, 0x45, 0x54, 0x4d };
static const uint8_t chiptest_12cb0ed34854_tag_55[] = { 0x98, 0x1b, 0x39, 0xe4, 0xd9, 0x48, 0xd0, 0x6c, 0x5c, 0x73, 0x24, 0x5f };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_56 = { .key     = chiptest_12cb0ed34854_key_50,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_51,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_52,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_53,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_54,
                                                                             .aad_len = 32,
                                                                             .tag     = chiptest_12cb0ed34854_tag_55,
                                                                             .tag_len = 12,
                                                                             .tcId    = 8,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_57[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_58[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_59[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_60[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_61[] = { 0xa1, 0xd0, 0x44, 0x40, 0xaa, 0x76, 0x31, 0x6e, 0x97, 0xdd, 0x6c,
                                                        0xcb, 0x7f, 0x2e, 0xb2, 0x1f, 0x1f, 0x1d, 0x9d, 0x6b, 0x73, 0xde,
                                                        0xeb, 0x56, 0x04, 0xab, 0xb5, 0x6e, 0x45, 0x45, 0x54, 0x4d };
static const uint8_t chiptest_12cb0ed34854_tag_62[] = { 0x3c, 0x0e, 0xc0, 0x5f, 0xa3, 0xbd, 0x3b, 0x44,
                                                        0xd8, 0x91, 0x6d, 0x91, 0x0a, 0xb5, 0x65, 0xd9 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_63 = { .key     = chiptest_12cb0ed34854_key_57,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_58,
                                                                             .pt_len  = 0,
                                                                             .ct      = chiptest_12cb0ed34854_ct_59,
                                                                             .ct_len  = 0,
                                                                             .iv      = chiptest_12cb0ed34854_iv_60,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_61,
                                                                             .aad_len = 32,
                                                                             .tag     = chiptest_12cb0ed34854_tag_62,
                                                                             .tag_len = 16,
                                                                             .tcId    = 9,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_64[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_65[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                       0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_66[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                       0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_67[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_68[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_69[] = { 0xdc, 0xd2, 0x22, 0xd3, 0xa8, 0xfe, 0x64, 0x31 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_70 = { .key     = chiptest_12cb0ed34854_key_64,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_65,
                                                                             .pt_len  = 16,
                                                                             .ct      = chiptest_12cb0ed34854_ct_66,
                                                                             .ct_len  = 16,
                                                                             .iv      = chiptest_12cb0ed34854_iv_67,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_68,
                                                                             .aad_len = 0,
                                                                             .tag     = chiptest_12cb0ed34854_tag_69,
                                                                             .tag_len = 8,
                                                                             .tcId    = 10,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_71[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_72[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                       0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_73[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                       0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_74[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_75[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_76[] = { 0xdf, 0x33, 0xdd, 0x45, 0x15, 0x16, 0x84, 0x18, 0x2e, 0x30, 0x64, 0x27 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_77 = { .key     = chiptest_12cb0ed34854_key_71,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_72,
                                                                             .pt_len  = 16,
                                                                             .ct      = chiptest_12cb0ed34854_ct_73,
                                                                             .ct_len  = 16,
                                                                             .iv      = chiptest_12cb0ed34854_iv_74,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_75,
                                                                             .aad_len = 0,
                                                                             .tag     = chiptest_12cb0ed34854_tag_76,
                                                                             .tag_len = 12,
                                                                             .tcId    = 11,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_78[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_79[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                       0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_80[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                       0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_81[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_82[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_83[] = { 0xdd, 0x5c, 0xac, 0xb1, 0x27, 0x41, 0xf5, 0x2b,
                                                        0xa4, 0x51, 0xef, 0x8b, 0x5e, 0x66, 0xac, 0x9e };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_84 = { .key     = chiptest_12cb0ed34854_key_78,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_79,
                                                                             .pt_len  = 16,
                                                                             .ct      = chiptest_12cb0ed34854_ct_80,
                                                                             .ct_len  = 16,
                                                                             .iv      = chiptest_12cb0ed34854_iv_81,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_82,
                                                                             .aad_len = 0,
                                                                             .tag     = chiptest_12cb0ed34854_tag_83,
                                                                             .tag_len = 16,
                                                                             .tcId    = 12,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_85[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_86[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                       0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_87[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                       0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_88[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_89[] = { 0xda };
static const uint8_t chiptest_12cb0ed34854_tag_90[] = { 0x84, 0x8d, 0x7b, 0xc1, 0x4a, 0x42, 0xbb, 0x56 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_91 = { .key     = chiptest_12cb0ed34854_key_85,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_86,
                                                                             .pt_len  = 16,
                                                                             .ct      = chiptest_12cb0ed34854_ct_87,
                                                                             .ct_len  = 16,
                                                                             .iv      = chiptest_12cb0ed34854_iv_88,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_89,
                                                                             .aad_len = 1,
                                                                             .tag     = chiptest_12cb0ed34854_tag_90,
                                                                             .tag_len = 8,
                                                                             .tcId    = 13,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_92[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_93[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                       0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_94[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                       0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_95[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_96[] = { 0xda };
static const uint8_t chiptest_12cb0ed34854_tag_97[] = { 0x43, 0x46, 0x00, 0xf5, 0xb7, 0xc5, 0x59, 0x85, 0x12, 0x79, 0xea, 0xfe };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_98 = { .key     = chiptest_12cb0ed34854_key_92,
                                                                             .key_len = 32,
                                                                             .pt      = chiptest_12cb0ed34854_pt_93,
                                                                             .pt_len  = 16,
                                                                             .ct      = chiptest_12cb0ed34854_ct_94,
                                                                             .ct_len  = 16,
                                                                             .iv      = chiptest_12cb0ed34854_iv_95,
                                                                             .iv_len  = 7,
                                                                             .aad     = chiptest_12cb0ed34854_aad_96,
                                                                             .aad_len = 1,
                                                                             .tag     = chiptest_12cb0ed34854_tag_97,
                                                                             .tag_len = 12,
                                                                             .tcId    = 14,
                                                                             .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_99[]  = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                        0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                        0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_100[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                        0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_101[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                        0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_102[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_103[] = { 0xda };
static const uint8_t chiptest_12cb0ed34854_tag_104[] = { 0x10, 0x8d, 0x3c, 0xcf, 0xee, 0x1c, 0xed, 0xcd,
                                                         0x1e, 0xef, 0x8c, 0x6b, 0xda, 0xbf, 0xa4, 0xf9 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_105 = { .key     = chiptest_12cb0ed34854_key_99,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_100,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_101,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_102,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_103,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_104,
                                                                              .tag_len = 16,
                                                                              .tcId    = 15,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_106[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_107[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                        0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_108[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                        0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_109[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_110[] = { 0x67, 0xc0, 0xf4, 0xac, 0xb9, 0x6f, 0x73, 0x5e, 0xd0, 0xa2, 0xcf,
                                                         0x95, 0x8a, 0x7c, 0xc3, 0xc5, 0xf7, 0x96, 0xf5, 0xde, 0x40, 0xcd,
                                                         0x99, 0x8f, 0xdd, 0xb9, 0xa3, 0x0b, 0x2f, 0x6e, 0x74, 0x5f };
static const uint8_t chiptest_12cb0ed34854_tag_111[] = { 0xee, 0xfd, 0x63, 0x48, 0xf1, 0x79, 0x34, 0x5f };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_112 = { .key     = chiptest_12cb0ed34854_key_106,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_107,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_108,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_109,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_110,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_111,
                                                                              .tag_len = 8,
                                                                              .tcId    = 16,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_113[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_114[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                        0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_115[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                        0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_116[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_117[] = { 0x67, 0xc0, 0xf4, 0xac, 0xb9, 0x6f, 0x73, 0x5e, 0xd0, 0xa2, 0xcf,
                                                         0x95, 0x8a, 0x7c, 0xc3, 0xc5, 0xf7, 0x96, 0xf5, 0xde, 0x40, 0xcd,
                                                         0x99, 0x8f, 0xdd, 0xb9, 0xa3, 0x0b, 0x2f, 0x6e, 0x74, 0x5f };
static const uint8_t chiptest_12cb0ed34854_tag_118[] = { 0x19, 0xb6, 0x25, 0x1d, 0xbf, 0x0b, 0x07, 0x43, 0x46, 0xfc, 0xd1, 0x62 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_119 = { .key     = chiptest_12cb0ed34854_key_113,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_114,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_115,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_116,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_117,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_118,
                                                                              .tag_len = 12,
                                                                              .tcId    = 17,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_120[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_121[]  = { 0xe4, 0xe6, 0xfe, 0x17, 0xc6, 0xd6, 0xd0, 0xc8,
                                                        0x04, 0x34, 0x34, 0xfc, 0x09, 0x1a, 0xda, 0xc7 };
static const uint8_t chiptest_12cb0ed34854_ct_122[]  = { 0xb3, 0xf7, 0x94, 0x08, 0xd1, 0xdf, 0x32, 0x5d,
                                                        0xa3, 0x77, 0x90, 0xf4, 0x7d, 0x48, 0xce, 0xf9 };
static const uint8_t chiptest_12cb0ed34854_iv_123[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_124[] = { 0x67, 0xc0, 0xf4, 0xac, 0xb9, 0x6f, 0x73, 0x5e, 0xd0, 0xa2, 0xcf,
                                                         0x95, 0x8a, 0x7c, 0xc3, 0xc5, 0xf7, 0x96, 0xf5, 0xde, 0x40, 0xcd,
                                                         0x99, 0x8f, 0xdd, 0xb9, 0xa3, 0x0b, 0x2f, 0x6e, 0x74, 0x5f };
static const uint8_t chiptest_12cb0ed34854_tag_125[] = { 0x96, 0x42, 0x3d, 0x41, 0xbb, 0x98, 0x05, 0x37,
                                                         0x1f, 0x3f, 0x78, 0x4f, 0xa4, 0x76, 0xa4, 0x79 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_126 = { .key     = chiptest_12cb0ed34854_key_120,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_121,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_122,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_123,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_124,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_125,
                                                                              .tag_len = 16,
                                                                              .tcId    = 18,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_127[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_128[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_129[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_130[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_131[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_132[] = { 0x0e, 0x87, 0x96, 0xd6, 0x32, 0xc9, 0xb3, 0x2e };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_133 = { .key     = chiptest_12cb0ed34854_key_127,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_128,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_129,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_130,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_131,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_132,
                                                                              .tag_len = 8,
                                                                              .tcId    = 19,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_134[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_135[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_136[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_137[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_138[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_139[] = { 0x4f, 0x3a, 0x4c, 0x11, 0x07, 0x44, 0x86, 0x9e, 0xd1, 0x4d, 0x53, 0xaa };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_140 = { .key     = chiptest_12cb0ed34854_key_134,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_135,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_136,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_137,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_138,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_139,
                                                                              .tag_len = 12,
                                                                              .tcId    = 20,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_141[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_142[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_143[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_144[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_145[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_146[] = { 0xab, 0x73, 0x1f, 0xc8, 0x0a, 0xde, 0x38, 0xf1,
                                                         0xa9, 0x84, 0x60, 0x6e, 0xae, 0x05, 0xa0, 0x2b };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_147 = { .key     = chiptest_12cb0ed34854_key_141,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_142,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_143,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_144,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_145,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_146,
                                                                              .tag_len = 16,
                                                                              .tcId    = 21,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_148[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_149[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_150[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_151[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_152[] = { 0xf2 };
static const uint8_t chiptest_12cb0ed34854_tag_153[] = { 0x5f, 0x5e, 0xce, 0x87, 0x2f, 0x03, 0xe5, 0x07 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_154 = { .key     = chiptest_12cb0ed34854_key_148,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_149,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_150,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_151,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_152,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_153,
                                                                              .tag_len = 8,
                                                                              .tcId    = 22,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_155[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_156[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_157[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_158[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_159[] = { 0xf2 };
static const uint8_t chiptest_12cb0ed34854_tag_160[] = { 0x48, 0x35, 0x5c, 0xc2, 0xb4, 0x61, 0x8b, 0xd2, 0x4e, 0xa3, 0xe4, 0xc0 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_161 = { .key     = chiptest_12cb0ed34854_key_155,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_156,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_157,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_158,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_159,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_160,
                                                                              .tag_len = 12,
                                                                              .tcId    = 23,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_162[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_163[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_164[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_165[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_166[] = { 0xf2 };
static const uint8_t chiptest_12cb0ed34854_tag_167[] = { 0xf0, 0x89, 0x5d, 0xfa, 0x19, 0x3e, 0x56, 0x29,
                                                         0x62, 0x25, 0x5e, 0x24, 0xf5, 0x76, 0x57, 0x73 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_168 = { .key     = chiptest_12cb0ed34854_key_162,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_163,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_164,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_165,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_166,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_167,
                                                                              .tag_len = 16,
                                                                              .tcId    = 24,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_169[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_170[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_171[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_172[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_173[] = { 0x2d, 0x1e, 0x30, 0xdd, 0x3b, 0xbf, 0x40, 0xb2, 0xcd, 0x7c, 0x3d,
                                                         0x57, 0x45, 0xd5, 0x36, 0xcf, 0x38, 0x5e, 0x8c, 0xe5, 0xea, 0xf9,
                                                         0x40, 0xf4, 0x79, 0xf9, 0x73, 0x0e, 0x4c, 0x55, 0xef, 0x87 };
static const uint8_t chiptest_12cb0ed34854_tag_174[] = { 0x6a, 0x91, 0xf2, 0x05, 0xd1, 0x27, 0x02, 0x24 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_175 = { .key     = chiptest_12cb0ed34854_key_169,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_170,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_171,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_172,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_173,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_174,
                                                                              .tag_len = 8,
                                                                              .tcId    = 25,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_176[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_177[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_178[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_179[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_180[] = { 0x2d, 0x1e, 0x30, 0xdd, 0x3b, 0xbf, 0x40, 0xb2, 0xcd, 0x7c, 0x3d,
                                                         0x57, 0x45, 0xd5, 0x36, 0xcf, 0x38, 0x5e, 0x8c, 0xe5, 0xea, 0xf9,
                                                         0x40, 0xf4, 0x79, 0xf9, 0x73, 0x0e, 0x4c, 0x55, 0xef, 0x87 };
static const uint8_t chiptest_12cb0ed34854_tag_181[] = { 0xab, 0x65, 0x12, 0xab, 0x92, 0xf5, 0x89, 0x98, 0x5d, 0x72, 0xa0, 0xfd };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_182 = { .key     = chiptest_12cb0ed34854_key_176,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_177,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_178,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_179,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_180,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_181,
                                                                              .tag_len = 12,
                                                                              .tcId    = 26,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_183[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_184[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                        0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                        0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_185[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                        0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xec, 0xcf, 0x6a, 0x9c,
                                                        0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_186[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_187[] = { 0x2d, 0x1e, 0x30, 0xdd, 0x3b, 0xbf, 0x40, 0xb2, 0xcd, 0x7c, 0x3d,
                                                         0x57, 0x45, 0xd5, 0x36, 0xcf, 0x38, 0x5e, 0x8c, 0xe5, 0xea, 0xf9,
                                                         0x40, 0xf4, 0x79, 0xf9, 0x73, 0x0e, 0x4c, 0x55, 0xef, 0x87 };
static const uint8_t chiptest_12cb0ed34854_tag_188[] = { 0xe1, 0x75, 0xe0, 0x33, 0x4b, 0x4e, 0x23, 0x4c,
                                                         0x1e, 0xce, 0x5c, 0x73, 0x9e, 0xd1, 0x2c, 0x28 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_189 = { .key     = chiptest_12cb0ed34854_key_183,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_184,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_185,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_186,
                                                                              .iv_len  = 7,
                                                                              .aad     = chiptest_12cb0ed34854_aad_187,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_188,
                                                                              .tag_len = 16,
                                                                              .tcId    = 27,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_190[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_191[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_192[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_193[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_194[]                      = {};
static const uint8_t chiptest_12cb0ed34854_tag_195[]                      = { 0x42, 0x26, 0x5a, 0xcf, 0xad, 0xa3, 0x71, 0xd5 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_196 = { .key     = chiptest_12cb0ed34854_key_190,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_191,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_192,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_193,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_194,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_195,
                                                                              .tag_len = 8,
                                                                              .tcId    = 28,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_197[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_198[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_199[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_200[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_201[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_202[] = { 0x1c, 0xec, 0x5b, 0xff, 0xcf, 0xb4, 0xd7, 0x02, 0x34, 0xb3, 0xb0, 0x41 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_203 = { .key     = chiptest_12cb0ed34854_key_197,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_198,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_199,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_200,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_201,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_202,
                                                                              .tag_len = 12,
                                                                              .tcId    = 29,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_204[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_205[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_206[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_207[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_208[]                      = {};
static const uint8_t chiptest_12cb0ed34854_tag_209[]                      = { 0x5d, 0x79, 0x73, 0xc2, 0x5f, 0xe1, 0x6d, 0x69,
                                                         0x40, 0xfb, 0xca, 0xff, 0x79, 0x86, 0xe3, 0x3e };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_210 = { .key     = chiptest_12cb0ed34854_key_204,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_205,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_206,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_207,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_208,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_209,
                                                                              .tag_len = 16,
                                                                              .tcId    = 30,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_211[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_212[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_213[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_214[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_215[]                      = { 0xeb };
static const uint8_t chiptest_12cb0ed34854_tag_216[]                      = { 0xaf, 0xc6, 0xf9, 0x48, 0xae, 0x21, 0xc2, 0x7c };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_217 = { .key     = chiptest_12cb0ed34854_key_211,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_212,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_213,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_214,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_215,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_216,
                                                                              .tag_len = 8,
                                                                              .tcId    = 31,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_218[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_219[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_220[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_221[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_222[] = { 0xeb };
static const uint8_t chiptest_12cb0ed34854_tag_223[] = { 0xd7, 0x71, 0x56, 0x7f, 0xee, 0x7b, 0x52, 0x2a, 0x95, 0x6a, 0x86, 0x5e };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_224 = { .key     = chiptest_12cb0ed34854_key_218,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_219,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_220,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_221,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_222,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_223,
                                                                              .tag_len = 12,
                                                                              .tcId    = 32,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_225[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_226[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_227[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_228[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_229[]                      = { 0xeb };
static const uint8_t chiptest_12cb0ed34854_tag_230[]                      = { 0x6a, 0x94, 0x0c, 0x32, 0x1a, 0xa4, 0x22, 0xd6,
                                                         0x34, 0x6b, 0x83, 0x9f, 0x88, 0x90, 0x0d, 0xd0 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_231 = { .key     = chiptest_12cb0ed34854_key_225,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_226,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_227,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_228,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_229,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_230,
                                                                              .tag_len = 16,
                                                                              .tcId    = 33,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_232[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_233[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_234[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_235[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_236[] = { 0x7e, 0x39, 0xdd, 0x42, 0xab, 0xd1, 0xca, 0x47, 0x38, 0x3f, 0x31,
                                                         0xb5, 0x2c, 0x12, 0x4a, 0x5d, 0xba, 0xc4, 0xfe, 0x43, 0xb3, 0x0d,
                                                         0xed, 0x71, 0xb6, 0xca, 0x05, 0x40, 0x14, 0xbf, 0xb6, 0x00 };
static const uint8_t chiptest_12cb0ed34854_tag_237[] = { 0xa3, 0x8f, 0x71, 0x32, 0xfa, 0xb8, 0x43, 0xea };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_238 = { .key     = chiptest_12cb0ed34854_key_232,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_233,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_234,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_235,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_236,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_237,
                                                                              .tag_len = 8,
                                                                              .tcId    = 34,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_239[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_240[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_241[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_242[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_243[] = { 0x7e, 0x39, 0xdd, 0x42, 0xab, 0xd1, 0xca, 0x47, 0x38, 0x3f, 0x31,
                                                         0xb5, 0x2c, 0x12, 0x4a, 0x5d, 0xba, 0xc4, 0xfe, 0x43, 0xb3, 0x0d,
                                                         0xed, 0x71, 0xb6, 0xca, 0x05, 0x40, 0x14, 0xbf, 0xb6, 0x00 };
static const uint8_t chiptest_12cb0ed34854_tag_244[] = { 0x59, 0xe5, 0xf7, 0x0c, 0x8e, 0x86, 0x32, 0x6b, 0x61, 0x66, 0x77, 0x7c };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_245 = { .key     = chiptest_12cb0ed34854_key_239,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_240,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_241,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_242,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_243,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_244,
                                                                              .tag_len = 12,
                                                                              .tcId    = 35,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_246[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_247[]  = {};
static const uint8_t chiptest_12cb0ed34854_ct_248[]  = {};
static const uint8_t chiptest_12cb0ed34854_iv_249[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_250[] = { 0x7e, 0x39, 0xdd, 0x42, 0xab, 0xd1, 0xca, 0x47, 0x38, 0x3f, 0x31,
                                                         0xb5, 0x2c, 0x12, 0x4a, 0x5d, 0xba, 0xc4, 0xfe, 0x43, 0xb3, 0x0d,
                                                         0xed, 0x71, 0xb6, 0xca, 0x05, 0x40, 0x14, 0xbf, 0xb6, 0x00 };
static const uint8_t chiptest_12cb0ed34854_tag_251[] = { 0x25, 0x46, 0x20, 0x97, 0x7f, 0x47, 0xbf, 0xd6,
                                                         0x74, 0x30, 0xf3, 0xed, 0x01, 0xbc, 0x6b, 0x31 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_252 = { .key     = chiptest_12cb0ed34854_key_246,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_247,
                                                                              .pt_len  = 0,
                                                                              .ct      = chiptest_12cb0ed34854_ct_248,
                                                                              .ct_len  = 0,
                                                                              .iv      = chiptest_12cb0ed34854_iv_249,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_250,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_251,
                                                                              .tag_len = 16,
                                                                              .tcId    = 36,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_253[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_254[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_255[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_256[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_257[]                      = {};
static const uint8_t chiptest_12cb0ed34854_tag_258[]                      = { 0x43, 0xd5, 0xd9, 0x8c, 0xa0, 0xa2, 0x25, 0x50 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_259 = { .key     = chiptest_12cb0ed34854_key_253,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_254,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_255,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_256,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_257,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_258,
                                                                              .tag_len = 8,
                                                                              .tcId    = 37,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_260[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_261[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_262[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_263[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_264[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_265[] = { 0x80, 0x8f, 0x50, 0x04, 0xad, 0x52, 0xb0, 0x37, 0xbc, 0x38, 0xe1, 0xa5 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_266 = { .key     = chiptest_12cb0ed34854_key_260,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_261,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_262,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_263,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_264,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_265,
                                                                              .tag_len = 12,
                                                                              .tcId    = 38,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_267[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_268[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_269[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_270[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_271[]                      = {};
static const uint8_t chiptest_12cb0ed34854_tag_272[]                      = { 0x1f, 0x0b, 0x8f, 0x8b, 0x7d, 0xf5, 0x46, 0xa9,
                                                         0x93, 0x36, 0x6a, 0x02, 0x6f, 0x0c, 0x2d, 0x61 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_273 = { .key     = chiptest_12cb0ed34854_key_267,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_268,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_269,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_270,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_271,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_272,
                                                                              .tag_len = 16,
                                                                              .tcId    = 39,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_274[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_275[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_276[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_277[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_278[]                      = { 0xaa };
static const uint8_t chiptest_12cb0ed34854_tag_279[]                      = { 0xd8, 0x57, 0x1b, 0x1e, 0x92, 0xf4, 0x58, 0x27 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_280 = { .key     = chiptest_12cb0ed34854_key_274,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_275,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_276,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_277,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_278,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_279,
                                                                              .tag_len = 8,
                                                                              .tcId    = 40,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_281[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_282[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_283[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_284[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_285[] = { 0xaa };
static const uint8_t chiptest_12cb0ed34854_tag_286[] = { 0xef, 0xbf, 0xf4, 0xb4, 0x81, 0xb8, 0xc3, 0xd4, 0x55, 0xed, 0x75, 0x11 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_287 = { .key     = chiptest_12cb0ed34854_key_281,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_282,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_283,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_284,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_285,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_286,
                                                                              .tag_len = 12,
                                                                              .tcId    = 41,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_288[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_289[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_290[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_291[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_292[]                      = { 0xaa };
static const uint8_t chiptest_12cb0ed34854_tag_293[]                      = { 0x67, 0x15, 0x75, 0xcd, 0xb2, 0xd9, 0x80, 0xae,
                                                         0x02, 0x3a, 0x1f, 0xd6, 0xc1, 0xa6, 0x67, 0x52 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_294 = { .key     = chiptest_12cb0ed34854_key_288,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_289,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_290,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_291,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_292,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_293,
                                                                              .tag_len = 16,
                                                                              .tcId    = 42,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_295[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_296[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_297[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_298[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_299[] = { 0x97, 0x40, 0xdd, 0x5c, 0xa3, 0x0d, 0x59, 0x86, 0x7b, 0x01, 0x0f,
                                                         0xe3, 0x1a, 0xda, 0x21, 0x41, 0x4c, 0xfd, 0x30, 0xa4, 0x4a, 0x2a,
                                                         0xa0, 0x2a, 0x46, 0xcb, 0xfd, 0xaf, 0x94, 0x7e, 0x0a, 0x3b };
static const uint8_t chiptest_12cb0ed34854_tag_300[] = { 0xcf, 0x82, 0x65, 0x1d, 0x06, 0x79, 0xae, 0x2f };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_301 = { .key     = chiptest_12cb0ed34854_key_295,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_296,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_297,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_298,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_299,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_300,
                                                                              .tag_len = 8,
                                                                              .tcId    = 43,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_302[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_303[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_304[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_305[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_306[] = { 0x97, 0x40, 0xdd, 0x5c, 0xa3, 0x0d, 0x59, 0x86, 0x7b, 0x01, 0x0f,
                                                         0xe3, 0x1a, 0xda, 0x21, 0x41, 0x4c, 0xfd, 0x30, 0xa4, 0x4a, 0x2a,
                                                         0xa0, 0x2a, 0x46, 0xcb, 0xfd, 0xaf, 0x94, 0x7e, 0x0a, 0x3b };
static const uint8_t chiptest_12cb0ed34854_tag_307[] = { 0x2c, 0x03, 0xb6, 0x3a, 0xf7, 0x2a, 0x9c, 0x39, 0x24, 0x55, 0xd3, 0x43 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_308 = { .key     = chiptest_12cb0ed34854_key_302,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_303,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_304,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_305,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_306,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_307,
                                                                              .tag_len = 12,
                                                                              .tcId    = 44,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_309[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_310[]  = { 0xae, 0x0d, 0x82, 0x7b, 0xb5, 0xc3, 0x76, 0x7a,
                                                        0x7f, 0xec, 0x3e, 0x96, 0xcc, 0x3f, 0x4a, 0x3c };
static const uint8_t chiptest_12cb0ed34854_ct_311[]  = { 0x62, 0xc5, 0x1e, 0x87, 0x8e, 0xc5, 0x90, 0xa4,
                                                        0x87, 0x74, 0xa7, 0x42, 0x37, 0x8b, 0x4e, 0xd4 };
static const uint8_t chiptest_12cb0ed34854_iv_312[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_313[] = { 0x97, 0x40, 0xdd, 0x5c, 0xa3, 0x0d, 0x59, 0x86, 0x7b, 0x01, 0x0f,
                                                         0xe3, 0x1a, 0xda, 0x21, 0x41, 0x4c, 0xfd, 0x30, 0xa4, 0x4a, 0x2a,
                                                         0xa0, 0x2a, 0x46, 0xcb, 0xfd, 0xaf, 0x94, 0x7e, 0x0a, 0x3b };
static const uint8_t chiptest_12cb0ed34854_tag_314[] = { 0x02, 0x24, 0xde, 0x7c, 0xeb, 0x9d, 0xe4, 0x6c,
                                                         0xad, 0xee, 0xb9, 0x91, 0x17, 0x78, 0xdd, 0x5c };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_315 = { .key     = chiptest_12cb0ed34854_key_309,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_310,
                                                                              .pt_len  = 16,
                                                                              .ct      = chiptest_12cb0ed34854_ct_311,
                                                                              .ct_len  = 16,
                                                                              .iv      = chiptest_12cb0ed34854_iv_312,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_313,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_314,
                                                                              .tag_len = 16,
                                                                              .tcId    = 45,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_316[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_317[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_318[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_319[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_320[]                      = {};
static const uint8_t chiptest_12cb0ed34854_tag_321[]                      = { 0xeb, 0x2e, 0x20, 0xb6, 0x30, 0x40, 0x48, 0x7e };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_322 = { .key     = chiptest_12cb0ed34854_key_316,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_317,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_318,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_319,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_320,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_321,
                                                                              .tag_len = 8,
                                                                              .tcId    = 46,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_323[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_324[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_325[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_326[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_327[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_328[] = { 0x6a, 0xa7, 0xc4, 0x5d, 0x28, 0x5e, 0xce, 0x89, 0xd3, 0xe6, 0x99, 0x80 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_329 = { .key     = chiptest_12cb0ed34854_key_323,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_324,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_325,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_326,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_327,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_328,
                                                                              .tag_len = 12,
                                                                              .tcId    = 47,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_330[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_331[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_332[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_333[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_334[]                      = {};
static const uint8_t chiptest_12cb0ed34854_tag_335[]                      = { 0xfe, 0xbb, 0xb8, 0xc8, 0xf1, 0xce, 0x2a, 0xbd,
                                                         0x6f, 0x7e, 0x4c, 0x94, 0x20, 0x59, 0xc6, 0x50 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_336 = { .key     = chiptest_12cb0ed34854_key_330,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_331,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_332,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_333,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_334,
                                                                              .aad_len = 0,
                                                                              .tag     = chiptest_12cb0ed34854_tag_335,
                                                                              .tag_len = 16,
                                                                              .tcId    = 48,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_337[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_338[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_339[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_340[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_341[]                      = { 0x2e };
static const uint8_t chiptest_12cb0ed34854_tag_342[]                      = { 0xa6, 0xfe, 0xf8, 0xd7, 0x72, 0x8a, 0xb1, 0xe2 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_343 = { .key     = chiptest_12cb0ed34854_key_337,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_338,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_339,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_340,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_341,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_342,
                                                                              .tag_len = 8,
                                                                              .tcId    = 49,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_344[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_345[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_346[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_347[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_348[] = { 0x2e };
static const uint8_t chiptest_12cb0ed34854_tag_349[] = { 0xc4, 0xce, 0x9d, 0xc5, 0xbc, 0x36, 0xb7, 0xa0, 0xc0, 0x35, 0x6f, 0xd6 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_350 = { .key     = chiptest_12cb0ed34854_key_344,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_345,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_346,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_347,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_348,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_349,
                                                                              .tag_len = 12,
                                                                              .tcId    = 50,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_351[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_352[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_353[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_354[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_355[]                      = { 0x2e };
static const uint8_t chiptest_12cb0ed34854_tag_356[]                      = { 0xd4, 0x35, 0x7e, 0x1a, 0x3e, 0x22, 0xd0, 0x37,
                                                         0x25, 0x9d, 0x7b, 0xb0, 0x86, 0x32, 0x0a, 0x81 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_357 = { .key     = chiptest_12cb0ed34854_key_351,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_352,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_353,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_354,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_355,
                                                                              .aad_len = 1,
                                                                              .tag     = chiptest_12cb0ed34854_tag_356,
                                                                              .tag_len = 16,
                                                                              .tcId    = 51,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_358[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_359[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_360[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_361[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_362[] = { 0x98, 0x5b, 0xf0, 0x38, 0x16, 0xe9, 0x29, 0xce, 0x66, 0x81, 0x0d,
                                                         0x7e, 0x1a, 0x78, 0x46, 0xc9, 0x1e, 0x05, 0x68, 0x6d, 0x0e, 0xcf,
                                                         0x8f, 0x94, 0x31, 0x0a, 0x37, 0xa1, 0xc0, 0x76, 0x1b, 0x04 };
static const uint8_t chiptest_12cb0ed34854_tag_363[] = { 0x5f, 0x56, 0xb2, 0x9e, 0xc3, 0xdd, 0x21, 0x2d };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_364 = { .key     = chiptest_12cb0ed34854_key_358,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_359,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_360,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_361,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_362,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_363,
                                                                              .tag_len = 8,
                                                                              .tcId    = 52,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_365[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_366[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_367[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_368[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_369[] = { 0x98, 0x5b, 0xf0, 0x38, 0x16, 0xe9, 0x29, 0xce, 0x66, 0x81, 0x0d,
                                                         0x7e, 0x1a, 0x78, 0x46, 0xc9, 0x1e, 0x05, 0x68, 0x6d, 0x0e, 0xcf,
                                                         0x8f, 0x94, 0x31, 0x0a, 0x37, 0xa1, 0xc0, 0x76, 0x1b, 0x04 };
static const uint8_t chiptest_12cb0ed34854_tag_370[] = { 0x11, 0xcf, 0x30, 0xd3, 0xdf, 0x32, 0x4b, 0xa0, 0xe4, 0x82, 0x64, 0x8a };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_371 = { .key     = chiptest_12cb0ed34854_key_365,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_366,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_367,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_368,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_369,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_370,
                                                                              .tag_len = 12,
                                                                              .tcId    = 53,
                                                                              .result  = 1 };
static const uint8_t chiptest_12cb0ed34854_key_372[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                         0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                         0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_373[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                        0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                        0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_374[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                        0x10, 0x23, 0x3b, 0x5c, 0x70, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                        0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_375[]  = {
    0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d, 0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9
};
static const uint8_t chiptest_12cb0ed34854_aad_376[] = { 0x98, 0x5b, 0xf0, 0x38, 0x16, 0xe9, 0x29, 0xce, 0x66, 0x81, 0x0d,
                                                         0x7e, 0x1a, 0x78, 0x46, 0xc9, 0x1e, 0x05, 0x68, 0x6d, 0x0e, 0xcf,
                                                         0x8f, 0x94, 0x31, 0x0a, 0x37, 0xa1, 0xc0, 0x76, 0x1b, 0x04 };
static const uint8_t chiptest_12cb0ed34854_tag_377[] = { 0x07, 0x57, 0x3b, 0xc7, 0x7f, 0xa9, 0x58, 0x63,
                                                         0xde, 0xc4, 0x16, 0xd6, 0xbe, 0x3b, 0x1e, 0xb3 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_378 = { .key     = chiptest_12cb0ed34854_key_372,
                                                                              .key_len = 32,
                                                                              .pt      = chiptest_12cb0ed34854_pt_373,
                                                                              .pt_len  = 33,
                                                                              .ct      = chiptest_12cb0ed34854_ct_374,
                                                                              .ct_len  = 33,
                                                                              .iv      = chiptest_12cb0ed34854_iv_375,
                                                                              .iv_len  = 13,
                                                                              .aad     = chiptest_12cb0ed34854_aad_376,
                                                                              .aad_len = 32,
                                                                              .tag     = chiptest_12cb0ed34854_tag_377,
                                                                              .tag_len = 16,
                                                                              .tcId    = 54,
                                                                              .result  = 1 };
static const struct ccm_test_vector * ccm_test_vectors[]                  = {
    &chiptest_12cb0ed34854_test_vector_7,   &chiptest_12cb0ed34854_test_vector_14,  &chiptest_12cb0ed34854_test_vector_21,
    &chiptest_12cb0ed34854_test_vector_28,  &chiptest_12cb0ed34854_test_vector_35,  &chiptest_12cb0ed34854_test_vector_42,
    &chiptest_12cb0ed34854_test_vector_49,  &chiptest_12cb0ed34854_test_vector_56,  &chiptest_12cb0ed34854_test_vector_63,
    &chiptest_12cb0ed34854_test_vector_70,  &chiptest_12cb0ed34854_test_vector_77,  &chiptest_12cb0ed34854_test_vector_84,
    &chiptest_12cb0ed34854_test_vector_91,  &chiptest_12cb0ed34854_test_vector_98,  &chiptest_12cb0ed34854_test_vector_105,
    &chiptest_12cb0ed34854_test_vector_112, &chiptest_12cb0ed34854_test_vector_119, &chiptest_12cb0ed34854_test_vector_126,
    &chiptest_12cb0ed34854_test_vector_133, &chiptest_12cb0ed34854_test_vector_140, &chiptest_12cb0ed34854_test_vector_147,
    &chiptest_12cb0ed34854_test_vector_154, &chiptest_12cb0ed34854_test_vector_161, &chiptest_12cb0ed34854_test_vector_168,
    &chiptest_12cb0ed34854_test_vector_175, &chiptest_12cb0ed34854_test_vector_182, &chiptest_12cb0ed34854_test_vector_189,
    &chiptest_12cb0ed34854_test_vector_196, &chiptest_12cb0ed34854_test_vector_203, &chiptest_12cb0ed34854_test_vector_210,
    &chiptest_12cb0ed34854_test_vector_217, &chiptest_12cb0ed34854_test_vector_224, &chiptest_12cb0ed34854_test_vector_231,
    &chiptest_12cb0ed34854_test_vector_238, &chiptest_12cb0ed34854_test_vector_245, &chiptest_12cb0ed34854_test_vector_252,
    &chiptest_12cb0ed34854_test_vector_259, &chiptest_12cb0ed34854_test_vector_266, &chiptest_12cb0ed34854_test_vector_273,
    &chiptest_12cb0ed34854_test_vector_280, &chiptest_12cb0ed34854_test_vector_287, &chiptest_12cb0ed34854_test_vector_294,
    &chiptest_12cb0ed34854_test_vector_301, &chiptest_12cb0ed34854_test_vector_308, &chiptest_12cb0ed34854_test_vector_315,
    &chiptest_12cb0ed34854_test_vector_322, &chiptest_12cb0ed34854_test_vector_329, &chiptest_12cb0ed34854_test_vector_336,
    &chiptest_12cb0ed34854_test_vector_343, &chiptest_12cb0ed34854_test_vector_350, &chiptest_12cb0ed34854_test_vector_357,
    &chiptest_12cb0ed34854_test_vector_364, &chiptest_12cb0ed34854_test_vector_371, &chiptest_12cb0ed34854_test_vector_378
};

// These are some invalid vectors where the ct bits have been flipped.
static const uint8_t chiptest_12cb0ed34854_key_3721[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                          0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                          0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_3731[]  = { 0xee, 0xc3, 0xe6, 0xc0, 0xe5, 0x62, 0xa7, 0xea, 0xe0, 0x5c, 0x3d,
                                                         0xc4, 0xd8, 0x8f, 0x58, 0x98, 0x58, 0x17, 0xcb, 0x02, 0xa5, 0xae,
                                                         0x72, 0x03, 0xce, 0x79, 0x9f, 0x73, 0x4d, 0xfd, 0x25, 0xfa, 0x9a };
static const uint8_t chiptest_12cb0ed34854_ct_3741[]  = { 0x22, 0x0b, 0x7a, 0x3c, 0xde, 0x64, 0x41, 0x34, 0x18, 0xc4, 0xa4,
                                                         0x10, 0x23, 0x3b, 0x5c, 0x71, 0x85, 0x0e, 0x74, 0x40, 0xb9, 0x4b,
                                                         0x03, 0xad, 0xaa, 0xd7, 0x76, 0x6c, 0xe7, 0x13, 0xc5, 0x6d, 0xff };
static const uint8_t chiptest_12cb0ed34854_iv_3751[]  = { 0x95, 0xf9, 0x61, 0x8a, 0x2c, 0x8f, 0x0d,
                                                         0x28, 0xb5, 0xbb, 0xe5, 0x8c, 0xe9 };
static const uint8_t chiptest_12cb0ed34854_aad_3761[] = { 0x98, 0x5b, 0xf0, 0x38, 0x16, 0xe9, 0x29, 0xce, 0x66, 0x81, 0x0d,
                                                          0x7e, 0x1a, 0x78, 0x46, 0xc9, 0x1e, 0x05, 0x68, 0x6d, 0x0e, 0xcf,
                                                          0x8f, 0x94, 0x31, 0x0a, 0x37, 0xa1, 0xc0, 0x76, 0x1b, 0x04 };
static const uint8_t chiptest_12cb0ed34854_tag_3771[] = { 0x07, 0x57, 0x3b, 0xc7, 0x7f, 0xa9, 0x58, 0x63,
                                                          0xde, 0xc4, 0x16, 0xd6, 0xbe, 0x3b, 0x1e, 0xb3 };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_3781 = { .key     = chiptest_12cb0ed34854_key_3721,
                                                                               .key_len = 32,
                                                                               .pt      = chiptest_12cb0ed34854_pt_3731,
                                                                               .pt_len  = 33,
                                                                               .ct      = chiptest_12cb0ed34854_ct_3741,
                                                                               .ct_len  = 33,
                                                                               .iv      = chiptest_12cb0ed34854_iv_3751,
                                                                               .iv_len  = 13,
                                                                               .aad     = chiptest_12cb0ed34854_aad_3761,
                                                                               .aad_len = 32,
                                                                               .tag     = chiptest_12cb0ed34854_tag_3771,
                                                                               .tag_len = 16,
                                                                               .tcId    = 54,
                                                                               .result  = 1 };

static const uint8_t chiptest_12cb0ed34854_key_1411[] = { 0x4a, 0x45, 0x65, 0x85, 0xb8, 0xd3, 0xd2, 0xf2, 0x39, 0x51, 0xf2,
                                                          0x74, 0xbd, 0x98, 0xe6, 0x65, 0x5e, 0xd5, 0x3f, 0x3c, 0xec, 0x05,
                                                          0xa4, 0x65, 0xd0, 0x20, 0xb0, 0xdf, 0x6a, 0x33, 0x45, 0xd5 };
static const uint8_t chiptest_12cb0ed34854_pt_1421[]  = { 0xcd, 0x59, 0xde, 0x72, 0x61, 0x2d, 0x17, 0x54, 0xf8, 0x26, 0xe1,
                                                         0x99, 0x65, 0x6b, 0x47, 0x21, 0x50, 0xd8, 0xf5, 0x9a, 0xf1, 0x5f,
                                                         0xba, 0x7d, 0x49, 0xd7, 0xa3, 0x2b, 0x7f, 0xb4, 0x11, 0x30, 0x03 };
static const uint8_t chiptest_12cb0ed34854_ct_1431[]  = { 0x9a, 0x48, 0xb4, 0x6d, 0x76, 0x24, 0xf5, 0xc1, 0x5f, 0x65, 0x45,
                                                         0x91, 0x11, 0x39, 0x53, 0x1f, 0x2b, 0x25, 0xed, 0xcf, 0x6a, 0x9c,
                                                         0xfd, 0x27, 0x9b, 0x16, 0x28, 0xcd, 0xa4, 0x5f, 0x58, 0xd0, 0x3e };
static const uint8_t chiptest_12cb0ed34854_iv_1441[]  = { 0xaf, 0x38, 0xfc, 0xd0, 0x6b, 0x87, 0x80 };
static const uint8_t chiptest_12cb0ed34854_aad_1451[] = {};
static const uint8_t chiptest_12cb0ed34854_tag_1461[] = { 0xab, 0x73, 0x1f, 0xc8, 0x0a, 0xde, 0x38, 0xf1,
                                                          0xa9, 0x84, 0x60, 0x6e, 0xae, 0x05, 0xa0, 0x2b };
static const struct ccm_test_vector chiptest_12cb0ed34854_test_vector_1471 = { .key     = chiptest_12cb0ed34854_key_1411,
                                                                               .key_len = 32,
                                                                               .pt      = chiptest_12cb0ed34854_pt_1421,
                                                                               .pt_len  = 33,
                                                                               .ct      = chiptest_12cb0ed34854_ct_1431,
                                                                               .ct_len  = 33,
                                                                               .iv      = chiptest_12cb0ed34854_iv_1441,
                                                                               .iv_len  = 7,
                                                                               .aad     = chiptest_12cb0ed34854_aad_1451,
                                                                               .aad_len = 0,
                                                                               .tag     = chiptest_12cb0ed34854_tag_1461,
                                                                               .tag_len = 16,
                                                                               .tcId    = 21,
                                                                               .result  = 1 };

static const struct ccm_test_vector * ccm_invalid_test_vectors[] = { &chiptest_12cb0ed34854_test_vector_3781,
                                                                     &chiptest_12cb0ed34854_test_vector_1471 };
