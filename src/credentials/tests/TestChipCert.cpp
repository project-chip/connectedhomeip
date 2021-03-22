/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      Unit tests for CHIP certificate functionality.
 *
 */

#include <core/CHIPTLV.h>
#include <credentials/CHIPCert.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

#include "CHIPCert_test_vectors.h"

using namespace chip;
using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Credentials;
using namespace chip::TestCerts;

enum
{
    kStandardCertsCount = 3,
    kTestCertBufSize    = 1024, // Size of buffer needed to hold any of the test certificates
                                // (in either CHIP or DER form), or to decode the certificates.
};

static const BitFlags<CertValidateFlags> sIgnoreNotBeforeFlag(CertValidateFlags::kIgnoreNotBefore);
static const BitFlags<CertValidateFlags> sIgnoreNotAfterFlag(CertValidateFlags::kIgnoreNotAfter);

static const BitFlags<CertDecodeFlags> sNullDecodeFlag;
static const BitFlags<CertDecodeFlags> sGenTBSHashFlag(CertDecodeFlags::kGenerateTBSHash);
static const BitFlags<CertDecodeFlags> sTrustAnchorFlag(CertDecodeFlags::kIsTrustAnchor);

static const BitFlags<TestCertLoadFlags> sNullLoadFlag;
static const BitFlags<TestCertLoadFlags> sDerFormFlag(TestCertLoadFlags::kDERForm);
static const BitFlags<TestCertLoadFlags> sSupIsCAFlag(TestCertLoadFlags::kSuppressIsCA);
static const BitFlags<TestCertLoadFlags> sSupKeyUsageFlag(TestCertLoadFlags::kSuppressKeyUsage);
static const BitFlags<TestCertLoadFlags> sSupKeyCertSignFlag(TestCertLoadFlags::kSuppressKeyCertSign);
static const BitFlags<TestCertLoadFlags> sPathLenZeroFlag(TestCertLoadFlags::kSetPathLenConstZero);
static const BitFlags<TestCertLoadFlags> sAppDefCertTypeFlag(TestCertLoadFlags::kSetAppDefinedCertType);

static const BitFlags<KeyPurposeFlags> sNullKPFlag;
static const BitFlags<KeyPurposeFlags> sSA(KeyPurposeFlags::kServerAuth);
static const BitFlags<KeyPurposeFlags> sCA(KeyPurposeFlags::kClientAuth);
static const BitFlags<KeyPurposeFlags> sCS(KeyPurposeFlags::kCodeSigning);
static const BitFlags<KeyPurposeFlags> sEP(KeyPurposeFlags::kEmailProtection);
static const BitFlags<KeyPurposeFlags> sTS(KeyPurposeFlags::kTimeStamping);
static const BitFlags<KeyPurposeFlags> sOS(KeyPurposeFlags::kOCSPSigning);
static const BitFlags<KeyPurposeFlags> sSAandCA(sSA, sCA);
static const BitFlags<KeyPurposeFlags> sSAandCS(sSA, sCS);
static const BitFlags<KeyPurposeFlags> sSAandEP(sSA, sEP);
static const BitFlags<KeyPurposeFlags> sSAandTS(sSA, sTS);

