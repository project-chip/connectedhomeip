/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file contains definitions of test certificates used by various unit tests.
 *
 */

#pragma once

#include <credentials/CHIPCert.h>
#include <credentials/CHIPCertificateSet.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace TestCerts {

using namespace chip::Credentials;

enum TestCert
{
    kNone      = 0,
    kRoot01    = 1,
    kRoot02    = 2,
    kICA01     = 3,
    kICA02     = 4,
    kICA01_1   = 5,
    kFWSign01  = 6,
    kNode01_01 = 7,
    kNode01_02 = 8,
    kNode02_01 = 9,
    kNode02_02 = 10,
    kNode02_03 = 11,
    kNode02_04 = 12,
    kNode02_05 = 13,
    kNode02_06 = 14,
    kNode02_07 = 15,
    kNode02_08 = 16,
};

// Special flags to alter how certificates are fetched/loaded.
enum class TestCertLoadFlags : uint8_t
{
    kDERForm               = 0x01,
    kSuppressIsCA          = 0x02,
    kSuppressKeyUsage      = 0x04,
    kSuppressKeyCertSign   = 0x08,
    kSetPathLenConstZero   = 0x10,
    kSetAppDefinedCertType = 0x20,
};

extern CHIP_ERROR GetTestCert(uint8_t certType, BitFlags<TestCertLoadFlags> certLoadFlags, ByteSpan & cert);
extern const char * GetTestCertName(uint8_t certType);
extern CHIP_ERROR GetTestCertPubkey(uint8_t certType, ByteSpan & pubkey);
extern CHIP_ERROR GetTestCertSKID(uint8_t certType, ByteSpan & skid);
extern CHIP_ERROR GetTestCertAKID(uint8_t certType, ByteSpan & akid);
extern CHIP_ERROR LoadTestCert(ChipCertificateSet & certSet, uint8_t certType, BitFlags<TestCertLoadFlags> certLoadFlags,
                               BitFlags<CertDecodeFlags> decodeFlags);

extern const uint8_t gTestCerts[];
extern const size_t gNumTestCerts;

// ------------------------------ DECLARATIONS ----------------------------------------

extern const uint8_t sTestCert_Root01_Chip[];
extern const size_t sTestCert_Root01_Chip_Len;
extern const uint8_t sTestCert_Root01_DER[];
extern const size_t sTestCert_Root01_DER_Len;
extern const uint8_t sTestCert_Root01_PublicKey[];
extern const size_t sTestCert_Root01_PublicKey_Len;
extern const uint8_t sTestCert_Root01_PrivateKey[];
extern const size_t sTestCert_Root01_PrivateKey_Len;
extern const uint8_t sTestCert_Root01_SubjectKeyId[];
extern const size_t sTestCert_Root01_SubjectKeyId_Len;
extern const uint8_t sTestCert_Root01_AuthorityKeyId[];
extern const size_t sTestCert_Root01_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Root02_Chip[];
extern const size_t sTestCert_Root02_Chip_Len;
extern const uint8_t sTestCert_Root02_DER[];
extern const size_t sTestCert_Root02_DER_Len;
extern const uint8_t sTestCert_Root02_PublicKey[];
extern const size_t sTestCert_Root02_PublicKey_Len;
extern const uint8_t sTestCert_Root02_PrivateKey[];
extern const size_t sTestCert_Root02_PrivateKey_Len;
extern const uint8_t sTestCert_Root02_SubjectKeyId[];
extern const size_t sTestCert_Root02_SubjectKeyId_Len;
extern const uint8_t sTestCert_Root02_AuthorityKeyId[];
extern const size_t sTestCert_Root02_AuthorityKeyId_Len;

extern const uint8_t sTestCert_ICA01_Chip[];
extern const size_t sTestCert_ICA01_Chip_Len;
extern const uint8_t sTestCert_ICA01_DER[];
extern const size_t sTestCert_ICA01_DER_Len;
extern const uint8_t sTestCert_ICA01_PublicKey[];
extern const size_t sTestCert_ICA01_PublicKey_Len;
extern const uint8_t sTestCert_ICA01_PrivateKey[];
extern const size_t sTestCert_ICA01_PrivateKey_Len;
extern const uint8_t sTestCert_ICA01_SubjectKeyId[];
extern const size_t sTestCert_ICA01_SubjectKeyId_Len;
extern const uint8_t sTestCert_ICA01_AuthorityKeyId[];
extern const size_t sTestCert_ICA01_AuthorityKeyId_Len;

