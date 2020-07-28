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
 * @file - This file contains read / write field element test vectors.
 */

#ifndef _SPAKE2P_FE_RW_TEST_VECTORS_H_
#define _SPAKE2P_FE_RW_TEST_VECTORS_H_

namespace chip {
namespace Crypto {

struct spake2p_fe_rw_tv
{
    const unsigned char * fe_in;
    size_t fe_in_len;
    const unsigned char * fe_out;
    size_t fe_out_len;
};

static const unsigned char chiptest_53ea71b7cccd_fe_in_1[]  = {};
static const unsigned char chiptest_53ea71b7cccd_fe_out_2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const struct spake2p_fe_rw_tv chiptest_53ea71b7cccd_test_vector_3 = {
    .fe_in      = chiptest_53ea71b7cccd_fe_in_1,
    .fe_in_len  = 0,
    .fe_out     = chiptest_53ea71b7cccd_fe_out_2,
    .fe_out_len = 32,
};
static const unsigned char chiptest_53ea71b7cccd_fe_in_4[]  = { 0x01 };
static const unsigned char chiptest_53ea71b7cccd_fe_out_5[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
static const struct spake2p_fe_rw_tv chiptest_53ea71b7cccd_test_vector_6 = {
    .fe_in      = chiptest_53ea71b7cccd_fe_in_4,
    .fe_in_len  = 1,
    .fe_out     = chiptest_53ea71b7cccd_fe_out_5,
    .fe_out_len = 32,
};
static const unsigned char chiptest_53ea71b7cccd_fe_in_7[]  = { 0x7f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char chiptest_53ea71b7cccd_fe_out_8[] = { 0x7f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const struct spake2p_fe_rw_tv chiptest_53ea71b7cccd_test_vector_9 = {
    .fe_in      = chiptest_53ea71b7cccd_fe_in_7,
    .fe_in_len  = 32,
    .fe_out     = chiptest_53ea71b7cccd_fe_out_8,
    .fe_out_len = 32,
};
static const unsigned char chiptest_53ea71b7cccd_fe_in_10[]  = { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                                                                0xff, 0xff, 0xff, 0xff, 0xff, 0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17,
                                                                0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x50 };
static const unsigned char chiptest_53ea71b7cccd_fe_out_11[] = { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                                                                 0xff, 0xff, 0xff, 0xff, 0xff, 0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17,
                                                                 0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x50 };
static const struct spake2p_fe_rw_tv chiptest_53ea71b7cccd_test_vector_12 = {
    .fe_in      = chiptest_53ea71b7cccd_fe_in_10,
    .fe_in_len  = 32,
    .fe_out     = chiptest_53ea71b7cccd_fe_out_11,
    .fe_out_len = 32,
};
static const unsigned char chiptest_53ea71b7cccd_fe_in_13[]  = { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                                                                0xff, 0xff, 0xff, 0xff, 0xff, 0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17,
                                                                0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51 };
static const unsigned char chiptest_53ea71b7cccd_fe_out_14[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const struct spake2p_fe_rw_tv chiptest_53ea71b7cccd_test_vector_15 = {
    .fe_in      = chiptest_53ea71b7cccd_fe_in_13,
    .fe_in_len  = 32,
    .fe_out     = chiptest_53ea71b7cccd_fe_out_14,
    .fe_out_len = 32,
};
static const unsigned char chiptest_53ea71b7cccd_fe_in_16[]  = { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                                                                0xff, 0xff, 0xff, 0xff, 0xff, 0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17,
                                                                0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x52 };
static const unsigned char chiptest_53ea71b7cccd_fe_out_17[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
static const struct spake2p_fe_rw_tv chiptest_53ea71b7cccd_test_vector_18 = {
    .fe_in      = chiptest_53ea71b7cccd_fe_in_16,
    .fe_in_len  = 32,
    .fe_out     = chiptest_53ea71b7cccd_fe_out_17,
    .fe_out_len = 32,
};
static const unsigned char chiptest_53ea71b7cccd_fe_in_19[]  = { 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff,
                                                                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x79, 0xcd, 0xf5, 0x5b, 0x4e,
                                                                0x2f, 0x3d, 0x09, 0xe7, 0x73, 0x95, 0x85, 0xf8, 0xc6, 0x4a, 0xa1 };
static const unsigned char chiptest_53ea71b7cccd_fe_out_20[] = { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                                                                 0xff, 0xff, 0xff, 0xff, 0xff, 0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17,
                                                                 0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x50 };
static const struct spake2p_fe_rw_tv chiptest_53ea71b7cccd_test_vector_21 = {
    .fe_in      = chiptest_53ea71b7cccd_fe_in_19,
    .fe_in_len  = 33,
    .fe_out     = chiptest_53ea71b7cccd_fe_out_20,
    .fe_out_len = 32,
};
static const struct spake2p_fe_rw_tv * fe_rw_tvs[] = { &chiptest_53ea71b7cccd_test_vector_3,  &chiptest_53ea71b7cccd_test_vector_6,
                                                       &chiptest_53ea71b7cccd_test_vector_9,  &chiptest_53ea71b7cccd_test_vector_12,
                                                       &chiptest_53ea71b7cccd_test_vector_15, &chiptest_53ea71b7cccd_test_vector_18,
                                                       &chiptest_53ea71b7cccd_test_vector_21 };

} // namespace Crypto
} // namespace chip

#endif
