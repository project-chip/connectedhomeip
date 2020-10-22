/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

enum
{
    kTestCert_Mask         = 0x000FF000,
    kTestCertLoadFlag_Mask = 0x0FF00000,
    kDecodeFlag_Mask       = 0x0000000F, // values defined in CHIPCert.h

    // Available test cert types.
    kTestCert_None              = 0x00000000,
    kTestCert_Root              = 0x00001000,
    kTestCert_RootKey           = 0x00002000,
    kTestCert_NodeCA            = 0x00003000,
    kTestCert_Node01            = 0x00004000,
    kTestCert_FirmwareSigningCA = 0x00005000,
    kTestCert_FirmwareSigning   = 0x00006000,

    // Special flags to alter how certificates are fetched/loaded.
    kTestCertLoadFlag_DERForm               = 0x00100000,
    kTestCertLoadFlag_SuppressIsCA          = 0x00200000,
    kTestCertLoadFlag_SuppressKeyUsage      = 0x00400000,
    kTestCertLoadFlag_SuppressKeyCertSign   = 0x00800000,
    kTestCertLoadFlag_SetPathLenConstZero   = 0x01000000,
    kTestCertLoadFlag_SetAppDefinedCertType = 0x02000000,

    kTestCertBufSize = 1024, // Size of buffer needed to hold any of the test certificates
                             // (in either CHIP or DER form), or to decode the certificates.
};

extern CHIP_ERROR GetTestCert(int selector, const uint8_t *& certData, size_t & certDataLen);
extern const char * GetTestCertName(int selector);
extern CHIP_ERROR LoadTestCert(ChipCertificateSet & certSet, int selector);

extern const int gTestCerts[];
extern const size_t gNumTestCerts;

extern const uint8_t sTestCert_Root_Chip[];
extern const size_t sTestCert_Root_Chip_Len;
extern const uint8_t sTestCert_Root_DER[];
extern const size_t sTestCert_Root_DER_Len;
extern const uint8_t sTestCert_Root_PublicKey[];
extern const size_t sTestCert_Root_PublicKey_Len;
extern const uint8_t sTestCert_Root_PrivateKey[];
extern const size_t sTestCert_Root_PrivateKey_Len;
extern const uint8_t sTestCert_Root_SubjectKeyId[];
extern const size_t sTestCert_Root_SubjectKeyId_Len;
extern const uint64_t sTestCert_Root_Id;

extern const uint8_t sTestCert_NodeCA_Chip[];
extern const size_t sTestCert_NodeCA_Chip_Len;
extern const uint8_t sTestCert_NodeCA_DER[];
extern const size_t sTestCert_NodeCA_DER_Len;
extern const uint8_t sTestCert_NodeCA_PublicKey[];
extern const size_t sTestCert_NodeCA_PublicKey_Len;
extern const uint8_t sTestCert_NodeCA_PrivateKey[];
extern const size_t sTestCert_NodeCA_PrivateKey_Len;
extern const uint8_t sTestCert_NodeCA_SubjectKeyId[];
extern const size_t sTestCert_NodeCA_SubjectKeyId_Len;
extern const uint64_t sTestCert_NodeCA_Id;

extern const uint8_t sTestCert_Node01_Chip[];
extern const size_t sTestCert_Node01_Chip_Len;
extern const uint8_t sTestCert_Node01_DER[];
extern const size_t sTestCert_Node01_DER_Len;
extern const uint8_t sTestCert_Node01_PublicKey[];
extern const size_t sTestCert_Node01_PublicKey_Len;
extern const uint8_t sTestCert_Node01_PrivateKey[];
extern const size_t sTestCert_Node01_PrivateKey_Len;
extern const uint8_t sTestCert_Node01_SubjectKeyId[];
extern const size_t sTestCert_Node01_SubjectKeyId_Len;
extern const uint64_t sTestCert_Node01_Id;

extern const uint8_t sTestCert_FirmwareSigningCA_Chip[];
extern const size_t sTestCert_FirmwareSigningCA_Chip_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_DER[];
extern const size_t sTestCert_FirmwareSigningCA_DER_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_PublicKey[];
extern const size_t sTestCert_FirmwareSigningCA_PublicKey_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_PrivateKey[];
extern const size_t sTestCert_FirmwareSigningCA_PrivateKey_Len;
extern const uint8_t sTestCert_FirmwareSigningCA_SubjectKeyId[];
extern const size_t sTestCert_FirmwareSigningCA_SubjectKeyId_Len;
extern const uint64_t sTestCert_FirmwareSigningCA_Id;

extern const uint8_t sTestCert_FirmwareSigning_Chip[];
extern const size_t sTestCert_FirmwareSigning_Chip_Len;
extern const uint8_t sTestCert_FirmwareSigning_DER[];
extern const size_t sTestCert_FirmwareSigning_DER_Len;
extern const uint8_t sTestCert_FirmwareSigning_PublicKey[];
extern const size_t sTestCert_FirmwareSigning_PublicKey_Len;
extern const uint8_t sTestCert_FirmwareSigning_PrivateKey[];
extern const size_t sTestCert_FirmwareSigning_PrivateKey_Len;
extern const uint8_t sTestCert_FirmwareSigning_SubjectKeyId[];
extern const size_t sTestCert_FirmwareSigning_SubjectKeyId_Len;
extern const uint64_t sTestCert_FirmwareSigning_Id;

} // namespace TestCerts
} // namespace chip