extern const uint8_t sTestCert_ICA02_Chip[];
extern const size_t sTestCert_ICA02_Chip_Len;
extern const uint8_t sTestCert_ICA02_DER[];
extern const size_t sTestCert_ICA02_DER_Len;
extern const uint8_t sTestCert_ICA02_PublicKey[];
extern const size_t sTestCert_ICA02_PublicKey_Len;
extern const uint8_t sTestCert_ICA02_PrivateKey[];
extern const size_t sTestCert_ICA02_PrivateKey_Len;
extern const uint8_t sTestCert_ICA02_SubjectKeyId[];
extern const size_t sTestCert_ICA02_SubjectKeyId_Len;
extern const uint8_t sTestCert_ICA02_AuthorityKeyId[];
extern const size_t sTestCert_ICA02_AuthorityKeyId_Len;

extern const uint8_t sTestCert_ICA01_1_Chip[];
extern const size_t sTestCert_ICA01_1_Chip_Len;
extern const uint8_t sTestCert_ICA01_1_DER[];
extern const size_t sTestCert_ICA01_1_DER_Len;
extern const uint8_t sTestCert_ICA01_1_PublicKey[];
extern const size_t sTestCert_ICA01_1_PublicKey_Len;
extern const uint8_t sTestCert_ICA01_1_PrivateKey[];
extern const size_t sTestCert_ICA01_1_PrivateKey_Len;
extern const uint8_t sTestCert_ICA01_1_SubjectKeyId[];
extern const size_t sTestCert_ICA01_1_SubjectKeyId_Len;
extern const uint8_t sTestCert_ICA01_1_AuthorityKeyId[];
extern const size_t sTestCert_ICA01_1_AuthorityKeyId_Len;

extern const uint8_t sTestCert_FWSign01_Chip[];
extern const size_t sTestCert_FWSign01_Chip_Len;
extern const uint8_t sTestCert_FWSign01_DER[];
extern const size_t sTestCert_FWSign01_DER_Len;
extern const uint8_t sTestCert_FWSign01_PublicKey[];
extern const size_t sTestCert_FWSign01_PublicKey_Len;
extern const uint8_t sTestCert_FWSign01_PrivateKey[];
extern const size_t sTestCert_FWSign01_PrivateKey_Len;
extern const uint8_t sTestCert_FWSign01_SubjectKeyId[];
extern const size_t sTestCert_FWSign01_SubjectKeyId_Len;
extern const uint8_t sTestCert_FWSign01_AuthorityKeyId[];
extern const size_t sTestCert_FWSign01_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node01_01_Chip[];
extern const size_t sTestCert_Node01_01_Chip_Len;
extern const uint8_t sTestCert_Node01_01_DER[];
extern const size_t sTestCert_Node01_01_DER_Len;
extern const uint8_t sTestCert_Node01_01_PublicKey[];
extern const size_t sTestCert_Node01_01_PublicKey_Len;
extern const uint8_t sTestCert_Node01_01_PrivateKey[];
extern const size_t sTestCert_Node01_01_PrivateKey_Len;
extern const uint8_t sTestCert_Node01_01_SubjectKeyId[];
extern const size_t sTestCert_Node01_01_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node01_01_AuthorityKeyId[];
extern const size_t sTestCert_Node01_01_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node01_01_Err01_Chip[];
extern const size_t sTestCert_Node01_01_Err01_Chip_Len;

