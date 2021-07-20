/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 * @file - This file contains integer to ASN.1 DER conversion test vector
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

struct RawIntegerToDerVector
{
    const uint8_t * candidate;
    const size_t candidate_size;
    const uint8_t * expected;
    const size_t expected_size;
    const uint8_t * expected_without_tag;
    const size_t expected_without_tag_size;
};

// MSB set, no leading zeros
const uint8_t kRawToDerMsbSetNoLeadingZeroes_Candidate[5] = {
    0x80, 0x01, 0x02, 0x03, 0x04,
};

const uint8_t kRawToDerMsbSetNoLeadingZeroes_Expected[8] = {
    0x02, 0x06, 0x00, 0x80, 0x01, 0x02, 0x03, 0x04,
};

const uint8_t kRawToDerMsbSetNoLeadingZeroes_Expected_WithoutTag[6] = {
    0x00, 0x80, 0x01, 0x02, 0x03, 0x04,
};

const RawIntegerToDerVector kRawIntegerToDerVector1 = {
    .candidate                 = &kRawToDerMsbSetNoLeadingZeroes_Candidate[0],
    .candidate_size            = sizeof(kRawToDerMsbSetNoLeadingZeroes_Candidate),
    .expected                  = &kRawToDerMsbSetNoLeadingZeroes_Expected[0],
    .expected_size             = sizeof(kRawToDerMsbSetNoLeadingZeroes_Expected),
    .expected_without_tag      = &kRawToDerMsbSetNoLeadingZeroes_Expected_WithoutTag[0],
    .expected_without_tag_size = sizeof(kRawToDerMsbSetNoLeadingZeroes_Expected_WithoutTag),
};

// MSB clear, no leading zeros
const uint8_t kRawToDerMsbClearNoLeadingZeroes_Candidate[5] = {
    0x40, 0x01, 0x02, 0x03, 0x04,
};

const uint8_t kRawToDerMsbClearNoLeadingZeroes_Expected[7] = {
    0x02, 0x05, 0x40, 0x01, 0x02, 0x03, 0x04,
};

const uint8_t kRawToDerMsbClearNoLeadingZeroes_Expected_WithoutTag[5] = {
    0x40, 0x01, 0x02, 0x03, 0x04,
};

const RawIntegerToDerVector kRawIntegerToDerVector2 = {
    .candidate                 = &kRawToDerMsbClearNoLeadingZeroes_Candidate[0],
    .candidate_size            = sizeof(kRawToDerMsbClearNoLeadingZeroes_Candidate),
    .expected                  = &kRawToDerMsbClearNoLeadingZeroes_Expected[0],
    .expected_size             = sizeof(kRawToDerMsbClearNoLeadingZeroes_Expected),
    .expected_without_tag      = &kRawToDerMsbClearNoLeadingZeroes_Expected_WithoutTag[0],
    .expected_without_tag_size = sizeof(kRawToDerMsbClearNoLeadingZeroes_Expected_WithoutTag),
};

// Three leading zeroes
const uint8_t kRawToDerThreeLeadingZeroes_Candidate[7] = {
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
};

const uint8_t kRawToDerThreeLeadingZeroes_Expected[6] = {
    0x02, 0x04, 0x01, 0x02, 0x03, 0x04,
};

const uint8_t kRawToDerThreeLeadingZeroes_Expected_WithoutTag[4] = {
    0x01,
    0x02,
    0x03,
    0x04,
};

const RawIntegerToDerVector kRawIntegerToDerVector3 = {
    .candidate                 = &kRawToDerThreeLeadingZeroes_Candidate[0],
    .candidate_size            = sizeof(kRawToDerThreeLeadingZeroes_Candidate),
    .expected                  = &kRawToDerThreeLeadingZeroes_Expected[0],
    .expected_size             = sizeof(kRawToDerThreeLeadingZeroes_Expected),
    .expected_without_tag      = &kRawToDerThreeLeadingZeroes_Expected_WithoutTag[0],
    .expected_without_tag_size = sizeof(kRawToDerThreeLeadingZeroes_Expected_WithoutTag),
};

// Literal zero
const uint8_t kRawToDerLiteralZero_Candidate[1] = {
    0x00,
};

const uint8_t kRawToDerLiteralZero_Expected[3] = {
    0x02,
    0x01,
    0x00,
};

