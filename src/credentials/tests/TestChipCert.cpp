/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Credentials;
using namespace chip::TestCerts;

enum
{
    kStandardCertsCount = 3
};

static CHIP_ERROR LoadStandardCerts(ChipCertificateSet & certSet)
{
    CHIP_ERROR err;

    err = LoadTestCert(certSet, kTestCert_Root | kDecodeFlag_IsTrustAnchor);
    SuccessOrExit(err);

    err = LoadTestCert(certSet, kTestCert_NodeCA | kDecodeFlag_GenerateTBSHash);
    SuccessOrExit(err);

    err = LoadTestCert(certSet, kTestCert_Node01 | kDecodeFlag_GenerateTBSHash);
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

    return PackCertTime(effectiveTime, validContext.EffectiveTime);
}

static void TestChipCert_ChipToX509(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    const uint8_t * inCert;
    size_t inCertLen;
    const uint8_t * expectedOutCert;
    size_t expectedOutCertLen;
    uint8_t outCertBuf[kTestCertBufSize];
    uint32_t outCertLen;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        int certSelector = gTestCerts[i];

        err = GetTestCert(certSelector, inCert, inCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = GetTestCert(certSelector | kTestCertLoadFlag_DERForm, expectedOutCert, expectedOutCertLen);
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
    size_t inCertLen;
    const uint8_t * expectedOutCert;
    size_t expectedOutCertLen;
    uint8_t outCertBuf[kTestCertBufSize];
    uint32_t outCertLen;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        int certSelector = gTestCerts[i];

        err = GetTestCert(certSelector | kTestCertLoadFlag_DERForm, inCert, inCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = GetTestCert(certSelector, expectedOutCert, expectedOutCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = ConvertX509CertToChipCert(inCert, inCertLen, outCertBuf, sizeof(outCertBuf), outCertLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, outCertLen == expectedOutCertLen);
        NL_TEST_ASSERT(inSuite, memcmp(outCertBuf, expectedOutCert, outCertLen) == 0);
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
        int SubjectCertIndex;
        uint16_t ValidateFlags;
        uint8_t RequiredCertType;
        CHIP_ERROR ExpectedResult;
        int ExpectedCertIndex;
        int ExpectedTrustAnchorIndex;
        int InputCerts[kMaxCertsPerTestCase];
    };

    enum
    {
        // Short-hand names to make the test cases table more concise.
        Root                  = kTestCert_Root,
        RootKey               = kTestCert_RootKey,
        CA                    = kTestCert_NodeCA,
        Node                  = kTestCert_Node01,
        ReqSHA256             = kValidateFlag_RequireSHA256,
        IsTrusted             = kDecodeFlag_IsTrustAnchor,
        GenTBSHash            = kDecodeFlag_GenerateTBSHash,
        SupIsCA               = kTestCertLoadFlag_SuppressIsCA,
        SupKeyUsage           = kTestCertLoadFlag_SuppressKeyUsage,
        SupKeyCertSign        = kTestCertLoadFlag_SuppressKeyCertSign,
        SetPathLenZero        = kTestCertLoadFlag_SetPathLenConstZero,
        SetAppDefinedCertType = kTestCertLoadFlag_SetAppDefinedCertType,
        CTNS                  = kCertType_NotSpecified,
        CTCA                  = kCertType_CA,
        CTNode                = kCertType_Node,
        CTFS                  = kCertType_FirmwareSigning,
        CTAD                  = kCertType_AppDefinedBase,
    };

    // clang-format off
    static const ValidationTestCase sValidationTestCases[] = {
        //                       Reqd
        // Subject   Valid       Cert                                    Expected    Expected                  Input
        // Index     Flags       Type    Expected Result                 Cert Index  TA Index   Input Certs    Cert Flags
        // ==============================================================================================================

        // Basic validation of leaf certificate with different load orders.
        {  2,        0,          CTNS,   CHIP_NO_ERROR,                  2,          0,       { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash } },
        {  1,        0,          CTNS,   CHIP_NO_ERROR,                  1,          0,       { Root          | IsTrusted,
                                                                                                Node          | GenTBSHash,
                                                                                                CA            | GenTBSHash } },
        {  0,        0,          CTNS,   CHIP_NO_ERROR,                  0,          2,       { Node          | GenTBSHash,
                                                                                                CA            | GenTBSHash,
                                                                                                Root          | IsTrusted } },

        // Validation of leaf certificate with root key only.
        {  1,        0,          CTNS,   CHIP_NO_ERROR,                  1,          0,       { RootKey,
                                                                                                Node          | GenTBSHash,
                                                                                                CA            | GenTBSHash } },

        // Validation with two copies of root certificate, one trusted, one untrusted.
        {  2,        0,          CTNS,   CHIP_NO_ERROR,                  2,          1,       { Root,
                                                                                                Root          | IsTrusted,
                                                                                                Node          | GenTBSHash,
                                                                                                CA            | GenTBSHash } },

        // Validation with trusted root key and trusted root certificate.
        {  2,        0,          CTNS,   CHIP_NO_ERROR,                  2,          0,       { RootKey,
                                                                                                Root          | IsTrusted,
                                                                                                Node          | GenTBSHash,
                                                                                                CA            | GenTBSHash } },

        // Validation with trusted root key and untrusted root certificate.
        {  3,        0,          CTNS,   CHIP_NO_ERROR,                  3,          1,       { Root,
                                                                                                RootKey,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash } },

        // Failure due to missing CA certificate.
        {  1,        0,          CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root          | IsTrusted,
                                                                                                Node          | GenTBSHash } },

        // Failure due to missing root certificate.
        {  1,        0,          CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash } },

        // Failure due to lack of TBS hash.
        {  1,        0,          CTNS,   CHIP_ERROR_INVALID_ARGUMENT,    -1,         -1,      { Root          | IsTrusted,
                                                                                                Node,
                                                                                                CA            | GenTBSHash } },

        // Failure due to untrusted root.
        {  1,        0,          CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root,
                                                                                                Node          | GenTBSHash,
                                                                                                CA            | GenTBSHash } },

        // Failure due to intermediate cert with isCA flag = false
        {  2,        0,          CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash | SupIsCA,
                                                                                                Node          | GenTBSHash } },

        // Failure due to CA cert with no key usage.
        {  2,        0,          CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash | SupKeyUsage,
                                                                                                Node          | GenTBSHash } },

        // Failure due to CA cert with no cert sign key usage.
        {  2,        0,          CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash | SupKeyCertSign,
                                                                                                Node          | GenTBSHash } },

        // Failure due to 3-level deep cert chain and root cert with path constraint == 0
        {  2,        0,          CTNS,   CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root          | IsTrusted | SetPathLenZero,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash } },

        // Require a specific certificate type.
        {  2,        0,          CTNode, CHIP_NO_ERROR,                  2,          0,       { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash } },

        // Require a certificate with an application-defined type.
        {  2,        0,          CTAD,   CHIP_NO_ERROR,                  2,          0,       { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash | SetAppDefinedCertType } },

        // Select between two identical certificates with different types.
        {  2,        0,          CTAD,   CHIP_NO_ERROR,                  3,          0,       { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash,
                                                                                                Node          | GenTBSHash | SetAppDefinedCertType } },

        // Failure due to required certificate type not found.
        {  2,        0,          CTCA,   CHIP_ERROR_WRONG_CERT_TYPE,     -1,         -1,      { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash } },

        // Failure due to CA certificate having wrong type.
        {  2,        0,          CTNode, CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root          | IsTrusted,
                                                                                                CA            | GenTBSHash | SetAppDefinedCertType,
                                                                                                Node          | GenTBSHash } },

        // Failure due to root certificate having wrong type.
        {  2,        0,          CTNode, CHIP_ERROR_CA_CERT_NOT_FOUND,   -1,         -1,      { Root          | IsTrusted | SetAppDefinedCertType,
                                                                                                CA            | GenTBSHash,
                                                                                                Node          | GenTBSHash } },
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
            if (testCase.InputCerts[i2] != 0)
            {
                err = LoadTestCert(certSet, testCase.InputCerts[i2]);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            }
        // Make sure the test case is valid.
        NL_TEST_ASSERT(inSuite, testCase.SubjectCertIndex >= 0 && testCase.SubjectCertIndex < certSet.CertCount);
        if (testCase.ExpectedResult == CHIP_NO_ERROR)
        {
            NL_TEST_ASSERT(inSuite, testCase.ExpectedCertIndex >= 0 && testCase.ExpectedCertIndex < certSet.CertCount);
            NL_TEST_ASSERT(inSuite,
                           testCase.ExpectedTrustAnchorIndex >= 0 && testCase.ExpectedTrustAnchorIndex < certSet.CertCount);
        }

        // Initialize the validation context.
        memset(&validContext, 0, sizeof(validContext));
        err = SetEffectiveTime(validContext, 2021, 1, 1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        validContext.ValidateFlags       = testCase.ValidateFlags;
        validContext.RequiredKeyUsages   = kKeyUsageFlag_DigitalSignature;
        validContext.RequiredKeyPurposes = kKeyPurposeFlag_ServerAuth;
        validContext.RequiredCertType    = testCase.RequiredCertType;

        // Locate the subject DN and key id that will be used as input the FindValidCert() method.
        const ChipDN & subjectDN              = certSet.Certs[testCase.SubjectCertIndex].SubjectDN;
        const CertificateKeyId & subjectKeyId = certSet.Certs[testCase.SubjectCertIndex].SubjectKeyId;

        // Invoke the FindValidCert() method (the method being tested).
        err = certSet.FindValidCert(subjectDN, subjectKeyId, validContext, resultCert);
        NL_TEST_ASSERT(inSuite, err == testCase.ExpectedResult);

        // If the test case is expected to be successful...
        if (err == CHIP_NO_ERROR)
        {
            // Verify that the method found the correct certificate.
            NL_TEST_ASSERT(inSuite, resultCert == &certSet.Certs[testCase.ExpectedCertIndex]);

            // Verify that the method selected the correct trust anchor.
            NL_TEST_ASSERT(inSuite, validContext.TrustAnchor == &certSet.Certs[testCase.ExpectedTrustAnchorIndex]);
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

    memset(&validContext, 0, sizeof(validContext));
    validContext.RequiredKeyUsages   = kKeyUsageFlag_DigitalSignature;
    validContext.RequiredKeyPurposes = kKeyPurposeFlag_ServerAuth;

    // Before certificate validity period.
    err = SetEffectiveTime(validContext, 2020, 1, 3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_NOT_VALID_YET);

    // 1 second before validity period.
    err = SetEffectiveTime(validContext, 2020, 10, 14, 23, 59, 59);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_NOT_VALID_YET);

    // 1st second of 1st day of validity period.
    // NOTE: the given time is technically outside the stated certificate validity period, which starts mid-day.
    // However for simplicity's sake, the Chip cert validation algorithm rounds the validity period to whole days.
    err = SetEffectiveTime(validContext, 2020, 10, 15, 0, 0, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Last second of last day of validity period.
    // As above, this time is considered valid because of rounding to whole days.
    err = SetEffectiveTime(validContext, 2040, 10, 15, 23, 59, 59);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // 1 second after end of certificate validity period.
    err = SetEffectiveTime(validContext, 2040, 10, 16, 0, 0, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_EXPIRED);

    // After end of certificate validity period.
    err = SetEffectiveTime(validContext, 2042, 4, 25, 0, 0, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_CERT_EXPIRED);

    // Ignore 'not before' time.
    validContext.ValidateFlags = kValidateFlag_IgnoreNotBefore;
    err                        = SetEffectiveTime(validContext, 2020, 4, 23, 23, 59, 59);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Ignore 'not after' time.
    validContext.ValidateFlags = kValidateFlag_IgnoreNotAfter;
    err                        = SetEffectiveTime(validContext, 2042, 5, 25, 0, 0, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = certSet.ValidateCert(certSet.Certs[certSet.CertCount - 1], validContext);
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
        uint8_t CertIndex;
        uint16_t RequiredKeyUsages;
        uint16_t RequiredKeyPurposes;
        CHIP_ERROR ExpectedResult;
    };

    enum
    {
        // Short-hand names to make the test cases table more concise.
        SA = kKeyPurposeFlag_ServerAuth,
        CA = kKeyPurposeFlag_ClientAuth,
        CS = kKeyPurposeFlag_CodeSigning,
        EP = kKeyPurposeFlag_EmailProtection,
        TS = kKeyPurposeFlag_TimeStamping,
        OS = kKeyPurposeFlag_OCSPSigning,
        DS = kKeyUsageFlag_DigitalSignature,
        NR = kKeyUsageFlag_NonRepudiation,
        KE = kKeyUsageFlag_KeyEncipherment,
        DE = kKeyUsageFlag_DataEncipherment,
        KA = kKeyUsageFlag_KeyAgreement,
        KC = kKeyUsageFlag_KeyCertSign,
        CR = kKeyUsageFlag_CRLSign,
        EO = kKeyUsageFlag_EncipherOnly,
        DO = kKeyUsageFlag_DecipherOnly,
    };

    // clang-format off
    static UsageTestCase sUsageTestCases[] = {
        // CertIndex    KeyUsages   KeyPurposes     ExpectedResult
        // =================================================================================

        // ----- Key Usages for leaf Certificate -----
        {  2,           DS,         0,              CHIP_NO_ERROR                        },
        {  2,           NR,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           KE,         0,              CHIP_NO_ERROR                        },
        {  2,           DE,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           KA,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           KC,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           CR,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           EO,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DO,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|NR,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|KE,      0,              CHIP_NO_ERROR                        },
        {  2,           DS|DE,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|KA,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|KC,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|CR,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|EO,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|DO,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Key Usages for CA Certificate -----
        {  1,           DS,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           NR,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KE,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           DE,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KA,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC,         0,              CHIP_NO_ERROR                        },
        {  1,           CR,         0,              CHIP_NO_ERROR                        },
        {  1,           EO,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           DO,         0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC|DS,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC|NR,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC|KE,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC|DE,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC|KA,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC|CR,      0,              CHIP_NO_ERROR                        },
        {  1,           KC|EO,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           KC|DO,      0,              CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Key Purposes for leaf Certificate -----
        {  2,           0,          SA,             CHIP_NO_ERROR                        },
        {  2,           0,          CA,             CHIP_NO_ERROR                        },
        {  2,           0,          CS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           0,          EP,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           0,          TS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           0,          SA|CA,          CHIP_NO_ERROR                        },
        {  2,           0,          SA|CS,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           0,          SA|EP,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           0,          SA|TS,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Key Purposes for CA Certificate -----
        {  1,           0,          SA,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          CA,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          CS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          EP,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          TS,             CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          SA|CA,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          SA|CS,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          SA|EP,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  1,           0,          SA|TS,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },

        // ----- Combinations -----
        {  2,           DS|NR,      SA|CA,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|KE,      SA|CS,          CHIP_ERROR_CERT_USAGE_NOT_ALLOWED    },
        {  2,           DS|KE,      SA|CA,          CHIP_NO_ERROR                        },
    };
    // clang-format on
    size_t sNumUsageTestCases = sizeof(sUsageTestCases) / sizeof(sUsageTestCases[0]);

    certSet.Init(kStandardCertsCount, kTestCertBufSize);

    err = LoadStandardCerts(certSet);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    for (size_t i = 0; i < sNumUsageTestCases; i++)
    {
        memset(&validContext, 0, sizeof(validContext));
        err = SetEffectiveTime(validContext, 2020, 10, 16);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        validContext.RequiredKeyUsages   = sUsageTestCases[i].RequiredKeyUsages;
        validContext.RequiredKeyPurposes = sUsageTestCases[i].RequiredKeyPurposes;
        err                              = certSet.ValidateCert(certSet.Certs[sUsageTestCases[i].CertIndex], validContext);
        NL_TEST_ASSERT(inSuite, err == sUsageTestCases[i].ExpectedResult);
    }

    certSet.Release();
}

static void TestChipCert_CertType(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;

    struct TestCase
    {
        int Cert;
        uint8_t ExpectedCertType;
    };

    enum
    {
        // Short-hand names to make the test cases table more concise.
        Root            = kTestCert_Root,
        RootKey         = kTestCert_RootKey,
        CA              = kTestCert_NodeCA,
        Node            = kTestCert_Node01,
        FirmwareSigning = kTestCert_FirmwareSigning,
    };

    // clang-format off
    static TestCase sTestCases[] = {
        // Cert                ExpectedCertType
        // ================================================
        {  Root,               kCertType_CA               },
        {  RootKey,            kCertType_CA               },
        {  CA,                 kCertType_CA               },
        {  Node,               kCertType_Node             },
        {  FirmwareSigning,    kCertType_FirmwareSigning  },
    };
    // clang-format on
    static const size_t sNumTestCases = sizeof(sTestCases) / sizeof(sTestCases[0]);

    for (unsigned i = 0; i < sNumTestCases; i++)
    {
        const TestCase & testCase = sTestCases[i];

        // Initialize the certificate set and load the test certificate.
        certSet.Init(1, kTestCertBufSize);
        err = LoadTestCert(certSet, testCase.Cert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, certSet.Certs[0].CertType == testCase.ExpectedCertType);
    }
}

/**
 *  Set up the test suite.
 */
int TestChipCert_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
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
