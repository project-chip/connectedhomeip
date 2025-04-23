/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <credentials/CHIPCert.h>
#include <credentials/examples/LastKnownGoodTimeCertificateValidityPolicyExample.h>
#include <credentials/examples/StrictCertificateValidityPolicyExample.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/PeerId.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include "CHIPCert_error_test_vectors.h"
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

static constexpr BitFlags<CertDecodeFlags> sNullDecodeFlag;
static constexpr BitFlags<CertDecodeFlags> sGenTBSHashFlag(CertDecodeFlags::kGenerateTBSHash);
static constexpr BitFlags<CertDecodeFlags> sTrustAnchorFlag(CertDecodeFlags::kIsTrustAnchor);

static constexpr BitFlags<TestCertLoadFlags> sNullLoadFlag;
static constexpr BitFlags<TestCertLoadFlags> sDerFormFlag(TestCertLoadFlags::kDERForm);
static constexpr BitFlags<TestCertLoadFlags> sSupIsCAFlag(TestCertLoadFlags::kSuppressIsCA);
static constexpr BitFlags<TestCertLoadFlags> sSupKeyUsageFlag(TestCertLoadFlags::kSuppressKeyUsage);
static constexpr BitFlags<TestCertLoadFlags> sSupKeyCertSignFlag(TestCertLoadFlags::kSuppressKeyCertSign);
static constexpr BitFlags<TestCertLoadFlags> sPathLenZeroFlag(TestCertLoadFlags::kSetPathLenConstZero);

static constexpr BitFlags<KeyPurposeFlags> sNullKPFlag;
static constexpr BitFlags<KeyPurposeFlags> sSA(KeyPurposeFlags::kServerAuth);
static constexpr BitFlags<KeyPurposeFlags> sCA(KeyPurposeFlags::kClientAuth);
static constexpr BitFlags<KeyPurposeFlags> sCS(KeyPurposeFlags::kCodeSigning);
static constexpr BitFlags<KeyPurposeFlags> sEP(KeyPurposeFlags::kEmailProtection);
static constexpr BitFlags<KeyPurposeFlags> sTS(KeyPurposeFlags::kTimeStamping);
// static constexpr BitFlags<KeyPurposeFlags> sOS(KeyPurposeFlags::kOCSPSigning); // unused
static constexpr BitFlags<KeyPurposeFlags> sSAandCA(sSA, sCA);
static constexpr BitFlags<KeyPurposeFlags> sSAandCS(sSA, sCS);
static constexpr BitFlags<KeyPurposeFlags> sSAandEP(sSA, sEP);
static constexpr BitFlags<KeyPurposeFlags> sSAandTS(sSA, sTS);

static constexpr BitFlags<KeyUsageFlags> sNullKUFlag;
static constexpr BitFlags<KeyUsageFlags> sDS(KeyUsageFlags::kDigitalSignature);
static constexpr BitFlags<KeyUsageFlags> sNR(KeyUsageFlags::kNonRepudiation);
static constexpr BitFlags<KeyUsageFlags> sKE(KeyUsageFlags::kKeyEncipherment);
static constexpr BitFlags<KeyUsageFlags> sDE(KeyUsageFlags::kDataEncipherment);
static constexpr BitFlags<KeyUsageFlags> sKA(KeyUsageFlags::kKeyAgreement);
static constexpr BitFlags<KeyUsageFlags> sKC(KeyUsageFlags::kKeyCertSign);
static constexpr BitFlags<KeyUsageFlags> sCR(KeyUsageFlags::kCRLSign);
static constexpr BitFlags<KeyUsageFlags> sEO(KeyUsageFlags::kEncipherOnly);
static constexpr BitFlags<KeyUsageFlags> sDO(KeyUsageFlags::kDecipherOnly);
static constexpr BitFlags<KeyUsageFlags> sDSandNR(sDS, sNR);
static constexpr BitFlags<KeyUsageFlags> sDSandKE(sDS, sKE);
static constexpr BitFlags<KeyUsageFlags> sDSandDE(sDS, sDE);
static constexpr BitFlags<KeyUsageFlags> sDSandKA(sDS, sKA);
static constexpr BitFlags<KeyUsageFlags> sDSandKC(sDS, sKC);
static constexpr BitFlags<KeyUsageFlags> sDSandCR(sDS, sCR);
static constexpr BitFlags<KeyUsageFlags> sDSandEO(sDS, sEO);
static constexpr BitFlags<KeyUsageFlags> sDSandDO(sDS, sDO);
static constexpr BitFlags<KeyUsageFlags> sKCandDS(sKC, sDS);
static constexpr BitFlags<KeyUsageFlags> sKCandNR(sKC, sNR);
static constexpr BitFlags<KeyUsageFlags> sKCandKE(sKC, sKE);
static constexpr BitFlags<KeyUsageFlags> sKCandDE(sKC, sDE);
static constexpr BitFlags<KeyUsageFlags> sKCandKA(sKC, sKA);
static constexpr BitFlags<KeyUsageFlags> sKCandCR(sKC, sCR);
static constexpr BitFlags<KeyUsageFlags> sKCandEO(sKC, sEO);
static constexpr BitFlags<KeyUsageFlags> sKCandDO(sKC, sDO);

constexpr uint8_t sOID_Extension_SubjectAltName[] = { 0x55, 0x1d, 0x11 };
constexpr char kExtension_SubjectAltName[]        = "test@example.com";

FutureExtension ext{ ByteSpan(sOID_Extension_SubjectAltName),
                     ByteSpan(reinterpret_cast<uint8_t *>(const_cast<char *>(kExtension_SubjectAltName)),
                              strlen(kExtension_SubjectAltName)) };
Optional<FutureExtension> kSubjectAltNameAsFutureExt(ext);

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

static CHIP_ERROR SetCurrentTime(ValidationContext & validContext, uint16_t year, uint8_t mon, uint8_t day, uint8_t hour = 0,
                                 uint8_t min = 0, uint8_t sec = 0)
{
    ASN1UniversalTime currentTime;

    currentTime.Year   = year;
    currentTime.Month  = mon;
    currentTime.Day    = day;
    currentTime.Hour   = hour;
    currentTime.Minute = min;
    currentTime.Second = sec;

    return validContext.SetEffectiveTimeFromAsn1Time<CurrentChipEpochTime>(currentTime);
}

static CHIP_ERROR SetLastKnownGoodTime(ValidationContext & validContext, uint16_t year, uint8_t mon, uint8_t day, uint8_t hour = 0,
                                       uint8_t min = 0, uint8_t sec = 0)
{
    ASN1UniversalTime lastKnownGoodTime;

    lastKnownGoodTime.Year   = year;
    lastKnownGoodTime.Month  = mon;
    lastKnownGoodTime.Day    = day;
    lastKnownGoodTime.Hour   = hour;
    lastKnownGoodTime.Minute = min;
    lastKnownGoodTime.Second = sec;

    return validContext.SetEffectiveTimeFromAsn1Time<LastKnownGoodChipEpochTime>(lastKnownGoodTime);
}

static void ClearTimeSource(ValidationContext & validContext)
{
    validContext.mEffectiveTime = EffectiveTime{};
}

struct TestChipCert : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestChipCert, TestChipCert_ChipToX509)
{
    CHIP_ERROR err;
    ByteSpan inCert;
    ByteSpan expectedOutCert;
    uint8_t outCertBuf[kMaxDERCertLength];

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        TestCert certType = gTestCerts[i];

        err = GetTestCert(certType, sNullLoadFlag, inCert);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = GetTestCert(certType, sDerFormFlag, expectedOutCert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        MutableByteSpan outCert(outCertBuf);
        err = ConvertChipCertToX509Cert(inCert, outCert);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(expectedOutCert.data_equal(outCert));
    }

    // Error Case:
    MutableByteSpan outCert(outCertBuf);
    err = ConvertChipCertToX509Cert(sTestCert_Node01_01_Err01_Chip, outCert);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_TLV_TAG);
}

TEST_F(TestChipCert, TestChipCert_ChipToX509_ErrorCases)
{
    CHIP_ERROR err;
    uint8_t outCertBuf[kMaxDERCertLength];

    for (auto chipCert : gTestCert_ChipToX509_ErrorCases)
    {
        MutableByteSpan outCert(outCertBuf);

        err = ConvertChipCertToX509Cert(chipCert, outCert);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }
}