extern const uint8_t sTestCert_Node01_02_Chip[];
extern const size_t sTestCert_Node01_02_Chip_Len;
extern const uint8_t sTestCert_Node01_02_DER[];
extern const size_t sTestCert_Node01_02_DER_Len;
extern const uint8_t sTestCert_Node01_02_PublicKey[];
extern const size_t sTestCert_Node01_02_PublicKey_Len;
extern const uint8_t sTestCert_Node01_02_PrivateKey[];
extern const size_t sTestCert_Node01_02_PrivateKey_Len;
extern const uint8_t sTestCert_Node01_02_SubjectKeyId[];
extern const size_t sTestCert_Node01_02_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node01_02_AuthorityKeyId[];
extern const size_t sTestCert_Node01_02_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_01_Chip[];
extern const size_t sTestCert_Node02_01_Chip_Len;
extern const uint8_t sTestCert_Node02_01_DER[];
extern const size_t sTestCert_Node02_01_DER_Len;
extern const uint8_t sTestCert_Node02_01_PublicKey[];
extern const size_t sTestCert_Node02_01_PublicKey_Len;
extern const uint8_t sTestCert_Node02_01_PrivateKey[];
extern const size_t sTestCert_Node02_01_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_01_SubjectKeyId[];
extern const size_t sTestCert_Node02_01_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_01_AuthorityKeyId[];
extern const size_t sTestCert_Node02_01_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_02_Chip[];
extern const size_t sTestCert_Node02_02_Chip_Len;
extern const uint8_t sTestCert_Node02_02_DER[];
extern const size_t sTestCert_Node02_02_DER_Len;
extern const uint8_t sTestCert_Node02_02_PublicKey[];
extern const size_t sTestCert_Node02_02_PublicKey_Len;
extern const uint8_t sTestCert_Node02_02_PrivateKey[];
extern const size_t sTestCert_Node02_02_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_02_SubjectKeyId[];
extern const size_t sTestCert_Node02_02_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_02_AuthorityKeyId[];
extern const size_t sTestCert_Node02_02_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_03_Chip[];
extern const size_t sTestCert_Node02_03_Chip_Len;
extern const uint8_t sTestCert_Node02_03_DER[];
extern const size_t sTestCert_Node02_03_DER_Len;
extern const uint8_t sTestCert_Node02_03_PublicKey[];
extern const size_t sTestCert_Node02_03_PublicKey_Len;
extern const uint8_t sTestCert_Node02_03_PrivateKey[];
extern const size_t sTestCert_Node02_03_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_03_SubjectKeyId[];
extern const size_t sTestCert_Node02_03_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_03_AuthorityKeyId[];
extern const size_t sTestCert_Node02_03_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_04_Chip[];
extern const size_t sTestCert_Node02_04_Chip_Len;
extern const uint8_t sTestCert_Node02_04_DER[];
extern const size_t sTestCert_Node02_04_DER_Len;
extern const uint8_t sTestCert_Node02_04_PublicKey[];
extern const size_t sTestCert_Node02_04_PublicKey_Len;
extern const uint8_t sTestCert_Node02_04_PrivateKey[];
extern const size_t sTestCert_Node02_04_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_04_SubjectKeyId[];
extern const size_t sTestCert_Node02_04_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_04_AuthorityKeyId[];
extern const size_t sTestCert_Node02_04_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_05_Chip[];
extern const size_t sTestCert_Node02_05_Chip_Len;
extern const uint8_t sTestCert_Node02_05_DER[];
extern const size_t sTestCert_Node02_05_DER_Len;
extern const uint8_t sTestCert_Node02_05_PublicKey[];
extern const size_t sTestCert_Node02_05_PublicKey_Len;
extern const uint8_t sTestCert_Node02_05_PrivateKey[];
extern const size_t sTestCert_Node02_05_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_05_SubjectKeyId[];
extern const size_t sTestCert_Node02_05_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_05_AuthorityKeyId[];
extern const size_t sTestCert_Node02_05_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_06_Chip[];
extern const size_t sTestCert_Node02_06_Chip_Len;
extern const uint8_t sTestCert_Node02_06_DER[];
extern const size_t sTestCert_Node02_06_DER_Len;
extern const uint8_t sTestCert_Node02_06_PublicKey[];
extern const size_t sTestCert_Node02_06_PublicKey_Len;
extern const uint8_t sTestCert_Node02_06_PrivateKey[];
extern const size_t sTestCert_Node02_06_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_06_SubjectKeyId[];
extern const size_t sTestCert_Node02_06_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_06_AuthorityKeyId[];
extern const size_t sTestCert_Node02_06_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_07_Chip[];
extern const size_t sTestCert_Node02_07_Chip_Len;
extern const uint8_t sTestCert_Node02_07_DER[];
extern const size_t sTestCert_Node02_07_DER_Len;
extern const uint8_t sTestCert_Node02_07_PublicKey[];
extern const size_t sTestCert_Node02_07_PublicKey_Len;
extern const uint8_t sTestCert_Node02_07_PrivateKey[];
extern const size_t sTestCert_Node02_07_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_07_SubjectKeyId[];
extern const size_t sTestCert_Node02_07_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_07_AuthorityKeyId[];
extern const size_t sTestCert_Node02_07_AuthorityKeyId_Len;

extern const uint8_t sTestCert_Node02_08_Chip[];
extern const size_t sTestCert_Node02_08_Chip_Len;
extern const uint8_t sTestCert_Node02_08_DER[];
extern const size_t sTestCert_Node02_08_DER_Len;
extern const uint8_t sTestCert_Node02_08_PublicKey[];
extern const size_t sTestCert_Node02_08_PublicKey_Len;
extern const uint8_t sTestCert_Node02_08_PrivateKey[];
extern const size_t sTestCert_Node02_08_PrivateKey_Len;
extern const uint8_t sTestCert_Node02_08_SubjectKeyId[];
extern const size_t sTestCert_Node02_08_SubjectKeyId_Len;
extern const uint8_t sTestCert_Node02_08_AuthorityKeyId[];
extern const size_t sTestCert_Node02_08_AuthorityKeyId_Len;

} // namespace TestCerts
} // namespace chip
