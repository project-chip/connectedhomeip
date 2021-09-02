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
#include <credentials/CHIPCert.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

#include "CHIPCert_test_vectors.h"

using namespace chip;
using namespace chip::TestCerts;

namespace {
static const BitFlags<CertDecodeFlags> sGenTBSHashFlag(CertDecodeFlags::kGenerateTBSHash);
static const BitFlags<CertDecodeFlags> sTrustAnchorFlag(CertDecodeFlags::kIsTrustAnchor);

static const BitFlags<TestCertLoadFlags> sNullLoadFlag;

static OperationalCredentialSerializable sSerialized;
static OperationalCredentialSerializable sSerialized2;
} // namespace

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

static void TestChipOperationalCredentials_CertValidation(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    OperationalCredentialSet opCredSet;
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
        //            Reqd                                    Exp   Exp                           Cert              Cert
        // Subj Valid Cert                                    Cert  TA     Cert                   Decode            Load
        // Ind  Flags Type    Expected Result                 Index Index  Type                   Flags             Flags
        // ==================================================================================================================================

        // Basic validation of leaf certificate with different Trusted Anchor indexes to be used on TrustedRootID Search.
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCerts::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCerts::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCerts::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  2,   0,    CTNS,   CHIP_NO_ERROR,                  2,    0, { { TestCerts::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCerts::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCerts::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
        {  2,   0,    CTNS,   CHIP_ERROR_CERT_NOT_FOUND,      2,    2, { { TestCerts::kRoot01,    sTrustAnchorFlag, sNullLoadFlag       },
                                                                         { TestCerts::kICA01,     sGenTBSHashFlag,  sNullLoadFlag       },
                                                                         { TestCerts::kNode01_01, sGenTBSHashFlag,  sNullLoadFlag       } } },
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
            if (testCase.InputCerts[i2].Type != TestCerts::kNone)
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
        NL_TEST_ASSERT(inSuite, opCredSet.Init(&certSet, 1) == CHIP_NO_ERROR);

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
        err = opCredSet.FindValidCert(trustedRootId, subjectDN, subjectKeyId, validContext, &resultCert);
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
        // Clear the Operational Credential Set
        opCredSet.Release();
    }
}

static void TestChipOperationalCredentials_Serialization(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    OperationalCredentialSet opCredSet;
    OperationalCredentialSet opCredSet2;
    P256Keypair keypair;
    P256SerializedKeypair serializedKeypair;
    enum
    {
        kMaxCerts = 2
    };

    // Initialize the certificate set and load the specified test certificates.
    err = certSet.Init(kMaxCerts);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = LoadTestCert(certSet, TestCerts::kRoot01, sNullLoadFlag, sTrustAnchorFlag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = LoadTestCert(certSet, TestCerts::kICA01, sNullLoadFlag, sGenTBSHashFlag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Initialize the Operational Credential Set and load certificate set
    NL_TEST_ASSERT(inSuite, opCredSet.Init(&certSet, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opCredSet2.Init(1) == CHIP_NO_ERROR);

    CertificateKeyId trustedRootId = opCredSet.GetTrustedRootId(static_cast<uint16_t>(opCredSet.GetCertCount() - 1));
    NL_TEST_ASSERT(inSuite, !trustedRootId.empty());

    NL_TEST_ASSERT(inSuite,
                   serializedKeypair.SetLength(sTestCert_Node01_01_PublicKey_Len + sTestCert_Node01_01_PrivateKey_Len) ==
                       CHIP_NO_ERROR);

    memcpy(static_cast<uint8_t *>(serializedKeypair), sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PublicKey_Len);
    memcpy(static_cast<uint8_t *>(serializedKeypair) + sTestCert_Node01_01_PublicKey_Len, sTestCert_Node01_01_PrivateKey,
           sTestCert_Node01_01_PrivateKey_Len);

    NL_TEST_ASSERT(inSuite, keypair.Deserialize(serializedKeypair) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, opCredSet.SetDevOpCredKeypair(trustedRootId, &keypair) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   opCredSet.SetDevOpCred(trustedRootId, sTestCert_Node01_01_Chip,
                                          static_cast<uint16_t>(sTestCert_Node01_01_Chip_Len)) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, opCredSet.ToSerializable(trustedRootId, sSerialized) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opCredSet2.FromSerializable(sSerialized) == CHIP_NO_ERROR);

    CertificateKeyId trustedRootId2 = opCredSet2.GetTrustedRootId(static_cast<uint16_t>(opCredSet2.GetCertCount() - 1));
    NL_TEST_ASSERT(inSuite, trustedRootId2.data_equal(trustedRootId));

    NL_TEST_ASSERT(inSuite, opCredSet2.ToSerializable(trustedRootId2, sSerialized2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   strncmp(reinterpret_cast<const char *>(&sSerialized), reinterpret_cast<const char *>(&sSerialized2),
                           sizeof(sSerialized)) == 0);

    // Clear the certificate set.
    certSet.Release();
    // Clear the Operational Credential Set
    opCredSet2.Release();
    opCredSet.Release();
}

/**
 *  Set up the test suite.
 */
int TestChipOperationalCredentials_Setup(void * inContext)
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
int TestChipOperationalCredentials_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test CHIP Certificate Validation", TestChipOperationalCredentials_CertValidation),
    NL_TEST_DEF("Test CHIP Certificate Serialization", TestChipOperationalCredentials_Serialization),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestChipOperationalCredentials(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "OperationalCredentials-CHIP-Certs",
        &sTests[0],
        TestChipOperationalCredentials_Setup,
        TestChipOperationalCredentials_Teardown
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestChipOperationalCredentials);