TEST_F(TestChipCert, TestChipCert_ChipCertLoad_ErrorCases)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;

    err = certSet.Init(1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    for (auto chipCert : gTestCert_ChipCertLoad_ErrorCases)
    {
        err = certSet.LoadCert(chipCert, sNullDecodeFlag);
        EXPECT_NE(err, CHIP_NO_ERROR);

        certSet.Clear();
    }

    certSet.Release();
}

TEST_F(TestChipCert, TestChipCert_ValidateChipRCAC_ErrorCases)
{
    CHIP_ERROR err;

    for (auto chipCert : gTestCert_ValidateChipRCAC_ErrorCases)
    {
        err = ValidateChipRCAC(chipCert);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }
}

TEST_F(TestChipCert, TestChipCert_GetCertType_ErrorCases)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;

    err = certSet.Init(1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    for (auto chipCert : gTestCert_GetCertType_ErrorCases)
    {
        CertType certType;

        err = certSet.LoadCert(chipCert, sNullDecodeFlag);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = certSet.GetCertSet()->mSubjectDN.GetCertType(certType);
        EXPECT_TRUE(err != CHIP_NO_ERROR || certType == CertType::kNotSpecified);

        certSet.Clear();
    }

    certSet.Release();
}

TEST_F(TestChipCert, TestChipCert_X509ToChip)
{
    CHIP_ERROR err;
    ByteSpan inCert;
    ByteSpan expectedOutCert;
    uint8_t outCertBuf[kMaxCHIPCertLength];

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        TestCert certType = gTestCerts[i];

        err = GetTestCert(certType, sDerFormFlag, inCert);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = GetTestCert(certType, sNullLoadFlag, expectedOutCert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        MutableByteSpan outCert(outCertBuf);
        err = ConvertX509CertToChipCert(inCert, outCert);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(expectedOutCert.data_equal(outCert));
    }
}

TEST_F(TestChipCert, TestChipCert_X509ToChip_ErrorCases)
{
    CHIP_ERROR err;
    uint8_t outCertBuf[kMaxCHIPCertLength];

    for (auto derCert : gTestCert_X509ToChip_ErrorCases)
    {
        MutableByteSpan outCert(outCertBuf);

        err = ConvertX509CertToChipCert(derCert, outCert);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }
}

TEST_F(TestChipCert, TestChipCert_ChipDN)
{
    const static char noc_rdn[]     = "Test NOC";
    const static char noc_rdn2[]    = "John";
    const static CATValues noc_cats = { { 0xABCD0001, chip::kUndefinedCAT, chip::kUndefinedCAT } };

    ChipDN chip_dn;
    CertType certType = CertType::kFirmwareSigning; // Start with non-default value

    EXPECT_TRUE(chip_dn.IsEmpty());
    EXPECT_EQ(chip_dn.RDNCount(), 0);
    EXPECT_EQ(chip_dn.GetCertType(certType), CHIP_NO_ERROR);
    EXPECT_TRUE(chip_dn.IsEmpty());
    EXPECT_EQ(certType, CertType::kNotSpecified);

    EXPECT_EQ(chip_dn.AddAttribute_CommonName(CharSpan(noc_rdn, strlen(noc_rdn)), false), CHIP_NO_ERROR);
    EXPECT_EQ(chip_dn.AddAttribute_MatterNodeId(0xAAAABBBBCCCCDDDD), CHIP_NO_ERROR);
    EXPECT_EQ(chip_dn.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);
    EXPECT_EQ(chip_dn.AddAttribute_GivenName(CharSpan(noc_rdn2, strlen(noc_rdn2)), true), CHIP_NO_ERROR);
    EXPECT_EQ(chip_dn.AddCATs(noc_cats), CHIP_NO_ERROR);
    EXPECT_EQ(chip_dn.RDNCount(), 5);

    EXPECT_EQ(chip_dn.AddAttribute_GivenName(CharSpan(noc_rdn2, strlen(noc_rdn2)), true), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(chip_dn.RDNCount(), 5);

    EXPECT_EQ(chip_dn.GetCertType(certType), CHIP_NO_ERROR);
    EXPECT_EQ(certType, CertType::kNode);

    uint64_t certId;
    EXPECT_EQ(chip_dn.GetCertChipId(certId), CHIP_NO_ERROR);
    EXPECT_EQ(certId, 0xAAAABBBBCCCCDDDD);

    uint64_t fabricId;
    EXPECT_EQ(chip_dn.GetCertFabricId(fabricId), CHIP_NO_ERROR);
    EXPECT_EQ(fabricId, 0xFAB00000FAB00001);

    chip_dn.Clear();
    EXPECT_EQ(chip_dn.GetCertType(certType), CHIP_NO_ERROR);
    EXPECT_TRUE(chip_dn.IsEmpty());
    EXPECT_EQ(certType, CertType::kNotSpecified);

    CATValues noc_cats2;
    chip::CATValues::Serialized serializedCATs;
    EXPECT_EQ(noc_cats.Serialize(serializedCATs), CHIP_NO_ERROR);
    EXPECT_EQ(noc_cats2.Deserialize(serializedCATs), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(&noc_cats, &noc_cats2, chip::CATValues::kSerializedLength), 0);

    CATValues noc_cats3 = { { 0xABCD0001, 0xFFEEAA00, 0x0001F012 } };
    EXPECT_EQ(noc_cats3.Serialize(serializedCATs), CHIP_NO_ERROR);
    EXPECT_EQ(noc_cats2.Deserialize(serializedCATs), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(&noc_cats3, &noc_cats2, chip::CATValues::kSerializedLength), 0);
}

TEST_F(TestChipCert, TestChipCert_CertValidation)
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
        CertType mRequiredCertType;
        CHIP_ERROR mExpectedResult;
        int mExpectedCertIndex;
        int mExpectedTrustAnchorIndex;
        struct
        {
            TestCert Type;
            BitFlags<CertDecodeFlags> DecodeFlags;
            BitFlags<TestCertLoadFlags> LoadFlags;
        } InputCerts[kMaxCertsPerTestCase];
    };

    // Short-hand names to make the test cases table more concise.
    const auto CTNS   = CertType::kNotSpecified;
    const auto CTCA   = CertType::kICA;
    const auto CTNode = CertType::kNode;

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

    for (const auto & testCase : sValidationTestCases)
    {
        const ChipCertificateData * resultCert = nullptr;
        err                                    = certSet.Init(kMaxCertsPerTestCase);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        for (auto inputCert : testCase.InputCerts)
        {
            if (inputCert.Type != TestCert::kNone)
            {
                err = LoadTestCert(certSet, inputCert.Type, inputCert.LoadFlags, inputCert.DecodeFlags);
                EXPECT_EQ(err, CHIP_NO_ERROR);
            }
        }
        // Make sure the test case is valid.
        EXPECT_TRUE(testCase.mSubjectCertIndex >= 0 && testCase.mSubjectCertIndex < certSet.GetCertCount());
        if (testCase.mExpectedResult == CHIP_NO_ERROR)
        {
            EXPECT_TRUE(testCase.mExpectedCertIndex >= 0 && testCase.mExpectedCertIndex < certSet.GetCertCount());
            EXPECT_TRUE(testCase.mExpectedTrustAnchorIndex >= 0 && testCase.mExpectedTrustAnchorIndex < certSet.GetCertCount());
        }

        // Initialize the validation context.
        validContext.Reset();
        err = SetCurrentTime(validContext, 2021, 1, 1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
        validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
        validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);
        validContext.mRequiredCertType = testCase.mRequiredCertType;

        // Locate the subject DN and key id that will be used as input the FindValidCert() method.
        const ChipDN & subjectDN              = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectDN;
        const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[testCase.mSubjectCertIndex].mSubjectKeyId;

        // Invoke the FindValidCert() method (the method being tested).
        err = certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert);
        EXPECT_EQ(err, testCase.mExpectedResult);

        // If the test case is expected to be successful...
        if (err == CHIP_NO_ERROR)
        {
            // Verify that the method found the correct certificate.
            EXPECT_EQ(resultCert, &certSet.GetCertSet()[testCase.mExpectedCertIndex]);

            // Verify that the method selected the correct trust anchor.
            EXPECT_EQ(validContext.mTrustAnchor, &certSet.GetCertSet()[testCase.mExpectedTrustAnchorIndex]);
        }

        // Clear the certificate set.
        certSet.Release();
    }
}

