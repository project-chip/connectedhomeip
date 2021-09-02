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

#include <credentials/CHIPCert.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/PeerId.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

#include "CHIPCert_test_vectors.h"

using namespace chip;
using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Credentials;
using namespace chip::TestCerts;
using namespace chip::Crypto;

enum
{
    kStandardCertsCount = 3,
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

static CHIP_ERROR LoadTestCertSet01(ChipCertificateSet & certSet)
{
    CHIP_ERROR err;

    err = LoadTestCert(certSet, TestCert::kRoot01, sNullLoadFlag, sTrustAnchorFlag);
    SuccessOrExit(err);

    err = LoadTestCert(certSet, TestCert::kICA01, sNullLoadFlag, sGenTBSHashFlag);
    SuccessOrExit(err);

    err = LoadTestCert(certSet, TestCert::kNode01_01, sNullLoadFlag, sGenTBSHashFlag);
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
    ByteSpan inCert;
    ByteSpan expectedOutCert;
    uint8_t outCertBuf[kMaxDERCertLength];

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        uint8_t certType = gTestCerts[i];

        err = GetTestCert(certType, sNullLoadFlag, inCert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = GetTestCert(certType, sDerFormFlag, expectedOutCert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        MutableByteSpan outCert(outCertBuf);
        err = ConvertChipCertToX509Cert(inCert, outCert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, expectedOutCert.data_equal(outCert));
    }
}

static void TestChipCert_X509ToChip(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ByteSpan inCert;
    ByteSpan expectedOutCert;
    uint8_t outCertBuf[kMaxCHIPCertLength];

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        uint8_t certType = gTestCerts[i];

        err = GetTestCert(certType, sDerFormFlag, inCert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = GetTestCert(certType, sNullLoadFlag, expectedOutCert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        MutableByteSpan outCert(outCertBuf);
        err = ConvertX509CertToChipCert(inCert, outCert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, expectedOutCert.data_equal(outCert));
    }
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
        CTCA   = kCertType_ICA,
        CTNode = kCertType_Node,
        CTFS   = kCertType_FirmwareSigning,
    };

    // clang-format off
    static const ValidationTestCase sValidationTestCases[] = {
        //            Reqd                                    Exp   Exp                             Cert             Cert
        // Subj Valid Cert                                    Cert  TA     Cert                     Decode           Load
        // Ind  Flags Type    Expected Result                 Index Index  Type                     Flags            Flags
        // ==================================================================================================================================

        // Basic validation of node certificate with different load orders.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  1,   0,    CTNS,   CHIP_NO_ERROR,                  1,    0, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       } } },

        // Basic validation of certificate, which is signed by Root.
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    1, { { TestCert::kNode01_02, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode01_02, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    3, { { TestCert::kNode01_02, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       } } },

        // Basic validation of node certificates chaining up to another root.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode02_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  1,   0,    CTNS,   CHIP_NO_ERROR,                  1,    0, { { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kNode02_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode02_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode02_02, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode02_03, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode02_04, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode02_05, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       } } },
        {  0,   0,    CTNS,   CHIP_NO_ERROR,                  0,    2, { { TestCert::kNode02_06, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       } } },

        // Failure due to presence of "critical" future-extension in the kNode02_07 node certificate.
        {  0,   0,    CTNS,   CHIP_ERROR_CERT_USAGE_NOT_ALLOWED, 0, 2, { { TestCert::kNode02_07, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       } } },

        // Validation with two copies of root certificate, one trusted, one untrusted.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    1, { { TestCert::kRoot01,    sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Validation with two trusted certificates.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    1, { { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to missing CA certificate.
        {  1,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA02,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to missing root certificate.
        {  1,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  1,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot02,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to lack of TBS hash.
        {  1,   0,    CTNS,   CHIP_ERROR_INVALID_ARGUMENT,   -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to untrusted root.
        {  1,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot01,    sNullDecodeFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to intermediate cert with isCA flag = false
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sSupIsCAFlag        },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to CA cert with no key usage.
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sSupKeyUsageFlag    },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to CA cert with no cert sign key usage.
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sSupKeyCertSignFlag },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to 3-level deep cert chain and root cert with path constraint == 0
        {  2,   0,    CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,  -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sPathLenZeroFlag    },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Require a specific certificate type.
        {  2,   0,    CTNode, CHIP_NO_ERROR,                  2,    0, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },

        // Failure due to required certificate type not found.
        {  2,   0,    CTCA,   CHIP_ERROR_WRONG_CERT_TYPE,    -1,   -1, { { TestCert::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCert::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCert::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
    };
    // clang-format on
    static const size_t sNumValidationTestCases = ArraySize(sValidationTestCases);

    for (unsigned i = 0; i < sNumValidationTestCases; i++)
    {
        const ChipCertificateData * resultCert = nullptr;
        const ValidationTestCase & testCase    = sValidationTestCases[i];

        err = certSet.Init(kMaxCertsPerTestCase);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        for (size_t i2 = 0; i2 < kMaxCertsPerTestCase; i2++)
        {
            if (testCase.InputCerts[i2].Type != TestCert::kNone)
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
        validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);
        validContext.mValidateFlags.SetRaw(testCase.mValidateFlags);
        validContext.mRequiredCertType = testCase.mRequiredCertType;

        // Locate the subject DN and key id that will be used as input the FindValidCert() method.
        const ChipDN & subjectDN              = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectDN;
        const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectKeyId;

        // Invoke the FindValidCert() method (the method being tested).
        err = certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert);
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

    err = certSet.Init(kStandardCertsCount);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = LoadTestCertSet01(certSet);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    // Before certificate validity period.
    err = SetEffectiveTime(validContext, 2020, 1, 3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // TODO - enable check for certificate validity dates
    // err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    // NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_NOT_VALID_YET);

    // 1 second before validity period.
    err = SetEffectiveTime(validContext, 2020, 10, 15, 14, 23, 42);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // TODO - enable check for certificate validity dates
    // err = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    // NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_NOT_VALID_YET);

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
    ChipCertificateData certDataArray[kStandardCertsCount];

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
        {  2,  sKE,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDE,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sKA,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sKC,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sCR,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sEO,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDO,          sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandNR,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandKE,     sNullKPFlag,     CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
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
        {  2,  sDS,          sSAandCS,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDSandKE,     sSAandCA,        CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,  sDS,          sSAandCA,        CHIP_NO_ERROR                        },
    };
    // clang-format on
    size_t sNumUsageTestCases = ArraySize(sUsageTestCases);

    err = certSet.Init(certDataArray, ArraySize(certDataArray));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = LoadTestCertSet01(certSet);
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
        // Cert                        ExpectedCertType
        // =============================================================
        {  TestCert::kRoot01,          kCertType_Root            },
        {  TestCert::kRoot02,          kCertType_Root            },
        {  TestCert::kICA01,           kCertType_ICA             },
        {  TestCert::kICA02,           kCertType_ICA             },
        {  TestCert::kICA01_1,         kCertType_ICA             },
        {  TestCert::kFWSign01,        kCertType_FirmwareSigning },
        {  TestCert::kNode01_01,       kCertType_Node            },
        {  TestCert::kNode01_02,       kCertType_Node            },
        {  TestCert::kNode02_01,       kCertType_Node            },
        {  TestCert::kNode02_02,       kCertType_Node            },
    };
    // clang-format on
    static const size_t sNumTestCases = ArraySize(sTestCases);

    for (unsigned i = 0; i < sNumTestCases; i++)
    {
        const TestCase & testCase = sTestCases[i];
        uint8_t certType;

        err = certSet.Init(1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = LoadTestCert(certSet, testCase.Cert, sNullLoadFlag, sNullDecodeFlag);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = certSet.GetCertSet()->mSubjectDN.GetCertType(certType);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, certType == testCase.ExpectedCertType);
        certSet.Release();
    }
}

static void TestChipCert_CertId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ChipCertificateData certData[1];

    struct TestCase
    {
        uint8_t Cert;
        uint64_t ExpectedCertId;
    };

    // clang-format off
    static TestCase sTestCases[] = {
        // Cert                        ExpectedCertId
        // =============================================================
        {  TestCert::kRoot01,          0xCACACACA00000001 },
        {  TestCert::kRoot02,          0xCACACACA00000002 },
        {  TestCert::kICA01,           0xCACACACA00000003 },
        {  TestCert::kICA02,           0xCACACACA00000004 },
        {  TestCert::kICA01_1,         0xCACACACA00000005 },
        {  TestCert::kFWSign01,        0xFFFFFFFF00000001 },
        {  TestCert::kNode01_01,       0xDEDEDEDE00010001 },
        {  TestCert::kNode01_02,       0xDEDEDEDE00010002 },
        {  TestCert::kNode02_01,       0xDEDEDEDE00020001 },
        {  TestCert::kNode02_02,       0xDEDEDEDE00020002 },
    };
    // clang-format on
    static const size_t sNumTestCases = ArraySize(sTestCases);

    for (unsigned i = 0; i < sNumTestCases; i++)
    {
        const TestCase & testCase = sTestCases[i];
        uint64_t chipId;

        err = certSet.Init(certData, 1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = LoadTestCert(certSet, testCase.Cert, sNullLoadFlag, sNullDecodeFlag);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = certSet.GetCertSet()->mSubjectDN.GetCertChipId(chipId);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, chipId == testCase.ExpectedCertId);
        certSet.Release();
    }
}

static void TestChipCert_LoadDuplicateCerts(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ValidationContext validContext;

    err = certSet.Init(kStandardCertsCount);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Let's load two distinct certificates, and make sure cert count is 2
    err = LoadTestCert(certSet, TestCert::kRoot01, sNullLoadFlag, sTrustAnchorFlag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = LoadTestCert(certSet, TestCert::kICA01, sNullLoadFlag, sGenTBSHashFlag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.GetCertCount() == 2);

    // Let's load a previously loaded cert and make sure cert count is still 2
    err = LoadTestCert(certSet, TestCert::kRoot01, sNullLoadFlag, sTrustAnchorFlag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.GetCertCount() == 2);

    // Let's load the other previously loaded cert and make sure cert count is still 2
    err = LoadTestCert(certSet, TestCert::kICA01, sNullLoadFlag, sGenTBSHashFlag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.GetCertCount() == 2);

    // Let's load a new cert and make sure cert count updates to 3
    err = LoadTestCert(certSet, TestCert::kNode01_01, sNullLoadFlag, sGenTBSHashFlag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.GetCertCount() == 3);
}

static void TestChipCert_GenerateRootCert(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    ChipCertificateData certData;

    X509CertRequestParams root_params = { 1234, 0xabcdabcd, 631161876, 729942000, false, 0, false, 0 };
    MutableByteSpan signed_cert_span(signed_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, signed_cert_span) == CHIP_NO_ERROR);

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(signed_cert_span, outCert) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, DecodeChipCert(outCert, certData) == CHIP_NO_ERROR);

    // Test that root cert cannot be provided a node ID
    root_params.HasNodeID = true;
    MutableByteSpan signed_cert_span1(signed_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, signed_cert_span1) == CHIP_ERROR_INVALID_ARGUMENT);

    // Test that serial number cannot be negative
    root_params.HasNodeID    = false;
    root_params.SerialNumber = -1;
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, signed_cert_span1) == CHIP_ERROR_INVALID_ARGUMENT);
}

static void TestChipCert_GenerateRootFabCert(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    ChipCertificateData certData;

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    X509CertRequestParams root_params_fabric = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0xabcd, false, 0 };

    MutableByteSpan signed_cert_span(signed_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params_fabric, keypair, signed_cert_span) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(signed_cert_span, outCert) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, DecodeChipCert(outCert, certData) == CHIP_NO_ERROR);
}