const uint8_t kRawToDerLiteralZero_Expected_WithoutTag[1] = {
    0x00,
};

const RawIntegerToDerVector kRawIntegerToDerVector4 = {
    .candidate                 = &kRawToDerLiteralZero_Candidate[0],
    .candidate_size            = sizeof(kRawToDerLiteralZero_Candidate),
    .expected                  = &kRawToDerLiteralZero_Expected[0],
    .expected_size             = sizeof(kRawToDerLiteralZero_Expected),
    .expected_without_tag      = &kRawToDerLiteralZero_Expected_WithoutTag[0],
    .expected_without_tag_size = sizeof(kRawToDerLiteralZero_Expected_WithoutTag),
};

// Only leading zeroes
const uint8_t kRawToDerOnlyLeadingZeroes_Candidate[4] = {
    0x00,
    0x00,
    0x00,
    0x00,
};

const uint8_t kRawToDerOnlyLeadingZeroes_Expected[3] = {
    0x02,
    0x01,
    0x00,
};

const uint8_t kRawToDerOnlyLeadingZeroes_Expected_WithoutTag[1] = {
    0x00,
};

const RawIntegerToDerVector kRawIntegerToDerVector5 = {
    .candidate                 = &kRawToDerOnlyLeadingZeroes_Candidate[0],
    .candidate_size            = sizeof(kRawToDerOnlyLeadingZeroes_Candidate),
    .expected                  = &kRawToDerOnlyLeadingZeroes_Expected[0],
    .expected_size             = sizeof(kRawToDerOnlyLeadingZeroes_Expected),
    .expected_without_tag      = &kRawToDerOnlyLeadingZeroes_Expected_WithoutTag[0],
    .expected_without_tag_size = sizeof(kRawToDerOnlyLeadingZeroes_Expected_WithoutTag),
};

// Only one byte, non-zero, MSB set
const uint8_t kRawToDerMsbSetOneByte_Candidate[1] = {
    0xff,
};

const uint8_t kRawToDerMsbSetOneByte_Expected[4] = {
    0x02,
    0x02,
    0x00,
    0xff,
};

const uint8_t kRawToDerMsbSetOneByte_Expected_WithoutTag[2] = {
    0x00,
    0xff,
};

const RawIntegerToDerVector kRawIntegerToDerVector6 = {
    .candidate                 = &kRawToDerMsbSetOneByte_Candidate[0],
    .candidate_size            = sizeof(kRawToDerMsbSetOneByte_Candidate),
    .expected                  = &kRawToDerMsbSetOneByte_Expected[0],
    .expected_size             = sizeof(kRawToDerMsbSetOneByte_Expected),
    .expected_without_tag      = &kRawToDerMsbSetOneByte_Expected_WithoutTag[0],
    .expected_without_tag_size = sizeof(kRawToDerMsbSetOneByte_Expected_WithoutTag),
};

// Only one byte, non-zero, MSB clear
const uint8_t kRawToDerMsbClearOneByte_Candidate[1] = {
    0x7f,
};

const uint8_t kRawToDerMsbClearOneByte_Expected[3] = {
    0x02,
    0x01,
    0x7f,
};

const uint8_t kRawToDerMsbClearOneByte_Expected_WithoutTag[1] = {
    0x7f,
};

const RawIntegerToDerVector kRawIntegerToDerVector7 = {
    .candidate                 = &kRawToDerMsbClearOneByte_Candidate[0],
    .candidate_size            = sizeof(kRawToDerMsbClearOneByte_Candidate),
    .expected                  = &kRawToDerMsbClearOneByte_Expected[0],
    .expected_size             = sizeof(kRawToDerMsbClearOneByte_Expected),
    .expected_without_tag      = &kRawToDerMsbClearOneByte_Expected_WithoutTag[0],
    .expected_without_tag_size = sizeof(kRawToDerMsbClearOneByte_Expected_WithoutTag),
};

const RawIntegerToDerVector kRawIntegerToDerVectors[] = { kRawIntegerToDerVector1, kRawIntegerToDerVector2, kRawIntegerToDerVector3,
                                                          kRawIntegerToDerVector4, kRawIntegerToDerVector5, kRawIntegerToDerVector6,
                                                          kRawIntegerToDerVector7 };