TEST_F(TestChipCert, TestChipCert_CertValidTime)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ValidationContext validContext;

    err = certSet.Init(kStandardCertsCount);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = LoadTestCertSet01(certSet);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    Credentials::StrictCertificateValidityPolicyExample strictCertificateValidityPolicy;
    Credentials::LastKnownGoodTimeCertificateValidityPolicyExample lastKnownGoodTimeValidityPolicy;

    // No time source available.
    ClearTimeSource(validContext);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Current time before certificate validity period.
    err = SetCurrentTime(validContext, 2020, 1, 3);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);

    // Current time 1 second before validity period.
    err = SetCurrentTime(validContext, 2020, 10, 15, 14, 23, 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);

    // Current time 1st second of validity period.
    err = SetCurrentTime(validContext, 2020, 10, 15, 14, 23, 43);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Current time within validity period.
    err = SetCurrentTime(validContext, 2022, 02, 23, 12, 30, 01);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Current time at last second of validity period.
    err = SetCurrentTime(validContext, 2040, 10, 15, 14, 23, 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Current time at 1 second after end of certificate validity period.
    err = SetCurrentTime(validContext, 2040, 10, 15, 14, 23, 43);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);

    // Current time after end of certificate validity period.
    err = SetCurrentTime(validContext, 2042, 4, 25, 0, 0, 0);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);

    // Last known good time before certificate validity period.
    // We can't invalidate based on NotBefore with Last Known Good Time.
    // Hence, we expect CHIP_NO_ERROR.
    err = SetLastKnownGoodTime(validContext, 2020, 1, 3);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Last known good time 1 second before certificate validity period.
    // We can't invalidate based on NotBefore with Last Known Good Time.
    // Hence, we expect CHIP_NO_ERROR.
    err = SetLastKnownGoodTime(validContext, 2020, 10, 15, 14, 23, 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Last Known Good Time 1st second of validity period.
    err = SetLastKnownGoodTime(validContext, 2020, 10, 15, 14, 23, 43);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Last Known Good Time within validity period.
    err = SetLastKnownGoodTime(validContext, 2022, 02, 23, 12, 30, 01);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Last Known Good Time at last second of validity period.
    err = SetLastKnownGoodTime(validContext, 2040, 10, 15, 14, 23, 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Last Known Good Time at 1 second after end of certificate validity period.
    err = SetLastKnownGoodTime(validContext, 2040, 10, 15, 14, 23, 43);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);

    // Last Known Good Time after end of certificate validity period.
    err = SetLastKnownGoodTime(validContext, 2042, 4, 25, 0, 0, 0);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictCertificateValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimeValidityPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);

    certSet.Release();
}

TEST_F(TestChipCert, TestChipCert_ValidateChipRCAC)
{
    struct RCACTestCase
    {
        TestCert Cert;
        CHIP_ERROR mExpectedResult;
    };

    // clang-format off
    static RCACTestCase sRCACTestCases[] = {
        // Cert                    Expected Result
        // ====================================================
        {  TestCert::kRoot01,      CHIP_NO_ERROR              },
        {  TestCert::kRoot02,      CHIP_NO_ERROR              },
        {  TestCert::kICA01,       CHIP_ERROR_WRONG_CERT_TYPE },
        {  TestCert::kICA02,       CHIP_ERROR_WRONG_CERT_TYPE },
        {  TestCert::kICA01_1,     CHIP_ERROR_WRONG_CERT_TYPE },
        {  TestCert::kFWSign01,    CHIP_ERROR_WRONG_CERT_TYPE },
        {  TestCert::kNode01_01,   CHIP_ERROR_WRONG_CERT_TYPE },
        {  TestCert::kNode02_08,   CHIP_ERROR_WRONG_CERT_TYPE },
    };
    // clang-format on

    for (auto & testCase : sRCACTestCases)
    {
        ByteSpan cert;
        EXPECT_EQ(GetTestCert(testCase.Cert, sNullLoadFlag, cert), CHIP_NO_ERROR);
        EXPECT_EQ(ValidateChipRCAC(cert), testCase.mExpectedResult);
    }
}

class AlwaysAcceptValidityPolicy : public CertificateValidityPolicy
{
public:
    ~AlwaysAcceptValidityPolicy() {}

    CHIP_ERROR ApplyCertificateValidityPolicy(const ChipCertificateData * cert, uint8_t depth,
                                              CertificateValidityResult result) override
    {
        return CHIP_NO_ERROR;
    }
};

class AlwaysRejectValidityPolicy : public CertificateValidityPolicy
{
public:
    ~AlwaysRejectValidityPolicy() {}

    CHIP_ERROR ApplyCertificateValidityPolicy(const ChipCertificateData * cert, uint8_t depth,
                                              CertificateValidityResult result) override
    {
        return CHIP_ERROR_CERT_EXPIRED;
    }
};

TEST_F(TestChipCert, TestChipCert_CertValidityPolicyInjection)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ValidationContext validContext;
    StrictCertificateValidityPolicyExample strictPolicy;
    LastKnownGoodTimeCertificateValidityPolicyExample lastKnownGoodTimePolicy;
    AlwaysAcceptValidityPolicy alwaysAcceptPolicy;
    AlwaysRejectValidityPolicy alwaysRejectPolicy;

    err = certSet.Init(kStandardCertsCount);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = LoadTestCertSet01(certSet);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    // Current time unknown.

    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimePolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always accept policy
    validContext.mValidityPolicy = &alwaysAcceptPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always reject policy
    validContext.mValidityPolicy = &alwaysRejectPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Curent time before certificate validity period.
    err = SetCurrentTime(validContext, 2020, 1, 3);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);
    // Strict policy
    validContext.mValidityPolicy = &strictPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimePolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_NOT_VALID_YET);
    // Always accept policy
    validContext.mValidityPolicy = &alwaysAcceptPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always reject policy
    validContext.mValidityPolicy = &alwaysRejectPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Last known good time before certificate validity period.
    err = SetLastKnownGoodTime(validContext, 2020, 1, 3);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always accept policy
    validContext.mValidityPolicy = &alwaysAcceptPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always reject policy
    validContext.mValidityPolicy = &alwaysRejectPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Current time during validity period
    err = SetCurrentTime(validContext, 2022, 02, 23, 12, 30, 01);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always accept policy
    validContext.mValidityPolicy = &alwaysAcceptPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always reject policy
    validContext.mValidityPolicy = &alwaysRejectPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Last Known Good Time during validity period
    err = SetLastKnownGoodTime(validContext, 2022, 02, 23, 12, 30, 01);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimePolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always accept policy
    validContext.mValidityPolicy = &alwaysAcceptPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always reject policy
    validContext.mValidityPolicy = &alwaysRejectPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Current time after end of certificate validity period.
    err = SetCurrentTime(validContext, 2042, 4, 25, 0, 0, 0);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Strict policy
    validContext.mValidityPolicy = &strictPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimePolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Always accept policy
    validContext.mValidityPolicy = &alwaysAcceptPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always reject policy
    validContext.mValidityPolicy = &alwaysRejectPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Last Known Good Time after end of certificate validity period.
    err = SetLastKnownGoodTime(validContext, 2042, 4, 25, 0, 0, 0);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Default policy
    validContext.mValidityPolicy = nullptr;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Strict policy
    validContext.mValidityPolicy = &strictPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Last Known Good Time policy
    validContext.mValidityPolicy = &lastKnownGoodTimePolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_ERROR_CERT_EXPIRED);
    // Always accept policy
    validContext.mValidityPolicy = &alwaysAcceptPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Always reject policy
    validContext.mValidityPolicy = &alwaysRejectPolicy;
    err                          = certSet.ValidateCert(certSet.GetLastCert(), validContext);
    EXPECT_NE(err, CHIP_NO_ERROR);

    certSet.Release();
}

TEST_F(TestChipCert, TestChipCert_CertUsage)
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
    size_t sNumUsageTestCases = MATTER_ARRAY_SIZE(sUsageTestCases);

    err = certSet.Init(certDataArray, MATTER_ARRAY_SIZE(certDataArray));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = LoadTestCertSet01(certSet);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    for (size_t i = 0; i < sNumUsageTestCases; i++)
    {
        validContext.Reset();
        validContext.mRequiredKeyUsages   = sUsageTestCases[i].mRequiredKeyUsages;
        validContext.mRequiredKeyPurposes = sUsageTestCases[i].mRequiredKeyPurposes;

        err = SetCurrentTime(validContext, 2020, 10, 16);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = certSet.ValidateCert(&certSet.GetCertSet()[sUsageTestCases[i].mCertIndex], validContext);
        EXPECT_EQ(err, sUsageTestCases[i].mExpectedResult);
    }

    certSet.Release();
}