static void TestChipCert_GenerateICACert(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    ChipCertificateData certData;

    X509CertRequestParams ica_params = { 1234, 0xabcdabcd, 631161876, 729942000, false, 0, false, 0 };
    P256Keypair ica_keypair;
    NL_TEST_ASSERT(inSuite, ica_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan signed_cert_span(signed_cert);
    NL_TEST_ASSERT(inSuite, NewICAX509Cert(ica_params, 4321, ica_keypair.Pubkey(), keypair, signed_cert_span) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(signed_cert_span, outCert) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, DecodeChipCert(outCert, certData) == CHIP_NO_ERROR);

    // Test that ICA cert cannot be provided a node ID
    ica_params.HasNodeID = true;
    MutableByteSpan signed_cert_span1(signed_cert);
    NL_TEST_ASSERT(
        inSuite, NewICAX509Cert(ica_params, 4321, ica_keypair.Pubkey(), keypair, signed_cert_span1) == CHIP_ERROR_INVALID_ARGUMENT);

    // Test that serial number cannot be negative
    ica_params.HasNodeID    = false;
    ica_params.SerialNumber = -1;
    NL_TEST_ASSERT(
        inSuite, NewICAX509Cert(ica_params, 4321, ica_keypair.Pubkey(), keypair, signed_cert_span1) == CHIP_ERROR_INVALID_ARGUMENT);
}

static void TestChipCert_GenerateNOCRoot(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    ChipCertificateData certData;

    X509CertRequestParams noc_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan signed_cert_span(signed_cert, sizeof(signed_cert));
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsRootCA, noc_keypair.Pubkey(), keypair, signed_cert_span) ==
                       CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(signed_cert_span, outCert) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, DecodeChipCert(outCert, certData) == CHIP_NO_ERROR);

    // Test that NOC cert must be provided a node ID
    noc_params.HasNodeID = false;
    MutableByteSpan signed_cert_span1(signed_cert, sizeof(signed_cert));
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsRootCA, noc_keypair.Pubkey(), keypair, signed_cert_span1) ==
                       CHIP_ERROR_INVALID_ARGUMENT);

    // Test that NOC cert must be provided a fabric ID
    noc_params.HasNodeID   = true;
    noc_params.HasFabricID = false;
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsRootCA, noc_keypair.Pubkey(), keypair, signed_cert_span1) ==
                       CHIP_ERROR_INVALID_ARGUMENT);

    // Test that serial number cannot be negative
    noc_params.HasNodeID    = true;
    noc_params.HasFabricID  = true;
    noc_params.SerialNumber = -1;
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsRootCA, noc_keypair.Pubkey(), keypair, signed_cert_span1) ==
                       CHIP_ERROR_INVALID_ARGUMENT);
}