static const BitFlags<KeyUsageFlags> sNullKUFlag;
static const BitFlags<KeyUsageFlags> sDS(KeyUsageFlags::kDigitalSignature);
static const BitFlags<KeyUsageFlags> sNR(KeyUsageFlags::kNonRepudiation);
static const BitFlags<KeyUsageFlags> sKE(KeyUsageFlags::kKeyEncipherment);
static const BitFlags<KeyUsageFlags> sDE(KeyUsageFlags::kDataEncipherment);
static const BitFlags<KeyUsageFlags> sKA(KeyUsageFlags::kKeyAgreement);
static const BitFlags<KeyUsageFlags> sKC(KeyUsageFlags::kKeyCertSign);
static const BitFlags<KeyUsageFlags> sCR(KeyUsageFlags::kCRLSign);
static const BitFlags<KeyUsageFlags> sEO(KeyUsageFlags::kEncipherOnly);
static const BitFlags<KeyUsageFlags> sDO(KeyUsageFlags::kDecipherOnly);
static const BitFlags<KeyUsageFlags> sDSandNR(sDS, sNR);
static const BitFlags<KeyUsageFlags> sDSandKE(sDS, sKE);
static const BitFlags<KeyUsageFlags> sDSandDE(sDS, sDE);
static const BitFlags<KeyUsageFlags> sDSandKA(sDS, sKA);
static const BitFlags<KeyUsageFlags> sDSandKC(sDS, sKC);
static const BitFlags<KeyUsageFlags> sDSandCR(sDS, sCR);
static const BitFlags<KeyUsageFlags> sDSandEO(sDS, sEO);
static const BitFlags<KeyUsageFlags> sDSandDO(sDS, sDO);
static const BitFlags<KeyUsageFlags> sKCandDS(sKC, sDS);
static const BitFlags<KeyUsageFlags> sKCandNR(sKC, sNR);
static const BitFlags<KeyUsageFlags> sKCandKE(sKC, sKE);
static const BitFlags<KeyUsageFlags> sKCandDE(sKC, sDE);
static const BitFlags<KeyUsageFlags> sKCandKA(sKC, sKA);
static const BitFlags<KeyUsageFlags> sKCandCR(sKC, sCR);
static const BitFlags<KeyUsageFlags> sKCandEO(sKC, sEO);
static const BitFlags<KeyUsageFlags> sKCandDO(sKC, sDO);

static CHIP_ERROR LoadStandardCerts(ChipCertificateSet & certSet)
{
    CHIP_ERROR err;

    err = LoadTestCert(certSet, TestCertTypes::kRoot, sNullLoadFlag, sTrustAnchorFlag);
    SuccessOrExit(err);

    err = LoadTestCert(certSet, TestCertTypes::kNodeCA, sNullLoadFlag, sGenTBSHashFlag);
    SuccessOrExit(err);

    err = LoadTestCert(certSet, TestCertTypes::kNode01, sNullLoadFlag, sGenTBSHashFlag);
    SuccessOrExit(err);

exit:
    return err;
}

static CHIP_ERROR SetEffectiveTime(ValidationContext & validContext, uint16_t year, uint8_t mon, uint8_t day, uint8_t hour = 0,
                                   uint8_t min = 0, uint8_t sec = 0)
{
    ASN1UniversalTime effectiveTime;

    effectiveTime.Year   = year;
    effectiveTime.Month  = mon;
    effectiveTime.Day    = day;
    effectiveTime.Hour   = hour;
    effectiveTime.Minute = min;
    effectiveTime.Second = sec;

    return ASN1ToChipEpochTime(effectiveTime, validContext.mEffectiveTime);
}