TEST_F(TestChipCert, TestChipCert_CertType)
{
    CHIP_ERROR err;
    ChipCertificateData certData;

    struct TestCase
    {
        TestCert Cert;
        CertType ExpectedCertType;
    };

    // clang-format off
    static TestCase sTestCases[] = {
        // Cert                        ExpectedCertType
        // =============================================================
        {  TestCert::kRoot01,          CertType::kRoot            },
        {  TestCert::kRoot02,          CertType::kRoot            },
        {  TestCert::kICA01,           CertType::kICA             },
        {  TestCert::kICA02,           CertType::kICA             },
        {  TestCert::kICA01_1,         CertType::kICA             },
        {  TestCert::kFWSign01,        CertType::kFirmwareSigning },
        {  TestCert::kNode01_01,       CertType::kNode            },
        {  TestCert::kNode01_02,       CertType::kNode            },
        {  TestCert::kNode02_01,       CertType::kNode            },
        {  TestCert::kNode02_02,       CertType::kNode            },
        {  TestCert::kPDCID01,         CertType::kNetworkIdentity },
    };
    // clang-format on
    for (const auto & testCase : sTestCases)
    {
        CertType certType;

        err = DecodeTestCert(certData, testCase.Cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = certData.mSubjectDN.GetCertType(certType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(certType, testCase.ExpectedCertType);
    }
}

TEST_F(TestChipCert, TestChipCert_CertId)
{
    CHIP_ERROR err;
    ChipCertificateData certData;

    struct TestCase
    {
        TestCert Cert;
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
        {  TestCert::kPDCID01,         0 },
    };
    // clang-format on
    for (const auto & testCase : sTestCases)
    {
        uint64_t chipId;

        err = DecodeTestCert(certData, testCase.Cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = certData.mSubjectDN.GetCertChipId(chipId);
        if (testCase.ExpectedCertId != 0)
        {
            EXPECT_EQ(err, CHIP_NO_ERROR);
            EXPECT_EQ(chipId, testCase.ExpectedCertId);
        }
        else
        {
            EXPECT_EQ(err, CHIP_ERROR_WRONG_CERT_DN);
        }
    }
}

TEST_F(TestChipCert, TestChipCert_DecodingOptions)
{
    CHIP_ERROR err;
    ByteSpan cert;
    ChipCertificateData certData;

    err = GetTestCert(TestCert::kRoot01, sNullLoadFlag, cert);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Decode with default (null) options
    err = DecodeChipCert(cert, certData);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(certData.mCertFlags.Has(CertFlags::kIsTrustAnchor));

    // Decode as trust anchor
    err = DecodeChipCert(cert, certData, CertDecodeFlags::kIsTrustAnchor);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(certData.mCertFlags.Has(CertFlags::kIsTrustAnchor));

    // Decode with TBS Hash calculation
    err = DecodeChipCert(cert, certData, CertDecodeFlags::kGenerateTBSHash);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(certData.mCertFlags.Has(CertFlags::kTBSHashPresent));
    // When the TBS hash is available signature verification should work
    err = VerifyCertSignature(certData, certData); // test cert is a self-signed root
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestChipCert, TestChipCert_LoadDuplicateCerts)
{
    CHIP_ERROR err;
    ChipCertificateSet certSet;
    ValidationContext validContext;

    err = certSet.Init(kStandardCertsCount);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Let's load two distinct certificates, and make sure cert count is 2
    err = LoadTestCert(certSet, TestCert::kRoot01, sNullLoadFlag, sTrustAnchorFlag);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = LoadTestCert(certSet, TestCert::kICA01, sNullLoadFlag, sGenTBSHashFlag);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(certSet.GetCertCount(), 2);

    // Let's load a previously loaded cert and make sure cert count is still 2
    err = LoadTestCert(certSet, TestCert::kRoot01, sNullLoadFlag, sTrustAnchorFlag);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(certSet.GetCertCount(), 2);

    // Let's load the other previously loaded cert and make sure cert count is still 2
    err = LoadTestCert(certSet, TestCert::kICA01, sNullLoadFlag, sGenTBSHashFlag);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(certSet.GetCertCount(), 2);

    // Let's load a new cert and make sure cert count updates to 3
    err = LoadTestCert(certSet, TestCert::kNode01_01, sNullLoadFlag, sGenTBSHashFlag);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(certSet.GetCertCount(), 3);
}

TEST_F(TestChipCert, TestChipCert_GenerateRootCert)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    ChipCertificateData certData;

    ChipDN root_dn;
    EXPECT_EQ(root_dn.AddAttribute_MatterRCACId(0xabcdabcd), CHIP_NO_ERROR);

    X509CertRequestParams root_params = { 1234, 631161876, 729942000, root_dn, root_dn };
    MutableByteSpan signed_cert_span(signed_cert);
    EXPECT_EQ(NewRootX509Cert(root_params, keypair, signed_cert_span), CHIP_NO_ERROR);

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span, outCert), CHIP_NO_ERROR);

    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);

    // Test with FutureExtension
    X509CertRequestParams root_params2 = { 1234, 631161876, 729942000, root_dn, root_dn, kSubjectAltNameAsFutureExt };
    MutableByteSpan signed_cert_span2(signed_cert);
    EXPECT_EQ(NewRootX509Cert(root_params2, keypair, signed_cert_span2), CHIP_NO_ERROR);
    outCert = MutableByteSpan(outCertBuf);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span2, outCert), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);

    // Test with no defined notAfter time.
    {
        X509CertRequestParams root_params3 = { .SerialNumber  = 1234,
                                               .ValidityStart = 631161876,
                                               .ValidityEnd   = kNullCertTime,
                                               .SubjectDN     = root_dn,
                                               .IssuerDN      = root_dn };
        MutableByteSpan signed_cert_span_no_expiry(signed_cert);

        EXPECT_EQ(NewRootX509Cert(root_params3, keypair, signed_cert_span_no_expiry), CHIP_NO_ERROR);
        outCert = MutableByteSpan(outCertBuf);

        EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span_no_expiry, outCert), CHIP_NO_ERROR);
        EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);
        EXPECT_EQ(certData.mNotAfterTime, kNullCertTime);
    }

    // Test error case: root cert subject provided ICA OID Attribute.
    root_params.SubjectDN.Clear();
    EXPECT_EQ(root_params.SubjectDN.AddAttribute_MatterICACId(0xabcdabcd), CHIP_NO_ERROR);
    root_params.IssuerDN.Clear();
    EXPECT_EQ(root_params.IssuerDN.AddAttribute_MatterICACId(0xabcdabcd), CHIP_NO_ERROR);
    MutableByteSpan signed_cert_span1(signed_cert);
    EXPECT_EQ(NewRootX509Cert(root_params, keypair, signed_cert_span1), CHIP_ERROR_INVALID_ARGUMENT);

    // Test error case: root cert provided different subject and issuer DNs.
    root_params.SubjectDN.Clear();
    EXPECT_EQ(root_params.SubjectDN.AddAttribute_MatterRCACId(0xabcdabcd), CHIP_NO_ERROR);
    root_params.IssuerDN.Clear();
    EXPECT_EQ(root_params.IssuerDN.AddAttribute_MatterRCACId(0xffffeeee), CHIP_NO_ERROR);
    EXPECT_EQ(NewRootX509Cert(root_params, keypair, signed_cert_span1), CHIP_ERROR_INVALID_ARGUMENT);

    // Test that serial number cannot be negative
    root_params.IssuerDN.Clear();
    EXPECT_EQ(root_params.IssuerDN.AddAttribute_MatterRCACId(0xabcdabcd), CHIP_NO_ERROR);
    root_params.SerialNumber = -1;
    EXPECT_EQ(NewRootX509Cert(root_params, keypair, signed_cert_span1), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestChipCert, TestChipCert_GenerateRootFabCert)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    ChipCertificateData certData;

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    ChipDN root_dn;
    EXPECT_EQ(root_dn.AddAttribute_MatterRCACId(0xabcdabcd), CHIP_NO_ERROR);
    EXPECT_EQ(root_dn.AddAttribute_MatterFabricId(0xabcd), CHIP_NO_ERROR);

    X509CertRequestParams root_params_fabric = { 1234, 631161876, 729942000, root_dn, root_dn };

    MutableByteSpan signed_cert_span(signed_cert);
    EXPECT_EQ(NewRootX509Cert(root_params_fabric, keypair, signed_cert_span), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span, outCert), CHIP_NO_ERROR);

    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);
}