static void TestChipCert_GenerateNOCICA(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    ChipCertificateData certData;

    X509CertRequestParams noc_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan signed_cert_span(signed_cert);
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsIntermediateCA, noc_keypair.Pubkey(), keypair,
                                              signed_cert_span) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(signed_cert_span, outCert) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, DecodeChipCert(outCert, certData) == CHIP_NO_ERROR);
}

static void TestChipCert_VerifyGeneratedCerts(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    X509CertRequestParams root_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    MutableByteSpan root_cert_span(root_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, root_cert_span) == CHIP_NO_ERROR);

    static uint8_t ica_cert[kMaxDERCertLength];

    X509CertRequestParams ica_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    P256Keypair ica_keypair;
    NL_TEST_ASSERT(inSuite, ica_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan ica_cert_span(ica_cert);
    NL_TEST_ASSERT(inSuite, NewICAX509Cert(ica_params, 0xaabbccdd, ica_keypair.Pubkey(), keypair, ica_cert_span) == CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    X509CertRequestParams noc_params = { 1234, 0xaabbccdd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert, sizeof(noc_cert));
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsIntermediateCA, noc_keypair.Pubkey(), ica_keypair,
                                              noc_cert_span) == CHIP_NO_ERROR);

    ChipCertificateSet certSet;
    NL_TEST_ASSERT(inSuite, certSet.Init(3) == CHIP_NO_ERROR);

    static uint8_t chipRootCertBuf[kMaxCHIPCertLength];
    static uint8_t chipICACertBuf[kMaxCHIPCertLength];
    static uint8_t chipNOCCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipRootCert(chipRootCertBuf);
    MutableByteSpan chipICACert(chipICACertBuf);
    MutableByteSpan chipNOCCert(chipNOCCertBuf);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(root_cert_span, chipRootCert) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.LoadCert(chipRootCert, sTrustAnchorFlag) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(ica_cert_span, chipICACert) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.LoadCert(chipICACert, sGenTBSHashFlag) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(noc_cert_span, chipNOCCert) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.LoadCert(chipNOCCert, sGenTBSHashFlag) == CHIP_NO_ERROR);

    ValidationContext validContext;

    validContext.Reset();
    NL_TEST_ASSERT(inSuite, SetEffectiveTime(validContext, 2022, 1, 1) == CHIP_NO_ERROR);
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    const ChipDN & subjectDN              = certSet.GetCertSet()[2].mSubjectDN;
    const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[2].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    NL_TEST_ASSERT(inSuite, certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert) == CHIP_NO_ERROR);
}

