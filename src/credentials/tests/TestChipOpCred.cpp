/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Unit tests for CHIP Operational Credential Set functionality.
 *
 */

#include <core/CHIPTLV.h>
#include <credentials/CHIPCert.h>
#include <credentials/CHIPOpCred.h>
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

static const BitFlags<uint8_t, CertValidateFlags> sIgnoreNotBeforeFlag(CertValidateFlags::kIgnoreNotBefore);
static const BitFlags<uint8_t, CertValidateFlags> sIgnoreNotAfterFlag(CertValidateFlags::kIgnoreNotAfter);

static const BitFlags<uint8_t, CertDecodeFlags> sNullDecodeFlag;
static const BitFlags<uint8_t, CertDecodeFlags> sGenTBSHashFlag(CertDecodeFlags::kGenerateTBSHash);
static const BitFlags<uint8_t, CertDecodeFlags> sTrustAnchorFlag(CertDecodeFlags::kIsTrustAnchor);

static const BitFlags<uint8_t, TestCertLoadFlags> sNullLoadFlag;
static const BitFlags<uint8_t, TestCertLoadFlags> sDerFormFlag(TestCertLoadFlags::kDERForm);
static const BitFlags<uint8_t, TestCertLoadFlags> sSupIsCAFlag(TestCertLoadFlags::kSuppressIsCA);
static const BitFlags<uint8_t, TestCertLoadFlags> sSupKeyUsageFlag(TestCertLoadFlags::kSuppressKeyUsage);
static const BitFlags<uint8_t, TestCertLoadFlags> sSupKeyCertSignFlag(TestCertLoadFlags::kSuppressKeyCertSign);
static const BitFlags<uint8_t, TestCertLoadFlags> sPathLenZeroFlag(TestCertLoadFlags::kSetPathLenConstZero);
static const BitFlags<uint8_t, TestCertLoadFlags> sAppDefCertTypeFlag(TestCertLoadFlags::kSetAppDefinedCertType);

static const BitFlags<uint8_t, KeyPurposeFlags> sNullKPFlag;
static const BitFlags<uint8_t, KeyPurposeFlags> sSA(KeyPurposeFlags::kServerAuth);
static const BitFlags<uint8_t, KeyPurposeFlags> sCA(KeyPurposeFlags::kClientAuth);
static const BitFlags<uint8_t, KeyPurposeFlags> sCS(KeyPurposeFlags::kCodeSigning);
static const BitFlags<uint8_t, KeyPurposeFlags> sEP(KeyPurposeFlags::kEmailProtection);
static const BitFlags<uint8_t, KeyPurposeFlags> sTS(KeyPurposeFlags::kTimeStamping);
static const BitFlags<uint8_t, KeyPurposeFlags> sOS(KeyPurposeFlags::kOCSPSigning);
static const BitFlags<uint8_t, KeyPurposeFlags> sSAandCA(sSA.Raw() | sCA.Raw());
static const BitFlags<uint8_t, KeyPurposeFlags> sSAandCS(sSA.Raw() | sCS.Raw());
static const BitFlags<uint8_t, KeyPurposeFlags> sSAandEP(sSA.Raw() | sEP.Raw());
static const BitFlags<uint8_t, KeyPurposeFlags> sSAandTS(sSA.Raw() | sTS.Raw());

static const BitFlags<uint16_t, KeyUsageFlags> sNullKUFlag;
static const BitFlags<uint16_t, KeyUsageFlags> sDS(KeyUsageFlags::kDigitalSignature);
static const BitFlags<uint16_t, KeyUsageFlags> sNR(KeyUsageFlags::kNonRepudiation);
static const BitFlags<uint16_t, KeyUsageFlags> sKE(KeyUsageFlags::kKeyEncipherment);
static const BitFlags<uint16_t, KeyUsageFlags> sDE(KeyUsageFlags::kDataEncipherment);
static const BitFlags<uint16_t, KeyUsageFlags> sKA(KeyUsageFlags::kKeyAgreement);
static const BitFlags<uint16_t, KeyUsageFlags> sKC(KeyUsageFlags::kKeyCertSign);
static const BitFlags<uint16_t, KeyUsageFlags> sCR(KeyUsageFlags::kCRLSign);
static const BitFlags<uint16_t, KeyUsageFlags> sEO(KeyUsageFlags::kEncipherOnly);
static const BitFlags<uint16_t, KeyUsageFlags> sDO(KeyUsageFlags::kDecipherOnly);
static const BitFlags<uint16_t, KeyUsageFlags> sDSandNR(sDS.Raw() | sNR.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sDSandKE(sDS.Raw() | sKE.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sDSandDE(sDS.Raw() | sDE.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sDSandKA(sDS.Raw() | sKA.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sDSandKC(sDS.Raw() | sKC.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sDSandCR(sDS.Raw() | sCR.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sDSandEO(sDS.Raw() | sEO.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sDSandDO(sDS.Raw() | sDO.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandDS(sKC.Raw() | sDS.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandNR(sKC.Raw() | sNR.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandKE(sKC.Raw() | sKE.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandDE(sKC.Raw() | sDE.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandKA(sKC.Raw() | sKA.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandCR(sKC.Raw() | sCR.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandEO(sKC.Raw() | sEO.Raw());
static const BitFlags<uint16_t, KeyUsageFlags> sKCandDO(sKC.Raw() | sDO.Raw());

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

static void TestChipOpCred_CertValidation(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ChipOperationalCredentialSet opCredSet;
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
            BitFlags<uint8_t, CertDecodeFlags> DecodeFlags;
            BitFlags<uint8_t, TestCertLoadFlags> LoadFlags;
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

        // Basic validation of leaf certificate with different Trusted Anchor indexes to be used on TrustedRootID Search.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCertTypes::kNodeCA,  sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCertTypes::kNode01,  sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  2,   0,    CTNS,   CHIP_ERROR_CERT_NOT_FOUND,      2,    2, { { TestCertTypes::kRoot,    sTrustAnchorFlag, sNullLoadFlag       },
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

        // Initialize the Operational Credential Set and load certificate set
        NL_TEST_ASSERT(inSuite, opCredSet.Init(&certSet, 1, nullptr, nullptr, 0) == CHIP_NO_ERROR);

        // Initialize the validation context.
        validContext.Reset();
        err = SetEffectiveTime(validContext, 2021, 1, 1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
        validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
        validContext.mValidateFlags.SetRaw(testCase.mValidateFlags);
        validContext.mRequiredCertType = testCase.mRequiredCertType;

        // Locate the subject DN and key id that will be used as input the FindValidCert() method.
        const ChipDN & subjectDN               = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectDN;
        const CertificateKeyId & subjectKeyId  = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectKeyId;
        const CertificateKeyId & trustedRootId = certSet.GetCertSet()[testCase.mExpectedTrustAnchorIndex].mAuthKeyId;

        // Invoke the FindValidCert() method (the method being tested).
        err = opCredSet.FindValidCert(trustedRootId, subjectDN, subjectKeyId, validContext, resultCert);
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

/**
 *  Set up the test suite.
 */
int TestChipOpCred_Setup(void * inContext)
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
int TestChipOpCred_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test CHIP Certificate Validation", TestChipOpCred_CertValidation),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestChipOpCred(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "OperationalCredentials-CHIP-Certs",
        &sTests[0],
        TestChipOpCred_Setup,
        TestChipOpCred_Teardown
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestChipOpCred);