TEST_F(TestChipCert, TestChipCert_GenerateICACert)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    ChipCertificateData certData;

    ChipDN ica_dn;
    EXPECT_EQ(ica_dn.AddAttribute_MatterICACId(0xABCDABCDABCDABCD), CHIP_NO_ERROR);
    ChipDN issuer_dn;
    EXPECT_EQ(issuer_dn.AddAttribute_MatterRCACId(0x43215678FEDCABCD), CHIP_NO_ERROR);

    X509CertRequestParams ica_params = { 1234, 631161876, 729942000, ica_dn, issuer_dn };
    P256Keypair ica_keypair;
    EXPECT_EQ(ica_keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    MutableByteSpan signed_cert_span(signed_cert);
    EXPECT_EQ(NewICAX509Cert(ica_params, ica_keypair.Pubkey(), keypair, signed_cert_span), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span, outCert), CHIP_NO_ERROR);

    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);

    // Test with FutureExtension
    X509CertRequestParams ica_params2 = { 1234, 631161876, 729942000, ica_dn, issuer_dn, kSubjectAltNameAsFutureExt };
    MutableByteSpan signed_cert_span2(signed_cert);
    EXPECT_EQ(NewICAX509Cert(ica_params2, ica_keypair.Pubkey(), keypair, signed_cert_span2), CHIP_NO_ERROR);
    outCert = MutableByteSpan(outCertBuf);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span2, outCert), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);

    // Test error case: ICA cert subject provided a node ID attribute
    ica_params.SubjectDN.Clear();
    EXPECT_EQ(ica_params.SubjectDN.AddAttribute_MatterNodeId(0xABCDABCDABCDABCD), CHIP_NO_ERROR);
    EXPECT_EQ(ica_params.SubjectDN.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);
    MutableByteSpan signed_cert_span1(signed_cert);
    EXPECT_EQ(NewICAX509Cert(ica_params, ica_keypair.Pubkey(), keypair, signed_cert_span1), CHIP_ERROR_INVALID_ARGUMENT);

    // Test that serial number cannot be negative
    ica_params.SubjectDN.Clear();
    EXPECT_EQ(ica_params.SubjectDN.AddAttribute_MatterICACId(0xABCDABCDABCDABCD), CHIP_NO_ERROR);
    ica_params.SerialNumber = -1;
    EXPECT_EQ(NewICAX509Cert(ica_params, ica_keypair.Pubkey(), keypair, signed_cert_span1), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestChipCert, TestChipCert_GenerateNOCRoot)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    ChipCertificateData certData;

    ChipDN noc_dn;
    EXPECT_EQ(noc_dn.AddAttribute_MatterNodeId(0xABCDABCDABCDABCD), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);
    ChipDN issuer_dn;
    EXPECT_EQ(issuer_dn.AddAttribute_MatterRCACId(0x8888999944442222), CHIP_NO_ERROR);

    X509CertRequestParams noc_params = { 123456, 631161876, 729942000, noc_dn, issuer_dn };
    P256Keypair noc_keypair;
    EXPECT_EQ(noc_keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    MutableByteSpan signed_cert_span(signed_cert, sizeof(signed_cert));
    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params, noc_keypair.Pubkey(), keypair, signed_cert_span), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span, outCert), CHIP_NO_ERROR);

    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);

    // Test with FutureExtension
    X509CertRequestParams noc_params2 = { 123456, 631161876, 729942000, noc_dn, issuer_dn, kSubjectAltNameAsFutureExt };
    MutableByteSpan signed_cert_span2(signed_cert);
    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params2, noc_keypair.Pubkey(), keypair, signed_cert_span2), CHIP_NO_ERROR);
    outCert = MutableByteSpan(outCertBuf);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span2, outCert), CHIP_NO_ERROR);
    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);

    // Test error case: NOC cert subject doesn't have NodeId attribute
    noc_params.SubjectDN.Clear();
    EXPECT_EQ(noc_params.SubjectDN.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);

    MutableByteSpan signed_cert_span1(signed_cert, sizeof(signed_cert));
    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params, noc_keypair.Pubkey(), keypair, signed_cert_span1),
              CHIP_ERROR_INVALID_ARGUMENT);

    // Test error case: NOC cert subject doesn't have fabric ID attribute
    noc_params.SubjectDN.Clear();
    EXPECT_EQ(noc_params.SubjectDN.AddAttribute_MatterNodeId(0xABCDABCDABCDABCD), CHIP_NO_ERROR);

    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params, noc_keypair.Pubkey(), keypair, signed_cert_span1), CHIP_ERROR_WRONG_CERT_DN);

    // Test error case: issuer cert DN type is Node certificate
    noc_params.SubjectDN.Clear();
    EXPECT_EQ(noc_params.SubjectDN.AddAttribute_MatterNodeId(0xABCDABCDABCDABCD), CHIP_NO_ERROR);
    EXPECT_EQ(noc_params.SubjectDN.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);
    noc_params.IssuerDN.Clear();
    EXPECT_EQ(noc_params.IssuerDN.AddAttribute_MatterNodeId(0x8888999944442222), CHIP_NO_ERROR);
    EXPECT_EQ(noc_params.IssuerDN.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);

    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params, noc_keypair.Pubkey(), keypair, signed_cert_span1),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestChipCert, TestChipCert_GenerateNOCICA)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    uint8_t signed_cert[kMaxDERCertLength];

    uint8_t outCertBuf[kMaxCHIPCertLength];
    MutableByteSpan outCert(outCertBuf);

    uint8_t outCertDERBuf[kMaxDERCertLength];
    MutableByteSpan outCertDER(outCertDERBuf);

    ChipCertificateData certData;

    const static char noc_cn_rdn[]        = "Test NOC";
    const static char noc_givenname_rdn[] = "John";
    const static char noc_name_rdn[]      = "Smith";

    ChipDN noc_dn;
    EXPECT_EQ(noc_dn.AddAttribute_CommonName(CharSpan(noc_cn_rdn, strlen(noc_cn_rdn)), false), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_MatterNodeId(0xAAAABBBBCCCCDDDD), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_GivenName(CharSpan(noc_givenname_rdn, strlen(noc_givenname_rdn)), true), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_Name(CharSpan(noc_name_rdn, strlen(noc_name_rdn)), true), CHIP_NO_ERROR);

    ChipDN ica_dn;
    EXPECT_EQ(ica_dn.AddAttribute_MatterICACId(0x8888999944442222), CHIP_NO_ERROR);
    EXPECT_EQ(ica_dn.AddAttribute_MatterFabricId(0xFAB00000FAB00001), CHIP_NO_ERROR);

    X509CertRequestParams noc_params = { 12348765, 631161876, 729942000, noc_dn, ica_dn };
    P256Keypair noc_keypair;
    EXPECT_EQ(noc_keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    MutableByteSpan signed_cert_span(signed_cert);
    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params, noc_keypair.Pubkey(), keypair, signed_cert_span), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertX509CertToChipCert(signed_cert_span, outCert), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertChipCertToX509Cert(outCert, outCertDER), CHIP_NO_ERROR);
    EXPECT_TRUE(signed_cert_span.data_equal(outCertDER));

    EXPECT_EQ(DecodeChipCert(outCert, certData), CHIP_NO_ERROR);
}