static void TestChipCert_X509ToChipArray(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    X509CertRequestParams root_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    MutableByteSpan root_cert_span(root_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, root_cert_span) == CHIP_NO_ERROR);

    static uint8_t ica_cert[kMaxDERCertLength];

    X509CertRequestParams ica_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    P256Keypair ica_keypair;
    NL_TEST_ASSERT(inSuite, ica_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan ica_cert_span(ica_cert);
    NL_TEST_ASSERT(inSuite, NewICAX509Cert(ica_params, 0xaabbccdd, ica_keypair.Pubkey(), keypair, ica_cert_span) == CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    X509CertRequestParams noc_params = { 1234, 0xaabbccdd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert);
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsIntermediateCA, noc_keypair.Pubkey(), ica_keypair,
                                              noc_cert_span) == CHIP_NO_ERROR);

    static uint8_t chipCertArrayBuf[kMaxCHIPCertLength * 2];
    static uint8_t chipRootCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipCertArray(chipCertArrayBuf);
    MutableByteSpan chipRootCert(chipRootCertBuf);

    NL_TEST_ASSERT(inSuite, ConvertX509CertsToChipCertArray(noc_cert_span, ica_cert_span, chipCertArray) == CHIP_NO_ERROR);

    ChipCertificateSet certSet;
    NL_TEST_ASSERT(inSuite, certSet.Init(3) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certSet.LoadCerts(chipCertArray, sGenTBSHashFlag) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(root_cert_span, chipRootCert) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.LoadCert(chipRootCert, sTrustAnchorFlag) == CHIP_NO_ERROR);

    ValidationContext validContext;

    validContext.Reset();
    NL_TEST_ASSERT(inSuite, SetEffectiveTime(validContext, 2022, 1, 1) == CHIP_NO_ERROR);
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    const ChipDN & subjectDN              = certSet.GetCertSet()[0].mSubjectDN;
    const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[0].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    NL_TEST_ASSERT(inSuite, certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert) == CHIP_NO_ERROR);
}