static void TestChipCert_ChipToX509(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const uint8_t * inCert;
    uint32_t inCertLen;
    const uint8_t * expectedOutCert;
    uint32_t expectedOutCertLen;
    uint8_t outCertBuf[kTestCertBufSize];
    uint32_t outCertLen;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        uint8_t certType = gTestCerts[i];

        err = GetTestCert(certType, sNullLoadFlag, inCert, inCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = GetTestCert(certType, sDerFormFlag, expectedOutCert, expectedOutCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = ConvertChipCertToX509Cert(inCert, inCertLen, outCertBuf, sizeof(outCertBuf), outCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, outCertLen == expectedOutCertLen);
        NL_TEST_ASSERT(inSuite, memcmp(outCertBuf, expectedOutCert, outCertLen) == 0);
    }
}

static void TestChipCert_X509ToChip(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const uint8_t * inCert;
    uint32_t inCertLen;
    const uint8_t * expectedOutCert;
    uint32_t expectedOutCertLen;
    uint8_t outCertBuf[kTestCertBufSize];
    uint32_t outCertLen;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        uint8_t certType = gTestCerts[i];

        err = GetTestCert(certType, sDerFormFlag, inCert, inCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = GetTestCert(certType, sNullLoadFlag, expectedOutCert, expectedOutCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = ConvertX509CertToChipCert(inCert, inCertLen, outCertBuf, sizeof(outCertBuf), outCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, outCertLen == expectedOutCertLen);
        NL_TEST_ASSERT(inSuite, memcmp(outCertBuf, expectedOutCert, outCertLen) == 0);
    }
}

static void TestChipCert_MultiOID(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    uint8_t outCertBuf[kTestCertBufSize];
    uint32_t outCertLen;
    ChipCertificateData certData;

    ChipCertificateSet certSet;

    uint8_t testCert_Root_Chip[] = {
        0xd5, 0x0, 0x0, 0x8, 0x0, 0x1, 0x0, 0x30, 0x1, 0x14, 0x1b, 0x1c, 0x1c, 0x94, 0x47, 0x19, 0x3e, 0x1e, 0x2f,
        0xe9, 0xa9, 0x3e, 0x38, 0xea, 0x48, 0x97, 0x58, 0x56, 0x19, 0xcf, 0x24, 0x2, 0x1, 0x37, 0x3, 0x27, 0x12, 0x94,
        0xf1, 0x7b, 0xd0, 0xc7, 0xc8, 0x6e, 0x5c, 0x18, 0x26, 0x4, 0x7d, 0x49, 0x34, 0x2, 0x26, 0x5, 0xfd, 0x50, 0x32,
        0x31, 0x37, 0x6, 0x27, 0x12, 0x94, 0xf1, 0x7b, 0xd0, 0xc7, 0xc8, 0x6e, 0x5c, 0x18, 0x24, 0x7, 0x1, 0x24, 0x8,
        0x1, 0x30, 0x9, 0x41, 0x4, 0x42, 0xc6, 0xa7, 0xd1, 0x33, 0xa1, 0xb5, 0x7b, 0x1b, 0x4f, 0x53, 0x23, 0x53, 0x8b,
        0x52, 0x20, 0xfe, 0x8a, 0x1d, 0x3, 0xec, 0x51, 0xdd, 0x8, 0x9d, 0x47, 0xaf, 0xf, 0x24, 0x2e, 0x35, 0x1, 0x7a,
        0xea, 0x9e, 0xae, 0x4e, 0xce, 0x3c, 0x9f, 0x8e, 0x86, 0xcc, 0x37, 0xae, 0x1a, 0xe3, 0xef, 0xaf, 0x1d, 0xaf,
        0x6a, 0x19, 0x9d, 0x29, 0x49, 0x98, 0x8c, 0x5a, 0x95, 0x1b, 0xcf, 0x35, 0x19, 0x35, 0x80, 0x29, 0x1, 0x24, 0x2,
        0x1, 0x18, 0x30, 0x84, 0x14, 0x84, 0x58, 0xb1, 0xba, 0xfc, 0xf2, 0x43, 0x5a, 0x61, 0xa8, 0x7c, 0xfd, 0xf8, 0x6,
        0x10, 0x9c, 0xca, 0xa5, 0xc0, 0xb, 0x30, 0x83, 0x14, 0x84, 0x58, 0xb1, 0xba, 0xfc, 0xf2, 0x43, 0x5a, 0x61, 0xa8,
        0x7c, 0xfd, 0xf8, 0x6, 0x10, 0x9c, 0xca, 0xa5, 0xc0, 0xb, 0x24, 0x81, 0x61, 0x35, 0xa, 0x30, 0x1, 0x20, 0x2c,
        0xb9, 0x56, 0xee, 0x87, 0xd1, 0xd, 0x8d, 0xc1, 0xf1, 0xd7, 0x98, 0x44, 0x94, 0xb8, 0xf5, 0xe1, 0xfa, 0xca, 0xa9,
        0x3b, 0x40, 0x9, 0xad, 0x24, 0xc5, 0x67, 0xb3, 0x99, 0x66, 0x7c, 0x21, 0x30, 0x2, 0x21, 0x0, 0x9a, 0x87, 0x98,
        0xc6, 0x87, 0xe6, 0x0, 0x68, 0xe3, 0x68, 0xbd, 0x2b, 0xa2, 0x68, 0x75, 0xcf, 0x97, 0xf, 0x67, 0x2a, 0xa7, 0xae,
        0x91, 0xb0, 0x9b, 0xb8, 0x62, 0x91, 0xcf, 0xf5, 0x40, 0x8, 0x18, 0x18
    };
    uint32_t testCert_Root_Chip_Len = sizeof(testCert_Root_Chip);

    uint8_t testCert_NodeCA_Chip[] = {
        0xd5, 0x0, 0x0, 0x8, 0x0, 0x1, 0x0, 0x30, 0x1, 0x14, 0x1f, 0x45, 0x4d, 0x70, 0x67, 0xd6, 0xad, 0x64, 0xcd,
        0xdd, 0xbe, 0x9c, 0xd8, 0x7f, 0xad, 0x67, 0x86, 0x86, 0xb1, 0x17, 0x24, 0x2, 0x1, 0x37, 0x3, 0x27, 0x12, 0x94,
        0xf1, 0x7b, 0xd0, 0xc7, 0xc8, 0x6e, 0x5c, 0x18, 0x26, 0x4, 0x1e, 0x4a, 0x34, 0x2, 0x26, 0x5, 0x1e, 0x50, 0xcc,
        0x27, 0x37, 0x6, 0x27, 0x12, 0xf4, 0xe0, 0x36, 0xb5, 0x14, 0x9, 0xa3, 0x3c, 0x18, 0x24, 0x7, 0x1, 0x24, 0x8,
        0x1, 0x30, 0x9, 0x41, 0x4, 0xd4, 0xa1, 0xef, 0x10, 0x76, 0x20, 0xed, 0x83, 0xd, 0xd9, 0x40, 0xd4, 0xa9, 0xe8,
        0xb2, 0xc7, 0x22, 0xae, 0xc5, 0x76, 0x90, 0x42, 0x31, 0x1, 0x1, 0xbf, 0x8, 0xae, 0x62, 0x41, 0x72, 0xcf, 0x2e,
        0x6, 0xed, 0xb5, 0xdd, 0x47, 0xae, 0x7e, 0x9b, 0xc, 0xc9, 0x24, 0x8, 0x53, 0xf5, 0xb1, 0x39, 0x72, 0x3e, 0x1d,
        0x77, 0x46, 0x96, 0xe0, 0xd1, 0x8d, 0x2c, 0x5e, 0x5e, 0x24, 0x7f, 0x9f, 0x35, 0x80, 0x29, 0x1, 0x24, 0x2, 0x0,
        0x18, 0x30, 0x84, 0x14, 0x84, 0x58, 0xb1, 0xba, 0xfc, 0xf2, 0x43, 0x5a, 0x61, 0xa8, 0x7c, 0xfd, 0xf8, 0x6, 0x10,
        0x9c, 0xca, 0xa5, 0xc0, 0xb, 0x30, 0x83, 0x14, 0x4, 0xb, 0xd3, 0x51, 0xfa, 0xdf, 0x8a, 0x7, 0x74, 0xce, 0xcc,
        0xba, 0xaf, 0xeb, 0x56, 0x49, 0x48, 0x48, 0x12, 0x75, 0x24, 0x81, 0x61, 0x35, 0xa, 0x30, 0x1, 0x20, 0x12, 0x54,
        0xaa, 0x9d, 0x23, 0xd1, 0x41, 0x29, 0x30, 0x23, 0xe8, 0x91, 0xe6, 0x8e, 0xbf, 0xd7, 0xa8, 0xf8, 0xd2, 0xf0,
        0x31, 0x2d, 0x1b, 0xa5, 0x55, 0xb, 0xc2, 0x69, 0x83, 0x23, 0xad, 0x7d, 0x30, 0x2, 0x20, 0x25, 0x3f, 0x61, 0x7d,
        0x2e, 0xf9, 0x2d, 0xad, 0xba, 0xf2, 0x5, 0xaa, 0x44, 0x3d, 0x96, 0xed, 0xb1, 0xc3, 0x59, 0x47, 0x57, 0x6d, 0xa0,
        0x44, 0x95, 0x5e, 0x9a, 0x5c, 0xd2, 0x83, 0x3e, 0x52, 0x18, 0x18
    };
    uint32_t testCert_NodeCA_Chip_Len = sizeof(testCert_NodeCA_Chip);

    err = certSet.Init(kStandardCertsCount, kTestCertBufSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = certSet.LoadCert(
            testCert_Root_Chip,
	    testCert_Root_Chip_Len,
            BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)
    );
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = certSet.LoadCert(
            testCert_NodeCA_Chip,
            testCert_NodeCA_Chip_Len,
            BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)
    );
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConvertX509CertToChipCert(sTestCert_2OIDs_Node01_DER, sTestCert_2OIDs_Node01_DER_Len, outCertBuf, sizeof(outCertBuf), outCertLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = certSet.LoadCert(
            outCertBuf,
            outCertLen,
            BitFlags<CertDecodeFlags>(sGenTBSHashFlag)
    );
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = DecodeChipCert(outCertBuf, outCertLen, certData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ValidationContext validContext;
    ChipCertificateData * resultCert    = nullptr;
    const ChipDN & subjectDN = certData.mSubjectDN;
    const CertificateKeyId & subjectKeyId = certData.mSubjectKeyId;

    validContext.Reset();
    err = SetEffectiveTime(validContext, 2021, 3, 26);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    err = certSet.FindValidCert(subjectDN, subjectKeyId, validContext, resultCert);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestChipCert_CertValidation(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ValidationContext validContext;
    enum
    {
        kMaxCertsPerTestCase = 10
    };

    struct ValidationTestCase
    {
        int mSubjectCertIndex;
        uint8_t mValidateFlags;
        uint8_t mRequiredCertType;
        CHIP_ERROR mExpectedResult;
        int mExpectedCertIndex;
        int mExpectedTrustAnchorIndex;
        struct
        {
            uint8_t Type;
            BitFlags<CertDecodeFlags> DecodeFlags;
            BitFlags<TestCertLoadFlags> LoadFlags;
        } InputCerts[kMaxCertsPerTestCase];
    };

    // Short-hand names to make the test cases table more concise.
    enum
    {
        CTNS   = kCertType_NotSpecified,
        CTCA   = kCertType_CA,
        CTNode = kCertType_Node,
        CTFS   = kCertType_FirmwareSigning,
        CTAD   = kCertType_AppDefinedBase,
    };

    // clang-format off
    static const ValidationTestCase sValidationTestCases[] = {
        //            Reqd                                    Exp   Exp                             Cert             Cert
        // Subj Valid Cert                                    Cert  TA     Cert                     Decode           Load
        // Ind  Flags Type    Expected Result                 Index Index  Type                     Flags            Flags
        // ==================================================================================================================================

        // Basic validation of leaf certificate with different load orders.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  1,   0,    CTNS,   CHIP_NO_ERROR,                  1,    0, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       } } },

        // Validation of leaf certificate with root key only.
        {  1,   0,    CTNS,   CHIP_NO_ERROR,                  1,    0, { { TestCertTypes::kRootKey, sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Validation with two copies of root certificate, one trusted, one untrusted.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    1, { { TestCertTypes::kRoot,    sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Validation with trusted root key and trusted root certificate.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCertTypes::kRootKey, sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Validation with trusted root key and untrusted root certificate.
        {  3,   0,    CTNS,   CHIP_NO_ERROR,                  3,    1, { { TestCertTypes::kRoot,    sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kRootKey, sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to missing CA certificate.
        {  1,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to missing root certificate.
        {  1,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to lack of TBS hash.
        {  1,   0,    CTNS,   CHIP_ERROR_INVALID_ARGUMENT,   -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to untrusted root.
        {  1,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to intermediate cert with isCA flag = false
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sSupIsCAFlag        },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to CA cert with no key usage.
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sSupKeyUsageFlag    },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to CA cert with no cert sign key usage.
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sSupKeyCertSignFlag },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to 3-level deep cert chain and root cert with path constraint == 0
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sPathLenZeroFlag    },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Require a specific certificate type.
        {  2,   0,    CTNode, CHIP_NO_ERROR,                  2,    0, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Require a certificate with an application-defined type.
        {  2,   0,    CTAD,   CHIP_NO_ERROR,                  2,    0, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sAppDefCertTypeFlag } } },

        // Select between two identical certificates with different types.
        {  2,   0,    CTAD,   CHIP_NO_ERROR,                  3,    0, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sAppDefCertTypeFlag } } },

        // Failure due to required certificate type not found.
        {  2,   0,    CTCA,   CHIP_ERROR_WRONG_CERT_TYPE,    -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to CA certificate having wrong type.
        {  2,   0,    CTNode, CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sAppDefCertTypeFlag },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to root certificate having wrong type.
        {  2,   0,    CTNode, CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sAppDefCertTypeFlag },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },
    };
    // clang-format on
    static const size_t sNumValidationTestCases = sizeof(sValidationTestCases) / sizeof(sValidationTestCases[0]);

    for (unsigned i = 0; i < sNumValidationTestCases; i++)
    {
        ChipCertificateData * resultCert    = nullptr;
        const ValidationTestCase & testCase = sValidationTestCases[i];

        // Initialize the certificate set and load the specified test certificates.
        certSet.Init(kMaxCertsPerTestCase, kTestCertBufSize);
        for (size_t i2 = 0; i2 < kMaxCertsPerTestCase; i2++)
        {
            if (testCase.InputCerts[i2].Type != TestCertTypes::kNone)
            {
                err = LoadTestCert(certSet, testCase.InputCerts[i2].Type, testCase.InputCerts[i2].LoadFlags,
                                   testCase.InputCerts[i2].DecodeFlags);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            }
        }
        // Make sure the test case is valid.
        NL_TEST_ASSERT(inSuite, testCase.mSubjectCertIndex >= 0 && testCase.mSubjectCertIndex < certSet.GetCertCount());
        if (testCase.mExpectedResult == CHIP_NO_ERROR)
        {
            NL_TEST_ASSERT(inSuite, testCase.mExpectedCertIndex >= 0 && testCase.mExpectedCertIndex < certSet.GetCertCount());
            NL_TEST_ASSERT(inSuite,
                           testCase.mExpectedTrustAnchorIndex >= 0 && testCase.mExpectedTrustAnchorIndex < certSet.GetCertCount());
        }

        // Initialize the validation context.
        validContext.Reset();
        err = SetEffectiveTime(validContext, 2021, 1, 1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
        validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
        validContext.mValidateFlags.SetRaw(testCase.mValidateFlags);
        validContext.mRequiredCertType = testCase.mRequiredCertType;

        // Locate the subject DN and key id that will be used as input the FindValidCert() method.
        const ChipDN & subjectDN              = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectDN;
        const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectKeyId;

        // Invoke the FindValidCert() method (the method being tested).
        err = certSet.FindValidCert(subjectDN, subjectKeyId, validContext, resultCert);
        NL_TEST_ASSERT(inSuite, err == testCase.mExpectedResult);

        // If the test case is expected to be successful...
        if (err == CHIP_NO_ERROR)
        {
            // Verify that the method found the correct certificate.
            NL_TEST_ASSERT(inSuite, resultCert == &certSet.GetCertSet()[testCase.mExpectedCertIndex]);

            // Verify that the method selected the correct trust anchor.
            NL_TEST_ASSERT(inSuite, validContext.mTrustAnchor == &certSet.GetCertSet()[testCase.mExpectedTrustAnchorIndex]);
        }

        // Clear the certificate set.
        certSet.Release();
    }
}

static void TestChipCert_CertValidTime(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ValidationContext validContext;

    certSet.Init(kStandardCertsCount, kTestCertBufSize);

    err = LoadStandardCerts(certSet);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    // Before certificate validity period.
    err = SetEffectiveTime(validContext, 2020, 1, 3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_NOT_VALID_YET);

    // 1 second before validity period.
    err = SetEffectiveTime(validContext, 2020, 10, 15, 14, 23, 42);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_NOT_VALID_YET);

    // 1st second of validity period.
    err = SetEffectiveTime(validContext, 2020, 10, 15, 14, 23, 43);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Validity period.
    err = SetEffectiveTime(validContext, 2022, 02, 23, 12, 30, 01);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Last second of validity period.
    err = SetEffectiveTime(validContext, 2040, 10, 15, 14, 23, 42);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // 1 second after end of certificate validity period.
    err = SetEffectiveTime(validContext, 2040, 10, 15, 14, 23, 43);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_EXPIRED);

    // After end of certificate validity period.
    err = SetEffectiveTime(validContext, 2042, 4, 25, 0, 0, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_EXPIRED);

    // Ignore 'not before' time.
    validContext.mValidateFlags.Set(sIgnoreNotBeforeFlag);
    err = SetEffectiveTime(validContext, 2020, 4, 23, 23, 59, 59);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Ignore 'not after' time.
    validContext.mValidateFlags.Set(sIgnoreNotAfterFlag);
    err = SetEffectiveTime(validContext, 2042, 5, 25, 0, 0, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    certSet.Release();
}

static void TestChipCert_CertUsage(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ValidationContext validContext;

    struct UsageTestCase
    {
        uint8_t mCertIndex;
        BitFlags<KeyUsageFlags> mRequiredKeyUsages;
        BitFlags<KeyPurposeFlags> mRequiredKeyPurposes;
        CHIP_ERROR mExpectedResult;
    };

    // clang-format off
    static UsageTestCase sUsageTestCases[] = {
        // Cert Key
        // Ind  Usages       Key Purposes     Expected Result
        // =========================================================================

        // ----- Key Usages for leaf Certificate -----
        {  2,  sDS,          sNullKPFlag,     CHIP_NO_ERROR                        },
        {  2,  sNR,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sKE,          sNullKPFlag,     CHIP_NO_ERROR                        },
        {  2,  sDE,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sKA,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sKC,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sCR,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sEO,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDO,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandNR,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandKE,     sNullKPFlag,     CHIP_NO_ERROR                        },
        {  2,  sDSandDE,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandKA,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandKC,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandCR,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandEO,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandDO,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Key Usages for CA Certificate -----
        {  1,  sDS,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNR,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKE,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sDE,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKA,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKC,          sNullKPFlag,     CHIP_NO_ERROR                        },
        {  1,  sCR,          sNullKPFlag,     CHIP_NO_ERROR                        },
        {  1,  sEO,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sDO,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKCandDS,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKCandNR,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKCandKE,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKCandDE,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKCandKA,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKCandCR,     sNullKPFlag,     CHIP_NO_ERROR                        },
        {  1,  sKCandEO,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sKCandDO,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Key Purposes for leaf Certificate -----
        {  2,  sNullKUFlag,  sSA,             CHIP_NO_ERROR                        },
        {  2,  sNullKUFlag,  sCA,             CHIP_NO_ERROR                        },
        {  2,  sNullKUFlag,  sCS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sNullKUFlag,  sEP,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sNullKUFlag,  sTS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sNullKUFlag,  sSAandCA,        CHIP_NO_ERROR                        },
        {  2,  sNullKUFlag,  sSAandCS,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sNullKUFlag,  sSAandEP,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sNullKUFlag,  sSAandTS,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Key Purposes for CA Certificate -----
        {  1,  sNullKUFlag,  sSA,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sCA,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sCS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sEP,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sTS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sSAandCA,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sSAandCS,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sSAandEP,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,  sNullKUFlag,  sSAandTS,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Combinations -----
        {  2,  sDSandNR,     sSAandCA,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandKE,     sSAandCS,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandKE,     sSAandCA,        CHIP_NO_ERROR                        },
    };
    // clang-format on
    size_t sNumUsageTestCases = sizeof(sUsageTestCases) / sizeof(sUsageTestCases[0]);

    certSet.Init(kStandardCertsCount, kTestCertBufSize);

    err = LoadStandardCerts(certSet);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    for (size_t i = 0; i < sNumUsageTestCases; i++)
    {
        validContext.Reset();
        validContext.mRequiredKeyUsages   = sUsageTestCases[i].mRequiredKeyUsages;
        validContext.mRequiredKeyPurposes = sUsageTestCases[i].mRequiredKeyPurposes;

        err = SetEffectiveTime(validContext, 2020, 10, 16);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = certSet.ValidateCert(&certSet.GetCertSet()[sUsageTestCases[i].mCertIndex], validContext);
        NL_TEST_ASSERT(inSuite, err == sUsageTestCases[i].mExpectedResult);
    }

    certSet.Release();
}

static void TestChipCert_CertType(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;

    struct TestCase
    {
        uint8_t Cert;
        uint8_t ExpectedCertType;
    };

    // clang-format off
    static TestCase sTestCases[] = {
        // Cert                             ExpectedCertType
        // =============================================================
        {  TestCertTypes::kRoot,            kCertType_CA              },
        {  TestCertTypes::kRootKey,         kCertType_CA              },
        {  TestCertTypes::kNodeCA,          kCertType_CA              },
        {  TestCertTypes::kNode01,          kCertType_Node            },
        {  TestCertTypes::kFirmwareSigning, kCertType_FirmwareSigning },
    };
    // clang-format on
    static const size_t sNumTestCases = sizeof(sTestCases) / sizeof(sTestCases[0]);

    for (unsigned i = 0; i < sNumTestCases; i++)
    {
        const TestCase & testCase = sTestCases[i];

        // Initialize the certificate set and load the test certificate.
        certSet.Init(1, kTestCertBufSize);
        err = LoadTestCert(certSet, testCase.Cert, sNullLoadFlag, sNullDecodeFlag);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, certSet.GetCertSet()->mCertType == testCase.ExpectedCertType);
    }
}

/**
 *  Set up the test suite.
 */
int TestChipCert_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();

    if (error != CHIP_NO_ERROR)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestChipCert_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test CHIP Certificate CHIP to X509 Conversion", TestChipCert_ChipToX509),
    NL_TEST_DEF("Test CHIP Certificate X509 to CHIP Conversion", TestChipCert_X509ToChip),
    NL_TEST_DEF("Test CHIP Certificate Validation", TestChipCert_CertValidation),
    NL_TEST_DEF("Test CHIP Certificate Validation time", TestChipCert_CertValidTime),
    NL_TEST_DEF("Test CHIP Certificate Usage", TestChipCert_CertUsage),
    NL_TEST_DEF("Test CHIP Certificate Type", TestChipCert_CertType),
    NL_TEST_DEF("Test CHIP Certificate Multiple OIDs", TestChipCert_MultiOID),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestChipCert(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Credentials-CHIP-Certs",
        &sTests[0],
        TestChipCert_Setup,
        TestChipCert_Teardown
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestChipCert);