TEST_F(TestChipCert, TestChipCert_VerifyGeneratedCerts)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    ChipDN root_dn;
    EXPECT_EQ(root_dn.AddAttribute_MatterRCACId(0xAAAABBBBCCCCDDDD), CHIP_NO_ERROR);
    EXPECT_EQ(root_dn.AddAttribute_MatterFabricId(0xFAB0000000008888), CHIP_NO_ERROR);

    X509CertRequestParams root_params = { 1234, 631161876, 729942000, root_dn, root_dn };
    MutableByteSpan root_cert_span(root_cert);
    EXPECT_EQ(NewRootX509Cert(root_params, keypair, root_cert_span), CHIP_NO_ERROR);

    static uint8_t ica_cert[kMaxDERCertLength];

    ChipDN ica_dn;
    EXPECT_EQ(ica_dn.AddAttribute_MatterICACId(0xAABBCCDDAABBCCDD), CHIP_NO_ERROR);
    EXPECT_EQ(ica_dn.AddAttribute_MatterFabricId(0xFAB0000000008888), CHIP_NO_ERROR);

    X509CertRequestParams ica_params = { 12345, 631161876, 729942000, ica_dn, root_dn };
    P256Keypair ica_keypair;
    EXPECT_EQ(ica_keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    MutableByteSpan ica_cert_span(ica_cert);
    EXPECT_EQ(NewICAX509Cert(ica_params, ica_keypair.Pubkey(), keypair, ica_cert_span), CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    ChipDN noc_dn;
    EXPECT_EQ(noc_dn.AddAttribute_MatterNodeId(0xAABBCCDDAABBCCDD), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_MatterFabricId(0xFAB0000000008888), CHIP_NO_ERROR);

    X509CertRequestParams noc_params = { 123456, 631161876, 729942000, noc_dn, ica_dn };
    P256Keypair noc_keypair;
    EXPECT_EQ(noc_keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert, sizeof(noc_cert));
    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params, noc_keypair.Pubkey(), ica_keypair, noc_cert_span), CHIP_NO_ERROR);

    ChipCertificateSet certSet;
    EXPECT_EQ(certSet.Init(3), CHIP_NO_ERROR);

    static uint8_t chipRootCertBuf[kMaxCHIPCertLength];
    static uint8_t chipICACertBuf[kMaxCHIPCertLength];
    static uint8_t chipNOCCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipRootCert(chipRootCertBuf);
    MutableByteSpan chipICACert(chipICACertBuf);
    MutableByteSpan chipNOCCert(chipNOCCertBuf);

    EXPECT_EQ(ConvertX509CertToChipCert(root_cert_span, chipRootCert), CHIP_NO_ERROR);
    EXPECT_EQ(certSet.LoadCert(chipRootCert, sTrustAnchorFlag), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertX509CertToChipCert(ica_cert_span, chipICACert), CHIP_NO_ERROR);
    EXPECT_EQ(certSet.LoadCert(chipICACert, sGenTBSHashFlag), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertX509CertToChipCert(noc_cert_span, chipNOCCert), CHIP_NO_ERROR);
    EXPECT_EQ(certSet.LoadCert(chipNOCCert, sGenTBSHashFlag), CHIP_NO_ERROR);

    ValidationContext validContext;

    validContext.Reset();
    EXPECT_EQ(SetCurrentTime(validContext, 2022, 1, 1), CHIP_NO_ERROR);
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    const ChipDN & subjectDN              = certSet.GetCertSet()[2].mSubjectDN;
    const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[2].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    EXPECT_EQ(certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert), CHIP_NO_ERROR);
}