static void TestChipCert_X509ToChipArrayNoICA(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    X509CertRequestParams root_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    MutableByteSpan root_cert_span(root_cert, sizeof(root_cert));
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, root_cert_span) == CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    X509CertRequestParams noc_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert, sizeof(noc_cert));
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsRootCA, noc_keypair.Pubkey(), keypair, noc_cert_span) ==
                       CHIP_NO_ERROR);

    static uint8_t chipCertArrayBuf[kMaxCHIPCertLength];
    static uint8_t chipRootCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipCertArray(chipCertArrayBuf);
    MutableByteSpan chipRootCert(chipRootCertBuf);

    NL_TEST_ASSERT(inSuite, ConvertX509CertsToChipCertArray(noc_cert_span, ByteSpan(nullptr, 0), chipCertArray) == CHIP_NO_ERROR);

    ChipCertificateSet certSet;
    NL_TEST_ASSERT(inSuite, certSet.Init(2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certSet.LoadCerts(chipCertArray, sGenTBSHashFlag) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(root_cert_span, chipRootCert) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.LoadCert(chipRootCert, sTrustAnchorFlag) == CHIP_NO_ERROR);

    ValidationContext validContext;

    validContext.Reset();
    NL_TEST_ASSERT(inSuite, SetEffectiveTime(validContext, 2022, 1, 1) == CHIP_NO_ERROR);
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    const ChipDN & subjectDN              = certSet.GetCertSet()[0].mSubjectDN;
    const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[0].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    NL_TEST_ASSERT(inSuite, certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert) == CHIP_NO_ERROR);
}

