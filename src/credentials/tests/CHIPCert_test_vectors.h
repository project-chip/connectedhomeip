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

#include <asn1/ASN1OID.h>
#include <core/CHIPConfig.h>
#include <support/CodeUtils.h>

namespace chip {
namespace TestCerts {

using namespace chip::Credentials;
using namespace chip::ASN1;

enum TestCertTypes
{
    kNone              = 0x00,
    kRoot              = 0x01,
    kRootKey           = 0x02,
    kNodeCA            = 0x03,
    kNode01            = 0x04,
    kFirmwareSigningCA = 0x05,
    kFirmwareSigning   = 0x06,
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

extern CHIP_ERROR GetTestCert(uint8_t certType, BitFlags<uint8_t, TestCertLoadFlags> certLoadFlags, const uint8_t *& certData,
                              uint32_t & certDataLen);
extern const char * GetTestCertName(uint8_t certType);
extern CHIP_ERROR LoadTestCert(ChipCertificateSet & certSet, uint8_t certType, BitFlags<uint8_t, TestCertLoadFlags> certLoadFlags,
                               BitFlags<uint8_t, CertDecodeFlags> decodeFlags);

extern const uint8_t gTestCerts[];
extern const size_t gNumTestCerts;

extern const uint8_t sTestCert_Root_Chip[];
extern const uint32_t sTestCert_Root_Chip_Len;
extern const uint8_t sTestCert_Root_DER[];
extern const uint32_t sTestCert_Root_DER_Len;
extern const uint8_t sTestCert_Root_PublicKey[];
extern const uint8_t sTestCert_Root_PublicKey_Len;
extern const uint8_t sTestCert_Root_PrivateKey[];
extern const uint8_t sTestCert_Root_PrivateKey_Len;
extern const uint8_t sTestCert_Root_SubjectKeyId[];
extern const uint8_t sTestCert_Root_SubjectKeyId_Len;
extern const uint64_t sTestCert_Root_Id;

extern const uint8_t sTestCert_NodeCA_Chip[];
extern const uint32_t sTestCert_NodeCA_Chip_Len;
extern const uint8_t sTestCert_NodeCA_DER[];
extern const uint32_t sTestCert_NodeCA_DER_Len;
extern const uint8_t sTestCert_NodeCA_PublicKey[];
extern const uint8_t sTestCert_NodeCA_PublicKey_Len;
extern const uint8_t sTestCert_NodeCA_PrivateKey[];
extern const uint8_t sTestCert_NodeCA_PrivateKey_Len;
extern const uint8_t sTestCert_NodeCA_SubjectKeyId[];
extern const uint8_t sTestCert_NodeCA_SubjectKeyId_Len;
extern const uint64_t sTestCert_NodeCA_Id;

extern const uint8_t sTestCert_Node01_Chip[];
extern const uint32_t sTestCert_Node01_Chip_Len;
extern const uint8_t sTestCert_Node01_DER[];
extern const uint32_t sTestCert_Node01_DER_Len;
extern const uint8_t sTestCert_Node01_PublicKey[];
extern const uint8_t sTestCert_Node01_PublicKey_Len;
extern const uint8_t sTestCert_Node01_PrivateKey[];
extern const uint8_t sTestCert_Node01_PrivateKey_Len;
extern const uint8_t sTestCert_Node01_SubjectKeyId[];
extern const uint8_t sTestCert_Node01_SubjectKeyId_Len;
extern const uint64_t sTestCert_Node01_Id;

extern const uint8_t sTestCert_FirmwareSigningCA_Chip[];
extern const uint32_t sTestCert_FirmwareSigningCA_Chip_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_DER[];
extern const uint32_t sTestCert_FirmwareSigningCA_DER_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_PublicKey[];
extern const uint8_t sTestCert_FirmwareSigningCA_PublicKey_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_PrivateKey[];
extern const uint8_t sTestCert_FirmwareSigningCA_PrivateKey_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_SubjectKeyId[];
extern const uint8_t sTestCert_FirmwareSigningCA_SubjectKeyId_Len;
extern const uint64_t sTestCert_FirmwareSigningCA_Id;

extern const uint8_t sTestCert_FirmwareSigning_Chip[];
extern const uint32_t sTestCert_FirmwareSigning_Chip_Len;
extern const uint8_t sTestCert_FirmwareSigning_DER[];
extern const uint32_t sTestCert_FirmwareSigning_DER_Len;
extern const uint8_t sTestCert_FirmwareSigning_PublicKey[];
extern const uint8_t sTestCert_FirmwareSigning_PublicKey_Len;
extern const uint8_t sTestCert_FirmwareSigning_PrivateKey[];
extern const uint8_t sTestCert_FirmwareSigning_PrivateKey_Len;
extern const uint8_t sTestCert_FirmwareSigning_SubjectKeyId[];
extern const uint8_t sTestCert_FirmwareSigning_SubjectKeyId_Len;
extern const uint64_t sTestCert_FirmwareSigning_Id;

} // namespace TestCerts
} // namespace chip