TEST_F(TestChipCert, TestChipCert_VerifyGeneratedCertsNoICA)
{
    // Generate a new keypair for cert signing
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    static uint8_t root_cert[kMaxDERCertLength];

    const static char root_cn_rdn[] = "Test Root Operational Cert";

    ChipDN root_dn;
    EXPECT_EQ(root_dn.AddAttribute_CommonName(CharSpan(root_cn_rdn, strlen(root_cn_rdn)), false), CHIP_NO_ERROR);
    EXPECT_EQ(root_dn.AddAttribute_MatterRCACId(0xAAAABBBBCCCCDDDD), CHIP_NO_ERROR);
    EXPECT_EQ(root_dn.AddAttribute_MatterFabricId(0xFAB0000000008888), CHIP_NO_ERROR);

    X509CertRequestParams root_params = { 1234, 631161876, 729942000, root_dn, root_dn };
    MutableByteSpan root_cert_span(root_cert);
    EXPECT_EQ(NewRootX509Cert(root_params, keypair, root_cert_span), CHIP_NO_ERROR);

    static uint8_t noc_cert[kMaxDERCertLength];

    const static char noc_cn_rdn[] = "Test NOC";

    ChipDN noc_dn;
    EXPECT_EQ(noc_dn.AddAttribute_CommonName(CharSpan(noc_cn_rdn, strlen(noc_cn_rdn)), true), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_MatterNodeId(0xAABBCCDDAABBCCDD), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_MatterFabricId(0xFAB0000000008888), CHIP_NO_ERROR);
    EXPECT_EQ(noc_dn.AddAttribute_MatterCASEAuthTag(0xABCD0010), CHIP_NO_ERROR);

    X509CertRequestParams noc_params = { 1234, 631161876, 729942000, noc_dn, root_dn };
    P256Keypair noc_keypair;
    EXPECT_EQ(noc_keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    MutableByteSpan noc_cert_span(noc_cert);
    EXPECT_EQ(NewNodeOperationalX509Cert(noc_params, noc_keypair.Pubkey(), keypair, noc_cert_span), CHIP_NO_ERROR);

    ChipCertificateSet certSet;
    EXPECT_EQ(certSet.Init(2), CHIP_NO_ERROR);

    static uint8_t chipRootCertBuf[kMaxCHIPCertLength];
    static uint8_t chipNOCCertBuf[kMaxCHIPCertLength];
    MutableByteSpan chipRootCert(chipRootCertBuf);
    MutableByteSpan chipNOCCert(chipNOCCertBuf);

    EXPECT_EQ(ConvertX509CertToChipCert(root_cert_span, chipRootCert), CHIP_NO_ERROR);
    EXPECT_EQ(certSet.LoadCert(chipRootCert, sTrustAnchorFlag), CHIP_NO_ERROR);

    EXPECT_EQ(ConvertX509CertToChipCert(noc_cert_span, chipNOCCert), CHIP_NO_ERROR);
    EXPECT_EQ(certSet.LoadCert(chipNOCCert, sGenTBSHashFlag), CHIP_NO_ERROR);

    ValidationContext validContext;

    validContext.Reset();
    EXPECT_EQ(SetCurrentTime(validContext, 2022, 1, 1), CHIP_NO_ERROR);
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    const ChipDN & subjectDN              = certSet.GetCertSet()[1].mSubjectDN;
    const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[1].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    EXPECT_EQ(certSet.FindValidCert(subjectDN, subjectKeyId, validContext, &resultCert), CHIP_NO_ERROR);
}

TEST_F(TestChipCert, TestChipCert_ExtractNodeIdFabricId)
{
    struct TestCase
    {
        TestCert Cert;
        TestCert ICACert;
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
        {  TestCert::kNode02_08, TestCert::kICA02, 0xDEDEDEDE00020008, 0xFAB000000000001D },
    };
    // clang-format on

    // Test node ID and fabric ID extraction from the raw ByteSpan form.
    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, sNullLoadFlag, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        NodeId nodeId;
        FabricId fabricId;
        err = ExtractNodeIdFabricIdFromOpCert(cert, &nodeId, &fabricId);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(nodeId, testCase.ExpectedNodeId);
        EXPECT_EQ(fabricId, testCase.ExpectedFabricId);
    }

    // Test node ID and fabric ID extraction from the parsed form.
    ChipCertificateSet certSet;
    for (auto & testCase : sTestCases)
    {
        CHIP_ERROR err = certSet.Init(1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = LoadTestCert(certSet, testCase.Cert, sNullLoadFlag, sNullDecodeFlag);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        NodeId nodeId;
        FabricId fabricId;
        err = ExtractNodeIdFabricIdFromOpCert(certSet.GetCertSet()[0], &nodeId, &fabricId);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(nodeId, testCase.ExpectedNodeId);
        EXPECT_EQ(fabricId, testCase.ExpectedFabricId);
        certSet.Release();
    }

    // Test fabric ID extraction from the raw ByteSpan form.
    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, sNullLoadFlag, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        FabricId fabricId;
        err = ExtractFabricIdFromCert(cert, &fabricId);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(fabricId, testCase.ExpectedFabricId);
    }

    // Test fabric ID extraction from the parsed form.
    for (auto & testCase : sTestCases)
    {
        CHIP_ERROR err = certSet.Init(1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = LoadTestCert(certSet, testCase.Cert, sNullLoadFlag, sNullDecodeFlag);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        FabricId fabricId;
        err = ExtractFabricIdFromCert(certSet.GetCertSet()[0], &fabricId);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(fabricId, testCase.ExpectedFabricId);
        certSet.Release();
    }

    // Test fabric ID extraction from the raw ByteSpan form of ICA Cert that doesn't have FabricId.
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(TestCert::kICA01, sNullLoadFlag, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        FabricId fabricId;
        err = ExtractFabricIdFromCert(cert, &fabricId);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
    }

    // Test extraction from the parsed form of ICA Cert that doesn't have FabricId.
    {
        CHIP_ERROR err = certSet.Init(1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = LoadTestCert(certSet, TestCert::kICA01, sNullLoadFlag, sNullDecodeFlag);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        FabricId fabricId;
        err = ExtractFabricIdFromCert(certSet.GetCertSet()[0], &fabricId);
        EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
        certSet.Release();
    }
}

TEST_F(TestChipCert, TestChipCert_ExtractOperationalDiscoveryId)
{
    struct TestCase
    {
        TestCert Noc;
        TestCert Rcac;
        uint64_t ExpectedNodeId;
        uint64_t ExpectedFabricId;
        uint64_t ExpectedCompressedFabricId;
    };

    // clang-format off
    static constexpr TestCase sTestCases[] = {
        // Cert                  ICA               ExpectedNodeId       ExpectedFabricId    ExpectedCompressedFabricId
        // ===========================================================================================================
        {  TestCert::kNode01_01, TestCert::kRoot01, 0xDEDEDEDE00010001, 0xFAB000000000001D, 0x3893C4324526C775 },
        {  TestCert::kNode01_02, TestCert::kRoot01, 0xDEDEDEDE00010002, 0xFAB000000000001D, 0x3893C4324526C775 },
        {  TestCert::kNode02_01, TestCert::kRoot02, 0xDEDEDEDE00020001, 0xFAB000000000001D, 0x89E8911178DAC089 },
        {  TestCert::kNode02_02, TestCert::kRoot02, 0xDEDEDEDE00020002, 0xFAB000000000001D, 0x89E8911178DAC089 },
        {  TestCert::kNode02_03, TestCert::kRoot02, 0xDEDEDEDE00020003, 0xFAB000000000001D, 0x89E8911178DAC089 },
        {  TestCert::kNode02_04, TestCert::kRoot02, 0xDEDEDEDE00020004, 0xFAB000000000001D, 0x89E8911178DAC089 },
        {  TestCert::kNode02_05, TestCert::kRoot02, 0xDEDEDEDE00020005, 0xFAB000000000001D, 0x89E8911178DAC089 },
        {  TestCert::kNode02_06, TestCert::kRoot02, 0xDEDEDEDE00020006, 0xFAB000000000001D, 0x89E8911178DAC089 },
        {  TestCert::kNode02_07, TestCert::kRoot02, 0xDEDEDEDE00020007, 0xFAB000000000001D, 0x89E8911178DAC089 },
        {  TestCert::kNode02_08, TestCert::kRoot02, 0xDEDEDEDE00020008, 0xFAB000000000001D, 0x89E8911178DAC089 },
    };
    // clang-format on

    for (auto & testCase : sTestCases)
    {
        ByteSpan noc;
        ByteSpan rcac;
        CHIP_ERROR err = GetTestCert(testCase.Noc, sNullLoadFlag, noc);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = GetTestCert(testCase.Rcac, sNullLoadFlag, rcac);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Extract Node ID and Fabric ID from the leaf node certificate.
        NodeId nodeId;
        FabricId fabricId;
        err = ExtractNodeIdFabricIdFromOpCert(noc, &nodeId, &fabricId);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(nodeId, testCase.ExpectedNodeId);
        EXPECT_EQ(fabricId, testCase.ExpectedFabricId);

        // Extract Node ID, Fabric ID and Compressed Fabric ID from the
        // NOC and root certificate.
        CompressedFabricId compressedFabricId;
        err = ExtractNodeIdFabricIdCompressedFabricIdFromOpCerts(rcac, noc, compressedFabricId, fabricId, nodeId);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(compressedFabricId, testCase.ExpectedCompressedFabricId);
        EXPECT_EQ(fabricId, testCase.ExpectedFabricId);
        EXPECT_EQ(nodeId, testCase.ExpectedNodeId);
    }
}

TEST_F(TestChipCert, TestChipCert_ExtractAndValidateCATsFromOpCert)
{
    struct TestCase
    {
        TestCert Cert;
        CATValues ExpectedCATs;
    };

    // clang-format off
    static constexpr TestCase sTestCases[] = {
        // Cert                  CATs
        // ============================================================================
        {  TestCert::kNode01_01, { { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } } },
        {  TestCert::kNode01_02, { { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } } },
        {  TestCert::kNode02_01, { { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } } },
        {  TestCert::kNode02_02, { { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } } },
        {  TestCert::kNode02_03, { {    0xABCD0001, kUndefinedCAT, kUndefinedCAT } } },
        {  TestCert::kNode02_04, { {    0xABCE1002,    0xABCD0003, kUndefinedCAT } } },
        {  TestCert::kNode02_05, { {    0xABCD0010,    0xABCE1008, kUndefinedCAT } } },
        {  TestCert::kNode02_06, { { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } } },
        {  TestCert::kNode02_07, { { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } } },
        {  TestCert::kNode02_08, { {    0xABCF00A0,    0xABCD0020,    0xABCE0100 } } },
    };
    // clang-format on

    // Test extraction from the raw ByteSpan form.
    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, sNullLoadFlag, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        CATValues cats;
        err = ExtractCATsFromOpCert(cert, cats);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(&cats, &testCase.ExpectedCATs, sizeof(cats)), 0);
    }

    // Test extraction from the parsed form.
    ChipCertificateSet certSet;
    for (auto & testCase : sTestCases)
    {
        CHIP_ERROR err = certSet.Init(1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = LoadTestCert(certSet, testCase.Cert, sNullLoadFlag, sNullDecodeFlag);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        CATValues cats;
        err = ExtractCATsFromOpCert(certSet.GetCertSet()[0], cats);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(&cats, &testCase.ExpectedCATs, sizeof(cats)), 0);

        certSet.Release();
    }

    // Error case: trying to extract CAT from Root Cert.
    {
        CHIP_ERROR err = certSet.Init(1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = LoadTestCert(certSet, TestCert::kRoot01, sNullLoadFlag, sNullDecodeFlag);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        CATValues cats;
        err = ExtractCATsFromOpCert(certSet.GetCertSet()[0], cats);
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

        certSet.Release();
    }

    // Error case: NOC with invalid CAT version.
    {
        CATValues cats;
        CHIP_ERROR err = ExtractCATsFromOpCert(ByteSpan(sChipTest_NOC_Subject_CAT_Invalid_Cert_CHIP), cats);
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Error case: NOC with multiple versions of the same CAT tag.
    {
        CATValues cats;
        CHIP_ERROR err = ExtractCATsFromOpCert(ByteSpan(sChipTest_NOC_Subject_CAT_Twice_Cert_CHIP), cats);
        EXPECT_EQ(err, CHIP_ERROR_WRONG_CERT_DN);
    }
}

TEST_F(TestChipCert, TestChipCert_ExtractSubjectDNFromChipCert)
{
    struct TestCase
    {
        TestCert Cert;
        ChipDN ExpectedSubjectDN;
    };

    ChipDN expectedSubjectDN_Root01;
    EXPECT_EQ(expectedSubjectDN_Root01.AddAttribute_MatterRCACId(0xCACACACA00000001), CHIP_NO_ERROR);

    ChipDN expectedSubjectDN_Root02;
    EXPECT_EQ(expectedSubjectDN_Root02.AddAttribute_MatterRCACId(0xCACACACA00000002), CHIP_NO_ERROR);
    EXPECT_EQ(expectedSubjectDN_Root02.AddAttribute_MatterFabricId(0xFAB000000000001D), CHIP_NO_ERROR);

    ChipDN expectedSubjectDN_ICA02;
    EXPECT_EQ(expectedSubjectDN_ICA02.AddAttribute_MatterICACId(0xCACACACA00000004), CHIP_NO_ERROR);
    EXPECT_EQ(expectedSubjectDN_ICA02.AddAttribute_MatterFabricId(0xFAB000000000001D), CHIP_NO_ERROR);

    ChipDN expectedSubjectDN_Node01_01;
    EXPECT_EQ(expectedSubjectDN_Node01_01.AddAttribute_MatterNodeId(0xDEDEDEDE00010001), CHIP_NO_ERROR);
    EXPECT_EQ(expectedSubjectDN_Node01_01.AddAttribute_MatterFabricId(0xFAB000000000001D), CHIP_NO_ERROR);

    const static char commonName_RDN[] = "TestCert02_03";

    ChipDN expectedSubjectDN_Node02_03;
    EXPECT_EQ(expectedSubjectDN_Node02_03.AddAttribute_MatterNodeId(0xDEDEDEDE00020003), CHIP_NO_ERROR);
    EXPECT_EQ(expectedSubjectDN_Node02_03.AddAttribute_MatterFabricId(0xFAB000000000001D), CHIP_NO_ERROR);
    EXPECT_EQ(expectedSubjectDN_Node02_03.AddAttribute_CommonName(CharSpan(commonName_RDN, strlen(commonName_RDN)), false),
              CHIP_NO_ERROR);
    EXPECT_EQ(expectedSubjectDN_Node02_03.AddAttribute_MatterCASEAuthTag(0xABCD0001), CHIP_NO_ERROR);

    // clang-format off
    TestCase sTestCases[] = {
        // Cert                  SubjectDN
        // ============================================================================
        {  TestCert::kRoot01,    expectedSubjectDN_Root01    },
        {  TestCert::kRoot02,    expectedSubjectDN_Root02    },
        {  TestCert::kICA02,     expectedSubjectDN_ICA02     },
        {  TestCert::kNode01_01, expectedSubjectDN_Node01_01 },
        {  TestCert::kNode02_03, expectedSubjectDN_Node02_03 },
        {  TestCert::kPDCID01,   {}                          },
    };
    // clang-format on

    // Test extraction from the raw ByteSpan form.
    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, sNullLoadFlag, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        ChipDN subjectDN;
        err = ExtractSubjectDNFromChipCert(cert, subjectDN);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        if (!testCase.ExpectedSubjectDN.IsEmpty())
        {
            EXPECT_TRUE(subjectDN.IsEqual(testCase.ExpectedSubjectDN));
        }
    }

    // Test extraction from the X509 ByteSpan form.
    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, TestCertLoadFlags::kDERForm, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        ChipDN subjectDN;
        err = ExtractSubjectDNFromX509Cert(cert, subjectDN);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        if (!testCase.ExpectedSubjectDN.IsEmpty())
        {
            EXPECT_TRUE(subjectDN.IsEqual(testCase.ExpectedSubjectDN));
        }
    }
}