static void TestChipCert_X509ToChipArrayErrorScenarios(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    X509CertRequestParams root_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    MutableByteSpan root_cert_span(root_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, root_cert_span) == CHIP_NO_ERROR);

    static uint8_t ica_cert[kMaxDERCertLength];

    X509CertRequestParams ica_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    P256Keypair ica_keypair;
    NL_TEST_ASSERT(inSuite, ica_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan ica_cert_span(ica_cert, sizeof(ica_cert));
    NL_TEST_ASSERT(inSuite, NewICAX509Cert(ica_params, 0xaabbccdd, ica_keypair.Pubkey(), keypair, ica_cert_span) == CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    X509CertRequestParams noc_params = { 1234, 0xaabbccdd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert, sizeof(noc_cert));
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsIntermediateCA, noc_keypair.Pubkey(), ica_keypair,
                                              noc_cert_span) == CHIP_NO_ERROR);

    static uint8_t chipCertArrayBuf[kMaxCHIPCertLength * 2];
    MutableByteSpan chipCertArray(chipCertArrayBuf);
    // Test that NOC is mandatory
    NL_TEST_ASSERT(inSuite,
                   ConvertX509CertsToChipCertArray(ByteSpan(), ica_cert_span, chipCertArray) == CHIP_ERROR_INVALID_ARGUMENT);

    // Test that NOC issuer must match ICA
    NL_TEST_ASSERT(inSuite,
                   ConvertX509CertsToChipCertArray(noc_cert_span, root_cert_span, chipCertArray) == CHIP_ERROR_INVALID_ARGUMENT);

    X509CertRequestParams ica_params_wrong_fabric = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x9999, false, 0 };

    MutableByteSpan ica_cert_span1(ica_cert, sizeof(ica_cert));
    NL_TEST_ASSERT(inSuite,
                   NewICAX509Cert(ica_params_wrong_fabric, 0xaabbccdd, ica_keypair.Pubkey(), keypair, ica_cert_span1) ==
                       CHIP_NO_ERROR);
    // Test that NOC fabric must match ICA fabric
    NL_TEST_ASSERT(inSuite,
                   ConvertX509CertsToChipCertArray(noc_cert_span, ica_cert_span, chipCertArray) == CHIP_ERROR_INVALID_ARGUMENT);
}

static void TestChipCert_ChipArrayToChipCerts(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    X509CertRequestParams root_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    MutableByteSpan root_cert_span(root_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, root_cert_span) == CHIP_NO_ERROR);

    static uint8_t ica_cert[kMaxDERCertLength];

    X509CertRequestParams ica_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    P256Keypair ica_keypair;
    NL_TEST_ASSERT(inSuite, ica_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan ica_cert_span(ica_cert);
    NL_TEST_ASSERT(inSuite, NewICAX509Cert(ica_params, 0xaabbccdd, ica_keypair.Pubkey(), keypair, ica_cert_span) == CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    X509CertRequestParams noc_params = { 1234, 0xaabbccdd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert);
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsIntermediateCA, noc_keypair.Pubkey(), ica_keypair,
                                              noc_cert_span) == CHIP_NO_ERROR);

    static uint8_t chipCertArrayBuf[kMaxCHIPCertLength * 2];
    static uint8_t chipRootCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipCertArray(chipCertArrayBuf);
    MutableByteSpan chipRootCert(chipRootCertBuf);
    NL_TEST_ASSERT(inSuite, ConvertX509CertsToChipCertArray(noc_cert_span, ica_cert_span, chipCertArray) == CHIP_NO_ERROR);

    ByteSpan noc_chip_cert;
    ByteSpan ica_chip_cert;
    NL_TEST_ASSERT(inSuite, ExtractCertsFromCertArray(chipCertArray, noc_chip_cert, ica_chip_cert) == CHIP_NO_ERROR);

    ChipCertificateSet certSet;
    NL_TEST_ASSERT(inSuite, certSet.Init(3) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certSet.LoadCert(noc_chip_cert, sGenTBSHashFlag) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certSet.LoadCert(ica_chip_cert, sGenTBSHashFlag) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(root_cert_span, chipRootCert) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.LoadCert(chipRootCert, sTrustAnchorFlag) == CHIP_NO_ERROR);

    ValidationContext validContext;

    validContext.Reset();
    NL_TEST_ASSERT(inSuite, SetEffectiveTime(validContext, 2022, 1, 1) == CHIP_NO_ERROR);
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    const ChipDN & subjectDN              = certSet.GetCertSet()[0].mSubjectDN;
    const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[0].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    NL_TEST_ASSERT(inSuite, certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert) == CHIP_NO_ERROR);
}

static void TestChipCert_ChipArrayToChipCertsNoICA(nlTestSuite * inSuite, void * inContext)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    X509CertRequestParams root_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, false, 0 };
    MutableByteSpan root_cert_span(root_cert);
    NL_TEST_ASSERT(inSuite, NewRootX509Cert(root_params, keypair, root_cert_span) == CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    X509CertRequestParams noc_params = { 1234, 0xabcdabcd, 631161876, 729942000, true, 0x8888, true, 0x1234 };
    P256Keypair noc_keypair;
    NL_TEST_ASSERT(inSuite, noc_keypair.Initialize() == CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert);
    NL_TEST_ASSERT(inSuite,
                   NewNodeOperationalX509Cert(noc_params, kIssuerIsRootCA, noc_keypair.Pubkey(), keypair, noc_cert_span) ==
                       CHIP_NO_ERROR);

    static uint8_t chipCertArrayBuf[kMaxCHIPCertLength];
    static uint8_t chipRootCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipCertArray(chipCertArrayBuf);
    MutableByteSpan chipRootCert(chipRootCertBuf);
    NL_TEST_ASSERT(inSuite, ConvertX509CertsToChipCertArray(noc_cert_span, ByteSpan(), chipCertArray) == CHIP_NO_ERROR);

    ByteSpan noc_chip_cert;
    ByteSpan ica_chip_cert;
    NL_TEST_ASSERT(inSuite, ExtractCertsFromCertArray(chipCertArray, noc_chip_cert, ica_chip_cert) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ica_chip_cert.data() == nullptr && ica_chip_cert.size() == 0);

    ChipCertificateSet certSet;
    NL_TEST_ASSERT(inSuite, certSet.Init(2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certSet.LoadCert(noc_chip_cert, sGenTBSHashFlag) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, ConvertX509CertToChipCert(root_cert_span, chipRootCert) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, certSet.LoadCert(chipRootCert, sTrustAnchorFlag) == CHIP_NO_ERROR);

    ValidationContext validContext;

    validContext.Reset();
    NL_TEST_ASSERT(inSuite, SetEffectiveTime(validContext, 2022, 1, 1) == CHIP_NO_ERROR);
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    const ChipDN & subjectDN              = certSet.GetCertSet()[0].mSubjectDN;
    const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[0].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    NL_TEST_ASSERT(inSuite, certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert) == CHIP_NO_ERROR);
}