TEST_F(TestChipCert, TestChipCert_ExtractPublicKeyAndSKID)
{
    struct TestCase
    {
        TestCert Cert;
        ByteSpan ExpectedPublicKey;
        ByteSpan ExpectedSKID;
    };

    // clang-format off
    static const TestCase sTestCases[] = {
        // Cert                  ExpectedPublicKey              ExpectedSKID
        // =======================================================================================
        {  TestCert::kRoot01,    sTestCert_Root01_PublicKey,    sTestCert_Root01_SubjectKeyId    },
        {  TestCert::kRoot02,    sTestCert_Root02_PublicKey,    sTestCert_Root02_SubjectKeyId    },
        {  TestCert::kICA01,     sTestCert_ICA01_PublicKey,     sTestCert_ICA01_SubjectKeyId     },
        {  TestCert::kICA02,     sTestCert_ICA02_PublicKey,     sTestCert_ICA02_SubjectKeyId     },
        {  TestCert::kICA01_1,   sTestCert_ICA01_1_PublicKey,   sTestCert_ICA01_1_SubjectKeyId   },
        {  TestCert::kNode01_01, sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_SubjectKeyId },
        {  TestCert::kNode01_02, sTestCert_Node01_02_PublicKey, sTestCert_Node01_02_SubjectKeyId },
        {  TestCert::kNode02_01, sTestCert_Node02_01_PublicKey, sTestCert_Node02_01_SubjectKeyId },
        {  TestCert::kNode02_02, sTestCert_Node02_02_PublicKey, sTestCert_Node02_02_SubjectKeyId },
        {  TestCert::kNode02_03, sTestCert_Node02_03_PublicKey, sTestCert_Node02_03_SubjectKeyId },
        {  TestCert::kNode02_04, sTestCert_Node02_04_PublicKey, sTestCert_Node02_04_SubjectKeyId },
        {  TestCert::kNode02_05, sTestCert_Node02_05_PublicKey, sTestCert_Node02_05_SubjectKeyId },
        {  TestCert::kNode02_06, sTestCert_Node02_06_PublicKey, sTestCert_Node02_06_SubjectKeyId },
        {  TestCert::kNode02_07, sTestCert_Node02_07_PublicKey, sTestCert_Node02_07_SubjectKeyId },
        {  TestCert::kNode02_08, sTestCert_Node02_08_PublicKey, sTestCert_Node02_08_SubjectKeyId },
        {  TestCert::kPDCID01,   sTestCert_PDCID01_PublicKey,   ByteSpan() },
    };
    // clang-format on

    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        CHIP_ERROR err = GetTestCert(testCase.Cert, sNullLoadFlag, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        P256PublicKeySpan publicKey;
        err = ExtractPublicKeyFromChipCert(cert, publicKey);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(publicKey.data_equal(testCase.ExpectedPublicKey));

        CertificateKeyId skid;
        err = ExtractSKIDFromChipCert(cert, skid);
        if (!testCase.ExpectedSKID.empty())
        {
            EXPECT_EQ(err, CHIP_NO_ERROR);
            EXPECT_TRUE(skid.data_equal(testCase.ExpectedSKID));
        }
        else
        {
            EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
        }
    }
}

TEST_F(TestChipCert, TestChipCert_PDCIdentityValidation)
{
    CertificateKeyIdStorage keyId;

    // Test with both the full and compact TLV representations
    for (auto && cert : { sTestCert_PDCID01_Chip, sTestCert_PDCID01_ChipCompact })
    {
        // Validate only
        EXPECT_EQ(ValidateChipNetworkIdentity(cert), CHIP_NO_ERROR);

        // Validate and calculate identifier
        keyId.fill(0xaa);
        EXPECT_EQ(ValidateChipNetworkIdentity(cert, keyId), CHIP_NO_ERROR);
        EXPECT_TRUE(CertificateKeyId(keyId).data_equal(sTestCert_PDCID01_KeyId));

        // Extract identifier only
        keyId.fill(0xaa);
        EXPECT_EQ(ExtractIdentifierFromChipNetworkIdentity(cert, keyId), CHIP_NO_ERROR);
        EXPECT_TRUE(CertificateKeyId(keyId).data_equal(sTestCert_PDCID01_KeyId));
    }
}

TEST_F(TestChipCert, TestChipCert_PDCIdentityGeneration)
{
    // Generate a new keypair
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    // Generate an identity certificate based on the keypair
    uint8_t buffer[kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan cert(buffer);
    EXPECT_EQ(NewChipNetworkIdentity(keypair, cert), CHIP_NO_ERROR);
    EXPECT_EQ(ValidateChipNetworkIdentity(cert), CHIP_NO_ERROR);

    // It should round-trip to X.509 DER and back, and remain valid.
    uint8_t derBuffer[kMaxDERCertLength];
    MutableByteSpan derCert(derBuffer);
    EXPECT_EQ(ConvertChipCertToX509Cert(cert, derCert), CHIP_NO_ERROR);
    uint8_t tlvBuffer[kMaxCHIPCertLength];
    MutableByteSpan tlvCert(tlvBuffer); // won't be compact after round-tripping
    EXPECT_EQ(ConvertX509CertToChipCert(derCert, tlvCert), CHIP_NO_ERROR);
    EXPECT_EQ(ValidateChipNetworkIdentity(tlvCert), CHIP_NO_ERROR);
}

TEST_F(TestChipCert, TestChipCert_KeypairConversion)
{
    P256SerializedKeypair keypair;
    EXPECT_EQ(GetTestCertKeypair(kPDCID01, keypair), CHIP_NO_ERROR);

    uint8_t buffer[kP256ECPrivateKeyDERLength];
    MutableByteSpan keypairDer(buffer);
    EXPECT_EQ(ConvertECDSAKeypairRawToDER(keypair, keypairDer), CHIP_NO_ERROR);

    // Technically the curve name and public key are optional in the DER format,
    // but both our code and standard tools include them, so we can just compare.
    EXPECT_TRUE(keypairDer.data_equal(sTestCert_PDCID01_KeypairDER));
}