static void TestChipCert_ExtractPeerId(nlTestSuite * inSuite, void * inContext)
{
    struct TestCase
    {
        uint8_t Cert;
        uint8_t ICACert;
        uint64_t ExpectedNodeId;
        uint64_t ExpectedFabricId;
    };

    // clang-format off
    static constexpr TestCase sTestCases[] = {
        // Cert                  ICA               ExpectedNodeId      ExpectedFabricId
        // =============================================================
        {  TestCert::kNode01_01, TestCert::kICA01, 0xDEDEDEDE00010001, 0xFAB000000000001D },
        {  TestCert::kNode01_02, TestCert::kNone,  0xDEDEDEDE00010002, 0xFAB000000000001D },
        {  TestCert::kNode02_01, TestCert::kICA02, 0xDEDEDEDE00020001, 0xFAB000000000001D },
        {  TestCert::kNode02_02, TestCert::kICA02, 0xDEDEDEDE00020002, 0xFAB000000000001D },
        {  TestCert::kNode02_03, TestCert::kICA02, 0xDEDEDEDE00020003, 0xFAB000000000001D },
        {  TestCert::kNode02_04, TestCert::kICA02, 0xDEDEDEDE00020004, 0xFAB000000000001D },
        {  TestCert::kNode02_05, TestCert::kICA02, 0xDEDEDEDE00020005, 0xFAB000000000001D },
        {  TestCert::kNode02_06, TestCert::kICA02, 0xDEDEDEDE00020006, 0xFAB000000000001D },
        {  TestCert::kNode02_07, TestCert::kICA02, 0xDEDEDEDE00020007, 0xFAB000000000001D },
    };
    // clang-format on

    // Test extraction from the raw ByteSpan form.
    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, sNullLoadFlag, cert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NodeId nodeId;
        FabricId fabricId;
        err = ExtractNodeIdFabricIdFromOpCert(cert, &nodeId, &fabricId);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, nodeId == testCase.ExpectedNodeId);
        NL_TEST_ASSERT(inSuite, fabricId == testCase.ExpectedFabricId);
    }

    // Test extraction from the parsed form.
    ChipCertificateSet certSet;
    for (auto & testCase : sTestCases)
    {
        CHIP_ERROR err = certSet.Init(1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = LoadTestCert(certSet, testCase.Cert, sNullLoadFlag, sNullDecodeFlag);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NodeId nodeId;
        FabricId fabricId;
        err = ExtractNodeIdFabricIdFromOpCert(certSet.GetCertSet()[0], &nodeId, &fabricId);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, nodeId == testCase.ExpectedNodeId);
        NL_TEST_ASSERT(inSuite, fabricId == testCase.ExpectedFabricId);
        certSet.Release();
    }

    // Test extraction from cert array form.
    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, sDerFormFlag, cert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        ByteSpan icaCert;
        if (testCase.ICACert != TestCert::kNone)
        {
            err = GetTestCert(testCase.ICACert, sDerFormFlag, icaCert);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        uint8_t certArray[kMaxCHIPCertLength * 2];
        MutableByteSpan certs(certArray);
        err = ConvertX509CertsToChipCertArray(cert, icaCert, certs);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NodeId nodeId;
        FabricId fabricId;
        err = ExtractNodeIdFabricIdFromOpCertArray(certs, &nodeId, &fabricId);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, nodeId == testCase.ExpectedNodeId);
        NL_TEST_ASSERT(inSuite, fabricId == testCase.ExpectedFabricId);
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
    NL_TEST_DEF("Test CHIP Certificate ID", TestChipCert_CertId),
    NL_TEST_DEF("Test Loading Duplicate Certificates", TestChipCert_LoadDuplicateCerts),
    NL_TEST_DEF("Test CHIP Generate Root Certificate", TestChipCert_GenerateRootCert),
    NL_TEST_DEF("Test CHIP Generate Root Certificate with Fabric", TestChipCert_GenerateRootFabCert),
    NL_TEST_DEF("Test CHIP Generate ICA Certificate", TestChipCert_GenerateICACert),
    NL_TEST_DEF("Test CHIP Generate NOC using Root", TestChipCert_GenerateNOCRoot),
    NL_TEST_DEF("Test CHIP Generate NOC using ICA", TestChipCert_GenerateNOCICA),
    NL_TEST_DEF("Test CHIP Verify Generated Cert Chain", TestChipCert_VerifyGeneratedCerts),
    NL_TEST_DEF("Test CHIP Certificates X509 to CHIP Array Conversion", TestChipCert_X509ToChipArray),
    NL_TEST_DEF("Test CHIP Certificates X509 No ICA to CHIP Array Conversion", TestChipCert_X509ToChipArrayNoICA),
    NL_TEST_DEF("Test CHIP Certificates X509 to CHIP Array Conversion Error Scenarios", TestChipCert_X509ToChipArrayErrorScenarios),
    NL_TEST_DEF("Test CHIP Array to Chip Certificates Conversion", TestChipCert_ChipArrayToChipCerts),
    NL_TEST_DEF("Test No ICA CHIP Array to Chip Certificates Conversion", TestChipCert_ChipArrayToChipCertsNoICA),
    NL_TEST_DEF("Test extracting PeerId from node certificate", TestChipCert_ExtractPeerId),
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
